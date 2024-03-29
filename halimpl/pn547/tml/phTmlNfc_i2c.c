/*
 * Copyright (C) 2010-2014 NXP Semiconductors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * DAL I2C port implementation for linux
 *
 * Project: Trusted NFC Linux
 *
 */
#include <hardware/nfc.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <errno.h>

#include <phNxpLog.h>
#include <phTmlNfc_i2c.h>
#include <phNfcStatus.h>
#include <string.h>

#include <linux/pn544.h>

#define CRC_LEN                     2
#define NORMAL_MODE_HEADER_LEN      3
#define FW_DNLD_HEADER_LEN          2
#define FW_DNLD_LEN_OFFSET          1
#define NORMAL_MODE_LEN_OFFSET      2

static bool_t bFwDnldFlag = FALSE;

/*******************************************************************************
**
** Function         phTmlNfc_i2c_close
**
** Description      Closes PN547 device
**
** Parameters       pDevHandle - device handle
**
** Returns          None
**
*******************************************************************************/
void phTmlNfc_i2c_close(void *pDevHandle)
{
    if (NULL != pDevHandle)
    {
        close((int32_t)pDevHandle);
    }

    return;
}

/*******************************************************************************
**
** Function         phTmlNfc_i2c_open_and_configure
**
** Description      Open and configure pn547 device
**
** Parameters       pConfig     - hardware information
**                  pLinkHandle - device handle
**
** Returns          NFC status:
**                  NFCSTATUS_SUCCESS            - open_and_configure operation success
**                  NFCSTATUS_INVALID_DEVICE     - device open operation failure
**
*******************************************************************************/
NFCSTATUS phTmlNfc_i2c_open_and_configure(pphTmlNfc_Config_t pConfig, void ** pLinkHandle)
{
    int nHandle;


    NXPLOG_TML_D("Opening port=%s\n", pConfig->pDevName);
    /* open port */
    nHandle = open((char const *)pConfig->pDevName, O_RDWR);
    if (nHandle < 0)
    {
        NXPLOG_TML_E("_i2c_open() Failed: retval %x",nHandle);
        *pLinkHandle = NULL;
        return NFCSTATUS_INVALID_DEVICE;
    }

    *pLinkHandle = (void*) nHandle;

    /*Reset PN547*/
    phTmlNfc_i2c_reset((void *)nHandle, 1);
    usleep(100 * 1000);
    phTmlNfc_i2c_reset((void *)nHandle, 0);
    usleep(100 * 1000);
    phTmlNfc_i2c_reset((void *)nHandle, 1);

    return NFCSTATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         phTmlNfc_i2c_read
**
** Description      Reads requested number of bytes from pn547 device into given buffer
**
** Parameters       pDevHandle       - valid device handle
**                  pBuffer          - buffer for read data
**                  nNbBytesToRead   - number of bytes requested to be read
**
** Returns          numRead   - number of successfully read bytes
**                  -1        - read operation failure
**
*******************************************************************************/
int phTmlNfc_i2c_read(void *pDevHandle, uint8_t * pBuffer, int nNbBytesToRead)
{
    int ret_Read;
    int ret_Select;
    int numRead = 0;
    struct timeval tv;
    fd_set rfds;
    uint16_t totalBtyesToRead = 0;

    int i;

    if (NULL == pDevHandle)
    {
        return -1;
    }

    if (FALSE == bFwDnldFlag)
    {
        totalBtyesToRead = NORMAL_MODE_HEADER_LEN;
    }
    else
    {
        totalBtyesToRead = FW_DNLD_HEADER_LEN;
    }

    /* Read with 2 second timeout, so that the read thread can be aborted
       when the pn547 does not respond and we need to switch to FW download
       mode. This should be done via a control socket instead. */
    FD_ZERO(&rfds);
    FD_SET((int) pDevHandle, &rfds);
    tv.tv_sec = 2;
    tv.tv_usec = 1;

    ret_Select = select((int)((int)pDevHandle + (int)1), &rfds, NULL, NULL, &tv);
    if (ret_Select < 0)
    {
        NXPLOG_TML_E("i2c select() errno : %x",errno);
        return -1;
    }
    else if (ret_Select == 0)
    {
        NXPLOG_TML_E("i2c select() Timeout");
        return -1;
    }
    else
    {
        ret_Read = read((int)pDevHandle, pBuffer, totalBtyesToRead - numRead);
        if (ret_Read > 0)
        {
            numRead += ret_Read;
        }
        else if (ret_Read == 0)
        {
            NXPLOG_TML_E("_i2c_read() [hdr]EOF");
            return -1;
        }
        else
        {
            NXPLOG_TML_E("_i2c_read() [hdr] errno : %x",errno);
            return -1;
        }

        if (FALSE == bFwDnldFlag)
        {
            totalBtyesToRead = NORMAL_MODE_HEADER_LEN;
        }
        else
        {
            totalBtyesToRead = FW_DNLD_HEADER_LEN;
        }

        if(numRead < totalBtyesToRead)
        {
            ret_Read = read((int)pDevHandle, pBuffer, totalBtyesToRead - numRead);
            if (ret_Read != totalBtyesToRead - numRead)
            {
                NXPLOG_TML_E("_i2c_read() [hdr] errno : %x",errno);
                return -1;
            }
            else
            {
                numRead += ret_Read;
            }
        }
        if(TRUE == bFwDnldFlag)
        {
            totalBtyesToRead = pBuffer[FW_DNLD_LEN_OFFSET] + FW_DNLD_HEADER_LEN + CRC_LEN;
        }
        else
        {
            totalBtyesToRead = pBuffer[NORMAL_MODE_LEN_OFFSET] + NORMAL_MODE_HEADER_LEN;
        }
        ret_Read = read((int)pDevHandle, (pBuffer + numRead), totalBtyesToRead - numRead);
        if (ret_Read > 0)
        {
            numRead += ret_Read;
        }
        else if (ret_Read == 0)
        {
            NXPLOG_TML_E("_i2c_read() [pyld] EOF");
            return -1;
        }
        else
        {
            NXPLOG_TML_E("_i2c_read() [pyld] errno : %x",errno);
            if (errno == EINTR || errno == EAGAIN )
            {
                return -1;
            }
        }
    }
    return numRead;
}

/*******************************************************************************
**
** Function         phTmlNfc_i2c_write
**
** Description      Writes requested number of bytes from given buffer into pn547 device
**
** Parameters       pDevHandle       - valid device handle
**                  pBuffer          - buffer for read data
**                  nNbBytesToWrite  - number of bytes requested to be written
**
** Returns          numWrote   - number of successfully written bytes
**                  -1         - write operation failure
**
*******************************************************************************/
int phTmlNfc_i2c_write(void *pDevHandle, uint8_t * pBuffer, int nNbBytesToWrite)
{
    int ret;
    int numWrote = 0;
    int i;

    if (NULL == pDevHandle)
    {
        return -1;
    }

    while (numWrote < nNbBytesToWrite)
    {
        ret = write((int32_t)pDevHandle, pBuffer + numWrote, nNbBytesToWrite - numWrote);
        if (ret > 0)
        {
            numWrote += ret;
        }
        else if (ret == 0)
        {
            NXPLOG_TML_E("_i2c_write() EOF");
            return -1;
        }
        else
        {
            NXPLOG_TML_E("_i2c_write() errno : %x",errno);
            if (errno == EINTR || errno == EAGAIN)
            {
                continue;
            }
            return -1;
        }
    }

    return numWrote;
}

/*******************************************************************************
**
** Function         phTmlNfc_i2c_reset
**
** Description      Reset pn547 device, using VEN pin
**
** Parameters       pDevHandle     - valid device handle
**                  level          - reset level
**
** Returns           0   - reset operation success
**                  -1   - reset operation failure
**
*******************************************************************************/
int phTmlNfc_i2c_reset(void *pDevHandle, long level)
{
    int ret;
    NXPLOG_TML_D("phTmlNfc_i2c_reset(), VEN level %ld", level);

    if (NULL == pDevHandle)
    {
        return -1;
    }

    ret = ioctl((int32_t)pDevHandle, PN544_SET_PWR, level);
    if(level == 2 && ret == 0)
    {
        bFwDnldFlag = TRUE;
    }else{
        bFwDnldFlag = FALSE;
    }
    return ret;
}

/*******************************************************************************
**
** Function         getDownloadFlag
**
** Description      Returns the current mode
**
** Parameters       none
**
** Returns           Current mode download/NCI
*******************************************************************************/
bool_t getDownloadFlag(void)
{

    return bFwDnldFlag;
}


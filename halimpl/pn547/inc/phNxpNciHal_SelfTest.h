/*
 * Copyright (C) 2012 NXP Semiconductors
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
#ifndef _PHNXPNCIHAL_SELFTEST_H_
#define _PHNXPNCIHAL_SELFTEST_H_

#ifdef NXP_HW_SELF_TEST

#include <phNfcStatus.h>
#include <phNxpNciHal.h>
#include <phTmlNfc.h>

/* RF Technology */
typedef enum
{
    NFC_RF_TECHNOLOGY_A,
    NFC_RF_TECHNOLOGY_B,
    NFC_RF_TECHNOLOGY_F,
} phNxpNfc_Tech_t;

/* Bit rates */
typedef enum
{
    NFC_BIT_RATE_106,
    NFC_BIT_RATE_212,
    NFC_BIT_RATE_424,
    NFC_BIT_RATE_848,
} phNxpNfc_Bitrate_t;

/*******************************************************************************
 **
 ** Function         phNxpNciHal_TestMode_open
 **
 ** Description      It opens the physical connection with NFCC (pn547) and
 **                  creates required client thread for operation.
 **
 ** Returns          NFCSTATUS_SUCCESS if successful,otherwise NFCSTATUS_FAILED.
 **
 *******************************************************************************/

NFCSTATUS phNxpNciHal_TestMode_open (void);

/*******************************************************************************
 **
 ** Function         phNxpNciHal_TestMode_close
 **
 ** Description      This function close the NFCC interface and free all
 **                  resources.
 **
 ** Returns          None.
 **
 *******************************************************************************/

void phNxpNciHal_TestMode_close (void);

/*******************************************************************************
 **
 ** Function         phNxpNciHal_SwpTest
 **
 ** Description      Test function to validate the SWP line. SWP line number is
 **                  is sent as parameter to the API.
 **
 ** Returns          NFCSTATUS_SUCCESS if successful,otherwise NFCSTATUS_FAILED.
 **
 *******************************************************************************/

NFCSTATUS phNxpNciHal_SwpTest (uint8_t swp_line);

/*******************************************************************************
 **
 ** Function         phNxpNciHal_PrbsTestStart
 **
 ** Description      Test function start RF generation for RF technology and bit
 **                  rate. RF technology and bit rate are sent as parameter to
 **                  the API.
 **
 ** Returns          NFCSTATUS_SUCCESS if RF generation successful,
 **                  otherwise NFCSTATUS_FAILED.
 **
 *******************************************************************************/

NFCSTATUS phNxpNciHal_PrbsTestStart (phNxpNfc_Tech_t tech, phNxpNfc_Bitrate_t bitrate);

/*******************************************************************************
 **
 ** Function         phNxpNciHal_PrbsTestStop
 **
 ** Description      Test function stop RF generation for RF technology started
 **                  by phNxpNciHal_PrbsTestStart.
 **
 ** Returns          NFCSTATUS_SUCCESS if operation successful,
 **                  otherwise NFCSTATUS_FAILED.
 **
 *******************************************************************************/

NFCSTATUS phNxpNciHal_PrbsTestStop ();

/*******************************************************************************
**
** Function         phNxpNciHal_AntennaSelfTest
**
** Description      Test function to validate the Antenna's discrete
**                  components connection.
**
** Returns          NFCSTATUS_SUCCESS if successful,otherwise NFCSTATUS_FAILED.
**
*******************************************************************************/

NFCSTATUS phNxpNciHal_AntennaTest (void);

/*******************************************************************************
**
** Function         phNxpNciHal_RfFieldTest
**
** Description      Test function performs RF filed test.
**
** Returns          NFCSTATUS_SUCCESS if successful,otherwise NFCSTATUS_FAILED.
**
*******************************************************************************/

NFCSTATUS phNxpNciHal_RfFieldTest (uint8_t on);

/*******************************************************************************
 **
 ** Function         phNxpNciHal_DownloadPinTest
 **
 ** Description      Test function to validate the FW download pin connection.
 **
 ** Returns          NFCSTATUS_SUCCESS if successful,otherwise NFCSTATUS_FAILED.
 **
 *******************************************************************************/

NFCSTATUS phNxpNciHal_DownloadPinTest (void);

#endif /* _NXP_HW_SELF_TEST_H_ */
#endif /* _PHNXPNCIHAL_SELFTEST_H_ */

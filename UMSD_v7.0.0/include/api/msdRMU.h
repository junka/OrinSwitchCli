/**********************************************************************************************
* Copyright (c) 2023 Marvell.
* All rights reserved.
* Use of this source code is governed by a BSD3 license that
* can be found in the LICENSE file and also at https://opensource.org/licenses/BSD-3-Clause
**********************************************************************************************/

/*******************************************************************************
* msdRMU.h
*
* DESCRIPTION:
*       API/Structure definitions for Marvell RMU functionality.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*******************************************************************************/

#ifndef msdRMU_h
#define msdRMU_h

#include <driver/msdApiTypes.h>
#include <driver/msdSysConfig.h>
#include <driver/msdHwAccess.h>
#include <api/msdBrgFdb.h>
#include <api/msdPortRmon.h>
#include <api/msdTCAM.h>


#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* msdRMUGetID
*
* DESCRIPTION:
*       Gets the current Device ID.
*
* INPUTS:
*       None
*
* OUTPUTS:
*       id - DUT Id.
*
* RETURNS:
*       MSD_OK      - on success.
*       MSD_FAIL    - on error.
*       MSD_BAD_PARAM - if invalid parameter is given
*       MSD_NOT_SUPPORTED - device not supported
*
* COMMENTS:
*
*******************************************************************************/
MSD_STATUS msdRMUGetID
(
	IN  MSD_U8  devNum,
	OUT MSD_U16 *id
);

/*******************************************************************************
* msdRMUAtuDump
*
* DESCRIPTION:
*       Dump ATU entry from the specified starting address.
*
* INPUTS:
*       startAddr - starting address to search the valid ATU entryies
*
* OUTPUTS:
*       atuEntry - array of atuEntries.
*		numOfEntry - number of returned valued entries
*
* RETURNS:
*       MSD_OK      - on success.
*       MSD_FAIL    - on error.
*       MSD_BAD_PARAM - if invalid parameter is given
*       MSD_NOT_SUPPORTED - device not supported
*
* COMMENTS:
*
*******************************************************************************/
MSD_STATUS msdRMUAtuDump
(
	IN  MSD_U8  devNum,
	INOUT MSD_U32 *startAddr,
	INOUT MSD_U32 *numOfEntry,
	OUT MSD_ATU_ENTRY **atuEntry
);

/*******************************************************************************
* msdRMUEcidDump
*
* DESCRIPTION:
*       Dump ECID entry from the specified starting address.
*
* INPUTS:
*       startAddr - starting address to search the valid ATU entryies
*
* OUTPUTS:
*       ecidEntry - array of atuEntries.
*		numOfEntry - number of returned valued entries
*
* RETURNS:
*       MSD_OK      - on success.
*       MSD_FAIL    - on error.
*       MSD_BAD_PARAM - if invalid parameter is given
*       MSD_NOT_SUPPORTED - device not supported
*
* COMMENTS:
*
*******************************************************************************/
MSD_STATUS msdRMUEcidDump
(
	IN  MSD_U8  devNum,
	INOUT MSD_U32 *startAddr,
	INOUT MSD_U32 *numOfEntry,
	OUT MSD_ECID_ENTRY **atuEntry
);

/*******************************************************************************
* msdRMUMibDump
*
* DESCRIPTION:
*       Dump MIB from the specified port.
*
* INPUTS:
*       enClearOnRead - indicate dump mib clear or not
*		portNum - request port
*
* OUTPUTS:
*       statsCounterSet - mib info.
*
* RETURNS:
*       MSD_OK      - on success.
*       MSD_FAIL    - on error.
*       MSD_BAD_PARAM - if invalid parameter is given
*       MSD_NOT_SUPPORTED - device not supported
*
* COMMENTS:
*
*******************************************************************************/
MSD_STATUS msdRMUMibDump
(
	IN  MSD_U8  devNum,
	IN MSD_BOOL enClearOnRead,
	IN MSD_LPORT portNum,
	OUT MSD_STATS_COUNTER_SET_LEGACY *statsCounterSet
);

/*******************************************************************************
* msdRMUMib2Dump
*
* DESCRIPTION:
*       Dump MIB from the specified port.
*
* INPUTS:
*       enClearOnRead - indicate dump mib clear or not
*		portNum - request port
*
* OUTPUTS:
*       statsCounterSet - mib info.
*
* RETURNS:
*       MSD_OK      - on success.
*       MSD_FAIL    - on error.
*       MSD_BAD_PARAM - if invalid parameter is given
*       MSD_NOT_SUPPORTED - device not supported
*
* COMMENTS:
*
*******************************************************************************/
MSD_STATUS msdRMUMib2Dump
(
	IN  MSD_U8  devNum,
	IN MSD_BOOL enClearOnRead,
	IN MSD_LPORT portNum,
	OUT MSD_STATS_COUNTER_SET *statsCounterSet
);

/*******************************************************************************
* msdRMUMultiRegAccess
*
* DESCRIPTION:
*       multiple register read/write.
*
* INPUTS:
*       RegCmd - array of Read/Write requests
*		nCmd - number of register access commands
*
* OUTPUTS:
*       RegCmd - update read-back register value if it has register read request
*
* RETURNS:
*       MSD_OK      - on success.
*       MSD_FAIL    - on error.
*       MSD_BAD_PARAM - if invalid parameter is given
*       MSD_NOT_SUPPORTED - device not supported
*
* COMMENTS:
*
*******************************************************************************/
MSD_STATUS msdRMUMultiRegAccess
(
	IN  MSD_U8  devNum,
	INOUT MSD_RegCmd **regCmdList,
	IN MSD_U32 nCmd
);

/*******************************************************************************
* msdRMUTcamDump
*
* DESCRIPTION:
*       Dump valid TCAM entries.
*
* INPUTS:
*       startAddr - starting address to search the valid tcam entryies
*
* OUTPUTS:
*       tcamEntry - array of tcam Entries.
*		numOfEntry - number of returned valued entries
*
* RETURNS:
*       MSD_OK      - on success.
*       MSD_FAIL    - on error.
*       MSD_BAD_PARAM - if invalid parameter is given
*       MSD_NOT_SUPPORTED - device not supported
*
* COMMENTS:
*
*******************************************************************************/
MSD_STATUS msdRMUTcamDump
(
    IN  MSD_U8  devNum,
	INOUT MSD_U32 *startAddr,
	INOUT MSD_U32 *numOfEntry,
	OUT MSD_TCAM_DATA **tcamEntry
);

/*******************************************************************************
* msdRMUTcamEntryAdd
*
* DESCRIPTION:
*       single TCAM entry load.
*
* INPUTS:
*       tcamEntry - entry
*
* OUTPUTS:
*       NONE
*
* RETURNS:
*       MSD_OK      - on success.
*       MSD_FAIL    - on error.
*       MSD_BAD_PARAM - if invalid parameter is given
*       MSD_NOT_SUPPORTED - device not supported
*
* COMMENTS:
*
*******************************************************************************/
MSD_STATUS msdRMUTcamEntryAdd
(
	IN  MSD_U8  devNum,
	IN MSD_U32 tcamPointer,
	IN MSD_TCAM_DATA *tcamEntry
);

/*******************************************************************************
* msdRMUTcamEntryRead
*
* DESCRIPTION:
*       single TCAM entry read-back.
*
* INPUTS:
*       tcamPointer - tcam pointer
*
* OUTPUTS:
*       tcamEntry - entry
*
* RETURNS:
*       MSD_OK      - on success.
*       MSD_FAIL    - on error.
*       MSD_BAD_PARAM - if invalid parameter is given
*       MSD_NOT_SUPPORTED - device not supported
*
* COMMENTS:
*
*******************************************************************************/
MSD_STATUS msdRMUTcamEntryRead
(
	IN  MSD_U8  devNum,
	IN MSD_U32 tcamPointer,
	OUT MSD_TCAM_DATA *tcamEntry
);

/*******************************************************************************
* msdRMURegisterDump
*
* DESCRIPTION:
*       Dump 0~31 registers for the specified devAddr.
*
* INPUTS:
*       devAddr - device address
*
* OUTPUTS:
*       dataList - 32 registers value for the specified devive address
*
* RETURNS:
*       MSD_OK      - on success.
*       MSD_FAIL    - on error.
*       MSD_BAD_PARAM - if invalid parameter is given
*       MSD_NOT_SUPPORTED - device not supported
*
* COMMENTS:
*
*******************************************************************************/
MSD_STATUS msdRMURegisterDump
(
    IN  MSD_U8 devNum,
    IN  MSD_U8 devAddr,
    OUT MSD_RegList *dataList
);

MSD_STATUS msdRMUFwVersionGet
(
    IN  MSD_U8 devNum,
    OUT MSD_U16 *major,
	OUT MSD_U16 *minor,
	OUT MSD_U16 *build,
	OUT char *version
);

MSD_STATUS msdRMURegVersionGet
(
    IN  MSD_U8 devNum,
    OUT MSD_U32 *crc
);

#ifdef __cplusplus
}
#endif

#endif /* __msdRMU_h */

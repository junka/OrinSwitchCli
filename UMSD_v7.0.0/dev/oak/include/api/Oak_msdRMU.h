/**********************************************************************************************
* Copyright (c) 2023 Marvell.
* All rights reserved.
* Use of this source code is governed by a BSD3 license that
* can be found in the LICENSE file and also at https://opensource.org/licenses/BSD-3-Clause
**********************************************************************************************/

/*******************************************************************************
* Oak_msdRMU.h
*
* DESCRIPTION:
*       API/Structure definitions for Marvell RMU functionality.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*******************************************************************************/

#ifndef Oak_msdRMU_h
#define Oak_msdRMU_h

#include <driver/msdApiTypes.h>
#include <driver/msdSysConfig.h>
#include <driver/msdHwAccess.h>
#include <utils/msdUtils.h>

#include <oak/include/api/Oak_msdBrgFdb.h>
#include <oak/include/api/Oak_msdEcid.h>
#include <oak/include/api/Oak_msdPortRmon.h>
#include <oak/include/api/Oak_msdTCAM.h>

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* msdRmuGetID
*
* DESCRIPTION:
*       Get the current Device ID.
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
*
* COMMENTS:
*
*******************************************************************************/
MSD_STATUS Oak_msdRmuGetID
(
	IN  MSD_QD_DEV *dev,
	OUT MSD_U16 *id
);
MSD_STATUS Oak_msdRmuGetIDIntf
(
	IN  MSD_QD_DEV *dev,
	OUT MSD_U16 *id
);

/*******************************************************************************
* msdRmuAtuDump
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
*
* COMMENTS:
*
*******************************************************************************/
MSD_STATUS Oak_msdRmuAtuDump
(
	IN MSD_QD_DEV *dev,
	INOUT MSD_U32 *startAddr,
	OUT MSD_U32 *numOfEntry,
	OUT OAK_MSD_ATU_ENTRY **atuEntry
);
MSD_STATUS Oak_msdRmuAtuDumpIntf
(
	IN MSD_QD_DEV *dev,
	INOUT MSD_U32 *startAddr,
	OUT MSD_U32 *numOfEntry,
	OUT MSD_ATU_ENTRY **atuEntry
);

/*******************************************************************************
* Oak_msdRmuEcidDump
*
* DESCRIPTION:
*       Dump ECID entry from the specified starting address.
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
*
* COMMENTS:
*
*******************************************************************************/
MSD_STATUS Oak_msdRmuEcidDump
(
	IN MSD_QD_DEV *dev,
	INOUT MSD_U32 *startAddr,
	OUT MSD_U32 *numOfEntry,
	OUT OAK_MSD_ECID_ENTRY **ecidEntry
);
MSD_STATUS Oak_msdRmuEcidDumpIntf
(
	IN MSD_QD_DEV *dev,
	INOUT MSD_U32 *startAddr,
	OUT MSD_U32 *numOfEntry,
	OUT MSD_ECID_ENTRY **ecidEntry
);
/*******************************************************************************
* msdRmuMib2Dump
*
* DESCRIPTION:
*       Dump MIB from the specified port.
*
* INPUTS:
*       enClearOnRead - indicate dump mib clear or not
*		port - request port
*
* OUTPUTS:
*       statsCounterSet - mib info.
*
* RETURNS:
*       MSD_OK      - on success.
*       MSD_FAIL    - on error .
*
* COMMENTS:
*
*******************************************************************************/
MSD_STATUS Oak_msdRmuMib2Dump
(
	IN MSD_QD_DEV *dev,
	IN MSD_BOOL enClearOnRead,
	IN MSD_LPORT port,
	OUT OAK_MSD_STATS_COUNTER_SET *statsCounterSet
);
MSD_STATUS Oak_msdRmuMib2DumpIntf
(
	IN MSD_QD_DEV *dev,
	IN MSD_BOOL enClearOnRead,
	IN MSD_LPORT port,
	OUT MSD_STATS_COUNTER_SET *statsCounterSet
);

/*******************************************************************************
* msdRmuReadRegister
*
* DESCRIPTION:
*       single register read.
*
* INPUTS:
*       devAddr - device address
*		regAddr - register address
*
* OUTPUTS:
*       data - return register value.
*
* RETURNS:
*       MSD_OK      - on success.
*       MSD_FAIL    - on error .
*
* COMMENTS:
*
*******************************************************************************/
MSD_STATUS Oak_msdRmuReadRegister
(
	IN MSD_QD_DEV *dev,
	IN MSD_U8 devAddr,
	IN MSD_U8 regAddr,
	OUT MSD_U16 *data
);
MSD_STATUS Oak_msdRmuReadRegisterIntf
(
	IN MSD_QD_DEV *dev,
	IN MSD_U8 devAddr,
	IN MSD_U8 regAddr,
	OUT MSD_U16 *data
);

/*******************************************************************************
* msdRmuWriteRegister
*
* DESCRIPTION:
*       single register write.
*
* INPUTS:
*       devAddr - device address
*		regAddr - register address
*		data - register value.
*
* OUTPUTS:
*       NONE
*
* RETURNS:
*       MSD_OK      - on success.
*       MSD_FAIL    - on error .
*
* COMMENTS:
*
*******************************************************************************/
MSD_STATUS Oak_msdRmuWriteRegister
(
	IN MSD_QD_DEV *dev,
	IN MSD_U8 devAddr,
	IN MSD_U8 regAddr,
	IN MSD_U16 data
);
MSD_STATUS Oak_msdRmuWriteRegisterIntf
(
	IN MSD_QD_DEV *dev,
	IN MSD_U8 devAddr,
	IN MSD_U8 regAddr,
	IN MSD_U16 data
);

/*******************************************************************************
* msdRmuMultiRegAccess
*
* DESCRIPTION:
*       multiple register read/write.
*
* INPUTS:
*       MSD_RegCmd - array of Read/Write requests
*		nCmd - number of register access commands
*
* OUTPUTS:
*       MSD_RegCmd - update read-back register value if it has register read request
*
* RETURNS:
*       MSD_OK      - on success.
*       MSD_FAIL    - on error .
*
* COMMENTS:
*
*******************************************************************************/
MSD_STATUS Oak_msdRmuMultiRegAccess
(
	IN MSD_QD_DEV *dev,
	INOUT MSD_RegCmd **regCmdList,
	IN MSD_U32 nCmd
);
MSD_STATUS Oak_msdRmuMultiRegAccessIntf
(
	IN MSD_QD_DEV *dev,
	INOUT MSD_RegCmd **regCmdList,
	IN MSD_U32 nCmd
);

/*******************************************************************************
* msdRmuTCAMDump
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
*
* COMMENTS:
*
*******************************************************************************/
MSD_STATUS Oak_msdRmuTCAMDump
(
	IN MSD_QD_DEV *dev,
	INOUT MSD_U32 *startAddr,
	OUT MSD_U32 *numOfEntry,
	OUT OAK_MSD_TCAM_DATA **tcamEntry
);
MSD_STATUS Oak_msdRmuTCAMDumpIntf
(
	IN MSD_QD_DEV *dev,
	INOUT MSD_U32 *startAddr,
	OUT MSD_U32 *numOfEntry,
	OUT MSD_TCAM_DATA **tcamEntry
);

/*******************************************************************************
* msdRmuTCAMWrite
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
*       MSD_FAIL    - on error .
*
* COMMENTS:
*
*******************************************************************************/
MSD_STATUS Oak_msdRmuTCAMWrite
(
	IN MSD_QD_DEV *dev,
	IN MSD_U32 tcamPointer,
	IN OAK_MSD_TCAM_DATA *tcamEntry
);
MSD_STATUS Oak_msdRmuTCAMWriteIntf
(
	IN MSD_QD_DEV *dev,
	IN MSD_U32 tcamPointer,
	IN MSD_TCAM_DATA *tcamEntry
);

/*******************************************************************************
* msdRmuTCAMRead
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
*       MSD_FAIL    - on error .
*
* COMMENTS:
*
*******************************************************************************/
MSD_STATUS Oak_msdRmuTCAMRead
(
	IN MSD_QD_DEV *dev,
	IN MSD_U32 tcamPointer,
	OUT OAK_MSD_TCAM_DATA *tcamEntry
);
MSD_STATUS Oak_msdRmuTCAMReadIntf
(
	IN MSD_QD_DEV *dev,
	IN MSD_U32 tcamPointer,
	OUT MSD_TCAM_DATA *tcamEntry
);

/*******************************************************************************
* Oak_msdRmuRegDump
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
MSD_STATUS Oak_msdRmuRegDump
(
    IN MSD_QD_DEV *dev,
    IN  MSD_U8 devAddr,
    OUT MSD_RegList *dataList
);


MSD_STATUS Oak_msdRmuFwVersionGet
(
	IN  MSD_QD_DEV *dev,
	OUT MSD_U16 *major,
	OUT MSD_U16 *minor,
	OUT MSD_U16 *build,
	OUT char *version
);
MSD_STATUS Oak_msdRmuFwVersionGetIntf
(
	IN  MSD_QD_DEV *dev,
	OUT MSD_U16 *major,
	OUT MSD_U16 *minor,
	OUT MSD_U16 *build,
	OUT char *version
);

#ifdef __cplusplus
}
#endif

#endif /* __Oak_msdRMU_h */

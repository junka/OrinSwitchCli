/**********************************************************************************************
* Copyright (c) 2023 Marvell.
* All rights reserved.
* Use of this source code is governed by a BSD3 license that
* can be found in the LICENSE file and also at https://opensource.org/licenses/BSD-3-Clause
**********************************************************************************************/

/*******************************************************************************
* Spruce_msdRMU.h
*
* DESCRIPTION:
*       API/Structure definitions for Marvell RMU functionality.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*******************************************************************************/

#ifndef Spruce_msdRMU_h
#define Spruce_msdRMU_h

#include <driver/msdApiTypes.h>
#include <driver/msdSysConfig.h>
#include <driver/msdHwAccess.h>
#include <utils/msdUtils.h>

#include <spruce/include/api/Spruce_msdBrgFdb.h>
#include <spruce/include/api/Spruce_msdEcid.h>
#include <spruce/include/api/Spruce_msdPortRmon.h>
#include <spruce/include/api/Spruce_msdTCAM.h>

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
MSD_STATUS Spruce_msdRmuGetID
(
	IN  MSD_QD_DEV *dev,
	OUT MSD_U16 *id
);
MSD_STATUS Spruce_msdRmuGetIDIntf
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
MSD_STATUS Spruce_msdRmuAtuDump
(
	IN MSD_QD_DEV *dev,
	INOUT MSD_U32 *startAddr,
	OUT MSD_U32 *numOfEntry,
	OUT SPRUCE_MSD_ATU_ENTRY **atuEntry
);
MSD_STATUS Spruce_msdRmuAtuDumpIntf
(
	IN MSD_QD_DEV *dev,
	INOUT MSD_U32 *startAddr,
	OUT MSD_U32 *numOfEntry,
	OUT MSD_ATU_ENTRY **atuEntry
);

/*******************************************************************************
* Spruce_msdRmuEcidDump
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
MSD_STATUS Spruce_msdRmuEcidDump
(
	IN MSD_QD_DEV *dev,
	INOUT MSD_U32 *startAddr,
	OUT MSD_U32 *numOfEntry,
	OUT SPRUCE_MSD_ECID_ENTRY **ecidEntry
);
MSD_STATUS Spruce_msdRmuEcidDumpIntf
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
MSD_STATUS Spruce_msdRmuMib2Dump
(
	IN MSD_QD_DEV *dev,
	IN MSD_BOOL enClearOnRead,
	IN MSD_LPORT port,
	OUT SPRUCE_MSD_STATS_COUNTER_SET *statsCounterSet
);
MSD_STATUS Spruce_msdRmuMib2DumpIntf
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
MSD_STATUS Spruce_msdRmuReadRegister
(
	IN MSD_QD_DEV *dev,
	IN MSD_U8 devAddr,
	IN MSD_U8 regAddr,
	OUT MSD_U16 *data
);
MSD_STATUS Spruce_msdRmuReadRegisterIntf
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
MSD_STATUS Spruce_msdRmuWriteRegister
(
	IN MSD_QD_DEV *dev,
	IN MSD_U8 devAddr,
	IN MSD_U8 regAddr,
	IN MSD_U16 data
);
MSD_STATUS Spruce_msdRmuWriteRegisterIntf
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
MSD_STATUS Spruce_msdRmuMultiRegAccess
(
	IN MSD_QD_DEV *dev,
	INOUT MSD_RegCmd **regCmdList,
	IN MSD_U32 nCmd
);
MSD_STATUS Spruce_msdRmuMultiRegAccessIntf
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
MSD_STATUS Spruce_msdRmuTCAMDump
(
	IN MSD_QD_DEV *dev,
	INOUT MSD_U32 *startAddr,
	OUT MSD_U32 *numOfEntry,
	OUT SPRUCE_MSD_TCAM_DATA **tcamEntry
);
MSD_STATUS Spruce_msdRmuTCAMDumpIntf
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
MSD_STATUS Spruce_msdRmuTCAMWrite
(
	IN MSD_QD_DEV *dev,
	IN MSD_U32 tcamPointer,
	IN SPRUCE_MSD_TCAM_DATA *tcamEntry
);
MSD_STATUS Spruce_msdRmuTCAMWriteIntf
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
MSD_STATUS Spruce_msdRmuTCAMRead
(
	IN MSD_QD_DEV *dev,
	IN MSD_U32 tcamPointer,
	OUT SPRUCE_MSD_TCAM_DATA *tcamEntry
);
MSD_STATUS Spruce_msdRmuTCAMReadIntf
(
	IN MSD_QD_DEV *dev,
	IN MSD_U32 tcamPointer,
	OUT MSD_TCAM_DATA *tcamEntry
);

/*******************************************************************************
* Spruce_msdRmuRegDump
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
MSD_STATUS Spruce_msdRmuRegDump
(
    IN MSD_QD_DEV *dev,
    IN  MSD_U8 devAddr,
    OUT MSD_RegList *dataList
);

MSD_STATUS Spruce_msdRmuFwVersionGet
(
	IN  MSD_QD_DEV *dev,
	OUT MSD_U16 *major,
	OUT MSD_U16 *minor,
	OUT MSD_U16 *build,
	OUT char *version
);
MSD_STATUS Spruce_msdRmuFwVersionGetIntf
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

#endif /* Spruce_msdRMU_h */

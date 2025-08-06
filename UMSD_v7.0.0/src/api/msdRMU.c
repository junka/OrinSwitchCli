/**********************************************************************************************
* Copyright (c) 2023 Marvell.
* All rights reserved.
* Use of this source code is governed by a BSD3 license that
* can be found in the LICENSE file and also at https://opensource.org/licenses/BSD-3-Clause
**********************************************************************************************/


/********************************************************************************
* msdRMU.c
*
* DESCRIPTION:
*       API definitions for Remote Management Unit
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*******************************************************************************/

#include <api/msdRMU.h>
#include <driver/msdApiTypes.h>
#include <utils/msdUtils.h>

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
)
{
	MSD_STATUS retVal;
	MSD_QD_DEV* dev = sohoDevGet(devNum);
	if (NULL == dev)
	{
		MSD_DBG_ERROR(("Dev is NULL for devNum %d.\n", devNum));
		retVal = MSD_FAIL;
    }
	else
	{
		if (dev->SwitchDevObj.RMUObj.grmuGetID != NULL)
		{
			retVal = dev->SwitchDevObj.RMUObj.grmuGetID(dev, id);
		}
		else
		{
			retVal = MSD_NOT_SUPPORTED;
		}
	}

    return retVal;    
}

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
)
{
	MSD_STATUS retVal;
    MSD_QD_DEV* dev = sohoDevGet(devNum);
	if (NULL == dev)
	{
		MSD_DBG_ERROR(("Dev is NULL for devNum %d.\n", devNum));
		retVal = MSD_FAIL;
    }
	else
	{
		if (dev->SwitchDevObj.RMUObj.grmuAtuDump != NULL)
		{
			retVal = dev->SwitchDevObj.RMUObj.grmuAtuDump(dev, startAddr, numOfEntry, atuEntry);
		}
		else
		{
			retVal = MSD_NOT_SUPPORTED;
		}
	}

    return retVal;    
}

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
	OUT MSD_ECID_ENTRY **ecidEntry
)
{
	MSD_STATUS retVal;
    MSD_QD_DEV* dev = sohoDevGet(devNum);
	if (NULL == dev)
	{
		MSD_DBG_ERROR(("Dev is NULL for devNum %d.\n", devNum));
		retVal = MSD_FAIL;
    }
	else
	{
		if (dev->SwitchDevObj.RMUObj.grmuEcidDump != NULL)
		{
			retVal = dev->SwitchDevObj.RMUObj.grmuEcidDump(dev, startAddr, numOfEntry, ecidEntry);
		}
		else
		{
			retVal = MSD_NOT_SUPPORTED;
		}
	}

    return retVal;    
}

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
)
{
	MSD_STATUS retVal;
    MSD_QD_DEV* dev = sohoDevGet(devNum);
	if (NULL == dev)
	{
		MSD_DBG_ERROR(("Dev is NULL for devNum %d.\n", devNum));
		retVal = MSD_FAIL;
    }
	else
	{
		if (dev->SwitchDevObj.RMUObj.grmuMibDump != NULL)
		{
			retVal = dev->SwitchDevObj.RMUObj.grmuMibDump(dev, enClearOnRead, portNum, statsCounterSet);
		}
		else
		{
			retVal = MSD_NOT_SUPPORTED;
		}
	}

    return retVal;    
}

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
)
{
	MSD_STATUS retVal;
    MSD_QD_DEV* dev = sohoDevGet(devNum);
	if (NULL == dev)
	{
		MSD_DBG_ERROR(("Dev is NULL for devNum %d.\n", devNum));
		retVal = MSD_FAIL;
    }
	else
	{
		if (dev->SwitchDevObj.RMUObj.grmuMib2Dump != NULL)
		{
			retVal = dev->SwitchDevObj.RMUObj.grmuMib2Dump(dev, enClearOnRead, portNum, statsCounterSet);
		}
		else
		{
			retVal = MSD_NOT_SUPPORTED;
		}
	}

    return retVal;    
}

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
)
{
	MSD_STATUS retVal;
    MSD_QD_DEV* dev = sohoDevGet(devNum);
	if (NULL == dev)
	{
		MSD_DBG_ERROR(("Dev is NULL for devNum %d.\n", devNum));
		retVal = MSD_FAIL;
    }
	else
	{
		if (dev->SwitchDevObj.RMUObj.grmuMultiRegAccess != NULL)
		{
			retVal = dev->SwitchDevObj.RMUObj.grmuMultiRegAccess(dev, regCmdList, nCmd);
		}
		else
		{
			retVal = MSD_NOT_SUPPORTED;
		}
	}

    return retVal;    
}

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
)
{
	MSD_STATUS retVal;
    MSD_QD_DEV* dev = sohoDevGet(devNum);
	if (NULL == dev)
	{
		MSD_DBG_ERROR(("Dev is NULL for devNum %d.\n", devNum));
		retVal = MSD_FAIL;
    }
	else
	{
		if (dev->SwitchDevObj.RMUObj.grmuTCAMDump != NULL)
		{
			retVal = dev->SwitchDevObj.RMUObj.grmuTCAMDump(dev, startAddr, numOfEntry, tcamEntry);
		}
		else
		{
			retVal = MSD_NOT_SUPPORTED;
		}
	}

    return retVal;    
}

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
)
{
	MSD_STATUS retVal;
    MSD_QD_DEV* dev = sohoDevGet(devNum);
	if (NULL == dev)
	{
		MSD_DBG_ERROR(("Dev is NULL for devNum %d.\n", devNum));
		retVal = MSD_FAIL;
    }
	else
	{
		if (dev->SwitchDevObj.RMUObj.grmuTCAMWrite != NULL)
		{
			retVal = dev->SwitchDevObj.RMUObj.grmuTCAMWrite(dev, tcamPointer, tcamEntry);
		}
		else
		{
			retVal = MSD_NOT_SUPPORTED;
		}
	}

    return retVal;    
}

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
)
{
	MSD_STATUS retVal;
    MSD_QD_DEV* dev = sohoDevGet(devNum);
	if (NULL == dev)
	{
		MSD_DBG_ERROR(("Dev is NULL for devNum %d.\n", devNum));
		retVal = MSD_FAIL;
    }
	else
	{
		if (dev->SwitchDevObj.RMUObj.grmuTCAMRead != NULL)
		{
			retVal = dev->SwitchDevObj.RMUObj.grmuTCAMRead(dev, tcamPointer, tcamEntry);
		}
		else
		{
			retVal = MSD_NOT_SUPPORTED;
		}
	}

    return retVal;    
}

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
)
{
    MSD_STATUS retVal;
    MSD_QD_DEV* dev = sohoDevGet(devNum);
    if (NULL == dev)
    {
        MSD_DBG_ERROR(("Dev is NULL for devNum %d.\n", devNum));
        retVal = MSD_FAIL;
    }
	else
	{
		if (dev->SwitchDevObj.RMUObj.grmuRegDump != NULL)
		{
			retVal = dev->SwitchDevObj.RMUObj.grmuRegDump(dev, devAddr, dataList);
		}
		else
		{
			retVal = MSD_NOT_SUPPORTED;
		}
	}

    return retVal;    
}

MSD_STATUS msdRMUFwVersionGet
(
    IN  MSD_U8  devNum,
	OUT MSD_U16 *major,
	OUT MSD_U16 *minor,
	OUT MSD_U16 *build,
	OUT char *version
)
{
	MSD_STATUS retVal;
	MSD_QD_DEV* dev = sohoDevGet(devNum);
	if (NULL == dev)
	{
		MSD_DBG_ERROR(("Dev is NULL for devNum %d.\n", devNum));
		retVal = MSD_FAIL;
    }
	else
	{
		if (dev->SwitchDevObj.RMUObj.grmuFwVersionGet != NULL)
		{
			retVal = dev->SwitchDevObj.RMUObj.grmuFwVersionGet(dev, major, minor, build, version);
		}
		else
		{
			retVal = MSD_NOT_SUPPORTED;
		}
	}

    return retVal;    
}
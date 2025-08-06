/**********************************************************************************************
* Copyright (c) 2023 Marvell.
* All rights reserved.
* Use of this source code is governed by a BSD3 license that
* can be found in the LICENSE file and also at https://opensource.org/licenses/BSD-3-Clause
**********************************************************************************************/


/*******************************************************************************
* Oak_msdRMU.c
*
* DESCRIPTION:
*       API definitions for control of Remote Management Unit
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*******************************************************************************/

#include <oak/include/api/Oak_msdRMU.h>
#include <oak/include/api/Oak_msdApiInternal.h>
#include <platform/msdSem.h>
#include <string.h>

static MSD_U32 dump_atuPacket
(
    IN      MSD_U8 delta,
	IN		MSD_U8 *packet,
	IN		MSD_U32 pkt_size,
	OUT	    MSD_U32 *numOfEntry,
	OUT		OAK_MSD_ATU_ENTRY **atuEntry
);

static MSD_U32 dump_ecidPacket
(
    IN      MSD_U8 delta,
	IN		MSD_U8 *packet,
	IN		MSD_U32 pkt_size,
	OUT	    MSD_U32 *numOfEntry,
	OUT		OAK_MSD_ECID_ENTRY **ecidEntry
);

static MSD_STATUS dump_mib2Packet
(
    IN      MSD_U8 delta,
	IN	MSD_U8 *packet,
	IN	MSD_U32 pkt_size,
	OUT	OAK_MSD_STATS_COUNTER_SET *statsCounterSet
);

static MSD_U32 dump_TCAMPacket
(
    IN      MSD_U8 delta,
	IN		MSD_U8 *packet,
	IN		MSD_U32 pkt_size,
	OUT	    MSD_U32 *numOfEntry,
	OUT		OAK_MSD_TCAM_DATA **tcamEntry
);

static MSD_STATUS dump_MultiRWPacket
(
    IN      MSD_U8 delta,
	IN		MSD_U8 *packet,
	IN		MSD_U32 pkt_size,
	INOUT	MSD_RegCmd **regCmdList
);

static MSD_STATUS msdRmuTcamPack
(
	IN	MSD_U8	opCode,
	IN  MSD_U32	tcamPointer,
	IN  OAK_MSD_TCAM_DATA *entry,
	OUT	MSD_RMU_TCAM_RW_ENTRY	*data
);

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
)
{
	MSD_Packet ReqPkt;
	/*Packet RspPkt;*/
	MSD_U8 reqEthPacket[512];
	MSD_U8 rspEthPacket[512];
	MSD_U32 req_pktlen, rsp_pktlen;
	MSD_STATUS retVal = 0;
    MSD_U8 delta;

	MSD_RMU_CMD cmd = MSD_GetID;

	MSD_U8 *rspEthPacketPtr = &(rspEthPacket[0]);

	MSD_DBG_INFO(("Oak_msdRmuGetID Called.\n"));

	if (NULL == id)
	{
		MSD_DBG_ERROR(("input id is NULL.\n"));
		return MSD_BAD_PARAM;
	}

	if (dev->rmuMode == MSD_RMU_ETHERT_TYPE_DSA_MODE)
	{
		delta = 0;
	}
	else if (dev->rmuMode == MSD_RMU_DSA_MODE)
	{
		delta = (MSD_U8)4;
	}
	else
	{
		delta = 0;
	}
		
	/*Request Packet*/
	retVal = msdRmuReqPktCreate(dev, cmd, &ReqPkt);
	if (retVal != MSD_OK) 
	{
		MSD_DBG_ERROR(("msdRmuReqPktCreate returned: %s.\n", msdDisplayStatus(retVal)));
		return retVal;
	}

	msdMemSet(reqEthPacket, 0, sizeof(reqEthPacket));

	retVal = msdRmuPackEthReqPkt(&ReqPkt, cmd, reqEthPacket);
	if (retVal != MSD_OK) 
	{
		MSD_DBG_ERROR(("msdRmuPackEthReqPkt returned: %s.\n", msdDisplayStatus(retVal)));
		return retVal;
	}

	req_pktlen = (MSD_U32)((MSD_RMU_PACKET_PREFIX_SIZE - (MSD_U32)delta) + 2U);

	/*dump_packet(&reqEthPacket, req_pktlen, "Read Register Request Packet");*/
	retVal = msdRmuTxRxPkt(dev, reqEthPacket, req_pktlen, &rspEthPacketPtr, &rsp_pktlen);
	if ((retVal != MSD_OK) || (rsp_pktlen == 0U))
	{
		MSD_DBG_ERROR(("rmu_tx_rx returned: %s with rsp_pktLen %d.\n", msdDisplayStatus(retVal), (MSD_32)rsp_pktlen));
		return MSD_FAIL;
	}

	/* Receive Packet */
	if (rsp_pktlen >= req_pktlen) 
	{
		/*dump_packet(&rspEthPacket, req_pktlen,
		//	"Read Register Response Packet");
		//msdRmuParseEthPacket(&rspEthPacket, RspPkt);*/
	}
	else 
	{
		MSD_DBG_ERROR(("response_pktlen [%d] < request_pktlen [%d]\n", rsp_pktlen, req_pktlen));
		return MSD_FAIL;
	}

	*id = ((*(rspEthPacketPtr + 24U - delta) & (MSD_U16)0xff) << 8) | (*(rspEthPacketPtr + 25U - delta) & (MSD_U16)0xff);

	MSD_DBG_INFO(("Oak_msdRmuGetID Exit.\n"));
	return MSD_OK;
}

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
)
{
	MSD_Packet ReqPkt;
	MSD_U8 reqEthPacket[512];
	MSD_U8 rspEthPacket[512];
	MSD_U32 req_pktlen, rsp_pktlen;
	MSD_U32 next_cont_code = 0;
	MSD_STATUS retVal = 0;
    MSD_U8 delta;

	MSD_RMU_CMD cmd = MSD_DumpATU;

	MSD_U8 *rspEthPacketPtr = &(rspEthPacket[0]);

	MSD_DBG_INFO(("Oak_msdRmuAtuDump Called.\n"));

	if ((startAddr == NULL) || (numOfEntry == NULL))
	{
		MSD_DBG_ERROR(("startAddr: %d, numOfEntry: %d.\n", startAddr, numOfEntry));
		return MSD_BAD_PARAM;
	}
	
	if (dev->rmuMode == MSD_RMU_ETHERT_TYPE_DSA_MODE)
	{
		delta = 0;
	}
	else if (dev->rmuMode == MSD_RMU_DSA_MODE)
	{
		delta = (MSD_U8)4;
	}
	else
	{
		delta = 0;
	}

	/*Request Packet*/
	retVal = msdRmuReqPktCreate(dev, cmd, &ReqPkt);
	if (retVal != MSD_OK) {
		MSD_DBG_ERROR(("msdRmuReqPktCreate returned: %s.\n", msdDisplayStatus(retVal)));
		return retVal;
	}

	ReqPkt.reqData._reqData = (MSD_U16)*startAddr;

	msdMemSet(reqEthPacket, 0, sizeof(reqEthPacket));
	retVal = msdRmuPackEthReqPkt(&ReqPkt, cmd, reqEthPacket);
	if (retVal != MSD_OK) 
	{
		MSD_DBG_ERROR(("msdRmuPackEthReqPkt returned: %s.\n", msdDisplayStatus(retVal)));
		return retVal;
	}

	req_pktlen = (MSD_U32)((MSD_RMU_PACKET_PREFIX_SIZE - (MSD_U32)delta) + 2U);

	/*dump_packet(&reqEthPacket, req_pktlen, "Read Register Request Packet");*/
	retVal = msdRmuTxRxPkt(dev, reqEthPacket, req_pktlen, &rspEthPacketPtr, &rsp_pktlen);
	if ((retVal != MSD_OK) || (rsp_pktlen == 0U))
	{
		MSD_DBG_ERROR(("msdRmu_tx_rx returned: %s with rsp_pktLen %d.\n", msdDisplayStatus(retVal), (MSD_32)rsp_pktlen));
		return MSD_FAIL;
	}

	if (rsp_pktlen < req_pktlen) 
	{
		MSD_DBG_ERROR(("response_pktlen [%d] < request_pktlen [%d]\n", rsp_pktlen, req_pktlen));
		return MSD_FAIL;
	}

	/* Receive Packet*/
	next_cont_code = dump_atuPacket(delta, rspEthPacketPtr, rsp_pktlen, numOfEntry, atuEntry);

	*startAddr = next_cont_code;

	MSD_DBG_INFO(("Oak_msdRmuAtuDump Exit.\n"));

	return MSD_OK;
}

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
)
{
	MSD_Packet ReqPkt;
	MSD_U8 reqEthPacket[512];
	MSD_U8 rspEthPacket[512];
	MSD_U32 req_pktlen, rsp_pktlen;
	MSD_U32 next_cont_code = 0x2000U;
	MSD_STATUS retVal = 0;
    MSD_U8 delta;

	MSD_RMU_CMD cmd = MSD_DumpATU;

	MSD_U8 *rspEthPacketPtr = &(rspEthPacket[0]);

	MSD_DBG_INFO(("Oak_msdRmuEcidDump Called.\n"));

	if ((startAddr == NULL) || (numOfEntry == NULL))
	{
		MSD_DBG_ERROR(("startAddr: %d, numOfEntry: %d.\n", startAddr, numOfEntry));
		return MSD_BAD_PARAM;
	}

	if (dev->rmuMode == MSD_RMU_ETHERT_TYPE_DSA_MODE)
	{
		delta = 0;
	}
	else if (dev->rmuMode == MSD_RMU_DSA_MODE)
	{
		delta = (MSD_U8)4;
	}
	else
	{
		delta = 0;
	}

	/*Request Packet*/
	retVal = msdRmuReqPktCreate(dev, cmd, &ReqPkt);
	if (retVal != MSD_OK) 
	{
		MSD_DBG_ERROR(("msdRmuReqPktCreate returned: %s.\n", msdDisplayStatus(retVal)));
		return retVal;
	}

	ReqPkt.reqData._reqData = (MSD_U16)*startAddr;

	msdMemSet(reqEthPacket, 0, sizeof(reqEthPacket));
	retVal = msdRmuPackEthReqPkt(&ReqPkt, cmd, reqEthPacket);
	if (retVal != MSD_OK) 
	{
		MSD_DBG_ERROR(("msdRmuPackEthReqPkt returned: %s.\n", msdDisplayStatus(retVal)));
		return retVal;
	}

	req_pktlen = (MSD_U32)((MSD_RMU_PACKET_PREFIX_SIZE - (MSD_U32)delta) + 2U);

	/*dump_packet(&reqEthPacket, req_pktlen, "Read Register Request Packet");*/
	retVal = msdRmuTxRxPkt(dev, reqEthPacket, req_pktlen, &rspEthPacketPtr, &rsp_pktlen);
	if ((retVal != MSD_OK) || (rsp_pktlen == 0U))
	{
		MSD_DBG_ERROR(("msdRmu_tx_rx returned: %s with rsp_pktLen %d.\n", msdDisplayStatus(retVal), (MSD_32)rsp_pktlen));
		return MSD_FAIL;
	}

	if (rsp_pktlen < req_pktlen) 
	{
		MSD_DBG_ERROR(("response_pktlen [%d] < request_pktlen [%d]\n", rsp_pktlen, req_pktlen));
		return MSD_FAIL;
	}

	/* Receive Packet*/
	next_cont_code = dump_ecidPacket(delta, rspEthPacketPtr, rsp_pktlen, numOfEntry, ecidEntry);

	*startAddr = next_cont_code;

	MSD_DBG_INFO(("Oak_msdRmuEcidDump Exit.\n"));

	return MSD_OK;
}

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
)
{
	MSD_Packet ReqPkt;
	MSD_U8 reqEthPacket[512];
	MSD_U8 rspEthPacket[512];
	MSD_U32 req_pktlen, rsp_pktlen;
	MSD_STATUS retVal;
    MSD_U8 delta;

	MSD_RMU_CMD cmd = (enClearOnRead == MSD_TRUE ? MSD_DumpMIB2Clear : MSD_DumpMIB2);

	MSD_U8 *rspEthPacketPtr = &(rspEthPacket[0]);

	MSD_DBG_INFO(("Oak_msdRmuMib2Dump Called.\n"));

	if (NULL == statsCounterSet)
	{
		MSD_DBG_ERROR(("statsCounterSet is NULL.\n"));
		return MSD_BAD_PARAM;
	}

	if (dev->rmuMode == MSD_RMU_ETHERT_TYPE_DSA_MODE)
	{
		delta = 0;
	}
	else if (dev->rmuMode == MSD_RMU_DSA_MODE)
	{
		delta = (MSD_U8)4;
	}
	else
	{
		delta = 0;
	}

	/*Request Packet*/
	retVal = msdRmuReqPktCreate(dev, cmd, &ReqPkt);
	if (retVal != MSD_OK) {
		MSD_DBG_ERROR(("msdRmuReqPktCreate returned: %s.\n", msdDisplayStatus(retVal)));
		return retVal;
	}

	ReqPkt.reqData._reqData = (MSD_U16)((enClearOnRead == MSD_TRUE ? 0x8000U : 0x0U) | (MSD_U16)port);

	msdMemSet(reqEthPacket, 0, sizeof(reqEthPacket));
	retVal = msdRmuPackEthReqPkt(&ReqPkt, cmd, reqEthPacket);
	if (retVal != MSD_OK) 
	{
		MSD_DBG_ERROR(("msdRmuPackEthReqPkt returned: %s.\n", msdDisplayStatus(retVal)));
		return retVal;
	}

	req_pktlen = (MSD_U32)((MSD_RMU_PACKET_PREFIX_SIZE - (MSD_U32)delta) + 2U);

	/*dump_packet(&reqEthPacket, req_pktlen, "Read Register Request Packet");*/
	retVal = msdRmuTxRxPkt(dev, reqEthPacket, req_pktlen, &rspEthPacketPtr, &rsp_pktlen);
	if ((retVal != MSD_OK) || (rsp_pktlen == 0U))
	{
		MSD_DBG_ERROR(("msdRmu_tx_rx returned: %s with rsp_pktLen %d.\n", msdDisplayStatus(retVal), (MSD_32)rsp_pktlen));
		return MSD_FAIL;
	}

	/* Receive Packet*/
	retVal = dump_mib2Packet(delta, rspEthPacketPtr, rsp_pktlen, statsCounterSet);

	MSD_DBG_INFO(("Oak_msdRmuMib2Dump Exit.\n"));

	return retVal;
}


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
)
{
	MSD_STATUS retVal;

	MSD_DBG_INFO(("Oak_msdRmuReadRegister Called.\n"));

	if (NULL == data)
	{
		MSD_DBG_ERROR(("data is NULL.\n"));
		retVal = MSD_BAD_PARAM;
	}
	else
	{
		retVal = msdGetAnyReg(dev->devNum, devAddr, regAddr, data);
		if (retVal != MSD_OK)
		{
			MSD_DBG_ERROR(("msdGetAnyReg returned: %s.\n", msdDisplayStatus(retVal)));
		}
	}

	MSD_DBG_INFO(("Oak_msdRmuReadRegister Exit.\n"));

	return retVal;
}

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
)
{
	MSD_STATUS retVal;

	MSD_DBG_INFO(("Oak_msdRmuWriteRegister Called.\n"));

	retVal = msdSetAnyReg(dev->devNum, devAddr, regAddr, data);
	if (retVal != MSD_OK)
	{
		MSD_DBG_ERROR(("msdSetAnyReg returned: %s.\n", msdDisplayStatus(retVal)));
	}

	MSD_DBG_INFO(("Oak_msdRmuWriteRegister Exit.\n"));

	return retVal;
}

/*******************************************************************************
* msdRmuMultiRegAccess
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
)
{
	MSD_Packet ReqPkt;
	MSD_U8 reqEthPacket[512];
	MSD_U8 rspEthPacket[512];
	MSD_U32 req_pktlen, rsp_pktlen;
	MSD_STATUS retVal = 0;
	MSD_U32	i = 0;
    MSD_U8 delta;

	MSD_RMU_CMD cmd = MSD_RegRW;

	MSD_U8 *rspEthPacketPtr = &(rspEthPacket[0]);

	MSD_DBG_INFO(("Oak_msdRmuMultiRegAccess Called.\n"));

	if (nCmd > MSD_RMU_MAX_REGCMDS)
	{
		MSD_DBG_ERROR(("nCmd %u exceeds the maximum supported CMD number %d\n.", nCmd, MSD_RMU_MAX_REGCMDS));
		return MSD_FAIL;
	}

	if ((nCmd != 0U) && (NULL == regCmdList))
	{
		MSD_DBG_ERROR(("regCmdList is NULL with nCmd: %d.\n", nCmd));
		return MSD_FAIL;
	}

	if (dev->rmuMode == MSD_RMU_ETHERT_TYPE_DSA_MODE)
	{
		delta = 0;
	}
	else if (dev->rmuMode == MSD_RMU_DSA_MODE)
	{
		delta = (MSD_U8)4;
	}
	else
	{
		delta = 0;
	}

	/*Request Packet*/
	retVal = msdRmuReqPktCreate(dev, cmd, &ReqPkt);
	if (retVal != MSD_OK) 
	{
		MSD_DBG_ERROR(("msdRmuReqPktCreate returned: %s.\n", msdDisplayStatus(retVal)));
		return retVal;
	}

	ReqPkt.reqData._regRWData.nCmd = nCmd;
	for (i = 0; i < nCmd; i++)
	{
		ReqPkt.reqData._regRWData.regCmd[i].isWaitOnBit = (*regCmdList)[i].isWaitOnBit;
		ReqPkt.reqData._regRWData.regCmd[i].opCode = (*regCmdList)[i].opCode;
		ReqPkt.reqData._regRWData.regCmd[i].devAddr = (*regCmdList)[i].devAddr;
		ReqPkt.reqData._regRWData.regCmd[i].regAddr = (*regCmdList)[i].regAddr;
		ReqPkt.reqData._regRWData.regCmd[i].data = (*regCmdList)[i].data;
	}

	msdMemSet(reqEthPacket, 0, sizeof(reqEthPacket));
	retVal = msdRmuPackEthReqPkt(&ReqPkt, cmd, reqEthPacket);
	if (retVal != MSD_OK) 
	{
		MSD_DBG_ERROR(("msdRmuPackEthReqPkt returned: %s.\n", msdDisplayStatus(retVal)));
		return retVal;
	}

	req_pktlen = ((MSD_U32)MSD_RMU_PACKET_PREFIX_SIZE - (MSD_U32)delta) + ((nCmd + 1U) * MSD_RMU_REGCMD_WORD_SIZE);

	/* dump_packet(&reqEthPacket, req_pktlen, "Read Register Request Packet"); */
	retVal = msdRmuTxRxPkt(dev, reqEthPacket, req_pktlen, &rspEthPacketPtr, &rsp_pktlen);
	if ((retVal != MSD_OK) || (rsp_pktlen == 0U))
	{
		MSD_DBG_ERROR(("msdRmu_tx_rx returned: %s with rsp_pktLen %d.\n", msdDisplayStatus(retVal), (MSD_32)rsp_pktlen));
		return MSD_FAIL;
	}

	if (rsp_pktlen < req_pktlen)
	{
		MSD_DBG_ERROR(("response_pktlen [%d] < request_pktlen [%d]\n", rsp_pktlen, req_pktlen));
		return MSD_FAIL;
	}

	/* Receive Packet */
	retVal = dump_MultiRWPacket(delta, rspEthPacketPtr, rsp_pktlen, regCmdList);
	if (retVal != MSD_OK)
	{
		MSD_DBG_ERROR(("dump_MultiRWPacket returned: %s.\n", msdDisplayStatus(retVal)));
		return MSD_FAIL;
	}

	MSD_DBG_INFO(("Oak_msdRmuMultiRegAccess Exit.\n"));
	return retVal;
}

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
)
{
	MSD_Packet ReqPkt;
	MSD_U8 reqEthPacket[512];
	MSD_U8 rspEthPacket[512];
	MSD_U32 req_pktlen, rsp_pktlen;
	MSD_STATUS retVal = 0;
    MSD_U8 delta;

	MSD_RMU_CMD cmd = MSD_DumpTCAM;

	MSD_U8 *rspEthPacketPtr = &(rspEthPacket[0]);

	MSD_DBG_INFO(("Oak_msdRmuTCAMDump Called.\n"));

	if ((startAddr == NULL) || (numOfEntry == NULL))
	{
		MSD_DBG_ERROR(("startAddr: %d, numOfEntry: %d.\n", startAddr, numOfEntry));
		return MSD_BAD_PARAM;
	}

	if (dev->rmuMode == MSD_RMU_ETHERT_TYPE_DSA_MODE)
	{
		delta = 0;
	}
	else if (dev->rmuMode == MSD_RMU_DSA_MODE)
	{
		delta = (MSD_U8)4;
	}
	else
	{
		delta = 0;
	}

	/*Request Packet*/
	retVal = msdRmuReqPktCreate(dev, cmd, &ReqPkt);
	if (retVal != MSD_OK) 
	{
		MSD_DBG_ERROR(("msdRmuReqPktCreate returned: %s.\n", msdDisplayStatus(retVal)));
		return retVal;
	}

	ReqPkt.reqData._reqData = (MSD_U16)*startAddr;

	msdMemSet(reqEthPacket, 0, sizeof(reqEthPacket));
	retVal = msdRmuPackEthReqPkt(&ReqPkt, cmd, reqEthPacket);
	if (retVal != MSD_OK) 
	{
		MSD_DBG_ERROR(("msdRmuPackEthReqPkt returned: %s.\n", msdDisplayStatus(retVal)));
		return retVal;
	}

	req_pktlen = (MSD_U32)((MSD_RMU_PACKET_PREFIX_SIZE - (MSD_U32)delta) + 2U);

	/*dump_packet(&reqEthPacket, req_pktlen, "Read Register Request Packet");*/
	retVal = msdRmuTxRxPkt(dev, reqEthPacket, req_pktlen, &rspEthPacketPtr, &rsp_pktlen);
	if ((retVal != MSD_OK) || (rsp_pktlen == 0U))
	{
		MSD_DBG_ERROR(("msdRmu_tx_rx returned: %s with rsp_pktLen %u.\n", msdDisplayStatus(retVal), rsp_pktlen));
		return MSD_FAIL;
	}

	if (rsp_pktlen < req_pktlen) 
	{
		MSD_DBG_ERROR(("response_pktlen [%d] < request_pktlen [%d]\n", rsp_pktlen, req_pktlen));
		return MSD_FAIL;
	}

	/* Receive Packet*/
	*startAddr = dump_TCAMPacket(delta, rspEthPacketPtr, rsp_pktlen, numOfEntry, tcamEntry);

	MSD_DBG_INFO(("Oak_msdRmuTCAMDump Exit.\n"));
	return MSD_OK;
}

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
)
{
	MSD_Packet ReqPkt;
	MSD_U32 nCmd = 1U;
	MSD_U8 reqEthPacket[512];
	MSD_U8 rspEthPacket[512];
	MSD_U32 req_pktlen, rsp_pktlen;
	MSD_RMU_TCAM_RW_ENTRY tcamRWData;
	MSD_STATUS retVal = 0;
    MSD_U8 delta;

	MSD_RMU_CMD cmd = MSD_TCAMRW;

	MSD_U8 *rspEthPacketPtr = &(rspEthPacket[0]);

	MSD_DBG_INFO(("Oak_msdRmuTCAMWrite Called.\n"));

	if ((tcamPointer >= OAK_MAX_IGR_TCAM_ENTRY) || (tcamEntry == NULL))
	{
		MSD_DBG_ERROR(("tcamPointer: %d, tcamEntry: %d.\n", tcamPointer, tcamEntry));
		return MSD_BAD_PARAM;
	}

	if (dev->rmuMode == MSD_RMU_ETHERT_TYPE_DSA_MODE)
	{
		delta = 0;
	}
	else if (dev->rmuMode == MSD_RMU_DSA_MODE)
	{
		delta = (MSD_U8)4;
	}
	else
	{
		delta = 0;
	}

	/*Request Packet*/
	retVal = msdRmuReqPktCreate(dev, cmd, &ReqPkt);
	if (retVal != MSD_OK) 
	{
		MSD_DBG_ERROR(("msdRmuReqPktCreate returned: %s.\n", msdDisplayStatus(retVal)));
		return retVal;
	}

	retVal = msdRmuTcamPack(MSD_RMU_TCAM_OPCODE_WRITE, tcamPointer, tcamEntry, &tcamRWData);
	if (retVal != MSD_OK) 
	{
		MSD_DBG_ERROR(("msdRmuTcamPack returned: %s.\n", msdDisplayStatus(retVal)));
		return retVal;
	}

	ReqPkt.reqData._tcamRWData.nEntry = 1;
	msdMemCpy((void*)&ReqPkt.reqData._tcamRWData.tcamEntry[0], &tcamRWData, sizeof(MSD_RMU_TCAM_RW_ENTRY));

	msdMemSet(reqEthPacket, 0, sizeof(reqEthPacket));
	retVal = msdRmuPackEthReqPkt(&ReqPkt, cmd, reqEthPacket);
	if (retVal != MSD_OK) 
	{
		MSD_DBG_ERROR(("msdRmuPackEthReqPkt returned: %s.\n", msdDisplayStatus(retVal)));
		return retVal;
	}

	req_pktlen = (MSD_U32)((MSD_U32)(MSD_RMU_PACKET_PREFIX_SIZE - (MSD_U32)delta) + (MSD_RMU_TCAM_ENTRY_OP_SIZE + 2U)); /**/

	/*dump_packet(&reqEthPacket, req_pktlen, "Read Register Request Packet");*/
	retVal = msdRmuTxRxPkt(dev, reqEthPacket, req_pktlen, &rspEthPacketPtr, &rsp_pktlen);
	if ((retVal != MSD_OK) || (rsp_pktlen == 0U))
	{
		MSD_DBG_ERROR(("rmu_tx_rx returned: %s with rsp_pktLen %u.\n", msdDisplayStatus(retVal), rsp_pktlen));
		return MSD_FAIL;
	}

	if (rsp_pktlen < req_pktlen)
	{
		MSD_DBG_ERROR(("response_pktlen [%d] < request_pktlen [%d]\n", rsp_pktlen, req_pktlen));
		return MSD_FAIL;
	}

	/* Receive Packet*/
	dump_TCAMPacket(delta, rspEthPacketPtr, rsp_pktlen, &nCmd, &tcamEntry);

	MSD_DBG_INFO(("Oak_msdRmuTCAMWrite Exit.\n"));
	return MSD_OK;
}

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
)
{
	MSD_Packet ReqPkt;
	MSD_U32 nCmd = 1U;
	MSD_U8 reqEthPacket[512];
	MSD_U8 rspEthPacket[512];
	MSD_U32 req_pktlen, rsp_pktlen;
	MSD_RMU_TCAM_RW_ENTRY tcamRWData;
	MSD_STATUS retVal = 0;
    MSD_U8 delta;

	MSD_RMU_CMD cmd = MSD_TCAMRW;

	MSD_U8 *rspEthPacketPtr = &(rspEthPacket[0]);

	MSD_DBG_INFO(("Oak_rmuTCAMRead Called.\n"));

	if ((tcamPointer >= OAK_MAX_IGR_TCAM_ENTRY) || (tcamEntry == NULL))
	{
		MSD_DBG_ERROR(("tcamPointer: %d, tcamEntry: %d.\n", tcamPointer, tcamEntry));
		return MSD_BAD_PARAM;
	}

	if (dev->rmuMode == MSD_RMU_ETHERT_TYPE_DSA_MODE)
	{
		delta = 0;
	}
	else if (dev->rmuMode == MSD_RMU_DSA_MODE)
	{
		delta = (MSD_U8)4;
	}
	else
	{
		delta = 0;
	}

	/*Request Packet*/
	retVal = msdRmuReqPktCreate(dev, cmd, &ReqPkt);
	if (retVal != MSD_OK)
	{
		MSD_DBG_ERROR(("msdRmuReqPktCreate returned: %s.\n", msdDisplayStatus(retVal)));
		return retVal;
	}
	
	retVal = msdRmuTcamPack(MSD_RMU_TCAM_OPCODE_READ, tcamPointer, tcamEntry, &tcamRWData);
	if (retVal != MSD_OK) 
	{
		MSD_DBG_ERROR(("msdRmuTcamPack returned: %s.\n", msdDisplayStatus(retVal)));
		return retVal;
	}

	ReqPkt.reqData._tcamRWData.nEntry = 1;
	msdMemCpy((void*)ReqPkt.reqData._tcamRWData.tcamEntry, &tcamRWData, sizeof(MSD_RMU_TCAM_RW_ENTRY));
	
	msdMemSet(reqEthPacket, 0, sizeof(reqEthPacket));
	retVal = msdRmuPackEthReqPkt(&ReqPkt, cmd, reqEthPacket);
	if (retVal != MSD_OK) 
	{
		MSD_DBG_ERROR(("msdRmuPackEthReqPkt returned: %s.\n", msdDisplayStatus(retVal)));
		return retVal;
	}

	req_pktlen = (MSD_U32)((MSD_U32)(MSD_RMU_PACKET_PREFIX_SIZE - (MSD_U32)delta) + (MSD_U32)(MSD_RMU_TCAM_ENTRY_OP_SIZE + 2U)); /**/

	/*dump_packet(&reqEthPacket, req_pktlen, "Read Register Request Packet");*/
	retVal = msdRmuTxRxPkt(dev, reqEthPacket, req_pktlen, &rspEthPacketPtr, &rsp_pktlen);
	if ((retVal != MSD_OK) || (rsp_pktlen == 0U))
	{
		MSD_DBG_ERROR(("rmu_tx_rx returned: %s with rsp_pktLen %d.\n", msdDisplayStatus(retVal), (MSD_32)rsp_pktlen));
		return MSD_FAIL;
	}

	if (rsp_pktlen < req_pktlen) 
	{
		MSD_DBG_ERROR(("response_pktlen [%d] < request_pktlen [%d]\n", rsp_pktlen, req_pktlen));
		return MSD_FAIL;
	}

	/* Receive Packet*/
	dump_TCAMPacket(delta, rspEthPacketPtr, rsp_pktlen, &nCmd, &tcamEntry);

	MSD_DBG_INFO(("Oak_msdRmuTCAMRead Exit.\n"));
	return MSD_OK;
}

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
)
{
    MSD_STATUS retVal;
    MSD_RegCmd regcmdList[32];
    MSD_RegCmd *pRegCmd = &(regcmdList[0]);
    MSD_U32 nCmd = 32U;
    MSD_U8 index;

    MSD_DBG_INFO(("Oak_msdRmuRegDump Called.\n"));

    if (NULL == dataList)
    {
        MSD_DBG_ERROR(("dataList is null.\n"));
        return MSD_BAD_PARAM;
    }

    for (index = 0; index < 32U; index++)
    {
		regcmdList[index].isWaitOnBit = MSD_RMU_WAIT_ON_BIT_FALSE;
		regcmdList[index].opCode = MSD_RMU_REQ_OPCODE_READ;
        regcmdList[index].devAddr = devAddr;
        regcmdList[index].regAddr = index;
        regcmdList[index].data = 0;
    }

	retVal = Oak_msdRmuMultiRegAccess(dev, &pRegCmd, nCmd);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR(("Peridot_msdRmuMultiRegAccess returned fail.\n"));
        return retVal;
    }

    for (index = 0; index < 32U; index++)
    {
        dataList->data[index] = regcmdList[index].data;
    }

    MSD_DBG_INFO(("Oak_msdRmuRegDump Exit.\n"));
    return MSD_OK;
}

MSD_STATUS Oak_msdRmuFwVersionGet
(
	IN  MSD_QD_DEV *dev,
	OUT MSD_U16 *major,
	OUT MSD_U16 *minor,
	OUT MSD_U16 *build,
	OUT char *version
)
{
	MSD_Packet ReqPkt;
	/*Packet RspPkt;*/
	MSD_U8 reqEthPacket[512];
	MSD_U8 rspEthPacket[512];
	MSD_U32 req_pktlen, rsp_pktlen;
	MSD_STATUS retVal = 0;
    MSD_U8 delta;

	MSD_RMU_CMD cmd = MSD_FWVersionGet;

	MSD_U8 *rspEthPacketPtr = &(rspEthPacket[0]);

	MSD_DBG_INFO(("Oak_msdRmuFwVersionGet Called.\n"));

	if (NULL == version)
	{
		MSD_DBG_ERROR(("input version is NULL.\n"));
		return MSD_BAD_PARAM;
	}

	if (dev->rmuMode == MSD_RMU_ETHERT_TYPE_DSA_MODE)
	{
		delta = 0;
	}
	else if (dev->rmuMode == MSD_RMU_DSA_MODE)
	{
		delta = (MSD_U8)4;
	}
	else
	{
		delta = 0;
	}
		
	/*Request Packet*/
	retVal = msdRmuReqPktCreate(dev, cmd, &ReqPkt);
	if (retVal != MSD_OK) 
	{
		MSD_DBG_ERROR(("msdRmuReqPktCreate returned: %s.\n", msdDisplayStatus(retVal)));
		return retVal;
	}

	msdMemSet(reqEthPacket, 0, sizeof(reqEthPacket));

	retVal = msdRmuPackEthReqPkt(&ReqPkt, cmd, reqEthPacket);
	if (retVal != MSD_OK) 
	{
		MSD_DBG_ERROR(("msdRmuPackEthReqPkt returned: %s.\n", msdDisplayStatus(retVal)));
		return retVal;
	}

	req_pktlen = (MSD_U32)((MSD_RMU_PACKET_PREFIX_SIZE - (MSD_U32)delta) + 2U);

	retVal = msdRmuTxRxPkt(dev, reqEthPacket, req_pktlen, &rspEthPacketPtr, &rsp_pktlen);
	if ((retVal != MSD_OK) || (rsp_pktlen == 0U))
	{
		MSD_DBG_ERROR(("rmu_tx_rx returned: %s with rsp_pktLen %d.\n", msdDisplayStatus(retVal), (MSD_32)rsp_pktlen));
		return MSD_FAIL;
	}

	/* Receive Packet */
	if (rsp_pktlen >= req_pktlen) 
	{
		/*dump_packet(&rspEthPacket, req_pktlen,
		//	"Read Register Response Packet");
		//msdRmuParseEthPacket(&rspEthPacket, RspPkt);*/
	}
	else 
	{
		MSD_DBG_ERROR(("response_pktlen [%d] < request_pktlen [%d]\n", rsp_pktlen, req_pktlen));
		return MSD_FAIL;
	}
	*major = (*(rspEthPacketPtr + 28U - delta) & (MSD_U16)0xff) << 8 | (*(rspEthPacketPtr + 29U - delta) & (MSD_U16)0xff);
	*minor = (*(rspEthPacketPtr + 30U - delta) & (MSD_U16)0xff) << 8 | (*(rspEthPacketPtr + 31U - delta) & (MSD_U16)0xff);
	*build = (*(rspEthPacketPtr + 32U - delta) & (MSD_U16)0xff) << 8 | (*(rspEthPacketPtr + 33U - delta) & (MSD_U16)0xff);
	memcpy(version, rspEthPacketPtr + 34U - delta, 256U);
	
	MSD_DBG_INFO(("Oak_msdRmuFwVersionGet Exit.\n"));
	return MSD_OK;
}

/****************************************************************************/
/* Internal functions.                                                  */
/****************************************************************************/
static MSD_U32 dump_atuPacket
(
    IN      MSD_U8 delta,
	IN		MSD_U8 *packet, 
	IN		MSD_U32 pkt_size, 
	OUT	    MSD_U32 *numOfEntry, 
	OUT		OAK_MSD_ATU_ENTRY **atuEntry
)
{
	MSD_U8 *ptr = packet;
	MSD_U32 n;
	MSD_U32 idx = 0;
	/*OAK_MSD_ATU_ENTRY atu;*/
	/*static MSD_U32 atu_slno = 0;*/
	MSD_U32 atu_cnt = 0;
	MSD_U32 cont_addr = 0;
	MSD_BOOL flag = MSD_TRUE;

	ptr += (MSD_RMU_PACKET_PREFIX_SIZE - delta);
	idx += (MSD_RMU_PACKET_PREFIX_SIZE - delta);


	while (flag == MSD_TRUE)
	{

		if ((idx + 10U) > pkt_size)
		{
			break;
		}

		((*atuEntry) + atu_cnt)->entryState = ((*ptr) >> 4) & 0xf;
		if (((*atuEntry) + atu_cnt)->entryState == 0)
		{
			break;
		}

		((*atuEntry) + atu_cnt)->LAG = ((*ptr) >> 3) & 0x01;

		((*atuEntry) + atu_cnt)->portVec = *(ptr + 1) | (((*ptr) & 0x7) << 8);
		ptr += 2;

		/*atu.macAddr = 0x0L;*/
		for (n = 0; n < 6U; n++) 
		{
			((*atuEntry) + atu_cnt)->macAddr.arEther[n] = (MSD_U8)(*ptr);/* << 8 * n; */
			ptr++;
		}

		/*Check the Is ecid entry bit*/
		if ((*ptr & 0x80U) != 0U)
		{
			break;
		}

		/*macQPri and macFPri only return back one based on one hidden register setting*/
		((*atuEntry) + atu_cnt)->exPrio.macQPri = ((*ptr) >> 4) & 0x7;

		((*atuEntry) + atu_cnt)->DBNum = (*(ptr + 1)) | (((*ptr) & 0xf) << 8);
		ptr += 2;

		idx += 10U;
		atu_cnt++;
	}

	if (atu_cnt == MSD_RMU_MAX_ATUS)
	{
		cont_addr = (*ptr << 8) | *(ptr + 1);
	}

	*numOfEntry = atu_cnt;

	return cont_addr;
}

static MSD_U32 dump_ecidPacket
(
    IN      MSD_U8 delta,
	IN		MSD_U8 *packet,
	IN		MSD_U32 pkt_size,
	OUT	    MSD_U32 *numOfEntry,
	OUT		OAK_MSD_ECID_ENTRY **ecidEntry
)
{
	MSD_U8 *ptr = packet;
	MSD_U32 idx = 0;

	MSD_U32 atu_cnt = 0;
	MSD_U32 cont_addr = 0;
	MSD_BOOL flag = MSD_TRUE;

	ptr += (MSD_RMU_PACKET_PREFIX_SIZE - delta);
	idx += (MSD_RMU_PACKET_PREFIX_SIZE - delta);

	while (flag == MSD_TRUE) 
	{

		if ((idx + 10U) > pkt_size)
		{
			break;
		}

		((*ecidEntry) + atu_cnt)->entryState = ((*ptr) >> 4) & 0xf;
		if (((*ecidEntry) + atu_cnt)->entryState == 0)
		{
			break;
		}

		((*ecidEntry) + atu_cnt)->lag = ((*ptr) >> 3) & 0x01;

		((*ecidEntry) + atu_cnt)->portVec = *(ptr + 1) | (((*ptr) & 0x7) << 8);
		ptr += 2;

		ptr += 1;
		((*ecidEntry) + atu_cnt)->lagId = (*ptr) & 0x1f;

		ptr += 3;
		((*ecidEntry) + atu_cnt)->group = ((*ptr) >> 4) & 0x3;
		((*ecidEntry) + atu_cnt)->ecid = *(ptr + 1) | (((*ptr) & 0xf) << 8);
		ptr += 2;

		/*Check the is ecid entry bit*/
		if ((*ptr & 0x80U) == 0U)
		{
			break;
		}

		/*macQPri and macFPri only return back one based on one hidden register setting*/
		((*ecidEntry) + atu_cnt)->macQPri = ((*ptr) >> 4) & 0x7;
		((*ecidEntry) + atu_cnt)->removeEtag = ((*ptr) >> 3) & 0x01;

		ptr += 2;

		idx += 10U;
		atu_cnt++;
	}

	if (atu_cnt == MSD_RMU_MAX_ATUS)
	{
		cont_addr = (*(ptr) << 8) | (*(ptr + 1));
	}

	*numOfEntry = atu_cnt;

	return cont_addr;
}

static MSD_STATUS dump_mib2Packet
(
IN  MSD_U8 delta,
IN	MSD_U8 *packet,
IN	MSD_U32 pkt_size,
OUT	OAK_MSD_STATS_COUNTER_SET *statsCounterSet
)
{
    MSD_U8 *ptr = packet;

    MSD_U32  temp = 0;

	if (pkt_size < (MSD_U32)((MSD_U32)290 - (MSD_U32)delta))
	{
        MSD_DBG_ERROR(("Error: Mib Packet Size [%d] < 290 bytes\n", (MSD_32)pkt_size));
        return MSD_FAIL;
    }

	ptr += (MSD_RMU_PACKET_PREFIX_SIZE - delta);

    /*dev_num = (*ptr) & 0x1f;  */
    ptr++;
    /*port_num = ((*ptr) >> 3) & 0xf; */
    ptr++;

    /*time stamp*/
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;

    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->InGoodOctetsLo = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->InGoodOctetsHi = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->InBadOctets = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->OutFCSErr = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->InUnicasts = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->Deferred = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->InBroadcasts = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->InMulticasts = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->Octets64 = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->Octets127 = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->Octets255 = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->Octets511 = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->Octets1023 = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->OctetsMax = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->OutOctetsLo = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->OutOctetsHi = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->OutUnicasts = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->Excessive = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->OutMulticasts = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->OutBroadcasts = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->Single = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->OutPause = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->InPause = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->Multiple = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->InUndersize = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->InFragments = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->InOversize = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->InJabber = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->InRxErr = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->InFCSErr = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->Collisions = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->Late = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->InDiscards = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->InFiltered = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->InAccepted = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->InBadAccepted = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->InGoodAvbClassA = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->InGoodAvbClassB = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->InBadAvbClassA = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->InBadAvbClassB = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->TCAMCounter0 = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->TCAMCounter1 = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->TCAMCounter2 = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->TCAMCounter3 = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->InDroppedAvbA = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->InDroppedAvbB = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->InDaUnknown = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->InMGMT = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->OutQueue0 = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->OutQueue1 = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->OutQueue2 = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->OutQueue3 = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->OutQueue4 = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->OutQueue5 = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->OutQueue6 = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->OutQueue7 = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->OutCutThrough = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->InBadQbv = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->OutOctetsA = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->OutOctetsB = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->OutYel = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->OutDroppedYel = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->OutDiscards = temp;
    temp = 0;
    temp |= ((*ptr) & 0xff) << 24; ptr++;
    temp |= ((*ptr) & 0xff) << 16; ptr++;
    temp |= ((*ptr) & 0xff) << 8; ptr++;
    temp |= ((*ptr) & 0xff) << 0; ptr++;
    statsCounterSet->OutMGMT = temp;

	/*
	temp = 0;
	temp |= ((*ptr) & 0xff) << 24; ptr++;
	temp |= ((*ptr) & 0xff) << 16; ptr++;
	temp |= ((*ptr) & 0xff) << 8; ptr++;
	temp |= ((*ptr) & 0xff) << 0; ptr++;
	*/
	statsCounterSet->DropEvents = 0x0;

    return MSD_OK;
}

static MSD_U32 dump_TCAMPacket
(
    IN      MSD_U8 delta,
	IN		MSD_U8 *packet,
	IN		MSD_U32 pkt_size,
	OUT	    MSD_U32 *numOfEntry,
	OUT		OAK_MSD_TCAM_DATA **tcamEntry
)
{
	MSD_U8 *ptr = packet;
	MSD_U32 idx = 0;
	MSD_U16 tcam_temp = 0;
	MSD_U32 cont_addr = 0;
	MSD_U32 tcam_cnt = 0;
	MSD_BOOL isEnd = MSD_FALSE;
	MSD_BOOL flag = MSD_TRUE;

	ptr += (MSD_RMU_PACKET_PREFIX_SIZE - delta);

	while (flag == MSD_TRUE) 
	{

		if ((idx + MSD_RMU_TCAM_ENTRY_SIZE) > pkt_size)
		{
			break;
		}

		/*tcam_op/Entry*/
		tcam_temp = 0;
		tcam_temp |= ((*ptr) & 0xff) << 8; 
		ptr++;

		tcam_temp |= ((*ptr) & 0xff) << 0; 
		ptr++;

		ptr += 10; /* rsvd 8 byte? */

		/*if (idx == 0)*/
		{
			tcam_temp = 0;
			tcam_temp |= ((*ptr) & 0xff) << 8; 
			ptr++;

			tcam_temp |= ((*ptr) & 0xff) << 0; 
			ptr++;

			if (tcam_temp == (MSD_U16)0x00FF)
			{
				/*break;*/
				isEnd = MSD_TRUE;
			}

			((*tcamEntry) + tcam_cnt)->frameTypeMask = MSD_BF_GET(tcam_temp, 14, 2);
			((*tcamEntry) + tcam_cnt)->frameType = MSD_BF_GET(tcam_temp, 6, 2);

			((*tcamEntry) + tcam_cnt)->timeKeyMask = MSD_BF_GET(tcam_temp, 13, 1);
			((*tcamEntry) + tcam_cnt)->timeKey = MSD_BF_GET(tcam_temp, 5, 1);
			
			((*tcamEntry) + tcam_cnt)->spvMask = ((MSD_BF_GET(tcam_temp, 8, 4)) & 0xF) << 8;
			((*tcamEntry) + tcam_cnt)->spv = ((MSD_BF_GET(tcam_temp, 0, 4)) & 0xF) << 8;

			tcam_temp = 0;
			tcam_temp |= ((*ptr) & 0xff) << 8; 
			ptr++;

			tcam_temp |= ((*ptr) & 0xff) << 0; 
			ptr++;

			((*tcamEntry) + tcam_cnt)->spvMask |= (MSD_BF_GET(tcam_temp, 8, 8)) & 0xff;
			((*tcamEntry) + tcam_cnt)->spv |= (MSD_BF_GET(tcam_temp, 0, 8)) & 0xff;
			
			tcam_temp = 0;
			tcam_temp |= ((*ptr) & 0xff) << 8; 
			ptr++;

			tcam_temp |= ((*ptr) & 0xff) << 0; 
			ptr++;

			((*tcamEntry) + tcam_cnt)->ppriMask = MSD_BF_GET(tcam_temp, 12, 4);
			((*tcamEntry) + tcam_cnt)->ppri = MSD_BF_GET(tcam_temp, 4, 4);
			((*tcamEntry) + tcam_cnt)->pvidMask = ((MSD_BF_GET(tcam_temp, 8, 4)) << 8) & 0xf00;
			((*tcamEntry) + tcam_cnt)->pvid = ((MSD_BF_GET(tcam_temp, 0, 4)) << 8) & 0xf00;
			
			tcam_temp = 0;
			tcam_temp |= ((*ptr) & 0xff) << 8; 
			ptr++;

			tcam_temp |= ((*ptr) & 0xff) << 0; 
			ptr++;

			((*tcamEntry) + tcam_cnt)->pvidMask |= (MSD_BF_GET(tcam_temp, 8, 8)) & 0xff;
			((*tcamEntry) + tcam_cnt)->pvid |= (MSD_BF_GET(tcam_temp, 0, 8)) & 0xff;
			
			/*page0-DA*/
			MSD_U16 i;
			/*Page 0 (0~21) bytes*/
			for (i = 0; i < 22U; i++)
			{
				((*tcamEntry) + tcam_cnt)->frameOctetMask[i] = 0;

				tcam_temp = 0;
				tcam_temp |= ((*ptr) & 0xff) << 8; 
				ptr++;

				tcam_temp |= ((*ptr) & 0xff) << 0; 
				ptr++;

				((*tcamEntry) + tcam_cnt)->frameOctetMask[i] = MSD_BF_GET(tcam_temp, 8, 8);
				((*tcamEntry) + tcam_cnt)->frameOctet[i] = MSD_BF_GET(tcam_temp, 0, 8);
			}

			/*Page 1 (22~47) bytes*/
			for (i = 0; i < 26U; i++)
			{
				((*tcamEntry) + tcam_cnt)->frameOctetMask[i + 22] = 0;

				tcam_temp = 0;
				tcam_temp |= ((*ptr) & 0xff) << 8; 
				ptr++;

				tcam_temp |= ((*ptr) & 0xff) << 0; 
				ptr++;

				((*tcamEntry) + tcam_cnt)->frameOctetMask[i + 22] = MSD_BF_GET(tcam_temp, 8, 8);
				((*tcamEntry) + tcam_cnt)->frameOctet[i + 22] = MSD_BF_GET(tcam_temp, 0, 8);
			}

			/*Action*/

			tcam_temp = 0;
			tcam_temp |= ((*ptr) & 0xff) << 8; 
			ptr++;

			tcam_temp |= ((*ptr) & 0xff) << 0; 
			ptr++;

			((*tcamEntry) + tcam_cnt)->continu = MSD_BF_GET(tcam_temp, 13, 3);
			((*tcamEntry) + tcam_cnt)->vidOverride = (MSD_BOOL)MSD_BF_GET(tcam_temp, 12, 1);
			((*tcamEntry) + tcam_cnt)->vidData = MSD_BF_GET(tcam_temp, 0, 12);
			
			tcam_temp = 0;
			tcam_temp |= ((*ptr) & 0xff) << 8; 
			ptr++;

			tcam_temp |= ((*ptr) & 0xff) << 0; 
			ptr++;

			((*tcamEntry) + tcam_cnt)->nextId = MSD_BF_GET(tcam_temp, 8, 8);
			((*tcamEntry) + tcam_cnt)->qpriOverride = (MSD_BOOL)MSD_BF_GET(tcam_temp, 7, 1);
			((*tcamEntry) + tcam_cnt)->qpriData = MSD_BF_GET(tcam_temp, 4, 3);
			((*tcamEntry) + tcam_cnt)->fpriOverride = (MSD_BOOL)MSD_BF_GET(tcam_temp, 3, 1);
			((*tcamEntry) + tcam_cnt)->fpriData = MSD_BF_GET(tcam_temp, 0, 3);
			
			tcam_temp = 0;
			tcam_temp |= ((*ptr) & 0xff) << 8; 
			ptr++;

			tcam_temp |= ((*ptr) & 0xff) << 0; 
			ptr++;

			((*tcamEntry) + tcam_cnt)->dpvSF = MSD_BF_GET(tcam_temp, 11, 1);
			((*tcamEntry) + tcam_cnt)->dpvData = MSD_BF_GET(tcam_temp, 0, 11);

			ptr += 2;


			tcam_temp = 0;
			tcam_temp |= ((*ptr) & 0xff) << 8; 
			ptr++;

			tcam_temp |= ((*ptr) & 0xff) << 0; 
			ptr++;

			((*tcamEntry) + tcam_cnt)->dpvMode = MSD_BF_GET(tcam_temp, 14, 2);
			((*tcamEntry) + tcam_cnt)->colorMode = MSD_BF_GET(tcam_temp, 12, 2);
			((*tcamEntry) + tcam_cnt)->vtuPageOverride = MSD_BF_GET(tcam_temp, 11, 1);
			((*tcamEntry) + tcam_cnt)->vtuPage = MSD_BF_GET(tcam_temp, 10, 1);
			((*tcamEntry) + tcam_cnt)->unKnownFilter = MSD_BF_GET(tcam_temp, 8, 2);
			((*tcamEntry) + tcam_cnt)->egActPoint = MSD_BF_GET(tcam_temp, 0, 6);
			
			tcam_temp = 0;
			tcam_temp |= ((*ptr) & 0xff) << 8; 
			ptr++;

			tcam_temp |= ((*ptr) & 0xff) << 0; 
			ptr++;

			((*tcamEntry) + tcam_cnt)->ldBalanceOverride = MSD_BF_GET(tcam_temp, 15, 1);
			((*tcamEntry) + tcam_cnt)->ldBalanceData = MSD_BF_GET(tcam_temp, 12, 3);
			((*tcamEntry) + tcam_cnt)->tcamTunnel = MSD_BF_GET(tcam_temp, 10, 1);
			((*tcamEntry) + tcam_cnt)->ipMulticast = MSD_BF_GET(tcam_temp, 9, 1);
			((*tcamEntry) + tcam_cnt)->ip2me = MSD_BF_GET(tcam_temp, 8, 1);
			((*tcamEntry) + tcam_cnt)->routeEntry = MSD_BF_GET(tcam_temp, 7, 1);
			((*tcamEntry) + tcam_cnt)->DSCPOverride = MSD_BF_GET(tcam_temp, 6, 1);
			((*tcamEntry) + tcam_cnt)->DSCP = MSD_BF_GET(tcam_temp, 0, 6);

			tcam_temp = 0;
			tcam_temp |= ((*ptr) & 0xff) << 8; ptr++;
			tcam_temp |= ((*ptr) & 0xff) << 0; ptr++;

			((*tcamEntry) + tcam_cnt)->factionOverride = (MSD_BOOL)MSD_BF_GET(tcam_temp, 15, 1);
			((*tcamEntry) + tcam_cnt)->factionData = MSD_BF_GET(tcam_temp, 0, 15);

			ptr += 12 * 2;

			/*Ingress action reg 0x1b*/
			tcam_temp = 0;
			tcam_temp |= ((*ptr) & 0xff) << 8; ptr++;
			tcam_temp |= ((*ptr) & 0xff) << 0; ptr++;
			((*tcamEntry) + tcam_cnt)->interrupt = MSD_BF_GET(tcam_temp, 15, 1);
			((*tcamEntry) + tcam_cnt)->IncTcamCtr = MSD_BF_GET(tcam_temp, 14, 1);
			((*tcamEntry) + tcam_cnt)->tcamCtr = MSD_BF_GET(tcam_temp, 12, 2);
		}

		/* *((*tcamEntry) + tcam_cnt) = tcam; */
		if (isEnd == MSD_TRUE)
		{
			break;
		}

		idx += MSD_RMU_TCAM_ENTRY_SIZE;
		tcam_cnt++;
	}

	if (tcam_cnt == MSD_RMU_MAX_TCAMS) 
	{
		cont_addr = (*(ptr) << 8) | (*(ptr + 1));
	}


	*numOfEntry = tcam_cnt;

	return cont_addr;
}

static MSD_STATUS msdRmuTcamPack
(
	IN	MSD_U8	opCode,
	IN  MSD_U32	tcamPointer,
	IN  OAK_MSD_TCAM_DATA *entry,
	OUT	MSD_RMU_TCAM_RW_ENTRY	*data
)
{
	MSD_U16 temp;
	MSD_U16 *pktPtr;
	MSD_32 i;

	pktPtr = data->dataArray;
	
	msdMemSet((void*)data, 0, sizeof(MSD_RMU_TCAM_RW_ENTRY));

	temp = (MSD_U16)((MSD_U16)0x8000 | (MSD_U16)((MSD_U16)opCode << 12) | (MSD_U16)tcamPointer);
	msdMemCpy(pktPtr, &temp, sizeof(temp));

	pktPtr += 1;

	pktPtr += 5; /*rsvd*/

	if (MSD_RMU_TCAM_OPCODE_WRITE == opCode)
	{
		/*key*/
		temp = 0;
		MSD_BF_SET(temp, entry->frameTypeMask, 14, 2);
		MSD_BF_SET(temp, entry->frameType, 6, 2);
		MSD_BF_SET(temp, entry->timeKeyMask, 13, 1);
		MSD_BF_SET(temp, entry->timeKey, 5, 1);
		MSD_BF_SET(temp, ((entry->spvMask & 0xF00) >> 8), 8, 4);
		MSD_BF_SET(temp, ((entry->spv & 0xF00) >> 8), 0, 4);
		msdMemCpy(pktPtr, &temp, sizeof(temp)); 
		pktPtr += 1;

		temp = 0;
		MSD_BF_SET(temp, (entry->spvMask & 0xff), 8, 8);
		MSD_BF_SET(temp, (entry->spv & 0xff), 0, 8);
		msdMemCpy(pktPtr, &temp, sizeof(temp)); 
		pktPtr += 1;

		temp = 0;
		MSD_BF_SET(temp, entry->ppriMask, 12, 4);
		MSD_BF_SET(temp, entry->ppri, 4, 4);
		MSD_BF_SET(temp, ((entry->pvidMask & 0xf00) >> 8), 8, 4);
		MSD_BF_SET(temp, ((entry->pvid & 0xf00) >> 8), 0, 4);
		msdMemCpy(pktPtr, &temp, sizeof(temp)); 
		pktPtr += 1;

		temp = 0;
		MSD_BF_SET(temp, (entry->pvidMask & 0xff), 8, 8);
		MSD_BF_SET(temp, (entry->pvid & 0xff), 0, 8);
		msdMemCpy(pktPtr, &temp, sizeof(temp)); 
		pktPtr += 1;

		/*Page 0 (0~21) bytes*/
		for (i = 4; i<26; i++)
		{
			temp = 0;
			MSD_BF_SET(temp, entry->frameOctetMask[i - 4], 8, 8);
			MSD_BF_SET(temp, entry->frameOctet[i - 4], 0, 8);
			msdMemCpy(pktPtr, &temp, sizeof(temp)); 
			pktPtr += 1;
		}

		/*Page 1 (22~47) bytes*/
		for (i = 0; i<26; i++)
		{
			temp = 0;
			MSD_BF_SET(temp, entry->frameOctetMask[22 + i], 8, 8);
			MSD_BF_SET(temp, entry->frameOctet[22 + i], 0, 8);
			msdMemCpy(pktPtr, &temp, sizeof(temp)); 
			pktPtr += 1;
		}

		/*Action*/
		temp = 0;
		MSD_BF_SET(temp, entry->continu, 13, 3);
		MSD_BF_SET(temp, (MSD_U16)(entry->vidOverride), 12, 1);
		MSD_BF_SET(temp, entry->vidData, 0, 12);
		msdMemCpy(pktPtr, &temp, sizeof(temp)); 
		pktPtr += 1;

		temp = 0;
		MSD_BF_SET(temp, entry->nextId, 8, 8);
		MSD_BF_SET(temp, (MSD_U16)(entry->qpriOverride), 7, 1);
		MSD_BF_SET(temp, entry->qpriData, 4, 3);
		MSD_BF_SET(temp, (MSD_U16)(entry->fpriOverride), 3, 1);
		MSD_BF_SET(temp, entry->fpriData, 0, 3);
		msdMemCpy(pktPtr, &temp, sizeof(temp)); 
		pktPtr += 1;

		temp = 0;
		MSD_BF_SET(temp, entry->dpvSF, 11, 1);
		MSD_BF_SET(temp, (MSD_U16)(entry->dpvData), 0, 11);
		msdMemCpy(pktPtr, &temp, sizeof(temp)); 
		pktPtr += 1;

		msdMemSet(pktPtr, 0, 2);
		pktPtr += 1;

		temp = 0;
		MSD_BF_SET(temp, entry->dpvMode, 14, 2);
		MSD_BF_SET(temp, entry->colorMode, 12, 2);
		MSD_BF_SET(temp, (MSD_U16)(entry->vtuPageOverride), 11, 1);
		MSD_BF_SET(temp, entry->vtuPage, 10, 1);
		MSD_BF_SET(temp, entry->unKnownFilter, 8, 2);
		MSD_BF_SET(temp, entry->egActPoint, 0, 6);
		msdMemCpy(pktPtr, &temp, sizeof(temp)); 
		pktPtr += 1;

		temp = 0;
		MSD_BF_SET(temp, (MSD_U16)(entry->ldBalanceOverride), 15, 1);
		MSD_BF_SET(temp, entry->ldBalanceData, 12, 3);
		MSD_BF_SET(temp, entry->tcamTunnel, 10, 1);
		MSD_BF_SET(temp, entry->ipMulticast, 9, 1);
		MSD_BF_SET(temp, entry->ip2me, 8, 1);
		MSD_BF_SET(temp, entry->routeEntry, 7, 1);
		MSD_BF_SET(temp, (MSD_U16)(entry->DSCPOverride), 6, 1);
		MSD_BF_SET(temp, entry->DSCP, 0, 6);
		msdMemCpy(pktPtr, &temp, sizeof(temp)); 
		pktPtr += 1;

		temp = 0;
		MSD_BF_SET(temp, (MSD_U16)(entry->factionOverride), 15, 1);
		MSD_BF_SET(temp, entry->factionData, 0, 15);
		msdMemCpy(pktPtr, &temp, sizeof(temp)); 
		pktPtr += 1;

		/*rsvd*/
		pktPtr += 12;
		temp = 0;
		MSD_BF_SET(temp, (entry->interrupt), 15, 1);
		MSD_BF_SET(temp, entry->IncTcamCtr, 14, 1);
		MSD_BF_SET(temp, entry->tcamCtr, 12, 2);
		msdMemCpy(pktPtr, &temp, sizeof(temp)); 
		pktPtr += 1;
	}
	return MSD_OK;
}

static MSD_STATUS dump_MultiRWPacket
(
    IN      MSD_U8 delta,
    IN		MSD_U8 *packet,
    IN		MSD_U32 pkt_size,
    INOUT	MSD_RegCmd **regCmdList
)
{
	MSD_U8 *ptr = packet;
	MSD_U32  i = 0, size = 0;
	MSD_U16 *ptrr;
	MSD_BOOL flag = MSD_TRUE;
	MSD_BOOL isFoundEnd = MSD_FALSE;

	ptr += (MSD_RMU_PACKET_PREFIX_SIZE - delta);
	size += (MSD_RMU_PACKET_PREFIX_SIZE - delta);
	ptrr = (MSD_U16*)ptr;

	while (flag == MSD_TRUE)
	{
		if (size >= pkt_size)
		{
			break;
		}
		if ((*ptrr == (MSD_U16)0xFFFF) && (*(ptrr + 1) == (MSD_U16)0xFFFF))
		{
			isFoundEnd = MSD_TRUE;
			break;
		}

		ptrr += 1;
		(*regCmdList)[i].data = msd_ntohs((MSD_U16)((*ptrr) & (MSD_U16)0xffff));
		ptrr = ptrr + 1;
		i++;
		size += 4U;
	}

	if (MSD_FALSE == isFoundEnd)
	{
		return MSD_FAIL;
	}

	return MSD_OK;
}

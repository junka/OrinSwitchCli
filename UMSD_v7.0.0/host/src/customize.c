#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "Wrapper.h"
#include "msdApi.h"
#include "rmuPcap.h"
#include "bus_conf.h"
#include <string.h>
#include "Wrapper.h"
#include <sys/ioctl.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <net/if.h> 


#define DEV_NUM	0

MSD_RMU_MODE RmuMode = MSD_RMU_DSA_MODE;
MSD_U32 EtherTypeValue = 0xed5a;
MSD_U8 DevNum = DEV_NUM;

extern MSD_QD_DEV       *qddev;
extern char    AdapterList[Max_Num_Adapter][512];
extern int     NumOfNIC;
extern unsigned char  AdapterMACAddr[6];
extern MSD_RMU_MODE gRmuMode;
extern MSD_U8 gFWRmuFlag;
extern MSD_U32 gEtherTypeValue;
extern MSD_U8 gXMDIOFlag;

typedef enum
{
    MASTER = 0,
    SLAVE
} DeviceMode;

MSD_U32 checkPhyData(MSD_U8 *devNum, int *phyAddr)
{
    MSD_U32 offset = 0x834;
    MSD_U32 phyData = 0;
    MSD_U32 data = 0;
    MSD_U8 devAddr = 0x1C;
    MSD_U8 regAddr = 0x19;
    MSD_STATUS retVal;

    retVal = msdSetAnyExtendedReg(*devNum, devAddr, regAddr, offset);

    if (retVal != MSD_OK) {
        //printf("Write Register fail");
        return 1;
    }
    //printf("Write - dev: 0x%X, reg: 0x%X, data = 0x%X \n", devAddr, regAddr, data);
//-------------------------------
    regAddr = 0x0018;
    data = 0xA001 +  (*phyAddr << 5);
    retVal = msdSetAnyExtendedReg(*devNum, devAddr, regAddr, data);

    if (retVal != MSD_OK) {
        //printf("Write Register fail");
        return 1;
    }
    //printf("Write - dev: 0x%X, reg: 0x%X, data = 0x%X \n", devAddr, regAddr, data);
//-------------------------------
    regAddr = 0x0018;
    data = 0xAC01 +  (*phyAddr << 5);
    retVal = msdSetAnyExtendedReg(*devNum, devAddr, regAddr, data);

    if (retVal != MSD_OK) {
        //printf("Write Register fail\n");
        return 1;
    }
    //printf("Write - dev: 0x%X, reg: 0x%X, data = 0x%X \n", devAddr, regAddr, data);
//-------------------------------
    //Read Register
    regAddr = 0x0019;
	retVal = msdGetAnyExtendedReg(*devNum, devAddr, regAddr, &phyData);

    if (retVal != MSD_OK) {   
        //printf("Read Register fail");
        return 1;
    }

    printf("Read - phyAddr: 0x%X, offset: 0x%X, data = 0x%X \n", *phyAddr, regAddr, phyData);
    return phyData;
}

int resetPortBasedVlan(MSD_U8 devNum)
{
    MSD_STATUS retVal;
    MSD_U32 data;
    MSD_U32 mapDAdata = 0x2080;

    for (int port = 0; port <= 11; port++)
    {   
        data = (port == 11) ? 0x7FF : (0x107FF - (1 << port));
        retVal = msdSetAnyExtendedReg(devNum, port, 0x08, mapDAdata);       
        retVal = msdSetAnyExtendedReg(devNum, port, 0x06, data);
    }
    printf("Reset Port Based Vlan Success\n");
    return 0;
}

int setDQAVlan(MSD_U8 devNum, MSD_U8 portNum_1, MSD_U8 portNum_2)
{
    MSD_STATUS retVal;
    MSD_U8 devAddr;
    MSD_U8 regAddr = 0x0008;
    MSD_U32 data;

    //MapDA disable
    data = 0x2000;
    retVal = msdSetAnyExtendedReg(devNum, portNum_1, regAddr, data);
    retVal = msdSetAnyExtendedReg(devNum, portNum_2, regAddr, data);
    
    //A pair of ports bind with VLAN
    regAddr = 0x0006;
    data = (portNum_2 == 11) ? 1 << 16 : 1 << portNum_2;
    retVal = msdSetAnyExtendedReg(devNum, portNum_1, regAddr, data);

    data = (portNum_1 == 11) ? 1 << 16 : 1 << portNum_1;;
    retVal = msdSetAnyExtendedReg(devNum, portNum_2, regAddr, data);

    //Auto-Negotiation 1 setting 
    data = (portNum_1 == 8) ? 0xa007 : 0xa007 + (portNum_1 << 5);
    retVal = msdSetAnyExtendedReg(devNum, 0x1c, 0x19, 0x200);
    retVal = msdSetAnyExtendedReg(devNum, 0x1c, 0x18, data);
    data = (portNum_1 == 8) ? 0xa407 : 0xa407 + (portNum_1 << 5);
    retVal = msdSetAnyExtendedReg(devNum, 0x1c, 0x19, 0x1000);  // If 1000BT1 is using set 0x1000, if 100BT1 is using set 0x0
    retVal = msdSetAnyExtendedReg(devNum, 0x1c, 0x18, data);
    
    //Auto-Negotiation 1 setting 
    data = (portNum_2 == 8) ? 0xa007 : 0xa007 + (portNum_2 << 5);
    retVal = msdSetAnyExtendedReg(devNum, 0x1c, 0x19, 0x200);
    retVal = msdSetAnyExtendedReg(devNum, 0x1c, 0x18, data);
    data = (portNum_2 == 8) ? 0xa407 : 0xa407 + (portNum_2 << 5);
    retVal = msdSetAnyExtendedReg(devNum, 0x1c, 0x19, 0x1000);  // If 1000BT1 is using set 0x1000, if 100BT1 is using set 0x0
    retVal = msdSetAnyExtendedReg(devNum, 0x1c, 0x18, data);

    return 0;
}

int setMasterOrSlave(MSD_U8 devNum, int phyAddr, char mode[])
{
    MSD_STATUS retVal;
    MSD_U8 devAddr, regAddr;
    MSD_U32 data;
    MSD_U32 offset = 0x834;
    MSD_U32 phyData;
    DeviceMode slaveOrMaster;

    devAddr = 0x001C;
    regAddr = 0x0019;

    slaveOrMaster = (strcmp(mode,"master") == 0) ? MASTER : SLAVE;
    phyData = (slaveOrMaster == MASTER) ? 0xC001 : 0x8001;

    //Set PHY register offset
	retVal = msdSetAnyExtendedReg(devNum, devAddr, regAddr, offset);

    if (retVal != MSD_OK) {
        //printf("Set PHY register offset fail\n");
        return 1;
    }
    //printf("Set PHY register offset success\n");
    //printf("Write - dev: 0x%X, reg: 0x%X, data = 0x%X \n", devAddr, regAddr, offset);
//-------------------------------------
    regAddr = 0x0018;
    data = 0xA001 +  (phyAddr << 5) ;
    retVal = msdSetAnyExtendedReg(devNum, devAddr, regAddr, data);

    if (retVal != MSD_OK) {
        //printf("Start SMI PHY operation fail\n");
        return 1;
    }
    //printf("Start SMI PHY operation success\n");
    //printf("Write - dev: 0x%X, reg: 0x%X, data = 0x%X \n", devAddr, regAddr, data);
//-------------------------------------
    regAddr = 0x0019;
	retVal = msdSetAnyExtendedReg(devNum, devAddr, regAddr, phyData);

    if (retVal != MSD_OK) {
        //printf("Set PHY register data fail\n");
        return 1;
    }
    //printf("Set PHY register data success\n");
    //printf("Write - dev: 0x%X, reg: 0x%X, data = 0x%X \n", devAddr, regAddr, phyData);
//-------------------------------------
    regAddr = 0x0018;
    data = 0xA401 + (phyAddr << 5) ;
    retVal = msdSetAnyExtendedReg(devNum, devAddr, regAddr, data);
    
    if (retVal != MSD_OK) {
        //printf("Write data into SMI PHY fail\n");
        return 1;
    }
    //printf("Write data into SMI PHY success\n");
    //printf("Write - dev: 0x%X, reg: 0x%X, data = 0x%X \n", devAddr, regAddr, data);
    if (checkPhyData(&devNum, &phyAddr) != phyData)
    {
        return 1;
    }
    return 0;
}

int setPhyMode(MSD_U8 devNum, int portNum, char mode[])
{
	//Switch to Slave / Master
	MSD_STATUS retVal;
    MSD_U8 devAddr, regAddr;
    /*if (nargs != 3)
    {
        CLI_ERROR("Syntax Error, Using command as follows: rr  <devAddr> <regAddr> : Read register\n");
        return 1;
    }*/
    //printf("Please enter the Target phy address(dec) and mode(master=0/slave=1)\n(phy_Addr=3 and mode=1(slave) please enter 3 1 for example): ");

    if (setMasterOrSlave(devNum, portNum, mode) != 0)
        return 1;
    return 0;
//--------------------------------
}
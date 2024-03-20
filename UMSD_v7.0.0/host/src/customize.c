#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
extern char    pcap_device_name[512];
extern char    AdapterList[Max_Num_Adapter][512];
extern int     NumOfNIC;
extern unsigned char  AdapterMACAddr[6];
extern MSD_RMU_MODE gRmuMode;
extern MSD_U8 gFWRmuFlag;
extern MSD_U32 gEtherTypeValue;
extern MSD_U8 gXMDIOFlag;
extern MSD_U8 sohoDevNum;

void runCustomizeCode()
{
	int command = 0;
	// Customize test cases here
	int nicId = 0;
	int i = 0;
	int tmp_flag;
	sohoDevNum = 0xf;
	gRmuMode = MSD_RMU_ETHERT_TYPE_DSA_MODE;
	gEtherTypeValue = 0x9101;
	gFWRmuFlag = 1;
	int bus_interface = -1;

	printf("RmuMode = %d, DevNum = %d, EtherTypeValue = %x\n", gRmuMode, (int)sohoDevNum, (int)gEtherTypeValue);
	pcap_rmuGetDeviceList();
	do {
			printf("\nPlease select NIC Index: ");
			if (scanf("%d", &nicId) == 0)
				printf("Failed to read the input NIC index\n");
			clean_stdin();
			if (!((nicId > 0) && (nicId <= NumOfNIC) && (nicId < 10)))
			{
				printf("Input nicId is invalid.\n");
				continue;
			}
			strcpy(pcap_device_name, AdapterList[nicId - 1]);
			struct ifreq interface;
			int   sock;
			if ((sock = socket(AF_INET, SOCK_STREAM, 0)) <0)
	 		{
				perror("socket ");
				return   2;
			}
			strncpy(interface.ifr_name, pcap_device_name, IF_NAMESIZE - 1);
			if (ioctl(sock, SIOCGIFHWADDR, &interface) <0)
			{
				close(sock);
				perror("ioctl ");
				return   3;
			}
			memcpy(AdapterMACAddr, interface.ifr_hwaddr.sa_data, 6);

		} while (!((nicId > 0) && (nicId <= NumOfNIC)));

			if (pcap_rmuOpenEthDevice() != 0)
			{
				printf("NIC Open Failed\n");
				printf("Press any key to exit program...");
				command = getchar();
				return -1;
			}
			bus_interface = 3;

}

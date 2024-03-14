#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Wrapper.h"
#include "msdApi.h"
#include "rmuPcap.h"
#include "bus_conf.h"

#define DEV_NUM	0

MSD_RMU_MODE RmuMode = MSD_RMU_DSA_MODE;
MSD_U32 EtherTypeValue = 0xed5a;
MSD_U8 DevNum = DEV_NUM;

void runCustomizeCode(MSD_U8 devNum)
{
	int command = 0;
	// Customize test cases here
	int nicId = 0;
	int i = 0;
	int tmp_flag;
	do
	{
		tmp_flag = 0;
		printf("Select RMU Mode:\n");
		printf("\t0 : DSA Mode\n");
		printf("\t1 : Ether Type DSA Mode\n");
		printf("Please select one RMU mode: ");
		if (scanf(" %c", (char *)(&command)) == 0)
			printf("Failed to read the input RMU mode index\n");
		clean_stdin();
		switch (command)
		{
		case '0':
			RmuMode = MSD_RMU_DSA_MODE;
			printf("Please enter the Target Device number (hex, Trg_Dev=0 please enter 0x0 for example):");
			if (scanf("%x", (int *)&DevNum) == 0)
				printf("Failed to read the input Target Device number\n");
			break;
		case '1':
			RmuMode = MSD_RMU_ETHERT_TYPE_DSA_MODE;
			printf("Please enter the Target Device number and Ether Type value\n(hex, Trg_Dev=0x0 and Etype=0x9100 please enter 0x0 0x9100 for example):");
			if (scanf("%x %x", (int *)&DevNum, (int *)&EtherTypeValue) == 0)
				printf("Failed to read input Target Devcie number and Ether Type\n");
			clean_stdin();
			break;
		default:
			printf("Invalid option. Please try again.\n");
			tmp_flag = 1;
			break;
		}
	} while (tmp_flag == 1);
}

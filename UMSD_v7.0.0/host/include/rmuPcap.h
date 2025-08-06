#ifndef __RMUPCAP_H__
#define __RMUPCAP_H__

#define Max_Num_Adapter 20

void pcap_rmuGetDeviceList();
int pcap_rmuOpenEthDevice();
int pcap_rmuCloseEthDevice();
int send_and_receive_packet(
		unsigned char *req_packet,
		unsigned int req_pktlen,
		unsigned char **rsp_packet,
		unsigned int *rsp_pktlen);
void scanTargetDevicesList(MSD_RMU_MODE rmuMode, uint16_t eTypeValue, const char *name);
#endif /*__RMUPCAP_H__*/

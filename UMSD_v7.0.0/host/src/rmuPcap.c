#ifdef LIBPCAP
#include <pcap/pcap.h>
#else
#include <unistd.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/filter.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "msdApi.h"
#include "rmuPcap.h"

#ifdef LIBPCAP
static pcap_t *fp_pcap;
#else
static int sockfd;
#endif
char    pcap_device_name[512] = { 0 };
char    AdapterList[Max_Num_Adapter][512];
int		NumOfNIC = 0;
unsigned char	AdapterMACAddr[6] = { 0 };
extern MSD_RMU_MODE gRmuMode;
extern MSD_U8 gFWRmuFlag;
extern MSD_U32 gEtherTypeValue;
extern MSD_U8 sohoDevNum;

void pcap_rmuGetDeviceList()
{
#ifdef LIBPCAP
	pcap_if_t *interfaces;
	char errbuf[PCAP_ERRBUF_SIZE];
	/* Retrieve the device list from the local machine */
	if (pcap_findalldevs(&interfaces, errbuf) == -1)
	{
		fprintf(stderr, "Error in pcap_findalldevs_ex: %s\n", errbuf);
		exit(1);
	}
	printf("\n================ NIC CARD List ================\n");
	for (pcap_if_t *d = interfaces; d != NULL; d = d->next)
	{
		//strcpy(pcap_device_name, d->name);
		//printf("device: %s\n", d->description);
		strcpy(AdapterList[NumOfNIC], d->name);
		printf("( %d ) %s\n", ++NumOfNIC, d->name);
		if ((d->description) != NULL)
			printf("      %s\n", d->description);
		else
		{
			printf("\n");
		}
	}

	if (NumOfNIC == 0)
	{
		printf("\nNo interfaces found!\n");
	}

	/* We don't need any more the device list. Free it */
	pcap_freealldevs(interfaces);
#else
	struct ifaddrs *interfaces, *ifa;
	/* Retrieve the device list from the local machine */
	if (getifaddrs(&interfaces) == -1)
	{
		perror("Error in getifaddrs");
		exit(1);
	}
	printf("\n================ NIC CARD List ================\n");
	for (ifa = interfaces; ifa != NULL; ifa = ifa->ifa_next)
	{
		if (ifa->ifa_addr == NULL || !(ifa->ifa_flags & IFF_UP) || !(ifa->ifa_flags & IFF_RUNNING))
			continue;

		if (ifa->ifa_flags & IFF_LOOPBACK) // Skip loopback interfaces
			continue;

		strcpy(AdapterList[NumOfNIC], ifa->ifa_name);
		printf("( %d ) %s\n", ++NumOfNIC, ifa->ifa_name);
	}

	if (NumOfNIC == 0)
	{
		printf("\nNo interfaces found!\n");
	}

	/* Free the interface list */
	freeifaddrs(interfaces);
#endif
}

int pcap_rmuOpenEthDevice()
{
#ifdef LIBPCAP
	char *dev;
	char errbuf[PCAP_ERRBUF_SIZE];
    char packet_filter[512] = { 0 };
	struct bpf_program fcode;
	bpf_u_int32 NetMask;
	printf("\nUsing Interface: %s\n\n", pcap_device_name /*dev*/);

    if (gRmuMode == MSD_RMU_ETHERT_TYPE_DSA_MODE) {
        sprintf(packet_filter, "len <= 512 and ether[12] == 0x%02x and ether[13] == 0x%02x and ether[14] == 0x00 and ether[15] == 0x00 and ether[16] == 0x%02x and ether[17] == 0x00", 
            (gEtherTypeValue >> 8) & 0xff, gEtherTypeValue & 0xff, sohoDevNum & 0x1F);
    } else if (gRmuMode == MSD_RMU_DSA_MODE) {
        sprintf(packet_filter, "len <= 512 and ether[12] == 0x%02x and ether[13] == 0x00",
            sohoDevNum & 0x1F);
    }
	/* Open the adapter */
	if ((fp_pcap = pcap_open_live(pcap_device_name,//pcap_device_name /*dev*/,  // name of the device
		65536,	           // portion of the packet to capture. It doesn't matter in this case
		1,	               // promiscuous mode (nonzero means promiscuous)
		10,	               // read timeout
		errbuf	           // error buffer
		)) == NULL)
	{
		fprintf(stderr, "\nUnable to open the adapter %s.\n", pcap_device_name);
		perror("pcap_open_live");
		return 1;
	}

	// Let's do things simpler: we suppose to be in a C class network ;-)
	NetMask = 0xffffff;

	//compile the filter
	if (pcap_compile(fp_pcap, &fcode, packet_filter, 1, NetMask) < 0)
	{
		fprintf(stderr, "\nError compiling filter: wrong syntax.\n");

		pcap_close(fp_pcap);
		return 1;
		/* return FALSE; */
	}

	//set the filter
	if (pcap_setfilter(fp_pcap, &fcode)<0)
	{
		fprintf(stderr, "\nError setting the filter\n");

		pcap_close(fp_pcap);
		return 1;
	}
#else
	struct ifreq ifr;

	if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1) {
		perror("Error opening raw socket");
		return 1;
	}

	// Bind the socket to the specified network interface
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, pcap_device_name, IFNAMSIZ - 1);
	if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(struct ifreq)) < 0)
	{
		perror("Error binding socket to device");
		close(sockfd);
		return 1;
	}

	// Note: Additional filtering logic can be implemented here if needed
	// using BPF (Berkeley Packet Filter) or other mechanisms.
	struct sock_filter bpfcode[] = {
		{ 0x28, 0, 0, 0x0000000e },
		{ 0x15, 0, 7, 0x00009101 },
		{ 0x80, 0, 0, 0x00000000 },
		{ 0x25, 5, 0, 0x00000200 },
		{ 0x30, 0, 0, 0x00000010 },
		{ 0x15, 0, 3, 0x00000004 },
		{ 0x30, 0, 0, 0x00000011 },
		{ 0x15, 0, 1, 0x00000000 },
		{ 0x6, 0, 0, 0x00040000 },
		{ 0x6, 0, 0, 0x00000000 },
	};
	struct sock_filter bpfcode2[] = {
		{ 0x80, 0, 0, 0x00000000 },
		{ 0x25, 5, 0, 0x00000200 },
		{ 0x30, 0, 0, 0x0000000c },
		{ 0x15, 0, 3, 0x00000004 },
		{ 0x30, 0, 0, 0x0000000d },
		{ 0x15, 0, 1, 0x00000000 },
		{ 0x6, 0, 0, 0x00040000 },
		{ 0x6, 0, 0, 0x00000000 },
	};
	struct sock_filter *pbpfcode;
	size_t bpfcode_size;
	if (gRmuMode == MSD_RMU_ETHERT_TYPE_DSA_MODE) {
        bpfcode[1].k = gEtherTypeValue;
		bpfcode[5].k = sohoDevNum & 0x1F;
		pbpfcode = bpfcode;
		bpfcode_size = sizeof(bpfcode) / sizeof(struct sock_filter);
    } else if (gRmuMode == MSD_RMU_DSA_MODE) {
		bpfcode2[3].k = sohoDevNum & 0x1F;
		pbpfcode = bpfcode2;
		bpfcode_size = sizeof(bpfcode2) / sizeof(struct sock_filter);
	}
	struct sock_fprog prog = {
		.len = bpfcode_size,
		.filter  = pbpfcode
	};
	int ret = setsockopt(sockfd, SOL_SOCKET, SO_ATTACH_FILTER, &prog, sizeof(prog));
	if (ret < 0) {
		perror("Error attach filter to device");
		close(sockfd);
		return 1;
	}

#endif
	return 0;
}

int pcap_rmuSendPacket(unsigned char *packet)
{
#ifdef LIBPCAP
	if (pcap_sendpacket(fp_pcap,	// Adapter
		packet,				// buffer with the packet
		150					// size
		) != 0)
	{
		fprintf(stderr, "\nError sending the packet: %s\n", pcap_geterr(fp_pcap));
		return 1;
	}
#else

	// Send the packet
	if (send(sockfd, packet, 150, 0) <= 0)
	{
		perror("Error sending the packet");
		return 1;
	}

#endif
	return 0;
}

int pcap_rmuReceivePacket(unsigned char **packet, unsigned char id)
{
	unsigned int exit = 0;
	unsigned int drop_cnt = 0;
#ifdef LIBPCAP
	struct pcap_pkthdr *mypkt_hdr;

	while (exit == 0)
	{
		pcap_next_ex(fp_pcap, &mypkt_hdr, (const u_char **)packet);

		if (mypkt_hdr->len != 0)
		{
			if (*((*packet) + 19) == id)
			{
				exit = 1;
			}
		}

		drop_cnt++;
		if (drop_cnt > 100)
		{
			printf("Packet drop count exceeds limit: %d\n", drop_cnt);
			exit = 1;
		}
	}
	return mypkt_hdr->len;
#else
	int retlen;
	while (exit == 0) {
		retlen = recv(sockfd, *packet, 150, 0);
		if (retlen > 0) {
			if (*((*packet) + 19) == id)
			{
				exit = 1;
			}
		}
		drop_cnt++;
		if (drop_cnt > 100)
		{
			printf("Packet drop count exceeds limit: %d\n", drop_cnt);
			exit = 1;
		}
	}
	return retlen;
#endif
}

int pcap_rmuCloseEthDevice()
{
#ifdef LIBPCAP
	if (fp_pcap != NULL)
	{
		pcap_close(fp_pcap);
	}
#else
	if (sockfd >= 0)
	{
		close(sockfd);
	}
#endif
	return 0;
}

int send_and_receive_packet(
	unsigned char *req_packet,
	unsigned int req_pktlen,
	unsigned char **rsp_packet,
	unsigned int *rsp_pktlen)
{
	unsigned int retVal;
	unsigned int size = 512;
	unsigned int exit = 0;
	unsigned int drop_cnt = 0;
	unsigned int drop_cnt_limit = 100;
    unsigned int seqNumOffset = 19;
#ifdef LIBPCAP
	struct pcap_pkthdr *mypkt_hdr;
#endif
	/*Override packet SA = CPU MAC: 0x28, 0xD2, 0x44, 0x8C, 0xF9, 0xF3*/

	/* SA */
	*(req_packet + 6) = AdapterMACAddr[0];
	*(req_packet + 7) = AdapterMACAddr[1];
	*(req_packet + 8) = AdapterMACAddr[2];
	*(req_packet + 9) = AdapterMACAddr[3];
	*(req_packet + 10) = AdapterMACAddr[4];
	*(req_packet + 11) = AdapterMACAddr[5];

	/* check fw rmu, length = 60 */
	if (gFWRmuFlag != 0)
	{
		size = 60;
	}

    if (gRmuMode == MSD_RMU_ETHERT_TYPE_DSA_MODE)
        seqNumOffset = 19;
    else if (gRmuMode == MSD_RMU_DSA_MODE)
        seqNumOffset = 15;

#ifdef LIBPCAP
	retVal = pcap_sendpacket(fp_pcap, req_packet, size);
	if (retVal)
	{
		fprintf(stderr, "\nError sending packet: %s\n", pcap_geterr(fp_pcap));
		return retVal;
	}

	while (exit == 0)
	{
		retVal = pcap_next_ex(fp_pcap, &mypkt_hdr, (const u_char **)rsp_packet);
		if(retVal != 1)
			continue;

		if (mypkt_hdr->len != 0)
		{
			// dump_packet(&rsp_packet, mypkt_hdr->len, "Received Packet:");
			if (*(req_packet + seqNumOffset) != *(*(rsp_packet)+seqNumOffset)) // Check sequence number of dsatag
			{
				 printf("Error: DSA tag sequence numbers does not match - req: %X, rsp: %X\n",
                     *(req_packet + seqNumOffset), *((*rsp_packet) + seqNumOffset));

				//exit = 1;
			}
			else
			{
				if (*(req_packet + 6) != *(*(rsp_packet)))
				{

					;
				}
				else
					exit = 1;
			}
		}

		drop_cnt++;
		if (drop_cnt > drop_cnt_limit)
		{
			printf("Packet drop count exceeds limit: %d\n", drop_cnt_limit);
			//exit = 1;
			return -1;
		}
	}
	*rsp_pktlen = mypkt_hdr->len;

	
#else
	retVal = send(sockfd, req_packet, size, 0);
	if (retVal != 0)
	{
		fprintf(stderr, "\nError sending packet\n");
		return retVal;
	}

	while (exit == 0)
	{
		retVal = recv(sockfd, *rsp_packet, req_pktlen, 0);
		if(retVal != 1)
			continue;

		if (retVal != 0)
		{
			// dump_packet(&rsp_packet, mypkt_hdr->len, "Received Packet:");
			if (*(req_packet + seqNumOffset) != *(*(rsp_packet)+seqNumOffset)) // Check sequence number of dsatag
			{
				 printf("Error: DSA tag sequence numbers does not match - req: %X, rsp: %X\n",
                     *(req_packet + seqNumOffset), *((*rsp_packet) + seqNumOffset));

				//exit = 1;
			}
			else
			{
				if (*(req_packet + 6) != *(*(rsp_packet)))
				{

					;
				}
				else
					exit = 1;
			}
		}

		drop_cnt++;
		if (drop_cnt > drop_cnt_limit)
		{
			printf("Packet drop count exceeds limit: %d\n", drop_cnt_limit);
			//exit = 1;
			return -1;
		}
	}
	*rsp_pktlen = retVal;
#endif
	if (*rsp_pktlen < req_pktlen)
	{
		printf("Error: rsp_pktlen - %d < req_pktlen - %d\n", *rsp_pktlen, req_pktlen);
		return -1;
	}
	return 0;
}

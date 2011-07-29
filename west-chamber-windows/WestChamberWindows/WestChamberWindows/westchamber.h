/*
 *WestChamber Windows
 *Elysion
 *March 16 2010
 */

#pragma once
#pragma pack(1)

//Structures

struct ethhdr {
unsigned char ether_dhost[6];
unsigned char ether_shost[6];
unsigned short ether_type;
};

struct iphdr {
	unsigned char ihl:4;
	unsigned char version:4;
    unsigned char tos;
    unsigned short tot_len;
    unsigned short id;
    unsigned short frag_off;
    unsigned char ttl;
    unsigned char protocol;
    unsigned short check;
    unsigned int saddr;
    unsigned int daddr;
};

struct tcphdr {
    unsigned short source;
    unsigned short dest;
    unsigned int seq;
    unsigned int ack_seq;
	unsigned char res1:4;
	unsigned char doff:4;
	unsigned char fin:1;
	unsigned char syn:1;
	unsigned char rst:1;
	unsigned char psh:1;
	unsigned char ack:1;
	unsigned char urg:1;
	unsigned char res2:2;
    unsigned short window;
    unsigned short check;
    unsigned short urg_ptr;
};

struct udphdr {
unsigned short source;
unsigned short dest;
unsigned short len;
unsigned short check;
};

struct psudo_tcp_udp_header {
	unsigned int s_addr;
	unsigned int d_addr;
	unsigned char zero;
	unsigned char protocol;
	unsigned short length;
};

struct tcp_pack {
	struct ethhdr eth;
	struct iphdr ip;
	struct tcphdr tcp;
};

//Functions and Macros

PUCHAR GetPacket(PNDIS_PACKET Packet);
BOOLEAN IsGFWPoisoned(PUCHAR data);
BOOLEAN IsUdpWithPortFiftyThree(PUCHAR pContent);
BOOLEAN IsReceivedPacketInList(PUCHAR data);
BOOLEAN IsIPVerFour(PUCHAR packet);
BOOLEAN WestChamberReceiverMain(PNDIS_PACKET packet,PADAPT adapt);
NDIS_STATUS MySendPacket(NDIS_HANDLE NdisBindingHandle,NDIS_HANDLE NdisSendPacketPool,PVOID pBuffer,ULONG dwBufferLength);
void DebugPrintPacket(PUCHAR packet,ULONG size);

unsigned short ntohs(unsigned short x);
unsigned short htons(unsigned short x);
unsigned int htonl(unsigned int hostlong);
unsigned int ntohl(unsigned int hostlong);

#define PrintLog(a) KdPrint((a))

#define ETH_MAX_PACKET_SIZE 2048
#define ETH_MIN_PACKET_SIZE 64
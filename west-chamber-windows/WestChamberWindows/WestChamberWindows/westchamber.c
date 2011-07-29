/*
 * WestChamber Windows
 * Elysion
 * March 16 2010
 */

#include "precomp.h"
#pragma hdrstop

enum FILTER_STATE filter_state;

unsigned short ntohs(unsigned short x)
{
        unsigned char *s = (unsigned char *) &x;
        return (unsigned short)(s[0] << 8 | s[1]);
}
unsigned short htons(unsigned short x)
{
        unsigned char *s = (unsigned char *) &x;
        return (unsigned short)(s[0] << 8 | s[1]);
}
unsigned int htonl(unsigned int hostlong)
{
  return ((hostlong>>24) | ((hostlong&0xff0000)>>8) | ((hostlong&0xff00)<<8) | (hostlong<<24));
}
unsigned int ntohl(unsigned int hostlong)
{
  return ((hostlong>>24) | ((hostlong&0xff0000)>>8) | ((hostlong&0xff00)<<8) | (hostlong<<24));
}

PUCHAR GetPacket(PNDIS_PACKET packet)
//Ref:http://www.cppblog.com/ay19880703/archive/2009/06/23/62233.html
{
	NDIS_STATUS		status;
	PNDIS_BUFFER	buffer;
	UINT			tot_len = 0;
	UINT			copysize = 0;
	UINT			offset = 0;
	UINT			ph_count;
	UINT			count;
	PUCHAR			mybuffer = NULL;
	PUCHAR			tmpbuffer = NULL;  
	NDIS_PHYSICAL_ADDRESS HighestAcceptableMax = {-1,-1};

	//------------------------Preperation---------------------------------
	NdisQueryPacket(packet, &ph_count, &count, &buffer, &tot_len);
	status = NdisAllocateMemory(&mybuffer, ETH_MAX_PACKET_SIZE, 0, HighestAcceptableMax);
	if (status != NDIS_STATUS_SUCCESS)
		return NULL;
	NdisZeroMemory(mybuffer, ETH_MAX_PACKET_SIZE);

	//------------------------Copy First Buffer---------------------------
	NdisQueryBufferSafe(buffer, &tmpbuffer, &copysize ,NormalPagePriority);
	/*
	if (tmpbuffer == NULL) {
		NdisFreeMemory(mybuffer, ETH_MAX_PACKET_SIZE ,0);
		return NULL;
	}*/
	NdisMoveMemory(mybuffer, tmpbuffer, copysize);
	offset = copysize;

	//------------------------Copy everything-----------------------------
	while(1) {				//TODO: unsafe.
		NdisGetNextBuffer(buffer, &buffer);
		if (buffer == NULL)
			break;
		NdisQueryBufferSafe(buffer ,&tmpbuffer, &copysize, NormalPagePriority);
		NdisMoveMemory(mybuffer + offset, tmpbuffer, copysize);
		offset += copysize;
	}
	return mybuffer;
}

VOID FreePacket(PUCHAR packet)
{
	NdisFreeMemory(packet, ETH_MAX_PACKET_SIZE, 0);
}

NDIS_STATUS MySendPacket(NDIS_HANDLE biding_handle, NDIS_HANDLE pool, PVOID buffer, ULONG buffer_length)
//Ref:http://www.cnblogs.com/xuneng/archive/2009/11/30/1613452.html
{
   NDIS_STATUS     status;
   PNDIS_PACKET    send_packet = NULL;
   PNDIS_BUFFER    send_packet_buffer = NULL;
   PUCHAR          send_buffer = NULL;
   ULONG           send_buffer_length; 
   PSEND_RSVD      rsvd = NULL;
   NDIS_PHYSICAL_ADDRESS HighestAcceptableAddress={-1,-1};

   //--------------------------Preperation------------------------------
   if (!biding_handle || !buffer || buffer_length > ETH_MAX_PACKET_SIZE)
       return NDIS_STATUS_FAILURE;

   //-------------------------Copy the buffer---------------------------
   send_buffer_length = max(buffer_length, ETH_MIN_PACKET_SIZE);
   status = NdisAllocateMemory(&send_buffer, send_buffer_length, 0, HighestAcceptableAddress);
   if (status != NDIS_STATUS_SUCCESS)
       return status;
   RtlMoveMemory(send_buffer, buffer, send_buffer_length);

   //------------------------Allocate a packet--------------------------
   NdisAllocatePacket(&status, &send_packet, pool);
   if (status != NDIS_STATUS_SUCCESS) {
       NdisFreeMemory(send_buffer, send_buffer_length, 0);       
       return status;
   }

   //-----------------------Alloc a NDIS buffer-------------------------
   NdisAllocateBuffer(&status, &send_packet_buffer, pool, send_buffer, send_buffer_length);
   if (status != NDIS_STATUS_SUCCESS) {
       NdisFreeMemory(send_buffer, send_buffer_length, 0);
       NdisDprFreePacket(send_packet);
       return status;
   }
	
   //----------------------Link buffer to packet------------------------
   NdisChainBufferAtFront(send_packet, send_packet_buffer);

   //---------------------Set our sign----------------------------------
   rsvd = (PSEND_RSVD)(send_packet->ProtocolReserved); 
   rsvd->OriginalPkt = NULL;

   //---------------------Settings--------------------------------------
   send_packet->Private.Head->Next=NULL; 
   send_packet->Private.Tail=NULL;
   NdisSetPacketFlags(send_packet, NDIS_FLAGS_DONT_LOOPBACK);

   //--------------------Send it----------------------------------------
   NdisSend(&status, biding_handle, send_packet);
   
   //---------------------Free it now if not pending-------------------
   if (status != STATUS_PENDING)
   {
       NdisUnchainBufferAtFront(send_packet, &send_packet_buffer); 
       NdisQueryBufferSafe(send_packet_buffer, (PVOID *)&send_buffer, &send_buffer_length, HighPagePriority);
       NdisFreeBuffer(send_packet_buffer); 
       NdisFreeMemory(send_buffer, send_buffer_length, 0); 
       NdisDprFreePacket(send_packet);
   }

   return status;
}

BOOLEAN IsUdpWithPortFiftyThree(PUCHAR pContent) 
 { 
	return (pContent[12]==0x8 && pContent[13]==0x0		   //IPv4 
			&& 
			pContent[23]==0x11                             //UDP 
			&& 
			pContent[34]==0 && pContent[35]==53);          //Source Port: 53 
 } 
 


BOOLEAN IsTcpWithPortEighty(PUCHAR pContent)
{
	return (pContent[12]==0x8 && pContent[13]==0x0		   //IPv4
			&&
			pContent[23]==0x06                             //TCP
			&&
			pContent[34]==0 && pContent[35]==80);          //Source Port :80
}


BOOLEAN IsGFWPoisoned(PUCHAR data)
{
	unsigned short window;
	struct iphdr *ip;
	struct tcphdr *th;
	struct udphdr *uh;
	unsigned char *end;
	unsigned short *dns;
	unsigned int addr,ttl;
	unsigned short name;

    ip = (struct iphdr*)(data + sizeof(struct ethhdr));

    if( ip->frag_off & htons(0x1FFF))
		return FALSE;

    if (ip->protocol == 0x06) {		//TCP
		th = (struct tcphdr*)(data + sizeof(struct ethhdr) + sizeof(struct iphdr));
		if ( ip->tot_len <= sizeof(struct iphdr) || ( th->doff*4 < sizeof(struct tcphdr)) )
			return FALSE;
		if ( th->doff*4 != sizeof(struct tcphdr) )
            return FALSE;
		window = ntohs(th->window); 
		if ( ip->frag_off & htons(0x4000) ) {
			if ( ( (th->rst || th->syn) && th->ack && !th->fin && ntohs(ip->id) == (unsigned short)(-1 - window * 13) )
				|| ntohs(ip->id) == (unsigned short)(62753 - window * 79) )
				return TRUE;
		} //type2[a]
        else {
			if ( (ip->id == htons(64) && th->rst && !th->ack && !th->syn && !th->fin && window % 17 == 0)
				|| (window - ntohs(th->source) / 2) % 9 == 0 )
				return TRUE;
        } //type1[a]
    }
    else if (ip->protocol==0x11) {	//UDP
		uh = (struct udphdr*)(data + sizeof(struct ethhdr) + sizeof(struct iphdr));
		if (ip->tot_len < (sizeof(struct ethhdr) + sizeof(struct iphdr)) || ntohs(uh->len) < sizeof(struct udphdr))
			return FALSE;
		if ((ip->frag_off & htons(0x4000)) || (ntohs(uh->len) < (sizeof(struct udphdr) + 12 + 16) ))
			return FALSE;

		end = (unsigned char *)(uh + ntohs(uh->len));
		dns = (unsigned short *)((char*)uh + sizeof(struct udphdr));
		if (dns[2] != htons(1) || dns[3] != htons(1) || dns[4] != 0 || dns[5] != 0 || *(unsigned int *)(end-14) != htonl(0x00010001))
			return FALSE;
                        
		addr = *(unsigned int *)(end-4);
		ttl = *(unsigned int *)(end-10);
		name = *(unsigned short *)(end-16);
		if (addr == htonl(0x5d2e0859) || addr == htonl(0xcb620741) ||
		    addr == htonl(0x0807c62d) || addr == htonl(0x4e10310f) ||
		    addr == htonl(0x2e52ae44) || addr == htonl(0xf3b9bb27) ||
		    addr == htonl(0xf3b9bb1e) || addr == htonl(0x9f6a794b) ||
		    addr == htonl(0x253d369e) || addr == htonl(0x9f1803ad) ||
		    addr == htonl(0x3b1803ad))
			return true;
    }
    return FALSE;
}

USHORT GetChecksum(PVOID buf,int size)
//Ref:http://hi.bccn.net/space-112902-do-blog-id-12121.html
{
	USHORT* buffer = (USHORT*)buf;
	unsigned long cksum = 0;
	while (size>1) {
		cksum += *buffer++;
		size -= sizeof(USHORT);
	}
	if (size) {
		cksum += *(UCHAR *)buffer;
	}
	while (cksum >> 16) {
		cksum = (cksum >> 16) + (cksum & 0xffff);
	}
	return (USHORT)(~cksum);
}

USHORT GetTcpChecksum(PUCHAR packet)
{
	struct iphdr *ip;
	struct tcphdr *tcp;
	struct psudo_tcp_udp_header *header;
	USHORT tcp_length;
	USHORT result;
	PUCHAR buffer;
	
	ip = (struct iphdr *)(packet + sizeof(struct ethhdr));
	tcp = (struct tcphdr *)(packet + sizeof(struct ethhdr) + sizeof(struct iphdr));

	tcp_length = ntohs(ip->tot_len) - sizeof(struct iphdr);

	buffer = (PUCHAR)ExAllocatePool(NonPagedPool, sizeof(struct psudo_tcp_udp_header) + tcp_length);

	header = (struct psudo_tcp_udp_header *)buffer;
	header->s_addr = ip->saddr;
	header->d_addr = ip->daddr;
	header->zero = 0;
	header->protocol = 0x06;	//TCP
	header->length = ntohs(tcp_length);

	RtlMoveMemory(buffer + sizeof(struct psudo_tcp_udp_header), tcp, tcp_length);
	result = GetChecksum(buffer, sizeof(struct psudo_tcp_udp_header) + tcp_length);
	
	ExFreePool(buffer);
	return result;
}

void CodeZhang(PUCHAR packet,PADAPT adapter)
{
	struct ethhdr *eth;
	struct iphdr *ip;
	struct tcphdr *tcp;
	struct tcp_pack *sender;
	struct tcp_pack *sender2;

	eth = (struct ethhdr *)packet;
	ip = (struct iphdr *)(packet + sizeof(struct ethhdr));
	tcp = (struct tcphdr *)(packet + sizeof(struct ethhdr) + sizeof(struct iphdr));

	//check packet
	if(ip->frag_off&htons(0x1FFF) || ip->tot_len <= sizeof(struct iphdr) || !tcp->syn || !tcp->ack || tcp->rst || tcp->fin)
		return;
	else if(GetTcpChecksum(packet))
		return;

	//create packet 1 (FIN)
	sender = (struct tcp_pack *)ExAllocatePool(NonPagedPool, sizeof(struct tcp_pack));
	RtlZeroMemory(sender, sizeof(struct tcp_pack));

	//Ethernet
	RtlMoveMemory(sender->eth.ether_shost, eth->ether_dhost, 6);
	RtlMoveMemory(sender->eth.ether_dhost, eth->ether_shost, 6);
	sender->eth.ether_type = ntohs(0x0800);

	//IP
	sender->ip.version = 4;
	sender->ip.ihl = sizeof(struct iphdr) / 4;
	sender->ip.tos = 0;
	sender->ip.id = 0;
	sender->ip.frag_off = htons(0x4000);
	sender->ip.protocol = 0x06;
	sender->ip.saddr = ip->daddr;
	sender->ip.daddr = ip->saddr;	
	sender->ip.tot_len = ntohs(sizeof(struct tcp_pack) - sizeof(struct ethhdr));
	sender->ip.ttl = 0xFF;	//let the packet survive as long as possible.
	sender->ip.check = GetChecksum(&sender->ip, sizeof(struct iphdr));

	//TCP
	sender->tcp.source = tcp->dest;
	sender->tcp.dest = tcp->source;
	sender->tcp.doff = sizeof(struct tcphdr) / 4;
	sender->tcp.window = 0xFFFF;
	sender->tcp.fin = 1;
	sender->tcp.seq = tcp->ack_seq;
	sender->tcp.ack_seq = tcp->seq;
	sender->tcp.check = GetTcpChecksum((PUCHAR)sender);
	
	//create packet 2 (ACK)
	sender2 = (struct tcp_pack *)ExAllocatePool(NonPagedPool, sizeof(struct tcp_pack));
	
	//copy from packet 1
	RtlMoveMemory(sender2, sender, sizeof(struct tcp_pack));
	
	//TCP
	sender2->tcp.fin = 0;
	sender2->tcp.ack = 1;
	sender2->tcp.check = 0;
	sender2->tcp.check = GetTcpChecksum((PUCHAR)sender2);
	
	//Sending Packets.
	MySendPacket(adapter->BindingHandle, adapter->SendPacketPoolHandle, sender, sizeof(struct tcp_pack));
	MySendPacket(adapter->BindingHandle, adapter->SendPacketPoolHandle, sender2, sizeof(struct tcp_pack));

	//Release packets.
	ExFreePool(sender);
	ExFreePool(sender2);

	return;
}

BOOLEAN IsReceivedPacketInList(PUCHAR data)
{
	struct iphdr *ip = (struct iphdr *)(data + sizeof(struct ethhdr));
	return IsInIpTable(ip->saddr);		//BIG_ENDIAN
}

BOOLEAN IsTcpSynAck(PUCHAR packet)
{
	struct tcphdr *tcp = (struct tcphdr *)(packet + sizeof(struct ethhdr) + sizeof(struct iphdr));
	return((tcp->syn) && (tcp->ack) && (!tcp->rst) && (!tcp->fin));
}

BOOLEAN IsIPVerFour(PUCHAR packet)
{
	struct ethhdr *eth = (struct ethhdr *)packet;
	return (eth->ether_type == ntohs(0x0800));	//IPv4 only.
}

BOOLEAN WestChamberReceiverMain(PNDIS_PACKET packet,PADAPT adapt)
//The return value indicates whether if we should let the packet pass.
{
	BOOLEAN result = TRUE,udp = FALSE,tcp = FALSE,gfw = FALSE,inlist = FALSE,sign = FALSE;
	PUCHAR pack;
	
	pack = GetPacket(packet);
	if(pack == NULL)		//cannot get the packet.
		return TRUE;

	//ipv4 only.
	if(IsIPVerFour(pack) == FALSE) {
		FreePacket(pack);
		return TRUE;
	}

	udp = IsUdpWithPortFiftyThree(pack);
	tcp = IsTcpWithPortEighty(pack);
	if(udp || tcp)
		gfw = IsGFWPoisoned(pack);
	
	// drop any rst packet
	if(tcp && filter_state != FILTER_STATE_NONE) {
		rst = IsTcpRst(pack);
		
		if (rst) {
			if ( (filter_state == FILTER_STATE_ALL) || IsReceivedPacketInList(pack) ) {
				result = FALSE;
			}
		}
	}

	if(gfw)	{
		PrintLog("Detected GFW Poisoned Data -- ");
		if(udp) {
			PrintLog("Type=UDP, Port=53 (DNS data) -- dropped.\n");
			result = FALSE;
		}
		else
			PrintLog("Type=TCP, Port=80\n");
	}

	FreePacket(pack);
	return result;
}

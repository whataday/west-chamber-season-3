/*
 *WestChamber Windows
 *Elysion
 *March 16 2010
 */

#include "precomp.h"
#pragma hdrstop

////////////////////////Private Members////////////////////

//Hash table.
struct avl_node** nodes;

//Initialization Flag
BOOLEAN initialized;

///////////////////////////////////////////////////////////

void HashTableInit()
{
	int j;
	nodes = (struct avl_node**)ExAllocatePool(NonPagedPool, sizeof(struct avl_node*) * 0x10000);
	for(j = 0; j < 0x10000 ; j++)
		nodes[j]=NULL;               //Emply Trees
	initialized=TRUE;
}
void HashTableDeInit()
{
	int j;
	if(!initialized)
		return;
	for(j = 0; j < 0x10000 ; j++) {
		if(nodes[j] != NULL)
			avl_delete(nodes[j]);
	}
	ExFreePool(nodes);
}
void HashTableInsert(unsigned int value)
{
	unsigned short num = (value >> 16);
	unsigned short val = (value & 0xFFFF);
	avl_insert(avl_create(val), &nodes[num]);
}
BOOLEAN IsInHashTable(unsigned int value)
{
	unsigned short num = (value >> 16);
	unsigned short val = (value & 0xFFFF);
	return ((avl_search(nodes[num],val) != NULL));
}

BOOLEAN InitializeIpTable(LPCWSTR binary_file)
{
	HANDLE file;
	OBJECT_ATTRIBUTES attrib;
	UNICODE_STRING str;
	IO_STATUS_BLOCK block;
	unsigned int num = 0,ip = 0;
	LARGE_INTEGER offset = {0};
	unsigned int j = 0;
	initialized = FALSE;
	RtlInitUnicodeString(&str, binary_file);
	InitializeObjectAttributes(&attrib, &str, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,NULL,NULL);

	ZwCreateFile(&file, GENERIC_READ, &attrib, &block, NULL,
		FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ, FILE_OPEN,FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

	if(block.Status != STATUS_SUCCESS) {
		PrintLog("Cannot Read IP Table\n");
		return FALSE;
	}

	ZwReadFile(file,NULL,NULL,NULL,&block,&num,4,&offset,NULL);
	offset.QuadPart += 4;
	HashTableInit();

	for(j = 0; j < num; j++) {
		ZwReadFile(file,NULL,NULL,NULL,&block,&ip,4,&offset,NULL);
		offset.QuadPart += 4;
		HashTableInsert(ip);
	}

	ZwClose(file);
	KdPrint(("%d logs of IP Address loaded.\n",num));

	return TRUE;
}

BOOLEAN IsInIpTable(unsigned int ip_val)
{
	if(!initialized) {
		return FALSE;
	}
	return IsInHashTable(ip_val);
}

void DeInitializeIpTable()
{
	if(!initialized) {
		return;
	}
	HashTableDeInit();
	PrintLog("DeInitialization finished.\n");
}
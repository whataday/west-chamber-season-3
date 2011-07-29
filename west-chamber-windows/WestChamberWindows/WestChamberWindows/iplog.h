/*
WestChamber Windows
Elysion
March 16 2010
*/

#pragma once

BOOLEAN InitializeIpTable(LPCWSTR binary_file);
BOOLEAN IsInIpTable(unsigned int ip_val);
void DeInitializeIpTable();

enum FILTER_STATE {
	FILTER_STATE_NONE,
	FILTER_STATE_IPLOG,
	FILTER_STATE_ALL
};

extern enum FILTER_STATE filter_state;

#define WEST_CHAMBER_FILTER_SET CTL_CODE(FILE_DEVICE_NETWORK,0,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define WEST_CHAMBER_FILTER_GET CTL_CODE(FILE_DEVICE_NETWORK,1,METHOD_BUFFERED,FILE_ANY_ACCESS)
// WCWController.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

enum FILTER_STATE {
	FILTER_STATE_NONE,
	FILTER_STATE_IPLOG,
	FILTER_STATE_ALL
};

#define WEST_CHAMBER_FILTER_SET CTL_CODE(FILE_DEVICE_NETWORK,0,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define WEST_CHAMBER_FILTER_GET CTL_CODE(FILE_DEVICE_NETWORK,1,METHOD_BUFFERED,FILE_ANY_ACCESS)

int _tmain(int argc, _TCHAR* argv[])
{
	printf("westchamber windows controller\n");
	HANDLE handle = CreateFile(L"\\\\.\\WestChamber",GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(handle == INVALID_HANDLE_VALUE )	{
		printf("Cannot get device!\n");
		return -1;
	}
	printf("Current status(%d=NONE,%d=IPLOG,%d=ALL)   :   ",FILTER_STATE_NONE,FILTER_STATE_IPLOG,FILTER_STATE_ALL);
	UINT status;
	DWORD recv;
	DeviceIoControl(handle,WEST_CHAMBER_FILTER_GET,NULL,0,&status,4,&recv,NULL);
	printf("%d\n",status);
	printf("New status=");
	ULONG nstatus;
	scanf("%d",&nstatus);
	DeviceIoControl(handle,WEST_CHAMBER_FILTER_SET,&nstatus,4,NULL,0,&recv,NULL);
	printf("...done\n");
	CloseHandle(handle);
	return 0;
}


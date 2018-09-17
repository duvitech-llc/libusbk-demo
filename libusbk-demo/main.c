#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "libusbk.h"

#define USBD_VID     0x0483
#define USBD_PID_HS     0x2E18

// This function is called by the LstK_Enumerate function for each
// device until it returns FALSE.
static BOOL KUSB_API ShowDevicesCB(KLST_HANDLE DeviceList,
	KLST_DEVINFO_HANDLE deviceInfo,
	PVOID MyContext)
{
	// print some information about the device.
	printf("%04X:%04X (%s): %s - %s\n",
		deviceInfo->Common.Vid,
		deviceInfo->Common.Pid,
		deviceInfo->Common.InstanceID,
		deviceInfo->DeviceDesc,
		deviceInfo->Mfg);

	// If this function returns FALSE then enumeration ceases.
	return TRUE;
}

int main(int argc, char* argv[]) {

	printf("LIBUSBK Demo\n");
	KLST_HANDLE deviceList = NULL;
	KLST_DEVINFO_HANDLE deviceInfo = NULL;
	DWORD errorCode = ERROR_SUCCESS;
	ULONG count = 0;
	/*
	Initialize a new LstK (device list) handle.
	The list is polulated with all usb devices libusbK can access.
	*/
	if (!LstK_Init(&deviceList, 0))
	{
		errorCode = GetLastError();
		printf("An error occured getting the device list. errorCode=%08Xh\n", errorCode);
		return errorCode;
	}

	// Get the number of devices contained in the device list.
	LstK_Count(deviceList, &count);
	if (!count)
	{
		printf("No devices connected.\n");

		// Always free the device list if LstK_Init returns TRUE
		LstK_Free(deviceList);

		return ERROR_DEVICE_NOT_CONNECTED;
	}

	/*
	There are three (3) ways to search the device list:
	- #1 LstK_FindByVidPid
	- #2 LstK_MoveReset, LstK_MoveNext, and LstK_Current
	- #3 LstK_Enumerate
	*/

	// #1
	// Simple means of locating the fist device matching a vid/pid.
	//
	if (LstK_FindByVidPid(deviceList, USBD_VID, USBD_PID_HS, &deviceInfo))
		printf("LstK_FindByVidPid: Example device connected!\n");
	else
		printf("Example device not found.\n");


	if (deviceInfo)
	{
		BOOL success = LibK_SetContext(deviceInfo, KLIB_HANDLE_TYPE_LSTINFOK, 1);
		if (success)
		{
			UINT_PTR myValue = LibK_GetContext(deviceInfo, KLIB_HANDLE_TYPE_LSTINFOK);
			printf("MyContextValue = %u\n", myValue);
		}

	}

	// #2
	// Enumerates the device list using it's internal "current" position.
	//
	// Reset the device list position.
	LstK_MoveReset(deviceList);
	//
	errorCode = ERROR_NO_MATCH;
	//
	// Call LstK_MoveNext after a LstK_MoveReset to advance to the first
	// element.
	while (LstK_MoveNext(deviceList, &deviceInfo))
	{
		if (deviceInfo->Common.Vid == USBD_VID &&
			deviceInfo->Common.Pid == USBD_PID_HS)
		{
			errorCode = ERROR_SUCCESS;
			break;
		}
	}
	//
	// Report the connection state
	if (deviceInfo)
		printf("LstK_MoveNext: Example device connected!\n");
	else
		printf("Example device not found.\n");

	// #3
	// Enumerates the device list using the user supplied callback
	// function, ShowDevicesCB(). LstK_Enumerate calls this function for
	// each device info element until ShowDevicesCB(0 returns FALSE.
	//
	// Show all devices using the enumerate function.
	LstK_Enumerate(deviceList, ShowDevicesCB, NULL);

	// Free the device list
	LstK_Free(deviceList);

	// return the win32 error code.
	return errorCode;

}
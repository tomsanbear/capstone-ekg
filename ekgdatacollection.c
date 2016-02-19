/*	Following code is for data collection from the GoLink Device, custom 
 *	parameters may be set in the argument of the code, to be implemented.
 *
 *	Author: Thomas Santerre
 *	Date: February 17 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <memory.h>
#include <sys/time.h>
#include "GoIO_DLL_interface.h"

#define MAX_NUM_MEASUREMENTS 2000
#define GOIO_MAXSIZE_DEVICE_NAME 100


char *deviceDesc[8] = {"?", "?", "Go! Temp", "Go! Link", "Go! Motion", "?", "?", "Mini GC"};
bool GetAvailableDeviceName(char *deviceName, gtype_int32 nameLength, gtype_int32 *pVendorId, gtype_int32 *pProductId);
static void OSSleep(unsigned long msToSleep);

int main(int argc, char* argv[]){
	char deviceName[GOIO_MAXSIZE_DEVICE_NAME];
	gtype_int32 vendorId;
	gtype_int32 productId;
	gtype_int32 rawMeasurements[MAX_NUM_MEASUREMENTS];
	gtype_real64 volts[MAX_NUM_MEASUREMENTS];
	gtype_real64 calbMeasurements[MAX_NUM_MEASUREMENTS];
	gtype_int32 numMeasurements, i;
	gtype_real64 averageCalbMeasurement;
	gtype_real64 measureperiod = 0.004;
	char units[20];
	gtype_uint16 MajorVersion;
	gtype_uint16 MinorVersion;

	//	Initialize the library
	GoIO_Init();
	GoIO_GetDLLVersion(&MajorVersion, &MinorVersion);
	printf("Using library version %d.%d \n", MajorVersion, MinorVersion);
	bool bFoundDevice = GetAvailableDeviceName(deviceName, GOIO_MAXSIZE_DEVICE_NAME, &vendorId, &productId);
	if(!bFoundDevice)
		printf("No sensor Detected\n");
	else{
		GOIO_SENSOR_HANDLE hDevice = GoIO_Sensor_Open(deviceName, vendorId, productId, 0);
		if(hDevice != NULL){
			printf("Sensor Opened Succesfully \n");
			unsigned char charId;
			//Preparing to collect data from the device
			GoIO_Sensor_SetMeasurementPeriod(hDevice,measureperiod,SKIP_TIMEOUT_MS_DEFAULT);
			GoIO_Sensor_SendCmdAndGetResponse(hDevice, SKIP_CMD_ID_START_MEASUREMENTS,NULL,0,NULL,NULL,SKIP_TIMEOUT_MS_DEFAULT);
			OSSleep(5000);
			numMeasurements = GoIO_Sensor_ReadRawMeasurements(hDevice, rawMeasurements, MAX_NUM_MEASUREMENTS);
			printf("Recorded %d measurements \n", numMeasurements);
			// Begin outputting data to file to be processed further
			FILE * fp;
			fp = fopen("ekgdata.txt","w+");
			for (i = 0; i < numMeasurements; i++){
				volts[i]= GoIO_Sensor_ConvertToVoltage(hDevice, rawMeasurements[i]);
				calbMeasurements[i] = GoIO_Sensor_CalibrateData(hDevice,volts[i]);
				fprintf(fp,"%f\n",calbMeasurements[i]);
			}
			fclose(fp);
			printf("Closing Sensor \n");
			GoIO_Sensor_Close(hDevice);
		}
		else{
			printf("Unable to open Sensor, exiting.");
			return 0;
		}
	GoIO_Uninit();
	}
}

bool GetAvailableDeviceName(char*deviceName, gtype_int32 nameLength, gtype_int32 *pVendorId, gtype_int32 *pProductId){
	bool bFoundDevice = false;
	deviceName[0] = 0;
	int numSkips = GoIO_UpdateListOfAvailableDevices(VERNIER_DEFAULT_VENDOR_ID,SKIP_DEFAULT_PRODUCT_ID);
	int numJonahs = GoIO_UpdateListOfAvailableDevices(VERNIER_DEFAULT_VENDOR_ID, USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID);
	int numCyclopses = GoIO_UpdateListOfAvailableDevices(VERNIER_DEFAULT_VENDOR_ID, CYCLOPS_DEFAULT_PRODUCT_ID);
	int numMiniGCs = GoIO_UpdateListOfAvailableDevices(VERNIER_DEFAULT_VENDOR_ID, MINI_GC_DEFAULT_PRODUCT_ID);
	if (numSkips > 0)
	{
		GoIO_GetNthAvailableDeviceName(deviceName, nameLength, VERNIER_DEFAULT_VENDOR_ID, SKIP_DEFAULT_PRODUCT_ID, 0);
		*pVendorId = VERNIER_DEFAULT_VENDOR_ID;
		*pProductId = SKIP_DEFAULT_PRODUCT_ID;
		bFoundDevice = true;
	}
	else if (numJonahs > 0)
	{
		GoIO_GetNthAvailableDeviceName(deviceName, nameLength, VERNIER_DEFAULT_VENDOR_ID, USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID, 0);
		*pVendorId = VERNIER_DEFAULT_VENDOR_ID;
		*pProductId = USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID;
		bFoundDevice = true;
	}
	else if (numCyclopses > 0)
	{
		GoIO_GetNthAvailableDeviceName(deviceName, nameLength, VERNIER_DEFAULT_VENDOR_ID, CYCLOPS_DEFAULT_PRODUCT_ID, 0);
		*pVendorId = VERNIER_DEFAULT_VENDOR_ID;
		*pProductId = CYCLOPS_DEFAULT_PRODUCT_ID;
		bFoundDevice = true;
	}
	else if (numMiniGCs > 0)
	{
		GoIO_GetNthAvailableDeviceName(deviceName, nameLength, VERNIER_DEFAULT_VENDOR_ID, MINI_GC_DEFAULT_PRODUCT_ID, 0);
		*pVendorId = VERNIER_DEFAULT_VENDOR_ID;
		*pProductId = MINI_GC_DEFAULT_PRODUCT_ID;
		bFoundDevice = true;
	}

	return bFoundDevice;
}

void OSSleep(unsigned long msToSleep){
	struct timeval tv;
	unsigned long usToSleep = msToSleep*1000;
	tv.tv_sec = usToSleep/1000000;
	tv.tv_usec = usToSleep  % 1000000;
	select(0, NULL, NULL, NULL, &tv);
}

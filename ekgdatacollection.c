/*	Following code is for data collection from the GoLink Device, custom 
 *	parameters may be set in the argument of the code, to be implemented.
 *
 *	Author: Thomas Santerre
 *	Date: February 17 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <sys/time.h>
#include "GoIO_DLL_interface.h"

#define MAX_NUM_MEASUREMENTS 2000

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
			GoIOSensor_SendCmdAndGetResponse(hDevice, SKIP_CMD_ID_START_MEASUREMENTS,NULL,0,NULL,NULL,SKIP_TIMEOUT_MS_DEFAULT);
			numMeasurements = GoIO_Sensor_ReadRawMeasurements(hDevice, rawMeasurements, MAX_NUM_MEASUREMENTS);
			printf("Recorded %d measurements \n", numMeasurements);
			// Begin outputting data to file to be processed further
			FILE * fp;
			fp = fopen("ekgdata.txt","w+");
			for (i = 0; i < numMeasurements; i++){
				volts[i]= GoIO_Sensor_ConvertToVoltage(hDevice, rawMeasurements,MAX_NUM_MEASUREMENTS);
				calbMeasurements[i] = GoIO_Sensor_CalibrateData(hDevice,volts[i]);
				fprintf(fp,"%f\n",calbMeasurements[i]);
			}
			fclose(fp);
		}
		else{
			printf("Unable to open Sensor, exiting.");
			return 0;
		}
	}
}

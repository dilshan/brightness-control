/* Monitor brightness controller – Demo application.

   Copyright (c) 2016 Dilshan R Jayakody. [jayakody2000lk at gmail dot com]
   
   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to
   deal in the Software without restriction, including without limitation the
   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
   sell copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
   
   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.
   
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
   IN THE SOFTWARE. 
*/

#include "stdafx.h"
#include "hidapi.h"
#include "brightness-control.h"

/**
* Application main entry point.
*/
INT _tmain(INT argc, _TCHAR* argv[])
{	
	// Shared data resources.
	USHORT sensorValue = 0;
	BOOL sharedLock = FALSE;
		
	workerPipeline.totalPhysicalMonitors = 0;
	workerPipeline.totalPhysicalMonitorsStruct = NULL;
	workerPipeline.deviceHandle = NULL;
	workerPipeline.isActive = &sharedLock;
	workerPipeline.sensorValue = &sensorValue;
	
	// Initialize HID-USB library.
	if (hid_init())
	{
		printf("USB library initialization fail.");
		return 1;
	}

	if(IsDeviceAvailable() != NO_ERROR)
	{
		printf("Sensor device is not attached to the system.");
		return 1;
	}

	// Initialize Display device(s).
	if(InitPhysicalMonitors() != NO_ERROR)
	{
		printf("Display device initialization fail.");
		return 1;
	}

	// Create USB HID communication channel to work with sensor.
	workerPipeline.deviceHandle = hid_open(DEVICE_VENDOR_ID, DEVICE_PRODUCT_ID, NULL);
	if (!workerPipeline.deviceHandle) 
	{
		printf("Unable to open USB sensor device.");
		return 1;
	}

	hid_set_nonblocking(workerPipeline.deviceHandle, 1);
	(*workerPipeline.isActive) = TRUE;

	sensorWorkerHandle = CreateThread(NULL, 0, SensorWorker, &workerPipeline, 0, NULL);
	if(sensorWorkerHandle == NULL)
	{
		printf("Unable to create sensor monitoring thread.");
		return 1;
	}

	// Create display device control thread.
	displayWorkerHandle = CreateThread(NULL, 0, DisplayWorker, &workerPipeline, 0, NULL);
	if(sensorWorkerHandle == NULL)
	{
		printf("Unable to create display control thread.");
		return 1;
	}

	// Wait for thread exit or completion.
	waitThreadList[0] = sensorWorkerHandle;
	waitThreadList[1] = displayWorkerHandle;
	WaitForMultipleObjects(2, waitThreadList, TRUE, INFINITE);
	
	// Clean up all allocated HID API resources.
	hid_close(workerPipeline.deviceHandle);
	workerPipeline.deviceHandle = NULL;
	hid_exit();

	// Cleanup any available data structures.
	if(workerPipeline.totalPhysicalMonitorsStruct != NULL)
	{
		DestroyPhysicalMonitors(workerPipeline.totalPhysicalMonitors, workerPipeline.totalPhysicalMonitorsStruct); 
		free(workerPipeline.totalPhysicalMonitorsStruct); 
		workerPipeline.totalPhysicalMonitors = 0;
		workerPipeline.totalPhysicalMonitorsStruct = NULL;
	}
	return 0;
}

/**
* Check sensor device is attached to the system.
*/
UINT IsDeviceAvailable()
{
	struct hid_device_info *probeDevices = NULL;
	UINT returnCode = ERROR_DEV_NOT_EXIST;

	probeDevices = hid_enumerate(DEVICE_VENDOR_ID, DEVICE_PRODUCT_ID);
	if(probeDevices != NULL)
	{
		// Devices with specified VID and PID are available.
		returnCode = ERROR_SUCCESS;
		hid_free_enumeration(probeDevices);
	}

	return returnCode;
}

/**
* Request last captured data-set from sensor device.
*/
INT RequestDataFromSensor(hid_device* deviceHandle)
{
	UCHAR requestBuffer[4];

	requestBuffer[0] = 0x00;
	requestBuffer[1] = INPUT_SIGNATURE;
	requestBuffer[2] = USB_CMD_MANUAL_TX;
	requestBuffer[3] = BUFFER_END;

	return hid_write(deviceHandle, requestBuffer, sizeof(requestBuffer));
}

/**
* Initialize data structures related to physical display devices.
*/
UINT InitPhysicalMonitors()
{
	HMONITOR monitorHandle = NULL;
	DWORD physicalMonitors; 
	LPPHYSICAL_MONITOR physicalMonitorStruct = NULL;

	 monitorHandle = MonitorFromWindow(NULL , MONITOR_DEFAULTTOPRIMARY); 
	 if(monitorHandle == NULL)
	 {
		 return ERROR_NOT_SUPPORTED;
	 }

	 if(GetNumberOfPhysicalMonitorsFromHMONITOR(monitorHandle, &physicalMonitors))
	 {
		 physicalMonitorStruct = (LPPHYSICAL_MONITOR)malloc(physicalMonitors * sizeof(PHYSICAL_MONITOR));
		 if(physicalMonitorStruct == NULL)
		 {
			return ERROR_NOT_SUPPORTED;
		 }

		 // Cleanup any available data structures.
		 if(workerPipeline.totalPhysicalMonitorsStruct != NULL)
		 {
			 DestroyPhysicalMonitors(workerPipeline.totalPhysicalMonitors, workerPipeline.totalPhysicalMonitorsStruct); 
			 free(workerPipeline.totalPhysicalMonitorsStruct); 
			 workerPipeline.totalPhysicalMonitors = 0;
			 workerPipeline.totalPhysicalMonitorsStruct = NULL;
		 }

		 workerPipeline.totalPhysicalMonitorsStruct = physicalMonitorStruct;
		 workerPipeline.totalPhysicalMonitors = physicalMonitors;

		 if(GetPhysicalMonitorsFromHMONITOR(monitorHandle, physicalMonitors, physicalMonitorStruct))
		 {
			 return ERROR_SUCCESS;
		 }
		 else
		 {
			 // Cleanup allocated structures.
			 free(workerPipeline.totalPhysicalMonitorsStruct);
			 workerPipeline.totalPhysicalMonitors = 0;
			 workerPipeline.totalPhysicalMonitorsStruct = NULL;

			 return ERROR_NOT_SUPPORTED;
		 }
	 }
	 else
	 {
		 // Physical monitor data retrieval is fail.
		 return ERROR_NOT_SUPPORTED;
	 }
}

/**
* Thread routine for sensor communication.
*/
DWORD WINAPI SensorWorker(LPVOID thParam)
{
	ThreadData pipeLine = *((ThreadData*)thParam);
	INT readStatus = 0;
	UCHAR dataBuffer[4];

	// Trigget sensor to submit last captured value.
	if(RequestDataFromSensor(pipeLine.deviceHandle) < 0)
	{
		printf("Unable to write data to the sensor device.");
		return 0;
	}

	Sleep(10);

	// Data read loop.
	while(readStatus == 0)
	{
		// Check for termination signal.
		if(!(*pipeLine.isActive))
		{
			break;
		}
		
		// Send data read request to HID library.
		memset(dataBuffer, 0, sizeof(dataBuffer));
		readStatus = hid_read(pipeLine.deviceHandle, dataBuffer, sizeof(dataBuffer));

		if(readStatus < 0)
		{
			printf("Unable to read from sensor device.");
			(*pipeLine.isActive) = FALSE;
			break;
		}
		else if(readStatus > 0)
		{
			if(dataBuffer[0] == OUTPUT_SIGNATURE)
			{
				readStatus = 0;
				(*pipeLine.sensorValue) = (ULONG)(dataBuffer[1] + (dataBuffer[2] << 8));
			}
			else
			{
				printf("Sensor data is invalid or not compatible.");
				(*pipeLine.isActive) = FALSE;
				break;
			}
		}

		Sleep(100);
	}
	
	return 0;
}

/**
* Thread routine for display device base operations.
*/
DWORD WINAPI DisplayWorker(LPVOID thParam)
{
	ThreadData pipeLine = *((ThreadData*)thParam);
	USHORT lastSensorVal = 60, currentVal;
	INT monitorBrightness;
	DWORD monitorPos;

	while((*pipeLine.isActive))
	{		
		if(lastSensorVal != (*pipeLine.sensorValue))
		{
			currentVal = lastSensorVal;

			// Perform linear change of brightness. 
			while(currentVal != (*pipeLine.sensorValue))
			{
				currentVal = currentVal + (((*pipeLine.sensorValue) > currentVal) ? 1 : -1);
				monitorBrightness = TransformToBrightness(currentVal);

				// Update new brightness value on all available monitor devices.
				for(monitorPos = 0; monitorPos < pipeLine.totalPhysicalMonitors; monitorPos++)
				{
					SetMonitorBrightness(pipeLine.totalPhysicalMonitorsStruct[monitorPos].hPhysicalMonitor, monitorBrightness);
				}
			}

			lastSensorVal = currentVal;
		}

		Sleep(50);
	}

	return 0;
}

/**
* Transform USB sensor value to monitor brightness.
*/ 
INT TransformToBrightness(USHORT usbVal)
{
	// NOTE: This is generic transformation for generally available 
	// monitors. For some display panels this function may need some change.

	FLOAT sensorRatio = ((FLOAT)usbVal)/SENSOR_MIN_BRIGHT;
	return int(MONITOR_MAX_BRIGHT - (sensorRatio * MONITOR_MAX_BRIGHT));
}
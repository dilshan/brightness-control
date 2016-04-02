/* Monitor brightness controller – Sensor data logger application.

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
#include "sensor-logger.h"

int _tmain(int argc, _TCHAR* argv[])
{
	INT readStatus = 0;
	hid_device* deviceHandle = NULL;
	UCHAR dataBuffer[4];
	
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

	// Open USB sensor device.
	deviceHandle = hid_open(DEVICE_VENDOR_ID, DEVICE_PRODUCT_ID, NULL);
	if (!deviceHandle) 
	{
		printf("Unable to open USB sensor device.");
		return 1;
	}

	hid_set_nonblocking(deviceHandle, 1);

	// Trigget sensor to submit last captured value.
	if(RequestDataFromSensor(deviceHandle) < 0)
	{
		printf("Unable to write data to the sensor device.");
		return 1;
	}
	
	// Data read loop.
	while(readStatus == 0)
	{
		memset(dataBuffer, 0, sizeof(dataBuffer));
		readStatus = hid_read(deviceHandle, dataBuffer, sizeof(dataBuffer));

		if(readStatus < 0)
		{
			printf("Unable to read from sensor device.");
			break;
		}
		else if(readStatus > 0)
		{
			if(dataBuffer[0] == OUTPUT_SIGNATURE)
			{
				readStatus = 0;
				printf(":%d\n", (ULONG)(dataBuffer[1] + (dataBuffer[2] << 8)));
			}
			else
			{
				printf("Sensor data is invalid or not compatible.");
				break;
			}
		}

		Sleep(250);
	}

	// Clean up all allocated HID API resources.
	hid_close(deviceHandle);
	deviceHandle = NULL;

	hid_exit();

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
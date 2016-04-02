/* Monitor brightness controller – PIC18F2550 firmware.

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

#include "usb-bright.h"

// USB I/O buffers.
unsigned char usbReadBuffer[64] absolute 0x500;
unsigned char usbWriteBuffer[64] absolute 0x540;

// Buffer for mean value.
unsigned int filterTap[FILTER_TAP_COUNT];

// Current and last value send to host terminal.
unsigned int lastFilterVal, currentFilterVal;

// Number of background scan cycles performed on ADC.
unsigned char scanCount;

// Flag for manual data transmission.
unsigned char isSendData;

/**
* Push new value to filter buffer.
*/
void PushToFilter(unsigned int newVal)
{
  unsigned char pos = (FILTER_TAP_COUNT - 1);
  
  while(pos > 0)
  {
    filterTap[pos] = filterTap[pos - 1];
    pos--;
  }
  
  filterTap[0] = newVal;
}

/**
* Get value from mean filter.
*/
unsigned int GetFilterValue()
{
  unsigned char pos = 0;
  unsigned int outputVal = 0;
  
  while(pos < FILTER_TAP_COUNT)
  {
    outputVal += (filterTap[pos] / (FILTER_TAP_COUNT + 1));
    pos++;
  }
  
  return outputVal;
}

/**
* Reset values of filter buffer.
*/
void resetFilter()
{
  unsigned char pos = 0;
  while(pos < FILTER_TAP_COUNT)
  {
    filterTap[pos] = 0;
    pos++;
  }
}

/**
* MCU ISR.
*/
void interrupt()
{
  USB_Interrupt_Proc();
}

/**
* Application entry-point.
*/
void main() 
{
  lastFilterVal = 0xFFFF;
  isSendData = 0x00;
  
  // Initialize MCU peripherals.
  CMCON = 0x07;
  ADC_Init();
  HID_Enable(&usbReadBuffer, &usbWriteBuffer);
  
  resetFilter();
  Delay_ms(1000);
  
  while(1)
  {
    scanCount = 0;
    
    // Scan ADC background scan.
    while(scanCount < 100)
    {
      PushToFilter(ADC_Read(0));
      scanCount++;
      Delay_ms(10);
    }
    
    // Get calculated value from filter buffer.
    currentFilterVal = GetFilterValue();
    
    // Check for manual data transmission requests.
    if(HID_Read() != 0)
    {
      if((usbReadBuffer[0] == INPUT_SIGNATURE) && (usbReadBuffer[1] == USB_CMD_MANUAL_TX) && (usbReadBuffer[2] == BUFFER_END))
      {
        isSendData = 0xFF;
      }
    }
    
    // Send new data to host, if request is available or change is detected.
    if((lastFilterVal != currentFilterVal) || (isSendData != 0x00))
    {
      usbWriteBuffer[0] = OUTPUT_SIGNATURE;
      usbWriteBuffer[1] = currentFilterVal & 0xFF;
      usbWriteBuffer[2] = ((currentFilterVal >> 8) & 0xFF);
      usbWriteBuffer[3] = BUFFER_END;
      
      while(!HID_Write(&usbWriteBuffer, USB_IO_REPORT_SIZE));
      lastFilterVal = currentFilterVal;
      isSendData = 0x00;
    }
    
    Delay_ms(100);
  }
}
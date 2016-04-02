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

const unsigned int USB_VENDOR_ID = DEVICE_VENDOR_ID;
const unsigned int USB_PRODUCT_ID = DEVICE_PRODUCT_ID;
const char USB_SELF_POWER = 0x80;
const char USB_MAX_POWER = 50;
const char HID_INPUT_REPORT_BYTES = USB_IO_REPORT_SIZE;
const char HID_OUTPUT_REPORT_BYTES = USB_IO_REPORT_SIZE;
const char USB_TRANSFER_TYPE = 0x03;
const char EP_IN_INTERVAL = 1;
const char EP_OUT_INTERVAL = 1;

const char USB_INTERRUPT = 1;
const char USB_HID_EP = 1;
const char USB_HID_RPT_SIZE = 33;

// Device descriptor.
const struct 
{
    char bLength;
    char bDescriptorType;
    unsigned int bcdUSB;
    char bDeviceClass;
    char bDeviceSubClass;
    char bDeviceProtocol;
    char bMaxPacketSize0;
    unsigned int idVendor;
    unsigned int idProduct;
    unsigned int bcdDevice;
    char iManufacturer;
    char iProduct;
    char iSerialNumber;
    char bNumConfigurations;
} 

device_dsc = 
{
      0x12,
      0x01,
      0x0200,
      0x00,
      0x00,
      0x00,
      8,
      USB_VENDOR_ID,
      USB_PRODUCT_ID,
      0x0001,
      0x01,
      0x02,
      0x00,
      0x01
};

// Configuration 1 descriptor
const char configDescriptor1[] =
{
    // Configuration descriptor
    0x09,
    0x02,
    0x29,0x00,
    1,
    1,
    0,
    USB_SELF_POWER,
    USB_MAX_POWER,

    // Interface descriptor
    0x09,
    0x04,
    0,
    0,
    2,
    0x03,
    0,
    0,
    0,

    // HID class-specific descriptor
    0x09,
    0x21,
    0x01,0x01,
    0x00,
    1,
    0x22,
    USB_HID_RPT_SIZE,0x00,

    // Endpoint descriptor.
    0x07,
    0x05,
    USB_HID_EP | 0x80,
    USB_TRANSFER_TYPE,
    0x40,0x00,
    EP_IN_INTERVAL,

    // Endpoint descriptor.
    0x07,
    0x05,
    USB_HID_EP,
    USB_TRANSFER_TYPE,
    0x40,0x00,
    EP_OUT_INTERVAL
};

const struct 
{
  char report[USB_HID_RPT_SIZE];
}

hid_rpt_desc =
{
     {0x06, 0x00, 0xFF,
      0x09, 0x01,
      0xA1, 0x01,

      // Input report.
      0x19, 0x01,
      0x29, 0x40,
      0x15, 0x00,
      0x26, 0xFF, 0x00,
      0x75, 0x08,
      0x95, HID_INPUT_REPORT_BYTES,
      0x81, 0x02,
      
      // Output report.
      0x19, 0x01,
      0x29, 0x40,
      0x75, 0x08,
      0x95, HID_OUTPUT_REPORT_BYTES,
      0x91, 0x02,
      0xC0}
};

// Language code string descriptor.
const struct 
{
  char bLength;
  char bDscType;
  unsigned int string[1];
} 

strd1 = 
{
  4,
  0x03,
  {0x0409}
};

// Manufacturer string descriptor.
const struct
{
  char bLength;
  char bDscType;
  unsigned int string[18];
}

strd2 =
{
    38,
    0x03,
    {'D','i','l','s','h','a','n',' ','R',' ','J','a','y','a','k','o','d','y'}
};

// Product string descriptor.
const struct
{
  char bLength;
  char bDscType;
  unsigned int string[25];
}

strd3 =
{
    52,
    0x03,
    {'U','S','B',' ','B','r','i','g','h','t','n','e','s','s',' ','C','o','n','t','r','o','l','l','e','r'}
};

// Array of configuration descriptors.
const char* USB_config_dsc_ptr[1];

// Array of string descriptors.
const char* USB_string_dsc_ptr[3];

void USB_Init_Desc()
{
  USB_config_dsc_ptr[0] = &configDescriptor1;
  USB_string_dsc_ptr[0] = (const char*)&strd1;
  USB_string_dsc_ptr[1] = (const char*)&strd2;
  USB_string_dsc_ptr[2] = (const char*)&strd3;
}
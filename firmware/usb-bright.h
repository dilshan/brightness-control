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

// Number of taps in smoothing filter.
#define FILTER_TAP_COUNT 63

// USB output buffer signature.
#define OUTPUT_SIGNATURE 0x12

// USB input buffer signature.
#define INPUT_SIGNATURE 0x11

// USB I/O buffer end mark.
#define BUFFER_END 0x00

// Host command to request data from sensor module.
#define USB_CMD_MANUAL_TX 0x01

// USB vendor id for device.
#define DEVICE_VENDOR_ID 0x1984

// USB product id for device.
#define DEVICE_PRODUCT_ID 0x0032

// Read and write report size.
#define USB_IO_REPORT_SIZE 4
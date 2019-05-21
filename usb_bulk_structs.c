//*****************************************************************************
//
// usb_bulk_structs.c - Data structures defining this bulk USB device.
//
// Copyright (c) 2012 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
//
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
//
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
//
// This is part of revision 9453 of the EK-LM4F120XL Firmware Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "driverlib/usb.h"
#include "usblib/usblib.h"
#include "usblib/usb-ids.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdbulk.h"
#include "usb_bulk_structs.h"

//*****************************************************************************
// The languages supported by this device.
//*****************************************************************************
const uint8_t lang_descriptor[] = {
    4,                         // bLength
    USB_DTYPE_STRING,          // bDescriptorType
    USBShort(USB_LANG_EN_US)   // wLANGID[0]
};

//*****************************************************************************
// The manufacturer string.
//*****************************************************************************
const uint8_t manufacturer_string[] = {
    (17 + 1) * 2,             // bLength
    USB_DTYPE_STRING,         // bDescriptorType
    'T', 0,                   // string
    'e', 0,
    'x', 0,
    'a', 0,
    's', 0,
    ' ', 0,
    'I', 0,
    'n', 0,
    's', 0,
    't', 0,
    'r', 0,
    'u', 0,
    'm', 0,
    'e', 0,
    'n', 0,
    't', 0,
    's', 0,
};

//*****************************************************************************
// The product string.
//*****************************************************************************
const uint8_t product_string[] = {
    (19 + 1) * 2,
    USB_DTYPE_STRING,
    'G', 0,
    'e', 0,
    'n', 0,
    'e', 0,
    'r', 0,
    'i', 0,
    'c', 0,
    ' ', 0,
    'B', 0,
    'u', 0,
    'l', 0,
    'k', 0,
    ' ', 0,
    'D', 0,
    'e', 0,
    'v', 0,
    'i', 0,
    'c', 0,
    'e', 0
};

//*****************************************************************************
// The serial number string.
//*****************************************************************************
const uint8_t serial_num_string[] = {
    (8 + 1) * 2,
    USB_DTYPE_STRING,
    '1', 0,
    '2', 0,
    '3', 0,
    '4', 0,
    '5', 0,
    '6', 0,
    '7', 0,
    '8', 0
};

//*****************************************************************************
// The data interface description string.
//*****************************************************************************
const uint8_t data_interface_string[] = {
    (19 + 1) * 2,
    USB_DTYPE_STRING,
    'B', 0,
    'u', 0,
    'l', 0,
    'k', 0,
    ' ', 0,
    'D', 0,
    'a', 0,
    't', 0,
    'a', 0,
    ' ', 0,
    'I', 0,
    'n', 0,
    't', 0,
    'e', 0,
    'r', 0,
    'f', 0,
    'a', 0,
    'c', 0,
    'e', 0
};

//*****************************************************************************
// The configuration description string.
//*****************************************************************************
const uint8_t config_string[] = {
    (23 + 1) * 2,
    USB_DTYPE_STRING,
    'B', 0,
    'u', 0,
    'l', 0,
    'k', 0,
    ' ', 0,
    'D', 0,
    'a', 0,
    't', 0,
    'a', 0,
    ' ', 0,
    'C', 0,
    'o', 0,
    'n', 0,
    'f', 0,
    'i', 0,
    'g', 0,
    'u', 0,
    'r', 0,
    'a', 0,
    't', 0,
    'i', 0,
    'o', 0,
    'n', 0
};

//*****************************************************************************
// The descriptor string table.
//*****************************************************************************
const uint8_t * const string_descriptors[] = {
    lang_descriptor,
    manufacturer_string,
    product_string,
    serial_num_string,
    data_interface_string,
    config_string
};

//*****************************************************************************
// The bulk device initialization and customization structures. In this case,
// we are using USBBuffers between the bulk device class driver and the
// application code. The function pointers and callback data values are set
// to insert a buffer in each of the data channels, transmit and receive.
//
// With the buffer in place, the bulk channel callback is set to the relevant
// channel function and the callback data is set to point to the channel
// instance data. The buffer, in turn, has its callback set to the application
// function and the callback data set to our bulk instance structure.
//*****************************************************************************
extern const tUSBBuffer g_tx_cb_buf;
extern const tUSBBuffer g_rx_cb_buf;

tUSBDBulkDevice g_bulk_device = {
    USB_VID_TI_1CBE,                               // vendor ID
    USB_PID_BULK,                                  // product ID
    500,                                           // max power (mA)
    USB_CONF_ATTR_SELF_PWR,                        // power attrs
    USBBufferEventCallback,                        // rx callback
    (void *)&g_rx_cb_buf,                          // rx callback buffer
    USBBufferEventCallback,                        // tx callback
    (void *)&g_tx_cb_buf,                          // tx callback buffer
    string_descriptors,                            // desriptors
    sizeof(string_descriptors) / sizeof(uint8_t *) // number of descriptors
};

//*****************************************************************************
// Receive buffer (from the USB perspective).
//*****************************************************************************
uint8_t g_usb_rx_buf[BULK_BUFFER_SIZE];
uint8_t rx_workspace_buf[USB_BUFFER_WORKSPACE_SIZE];
const tUSBBuffer g_rx_cb_buf = {
    false,                           // This is a receive buffer.
    RxHandler,                       // pfnCallback
    (void *)&g_bulk_device,          // Callback data is our device pointer.
    USBDBulkPacketRead,              // pfnTransfer
    USBDBulkRxPacketAvailable,       // pfnAvailable
    (void *)&g_bulk_device,          // pvHandle
    g_usb_rx_buf,                    // pcBuffer
    BULK_BUFFER_SIZE,                // ulBufferSize
    rx_workspace_buf                 // pvWorkspace
};

//*****************************************************************************
// Transmit buffer (from the USB perspective).
//*****************************************************************************
uint8_t g_usb_tx_buf[BULK_BUFFER_SIZE];
uint8_t tx_workspace_buf[USB_BUFFER_WORKSPACE_SIZE];
const tUSBBuffer g_tx_cb_buf = {
    true,                            // This is a transmit buffer.
    TxHandler,                       // pfnCallback
    (void *)&g_bulk_device,          // Callback data is our device pointer.
    USBDBulkPacketWrite,             // pfnTransfer
    USBDBulkTxPacketAvailable,       // pfnAvailable
    (void *)&g_bulk_device,          // pvHandle
    g_usb_tx_buf,                    // pcBuffer
    BULK_BUFFER_SIZE,                // ulBufferSize
    tx_workspace_buf                 // pvWorkspace
};

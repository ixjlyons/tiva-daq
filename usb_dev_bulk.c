//*****************************************************************************
//
// usb_dev_bulk.c - Main routines for the generic bulk device example.
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
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "driverlib/rom.h"
#include "usblib/usblib.h"
#include "usblib/usb-ids.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdbulk.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "usb_bulk_structs.h"

// system tick rate
#define SYSTICKS_PER_SECOND 100
#define SYSTICK_PERIOD_MS   (1000 / SYSTICKS_PER_SECOND)

// flags used to pass commands from interrupt context to the main loop.
#define COMMAND_PACKET_RECEIVED 0x00000001
#define COMMAND_STATUS_UPDATE   0x00000002

// global system tick counter
volatile uint32_t g_sys_tick_count = 0;

// keep track of tx and rx counts
volatile uint32_t g_tx_count = 0;
volatile uint32_t g_rx_count = 0;

// global flag indicating that a USB configuration has been set
volatile bool g_usb_configured = false;


#ifdef DEBUG
// map all debug print calls to UARTprintf in debug builds.
#define DEBUG_PRINT UARTprintf
// error routine called if the driver library encounters an error
void __error__(char *fname, uint32_t lnum) {
    UARTprintf("Error at line %d of %s\n", lnum, fname);
    while(1) {}
}
#else
// Compile out all debug print calls in release builds.
#define DEBUG_PRINT while(0) ((int (*)(char *, ...))0)
#endif

//*****************************************************************************
// Interrupt handler for the system tick counter.
//*****************************************************************************
void SysTickIntHandler(void) {
    g_sys_tick_count++;
}

//*****************************************************************************
// Receive new data and echo it back to the host.
//
// \param device points to the instance data for the device whose data is to
// be processed.
// \param data points to the newly received data in the USB receive buffer.
// \param nbytes is the number of bytes of data available to be processed.
//
// This function is called whenever we receive a notification that data is
// available from the host. We read the data, byte-by-byte and swap the case
// of any alphabetical characters found then write it back out to be
// transmitted back to the host.
//
// \return Returns the number of bytes of data processed.
//*****************************************************************************
static uint32_t EchoNewDataToHost(tUSBDBulkDevice *device, uint8_t *data, uint32_t nbytes) {
    uint32_t idx_loop;
    uint32_t space;
    uint32_t count;
    uint32_t idx_read;
    uint32_t idx_write;
    tUSBRingBufObject tx_buf;

    // Get the current buffer information to allow us to write directly to
    // the transmit buffer (we already have enough information from the
    // parameters to access the receive buffer directly).
    USBBufferInfoGet(&g_tx_cb_buf, &tx_buf);

    // How much space is there in the transmit buffer?
    space = USBBufferSpaceAvailable(&g_tx_cb_buf);

    // How many characters can we process this time round?
    count = (space < nbytes) ? space : nbytes;
    idx_loop = count;

    // Update our receive counter.
    g_rx_count += nbytes;

    DEBUG_PRINT("Received %d bytes\n", nbytes);

    // Set up to process the characters by directly accessing the USB buffers.
    idx_read = (uint32_t)(data - g_usb_rx_buf);
    idx_write = tx_buf.ui32WriteIndex;

    /*
    while (idx_loop) {
        // convert lowercase to uppercase and vice versa
        if ((g_usb_rx_buf[idx_read] >= 'a') && (g_usb_rx_buf[idx_read] <= 'z')) {
            g_usb_tx_buf[idx_write] = (g_usb_rx_buf[idx_read] - 'a') + 'A';
        }
        else if ((g_usb_rx_buf[idx_read] >= 'A') && (g_usb_rx_buf[idx_read] <= 'Z')) {
            g_usb_tx_buf[idx_write] = (g_usb_rx_buf[idx_read] - 'Z') + 'z';
        }
        else {
            g_usb_tx_buf[idx_write] = g_usb_rx_buf[idx_read];
        }

        // Move to the next character taking care to adjust the pointer for
        // the buffer wrap if necessary.
        idx_write++;
        idx_write = (idx_write == BULK_BUFFER_SIZE) ? 0 : idx_write;
        idx_read++;
        idx_read = (idx_read == BULK_BUFFER_SIZE) ? 0 : idx_read;

        idx_loop--;
    }
    */

    float samples[] = {
        0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0,
        8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
    };
    uint32_t bufsize = sizeof(samples);
    uint8_t *ptr = (uint8_t*)&samples;
    uint8_t i;

    for (i = 0; i < bufsize; i++) {
        g_usb_tx_buf[idx_write] = ptr[i];
        idx_write++;
        idx_write = (idx_write == BULK_BUFFER_SIZE) ? 0 : idx_write;
    }

    // We've processed the data in place so now send the processed data
    // back to the host.
    USBBufferDataWritten(&g_tx_cb_buf, bufsize);

    DEBUG_PRINT("Wrote %d bytes\n", bufsize);

    // We processed as much data as we can directly from the receive buffer so
    // we need to return the number of bytes to allow the lower layer to
    // update its read pointer appropriately.
    return bufsize;
}

//*****************************************************************************
// Handles bulk driver notifications related to the transmit channel (data to
// the USB host).
//
// \param data is the client-supplied callback pointer for this channel.
// \param event identifies the event we are being notified about.
// \param msgval is an event-specific value.
// \param msgdata is an event-specific pointer.
//
// This function is called by the bulk driver to notify us of any events
// related to operation of the transmit data channel (the IN channel carrying
// data to the USB host).
//
// In this case, we don't need to do anything in response to the transmit
// event, so we just increment the transmit counter and output a debug message.
//
// \return The return value is event-specific.
//*****************************************************************************
uint32_t TxHandler(void *data, uint32_t event, uint32_t msgval, void *msgdata) {
    if (event == USB_EVENT_TX_COMPLETE) {
        g_tx_count += msgval;
    }
    DEBUG_PRINT("TX complete %d\n", msgval);
    return(0);
}

//*****************************************************************************
// Handles bulk driver notifications related to the receive channel (data from
// the USB host).
//
// \param data is the client-supplied callback pointer for this channel.
// \param event identifies the event we are being notified about.
// \param msgval is an event-specific value.
// \param msgdata is an event-specific pointer.
//
// This function is called by the bulk driver to notify us of any events
// related to operation of the receive data channel (the OUT channel carrying
// data from the USB host).
//
// \return The return value is event-specific.
//*****************************************************************************
uint32_t RxHandler(void *data, uint32_t event, uint32_t msgval, void *msgdata) {
    switch(event) {
        // We are connected to a host and communication is now possible.
        case USB_EVENT_CONNECTED: {
            g_usb_configured = true;
            UARTprintf("Host connected.\n");
            USBBufferFlush(&g_tx_cb_buf);
            USBBufferFlush(&g_rx_cb_buf);
            break;
        }

        // The host has disconnected.
        case USB_EVENT_DISCONNECTED: {
            g_usb_configured = false;
            UARTprintf("Host disconnected.\n");
            break;
        }

        // A new packet has been received.
        case USB_EVENT_RX_AVAILABLE: {
            // Get pointer to instance data from the callback data parameter
            tUSBDBulkDevice *device = (tUSBDBulkDevice *)data;
            // Read the new packet and echo it back to the host.
            return EchoNewDataToHost(device, msgdata, msgval);
        }

        // Ignore SUSPEND and RESUME for now.
        case USB_EVENT_SUSPEND:
        case USB_EVENT_RESUME:
            break;

        // Ignore all other events and return 0.
        default:
            break;
    }

    return 0;
}

void init_uart0(void) {
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    UARTStdioConfig(0, 115200, 16000000);
}

void init_led(void) {
    // Enable the GPIO port that is used for the on-board LED.
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    // Enable the GPIO pins for the LED (PF2 & PF3).
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3|GPIO_PIN_2);
}


int main(void) {
    volatile uint32_t idx_loop;
    uint32_t tx_count = 0;
    uint32_t rx_count = 0;

    // Enable lazy stacking for interrupt handlers. This allows floating-point
    // instructions to be used within interrupt handlers, but at the expense of
    // extra stack usage.
    ROM_FPULazyStackingEnable();

    // Set the clocking to run from the PLL at 50MHz
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);

    init_led();
    init_uart0();

    UARTprintf("\033[2JStellaris USB bulk device example\n");
    UARTprintf("---------------------------------\n\n");

    g_usb_configured = false;

    // Enable the GPIO peripheral used for USB, and configure the USB pins
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    ROM_GPIOPinTypeUSBAnalog(GPIO_PORTD_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    // Enable the system tick.
    ROM_SysTickPeriodSet(ROM_SysCtlClockGet() / SYSTICKS_PER_SECOND);
    ROM_SysTickIntEnable();
    ROM_SysTickEnable();

    // Tell the user what we are up to.
    UARTprintf("Configuring USB\n");

    // Initialize the transmit and receive buffers.
    USBBufferInit((tUSBBuffer *)&g_tx_cb_buf);
    USBBufferInit((tUSBBuffer *)&g_rx_cb_buf);

    // Set the USB stack mode to Device mode with VBUS monitoring.
    USBStackModeSet(0, eUSBModeForceDevice, 0);

    // Pass our device info to the USB library and place the device on the bus
    USBDBulkInit(0, (tUSBDBulkDevice*)&g_bulk_device);

    UARTprintf("Waiting for host...\n");
    while (1) {
        // See if any data has been transferred.
        if ((tx_count != g_tx_count) || (rx_count != g_rx_count)) {
            if (tx_count != g_tx_count) {
                // blink the green LED
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
                for(idx_loop = 0; idx_loop < 150000; idx_loop++) {}
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);

                tx_count = g_tx_count;
            }

            if (rx_count != g_rx_count) {
                // blink the blue LED
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
                for(idx_loop = 0; idx_loop < 150000; idx_loop++) {}
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);

                rx_count = g_rx_count;
            }

            // Update the display of bytes transferred.
            UARTprintf("\rTx: %d  Rx: %d", tx_count, rx_count);
        }
    }
}

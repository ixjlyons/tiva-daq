========
tiva-daq
========

This is a work-in-progress implementation of a USB data acquisition system
using either the Texas Instruments `Stellaris LM4F120`_ or `Tiva TM4C123G`_
low-cost ARM Cortex-M4 development board.

.. note::

   Note on Stellaris vs. Tiva: The Stellaris LM4F120 LaunchPad is obsolete and
   has been replaced by the Tiva TM4C123G LaunchPad. They are very similar and
   I am actually using a Stellaris board I've had lying around for a few years
   even though I'm using the Tiva libraries.

Building
========

I got TI's Tiva libraries from here: https://github.com/yuvadm/tiva-c. I put
the whole thing at ``~/ti/tivaware``. Then I installed ``arm-none-eabi-gcc``
using my OS package manager.

The code here started out from ``usb_dev_bulk`` example from
``tivaware/boards/ek-tm4c123gxl/``.

Assuming you've done all the above, you should be able to build the binary with
just::

    $ make

Flashing
========

I installed the ``lm4flash`` tool from here: https://github.com/utzig/lm4tools.

To be able to access the ICDI USB port as a non-root user (and looking ahead to
be able to access the primary microcontroller's USB port), I installed the
following udev rules to ``/etc/udev/rules.d/71-ti-permissions.rules``::

    # rule for the ICDI USB
    SUBSYSTEM=="usb", ATTRS{idVendor}=="1cbe", ATTRS{idProduct}=="00fd", MODE:="0666"
    # rule for primary microcontroller USB peripheral
    SUBSYSTEM=="usb", ATTRS{idVendor}=="1cbe", ATTRS{idProduct}=="0003", MODE:="0666"

Then I added a rule to the ``Makefile`` to load the program onto the board::

    $ make flash

Testing
=======

Currently, the application prints some debug information to the
microcontroller's UART0 which routes through the board's second microcontroller
to the ICDI USB port. You can use ``screen`` to monitor that::

    $ screen /dev/ACM0 115200

More importantly, the point of the whole thing is to stream data over the
primary microcontroller's USB peripheral. To test the functionality and easily
update the test script as new features are developed, I'm using Python with the
pyusb_ library. I like to do everything in Python in a virtual environment::

    $ python -m venv .venv
    $ source .venv/bin/activate{.your-shell}
    (.venv) $ pip install pyusb numpy

pyusb is really just a wrapper around a USB library (it supports several), so
you'll need a backend installed. I have ``libusb-1.x`` installed.

Now you can run the test script::

    (.venv) $ python test.py

TODO
====

- Use a control transfer to send a "start" command from the host (laptop)
- Start a timer to periodically write data from the device
- Enable an ADC channel to actually sample some data
- Get some kind of turnkey SPI sensor and start collecting data from it

.. _Stellaris LM4F120: http://www.ti.com/tool/ek-lm4f120xl
.. _Tiva TM4C123G: http://www.ti.com/tool/ek-tm4c123gxl
.. _pyusb: https://github.com/pyusb/pyusb

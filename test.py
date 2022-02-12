"""USB bulk device test script."""

import usb
import array
import numpy as np


class TivaDaq(object):

    id_vendor = 0x1cbe
    id_product = 0x0003
    buf_size = 256

    def __init__(self):
        self.dev = usb.core.find(idVendor=self.id_vendor,
                                 idProduct=self.id_product)

        if self.dev is None:
            raise ValueError('Device not found')

        self.dev.set_configuration()

        self.cfg = self.dev.get_active_configuration()
        self.intf = self.cfg[(0, 0)]

        self.ep_in = self._find_ep(usb.util.ENDPOINT_IN)
        self.ep_out = self._find_ep(usb.util.ENDPOINT_OUT)

        self.read_size = self.ep_in.wMaxPacketSize
        self.buf = array.array('f', b'\x00'*self.read_size)

    def read(self, msg):
        self.ep_out.write(msg)
        rx = self.ep_in.read(self.buf)
        return np.frombuffer(self.buf, dtype=np.float32)

    def _find_ep(self, io):
        def match(ep):
            return usb.util.endpoint_direction(ep.bEndpointAddress) == io
        return usb.util.find_descriptor(self.intf, custom_match=match)


daq = TivaDaq()

while True:
    msg = input('Message (q to quit): ')
    if msg == 'q':
        break

    if len(msg) > daq.buf_size:
        print("Enter fewer than {} characters please".format(daq.buf_size))
        continue

    resp = daq.read(msg)
    print('sent:     {}'.format(msg))
    print('recieved: {}'.format(resp))

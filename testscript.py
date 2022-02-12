"""USB bulk device test script."""

import usb
import array
import numpy as np

id_vendor = 0x1cbe
id_product = 0x0003
buf_size = 256


def find_ep(intf, direction):
    def match(ep):
        return usb.util.endpoint_direction(ep.bEndpointAddress) == direction
    return usb.util.find_descriptor(intf, custom_match=match)


dev = usb.core.find(idVendor=id_vendor, idProduct=id_product)
if dev is None:
    raise ValueError('Device not found')

dev.set_configuration()

cfg = dev.get_active_configuration()
intf = cfg[(0, 0)]

ep_in = find_ep(intf, usb.util.ENDPOINT_IN)
ep_out = find_ep(intf, usb.util.ENDPOINT_OUT)

read_size = ep_in.wMaxPacketSize
print("read size: {} bytes".format(read_size))

buf = array.array('f', b'\x00'*read_size)

ep_out.write('yo')

for i in range(10):
    rx = ep_in.read(buf)
    arr = np.frombuffer(buf, dtype=np.float32)
    print('recieved: {}'.format(arr))

ep_out.write('stop')

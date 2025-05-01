#!/usr/bin/env python3
# Install python3 HID package https://pypi.org/project/hid/
import hid

USB_VID = (0x1209, 0x000f)

print("VID list: " + ", ".join('%02x' % v for v in USB_VID))

for vid in  USB_VID:
    for dict in hid.enumerate(vid):
        print(dict)
        dev = hid.Device(dict['vendor_id'], dict['product_id'])
        if dev:
            while True:
                # Get input from console and encode to UTF8 for array of chars.
                # hid generic in/out is single report therefore by HIDAPI requirement
                # it must be preceded, with 0x00 as dummy reportID
                in_str = input("Send text to HID Device : ")
                # Seen as hex array
                in_data = int(in_str, 16).to_bytes((len(in_str) + 1) // 2, byteorder='big')
                print("Sending to HID Device:", in_data, '\n')
                dev.write(in_data)
                if in_data[1] == 0x07:
                    str_in = dev.read(64)
                    print("Received from HID Device:", str_in, '\n')

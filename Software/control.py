import argparse
import hid

VID = 0x1209
PID = 0x7410

# return: Device ID: Path


def get_add_device() -> dict[bytes, str]:
    res = {}
    for d in hid.enumerate(vid=VID, pid=PID):
        dev = hid.Device(vid=d['vendor_id'],
                         pid=d['product_id'], path=d['path'])
        dev_in = 0x0005.to_bytes(2 + 1, byteorder='big')
        dev.write(dev_in)
        dev_out = dev.read(64)
        dev_out = str(dev_out.hex())
        p: bytes = d['path']
        res[p] = dev_out
    return res


def get_device_by(devices: dict[bytes, str], idx: int = None, path: str = None, device_id: str = None) -> hid.Device:
    if idx is None and path is None and device_id is None:
        raise ValueError("At least one selection parameter must be provided.")
    if idx is not None:
        dev = hid.Device(vid=VID, pid=PID, path=list(devices.keys())[idx])
    elif path is not None:
        if path not in devices:
            raise ValueError(f"Device with path {path} not found.")
        dev = hid.Device(vid=VID, pid=PID, path=path)
    elif device_id is not None:
        for p, dev_id in devices.items():
            if dev_id == device_id:
                dev = hid.Device(vid=VID, pid=PID, path=p)
                break
        else:
            raise ValueError(f"Device with ID {device_id} not found.")
    return dev


def send_cmd(dev: hid.Device, cmd: int) -> bytes | None:
    cmd_bytes = cmd.to_bytes(2 + 1, byteorder='big')
    dev.write(cmd_bytes)
    if cmd in (0x0004, 0x0005):
        return dev.read(64)
    return None


def main():
    parser = argparse.ArgumentParser(
        description="Control script for managing tf mux.")
    parser.add_argument(
        '-l', '--list', help="List all current devices", action='store_true')
    device_sel_group = parser.add_mutually_exclusive_group(required=False)
    device_sel_group.description = "Select a device by ID, path, or device ID."
    device_sel_group.add_argument(
        '-i', '--id', type=int, help="Select device by ID")
    device_sel_group.add_argument(
        '-p', '--path', type=str, help="Select device by path")
    device_sel_group.add_argument(
        '-d', '--device', type=str, help="Select device by device ID")

    options_group = parser.add_mutually_exclusive_group()
    options_group.description = "Select an action to perform on the device."

    sd_mux_group = parser.add_mutually_exclusive_group()
    sd_mux_group.description = "Control the SD mux device."
    sd_mux_group.add_argument(
        '--on', action='store_true', default=None, help="Turn on the device")
    sd_mux_group.add_argument(
        '--off', action='store_true', default=None, help="Turn off the device")
    sd_mux_group.add_argument(
        '--dut', '-2', action='store_true', default=None, help="Set DUT mode")
    sd_mux_group.add_argument(
        '--host', '-1', action='store_true', default=None, help="Set Host mode")

    brd_ctrl_group = parser.add_mutually_exclusive_group()
    brd_ctrl_group.description = "Control the board."
    brd_ctrl_group.add_argument(
        '--power_off', '--poff', action='store_true', default=None, help="Power off the board")
    brd_ctrl_group.add_argument(
        '--power_on', '--pon', action='store_true', default=None, help="Power on the board")

    options_group.add_argument(
        '--status', action='store_true', default=None, help="Get device status")
    args = parser.parse_args()
    devices = get_add_device()
    devices = dict(sorted(devices.items()))

    if args.list:
        if devices:
            print("Connected Devices:")
            for idx, (path, device_id) in enumerate(devices.items()):
                print(f"\tID: {idx} Device ID: {device_id}, Path: {path}")
        else:
            print("No devices found.")
        return

    if args.id is None and args.path is None and args.device is None:
        print("No device selected.")
        return

    dev = get_device_by(devices, idx=args.id,
                        path=args.path, device_id=args.device)

    if args.on:
        send_cmd(dev, 0x0001)
    if args.off:
        send_cmd(dev, 0x0000)
    if args.dut:
        send_cmd(dev, 0x0003)
    if args.host:
        send_cmd(dev, 0x0002)
    if args.power_on:
        send_cmd(dev, 0x0006)
    if args.power_off:
        send_cmd(dev, 0x0007)
    if args.status:
        status = send_cmd(dev, 0x0004)
        if status:
            mux_on = status[0] == 0x01
            host_mode = status[1] == 0x01
            dut_mode = status[2] == 0x01
            brd_pwr = status[3] == 0x01
            print(
                f"""Mux is {mux_on and 'ON' or 'OFF'},
The TF Card is in connected to {'Host' if host_mode else 'DUT' if dut_mode else 'Unknown'} mode.
Board power is {'ON' if brd_pwr else 'OFF'}."""
            )
        else:
            print("No status response received.")


if __name__ == "__main__":
    main()

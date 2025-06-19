use std::error::Error;

use hidapi::{DeviceInfo, HidApi};

use crate::{
    conf::{Confs, DevSelector, MuxOptions, PID, PwrOptions, VID},
    hid_cmds::{Cmds, CmdsGroup0, QueryStatus},
};

mod conf;
mod hid_cmds;

fn get_devices() -> Result<Vec<DeviceInfo>, Box<dyn Error>> {
    let api = HidApi::new()?;
    let devices: Vec<_> = api
        .device_list()
        .filter_map(|device| {
            if device.vendor_id() == VID && device.product_id() == PID {
                Some(device.to_owned())
            } else {
                None
            }
        })
        .collect();
    Ok(devices)
}

fn sort_devices(devices: Vec<DeviceInfo>) -> Result<Vec<(DeviceInfo, u32)>, Box<dyn Error>> {
    let api = HidApi::new()?;
    let mut devices_keys: Vec<(DeviceInfo, u32)> = devices
        .iter()
        .map(|dev_i| {
            let dev = dev_i.open_device(&api)?;
            let id = Cmds::get_device_id(&dev)?;
            Ok((dev_i.clone(), id))
        })
        .collect::<Result<_, Box<dyn Error>>>()?;
    devices_keys.sort_by_key(|(_, id)| *id);
    Ok(devices_keys)
}

fn main() -> Result<(), Box<dyn Error>> {
    let args = Confs::parse();

    let devices = get_devices()?;
    let devices = sort_devices(devices)?;

    if matches!(args.dev_sel, DevSelector::List) {
        if devices.is_empty() {
            eprintln!("No devices found.");
            return Ok(());
        }
        println!("Found {} devices:", devices.len());
        println!("Index\tDevice ID\tPath");
        for (idx, (device, id)) in devices.iter().enumerate() {
            println!("{}\t{:X}\t{}", idx, id, device.path().to_str()?);
        }

        return Ok(());
    }

    let api = HidApi::new()?;
    let dev = args.dev_sel.get_device(&api, &devices)?;

    if let Some(debug_cmd) = args.debug_cmd {
        Cmds::G1(debug_cmd).exec(&dev)?;
    }

    if let Some(mux) = args.mux {
        match mux {
            MuxOptions::On => Cmds::G0(CmdsGroup0::OpenMux),
            MuxOptions::Off => Cmds::G0(CmdsGroup0::CloseMux),
            MuxOptions::Host => Cmds::G0(CmdsGroup0::ToHost),
            MuxOptions::Dut => Cmds::G0(CmdsGroup0::ToSub),
        }
        .exec(&dev)?;
    }

    if let Some(pwr) = args.pwr {
        match pwr {
            PwrOptions::On => Cmds::G0(CmdsGroup0::BrdPwrOn),
            PwrOptions::Off => Cmds::G0(CmdsGroup0::BrdPwrOff),
        }
        .exec(&dev)?;
    }

    if args.status {
        let cmd = Cmds::G0(CmdsGroup0::Query);
        let res = cmd.exec(&dev)?;
        if let Some(data) = res {
            let stat: QueryStatus = data.into();
            print!(
                "Status: \n\tMux: {},\n\tHost: {},\n\tDut: {},\n\tBoard Power: {}\n",
                match stat.mux_status {
                    0 => "Closed",
                    _ => "Open",
                },
                match stat.is_host {
                    0 => "Disconnected",
                    _ => "Connected",
                },
                match stat.is_sub {
                    0 => "Disconnected",
                    _ => "Connected",
                },
                match stat.brd_pwr_status {
                    0 => "Off",
                    _ => "On",
                }
            );
        }
    }

    Ok(())
}

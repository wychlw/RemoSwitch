use std::{error::Error, str::FromStr};

use clap::{Args, Parser, Subcommand, ValueEnum};
use hidapi::{DeviceInfo, HidApi, HidDevice};

use crate::hid_cmds::CmdsGroup1;

pub const VID: u16 = 0x1209;
pub const PID: u16 = 0x7410;

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum DevSelector {
    Idx(String),
    Device(String),
    Path(String),
    List,
}

impl DevSelector {
    pub fn get_device(
        &self,
        api: &HidApi,
        devices: &[(DeviceInfo, u32)],
    ) -> Result<HidDevice, Box<dyn Error>> {
        match self {
            Self::List => Err("Can't get device in list mode".into()),
            Self::Idx(idx) => {
                let idx: usize = idx.parse()?;
                if idx >= devices.len() {
                    return Err("Index out of bounds".into());
                }
                let dev = &devices[idx];
                Ok(dev.0.open_device(api)?)
            }
            Self::Device(id) => {
                let id = u32::from_str_radix(id, 16)?;
                for (dev, device_id) in devices {
                    if *device_id == id {
                        return Ok(dev.open_device(api)?);
                    }
                }
                Err(format!("Device with ID {id} not found").into())
            }
            Self::Path(path) => {
                for (dev, _) in devices {
                    if dev.path().to_str()? == path {
                        return Ok(dev.open_device(api)?);
                    }
                }
                Err(format!("Device with path {path} not found").into())
            }
        }
    }
}

#[derive(Args, Debug)]
struct DevSelArgs {
    #[arg(long, short, conflicts_with_all = ["device", "path", "list"])]
    id: Option<String>,

    #[arg(long, short, conflicts_with_all = ["id", "path", "list"])]
    device: Option<String>,

    #[arg(long, conflicts_with_all = ["id", "device", "list"])]
    path: Option<String>,

    #[arg(long, short, conflicts_with_all = ["id", "device", "path"], 
         default_value_t = false)]
    list: bool,
}

#[derive(Debug, Clone, ValueEnum)]
pub enum MuxOptions {
    On,
    Off,
    Host,
    Dut,
}

#[derive(Debug, Clone, ValueEnum)]
pub enum PwrOptions {
    On,
    Off,
}

#[derive(Debug, Clone, Subcommand)]
pub enum Commands {
    #[command(name = "debug", alias = "d", hide = true)]
    DebugCmd { cmd: String, param: u8 },
}

#[derive(Parser, Debug)]
struct ConfsInternal {
    #[command(flatten)]
    /// Select the device to operate on
    dev_sel: DevSelArgs,

    #[arg(long, short, value_enum)]
    mux: Option<MuxOptions>,

    #[arg(long, short, value_enum)]
    pwr: Option<PwrOptions>,

    #[arg(long, short, default_value_t = false)]
    /// Show status of the device
    status: bool,

    #[command(subcommand)]
    /// Commands to execute
    cmd: Option<Commands>,
}

#[derive(Debug, Clone)]
pub struct Confs {
    pub dev_sel: DevSelector,
    pub mux: Option<MuxOptions>,
    pub pwr: Option<PwrOptions>,
    pub debug_cmd: Option<(CmdsGroup1, u8)>,
    pub status: bool,
}

impl Confs {
    pub fn parse() -> Self {
        let args = ConfsInternal::parse();
        let dev_sel = if let Some(id) = args.dev_sel.id {
            DevSelector::Idx(id)
        } else if let Some(device) = args.dev_sel.device {
            DevSelector::Device(device)
        } else if let Some(path) = args.dev_sel.path {
            DevSelector::Path(path)
        } else if args.dev_sel.list {
            DevSelector::List
        } else {
            panic!("No valid device selection provided. Use --help for usage information.");
        };
        if args.cmd.is_some() && (args.mux.is_some() || args.pwr.is_some() || args.status) {
            panic!("Cannot use device control options with command execution. Use --help for usage information.");
        }
        let mux = args.mux;
        let pwr = args.pwr;
        let debug_cmd = if let Some(Commands::DebugCmd { cmd, param }) = args.cmd {
            Some((CmdsGroup1::from_str(&cmd).unwrap(), param))
        } else {
            None
        };
        let status = args.status;
        Confs {
            dev_sel,
            mux,
            pwr,
            status,
            debug_cmd,
        }
    }
}

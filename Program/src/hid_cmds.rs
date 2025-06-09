use std::error::Error;

use hidapi::HidDevice;

#[allow(unused)]
pub struct QueryStatus {
    pub mux_status: u8,
    pub is_host: u8,
    pub is_sub: u8,
    pub brd_pwr_status: u8,
}

impl From<[u8; 4]> for QueryStatus {
    fn from(data: [u8; 4]) -> Self {
        QueryStatus {
            mux_status: data[0],
            is_host: data[1],
            is_sub: data[2],
            brd_pwr_status: data[3],
        }
    }
}
impl From<Vec<u8>> for QueryStatus {
    fn from(data: Vec<u8>) -> Self {
        if data.len() < 4 {
            panic!("Data length is less than 4 bytes");
        }
        QueryStatus {
            mux_status: data[0],
            is_host: data[1],
            is_sub: data[2],
            brd_pwr_status: data[3],
        }
    }
}
impl From<&[u8]> for QueryStatus {
    fn from(data: &[u8]) -> Self {
        if data.len() < 4 {
            panic!("Data length is less than 4 bytes");
        }
        QueryStatus {
            mux_status: data[0],
            is_host: data[1],
            is_sub: data[2],
            brd_pwr_status: data[3],
        }
    }
}

#[allow(unused)]
#[derive(Debug, Clone, Copy)]
#[repr(u8)]
pub enum CmdsGroup0 {
    CloseMux = 0x00,
    OpenMux = 0x01,
    ToHost = 0x02,
    ToSub = 0x03,
    Query = 0x04,
    DeviceId = 0x05,
    BrdPwrOn = 0x06,
    BrdPwrOff = 0x07,
}

#[allow(unused)]
#[derive(Debug, Clone, Copy)]
#[repr(u8)]
pub enum CmdsGroup1 {
    MuxEn = 0x00,
    Sel0 = 0x01,
    Sel1 = 0x02,
    TfVccEn = 0x03,
    DownVccEn = 0x04,
    UpVccEn = 0x05,
    UpCho = 0x06,
    SdHostLED = 0x07,
    SdTsLed = 0x08,
    BrdPwr = 0x09,
    UartSend = 0x0A,
    AllStatus = 0x0B,
}

#[allow(unused)]
#[derive(Debug, Clone, Copy)]
pub enum Cmds {
    G0(CmdsGroup0),
    G1((CmdsGroup1, u8)),
}

impl Cmds {
    fn send_to(dev: &HidDevice, data: &[u8]) -> Result<(), Box<dyn Error>> {
        let send_data = [[0u8].to_vec(), data.to_vec()].concat();
        dev.write(&send_data)?;
        Ok(())
    }

    fn recv_from(dev: &HidDevice) -> Result<Vec<u8>, Box<dyn Error>> {
        let mut buf = [0u8; 64];
        let bytes_read = dev.read(&mut buf)?;
        if bytes_read == 0 {
            return Err("No data received".into());
        }
        Ok(buf[..bytes_read].to_vec())
    }

    pub fn exec(&self, dev: &HidDevice) -> Result<Option<Vec<u8>>, Box<dyn Error>> {
        match self {
            Cmds::G0(cmd) => {
                let cmd_byte = *cmd as u8;
                let data = [0, cmd_byte];
                Self::send_to(dev, &data)?;
                match cmd {
                    CmdsGroup0::Query => {
                        let res = Self::recv_from(dev)?;
                        Ok(res.into())
                    }
                    CmdsGroup0::DeviceId => {
                        let res = Self::recv_from(dev)?;
                        Ok(Some(res))
                    }
                    _ => Ok(None),
                }
            }
            Cmds::G1(cmd) => {
                let (cmd_group, param) = cmd;
                let cmd_byte = *cmd_group as u8;
                let data = [0, cmd_byte, *param];
                Self::send_to(dev, &data)?;
                Ok(None)
            }
        }
    }

    pub fn get_device_id(dev: &HidDevice) -> Result<u32, Box<dyn Error>> {
        let cmd = Cmds::G0(CmdsGroup0::DeviceId);
        let res = cmd.exec(dev)?;
        if let Some(data) = res
            && data.len() >= 4
        {
            let id = u32::from_le_bytes([data[0], data[1], data[2], data[3]]);
            return Ok(id);
        }
        Err("Invalid device ID response".into())
    }
}

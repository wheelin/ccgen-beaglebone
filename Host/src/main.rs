use std::{ffi::CString, io::BufRead};

use engcfg::CFGS;
use libc;
use regex::{Regex};

const PRU_ADDR: i32 = 0x4A300000;
const PRU_DRAM0_OFFSET: isize = 0x00000;
const PRU_DRAM0_EPM_OFFSET: isize = 0x200;
const PRU_LEN: usize = 0x8000;

pub mod engcfg;

#[repr(C)]
pub struct EpmSigGen {
    pattern : [u8;7200],
    speed : u32,
    sig_idx: u16,
    activated: u8,
}

pub struct EpmSigCtrlr {
    epm_siggen_ptr: *mut EpmSigGen,
}

impl EpmSigCtrlr {
    pub fn new() -> EpmSigCtrlr {
        let mmap_path = CString::new("/dev/mem").expect("Cannot create path to /dev/mem");
        let mmap_fd = unsafe{ libc::open(mmap_path.as_ptr(), libc::O_RDWR | libc::O_SYNC)};

        if mmap_fd < 0 {
            panic!("Cannot open /dev/mem");
        }

        let mem_ptr = unsafe {
            libc::mmap(
                std::ptr::null_mut(),
                PRU_LEN,
                libc::PROT_READ | libc::PROT_WRITE,
                libc::MAP_SHARED,
                mmap_fd,
                PRU_ADDR
            )
        };
        if mem_ptr.is_null() {
            panic!("Cannot map PRU. mmap returned a NULL pointer.");
        }

        let epmctrl = EpmSigCtrlr {
            epm_siggen_ptr: unsafe{mem_ptr.offset(PRU_DRAM0_OFFSET + PRU_DRAM0_EPM_OFFSET)} as *mut EpmSigGen
        };

        unsafe {
            CFGS[0].gen_pulse_train(
                &mut (*epmctrl.epm_siggen_ptr).pattern,
                0x08,
                0x20,
                [0u8; 6]
            );
            (*epmctrl.epm_siggen_ptr).speed = 10000;
            (*epmctrl.epm_siggen_ptr).activated = 0;
        }
        epmctrl
    }

    fn get_ptr(&self) -> &EpmSigGen {
        unsafe {
            &(*self.epm_siggen_ptr)
        }
    }

    pub fn set_speed(&mut self, speed: u32) {
        unsafe{
            (*self.epm_siggen_ptr).speed = if speed > 0 { speed } else { 10 };
        }
    }

    pub fn get_speed(&self) -> u32 {
        unsafe{
            (*self.epm_siggen_ptr).speed
        }
    }

    pub fn set_state(&mut self, mask: u8) {
        unsafe{
            (*self.epm_siggen_ptr).activated = mask;
        }
    }

    pub fn print_info(&self) {
        println!("Index: {}", self.get_ptr().sig_idx);
        println!("Speed: {}", self.get_ptr().speed);
    }
}

fn main() {
    let mut epm = EpmSigCtrlr::new();

    let stdin = std::io::stdin();

    let re = Regex::new(r"^s(\d+).*").unwrap();

    println!("\n'u': increase speed, 'd': decrease speed, 'e': enable, 'n': disable, 'q': quit");
    print!("> ");
    for line in stdin.lock().lines() {
        println!("\n'u': increase speed, 'd': decrease speed, 'e': enable, 'n': disable, 'q': quit");
        print!("> ");

        let line_chars = line.unwrap();
        if line_chars.len() == 0 { continue; }
        match line_chars.chars().nth(0).unwrap() {
            'u' => epm.set_speed(epm.get_speed() + 50),
            'd' => epm.set_speed(epm.get_speed() - 50),
            's' => {
                let speed: u32 = match re.captures(&line_chars) {
                    None => epm.get_speed(),
                    Some(cap) => cap[1].parse().unwrap_or(epm.get_speed()),
                };
                epm.set_speed(speed);
            }
            'e' => epm.set_state(0),
            'n' => epm.set_state(0xFF),
            'q' => std::process::exit(0),
            _ => (),
        }
    }
}

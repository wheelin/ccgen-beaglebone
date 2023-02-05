//! # EngCfg
//!
//! ## Introduction
//!
//! This crate allows to generate 4-stroke engine waveforms for direct writing on GPIO as bit mask.
//!
//! The goal is to generate a pulse train with the following information:
//! * crankshaft wheel signal
//! * camshaft wheel signal
//! * top-dead-center piston position for each cylinder.
//!
//! ## Technical information
//!
//! The pulse train buffer is an array of integers (generally u8, u16 or u32, depending on the GPIO register width) with length 7200. An array has been used
//! in order to be compatible with DMA (Direct Memory Access) mechanisms. The goal
//! of this crate is really to create crank/cam/tdc generators for ECU development purpose.
//!
//! Each array element is a bit field representing the current crank, cam, tdc state, at a precise
//! position (the array index) in the engine cycle.
//!
//! Here is the implementation done for `u8` in this crate:
//!
//! | Bit | Desc.     |
//! |:----|-----------|
//! |0    | Camshaft  |
//! |1    | Crankshaft|
//! |2    | TDC cyl. 1|
//! |3    | TDC cyl. 2|
//! |4    | TDC cyl. 3|
//! |5    | TDC cyl. 4|
//! |6    | TDC cyl. 5|
//! |7    | TDC cyl. 6|
//!
//! When generated, the pulse train array shall be applied on output pins using a strict timing:
//! * either by using interrupts and an hardware timer, by writing each element of the array on the GPIO port one by one and restarting to 0 at the end.
//! * or by using a DMA with circular configuration and timer triggering. This way, the CPU has really nothing to do but reconfigure the timer in case of engine speed changes.
//!
//! ## Limitations
//!
//! The following engines can not be generated at the moment:
//! * Engines with more than 6 cylinders
//! * Asymmetrical engines (TDCs are not spaced evenly)
//! * The concept uses a relatively high amount of RAM. But with the use of appropriate DMA and timers, the pulse train generation should not even require CPU processing.
//!

use core::ops::{BitAnd, BitOrAssign, Not, BitOr};


/// Level implementation, for crank, cam and TDC signaling
#[derive(Eq, PartialEq, Clone, Copy, Debug)]
pub enum Level {
    Low = 0,
    High = 1,
}

impl core::ops::Not for Level {
    type Output = Level;

    fn not(self) -> Self::Output {
        if self == Level::High {
            Level::Low
        } else {
            Level::High
        }
    }
}

/// Number of cylinders for the current engine, helps to calculate TDC positions
pub enum CylNr {
    /// Engine with 4 cylinders
    Cyl4,
    /// Engine with 6 cylinders
    Cyl6,
}

impl CylNr {
    /// Enum conversion into real cylinder number
    pub const fn val(&self) -> usize {
        match *self {
            CylNr::Cyl4 => 4,
            CylNr::Cyl6 => 6,
        }
    }
}

/// Type of crankshaft
pub enum CrkType {
    /// 30 teeth, 1 missing, gap level is low
    Crk30m1,
    /// 30 teeth, 2 missing, gap level is low
    Crk30m2,
    /// 60 teeth, 1 missing, gap level is low
    Crk60m1,
    /// 60 teeth, 1 missing, gap level is low
    Crk60m2,
    /// 120 teeth, 1 missing, gap level is low
    Crk120m1,
    /// 120 teeth, 2 missing, gap level is low
    Crk120m2,
    /// 30 teeth, 1 missing, gap level is high
    Crk30m1Inv,
    /// 30 teeth, 2 missing, gap level is high
    Crk30m2Inv,
    /// 60 teeth, 1 missing, gap level is high
    Crk60m1Inv,
    /// 60 teeth, 2 missing, gap level is high
    Crk60m2Inv,
    /// 120 teeth, 1 missing, gap level is high
    Crk120m1Inv,
    /// 120 teeth, 2 missing, gap level is high
    Crk120m2Inv,
}

impl CrkType {
    /// Returns the number of teeth for the current wheel
    pub const fn nr_of_teeth(&self) -> usize {
        match *self {
            CrkType::Crk30m1 | CrkType::Crk30m2 | CrkType::Crk30m1Inv | CrkType::Crk30m2Inv => 30,
            CrkType::Crk60m1 | CrkType::Crk60m2 | CrkType::Crk60m1Inv | CrkType::Crk60m2Inv => 60,
            CrkType::Crk120m1 | CrkType::Crk120m2 | CrkType::Crk120m1Inv | CrkType::Crk120m2Inv => {
                120
            }
        }
    }

    /// Returns the number of missing teeth for the current wheel
    pub const fn nr_of_missing_teeth(&self) -> usize {
        match *self {
            CrkType::Crk30m1
            | CrkType::Crk30m1Inv
            | CrkType::Crk60m1
            | CrkType::Crk60m1Inv
            | CrkType::Crk120m1
            | CrkType::Crk120m1Inv => 1,
            CrkType::Crk30m2
            | CrkType::Crk30m2Inv
            | CrkType::Crk60m2
            | CrkType::Crk60m2Inv
            | CrkType::Crk120m2
            | CrkType::Crk120m2Inv => 2,
        }
    }

    /// Returns the tooth size in degrees DEG_I16_DEC1
    pub const fn angle_per_tooth(&self) -> usize {
        match *self {
            CrkType::Crk30m1 | CrkType::Crk30m2 | CrkType::Crk30m1Inv | CrkType::Crk30m2Inv => {
                3600 / 30
            }
            CrkType::Crk60m1 | CrkType::Crk60m2 | CrkType::Crk60m1Inv | CrkType::Crk60m2Inv => {
                3600 / 60
            }
            CrkType::Crk120m1 | CrkType::Crk120m2 | CrkType::Crk120m1Inv | CrkType::Crk120m2Inv => {
                3600 / 120
            }
        }
    }

    /// Returns first level seen when starting a rotation from angle 0
    pub const fn first_level(&self) -> Level {
        match *self {
            CrkType::Crk30m1
            | CrkType::Crk30m2
            | CrkType::Crk60m1
            | CrkType::Crk60m2
            | CrkType::Crk120m1
            | CrkType::Crk120m2 => Level::High,
            CrkType::Crk30m1Inv
            | CrkType::Crk30m2Inv
            | CrkType::Crk60m1Inv
            | CrkType::Crk60m2Inv
            | CrkType::Crk120m1Inv
            | CrkType::Crk120m2Inv => Level::Low,
        }
    }
}

/// Camshaft wheel configuration
pub struct Cam {
    /// Level when first crankshaft gap is met
    pub first_level: Level,
    /// Angle of each camshaft edge, starting from the first crankshaft gap, DEG_I16_DEC1. Negative angle if not used.
    pub ev_angles: [i16; 20],
}

/// Engine configuration
pub struct EngCfg {
    /// Camshaft wheel configuration
    pub cam: Cam,
    /// Crankshaft type configuration
    pub crk: CrkType,
    /// Angle from reference (crank gap) to first Top-Dead-Center (TDC): DEG_S16_DEC1
    pub ref_to_tdc0: i16,
    /// Number of cylinders, used for TDC generation
    pub nr_of_cyl: CylNr,
}

impl EngCfg {
    /// Arguments:
    /// * pt: output argument, pulse train generated from engine configuration for waveform generation
    /// * cam_msk: bitmask indicating camshaft signal position in bitfield
    /// * crk_msk: bitmask indicating crankshaft signal position in bitfield
    /// * tdc_msk: bitmasks (starts with TDC0) indicating TDCs signal position in bitfield
    ///
    /// Returns:
    /// * Ok: generation has been achieved correctly
    /// * Err: the buffer has not the minimal required length
    pub fn gen_pulse_train<T>(&self, pt: &mut [T; 7200], cam_msk: T, crk_msk: T, tdc_msk: [T; 6])
    where
        T: Copy
            + Clone
            + Sized
            + BitOrAssign
            + BitAnd<Output = T>
            + Not<Output = T>
            + BitOr<Output = T>,
    {
        let mut idx_cam_edges = 0;
        let mut cam_lvl = self.cam.first_level;

        let crk_tooth_angle = self.crk.angle_per_tooth();
        let angle_missing_teeth = self.crk.nr_of_missing_teeth() * self.crk.angle_per_tooth();
        let mut crk_lvl = self.crk.first_level();

        for (angle, val) in pt.iter_mut().enumerate() {
            *val = if cam_lvl == Level::High {
                *val | cam_msk
            } else {
                *val & !cam_msk
            };
            if idx_cam_edges < 20 {
                if self.cam.ev_angles[idx_cam_edges] == angle as i16 {
                    cam_lvl = !cam_lvl;
                    idx_cam_edges += 1;
                }
            }

            *val = if crk_lvl == Level::High {
                *val | crk_msk
            } else {
                *val & !crk_msk
            };
            if angle % ((crk_tooth_angle / 2) as usize) == 0 && angle != 0 {
                if (angle % 3600) >= 3600 - angle_missing_teeth {
                    crk_lvl = !self.crk.first_level();
                } else {
                    crk_lvl = !crk_lvl;
                }
            }
        }

        let tdc_to_tdc = 7200 / self.nr_of_cyl.val();
        pt[self.ref_to_tdc0 as usize] |= tdc_msk[0];

        for cyl in 1..self.nr_of_cyl.val() {
            pt[self.ref_to_tdc0 as usize + (cyl * tdc_to_tdc)] |= tdc_msk[cyl];
        }
    }
}

pub static CFGS: [EngCfg; 1] = [EngCfg {
    cam: Cam {
        first_level: Level::High,
        ev_angles: [
            289, 389, 1189, 1289, 1489, 1589, 2089, 2189, 2689, 2789, 3889, 3989, 5089, 5189, 5689,
            5789, 6289, 6389, 6589, 6689,
        ],
    },
    crk: CrkType::Crk60m2Inv,
    ref_to_tdc0: 658,
    nr_of_cyl: CylNr::Cyl6,
}];
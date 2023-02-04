/*
 * Source Modified by Derek Molloy for Exploring BeagleBone Rev2
 * Based on the examples distributed by TI
 *
 * Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the
 *      distribution.
 *
 *    * Neither the name of Texas Instruments Incorporated nor the names of
 *      its contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>
#include <pru_cfg.h>
#include <pru_ctrl.h>
#include "resource_table_empty.h"

#define EPM_PATTERN_LEN    (7200u)
typedef struct {
    uint8_t Patten[EPM_PATTERN_LEN];
    uint32_t Speed;
    uint32_t Counter;
    uint16_t SigIndex;
    uint8_t Activated;
} T_Epm_SigGen;

volatile register uint32_t __R30;
volatile register uint32_t __R31;

#define PRU0_DRAM   0x00000
volatile T_Epm_SigGen *Epm_SigGen = (T_Epm_SigGen *) (PRU0_DRAM + 0x200);

void main(void)
{
    // Clear SYSCFG[STANDBY_INIT] to enable OCP master port
    CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

    // Infinite loop
    while (1) {
        if (Epm_SigGen->Activated != 0u)
        {
            if (Epm_SigGen->Counter == 0u)
            {
                Epm_SigGen->Counter = Epm_SigGen->Speed;
                __R30 = Epm_SigGen->Patten[Epm_SigGen->SigIndex];
                Epm_SigGen->SigIndex = (Epm_SigGen->SigIndex + 1u) % EPM_PATTERN_LEN;
            }
            else
            {
                Epm_SigGen->Counter -= 1u;
            }
        }
        else
        {
            /* Generation is not activated, do nothing. */
        }
    }
}


 #include <stdint.h>
#include <pru_cfg.h>
#include <pru_ctrl.h>
#include "resource_table_empty.h"

extern void wait_cycles(uint32_t Delay);

#define EPM_PATTERN_LEN    (7200u)
typedef struct {
    uint8_t Patten[EPM_PATTERN_LEN];
    uint32_t Speed;
    uint16_t SigIndex;
    uint8_t Activated;
} T_Epm_SigGen;

volatile register uint32_t __R30;
volatile register uint32_t __R31;

#define PRU0_DRAM   0x00000
volatile T_Epm_SigGen *Epm_SigGen = (T_Epm_SigGen *) (PRU0_DRAM + 0x200);

void main(void)
{
    uint16_t Idx = 0u;
    // Clear SYSCFG[STANDBY_INIT] to enable OCP master port
    CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

    for (Idx = 1u; Idx < EPM_PATTERN_LEN; Idx++)
    {
        Epm_SigGen->Patten[Idx] = 0u;
    }

    Epm_SigGen->Patten[0] = 0x20 | 0x08;
    Epm_SigGen->Speed = 1u;
    Epm_SigGen->Activated = 0xFFu;
    Epm_SigGen->SigIndex = 0u;

    // Infinite loop
    while (1)
    {
        __R30 = Epm_SigGen->Patten[Epm_SigGen->SigIndex] & Epm_SigGen->Activated;
        Epm_SigGen->SigIndex = (Epm_SigGen->SigIndex + 1u) % EPM_PATTERN_LEN;
        wait_cycles(Epm_SigGen->Speed);
    }
}


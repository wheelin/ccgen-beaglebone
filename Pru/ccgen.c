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


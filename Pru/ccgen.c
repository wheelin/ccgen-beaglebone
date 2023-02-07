#include <stdint.h>
#include <pru_cfg.h>
#include <pru_ctrl.h>
#include <pru_iep.h>
#include "resource_table_empty.h"

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

    CT_IEP.TMR_GLB_CFG_bit.CNT_EN = 0u;             /* Disable timer first */
    CT_IEP.TMR_GLB_CFG_bit.DEFAULT_INC = 1u;        /* Set frequency to 200MHz */
    CT_IEP.TMR_GLB_STS_bit.CNT_OVF = 1u;            /* Clear overflow bit status */
    CT_IEP.TMR_CMP_STS_bit.CMP_HIT = 0xFFu;         /* Clear compare status register*/
    CT_IEP.TMR_CMP_CFG_bit.CMP_EN = 1u;             /* Enable first compare register */
    CT_IEP.TMR_CMP_CFG_bit.CMP0_RST_CNT_EN = 1u;    /* Reset counter on hit of CMP0 */
    CT_IEP.TMR_COMPEN_bit.COMPEN_CNT = 0x0;         /* Disable compensation */
    CT_IEP.TMR_CNT = 0u;

    for (Idx = 1u; Idx < EPM_PATTERN_LEN; Idx++)
    {
        Epm_SigGen->Patten[Idx] = 0u;
    }

    Epm_SigGen->Patten[0] = 0x20 | 0x08;
    Epm_SigGen->Speed = 1u;
    Epm_SigGen->Activated = 0xFFu;
    Epm_SigGen->SigIndex = 0u;

    // Infinite loop
    CT_IEP.TMR_GLB_CFG_bit.CNT_EN = 1u;
    while (1)
    {
        CT_IEP.TMR_CMP0 = Epm_SigGen->Speed;
        __R30 = Epm_SigGen->Patten[Epm_SigGen->SigIndex] & Epm_SigGen->Activated;
        Epm_SigGen->SigIndex = (Epm_SigGen->SigIndex + 1u) % EPM_PATTERN_LEN;
        while((CT_IEP.TMR_CMP_STS_bit.CMP_HIT & 1u) == 0u)
        {
            /* Wait for timer compare register 0 to hit target value. */
        }
        CT_IEP.TMR_CMP_STS_bit.CMP_HIT = 0xFFu;
    }
}


#include "EpmGen.h"

T_EpmGen_CrkWheelConfig EpmGen_CrkWheelConfig_60m2Hi = {
    .GapPolarity = EPMGEN_POL_HIGH,
    .NrOfTeethTotal = 60u,
    .NrOfTeethMissing = 2u,
};

T_EpmGen_CamWheelConfig EpmGen_CamWheelConfig_6p4D956 = {
    .Angle = {
        {EPMGEN_AG( 28.9), EPMGEN_AG( 38.9)},
        {EPMGEN_AG(118.9), EPMGEN_AG(128.9)},
        {EPMGEN_AG(148.9), EPMGEN_AG(158.9)},
        {EPMGEN_AG(208.9), EPMGEN_AG(218.9)},
        {EPMGEN_AG(268.9), EPMGEN_AG(278.9)},
        {EPMGEN_AG(388.9), EPMGEN_AG(398.9)},
        {EPMGEN_AG(508.9), EPMGEN_AG(518.9)},
        {EPMGEN_AG(568.9), EPMGEN_AG(578.9)},
        {EPMGEN_AG(628.9), EPMGEN_AG(638.9)},
        {EPMGEN_AG(658.9), EPMGEN_AG(668.9)},
    },
    .IndexPolarity = EPMGEN_POL_LOW,
    .NrOfTeethTotal = 10u,
};

T_EpmGen_EngConfig EpmGen_EngCfg[] = {
    {
        .pCamCfg = &EpmGen_CamWheelConfig_6p4D956,
        .pCrkCfg = &EpmGen_CrkWheelConfig_60m2Hi,
    },
};

const T_EpmGen_Config EpmGen_Config = {
    .pEngConfig = &EpmGen_EngCfg[0u],
    .NrOfEngConfig = sizeof(EpmGen_EngCfg)/sizeof(T_EpmGen_EngConfig),
};

void SpdGen_GenSig(uint8_t * pPattern, const T_EpmGen_Config *pEngCfg, const uint8_t CfgNr)
{
    uint8_t CamToothIdx = 0u;
    uint8_t CrkToothIdx = 0u;
    T_EpmGen_CamWheelConfig * pCamCfg = pEngCfg->pEngConfig[CfgNr].pCamCfg;
    T_EpmGen_CrkWheelConfig * pCrkCfg = pEngCfg->pEngConfig[CfgNr].pCrkCfg;
    uint32_t CrkHalfToothAngle = (EPMGEN_SIGNAL_NR_OF_ELEMENTS/pCrkCfg->NrOfTeethTotal)/4u;
    bool CrkLvl = !((bool)pCrkCfg->GapPolarity);
    for(uint16_t Deg = 0u; Deg < EPMGEN_SIGNAL_NR_OF_ELEMENTS; Deg++)
    {
        //pPattern[Deg] = 0u;
        /* Camshaft */
        if (Deg < pCamCfg->Angle[CamToothIdx][0u])
        {
            if (pCamCfg->IndexPolarity)
            {
                pPattern[Deg] &= ~(EPMGEN_CAM_MSK | EPMGEN_CAM_MSK_FLT);
            }
            else
            {
                pPattern[Deg] |= EPMGEN_CAM_MSK | EPMGEN_CAM_MSK_FLT;
            }
        }
        else if ((Deg >= pCamCfg->Angle[CamToothIdx][0u]) &&
                 (Deg < pCamCfg->Angle[CamToothIdx][1u]))
        {
            if (pCamCfg->IndexPolarity)
            {
                pPattern[Deg] |= EPMGEN_CAM_MSK | EPMGEN_CAM_MSK_FLT;
            }
            else
            {
                pPattern[Deg] &= ~(EPMGEN_CAM_MSK | EPMGEN_CAM_MSK_FLT);
            }
        }
        else
        {
            if (CamToothIdx < pCamCfg->NrOfTeethTotal)
            {
                CamToothIdx++;
            }
            else
            {
                if (pCamCfg->IndexPolarity)
                {
                    pPattern[Deg] &= ~(EPMGEN_CAM_MSK | EPMGEN_CAM_MSK_FLT);
                }
                else
                {
                    pPattern[Deg] |= EPMGEN_CAM_MSK | EPMGEN_CAM_MSK_FLT;
                }
            }
        }

        /* Crankshaft */
        if (((Deg % CrkHalfToothAngle) == 0u) &&
            (Deg != 0u) &&
            (CrkToothIdx < (pCrkCfg->NrOfTeethTotal - pCrkCfg->NrOfTeethMissing)))
        {
            CrkLvl = !CrkLvl;
        }
        pPattern[Deg] = (CrkLvl == true) ? (pPattern[Deg] | EPMGEN_CRK_MSK | EPMGEN_CRK_MSK_FLT) :
                                           (pPattern[Deg] & ~(EPMGEN_CRK_MSK | EPMGEN_CRK_MSK_FLT));

        if ((Deg % (CrkHalfToothAngle * 2u)) == 0u)
        {
            CrkToothIdx++;
            if (CrkToothIdx >= pCrkCfg->NrOfTeethTotal)
            {
                CrkToothIdx = 0u;
            }
        }
    }
}
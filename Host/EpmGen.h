#ifndef EPMGEN_CFG_H
#define EPMGEN_CFG_H

/*************************************************************************************************/
/* Includes                                                                                      */
/*************************************************************************************************/
#include <stdint.h>
#include <stdbool.h>

/*************************************************************************************************/
/* Public defines                                                                                */
/*************************************************************************************************/
#define EPMGEN_CAM_MAX_NR_OF_TEETH  (10u)
#define EPMGEN_SIGNAL_NR_OF_ELEMENTS (7200u)
#define EPMGEN_AG(Ag)   ((uint32_t)((Ag) * 10.0F))
#define EPMGEN_CRK_MSK     (0x08u)
#define EPMGEN_CRK_MSK_FLT (0x00u)
#define EPMGEN_CAM_MSK     (0x20u)
#define EPMGEN_CAM_MSK_FLT (0x00u)
#define EPMGEN_MSK_FLT_ENA (0x00u)

/*************************************************************************************************/
/* Public types                                                                                  */
/*************************************************************************************************/
typedef enum {
    EPMGEN_POL_LOW,
    EPMGEN_POL_HIGH,
} T_EpmGen_Polarity;

typedef struct {
    T_EpmGen_Polarity GapPolarity;
    uint8_t NrOfTeethTotal;
    uint8_t NrOfTeethMissing;
} T_EpmGen_CrkWheelConfig;

typedef struct {
    uint32_t Angle[EPMGEN_CAM_MAX_NR_OF_TEETH][2u];
    T_EpmGen_Polarity IndexPolarity;
    uint8_t NrOfTeethTotal;
} T_EpmGen_CamWheelConfig;

typedef struct {
    T_EpmGen_CrkWheelConfig * pCrkCfg;
    T_EpmGen_CamWheelConfig * pCamCfg;
} T_EpmGen_EngConfig;

typedef struct {
    T_EpmGen_EngConfig * pEngConfig;
    uint8_t NrOfEngConfig;
} T_EpmGen_Config;

extern const T_EpmGen_Config EpmGen_Config;

void SpdGen_GenSig(uint8_t * pPattern, const T_EpmGen_Config *pEngCfg, const uint8_t CfgNr);

#endif /*SPDGEN_CFG_H*/
#ifndef __ADJUSTVOLTAGE_H
#define __ADJUSTVOLTAGE_H

#include "stm32f4xx.h"

#define HIGHSET_HV1 7500
#define LOOWSET_HV1 1000
#define HIGHSET_HV2 7500
#define LOOWSET_HV2 500
#define HIGHSET_CW  500
#define LOOWSET_CW  250

#define PCW_DAC_CLOSE  0xFA
#define NCW_DAC_CLOSE  0xFA
#define VPP1_DAC_CLOSE 0xFFF
#define VNN1_DAC_CLOSE 0xFF
#define VPP2_DAC_CLOSE 0xFFF
#define VNN2_DAC_CLOSE 0xFF

void Adjust_Voltage_Vpp1(uint16_t T_Vpp1);
void Adjust_Voltage_Vpp2(uint16_t T_Vpp2);
void Adjust_Voltage_Vnn1(uint16_t T_Vnn1);
void Adjust_Voltage_Vnn2(uint16_t T_Vnn2);
void Adjust_Voltage_Pcw(uint16_t T_Pcw);
void Adjust_Voltage_Ncw(uint16_t T_Ncw);

void Adjust_Voltage_Vnn1_Vnn2(uint16_t T_Vnn1, uint16_t T_Vnn2); 

void Adjust_Hv_Reset(void);
void Adjust_Cw_Reset(void);
void Adjust_Hv1_Reset(void);
void Adjust_Hv2_Reset(void);

void Adjust_Voltage_HV(void);
void Adjust_Voltage_CW(void);

#endif

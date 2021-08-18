#ifndef __APP_ADJUSTVOLTAGE_H
#define __APP_ADJUSTVOLTAGE_H

#include "gouble.h"

#define RNAGE_MAX_VOL 300               //��ѹ������ѹĿ��Ĳ�ֵ���ڴ�ֵʱ������΢�� 3V
#define RNAGE_MIN_VOL 50                //��ѹ������ѹĿ��Ĳ�ֵС�ڴ�ֵʱ������΢�� 0.5V
#define SWITCH_ADJVOL_MODULE     0      //1���ֲ���ѹ, 0��һ���Ե�ѹ

#define HV_CHANGE_UP_TIMEOUT   100
#define HV_CHANGE_DOWN_TIMEOUT 200

#define STEP_DACMCU_UP       29         //MCU_DAC��ѹ����
#define STEP_DACMCU_DOWN     58         //MCU_DAC��ѹ����
        
#define STEP_SPIMCU_UP       3          //SPI_DAC��ѹ����
#define STEP_SPIMCU_DOWN     6          //SPI_DAC��ѹ����

#define MCU_DAC         1
#define SPI_DAC         2

void App_AdjVol_Task(void);
void App_AdjVol_TaskCreate(void);

void Get_AdjHvMsg(uint8_t *buffer);
void Get_AdjCwMsg(uint8_t *buffer);

#endif

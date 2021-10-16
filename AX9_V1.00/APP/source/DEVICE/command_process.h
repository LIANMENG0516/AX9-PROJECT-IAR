#ifndef __COMMAND_PROCESS_H
#define __COMMAND_PROCESS_H

#include "gouble.h"

typedef struct 
{
	uint8_t		Header;
	uint8_t		Id;
	uint8_t		Cid;
	uint8_t 	Len;
	uint8_t 	*Data;
	uint8_t		Chk;
	uint8_t 	Tail;
}CmdFrameStr;

#define RESPONSE_OK                             1
#define RESPONSE_FAIL                           0

#define CMD_FIRMWARE_VERSION                    0x01
#define CMD_COMPILE_INFO                        0x02
#define CMD_RDHARDWARE_VERSION                  0x03
#define CMD_WDHARDWARE_VERSION                  0x04
#define CMD_SLEEP_EN                            0x05
#define CMS_WEAK_UP                             0x06
#define CMD_RESTART                             0x07
#define CMD_UPGRADE                             0x08
#define CMD_POWER_INFO                          0x09          
#define CMD_FAN_INFO                            0x0A 
#define CMD_TEMPEATURE_INFO                     0x0B       
#define CMD_RDSTATIC_VOLTAGE                    0x0C           
#define CMD_RDADJUST_VOLTAGE                    0x0D            
#define CMD_VPP1VNN1_EN                         0x0E                
#define CMD_VPP1VNN1_DIS                        0x0F             
#define CMD_VPP2VNN2_EN                         0x10              
#define CMD_VPP2VNN2_DIS                        0x11                 
#define CMD_ADJUSTVOLTAGE_HV                    0x12
#define CMD_ADJUSTVOLTAGE_CW                    0x13

#define CMD_EC_COMMUNICATE                      0xA0


#define TIMEOUT                                 0xF0        //调压超时
#define CMD_INVALID                             0xF1        //非法命令

void FrameCmdPackage(uint8_t *pBuf);
void Send_CmdPackage(DMA_Stream_TypeDef* DMAy_Streamx);
void Cmd_Process(void);

ErrorStatus ReceiveFrameAnalysis(uint8_t *pData, uint8_t DataLen);

uint8_t DebugReceiveFrameAnalysis(char *pData);

uint8_t Deal_Compare(char *pData, uint8_t DataLen);

void Calc_TarVol_AlowRange(void);

void Update_EcInfo(void);

#endif












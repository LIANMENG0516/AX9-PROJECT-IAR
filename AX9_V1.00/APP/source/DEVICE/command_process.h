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

#define RESPONSE_OK         0x00


#define FW_VERSION  0x00

#define CMD_FW_VERSION      0x00
#define CHK_COMPILE_INFO    0x01
#define CMD_ADJUST_HV       0x02
#define CMD_ADJUST_CW       0x03
#define CMD_READ_VOLTAGE    0x04
#define CMD_FAN_INFO        0x05
#define CMD_PWR_INFO        0x06
#define CMD_VPP1VNN1_EN     0x07
#define CMD_VPP1VNN1_DIS    0x08
#define CMD_VPP2VNN2_EN     0x09
#define CMD_VPP2VNN2_DIS    0x0A
#define CMD_WRITE_BOARDINFO 0x0B
#define CMD_READ_BOARDINFO  0x0C

#define CMD_EC_COMMUNICATE  0xA0

#define TIMEOUT             0xF0        //调压超时
#define INVALID_CMD         0xF1        //非法命令

void FrameCmdPackage(uint8_t *pBuf);
void Send_CmdPackage(DMA_Stream_TypeDef* DMAy_Streamx);
void Cmd_Process(void);

ErrorStatus ReceiveFrameAnalysis(uint8_t *pData, uint8_t DataLen);

uint8_t DebugReceiveFrameAnalysis(char *pData);

uint8_t Deal_Compare(char *pData, uint8_t DataLen);

void Calc_TarVol_AlowRange(void);

void Update_EcInfo(void);

#endif












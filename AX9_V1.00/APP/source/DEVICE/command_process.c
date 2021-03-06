#include "command_process.h"

extern System_MsgStruct SysMsg;

extern Com_Buffer DebugComRX;
extern Com_Buffer DebugComTX;

extern Com_Buffer CommuComRX;
extern Com_Buffer CommuComTX;

OS_CPU_EXT __ALIGN_BEGIN uint8_t USB_Rx_Buffer[CDC_DATA_MAX_PACKET_SIZE] __ALIGN_END ;
OS_CPU_EXT __ALIGN_BEGIN uint8_t USB_Tx_Buffer[CDC_DATA_MAX_PACKET_SIZE] __ALIGN_END ;
OS_CPU_EXT CDC_IF_Prop_TypeDef VCP_fops;

uint8_t DateStr[11] = __DATE__;
uint8_t TimeStr[8]  = __TIME__;

uint8_t	RcvDataCmd[100];
CmdFrameStr RcvFrameCmd = {0x68, 0x04, 0x00, 0x00, (uint8_t *)RcvDataCmd, 0x00, 0x16};

uint8_t	SenDataCmd[100];
CmdFrameStr SenFrameCmd = {0x68, 0x04, 0x00, 0x00, (uint8_t *)SenDataCmd, 0x00, 0x16};

uint8_t Ec_Info[] = {
                        0x90,                                                               //BoardVersion_H
                        0x00,                                                               //BoardVersion_L
                        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,   //BoardSN
                        0x90,                                                               //FirmwareVersion_H
                        0x00,                                                               //FirmwareVersion_L
                        0x14,                                                               //Temperature_USPSU
                        0x14,                                                               //Temperature_FPGA
                        0x00,                                                               //Fan1Speed_H
                        0x00,                                                               //Fan1Speed_L
                        0x00,                                                               //Fan2Speed_H
                        0x00,                                                               //Fan2Speed_L
                        0x00,                                                               //Fan3Speed_H
                        0x00,                                                               //Fan3Speed_L
                        0x00,                                                               //Fan4Speed_H
                        0x00,                                                               //Fan4Speed_L
                        0x00,                                                               //Fan5Speed_H
                        0x00,                                                               //Fan5Speed_L
                        0xFF,                                                               //VolChk_H
                        0xFF,                                                               //VolChk_L
                        0x04,                                                               //AP12V_H
                        0xB0,                                                               //AP12V_L    
                        0x04,                                                               //AN12V_H
                        0xB0,                                                               //AN12V_L
                        0x02,                                                               //AP5V5_1_H
                        0x26,                                                               //AP5V5_1_L
                        0x02,                                                               //AP5V5_2_H
                        0x26,                                                               //AP5V5_2_L    
                        0x02,                                                               //AN5V5_H
                        0x26,                                                               //AN5V5_L    
                        0x01,                                                               //A3V75_H
                        0x77,                                                               //A3V75_L
                        0x00,                                                               //A2v25_H
                        0xE1,                                                               //A2v25_L
                        0x01,                                                               //D5V_H
                        0xF4,                                                               //D5V_L
                        0x00,                                                               //D1V45_H
                        0x91,                                                               //D1V45_L    
                        0x00,                                                               //D0V95_H
                        0x5F,                                                               //D0V95_L    
                    };

typedef struct
{
    vu16 b0:1;
    vu16 b1:1;
    vu16 b2:1;
    vu16 b3:1;
    vu16 b4:1;
    vu16 b5:1;
    vu16 b6:1;
    vu16 b7:1;
    vu16 b8:1;
    vu16 b9:1;
    vu16 b10:1;
    vu16 b11:1;
    vu16 b12:1;
    vu16 b13:1;
    vu16 b14:1;
    vu16 b15:1;   
}Vol_Def;    
                    
union VolCheck
{
    Vol_Def bit;
    uint16_t volChk;
};

void Update_EcInfo()
{
    union VolCheck VolChk;
    
    VolChk.volChk = 0;
    
    Ec_Info[13] = FirmwareVersion_H;        Ec_Info[14] = FirmwareVersion_L;
    
    Ec_Info[15] = SysMsg.Temperature.MCU;   Ec_Info[16] = SysMsg.Temperature.FPGA;
    
    Ec_Info[17] = SysMsg.Fan.Rpm1 >> 8;     Ec_Info[18] = SysMsg.Fan.Rpm1;
    Ec_Info[19] = SysMsg.Fan.Rpm2 >> 8;     Ec_Info[20] = SysMsg.Fan.Rpm2;
    Ec_Info[21] = SysMsg.Fan.Rpm3 >> 8;     Ec_Info[22] = SysMsg.Fan.Rpm3;
    Ec_Info[23] = SysMsg.Fan.Rpm4 >> 8;     Ec_Info[24] = SysMsg.Fan.Rpm4;
    Ec_Info[25] = SysMsg.Fan.Rpm5 >> 8;     Ec_Info[26] = SysMsg.Fan.Rpm5;

    if(SysMsg.AdjVol.R_AP12V >=1100 && SysMsg.AdjVol.R_AP12V <=1300)        VolChk.bit.b9 = 1; else    VolChk.bit.b9 = 0;
    if(SysMsg.AdjVol.R_AN12V >=1100 && SysMsg.AdjVol.R_AN12V <=1300)        VolChk.bit.b8 = 1; else    VolChk.bit.b8 = 0;
    if(SysMsg.AdjVol.R_AP5V5_1 >=500 && SysMsg.AdjVol.R_AP5V5_1 <=600)      VolChk.bit.b7 = 1; else    VolChk.bit.b7 = 0;
    if(SysMsg.AdjVol.R_AP5V5_2 >=500 && SysMsg.AdjVol.R_AP5V5_2 <=600)      VolChk.bit.b6 = 1; else    VolChk.bit.b6 = 0;
    if(SysMsg.AdjVol.R_AN5V5 >=500 && SysMsg.AdjVol.R_AN5V5 <=600)          VolChk.bit.b5 = 1; else    VolChk.bit.b5 = 0;
    if(SysMsg.AdjVol.R_A3V75 >=300 && SysMsg.AdjVol.R_A3V75 <=450)          VolChk.bit.b4 = 1; else    VolChk.bit.b4 = 0;
    if(SysMsg.AdjVol.R_A2V25 >=180 && SysMsg.AdjVol.R_A2V25 <=270)          VolChk.bit.b3 = 1; else    VolChk.bit.b3 = 0;
    if(SysMsg.AdjVol.R_D5V >=400 && SysMsg.AdjVol.R_D5V <=600)              VolChk.bit.b2 = 1; else    VolChk.bit.b2 = 0;
    if(SysMsg.AdjVol.R_D1V45 >=100 && SysMsg.AdjVol.R_D1V45 <=185)          VolChk.bit.b1 = 1; else    VolChk.bit.b1 = 0;
    if(SysMsg.AdjVol.R_D0V95 >=65 && SysMsg.AdjVol.R_D0V95 <=120)           VolChk.bit.b0 = 1; else    VolChk.bit.b0 = 0;
    
    Ec_Info[27] = VolChk.volChk >> 8;   Ec_Info[28] = VolChk.volChk;
    
    Ec_Info[29] = SysMsg.AdjVol.R_AP12V >> 8;       Ec_Info[30] = SysMsg.AdjVol.R_AP12V;
    Ec_Info[31] = SysMsg.AdjVol.R_AN12V >> 8;       Ec_Info[32] = SysMsg.AdjVol.R_AN12V;
    Ec_Info[33] = SysMsg.AdjVol.R_AP5V5_1 >> 8;     Ec_Info[34] = SysMsg.AdjVol.R_AP5V5_1;
    Ec_Info[35] = SysMsg.AdjVol.R_AP5V5_2 >> 8;     Ec_Info[36] = SysMsg.AdjVol.R_AP5V5_2;
    Ec_Info[37] = SysMsg.AdjVol.R_AN5V5 >> 8;       Ec_Info[38] = SysMsg.AdjVol.R_AN5V5;
    Ec_Info[39] = SysMsg.AdjVol.R_A3V75 >> 8;       Ec_Info[40] = SysMsg.AdjVol.R_A3V75;
    Ec_Info[41] = SysMsg.AdjVol.R_A2V25 >> 8;       Ec_Info[42] = SysMsg.AdjVol.R_A2V25;
    Ec_Info[43] = SysMsg.AdjVol.R_D5V >> 8;         Ec_Info[44] = SysMsg.AdjVol.R_D5V;
    Ec_Info[45] = SysMsg.AdjVol.R_D1V45 >> 8;       Ec_Info[46] = SysMsg.AdjVol.R_D1V45;
    Ec_Info[47] = SysMsg.AdjVol.R_D0V95 >> 8;       Ec_Info[48] = SysMsg.AdjVol.R_D0V95;
}

void Cmd_EcInfo()
{        
    SenFrameCmd.Cid = CMD_EC_COMMUNICATE;
    SenFrameCmd.Len = 49;
    SenFrameCmd.Data = Ec_Info;
        
    if(SysMsg.Cmd.Channel == ECCOM_CHANNEL)
    {
        FrameCmdPackage(CommuComTX.Data);
        Send_CmdPackage(COMMU_COM_DMAY_STREAMX_TX);
    }
    
    if(SysMsg.Cmd.Channel == USB_CHANNEL)
    {
        FrameCmdPackage(USB_Tx_Buffer);
        VCP_fops.pIf_DataTx(USB_Tx_Buffer, (USB_Tx_Buffer[3] + 6));
    }
    
    if(SysMsg.Cmd.Channel == DEBUGCOM_CHANNEL)
    {
        #if DEBUG_COMMAND
        FrameCmdPackage(DebugComTX.Data);
        Send_CmdPackage(DEBUG_COM_DMAY_STREAMX_TX);
        #endif
    }
}

void Cmd_ReadFirmWareVersion()
{
    SenFrameCmd.Cid = CMD_FIRMWARE_VERSION;
    SenFrameCmd.Len = 2;
    SenFrameCmd.Data[0] = FirmwareVersion_H;
    SenFrameCmd.Data[1] = FirmwareVersion_L;
    
    if(SysMsg.Cmd.Channel == ECCOM_CHANNEL)
    {
        FrameCmdPackage(CommuComTX.Data);
        Send_CmdPackage(COMMU_COM_DMAY_STREAMX_TX);
    }
    
    if(SysMsg.Cmd.Channel == USB_CHANNEL)
    {
        FrameCmdPackage(USB_Tx_Buffer);
        VCP_fops.pIf_DataTx(USB_Tx_Buffer, (USB_Tx_Buffer[3] + 6));
    }
    
    if(SysMsg.Cmd.Channel == DEBUGCOM_CHANNEL)
    {
        #if DEBUG_COMMAND
        FrameCmdPackage(DebugComTX.Data);
        Send_CmdPackage(DEBUG_COM_DMAY_STREAMX_TX);
        #endif
    }
}

void Cmd_ReadCompileInfo()
{
    SenFrameCmd.Cid = CMD_COMPILE_INFO;
    SenFrameCmd.Len = sizeof(DateStr) + sizeof(TimeStr);

    memcpy(&SenFrameCmd.Data[0], DateStr, 11);
    memcpy(&SenFrameCmd.Data[11], TimeStr, 8);
    
    if(SysMsg.Cmd.Channel == ECCOM_CHANNEL)
    {
        FrameCmdPackage(CommuComTX.Data);
        Send_CmdPackage(COMMU_COM_DMAY_STREAMX_TX);
    }
    
    if(SysMsg.Cmd.Channel == USB_CHANNEL)
    {
        FrameCmdPackage(USB_Tx_Buffer);
        VCP_fops.pIf_DataTx(USB_Tx_Buffer, (USB_Tx_Buffer[3] + 6));
    }
    
    if(SysMsg.Cmd.Channel == DEBUGCOM_CHANNEL)
    {
        #if DEBUG_COMMAND
        FrameCmdPackage(DebugComTX.Data);
        Send_CmdPackage(DEBUG_COM_DMAY_STREAMX_TX);
        #endif
    }
}

void Cmd_ReadHardWareVersion()
{ 
    SenFrameCmd.Cid = CMD_RDHARDWARE_VERSION;
    SenFrameCmd.Len = 2;

    SenFrameCmd.Data[0] = HardwareVersion_H;
    SenFrameCmd.Data[1] = HardwareVersion_L;
    
    if(SysMsg.Cmd.Channel == ECCOM_CHANNEL)
    {
        FrameCmdPackage(CommuComTX.Data);
        Send_CmdPackage(COMMU_COM_DMAY_STREAMX_TX);
    }
    
    if(SysMsg.Cmd.Channel == USB_CHANNEL)
    {
        FrameCmdPackage(USB_Tx_Buffer);
        VCP_fops.pIf_DataTx(USB_Tx_Buffer, (USB_Tx_Buffer[3] + 6));
    }
    
    if(SysMsg.Cmd.Channel == DEBUGCOM_CHANNEL)
    {
        #if DEBUG_COMMAND
        FrameCmdPackage(DebugComTX.Data);
        Send_CmdPackage(DEBUG_COM_DMAY_STREAMX_TX);
        #endif
    }

}

void Cmd_WriteHardWareVersion()
{        
    Write_UsPowerId(&RcvFrameCmd.Data[0]);
    
    SenFrameCmd.Cid = CMD_WDHARDWARE_VERSION;
    SenFrameCmd.Len = 1;
    SenFrameCmd.Data[0] = RESPONSE_OK;

    if(SysMsg.Cmd.Channel == ECCOM_CHANNEL)
    {
        FrameCmdPackage(CommuComTX.Data);
        Send_CmdPackage(COMMU_COM_DMAY_STREAMX_TX);
    }
    
    if(SysMsg.Cmd.Channel == USB_CHANNEL)
    {
        FrameCmdPackage(USB_Tx_Buffer);
        VCP_fops.pIf_DataTx(USB_Tx_Buffer, (USB_Tx_Buffer[3] + 6));
    }
    
    if(SysMsg.Cmd.Channel == DEBUGCOM_CHANNEL)
    {
        #if DEBUG_COMMAND
        FrameCmdPackage(DebugComTX.Data);
        Send_CmdPackage(DEBUG_COM_DMAY_STREAMX_TX);
        #endif
    }
}


void Cmd_SleepEn()
{
    SenFrameCmd.Cid = CMD_SLEEP_EN;
    SenFrameCmd.Len = 1;
    SenFrameCmd.Data[0] = RESPONSE_OK;
    
    if(SysMsg.Cmd.Channel == ECCOM_CHANNEL)
    {
        FrameCmdPackage(CommuComTX.Data);
        Send_CmdPackage(COMMU_COM_DMAY_STREAMX_TX);
    }
    
    if(SysMsg.Cmd.Channel == USB_CHANNEL)
    {
        FrameCmdPackage(USB_Tx_Buffer);
        VCP_fops.pIf_DataTx(USB_Tx_Buffer, (USB_Tx_Buffer[3] + 6));
    }
    
    if(SysMsg.Cmd.Channel == DEBUGCOM_CHANNEL)
    {
        #if DEBUG_COMMAND
        FrameCmdPackage(DebugComTX.Data);
        Send_CmdPackage(DEBUG_COM_DMAY_STREAMX_TX);
        #endif
    }
}

void Cmd_WeakUp()
{
    SenFrameCmd.Cid = CMS_WEAK_UP;
    SenFrameCmd.Len = 1;
    SenFrameCmd.Data[0] = RESPONSE_OK;
    
    if(SysMsg.Cmd.Channel == ECCOM_CHANNEL)
    {
        FrameCmdPackage(CommuComTX.Data);
        Send_CmdPackage(COMMU_COM_DMAY_STREAMX_TX);
    }
    
    if(SysMsg.Cmd.Channel == USB_CHANNEL)
    {
        FrameCmdPackage(USB_Tx_Buffer);
        VCP_fops.pIf_DataTx(USB_Tx_Buffer, (USB_Tx_Buffer[3] + 6));
    }
    
    if(SysMsg.Cmd.Channel == DEBUGCOM_CHANNEL)
    {
        #if DEBUG_COMMAND
        FrameCmdPackage(DebugComTX.Data);
        Send_CmdPackage(DEBUG_COM_DMAY_STREAMX_TX);
        #endif
    }
}

void Cmd_Restart()
{
    SenFrameCmd.Cid = CMD_RESTART;
    SenFrameCmd.Len = 1;
    SenFrameCmd.Data[0] = RESPONSE_OK;
    
    if(SysMsg.Cmd.Channel == ECCOM_CHANNEL)
    {
        FrameCmdPackage(CommuComTX.Data);
        Send_CmdPackage(COMMU_COM_DMAY_STREAMX_TX);
    }
    
    if(SysMsg.Cmd.Channel == USB_CHANNEL)
    {
        FrameCmdPackage(USB_Tx_Buffer);
        VCP_fops.pIf_DataTx(USB_Tx_Buffer, (USB_Tx_Buffer[3] + 6));
    }
    
    if(SysMsg.Cmd.Channel == DEBUGCOM_CHANNEL)
    {
        #if DEBUG_COMMAND
        FrameCmdPackage(DebugComTX.Data);
        Send_CmdPackage(DEBUG_COM_DMAY_STREAMX_TX);
        #endif
    }
}

void Cmd_Upgrade()
{
    SenFrameCmd.Cid = CMD_UPGRADE;
    SenFrameCmd.Len = 1;
    SenFrameCmd.Data[0] = RESPONSE_OK;
    
    if(SysMsg.Cmd.Channel == ECCOM_CHANNEL)
    {
        FrameCmdPackage(CommuComTX.Data);
        Send_CmdPackage(COMMU_COM_DMAY_STREAMX_TX); 
    }
    
    if(SysMsg.Cmd.Channel == USB_CHANNEL)
    {
        FrameCmdPackage(USB_Tx_Buffer);
        VCP_fops.pIf_DataTx(USB_Tx_Buffer, (USB_Tx_Buffer[3] + 6));
    }
    
    if(SysMsg.Cmd.Channel == DEBUGCOM_CHANNEL)
    {
        #if DEBUG_COMMAND
        FrameCmdPackage(DebugComTX.Data);
        Send_CmdPackage(DEBUG_COM_DMAY_STREAMX_TX);
        #endif
    }
}

void Cmd_ReadPowerInfo()
{
    SenFrameCmd.Cid = CMD_POWER_INFO;
    SenFrameCmd.Len = 7;
    SenFrameCmd.Data[0] = SysMsg.PwrInfo.Ac_Insert;
    SenFrameCmd.Data[1] = SysMsg.PwrInfo.Bat1_Insert;
    SenFrameCmd.Data[2] = SysMsg.PwrInfo.Bat1_Power;
    SenFrameCmd.Data[3] = SysMsg.PwrInfo.Bat1_State;
    SenFrameCmd.Data[4] = SysMsg.PwrInfo.Bat2_Insert;
    SenFrameCmd.Data[5] = SysMsg.PwrInfo.Bat2_Power;
    SenFrameCmd.Data[6] = SysMsg.PwrInfo.Bat2_State;
                
    if(SysMsg.Cmd.Channel == ECCOM_CHANNEL)
    {
        FrameCmdPackage(CommuComTX.Data);
        Send_CmdPackage(COMMU_COM_DMAY_STREAMX_TX);
    }
    
    if(SysMsg.Cmd.Channel == USB_CHANNEL)
    {
        FrameCmdPackage(USB_Tx_Buffer);
        VCP_fops.pIf_DataTx(USB_Tx_Buffer, (USB_Tx_Buffer[3] + 6));
    }
    
    if(SysMsg.Cmd.Channel == DEBUGCOM_CHANNEL)
    {
        #if DEBUG_COMMAND
        FrameCmdPackage(DebugComTX.Data);
        Send_CmdPackage(DEBUG_COM_DMAY_STREAMX_TX);
        #endif
    }
}

void Cmd_ReadFanInfo()
{
    SenFrameCmd.Cid = CMD_FAN_INFO;
    SenFrameCmd.Len = 10;
    SenFrameCmd.Data[0] = SysMsg.Fan.Rpm1 >> 8;
    SenFrameCmd.Data[1] = SysMsg.Fan.Rpm1;
    SenFrameCmd.Data[2] = SysMsg.Fan.Rpm2 >> 8;
    SenFrameCmd.Data[3] = SysMsg.Fan.Rpm2;
    SenFrameCmd.Data[4] = SysMsg.Fan.Rpm3 >> 8;
    SenFrameCmd.Data[5] = SysMsg.Fan.Rpm3;
    SenFrameCmd.Data[6] = SysMsg.Fan.Rpm4 >> 8;
    SenFrameCmd.Data[7] = SysMsg.Fan.Rpm4;
    SenFrameCmd.Data[8] = SysMsg.Fan.Rpm5 >> 8;
    SenFrameCmd.Data[9] = SysMsg.Fan.Rpm5;
    
    if(SysMsg.Cmd.Channel == ECCOM_CHANNEL)
    {
        FrameCmdPackage(CommuComTX.Data);
        Send_CmdPackage(COMMU_COM_DMAY_STREAMX_TX);
    }
    
    if(SysMsg.Cmd.Channel == USB_CHANNEL)
    {
        FrameCmdPackage(USB_Tx_Buffer);
        VCP_fops.pIf_DataTx(USB_Tx_Buffer, (USB_Tx_Buffer[3] + 6));
    }
    
    if(SysMsg.Cmd.Channel == DEBUGCOM_CHANNEL)
    {
        #if DEBUG_COMMAND
        FrameCmdPackage(DebugComTX.Data);
        Send_CmdPackage(DEBUG_COM_DMAY_STREAMX_TX);
        #endif
    }
}
        
void Cmd_ReadTempeatureInfo()
{
    SenFrameCmd.Cid = CMD_TEMPEATURE_INFO;
    SenFrameCmd.Len = 2;
    SenFrameCmd.Data[0] = SysMsg.Temperature.MCU;
    SenFrameCmd.Data[1] = SysMsg.Temperature.FPGA;

    if(SysMsg.Cmd.Channel == ECCOM_CHANNEL)
    {
        FrameCmdPackage(CommuComTX.Data);
        Send_CmdPackage(COMMU_COM_DMAY_STREAMX_TX);
    }
    
    if(SysMsg.Cmd.Channel == USB_CHANNEL)
    {
        FrameCmdPackage(USB_Tx_Buffer);
        VCP_fops.pIf_DataTx(USB_Tx_Buffer, (USB_Tx_Buffer[3] + 6));
    }
    
    if(SysMsg.Cmd.Channel == DEBUGCOM_CHANNEL)
    {
        #if DEBUG_COMMAND
        FrameCmdPackage(DebugComTX.Data);
        Send_CmdPackage(DEBUG_COM_DMAY_STREAMX_TX);
        #endif
    }
}

void Cmd_ReadStaticVoltage()
{
    SenFrameCmd.Cid = CMD_RDSTATIC_VOLTAGE;
    SenFrameCmd.Len = 20;
    
    SenFrameCmd.Data[0]  = SysMsg.AdjVol.R_AP12V >> 8;       SenFrameCmd.Data[1]  = SysMsg.AdjVol.R_AP12V;
    SenFrameCmd.Data[2]  = SysMsg.AdjVol.R_AN12V >> 8;       SenFrameCmd.Data[3]  = SysMsg.AdjVol.R_AN12V;
    SenFrameCmd.Data[4]  = SysMsg.AdjVol.R_AP5V5_1 >> 8;     SenFrameCmd.Data[5]  = SysMsg.AdjVol.R_AP5V5_1;
    SenFrameCmd.Data[6]  = SysMsg.AdjVol.R_AP5V5_2 >> 8;     SenFrameCmd.Data[7]  = SysMsg.AdjVol.R_AP5V5_2;
    SenFrameCmd.Data[8]  = SysMsg.AdjVol.R_AN5V5 >> 8;       SenFrameCmd.Data[9]  = SysMsg.AdjVol.R_AN5V5;
    SenFrameCmd.Data[10] = SysMsg.AdjVol.R_A3V75 >> 8;       SenFrameCmd.Data[11] = SysMsg.AdjVol.R_A3V75;
    SenFrameCmd.Data[12] = SysMsg.AdjVol.R_A2V25 >> 8;       SenFrameCmd.Data[13] = SysMsg.AdjVol.R_A2V25;
    SenFrameCmd.Data[14] = SysMsg.AdjVol.R_D5V >> 8;         SenFrameCmd.Data[15] = SysMsg.AdjVol.R_D5V;
    SenFrameCmd.Data[16] = SysMsg.AdjVol.R_D1V45 >> 8;       SenFrameCmd.Data[17] = SysMsg.AdjVol.R_D1V45;
    SenFrameCmd.Data[18] = SysMsg.AdjVol.R_D0V95 >> 8;       SenFrameCmd.Data[19] = SysMsg.AdjVol.R_D0V95;
    
    if(SysMsg.Cmd.Channel == ECCOM_CHANNEL)
    {
        FrameCmdPackage(CommuComTX.Data);
        Send_CmdPackage(COMMU_COM_DMAY_STREAMX_TX);
    }
    
    if(SysMsg.Cmd.Channel == USB_CHANNEL)
    {
        FrameCmdPackage(USB_Tx_Buffer);
        VCP_fops.pIf_DataTx(USB_Tx_Buffer, (USB_Tx_Buffer[3] + 6));
    }
    
    if(SysMsg.Cmd.Channel == DEBUGCOM_CHANNEL)
    {
        #if DEBUG_COMMAND
        FrameCmdPackage(DebugComTX.Data);
        Send_CmdPackage(DEBUG_COM_DMAY_STREAMX_TX);
        #endif
    }
}

void Cmd_ReadAdjustVoltage()
{
    SenFrameCmd.Cid = CMD_RDADJUST_VOLTAGE;
    SenFrameCmd.Len = 8;   
    SenFrameCmd.Data[0] = SysMsg.AdjVol.R_VPP1 >> 8;
    SenFrameCmd.Data[1] = SysMsg.AdjVol.R_VPP1;
    SenFrameCmd.Data[2] = SysMsg.AdjVol.R_VNN1 >> 8;
    SenFrameCmd.Data[3] = SysMsg.AdjVol.R_VNN1;
    SenFrameCmd.Data[4] = SysMsg.AdjVol.R_VPP2 >> 8;
    SenFrameCmd.Data[5] = SysMsg.AdjVol.R_VPP2;
    SenFrameCmd.Data[6] = SysMsg.AdjVol.R_VNN2 >> 8;
    SenFrameCmd.Data[7] = SysMsg.AdjVol.R_VNN2;

    if(SysMsg.Cmd.Channel == ECCOM_CHANNEL)
    {
        FrameCmdPackage(CommuComTX.Data);
        Send_CmdPackage(COMMU_COM_DMAY_STREAMX_TX);
    }
    
    if(SysMsg.Cmd.Channel == USB_CHANNEL)
    {
        FrameCmdPackage(USB_Tx_Buffer);
        VCP_fops.pIf_DataTx(USB_Tx_Buffer, (USB_Tx_Buffer[3] + 6));
    }
    
    if(SysMsg.Cmd.Channel == DEBUGCOM_CHANNEL)
    {
        #if DEBUG_COMMAND
        FrameCmdPackage(DebugComTX.Data);
        Send_CmdPackage(DEBUG_COM_DMAY_STREAMX_TX);
        #endif
    }   
    
    DEBUG_PRINTF(DEBUG_STRING, "Get Send Voltage : %d %d %d %d \r\n", SysMsg.AdjVol.R_VPP1, SysMsg.AdjVol.R_VNN1, SysMsg.AdjVol.R_VPP2, SysMsg.AdjVol.R_VNN2);
}

void Cmd_Vpp1Vnn1EnableRes()
{
    CTL_VPP1_VNN1_EN(1);  

    SenFrameCmd.Cid = CMD_VPP1VNN1_EN;
    SenFrameCmd.Len = 1;
    SenFrameCmd.Data[0] = RESPONSE_OK;
    
    if(SysMsg.Cmd.Channel == ECCOM_CHANNEL)
    {
        FrameCmdPackage(CommuComTX.Data);
        Send_CmdPackage(COMMU_COM_DMAY_STREAMX_TX);
    }
    
    if(SysMsg.Cmd.Channel == USB_CHANNEL)
    {
        FrameCmdPackage(USB_Tx_Buffer);
        VCP_fops.pIf_DataTx(USB_Tx_Buffer, (USB_Tx_Buffer[3] + 6));
    }
    
    if(SysMsg.Cmd.Channel == DEBUGCOM_CHANNEL)
    {
        #if DEBUG_COMMAND
        FrameCmdPackage(DebugComTX.Data);
        Send_CmdPackage(DEBUG_COM_DMAY_STREAMX_TX);
        #endif
    }
}

void Cmd_Vpp1Vnn1DisableRes()
{
    CTL_VPP1_VNN1_EN(0);

    SenFrameCmd.Cid = CMD_VPP1VNN1_DIS;
    SenFrameCmd.Len = 1;
    SenFrameCmd.Data[0] = RESPONSE_OK;
            
    if(SysMsg.Cmd.Channel == ECCOM_CHANNEL)
    {
        FrameCmdPackage(CommuComTX.Data);
        Send_CmdPackage(COMMU_COM_DMAY_STREAMX_TX);
    }
    
    if(SysMsg.Cmd.Channel == USB_CHANNEL)
    {
        FrameCmdPackage(USB_Tx_Buffer);
        VCP_fops.pIf_DataTx(USB_Tx_Buffer, (USB_Tx_Buffer[3] + 6));
    }
    
    if(SysMsg.Cmd.Channel == DEBUGCOM_CHANNEL)
    {
        #if DEBUG_COMMAND
        FrameCmdPackage(DebugComTX.Data);
        Send_CmdPackage(DEBUG_COM_DMAY_STREAMX_TX);
        #endif
    }
}

void Cmd_Vpp2Vnn2EnableRes()
{
    CTL_VPP2_VNN2_EN(1);
    
    SenFrameCmd.Cid = CMD_VPP2VNN2_EN;
    SenFrameCmd.Len = 1;
    SenFrameCmd.Data[0] = RESPONSE_OK;

    if(SysMsg.Cmd.Channel == ECCOM_CHANNEL)
    {
        FrameCmdPackage(CommuComTX.Data);
        Send_CmdPackage(COMMU_COM_DMAY_STREAMX_TX);
    }
    
    if(SysMsg.Cmd.Channel == USB_CHANNEL)
    {
        FrameCmdPackage(USB_Tx_Buffer);
        VCP_fops.pIf_DataTx(USB_Tx_Buffer, (USB_Tx_Buffer[3] + 6));
    }
    
    if(SysMsg.Cmd.Channel == DEBUGCOM_CHANNEL)
    {
        #if DEBUG_COMMAND
        FrameCmdPackage(DebugComTX.Data);
        Send_CmdPackage(DEBUG_COM_DMAY_STREAMX_TX);
        #endif
    }
}

void Cmd_Vpp2Vnn2DisableRes()
{
    CTL_VPP2_VNN2_EN(0);
    
    SenFrameCmd.Cid = CMD_VPP2VNN2_DIS;
    SenFrameCmd.Len = 1;
    SenFrameCmd.Data[0] = RESPONSE_OK;
            
   if(SysMsg.Cmd.Channel == ECCOM_CHANNEL)
    {
        FrameCmdPackage(CommuComTX.Data);
        Send_CmdPackage(COMMU_COM_DMAY_STREAMX_TX);
    }
    
    if(SysMsg.Cmd.Channel == USB_CHANNEL)
    {
        FrameCmdPackage(USB_Tx_Buffer);
        VCP_fops.pIf_DataTx(USB_Tx_Buffer, (USB_Tx_Buffer[3] + 6));
    }
    
    if(SysMsg.Cmd.Channel == DEBUGCOM_CHANNEL)
    {
        #if DEBUG_COMMAND
        FrameCmdPackage(DebugComTX.Data);
        Send_CmdPackage(DEBUG_COM_DMAY_STREAMX_TX);
        #endif
    }
}

void Cmd_AdjustVoltageHv()
{
#if PID_CTRL
    SysMsg.AdjVol.PidOpen = TRUE;
    Pid_AdjVolHv(RcvFrameCmd.Data);
#else
    Get_AdjHvMsg(RcvFrameCmd.Data);
#endif
}

void Cmd_AdjustVoltageCw()
{
    Get_AdjCwMsg(RcvFrameCmd.Data);
}

void Cmd_InValidData()
{
    SenFrameCmd.Cid = CMD_INVALID;
    SenFrameCmd.Len = 1;
    SenFrameCmd.Data[0] = CMD_INVALID;
        
    if(SysMsg.Cmd.Channel == ECCOM_CHANNEL)
    {
        FrameCmdPackage(CommuComTX.Data);
        Send_CmdPackage(COMMU_COM_DMAY_STREAMX_TX);
    }
    
    if(SysMsg.Cmd.Channel == USB_CHANNEL)
    {
        FrameCmdPackage(USB_Tx_Buffer);
        VCP_fops.pIf_DataTx(USB_Tx_Buffer, (USB_Tx_Buffer[3] + 6));
    }
    
    if(SysMsg.Cmd.Channel == DEBUGCOM_CHANNEL)
    {
        #if DEBUG_COMMAND
        FrameCmdPackage(DebugComTX.Data);
        Send_CmdPackage(DEBUG_COM_DMAY_STREAMX_TX);
        #endif
    }
}


void FrameCmdPackage(uint8_t *pBuf)	//????????
{
	uint8_t i = 0;

	SenFrameCmd.Chk = SenFrameCmd.Id + SenFrameCmd.Cid + SenFrameCmd.Len;
	
	for(i=0; i<SenFrameCmd.Len; i++)
	{
		SenFrameCmd.Chk += SenFrameCmd.Data[i];
	}

	pBuf[0] = SenFrameCmd.Header;
	pBuf[1] = SenFrameCmd.Id;
	pBuf[2] = SenFrameCmd.Cid;
	pBuf[3] = SenFrameCmd.Len;
        for(int i=0; i<SenFrameCmd.Len; i++)
	{
		pBuf[4 + i] = SenFrameCmd.Data[i];
	}
    
	pBuf[SenFrameCmd.Len + 6 - 2] = SenFrameCmd.Chk;
	pBuf[SenFrameCmd.Len + 6 - 1] = SenFrameCmd.Tail;
}

void Send_CmdPackage(DMA_Stream_TypeDef* DMAy_Streamx)	//????????????????????
{
	DMA_SetCurrDataCounter(DMAy_Streamx, SenFrameCmd.Len + 6);	
	DMA_Cmd(DMAy_Streamx, ENABLE);
}

ErrorStatus ReceiveFrameAnalysis(uint8_t *pData, uint8_t DataLen)
{
    uint8_t CmdCrc = 0;
    
    RcvFrameCmd.Header 		= 	*pData++;
    RcvFrameCmd.Id 		= 	*pData++;
    RcvFrameCmd.Cid 		= 	*pData++;
    RcvFrameCmd.Len 		= 	*pData++;
    for(int i=0; i<(DataLen-6); i++)
    {
            RcvFrameCmd.Data[i] = 	*pData++;
    }
    RcvFrameCmd.Chk 		= 	*pData++;
    RcvFrameCmd.Tail 		= 	*pData;
    
    
    CmdCrc += RcvFrameCmd.Id;
    CmdCrc += RcvFrameCmd.Cid;
    CmdCrc += RcvFrameCmd.Len;
    
    for(int i=0; i<RcvFrameCmd.Len; i++)
    {
        CmdCrc += RcvFrameCmd.Data[i];
    }
    
    if(RcvFrameCmd .Header == 0x68 && RcvFrameCmd.Id == 0x04 && RcvFrameCmd.Tail == 0x16 && CmdCrc == RcvFrameCmd.Chk)
    {
        DEBUG_PRINTF(DEBUG_STRING, "Receive Command : %02X %02X %02X %02X ", RcvFrameCmd.Header, RcvFrameCmd.Id, RcvFrameCmd.Cid, RcvFrameCmd.Len);            
        for(int i=0; i<RcvFrameCmd.Len; i++)
        {
            DEBUG_PRINTF(DEBUG_STRING, "%02X ", RcvFrameCmd.Data[i]);
        }
        DEBUG_PRINTF(DEBUG_STRING, "%02X %02X \r\n", RcvFrameCmd.Chk, RcvFrameCmd.Tail);
        
        return SUCCESS;
    }  

    return ERROR;
}

void Cmd_Process()
{
    switch(RcvFrameCmd.Cid)
    {
        case    CMD_EC_COMMUNICATE:             Cmd_EcInfo();                   break;
        case    CMD_FIRMWARE_VERSION:           Cmd_ReadFirmWareVersion();      break;
        case    CMD_COMPILE_INFO:               Cmd_ReadCompileInfo();          break;
        case    CMD_RDHARDWARE_VERSION:         Cmd_ReadHardWareVersion();      break;
        case    CMD_WDHARDWARE_VERSION:         Cmd_WriteHardWareVersion();     break;
        case    CMD_SLEEP_EN:                   Cmd_SleepEn();                  break;
        case    CMS_WEAK_UP:                    Cmd_WeakUp();                   break;
        case    CMD_RESTART:                    Cmd_Restart();                  break;    
        case    CMD_UPGRADE:                    Cmd_Upgrade();                  break;
        case    CMD_POWER_INFO:                 Cmd_ReadPowerInfo();            break;
        case    CMD_FAN_INFO:                   Cmd_ReadFanInfo();              break;
        case    CMD_TEMPEATURE_INFO:            Cmd_ReadTempeatureInfo();       break;
        case    CMD_RDSTATIC_VOLTAGE:           Cmd_ReadStaticVoltage();        break;
        case    CMD_RDADJUST_VOLTAGE:           Cmd_ReadAdjustVoltage();        break;
        case    CMD_VPP1VNN1_EN:                Cmd_Vpp1Vnn1EnableRes();        break;
        case    CMD_VPP1VNN1_DIS:               Cmd_Vpp1Vnn1DisableRes();       break;
        case    CMD_VPP2VNN2_EN:                Cmd_Vpp2Vnn2EnableRes();        break;
        case    CMD_VPP2VNN2_DIS:               Cmd_Vpp2Vnn2DisableRes();       break;
        
        
        
                
         case   CMD_ADJUSTVOLTAGE_HV:
                SysMsg.AdjVol.HvFlag = TRUE;                                    //????????????
                SysMsg.AdjVol.CwFlag = FALSE;
                SysMsg.AdjVol.VolMinitor = FALSE;

                Cmd_AdjustVoltageHv();
                break;
        
        case    CMD_ADJUSTVOLTAGE_CW:
                SysMsg.AdjVol.HvFlag = FALSE;                                   //????????
                SysMsg.AdjVol.CwFlag = TRUE;
                SysMsg.AdjVol.VolMinitor = FALSE;

                Cmd_AdjustVoltageCw();
                break;


                
        

        default:
                Cmd_InValidData();
                break;
    } 
}

/***********************************************************************************************************************************/
#define StringSize 2

char *String[] = {
                    "HV SET",
                    "CW SET",
                 };

uint8_t DebugReceiveFrameAnalysis(char *pData)
{

    for(int i=0; i<StringSize; i++)
    {
        if(strncasecmp(pData, String[i], 6) == 0)
        {
            return i + 1;
        }
    }
    
    return 0;
}

uint8_t Deal_Compare(char *pData, uint8_t DataLen)
{
    uint8_t i = 0;
    
    i = DebugReceiveFrameAnalysis(pData);
    
    switch(i)
    {
        case 1:
                SysMsg.AdjVol.T_VNN1 = SysMsg.AdjVol.T_VPP1 = (pData[7] - '0') * 1000 + (pData[8] - '0') * 100 + (pData[9] - '0') * 10 + (pData[10] - '0');
                
                if(DataLen == 16)
                {
                    SysMsg.AdjVol.T_VNN2 = SysMsg.AdjVol.T_VPP2 = (pData[12] - '0') * 1000 + (pData[13] - '0') * 100 + (pData[14] - '0') * 10 + (pData[15] - '0');  
                }
                if(DataLen == 15)
                {
                    SysMsg.AdjVol.T_VNN2 = SysMsg.AdjVol.T_VPP2 = (pData[12] - '0') * 100 + (pData[13] - '0') * 10 + (pData[14] - '0');
                }
                SysMsg.AdjVol.HvFlag = TRUE;
                Calc_TarVol_AlowRange();                        
                Adjust_Voltage_HV();                                      
                SysMsg.AdjVol.VolMinitor = TRUE;                      
                break;
        case 2: 
                SysMsg.AdjVol.T_VNN1 = SysMsg.AdjVol.T_VPP1 = (pData[7] - '0') * 1000 + (pData[8] - '0') * 100 + (pData[9] - '0') * 10 + (pData[10] - '0');
                SysMsg.AdjVol.T_VNN2 = SysMsg.AdjVol.T_VPP2 = (pData[12] - '0') * 100 + (pData[13] - '0') * 10 + (pData[14] - '0'); 
                
                SysMsg.AdjVol.CwFlag = TRUE;
                Calc_TarVol_AlowRange(); 
                Adjust_Voltage_CW();           
                SysMsg.AdjVol.VolMinitor = TRUE;       
                break;
    }
    
    return i;
}

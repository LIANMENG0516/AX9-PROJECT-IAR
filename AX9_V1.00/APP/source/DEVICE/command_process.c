#include "command_process.h"

extern System_MsgStruct SysMsg;

extern Com_Buffer DebugComRX;
extern Com_Buffer DebugComTX;

extern Com_Buffer CommuComRX;
extern Com_Buffer CommuComTX;

uint8_t BoardInfo[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

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

void Get_EC_Info()
{
    SysMsg.Cmd.EcInfo_Send = TRUE;
}

void Get_FireWare_Version()
{
    SysMsg.Cmd.Firmware_Send = TRUE;
}

void Get_Compile_Info()
{
     SysMsg.Cmd.CompileInfo_Send = TRUE;
}

void Calc_TarVol_AlowRange()
{    
    SysMsg.AdjVol.MAX_VPP1 = SysMsg.AdjVol.T_VPP1 + 150;
    SysMsg.AdjVol.MIN_VPP1 = SysMsg.AdjVol.T_VPP1 - 150;
    SysMsg.AdjVol.MAX_VNN1 = SysMsg.AdjVol.T_VPP1 + 150;
    SysMsg.AdjVol.MIN_VNN1 = SysMsg.AdjVol.T_VPP1 - 150;
    
    if(SysMsg.AdjVol.Adj_HV == TRUE)
    {
        SysMsg.AdjVol.Adj_HV = FALSE;
        
        SysMsg.AdjVol.MAX_VPP2 = SysMsg.AdjVol.T_VPP2 + 150;
        SysMsg.AdjVol.MIN_VPP2 = SysMsg.AdjVol.T_VPP2 - 150;
        SysMsg.AdjVol.MAX_VNN2 = SysMsg.AdjVol.T_VPP2 + 150;
        SysMsg.AdjVol.MIN_VNN2 = SysMsg.AdjVol.T_VPP2 - 150;
    }
    
    if(SysMsg.AdjVol.Adj_CW == TRUE)
    {
        SysMsg.AdjVol.Adj_CW = FALSE;
        
        SysMsg.AdjVol.MAX_VPP2 = SysMsg.AdjVol.T_VPP2 + 100;
        SysMsg.AdjVol.MIN_VPP2 = SysMsg.AdjVol.T_VPP2 - 100;
        SysMsg.AdjVol.MAX_VNN2 = SysMsg.AdjVol.T_VPP2 + 100;
        SysMsg.AdjVol.MIN_VNN2 = SysMsg.AdjVol.T_VPP2 - 100;
    }
}

void Get_AdjHv_Msg()
{
    SysMsg.AdjVol.TimeFlag = TRUE;
    
    SysMsg.AdjVol.T_VPP1 = (RcvFrameCmd.Data[0] << 8) | RcvFrameCmd.Data[1];
    SysMsg.AdjVol.T_VNN1 = (RcvFrameCmd.Data[2] << 8) | RcvFrameCmd.Data[3];
    SysMsg.AdjVol.T_VPP2 = (RcvFrameCmd.Data[4] << 8) | RcvFrameCmd.Data[5];
    SysMsg.AdjVol.T_VNN2 = (RcvFrameCmd.Data[6] << 8) | RcvFrameCmd.Data[7];
    
    SysMsg.AdjVol.Adj_HV = TRUE;
    Calc_TarVol_AlowRange();                                    //计算允许误差范围
    Adjust_Voltage_HV();                                        //执行高压调压处理
    SysMsg.AdjVol.HV_Minitor = TRUE;                            //处理完成打开高压监控 
}

void Get_AdjCw_Msg()
{
    SysMsg.AdjVol.TimeFlag = TRUE;

    SysMsg.AdjVol.T_VPP1 = (RcvFrameCmd.Data[0] << 8) | RcvFrameCmd.Data[1];
    SysMsg.AdjVol.T_VNN1 = (RcvFrameCmd.Data[2] << 8) | RcvFrameCmd.Data[3];
    SysMsg.AdjVol.T_VPP2 = (RcvFrameCmd.Data[4] << 8) | RcvFrameCmd.Data[5];
    SysMsg.AdjVol.T_VNN2 = (RcvFrameCmd.Data[6] << 8) | RcvFrameCmd.Data[7];
    
    SysMsg.AdjVol.Adj_CW = TRUE;
    Calc_TarVol_AlowRange(); 
    Adjust_Voltage_CW();                                        //执行低压调压处理
    SysMsg.AdjVol.CW_Minitor = TRUE;                            //处理完成打开低压监控
}

void Get_Voltage_Msg()
{
    SysMsg.Cmd.Voltage_Send = TRUE;
}

void Get_Fan_Info()
{
    SysMsg.Cmd.FanInfo_Send = TRUE;
}

void Get_Pwr_Info()
{
    SysMsg.Cmd.PwrInfo_Send = TRUE;
}

void Get_Vpp1Vnn1EnResponse()
{
    CTL_VPP1_VNN1_EN(1);  
    SysMsg.Cmd.Vpp1Vnn1En_Send = TRUE;    
}

void Get_Vpp1Vnn1DisResponse()
{
    CTL_VPP1_VNN1_EN(0);
    SysMsg.Cmd.Vpp1Vnn1Dis_Send = TRUE;
}

void Get_Vpp2Vnn2EnResponse()
{
    CTL_VPP2_VNN2_EN(1);
    SysMsg.Cmd.Vpp2Vnn2En_Send = TRUE;
}

void Get_Vpp2Vnn2DisResponse()
{
    CTL_VPP2_VNN2_EN(0);
    SysMsg.Cmd.Vpp2Vnn2Dis_Send = TRUE;
}

void InValid_CidData()
{
	SenFrameCmd.Len = 1;
	SenFrameCmd.Data[0] = INVALID_CMD;
}

void Write_BowarInfo()
{    
    for(uint8_t i=0; i<13; i++)
    {
        BoardInfo[i] = RcvFrameCmd.Data[i];
    }

    DS2431_WriteData(0, BoardInfo);
    
    SysMsg.Cmd.WriteBoardOk_Send = TRUE;
}

void Read_BowarInfo()
{
    SysMsg.Cmd.BoardInfo_Send = TRUE;

}

void FrameCmdPackage(uint8_t *pBuf)	//数据打包
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

void Send_CmdPackage(DMA_Stream_TypeDef* DMAy_Streamx)	//发送已经打包好的命令
{
	DMA_SetCurrDataCounter(DMAy_Streamx, SenFrameCmd.Len + 6);	
	DMA_Cmd(DMAy_Streamx, ENABLE);
}

ErrorStatus ReceiveFrameAnalysis(uint8_t *pData, uint8_t DataLen)
{
    uint8_t CmdCrc = 0;
    
	RcvFrameCmd.Header 		= 	*pData++;
	RcvFrameCmd.Id 			= 	*pData++;
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
        return SUCCESS;
    }  

    return ERROR;
}

void Cmd_Process()
{
    switch(RcvFrameCmd.Cid)
    {
        case    CMD_EC_COMMUNICATE:
                Get_EC_Info();
                break;
        
        case    CMD_FW_VERSION:
                Get_FireWare_Version();
                break;
        
        case    CHK_COMPILE_INFO:
                Get_Compile_Info();
                break;

        case    CMD_ADJUST_HV:
                Get_AdjHv_Msg();
                break;
        
        case    CMD_ADJUST_CW:
                Get_AdjCw_Msg();
                break;
        
        case    CMD_READ_VOLTAGE:
                Get_Voltage_Msg();
                break;
        
        case    CMD_FAN_INFO:
                Get_Fan_Info();
                break;
        
        case    CMD_PWR_INFO:
                Get_Pwr_Info();
                break;
        
        case    CMD_VPP1VNN1_EN:
                Get_Vpp1Vnn1EnResponse();
                break;
        
        case    CMD_VPP1VNN1_DIS:
                Get_Vpp1Vnn1DisResponse();
                break;
        
        case    CMD_VPP2VNN2_EN:
                Get_Vpp2Vnn2EnResponse();
                break;
        case    CMD_VPP2VNN2_DIS:
                Get_Vpp2Vnn2DisResponse();
                break;
                
        case    CMD_WRITE_BOARDINFO:
                Write_BowarInfo();
                break;
        
        case    CMD_READ_BOARDINFO:
                Read_BowarInfo();
                break;

        default:
                InValid_CidData();
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
                SysMsg.AdjVol.Adj_HV = TRUE;
                Calc_TarVol_AlowRange();                        
                Adjust_Voltage_HV();                                      
                SysMsg.AdjVol.HV_Minitor = TRUE;                      
                break;
        case 2: 
                SysMsg.AdjVol.T_VNN1 = SysMsg.AdjVol.T_VPP1 = (pData[7] - '0') * 1000 + (pData[8] - '0') * 100 + (pData[9] - '0') * 10 + (pData[10] - '0');
                SysMsg.AdjVol.T_VNN2 = SysMsg.AdjVol.T_VPP2 = (pData[12] - '0') * 100 + (pData[13] - '0') * 10 + (pData[14] - '0'); 
                
                SysMsg.AdjVol.Adj_CW = TRUE;
                Calc_TarVol_AlowRange(); 
                Adjust_Voltage_CW();           
                SysMsg.AdjVol.CW_Minitor= TRUE;       
                break;
    }
    
    return i;
}

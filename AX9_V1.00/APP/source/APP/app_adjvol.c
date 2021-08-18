#include "app_adjvol.h"

OS_TCB AdjVolTcb;

CPU_STK App_AdjVol_Task_Stk[APP_ADJVOL_STK_SIZE];

OS_CPU_EXT __ALIGN_BEGIN uint8_t USB_Tx_Buffer[CDC_DATA_MAX_PACKET_SIZE] __ALIGN_END;
OS_CPU_EXT CDC_IF_Prop_TypeDef VCP_fops;

extern System_MsgStruct SysMsg;
extern CmdFrameStr SenFrameCmd;
extern Com_Buffer DebugComTX;

void AdjVol_Result_Send()
{
    SenFrameCmd.Cid = CMD_ADJUST_HV;
    SenFrameCmd.Len = 8;
    
    SenFrameCmd.Data[0] = SysMsg.AdjVol.R_VPP1 >> 8;
    SenFrameCmd.Data[1] = SysMsg.AdjVol.R_VPP1;
    SenFrameCmd.Data[2] = SysMsg.AdjVol.R_VNN1 >> 8;
    SenFrameCmd.Data[3] = SysMsg.AdjVol.R_VNN1;
    SenFrameCmd.Data[4] = SysMsg.AdjVol.R_VPP2 >> 8;
    SenFrameCmd.Data[5] = SysMsg.AdjVol.R_VPP2;
    SenFrameCmd.Data[6] = SysMsg.AdjVol.R_VNN2 >> 8;
    SenFrameCmd.Data[7] = SysMsg.AdjVol.R_VNN2;

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
    
    DEBUG_PRINTF(DEBUG_STRING, "AdjVol Time : %d \r\n", SysMsg.AdjVol.Time);            
    DEBUG_PRINTF(DEBUG_STRING, "HV Send Voltage : %d %d %d %d \r\n", SysMsg.AdjVol.R_VPP1, SysMsg.AdjVol.R_VNN1, SysMsg.AdjVol.R_VPP2, SysMsg.AdjVol.R_VNN2);            

    
    SysMsg.AdjVol.TimeFlag = FALSE;
    SysMsg.AdjVol.Time = 0;

}

void Calc_TarVol_AlowRange()
{    
    SysMsg.AdjVol.MAX_VPP1 = SysMsg.AdjVol.T_VPP1 + 150;
    SysMsg.AdjVol.MIN_VPP1 = SysMsg.AdjVol.T_VPP1 - 150;
    SysMsg.AdjVol.MAX_VNN1 = SysMsg.AdjVol.T_VPP1 + 150;
    SysMsg.AdjVol.MIN_VNN1 = SysMsg.AdjVol.T_VPP1 - 150;
    
    if(SysMsg.AdjVol.HvFlag == TRUE)
    {
        SysMsg.AdjVol.HvFlag = FALSE;
        
        SysMsg.AdjVol.MAX_VPP2 = SysMsg.AdjVol.T_VPP2 + 150;
        SysMsg.AdjVol.MIN_VPP2 = SysMsg.AdjVol.T_VPP2 - 150;
        SysMsg.AdjVol.MAX_VNN2 = SysMsg.AdjVol.T_VPP2 + 150;
        SysMsg.AdjVol.MIN_VNN2 = SysMsg.AdjVol.T_VPP2 - 150;
    }
    
    if(SysMsg.AdjVol.CwFlag == TRUE)
    {
        SysMsg.AdjVol.CwFlag = FALSE;
        
        SysMsg.AdjVol.MAX_VPP2 = SysMsg.AdjVol.T_VPP2 + 100;
        SysMsg.AdjVol.MIN_VPP2 = SysMsg.AdjVol.T_VPP2 - 100;
        SysMsg.AdjVol.MAX_VNN2 = SysMsg.AdjVol.T_VNN2 + 100;
        SysMsg.AdjVol.MIN_VNN2 = SysMsg.AdjVol.T_VNN2 - 100;
    }
}

uint16_t Step_HvAdjVol_Calcuation(uint16_t Target, uint16_t Precent, u8 DacState)       //步进调压值计算
{
    uint16_t temp = 0, stepUp = 0, stepDown = 0;
    
    if(DacState == MCU_DAC)                                                             //片内MCU_DAC调节步进值
    {
        stepUp = STEP_DACMCU_UP;
        stepDown = STEP_DACMCU_DOWN;
    }
    else                                                                                //片外SPI_DAC调节步进值
    {
        stepUp = STEP_SPIMCU_UP;
        stepDown = STEP_SPIMCU_DOWN;
    }
  
    if(Target > Precent)
    {
        if(Target - Precent > stepUp)  
        {
            temp = Precent + stepUp;
        }
        else
        {
            temp = Target;
        }
    }
    else if(Precent > Target)
    {
        if(Precent - Target > stepDown)
        {
            temp = Precent - stepDown;
        }
        else
        {
            temp = Target;
        }
    }
    else
    {
        temp = Target;
    }

    return temp;
}

void Get_AdjHvMsg(uint8_t *buffer)
{         
    SysMsg.AdjVol.T_VPP1 = (buffer[0] << 8) | buffer[1];                                                    //获取目标电压
    SysMsg.AdjVol.T_VNN1 = (buffer[2] << 8) | buffer[3];
    SysMsg.AdjVol.T_VPP2 = (buffer[4] << 8) | buffer[5];
    SysMsg.AdjVol.T_VNN2 = (buffer[6] << 8) | buffer[7];
    
    if(SysMsg.AdjVol.T_VPP1 <= HIGHSET_HV1 && SysMsg.AdjVol.T_VPP1 >= LOOWSET_HV1 &&                                            //在允许调压范围之内
       SysMsg.AdjVol.T_VNN1 <= HIGHSET_HV1 && SysMsg.AdjVol.T_VNN1 >= LOOWSET_HV1 && 
       SysMsg.AdjVol.T_VPP2 <= HIGHSET_HV2 && SysMsg.AdjVol.T_VPP2 >= LOOWSET_HV2 && 
       SysMsg.AdjVol.T_VNN2 <= HIGHSET_HV2 && SysMsg.AdjVol.T_VNN2 >= LOOWSET_HV2 )
    {
      
        if(SysMsg.AdjVol.T_VPP1 != SysMsg.AdjVol.Old_T_VPP1 || SysMsg.AdjVol.T_VNN1 != SysMsg.AdjVol.Old_T_VNN1)                //本次调压目标值与上次不同
        {
              
             SysMsg.AdjVol.HV1NeedChange = TRUE;                                                                                //HV1需要调压
        }
        
        if(SysMsg.AdjVol.T_VPP2 != SysMsg.AdjVol.Old_T_VPP2 || SysMsg.AdjVol.T_VNN2 != SysMsg.AdjVol.Old_T_VNN2)                //本次调压目标值与上次不同
        {
            
            SysMsg.AdjVol.HV2NeedChange = TRUE;                                                                                  //HV2需要调压
        }
        
        if(SysMsg.AdjVol.HV1NeedChange == TRUE || SysMsg.AdjVol.HV2NeedChange == TRUE)
        {
            if(SysMsg.AdjVol.T_VPP1 >= SysMsg.AdjVol.Old_T_VPP1 && SysMsg.AdjVol.T_VNN1 >= SysMsg.AdjVol.Old_T_VNN1 &&           //升降压判断           
               SysMsg.AdjVol.T_VPP2 >= SysMsg.AdjVol.Old_T_VPP2 && SysMsg.AdjVol.T_VNN2 >= SysMsg.AdjVol.Old_T_VNN2  )
            {
                DEBUG_PRINTF(DEBUG_STRING, "Up Vol \r\n");            

                SysMsg.AdjVol.TimeOut = HV_CHANGE_UP_TIMEOUT;                                                                    //本次调压为升压
            }
            else
            {
                DEBUG_PRINTF(DEBUG_STRING, "Down Vol \r\n");
                SysMsg.AdjVol.TimeOut = HV_CHANGE_DOWN_TIMEOUT;                                                                  //本次调压为降压
            }
            
            SysMsg.AdjVol.Old_T_VPP1 = SysMsg.AdjVol.T_VPP1;    
            SysMsg.AdjVol.Old_T_VNN1 = SysMsg.AdjVol.T_VNN1; 
            SysMsg.AdjVol.Old_T_VPP2 = SysMsg.AdjVol.T_VPP2;    
            SysMsg.AdjVol.Old_T_VNN2 = SysMsg.AdjVol.T_VNN2;
            
            Calc_TarVol_AlowRange();                                                                                             //计算允许输出误差范围
            
            SysMsg.AdjVol.T_McuDacHv1 = Vpp_Calculate_AdjVol(SysMsg.AdjVol.T_VPP1);                                              //计算要调节到目标电压时, 目标DAC的值
            SysMsg.AdjVol.T_SpiDacHv1 = Vnn_Calculate_AdjVol(SysMsg.AdjVol.T_VNN1);
            SysMsg.AdjVol.T_McuDacHv2 = Vpp_Calculate_AdjVol(SysMsg.AdjVol.T_VPP2);                
            SysMsg.AdjVol.T_SpiDacHv2 = Vnn_Calculate_AdjVol(SysMsg.AdjVol.T_VNN2);
        
            DEBUG_PRINTF(DEBUG_STRING, "DacVol Target：%d %d %d %d \r\n", SysMsg.AdjVol.T_McuDacHv1, SysMsg.AdjVol.T_SpiDacHv1, SysMsg.AdjVol.T_McuDacHv2, SysMsg.AdjVol.T_SpiDacHv2);  
        
        
            #if SWITCH_ADJVOL_MODULE
        
            while(SysMsg.AdjVol.HV1NeedChange == TRUE || SysMsg.AdjVol.HV2NeedChange == TRUE)
            {
                if(SysMsg.AdjVol.HV1NeedChange == TRUE)
                {
                    SysMsg.AdjVol.P_McuDacHv1 = Step_HvAdjVol_Calcuation(SysMsg.AdjVol.T_McuDacHv1, SysMsg.AdjVol.P_McuDacHv1, MCU_DAC);  
                    SysMsg.AdjVol.P_SpiDacHv1 = Step_HvAdjVol_Calcuation(SysMsg.AdjVol.T_SpiDacHv1, SysMsg.AdjVol.P_SpiDacHv1, SPI_DAC);  
                  
                    DAC_SetChannel1Data(DAC_Align_12b_R, SysMsg.AdjVol.P_McuDacHv1);                                                //调节VPP1至目标值
                    DAC_SoftwareTriggerCmd(DAC_Channel_1, ENABLE);                                                                  //软件触发DAC转换
                    
                    DacHv_Tlv5626cd_ValueSet(SysMsg.AdjVol.P_SpiDacHv1, SysMsg.AdjVol.T_SpiDacHv2);                                 //触发VNN1转换

                    if(SysMsg.AdjVol.P_McuDacHv1 == SysMsg.AdjVol.T_McuDacHv1 && SysMsg.AdjVol.P_SpiDacHv1 == SysMsg.AdjVol.T_SpiDacHv1)                                    //调节完成，不在需要调节
                    {
                        SysMsg.AdjVol.HV1NeedChange = FALSE;
                    }
                }
                
                if(SysMsg.AdjVol.HV2NeedChange == TRUE)
                {
                    SysMsg.AdjVol.P_McuDacHv2 = Step_HvAdjVol_Calcuation(SysMsg.AdjVol.T_McuDacHv2, SysMsg.AdjVol.P_McuDacHv2, MCU_DAC);  
                    SysMsg.AdjVol.P_SpiDacHv2 = Step_HvAdjVol_Calcuation(SysMsg.AdjVol.T_SpiDacHv2, SysMsg.AdjVol.P_SpiDacHv2, SPI_DAC);
                
                    DAC_SetChannel2Data(DAC_Align_12b_R, SysMsg.AdjVol.T_McuDacHv2);                                                //调节VPP2至目标值
                    DAC_SoftwareTriggerCmd(DAC_Channel_2, ENABLE);                                                                  //软件触发DAC转换
                    
                    DacHv_Tlv5626cd_ValueSet(SysMsg.AdjVol.P_SpiDacHv1, SysMsg.AdjVol.T_SpiDacHv2);                                 //触发VNN2转换

                    if(SysMsg.AdjVol.P_McuDacHv2 == SysMsg.AdjVol.T_McuDacHv2 && SysMsg.AdjVol.P_SpiDacHv2 == SysMsg.AdjVol.T_SpiDacHv2)                                    //调节完成，不在需要调节
                    {
                        SysMsg.AdjVol.HV2NeedChange = FALSE;
                    }
                }
                
                Delay_ms(1);
                
                DEBUG_PRINTF(DEBUG_STRING, "DacVol Precent：%d %d %d %d \r\n", SysMsg.AdjVol.P_McuDacHv1, SysMsg.AdjVol.P_SpiDacHv1, SysMsg.AdjVol.P_McuDacHv2, SysMsg.AdjVol.P_SpiDacHv2);  
            }       
            
            #else
            
            Adjust_Voltage_HV();                                                                    //执行高压调压处理, 该调压处理为硬件一次性调压
            
            #endif
        
        
            SysMsg.AdjVol.AdjVolSuccess = FALSE;                                    //清零单次调压成功标志
            SysMsg.AdjVol.AdjVolOpen = TRUE;                                        //调压开启
            SysMsg.AdjVol.VolMinitor = FALSE;                                       //关闭监控
            SysMsg.AdjVol.TimeFlag = TRUE;                                          //开始计时
            SysMsg.AdjVol.MinAdjVolCnt = 0;                                         //微调次数
        } 
    }  
}

void Get_AdjCwMsg(uint8_t *buffer)
{
    SysMsg.AdjVol.T_VPP1 = (buffer[0] << 8) | buffer[1];
    SysMsg.AdjVol.T_VNN1 = (buffer[2] << 8) | buffer[3];
    SysMsg.AdjVol.T_VPP2 = (buffer[4] << 8) | buffer[5];
    SysMsg.AdjVol.T_VNN2 = (buffer[6] << 8) | buffer[7];
    
    if(SysMsg.AdjVol.T_VPP1 <= HIGHSET_HV1 && SysMsg.AdjVol.T_VPP1 >= LOOWSET_HV1 &&                                            //在允许调压范围之内
       SysMsg.AdjVol.T_VNN1 <= HIGHSET_HV1 && SysMsg.AdjVol.T_VNN1 >= LOOWSET_HV1 && 
       SysMsg.AdjVol.T_VPP2 <= HIGHSET_CW  && SysMsg.AdjVol.T_VPP2 >= LOOWSET_CW  && 
       SysMsg.AdjVol.T_VNN2 <= HIGHSET_CW  && SysMsg.AdjVol.T_VNN2 >= LOOWSET_CW  )
    {
    
        if(SysMsg.AdjVol.T_VPP1 != SysMsg.AdjVol.Old_T_VPP1 || SysMsg.AdjVol.T_VNN1 != SysMsg.AdjVol.Old_T_VNN1)                //本次调压目标值与上次不同
        {
             SysMsg.AdjVol.HV1NeedChange = TRUE;                                                                                //HV1需要调压
        }
        
        if(SysMsg.AdjVol.T_VPP2 != SysMsg.AdjVol.Old_T_VPP2 || SysMsg.AdjVol.T_VNN2 != SysMsg.AdjVol.Old_T_VNN2)                    //本次调压目标值与上次不同
        {
            SysMsg.AdjVol.HV2NeedChange = TRUE;                                                                                  //HV2需要调压
        }
    
        if(SysMsg.AdjVol.HV1NeedChange == TRUE || SysMsg.AdjVol.HV2NeedChange == TRUE)
        {
            if(SysMsg.AdjVol.T_VPP1 >= SysMsg.AdjVol.Old_T_VPP1 && SysMsg.AdjVol.T_VNN1 >= SysMsg.AdjVol.Old_T_VNN1 &&           //升降压判断           
               SysMsg.AdjVol.T_VPP2 >= SysMsg.AdjVol.Old_T_VPP2 && SysMsg.AdjVol.T_VNN2 >= SysMsg.AdjVol.Old_T_VNN2 )
            {
                DEBUG_PRINTF(DEBUG_STRING, "Up Vol \r\n");            

                SysMsg.AdjVol.TimeOut = HV_CHANGE_UP_TIMEOUT;                                                                    //本次调压为升压
            }
            else
            {
                DEBUG_PRINTF(DEBUG_STRING, "Down Vol \r\n");
                SysMsg.AdjVol.TimeOut = HV_CHANGE_DOWN_TIMEOUT;                                                                   //本次调压为降压
            }
            
            SysMsg.AdjVol.Old_T_VPP1 = SysMsg.AdjVol.T_VPP1;    
            SysMsg.AdjVol.Old_T_VNN1 = SysMsg.AdjVol.T_VNN1; 
            SysMsg.AdjVol.Old_T_VPP2 = SysMsg.AdjVol.T_VPP2;    
            SysMsg.AdjVol.Old_T_VNN2 = SysMsg.AdjVol.T_VNN2;
            
            Calc_TarVol_AlowRange();                                                                                                //计算允许输出误差范围
            
            SysMsg.AdjVol.T_McuDacHv1 = Vpp_Calculate_AdjVol(SysMsg.AdjVol.T_VPP1);                                                  //计算要调节到目标电压时, 目标DAC的值
            SysMsg.AdjVol.T_SpiDacHv1 = Vnn_Calculate_AdjVol(SysMsg.AdjVol.T_VNN1);
            SysMsg.AdjVol.T_SpiDacPcw = Pcw_Calculate_AdjVol(SysMsg.AdjVol.T_VPP2);
            SysMsg.AdjVol.T_SpiDacNcw = Ncw_Calculate_AdjVol(SysMsg.AdjVol.T_VNN2);
            
            
            DEBUG_PRINTF(DEBUG_STRING, "DacVol Target：%d %d %d %d \r\n", SysMsg.AdjVol.T_McuDacHv1, SysMsg.AdjVol.T_SpiDacHv1, SysMsg.AdjVol.T_SpiDacPcw, SysMsg.AdjVol.T_SpiDacNcw);  
        }
        
        #if SWITCH_ADJVOL_MODULE
        while(SysMsg.AdjVol.HV1NeedChange == TRUE || SysMsg.AdjVol.HV2NeedChange == TRUE)
        {
            if(SysMsg.AdjVol.HV1NeedChange == TRUE)
            {
                SysMsg.AdjVol.P_McuDacHv1 = Step_HvAdjVol_Calcuation(SysMsg.AdjVol.T_McuDacHv1, SysMsg.AdjVol.P_McuDacHv1, MCU_DAC);  
                SysMsg.AdjVol.P_SpiDacHv1 = Step_HvAdjVol_Calcuation(SysMsg.AdjVol.T_SpiDacHv1, SysMsg.AdjVol.P_SpiDacHv1, SPI_DAC);  
              
                DAC_SetChannel1Data(DAC_Align_12b_R, SysMsg.AdjVol.P_McuDacHv1);                                                //调节VPP1至目标值
                DAC_SoftwareTriggerCmd(DAC_Channel_1, ENABLE);                                                                  //软件触发DAC转换
                
                DacHv_Tlv5626cd_ValueSet(SysMsg.AdjVol.P_SpiDacHv1, SysMsg.AdjVol.P_SpiDacHv2);                                 //触发VNN1转换
                
                if(SysMsg.AdjVol.P_McuDacHv1 == SysMsg.AdjVol.T_McuDacHv1 && SysMsg.AdjVol.P_SpiDacHv1 == SysMsg.AdjVol.T_SpiDacHv1)                                    //调节完成，不在需要调节
                {
                    SysMsg.AdjVol.HV1NeedChange = FALSE;
                }
            }
            
            if(SysMsg.AdjVol.HV2NeedChange == TRUE)
            {
                SysMsg.AdjVol.P_SpiDacPcw = Step_HvAdjVol_Calcuation(SysMsg.AdjVol.T_SpiDacPcw, SysMsg.AdjVol.P_SpiDacPcw, SPI_DAC);  
                SysMsg.AdjVol.P_SpiDacNcw = Step_HvAdjVol_Calcuation(SysMsg.AdjVol.T_SpiDacNcw, SysMsg.AdjVol.P_SpiDacNcw, SPI_DAC);

                DacCw_Tlv5626cd_ValueSet(SysMsg.AdjVol.P_SpiDacPcw, SysMsg.AdjVol.P_SpiDacNcw);                                 //触发PCW、NCW转换
                
                if(SysMsg.AdjVol.P_SpiDacPcw == SysMsg.AdjVol.T_SpiDacPcw && SysMsg.AdjVol.P_SpiDacNcw == SysMsg.AdjVol.T_SpiDacNcw)                                    //调节完成，不在需要调节
                {
                    SysMsg.AdjVol.HV2NeedChange = FALSE;
                }
            }
            
            Delay_ms(1);
            
            DEBUG_PRINTF(DEBUG_STRING, "DacVol Precent：%d %d %d %d \r\n", SysMsg.AdjVol.P_McuDacHv1, SysMsg.AdjVol.P_SpiDacHv1, SysMsg.AdjVol.P_SpiDacPcw, SysMsg.AdjVol.P_SpiDacNcw);  
        }  
        
        #else
        
        Adjust_Voltage_CW();                                                    //执行低压调压处理
        
        #endif
        
        SysMsg.AdjVol.AdjVolOpen = TRUE;                                        //调压开启
        SysMsg.AdjVol.VolMinitor = FALSE;
        SysMsg.AdjVol.TimeFlag = TRUE;                                          //开始计时 
        SysMsg.AdjVol.MinAdjVolCnt = 0;
    }
}

void App_AdjVol_Task()
{
    OS_ERR err;

    while(1)
    {		
        if(SysMsg.AdjVol.MinAdjVolOpen)                                       
        {
            if(++SysMsg.AdjVol.MinAdjVolCnt >= 6)
            {
                SysMsg.AdjVol.MinAdjVolCnt = 6;
                SysMsg.AdjVol.MinAdjVolOpen = FALSE;                            //微调结束
                SysMsg.AdjVol.VolMinitor = TRUE;                                //开启监控
                DEBUG_PRINTF(DEBUG_STRING, "Exit MinAdjVol \r\n");             
            } 
            else
            {
                if(abs(SysMsg.AdjVol.R_VPP1 - SysMsg.AdjVol.T_VPP1) > RNAGE_MIN_VOL)           
                {
                    int16_t DesVol;
                    
                    DesVol = SysMsg.AdjVol.R_VPP1 - SysMsg.AdjVol.T_VPP1;
                    
                    if(DesVol > 0)
                    {
                        Adjust_Voltage_Vpp1(SysMsg.AdjVol.T_VPP1 - DesVol);
                    }
                    
                    if(DesVol < 0)
                    {
                        Adjust_Voltage_Vpp1(SysMsg.AdjVol.T_VPP1 + DesVol);
                    }
                }
                
                if(abs(SysMsg.AdjVol.R_VNN1 - SysMsg.AdjVol.T_VNN1) > RNAGE_MIN_VOL)           
                {
                    int16_t DesVol;
                    
                    DesVol = SysMsg.AdjVol.R_VNN1 - SysMsg.AdjVol.T_VNN1;
                    
                    if(DesVol > 0)
                    {
                        Adjust_Voltage_Vnn1_Vnn2(SysMsg.AdjVol.T_VNN1  - DesVol, SysMsg.AdjVol.T_VNN2);
                    }
                    if(DesVol < 0)
                    {
                        Adjust_Voltage_Vnn1_Vnn2(SysMsg.AdjVol.T_VNN1  + DesVol, SysMsg.AdjVol.T_VNN2);
                    }
                }
                
                if(SysMsg.AdjVol.HvFlag)
                {                
                    if(abs(SysMsg.AdjVol.R_VPP2 - SysMsg.AdjVol.T_VPP2) > RNAGE_MIN_VOL)           
                    {
                        int16_t DesVol;
                        
                        DesVol = SysMsg.AdjVol.R_VPP2 - SysMsg.AdjVol.T_VPP2;
                        
                        if(DesVol > 0)
                        {
                            Adjust_Voltage_Vpp2(SysMsg.AdjVol.T_VPP2 - DesVol);
                        }
                        
                        if(DesVol < 0)
                        {
                            Adjust_Voltage_Vpp2(SysMsg.AdjVol.T_VPP2 + DesVol);
                        }
                    }

                    if(abs(SysMsg.AdjVol.R_VNN2 - SysMsg.AdjVol.T_VNN2) > RNAGE_MIN_VOL)           
                    {
                        int16_t DesVol;
                        
                        DesVol = SysMsg.AdjVol.R_VNN2 - SysMsg.AdjVol.T_VNN2;
                        
                        if(DesVol > 0)
                        {
                            Adjust_Voltage_Vnn1_Vnn2(SysMsg.AdjVol.T_VNN1, SysMsg.AdjVol.T_VNN2 - DesVol);
                        }
                        if(DesVol < 0)
                        {
                            Adjust_Voltage_Vnn1_Vnn2(SysMsg.AdjVol.T_VNN1, SysMsg.AdjVol.T_VNN2 + DesVol);
                        }
                    }
                }
                
                if(SysMsg.AdjVol.CwFlag)
                {
                    if(abs(SysMsg.AdjVol.R_VPP2 - SysMsg.AdjVol.T_VPP2) > RNAGE_MIN_VOL)           
                    {
                        int16_t DesVol;
                        
                        DesVol = SysMsg.AdjVol.R_VPP2 - SysMsg.AdjVol.T_VPP2;
                        
                        if(DesVol > 0)
                        {
                            Adjust_Voltage_Pcw_Ncw(SysMsg.AdjVol.T_VPP2 - DesVol, SysMsg.AdjVol.T_VNN2);
                        }
                        
                        if(DesVol < 0)
                        {
                             Adjust_Voltage_Pcw_Ncw(SysMsg.AdjVol.T_VPP2 + DesVol, SysMsg.AdjVol.T_VNN2);
                        }
                    }
                    
                    if(abs(SysMsg.AdjVol.R_VNN2 - SysMsg.AdjVol.T_VNN2) > RNAGE_MIN_VOL)           
                    {
                        int16_t DesVol;
                        
                        DesVol = SysMsg.AdjVol.R_VNN2 - SysMsg.AdjVol.T_VNN2;
                        
                        if(DesVol > 0)
                        {
                            Adjust_Voltage_Pcw_Ncw(SysMsg.AdjVol.T_VPP2, SysMsg.AdjVol.T_VNN2 - DesVol);
                        }
                        if(DesVol < 0)
                        {
                            Adjust_Voltage_Pcw_Ncw(SysMsg.AdjVol.T_VPP2, SysMsg.AdjVol.T_VNN2 + DesVol);
                        }
                    }
                }
                    
                OSTimeDlyHMSM(0, 0, 0, 20, OS_OPT_TIME_PERIODIC, &err);                      //等待微调稳定
                
                Adc3_GetAdjVoltage();                                                       //获取微调结果
                
                DEBUG_PRINTF(DEBUG_STRING, "Inter MinAdjVol %d Times: %d %d %d %d \r\n", SysMsg.AdjVol.MinAdjVolCnt, SysMsg.AdjVol.R_VPP1, SysMsg.AdjVol.R_VNN1, SysMsg.AdjVol.R_VPP2, SysMsg.AdjVol.R_VNN2);            


                if(SysMsg.AdjVol.AdjVolSuccess == FALSE)                                    //微调之前单次调节失败                                                        
                {
                    if(SysMsg.AdjVol.R_VPP1 >= SysMsg.AdjVol.MIN_VPP1 &&                    //微调成功
                       SysMsg.AdjVol.R_VPP1 <= SysMsg.AdjVol.MAX_VPP1 &&      
                       SysMsg.AdjVol.R_VNN1 >= SysMsg.AdjVol.MIN_VNN1 && 
                       SysMsg.AdjVol.R_VNN1 <= SysMsg.AdjVol.MAX_VNN1 &&
                       SysMsg.AdjVol.R_VPP2 >= SysMsg.AdjVol.MIN_VPP2 && 
                       SysMsg.AdjVol.R_VPP2 <= SysMsg.AdjVol.MAX_VPP2 &&
                       SysMsg.AdjVol.R_VNN2 >= SysMsg.AdjVol.MIN_VNN2 && 
                       SysMsg.AdjVol.R_VNN2 <= SysMsg.AdjVol.MAX_VNN2 )
                    {
                        AdjVol_Result_Send();                                               //上传电压
                        DEBUG_PRINTF(DEBUG_STRING, "MinAdjVolTime Success \r\n");            
                    }
                }                        
            }
        }
        
        if(SysMsg.AdjVol.VolMinitor)
        {
            if(SysMsg.AdjVol.R_VPP1 <= SysMsg.AdjVol.MIN_VPP1 || SysMsg.AdjVol.R_VPP1 >= SysMsg.AdjVol.MAX_VPP1 ||
               SysMsg.AdjVol.R_VNN1 <= SysMsg.AdjVol.MIN_VNN1 || SysMsg.AdjVol.R_VNN1 >= SysMsg.AdjVol.MAX_VNN1 ||
               SysMsg.AdjVol.R_VPP2 <= SysMsg.AdjVol.MIN_VPP2 || SysMsg.AdjVol.R_VPP2 >= SysMsg.AdjVol.MAX_VPP2 ||
               SysMsg.AdjVol.R_VNN2 <= SysMsg.AdjVol.MIN_VNN2 || SysMsg.AdjVol.R_VNN2 >= SysMsg.AdjVol.MAX_VNN2 ) 
            {
                DEBUG_PRINTF(DEBUG_STRING, "Voltage output is out range : %d %d %d %d \r\n", SysMsg.AdjVol.R_VPP1, SysMsg.AdjVol.R_VNN1, SysMsg.AdjVol.R_VPP2, SysMsg.AdjVol.R_VNN2);            
                SysMsg.AdjVol.VolMinitor = FALSE;
//                Adjust_Voltage_Close();
            }
            
            if(SysMsg.AdjVol.R_VPP1 < (SysMsg.AdjVol.R_VPP2 - 200) || SysMsg.AdjVol.R_VNN1 < (SysMsg.AdjVol.R_VNN2 - 200))
            {
                DEBUG_PRINTF(DEBUG_STRING, "HV1 > HV2 : %d %d %d %d \r\n", SysMsg.AdjVol.R_VPP1, SysMsg.AdjVol.R_VNN1, SysMsg.AdjVol.R_VPP2, SysMsg.AdjVol.R_VNN2);            
                SysMsg.AdjVol.VolMinitor = FALSE;
//                Adjust_Voltage_Close();
            }
        }
        
        if(SysMsg.AdjVol.AdjVolOpen)
        {
            Adc3_GetAdjVoltage();                                                               //获取电压
            
            if(SysMsg.AdjVol.R_VPP1 >= SysMsg.AdjVol.MIN_VPP1 && SysMsg.AdjVol.R_VPP1 <= SysMsg.AdjVol.MAX_VPP1 &&
               SysMsg.AdjVol.R_VNN1 >= SysMsg.AdjVol.MIN_VNN1 && SysMsg.AdjVol.R_VNN1 <= SysMsg.AdjVol.MAX_VNN1 &&
               SysMsg.AdjVol.R_VPP2 >= SysMsg.AdjVol.MIN_VPP2 && SysMsg.AdjVol.R_VPP2 <= SysMsg.AdjVol.MAX_VPP2 &&
               SysMsg.AdjVol.R_VNN2 >= SysMsg.AdjVol.MIN_VNN2 && SysMsg.AdjVol.R_VNN2 <= SysMsg.AdjVol.MAX_VNN2 )  
            {
                DEBUG_PRINTF(DEBUG_STRING, "AdjVol Success : %d %d %d %d \r\n", SysMsg.AdjVol.R_VPP1, SysMsg.AdjVol.R_VNN1, SysMsg.AdjVol.R_VPP2, SysMsg.AdjVol.R_VNN2);            

                SysMsg.AdjVol.AdjVolOpen = FALSE;                                               //单次调压结束
                SysMsg.AdjVol.AdjVolSuccess = TRUE;                                             //单次调压成功
                SysMsg.AdjVol.MinAdjVolOpen = TRUE;                                             //微调开启
                AdjVol_Result_Send();                                                           //上传电压
            }
            
            if(SysMsg.AdjVol.Time >= SysMsg.AdjVol.TimeOut)
            {
                DEBUG_PRINTF(DEBUG_STRING, "AdjVol Timeout : %d %d %d %d \r\n", SysMsg.AdjVol.R_VPP1, SysMsg.AdjVol.R_VNN1, SysMsg.AdjVol.R_VPP2, SysMsg.AdjVol.R_VNN2);            

                SysMsg.AdjVol.AdjVolOpen = FALSE;                                               //调压结束  
                SysMsg.AdjVol.AdjVolSuccess = FALSE;                                            //单次调压失败
                
                
                if(abs(SysMsg.AdjVol.R_VPP1 - SysMsg.AdjVol.T_VPP1) >= RNAGE_MAX_VOL ||         //超时且单次调压超范围, 不进行微调, 上传超范围的电压, 并开启监控
                   abs(SysMsg.AdjVol.R_VNN1 - SysMsg.AdjVol.T_VNN1) >= RNAGE_MAX_VOL ||
                   abs(SysMsg.AdjVol.R_VPP1 - SysMsg.AdjVol.T_VPP1) >= RNAGE_MAX_VOL || 
                   abs(SysMsg.AdjVol.R_VNN1 - SysMsg.AdjVol.T_VNN1) >= RNAGE_MAX_VOL )
                {
                    DEBUG_PRINTF(DEBUG_STRING, "abs(R_VPP) - abs(T_VPP) >= RNAGE_MAX_VOL, not have mini AdjVol \r\n"); 
                    SysMsg.AdjVol.MinAdjVolOpen = FALSE;                                        //微调关闭
                    SysMsg.AdjVol.VolMinitor = TRUE;                                            //开启监控
                    AdjVol_Result_Send();                                                       //上传电压              
                }
                
                else                                                                            //超时但单次调节未超范围, 暂不上传电压, 开启微调, 暂不开启监控
                {
                    SysMsg.AdjVol.MinAdjVolOpen = TRUE;                                         //微调开启
                    SysMsg.AdjVol.VolMinitor = FALSE;                                           //监控暂不开启
                }
                
                          
            }
        }
        
        OSTimeDlyHMSM(0, 0, 0, 5, OS_OPT_TIME_PERIODIC, &err);
    }
}

void App_AdjVol_TaskCreate()
{
    OS_ERR err;
    
    OSTaskCreate((OS_TCB *		)&AdjVolTcb, 
                 (CPU_CHAR *		)"App_AdjVol_Task", 
                 (OS_TASK_PTR		)App_AdjVol_Task,
                 (void *		)0,
                 (OS_PRIO		)APP_ADJVOL_TASK_PRIO,
                 (CPU_STK *		)&App_AdjVol_Task_Stk[0],
                 (CPU_STK_SIZE		)APP_ADJVOL_STK_SIZE / 10,
                 (CPU_STK_SIZE		)APP_ADJVOL_STK_SIZE,
                 (OS_MSG_QTY		)0,
                 (OS_TICK		)0,
                 (void *		)0,
                 (OS_OPT		)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
                 (OS_ERR *		)&err);
}


#include "app_com.h"

extern System_MsgStruct SysMsg;

extern CmdFrameStr SenFrameCmd;

extern Com_Buffer DebugComRX;
extern Com_Buffer DebugComTX;

extern Com_Buffer CommuComRX;
extern Com_Buffer CommuComTX;

extern uint8_t Ec_Info[];


OS_TCB ComTaskTcb;

CPU_STK App_Com_Task_Stk[APP_COM_STK_SIZE];

void App_Com_Task()
{
    OS_ERR err;

    while(1)
    {		
        if(ReceiveFrameAnalysis(&CommuComRX.Data[0], CommuComRX.Len) == SUCCESS)   //格式化并解析串口数据
        {
            SysMsg.Cmd.Channel = ECCOM_CHANNEL;
            memset(CommuComRX.Data, 0, CommuComRX.Len);
            Cmd_Process();                                                          //命令处理                                    
        }

        if(SysMsg.Cmd.Channel == ECCOM_CHANNEL)
        {
            if(SysMsg.Cmd.EcInfo_Send == TRUE)
            {
                SysMsg.Cmd.EcInfo_Send = FALSE;
            
                SenFrameCmd.Cid = CMD_EC_COMMUNICATE;
                
                SenFrameCmd.Len = 49;
                SenFrameCmd.Data = Ec_Info;
                
                FrameCmdPackage(CommuComTX.Data);
                Send_CmdPackage(COMMU_COM_DMAY_STREAMX_TX);
            }
        }
    
        #if USE_DEBUG
        
        if(ReceiveFrameAnalysis(&DebugComRX.Data[0], DebugComRX.Len) == SUCCESS)   //格式化并解析串口数据
        {
            SysMsg.Cmd.Channel = DEBUGCOM_CHANNEL;
            memset(DebugComRX.Data, 0, DebugComRX.Len);
            Cmd_Process();                                 
        }

        if(Deal_Compare((char *)DebugComRX.Data, DebugComRX.Len) != 0)
        {
            SysMsg.Cmd.Channel = DEBUGCOM_CHANNEL;
            memset(DebugComRX.Data, 0, DebugComRX.Len);
        }
    
        if(SysMsg.Cmd.Channel == DEBUGCOM_CHANNEL)
        {   
            if(SysMsg.Cmd.HV_Send == TRUE)
            {
                SysMsg.Cmd.HV_Send = FALSE;
                
                #if DEBUG_COMMAND
                
                SenFrameCmd.Cid = CMD_ADJUST_HV;
                SenFrameCmd.Len = 10;
                
                SenFrameCmd.Data[0] = SysMsg.AdjVol.R_VPP1 >> 8;
                SenFrameCmd.Data[1] = SysMsg.AdjVol.R_VPP1;
                SenFrameCmd.Data[2] = SysMsg.AdjVol.R_VNN1 >> 8;
                SenFrameCmd.Data[3] = SysMsg.AdjVol.R_VNN1;
                SenFrameCmd.Data[4] = SysMsg.AdjVol.R_VPP2 >> 8;
                SenFrameCmd.Data[5] = SysMsg.AdjVol.R_VPP2;
                SenFrameCmd.Data[6] = SysMsg.AdjVol.R_VNN2 >> 8;
                SenFrameCmd.Data[7] = SysMsg.AdjVol.R_VNN2;
                SenFrameCmd.Data[8] = SysMsg.AdjVol.Time >> 8;
                SenFrameCmd.Data[9] = SysMsg.AdjVol.Time;
                
                FrameCmdPackage(DebugComTX.Data);
                Send_CmdPackage(DEBUG_COM_DMAY_STREAMX_TX); 
                
                #endif
                
                SysMsg.AdjVol.TimeFlag = FALSE;
                SysMsg.AdjVol.Time = 0;
                
                DEBUG_PRINTF(DEBUG_STRING, "HV Send Voltage : %d %d %d %d \r\n", SysMsg.AdjVol.R_VPP1, SysMsg.AdjVol.R_VNN1, SysMsg.AdjVol.R_VPP2, SysMsg.AdjVol.R_VNN2);            
            }
            
            if(SysMsg.Cmd.CW_Send == TRUE)
            {
                SysMsg.Cmd.CW_Send = FALSE;
                
                #if DEBUG_COMMAND
                
                SenFrameCmd.Cid = CMD_ADJUST_CW;
                SenFrameCmd.Len = 10;
                
                SenFrameCmd.Data[0] = SysMsg.AdjVol.R_VPP1 >> 8;
                SenFrameCmd.Data[1] = SysMsg.AdjVol.R_VPP1;
                SenFrameCmd.Data[2] = SysMsg.AdjVol.R_VNN1 >> 8;
                SenFrameCmd.Data[3] = SysMsg.AdjVol.R_VNN1;
                SenFrameCmd.Data[4] = SysMsg.AdjVol.R_VPP2 >> 8;
                SenFrameCmd.Data[5] = SysMsg.AdjVol.R_VPP2;
                SenFrameCmd.Data[6] = SysMsg.AdjVol.R_VNN2 >> 8;
                SenFrameCmd.Data[7] = SysMsg.AdjVol.R_VNN2;
                SenFrameCmd.Data[8] = SysMsg.AdjVol.Time >> 8;
                SenFrameCmd.Data[9] = SysMsg.AdjVol.Time;
                
                FrameCmdPackage(DebugComTX.Data);
                Send_CmdPackage(DEBUG_COM_DMAY_STREAMX_TX);
                
                SysMsg.AdjVol.TimeFlag = FALSE;
                SysMsg.AdjVol.Time = 0;
                
                #endif
                
                DEBUG_PRINTF(DEBUG_STRING, "CW Send Voltage : %d %d %d %d \r\n", SysMsg.AdjVol.R_VPP1, SysMsg.AdjVol.R_VNN1, SysMsg.AdjVol.R_VPP2, SysMsg.AdjVol.R_VNN2);
            }
            
            if(SysMsg.Cmd.Timeout == TRUE)
            {
                SysMsg.Cmd.Timeout = FALSE;
             
                
                #if DEBUG_COMMAND

                SenFrameCmd.Cid = TIMEOUT;
                SenFrameCmd.Len = 10;
                
                SenFrameCmd.Data[0] = SysMsg.AdjVol.R_VPP1 >> 8;
                SenFrameCmd.Data[1] = SysMsg.AdjVol.R_VPP1;
                SenFrameCmd.Data[2] = SysMsg.AdjVol.R_VNN1 >> 8;
                SenFrameCmd.Data[3] = SysMsg.AdjVol.R_VNN1;
                SenFrameCmd.Data[4] = SysMsg.AdjVol.R_VPP2 >> 8;
                SenFrameCmd.Data[5] = SysMsg.AdjVol.R_VPP2;
                SenFrameCmd.Data[6] = SysMsg.AdjVol.R_VNN2 >> 8;
                SenFrameCmd.Data[7] = SysMsg.AdjVol.R_VNN2;
                SenFrameCmd.Data[8] = SysMsg.AdjVol.Time >> 8;
                SenFrameCmd.Data[9] = SysMsg.AdjVol.Time;
                
                FrameCmdPackage(DebugComTX.Data);
                Send_CmdPackage(DEBUG_COM_DMAY_STREAMX_TX);
                
                SysMsg.AdjVol.TimeFlag = FALSE;
                SysMsg.AdjVol.Time = 0;
                
                #endif

                DEBUG_PRINTF(DEBUG_STRING, "Timeout Send Voltage : %d %d %d %d \r\n", SysMsg.AdjVol.R_VPP1, SysMsg.AdjVol.R_VNN1, SysMsg.AdjVol.R_VPP2, SysMsg.AdjVol.R_VNN2);
            }
            

        }
    
        #endif
        
        OSTimeDlyHMSM(0, 0, 0, 5, OS_OPT_TIME_PERIODIC, &err);
    }
}

void App_Com_TaskCreate()
{
    OS_ERR err;
    
    OSTaskCreate((OS_TCB *              )&ComTaskTcb, 
                 (CPU_CHAR *		)"App_Com_Task", 
                 (OS_TASK_PTR		)App_Com_Task,
                 (void *		)0,
                 (OS_PRIO		)APP_COM_TASK_PRIO,
                 (CPU_STK *		)&App_Com_Task_Stk[0],
                 (CPU_STK_SIZE		)APP_COM_STK_SIZE / 10,
                 (CPU_STK_SIZE		)APP_COM_STK_SIZE,
                 (OS_MSG_QTY		)0,
                 (OS_TICK		)0,
                 (void *		)0,
                 (OS_OPT		)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
                 (OS_ERR *		)&err);
}






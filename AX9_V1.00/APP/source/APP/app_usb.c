#include "app_usb.h"

extern System_MsgStruct SysMsg;
extern CmdFrameStr SenFrameCmd;
extern uint8_t Ec_Info[];



OS_CPU_EXT __IO uint32_t receive_count;
OS_CPU_EXT __ALIGN_BEGIN uint8_t USB_Rx_Buffer[CDC_DATA_MAX_PACKET_SIZE] __ALIGN_END ;
OS_CPU_EXT CDC_IF_Prop_TypeDef VCP_fops;

__ALIGN_BEGIN uint8_t USB_Tx_Buffer[CDC_DATA_MAX_PACKET_SIZE] __ALIGN_END ;

OS_TCB UsbTaskTcb;

CPU_STK App_Usb_Task_Stk[APP_LED_STK_SIZE];

void App_Usb_Task()
{
    OS_ERR err;
    
    while(1)
    {		
        if(VCP_CheckDataReceived() != 0)
        {
            if(ReceiveFrameAnalysis(USB_Rx_Buffer, receive_count) == SUCCESS)
            {
                SysMsg.Cmd.Channel = USB_CHANNEL;
                Cmd_Process();                       //ÃüÁî´¦Àí 
            }
            receive_count = 0;
        }
    
        if(SysMsg.Cmd.Channel == USB_CHANNEL)
        {
            if(SysMsg.Cmd.HV_Send == TRUE)
            {
                SysMsg.Cmd.HV_Send = FALSE;
                
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
                
                FrameCmdPackage(USB_Tx_Buffer);
                VCP_fops.pIf_DataTx(USB_Tx_Buffer, (USB_Tx_Buffer[3] + 6));
                
                SysMsg.AdjVol.TimeFlag = FALSE;
                SysMsg.AdjVol.Time = 0;
            }
        
            if(SysMsg.Cmd.CW_Send == TRUE)
            {
                SysMsg.Cmd.CW_Send = FALSE;
                
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
            
                FrameCmdPackage(USB_Tx_Buffer);
                VCP_fops.pIf_DataTx(USB_Tx_Buffer, (USB_Tx_Buffer[3] + 6));
                
                SysMsg.AdjVol.TimeFlag = FALSE;
                SysMsg.AdjVol.Time = 0;
            }

            if(SysMsg.Cmd.Timeout == TRUE)
            {
                SysMsg.Cmd.Timeout = FALSE;

                SenFrameCmd.Cid = TIMEOUT;
                SenFrameCmd.Len = 8;
                
                SenFrameCmd.Data[0] = SysMsg.AdjVol.R_VPP1 >> 8;
                SenFrameCmd.Data[1] = SysMsg.AdjVol.R_VPP1;
                SenFrameCmd.Data[2] = SysMsg.AdjVol.R_VNN1 >> 8;
                SenFrameCmd.Data[3] = SysMsg.AdjVol.R_VNN1;
                SenFrameCmd.Data[4] = SysMsg.AdjVol.R_VPP2 >> 8;
                SenFrameCmd.Data[5] = SysMsg.AdjVol.R_VPP2;
                SenFrameCmd.Data[6] = SysMsg.AdjVol.R_VNN2 >> 8;
                SenFrameCmd.Data[7] = SysMsg.AdjVol.R_VNN2;

                FrameCmdPackage(USB_Tx_Buffer);
                VCP_fops.pIf_DataTx(USB_Tx_Buffer, (USB_Tx_Buffer[3] + 6));
                
                SysMsg.AdjVol.TimeFlag = FALSE;
                SysMsg.AdjVol.Time = 0;
            }
        }

        OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_PERIODIC, &err);
    }
}

void App_Usb_TaskCreate()
{
    OS_ERR err;
    
    OSTaskCreate((OS_TCB *		)&UsbTaskTcb, 
                 (CPU_CHAR *		)"App_Usb_Task", 
                 (OS_TASK_PTR		)App_Usb_Task,
                 (void *		)0,
                 (OS_PRIO		)APP_USB_TASK_PRIO,
                 (CPU_STK *		)&App_Usb_Task_Stk[0],
                 (CPU_STK_SIZE		)APP_USB_STK_SIZE / 10,
                 (CPU_STK_SIZE		)APP_USB_STK_SIZE,
                 (OS_MSG_QTY		)0,
                 (OS_TICK		)0,
                 (void *		)0,
                 (OS_OPT		)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
                 (OS_ERR *		)&err);
}






#include "app_adjvol.h"

OS_TCB AdjVolTcb;

CPU_STK App_AdjVol_Task_Stk[APP_ADJVOL_STK_SIZE];

extern System_MsgStruct System_MsgStr;

void App_AdjVol_Task()
{
    OS_ERR err;

    while(1)
    {					
//        if(System_MsgStr.AdVolStr.CMD_HVFlag == TRUE)
//        {
//            System_MsgStr.AdVolStr.CMD_HVFlag = FALSE;
//            Adjust_Voltage_HV();                            //执行高压调压处理
//            System_MsgStr.AdVolStr.HV_Minitor = TRUE;       //处理完成打开高压监控
//        }
//        
//        if(System_MsgStr.AdVolStr.CMD_CWFlag == TRUE)
//        {
//            System_MsgStr.AdVolStr.CMD_CWFlag = FALSE;
//            Adjust_Voltage_CW();                            
//        }
    
            OSTimeDlyHMSM(0, 0, 0, 50, OS_OPT_TIME_PERIODIC, &err);
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


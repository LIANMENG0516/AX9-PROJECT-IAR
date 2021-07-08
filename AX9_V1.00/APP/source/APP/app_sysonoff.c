#include "app_sysonoff.h"

extern System_MsgStruct SysMsg;

extern CmdFrameStr RcvFrameCmd; 

OS_TCB SysOnOffTaskTcb;

CPU_STK App_SysOnOff_Task_Stk[APP_SYSONOFF_STK_SIZE];

void PowerOn_Sequence()
{
    OS_ERR err;
    PWR_CTL(1);
    PBUS_ON(1);
    OSTimeDly(2, OS_OPT_TIME_DLY, &err);
    CTL_P12V_EN(1);
    OSTimeDly(20, OS_OPT_TIME_DLY, &err);
    CTL_N12V_5V5_EN(1);
    OSTimeDly(20, OS_OPT_TIME_DLY, &err);
    CTL_P5V5_1_EN(1);
    OSTimeDly(20, OS_OPT_TIME_DLY, &err);
    CTL_P5V5_2_EN(1);
    OSTimeDly(20, OS_OPT_TIME_DLY, &err);
    CTL_P3V75_EN(1);
    CTL_P2V25_EN(1);
    OSTimeDly(20, OS_OPT_TIME_DLY, &err);
    CTL_D0V95_EN(1);
    OSTimeDly(15, OS_OPT_TIME_DLY, &err);
    CTL_VDD_P5V_EN(1);
    CTL_D1V45_EN(1);
    OSTimeDly(15, OS_OPT_TIME_DLY, &err);
    EN_FRONT(1);
    EN_FPGA_01(1); 
    AFE_EN1(1);
    OSTimeDly(15, OS_OPT_TIME_DLY, &err);
    EN_FPGA_02(1);
    AFE_EN2(1);
}

void PowerDown_Sequence()
{
    OS_ERR err;
    
    TX7516_EN(0);
    PWR_OK_COM(0);
    OSTimeDly(200, OS_OPT_TIME_DLY, &err);
    AFE_EN2(0);
    EN_FPGA_02(0);
    AFE_EN1(0);
    EN_FPGA_01(0);
    EN_FRONT(0);
    CTL_D1V45_EN(0);
    CTL_VDD_P5V_EN(0);
    CTL_D0V95_EN(0);
    CTL_P2V25_EN(0);
    CTL_P3V75_EN(0);
    CTL_P5V5_2_EN(0);
    CTL_P5V5_1_EN(0);
    CTL_N12V_5V5_EN(0);
    CTL_P12V_EN(0);
    PBUS_ON(0);
    PWR_CTL(0);
}

void System_OnCtrl()
{
    OS_ERR err;
    
    bool Pwr_OnSequence = FALSE; 
    
    if(SysMsg.SystemState == SYSTEM_OFF)
    {
        if(System_PwrKey_Minitor() == TRUE && SysMsg.KeyState == TRUE)                 //按键按下
        {
            if(SysMsg.PowerOnReq == TRUE)                                              //开机请求
            {
                SysMsg.PowerOnReq = FALSE;
                PWR_BTN_COM(0);
            }
        }
        
        if(SysMsg.KeyState == FALSE)
        {
            PWR_BTN_COM(1);
        }
        else
        {
            PWR_BTN_COM(0);
        }
        
        if(System_S3_State_Minitor() == TRUE)
        {
            if(SysMsg.S3_State == TRUE)
            {
                PowerOn_Sequence();
                Pwr_OnSequence = TRUE;
            }
            else
            {
                PowerDown_Sequence();
            }
        }
        
        if(Pwr_OnSequence == TRUE)
        {
            Pwr_OnSequence = FALSE;
            if(!FPGA_CFG_DOWN_CHK())
            {
                OSTimeDly(3000, OS_OPT_TIME_DLY, &err);
            }
            PWR_OK_COM(1);
            TX7516_EN(1);

            SysMsg.SystemState = SYSTEM_ON;
            
            USB_CTRL_EN(1);                                                             //使能USB插入
            
            
            //调试固化高压发射为10V 5V           
                        
            CTL_VPP1_VNN1_EN(0);                       //关闭高压输出
            CTL_VPP2_VNN2_EN(0);

            Adjust_Cw_Reset();

            Adjust_Voltage_Vpp1(4000);                 //调节VPP1至目标值
            Adjust_Voltage_Vpp2(2000);                  //调节VPP2至目标值
            Adjust_Voltage_Vnn1_Vnn2(4000, 2000);       //调节VNN1, VNN2至目标值

            CTL_VPP1_VNN1_EN(1);                       //打开高压输出
            CTL_VPP2_VNN2_EN(1);
        }
    }
}

void System_OffCtrl()
{  
    if(SysMsg.SystemState == SYSTEM_ON)
    {
        if(System_PwrKey_Minitor() == TRUE && SysMsg.KeyState == TRUE)                  //按键按下
        {
            if(SysMsg.ShutDownReq == TRUE)                                              //关机请求
            {
                SysMsg.ShutDownReq = FALSE;
                PWR_BTN_COM(0);
            }
        }
        
        if(SysMsg.KeyState == FALSE)
        {
            PWR_BTN_COM(1);
        }
        else
        {
            PWR_BTN_COM(0);
        }
            
        if(System_S3_State_Minitor() == TRUE)
        {
            if(SysMsg.S3_State == TRUE)
            {
                PowerOn_Sequence();
            }
            else
            {
                PowerDown_Sequence();                                                   
                USB_CTRL_EN(0);                                                         //卸载USB, 防止倒灌                                                                           //
                SysMsg.SystemState = SYSTEM_OFF;
            }
        } 
    }
}

void App_SysOnOff_Task()
{
    OS_ERR err;

    while(1)
    {      
        System_OnCtrl();
        System_OffCtrl();

        OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_PERIODIC, &err);
    }
}

void App_SysOnOff_TaskCreate()
{
    OS_ERR err;
    
    OSTaskCreate((OS_TCB *		)&SysOnOffTaskTcb, 
                 (CPU_CHAR *		)"SysOnOffTaskTcb", 
                 (OS_TASK_PTR		)App_SysOnOff_Task,
                 (void *		)0,
                 (OS_PRIO		)APP_SYSONOFF_TASK_PRIO,
                 (CPU_STK *		)&App_SysOnOff_Task_Stk[0],
                 (CPU_STK_SIZE		)APP_SYSONOFF_STK_SIZE / 10,
                 (CPU_STK_SIZE		)APP_SYSONOFF_STK_SIZE,
                 (OS_MSG_QTY		)0,
                 (OS_TICK		)0,
                 (void *		)0,
                 (OS_OPT		)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
                 (OS_ERR *		)&err);
}

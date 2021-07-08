#include "app_timer.h"

extern System_MsgStruct SysMsg;

void cb_Sys_StateChk_Tmr(void *p_tmr, void *p_arg)
{
    static uint16_t startCnt = 0;
    
    Update_EcInfo();

    Adc3_GetFixVoltage();
    
    Adc3_GetAdjVoltage();

    if(SysMsg.AdjVol.HV_Minitor)
    {
        if(SysMsg.AdjVol.R_VPP1 >= SysMsg.AdjVol.MIN_VPP1 && SysMsg.AdjVol.R_VPP1 <= SysMsg.AdjVol.MAX_VPP1 &&
           SysMsg.AdjVol.R_VNN1 >= SysMsg.AdjVol.MIN_VNN1 && SysMsg.AdjVol.R_VNN1 <= SysMsg.AdjVol.MAX_VNN1 &&
           SysMsg.AdjVol.R_VPP2 >= SysMsg.AdjVol.MIN_VPP2 && SysMsg.AdjVol.R_VPP2 <= SysMsg.AdjVol.MAX_VPP2 &&
           SysMsg.AdjVol.R_VNN2 >= SysMsg.AdjVol.MIN_VNN2 && SysMsg.AdjVol.R_VNN2 <= SysMsg.AdjVol.MAX_VNN2 )  
        {
            SysMsg.AdjVol.HV_Minitor = FALSE;
            SysMsg.Cmd.HV_Send = TRUE;
            startCnt = 0;
        }
        else
        {
            if(++startCnt >= ADJTIMEOUT/10)
            {
                SysMsg.AdjVol.HV_Minitor = FALSE;
                SysMsg.Cmd.Timeout = TRUE;
            }
        } 
    }
    
    if(SysMsg.AdjVol.CW_Minitor)
    {
        if(SysMsg.AdjVol.R_VPP1 >= SysMsg.AdjVol.MIN_VPP1 && SysMsg.AdjVol.R_VPP1 <= SysMsg.AdjVol.MAX_VPP1 &&
           SysMsg.AdjVol.R_VNN1 >= SysMsg.AdjVol.MIN_VNN1 && SysMsg.AdjVol.R_VNN1 <= SysMsg.AdjVol.MAX_VNN1 &&
           SysMsg.AdjVol.R_VPP2 >= SysMsg.AdjVol.MIN_VPP2 && SysMsg.AdjVol.R_VPP2 <= SysMsg.AdjVol.MAX_VPP2 &&
           SysMsg.AdjVol.R_VNN2 >= SysMsg.AdjVol.MIN_VNN2 && SysMsg.AdjVol.R_VNN2 <= SysMsg.AdjVol.MAX_VNN2 )  
        {
            SysMsg.AdjVol.CW_Minitor = FALSE;
            SysMsg.Cmd.CW_Send = TRUE;
            startCnt = 0;
        }
        else
        {
            if(++startCnt >= ADJTIMEOUT/10)
            {
                SysMsg.AdjVol.CW_Minitor = FALSE;
                SysMsg.Cmd.Timeout = TRUE;
            }
        } 
    }    
}













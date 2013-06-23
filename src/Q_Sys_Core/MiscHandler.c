//本页用于存杂项
#include "System.h"

//用于系统的各个地方互斥读写spi flash
bool Q_SpiFlashSync(FLASH_CMD cmd,u32 addr,u32 len,u8 *buf)
{
#if(QXW_PRODUCT_ID==114)
	OS_DeclareCritical();

	OS_EnterCritical();
#endif
	
	switch(cmd)
	{
		case FlashRead:
			M25P16_Fast_Read_Data(addr,len,buf);
			break;
		case FlashWrite:
			M25P16_Page_Program(addr,len,buf);
			break;
		case FlashSectorEarse:
			M25P16_Sector_Erase(addr);
			break;
		case FlashBlukEarse:
			M25P16_Bulk_Erase();
			break;
	}

#if(QXW_PRODUCT_ID==114)
	OS_ExitCritical();
#endif

	return TRUE;
}

//TimId用来选择定时器，当Sys_Timer事件触发时，IntParam参数传递的就是此id
//uS_Base 表示单位(时基)，为1时，单位是us；为100时，单位是100us；最小值1，最大值900
//Val最大值65535
//最终定时值= Val x uS_Base x 1us
//新定时设定会覆盖旧设定
//AutoReload用来设定是一次定时还是循环定时
//val和uS_Base其中任意一个为0，则停止当前定时。
void Q_TimSet(Q_TIM_ID TimId,u16 Val,u16 uS_Base, bool AutoReload)
{
	switch(TimId)
	{
		case Q_TIM1:
			Tim2_Set(Val,uS_Base,AutoReload);
			break;
		case Q_TIM2:
			Tim4_Set(Val,uS_Base,AutoReload);
			break;
		case Q_TIM3:
			Tim5_Set(Val,uS_Base,AutoReload);
			break;
		default:
			Debug("No Such Timer in Q-sys!\n\r");
	}
}


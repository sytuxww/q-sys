//��ҳ���ڴ�����
#include "System.h"

//����ϵͳ�ĸ����ط������дspi flash
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

//TimId����ѡ��ʱ������Sys_Timer�¼�����ʱ��IntParam�������ݵľ��Ǵ�id
//uS_Base ��ʾ��λ(ʱ��)��Ϊ1ʱ����λ��us��Ϊ100ʱ����λ��100us����Сֵ1�����ֵ900
//Val���ֵ65535
//���ն�ʱֵ= Val x uS_Base x 1us
//�¶�ʱ�趨�Ḳ�Ǿ��趨
//AutoReload�����趨��һ�ζ�ʱ����ѭ����ʱ
//val��uS_Base��������һ��Ϊ0����ֹͣ��ǰ��ʱ��
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


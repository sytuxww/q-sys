/*
 * FileName:        	main.c       
 * Author:          	YuanYin  
 * Hardware Version:	QXW-Summer-V1.x  
 * Date: 				2010-4-29
 * Description:			The main file of Q-OS 
 * version:				Q-OS V1.0
 */
 
/*****************************************************
 *  ����Ƽ�	 Q-Share We	����-��Դ-����		
 *  						   	
 *  ���������ǵĲ�Ʒ���κν������������ϵ����! 	
 *						   	
 *  �Ա�רӪ��Q-ShareWe.TaoBao.Com			
 *  ������̳��Www.Q-ShareWe.Com				
 *  ��ϵͳ����Ⱥ��37182463
 ****************************************************/

#include "System.h"   
/*******************************************************************************
* ���ڲ��Ե���������
*******************************************************************************/
void __asm UndefinedInstruction(void) 
{ 
	DCI 0xf123; 
	DCI 0x4567; 
	BX LR; 
} 

void __asm BadAlignedLDM(void) 
{ 
	MOVS r0, #1 
	LDM r0,{r1-r2} 
	BX LR; 
}

/*******************************************************************************
* ����: void RCC_Config(void)
* ����: ����ϵͳʱ�� 
* ����: ��
* ����: ��
*******************************************************************************/
void RCC_Config(void)
{
	ErrorStatus HSEStartUpStatus;//�����ⲿ���پ�������״̬ö�ٱ���
	
	RCC_DeInit();//��λRCC�ⲿ�豸�Ĵ�����Ĭ��ֵ
	RCC_HSEConfig(RCC_HSE_ON); //���ⲿ���پ���
	HSEStartUpStatus = RCC_WaitForHSEStartUp();//�ȴ��ⲿ����ʱ��׼����
	if(HSEStartUpStatus == SUCCESS)//�ⲿ����ʱ���Ѿ�׼���
  		{
			RCC_HCLKConfig(RCC_SYSCLK_Div1);//����AHB(HCLK)ʱ�ӵ���==SYSCLK
			RCC_PCLK2Config(RCC_HCLK_Div1); //����APB2(PCLK2)��==AHBʱ��
			RCC_PCLK1Config(RCC_HCLK_Div2);//����APB1(PCLK1)��==AHB1/2ʱ��			
		    FLASH_SetLatency(FLASH_Latency_2);//FLASH��ʱ2������
		    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);//ʹ��FLASHԤȡ������
			RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);	//����PLLʱ�� == 8M*9

		    RCC_PLLCmd(ENABLE);//ʹ��PLLʱ��
			while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET); //�ȴ�PLLʱ�Ӿ���

		    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);//����ϵͳʱ�� = PLLʱ��
		
		    while(RCC_GetSYSCLKSource() != 0x08); //���PLLʱ���Ƿ���Ϊϵͳʱ��
		}
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);//��GPIOD��AFIOʱ��
}
void *InputHandler_Task_Handle=NULL;
int main(void)
{   
	RCC_Config();

//#if !QXW_RELEASE_VER//for debug
	COM1_Init(); //����
//#endif
//	COM3_Init();	
	
	OS_WrapInit();
	//UndefinedInstruction();
	//BadAlignedLDM();

	OS_TaskCreate(InputHandler_Task,"Input",OS_MINIMAL_STACK_SIZE*8,NULL,INPUT_TASK_PRIORITY,&InputHandler_Task_Handle);
	OS_StartRun();
	
	return (0);
}

#if (OS_APP_HOOKS_EN > 0)
void App_TaskCreateHook(OS_TCB* ptcb)
{
}

void App_TaskDelHook(OS_TCB* ptcb)
{
   (void) ptcb;
}

void App_TaskIdleHook(void)
{
}

void App_TaskStatHook(void)
{
}

#if OS_TASK_SW_HOOK_EN > 0
void App_TaskSwHook(void)
{
}
#endif

void App_TCBInitHook(OS_TCB* ptcb)
{
   (void) ptcb;
}

#endif

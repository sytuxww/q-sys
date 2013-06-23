/*
 * FileName:        	main.c       
 * Author:          	YuanYin  
 * Hardware Version:	QXW-Summer-V1.x  
 * Date: 				2010-4-29
 * Description:			The main file of Q-OS 
 * version:				Q-OS V1.0
 */
 
/*****************************************************
 *  酷享科技	 Q-Share We	快乐-开源-分享		
 *  						   	
 *  如果你对我们的产品有任何建议或不满，请联系我们! 	
 *						   	
 *  淘宝专营：Q-ShareWe.TaoBao.Com			
 *  技术论坛：Www.Q-ShareWe.Com				
 *  酷系统交流群：37182463
 ****************************************************/

#include "System.h"   
/*******************************************************************************
* 用于测试的两个函数
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
* 函数: void RCC_Config(void)
* 功能: 配置系统时钟 
* 参数: 无
* 返回: 无
*******************************************************************************/
void RCC_Config(void)
{
	ErrorStatus HSEStartUpStatus;//定义外部高速晶体启动状态枚举变量
	
	RCC_DeInit();//复位RCC外部设备寄存器到默认值
	RCC_HSEConfig(RCC_HSE_ON); //打开外部高速晶振
	HSEStartUpStatus = RCC_WaitForHSEStartUp();//等待外部高速时钟准备好
	if(HSEStartUpStatus == SUCCESS)//外部高速时钟已经准别好
  		{
			RCC_HCLKConfig(RCC_SYSCLK_Div1);//配置AHB(HCLK)时钟等于==SYSCLK
			RCC_PCLK2Config(RCC_HCLK_Div1); //配置APB2(PCLK2)钟==AHB时钟
			RCC_PCLK1Config(RCC_HCLK_Div2);//配置APB1(PCLK1)钟==AHB1/2时钟			
		    FLASH_SetLatency(FLASH_Latency_2);//FLASH延时2个周期
		    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);//使能FLASH预取缓冲区
			RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);	//配置PLL时钟 == 8M*9

		    RCC_PLLCmd(ENABLE);//使能PLL时钟
			while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET); //等待PLL时钟就绪

		    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);//配置系统时钟 = PLL时钟
		
		    while(RCC_GetSYSCLKSource() != 0x08); //检查PLL时钟是否作为系统时钟
		}
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);//打开GPIOD和AFIO时钟
}
void *InputHandler_Task_Handle=NULL;
int main(void)
{   
	RCC_Config();

//#if !QXW_RELEASE_VER//for debug
	COM1_Init(); //串口
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

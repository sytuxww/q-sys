/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : stm32f10x_it.c
* Author             : MCD Application Team
* Version            : V2.0.1
* Date               : 06/13/2008
* Description        : Main Interrupt Service Routines.
*                      This file provides template for all exceptions handler
*                      and peripherals interrupt service routine.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "System.h"
#include "Drivers.h"
#include "SdCard.h"
#include "Usb_istr.h"

//extern OS_SemaphoreHandle gSD_Mutex;  //karlno add
extern OS_SemaphoreHandle gVsDreq_Sem;//vs准备好的信号量
extern OS_SemaphoreHandle gTouchHandler_Sem;
extern OS_SemaphoreHandle gRfRecvHandler_Sem;//射频接收数据的信号量

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define It_Debug(x,y...)		//Debug

/* Private variables ---------------------------------------------------------*/
u8 LCD_Light_Counter=0;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : NMIException
* Description    : This function handles NMI exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NMIException(void)
{}

/*******************************************************************************
* Function Name  : HardFaultException
* Description    : This function handles Hard Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
signed char *pcGetCurTaskName(void);
void HardFaultException(unsigned int * hardfault_args,unsigned int *sp) 
{ 
	unsigned int stacked_r0; 
	unsigned int stacked_r1; 
	unsigned int stacked_r2; 
	unsigned int stacked_r3; 
	unsigned int stacked_r12; 
	unsigned int stacked_lr; 
	unsigned int stacked_pc; 
	unsigned int stacked_psr; 

	stacked_r0 = ((unsigned long) hardfault_args[0]); 
	stacked_r1 = ((unsigned long) hardfault_args[1]); 
	stacked_r2 = ((unsigned long) hardfault_args[2]); 
	stacked_r3 = ((unsigned long) hardfault_args[3]); 
	stacked_r12 = ((unsigned long) hardfault_args[4]); 
	stacked_lr = ((unsigned long) hardfault_args[5]); 
	stacked_pc = ((unsigned long) hardfault_args[6]); 
	stacked_psr = ((unsigned long) hardfault_args[7]); 
	Debug ("\n\n\n\n\r################Hard fault handler]################\n\r"); 
	Debug ("R0 = 0x%08x\n\r", stacked_r0); 
	Debug ("R1 = 0x%08x\n\r", stacked_r1); 
	Debug ("R2 = 0x%08x\n\r", stacked_r2); 
	Debug ("R3 = 0x%08x\n\r", stacked_r3); 
	Debug ("R12 = 0x%08x\n\r", stacked_r12); 
	Debug ("SP = 0x%08x\n\r",(unsigned int)sp);
	Debug ("LR = 0x%08x\n\r", stacked_lr); 
	Debug ("PC = 0x%08x\n\r", stacked_pc); 
	Debug ("PSR = 0x%08x\n\r", stacked_psr); 
	Debug ("BFAR = 0x%08x\n\r", (*((volatile unsigned long *)(0xE000ED38)))); 
	Debug ("CFSR = 0x%08x\n\r", (*((volatile unsigned long *)(0xE000ED28)))); 
	Debug ("HFSR = 0x%08x\n\r", (*((volatile unsigned long *)(0xE000ED2C)))); 
	Debug ("DFSR = 0x%08x\n\r", (*((volatile unsigned long *)(0xE000ED30)))); 
	Debug ("AFSR = 0x%08x\n\r", (*((volatile unsigned long *)(0xE000ED3C)))); 
	#if OS_USE_UCOS
	Debug ("Now Task : %s\n\r",OSTCBPrioTbl[OSTCBCur->OSTCBPrio]->OSTCBTaskName);
	#elif OS_USE_FREERTOS
	Debug ("Now Task : %s\n\r",pcGetCurTaskName());
	#endif

	OS_TaskStkCheck(TRUE);
	OS_DebugHeap();	
	Q_ErrorStopScreen("HardFaultException");

	//return; 
}

/*******************************************************************************
* Function Name  : MemManageException
* Description    : This function handles Memory Manage exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MemManageException(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {}
}

/*******************************************************************************
* Function Name  : BusFaultException
* Description    : This function handles Bus Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void BusFaultException(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {}
}

/*******************************************************************************
* Function Name  : UsageFaultException
* Description    : This function handles Usage Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UsageFaultException(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {}
}

/*******************************************************************************
* Function Name  : DebugMonitor
* Description    : This function handles Debug Monitor exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DebugMonitor(void)
{}

/*******************************************************************************
* Function Name  : SVCHandler
* Description    : This function handles SVCall exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SVCHandler(void)
{}

/*******************************************************************************
* Function Name  : PendSVC
* Description    : This function handles PendSVC exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PendSVC(void)
{}

/*******************************************************************************
* Function Name  : SysTickHandler
* Description    : This function handles SysTick Handler.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SysTickHandler(void)
{}

/*******************************************************************************
* Function Name  : WWDG_IRQHandler
* Description    : This function handles WWDG interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void WWDG_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : PVD_IRQHandler
* Description    : This function handles PVD interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PVD_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : TAMPER_IRQHandler
* Description    : This function handles Tamper interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TAMPER_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : RTC_IRQHandler
* Description    : This function handles RTC global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
unsigned int RTC_Counter=0;
void RTC_IRQHandler(void)
{
	INPUT_EVENT EventParam;
	u8 ItemsTotal=0;
	
    OS_IntEnter();

	It_Debug("--R--%d %d %d\n\r",RTC_GetITStatus(RTC_IT_SEC),RTC_GetITStatus(RTC_IT_ALR),RTC_GetCounter());

	if(RTC_GetITStatus(RTC_IT_ALR) != RESET) //闹钟报警
	{
		It_Debug("RTC Alarm!\n\r");
		
	    /* Clear the RTC Alarm interrupt */
	    RTC_ClearITPendingBit(RTC_IT_ALR);

	    //背光处理
	    if(Gui_GetBgLightVal()==0)//如果lcd灭，先点亮
		{
			Gui_SetBgLight(Q_DB_GetValue(Setting_BgLightScale,NULL));
			LCD_Light_Counter=0;
		}

		if(Q_InspectPeripEvt(PRID_Current,Perip_RtcAlarm))//检查是否需要触发
		{
			EventParam.Info.Items[ItemsTotal++]=Input_RtcAlarm;//加到事件列表
		}
	}

	if(RTC_GetITStatus(RTC_IT_SEC) != RESET)
	{
	    //背光处理
	    if(Gui_GetBgLightVal()!=0)//如果lcd亮着
		    if(Q_DB_GetValue(Setting_BgLightTime,NULL))//如果获取到非0值
				if(LCD_Light_Counter++==Q_DB_GetValue(Setting_BgLightTime,NULL))//如果等于设定值
				{					
					Gui_SetBgLight(0);
					LCD_Light_Counter=0;
					if(Q_InspectPeripEvt(PRID_Current,Perip_LcdOff))//检查是否需要触发
					{
						EventParam.Info.Items[ItemsTotal++]=Input_LcdOff;//加到事件列表
					}
				}
		
	    //实时事件处理
	    RTC_WaitForLastTask();
		if(RTC_GetCounter()==RTC_Counter)
		{
			RTC_Counter+=60;
			if(Q_InspectPeripEvt(PRID_Current,Perip_RtcMin))//检查是否需要触发
			{
				EventParam.Info.Items[ItemsTotal++]=Input_RtcMin;
			}
		}
		else if(Q_InspectPeripEvt(PRID_Current,Perip_RtcSec))//检查是否需要触发
		{
			EventParam.Info.Items[ItemsTotal++]=Input_RtcSec;
		}
		RTC_WaitForLastTask();
		
	    RTC_ClearITPendingBit(RTC_IT_SEC);/* Clear the RTC Second interrupt */
	}

	if(ItemsTotal)
	{
		EventParam.uType=Rtc_Type;	
		EventParam.EventType=Input_RtcSec;//无用值
		EventParam.Num=(int)RTC_GetCounter();
		EventParam.Info.Items[ItemsTotal]=0;
		OS_MsgBoxSend(gInputHandler_Queue,&EventParam,OS_NO_DELAY,FALSE);
	}
	
    OS_IntExit();                                                /* Tell uC/OS-II that we are leaving the ISR            */
}

/*******************************************************************************
* Function Name  : FLASH_IRQHandler
* Description    : This function handles Flash interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void FLASH_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : RCC_IRQHandler
* Description    : This function handles RCC interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RCC_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : EXTI0_IRQHandler
* Description    : This function handles External interrupt Line 0 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI0_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : EXTI1_IRQHandler
* Description    : This function handles External interrupt Line 1 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI1_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : EXTI2_IRQHandler
* Description    : This function handles External interrupt Line 2 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI2_IRQHandler(void)
{
	OS_IntEnter();
	Debug("--E2--\n\r");
	
	if(EXTI_GetITStatus(EXTI_Line2) != RESET)
	{
		
		EXTI_ClearITPendingBit(EXTI_Line2);
	}
	OS_IntExit(); 
}

/*******************************************************************************
* Function Name  : EXTI3_IRQHandler
* Description    : This function handles External interrupt Line 3 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI3_IRQHandler(void)
{
#if QXW_LCM_ID == 210 || QXW_LCM_ID == 211 || QXW_LCM_ID == 220 || QXW_LCM_ID == 221
	OS_IntEnter();
	It_Debug("--E3--\n\r");
	
	if(EXTI_GetITStatus(EXTI_Line3) != RESET)
	{
		//if(!gRfRecvHandler_Sem->OSEventCnt)
		OS_SemaphoreGive(gRfRecvHandler_Sem);//告诉qweb线程有数据

		EXTI_ClearITPendingBit(EXTI_Line3);
	}
	OS_IntExit(); 
#endif
}

/*******************************************************************************
* Function Name  : EXTI4_IRQHandler
* Description    : This function handles External interrupt Line 4 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI4_IRQHandler(void)
{
	OS_IntEnter();
	It_Debug("--E4--\n\r");
	
	if(EXTI_GetITStatus(EXTI_Line4) != RESET)
	{		
		if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4))
		{
			//if(!gVsDreq_Sem->OSEventCnt)
			OS_SemaphoreGive(gVsDreq_Sem);
		}
		
		EXTI_ClearITPendingBit(EXTI_Line4);
	}
	OS_IntExit();
}

/*******************************************************************************
* Function Name  : DMA1_Channel1_IRQHandler
* Description    : This function handles DMA1 Channel 1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel1_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : DMA1_Channel2_IRQHandler
* Description    : This function handles DMA1 Channel 2 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel2_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : DMA1_Channel3_IRQHandler
* Description    : This function handles DMA1 Channel 3 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel3_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : DMA1_Channel4_IRQHandler
* Description    : This function handles DMA1 Channel 4 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel4_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : DMA1_Channel5_IRQHandler
* Description    : This function handles DMA1 Channel 5 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel5_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : DMA1_Channel6_IRQHandler
* Description    : This function handles DMA1 Channel 6 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel6_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : DMA1_Channel7_IRQHandler
* Description    : This function handles DMA1 Channel 7 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel7_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : ADC1_2_IRQHandler
* Description    : This function handles ADC1 and ADC2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ADC1_2_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : USB_HP_CAN_TX_IRQHandler
* Description    : This function handles USB High Priority or CAN TX interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USB_HP_CAN1_TX_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : USB_LP_CAN_RX0_IRQHandler
* Description    : This function handles USB Low Priority or CAN RX0 interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USB_LP_CAN1_RX0_IRQHandler(void)
{
	OS_IntEnter();
	USB_Istr();
	OS_IntExit(); 
}

/*******************************************************************************
* Function Name  : CAN1_RX1_IRQHandler
* Description    : This function handles CAN RX1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN1_RX1_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : CAN1_SCE_IRQHandler
* Description    : This function handles CAN SCE interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN1_SCE_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : EXTI9_5_IRQHandler
* Description    : This function handles External lines 9 to 5 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI9_5_IRQHandler(void)
{
#if QXW_LCM_ID == 212
	OS_IntEnter();
	It_Debug("--E9-5--\n\r");
	
	if(EXTI_GetITStatus(EXTI_Line6) != RESET)
	{
		//if(!gRfRecvHandler_Sem->OSEventCnt)
		OS_SemaphoreGive(gRfRecvHandler_Sem);//告诉qweb线程有数据

		EXTI_ClearITPendingBit(EXTI_Line6);
	}
	OS_IntExit(); 
#endif
}

/*******************************************************************************
* Function Name  : TIM1_BRK_IRQHandler
* Description    : This function handles TIM1 Break interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_BRK_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : TIM1_UP_IRQHandler
* Description    : This function handles TIM1 overflow and update interrupt
*                  request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_UP_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : TIM1_TRG_COM_IRQHandler
* Description    : This function handles TIM1 Trigger and commutation interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_TRG_COM_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : TIM1_CC_IRQHandler
* Description    : This function handles TIM1 capture compare interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_CC_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : TIM2_IRQHandler
* Description    : This function handles TIM2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM2_IRQHandler(void)
{
	INPUT_EVENT EventParam;
	
	OS_IntEnter();
	It_Debug("--T2--\n\r");
	
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		
		if(Q_InspectPeripEvt(PRID_Current,Perip_Timer))//检查是否需要触发
		{
			EventParam.uType=SingleNum_Type;
			EventParam.EventType=Input_Timer;
			EventParam.Num=Q_TIM1;
			OS_MsgBoxSend(gInputHandler_Queue,&EventParam,OS_NO_DELAY,TRUE);
		}
	}
	OS_IntExit(); 
}

/*******************************************************************************
* Function Name  : TIM3_IRQHandler
* Description    : This function handles TIM3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM3_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : TIM4_IRQHandler
* Description    : This function handles TIM4 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM4_IRQHandler(void)
{
	INPUT_EVENT EventParam;
	
	OS_IntEnter();
	It_Debug("--T4--\n\r");
	
	if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
		
		if(Q_InspectPeripEvt(PRID_Current,Perip_Timer))//检查是否需要触发
		{
			EventParam.uType=SingleNum_Type;
			EventParam.EventType=Input_Timer;
			EventParam.Num=Q_TIM2;
			OS_MsgBoxSend(gInputHandler_Queue,&EventParam,OS_NO_DELAY,TRUE);
		}
	}
	OS_IntExit(); 
}

/*******************************************************************************
* Function Name  : I2C1_EV_IRQHandler
* Description    : This function handles I2C1 Event interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C1_EV_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : I2C1_ER_IRQHandler
* Description    : This function handles I2C1 Error interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C1_ER_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : I2C2_EV_IRQHandler
* Description    : This function handles I2C2 Event interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C2_EV_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : I2C2_ER_IRQHandler
* Description    : This function handles I2C2 Error interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C2_ER_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : SPI1_IRQHandler
* Description    : This function handles SPI1 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI1_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : SPI2_IRQHandler
* Description    : This function handles SPI2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI2_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : USART1_IRQHandler
* Description    : This function handles USART1 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void QS_MonitorFragment(void);
void OS_TaskStkCheck(bool Display);
void USART1_IRQHandler(void)
{
	INPUT_EVENT IE;
	static u8 gUartBuf[UART_BUF_LEN];//串口缓冲
	static u8 Idx=0;
	static u8 EscFlag=0;//输入完成标志
	static u8 EscBuf[4];//控制支付存储地
	
	OS_IntEnter();
	
	It_Debug("--U1--\n\r");
	//QS_MonitorFragment();//add by cy for debug
	//OS_TaskStkCheck(TRUE);
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{ 
		IE.Num=USART_ReceiveData(USART1);

		if(EscFlag)//溢出了就省略2个字节
		{
			EscBuf[EscFlag-1]=IE.Num;//存入字符
			if(EscFlag==2)//控制字符
			{
				EscBuf[2]=0;
				
				//串口输入事件
				IE.uType=Sync_Type;
				IE.EventType=Input_UartInput;
				IE.Num=1;//表示串口1
				IE.Info.SyncInfo.IntParam=((IE.Num<<16)+0);
				IE.Info.SyncInfo.pParam=EscBuf;
				OS_MsgBoxSend(gInputHandler_Queue,&IE,OS_NO_DELAY,FALSE);

				EscFlag=0;
			}
			else	EscFlag++;//正常字符
		}
		else
		{
			if(IE.Num==13)//回车
			{
				gUartBuf[Idx]=0;
				
				//串口输入事件
				IE.uType=Sync_Type;
				IE.EventType=Input_UartInput;
				IE.Num=1;//表示串口1
				IE.Info.SyncInfo.IntParam=((IE.Num<<16)+Idx);
				IE.Info.SyncInfo.pParam=gUartBuf;
				OS_MsgBoxSend(gInputHandler_Queue,&IE,OS_NO_DELAY,FALSE);

				USART_SendData(USART1,'\n');//回显
				while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);//等待发送结束
				USART_SendData(USART1,'\r');//回显
				while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);//等待发送结束

				Idx=0;
			}
			else if(IE.Num==0x08)//回删
			{
				if(Idx>0)
				{
					gUartBuf[--Idx]=0;
					USART_SendData(USART1,'\b');//回显
					while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);//等待发送结束
					USART_SendData(USART1,' ');//回显
					while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);//等待发送结束
					USART_SendData(USART1,'\b');//回显
					while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);//等待发送结束
				}
			}
			else if(IE.Num>=0x20)
			{
				gUartBuf[Idx++]=IE.Num;
				USART_SendData(USART1,IE.Num);//回显
				while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);//等待发送结束
			}
			else if(IE.Num==0x1b)//溢出了
			{
				EscFlag=1;
			}

			if(Idx>=UART_BUF_LEN) Idx--;//超出范围，就停止加入新的了。
		}

		USART_ClearITPendingBit(USART1,USART_IT_RXNE);//清除中断标志
	}

	OS_IntExit();
}

/*******************************************************************************
* Function Name  : USART2_IRQHandler
* Description    : This function handles USART2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART2_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : USART3_IRQHandler
* Description    : This function handles USART3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART3_IRQHandler(void)
{
	INPUT_EVENT IE;
	static u8 gUartBuf[UART_BUF_LEN];//串口缓冲
	static u8 Idx=0;
	static u8 EscFlag=0;
	static u8 EscBuf[4];
	
	OS_IntEnter();
	
	It_Debug("--U3--\n\r");
	
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{ 
		IE.Num=USART_ReceiveData(USART3);

		if(EscFlag)//溢出了就省略2个字节
		{
			EscBuf[EscFlag-1]=IE.Num;
			if(EscFlag==2)
			{
				EscBuf[2]=0;
				
				//串口输入事件
				IE.uType=Sync_Type;
				IE.EventType=Input_UartInput;
				IE.Num=3;//串口3
				IE.Info.SyncInfo.IntParam=((IE.Num<<16)+0);
				IE.Info.SyncInfo.pParam=EscBuf;
				OS_MsgBoxSend(gInputHandler_Queue,&IE,OS_NO_DELAY,FALSE);

				EscFlag=0;
			}
			else	EscFlag++;
		}
		else
		{
			if(IE.Num==13)//回车
			{
				gUartBuf[Idx]=0;
				
				//串口输入事件
				IE.uType=Sync_Type;
				IE.EventType=Input_UartInput;
				IE.Num=3;//串口3
				IE.Info.SyncInfo.IntParam=((IE.Num<<16)+Idx);
				IE.Info.SyncInfo.pParam=gUartBuf;
				OS_MsgBoxSend(gInputHandler_Queue,&IE,OS_NO_DELAY,FALSE);

				USART_SendData(USART3,'\n');//回显
				while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);//等待发送结束
				USART_SendData(USART3,'\r');//回显
				while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);//等待发送结束
				
				Idx=0;
			}
			else if(IE.Num==0x08)//回删
			{
				if(Idx>0)
				{
					gUartBuf[--Idx]=0;
					USART_SendData(USART3,'\b');//回显
					while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);//等待发送结束
					USART_SendData(USART3,' ');//回显
					while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);//等待发送结束
					USART_SendData(USART3,'\b');//回显
					while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);//等待发送结束
				}
			}
			else if(IE.Num>=0x20)
			{
				gUartBuf[Idx++]=IE.Num;
				USART_SendData(USART3,IE.Num);//回显
				while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);//等待发送结束
			}
			else if(IE.Num==0x1b)//溢出了
			{
				EscFlag=1;
			}

			if(Idx>=UART_BUF_LEN) Idx--;//超出范围，就停止加入新的了。
		}

		USART_ClearITPendingBit(USART3,USART_IT_RXNE);//清除中断标志
	}

	OS_IntExit();
}

/*******************************************************************************
* Function Name  : EXTI15_10_IRQHandler
* Description    : This function handles External lines 15 to 10 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI15_10_IRQHandler(void)
{
	INPUT_EVENT EventParam;

	OS_IntEnter();
	
	It_Debug("--T--\n\r");
	
	if(EXTI_GetITStatus(EXTI_Line12) != RESET)
	{	
		if(HasTouch())
		{
			if(Gui_GetBgLightVal()==0)//灭的时候点亮
			{
				Gui_SetBgLight(Q_DB_GetValue(Setting_BgLightScale,NULL));

				if(Q_InspectPeripEvt(PRID_Current,Perip_LcdOn))//检查是否需要触发
				{
					//发送触摸屏点亮事件
					EventParam.uType=SingleNum_Type;
					EventParam.EventType=Input_LcdOn;
					OS_MsgBoxSend(gInputHandler_Queue,&EventParam,OS_NO_DELAY,FALSE);
				}
			}
			else//亮的时候置零计时器
			{
				LCD_Light_Counter=0;
				//Debug("T\n\r");
				//if(!gTouchHandler_Sem->OSEventCnt)
				OS_SemaphoreGive(gTouchHandler_Sem);//告诉touch线程有触摸点击
				//OS_TaskResume(TOUCH_TASK_PRIORITY);
			}
		}
		
		EXTI_ClearITPendingBit(EXTI_Line12);
	}
	
	OS_IntExit();
}

/*******************************************************************************
* Function Name  : RTCAlarm_IRQHandler
* Description    : This function handles RTC Alarm interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RTCAlarm_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : USBWakeUp_IRQHandler
* Description    : This function handles USB WakeUp interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USBWakeUp_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : TIM8_BRK_IRQHandler
* Description    : This function handles TIM8 Break interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM8_BRK_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : TIM8_UP_IRQHandler
* Description    : This function handles TIM8 overflow and update interrupt
*                  request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM8_UP_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : TIM8_TRG_COM_IRQHandler
* Description    : This function handles TIM8 Trigger and commutation interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM8_TRG_COM_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : TIM8_CC_IRQHandler
* Description    : This function handles TIM8 capture compare interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM8_CC_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : ADC3_IRQHandler
* Description    : This function handles ADC3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ADC3_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : FSMC_IRQHandler
* Description    : This function handles FSMC global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void FSMC_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : SDIO_IRQHandler
* Description    : This function handles SDIO global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SDIO_IRQHandler(void)
{
	OS_DeclareCritical();
	OS_EnterCritical();
	//OSIntNesting++;
	SD_ProcessIRQSrc();
	OS_ExitCritical();
	
	//OS_IntExit();
}

/*******************************************************************************
* Function Name  : TIM5_IRQHandler
* Description    : This function handles TIM5 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM5_IRQHandler(void)
{
	INPUT_EVENT EventParam;
	
	OS_IntEnter();
	It_Debug("--T5--\n\r");
	
	if(TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
		
		if(Q_InspectPeripEvt(PRID_Current,Perip_Timer))//检查是否需要触发
		{
			EventParam.uType=SingleNum_Type;
			EventParam.EventType=Input_Timer;
			EventParam.Num=Q_TIM3;
			OS_MsgBoxSend(gInputHandler_Queue,&EventParam,OS_NO_DELAY,TRUE);
		}
	}
	OS_IntExit(); 
}

/*******************************************************************************
* Function Name  : SPI3_IRQHandler
* Description    : This function handles SPI3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI3_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : UART4_IRQHandler
* Description    : This function handles UART4 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART4_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : UART5_IRQHandler
* Description    : This function handles UART5 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART5_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : TIM6_IRQHandler
* Description    : This function handles TIM6 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM6_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : TIM7_IRQHandler
* Description    : This function handles TIM7 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM7_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : DMA2_Channel1_IRQHandler
* Description    : This function handles DMA2 Channel 1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA2_Channel1_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : DMA2_Channel2_IRQHandler
* Description    : This function handles DMA2 Channel 2 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA2_Channel2_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : DMA2_Channel3_IRQHandler
* Description    : This function handles DMA2 Channel 3 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA2_Channel3_IRQHandler(void)
{}

/*******************************************************************************
* Function Name  : DMA2_Channel4_5_IRQHandler
* Description    : This function handles DMA2 Channel 4 and DMA2 Channel 5
*                  interrupts requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA2_Channel4_5_IRQHandler(void)
{}

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/

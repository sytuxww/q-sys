#include "System.h"

typedef struct{
	uint32_t RccId;
	GPIO_TypeDef* GpioGroup;
	uint16_t GpioPin;
	GPIOMode_TypeDef GpioMode;
}EXTI_KEY_DEFINE;

const EXTI_KEY_DEFINE gExtiKeyDefine[EXTI_KEY_MAX_NUM]={
	{RCC_APB2Periph_GPIOE, GPIOE, GPIO_Pin_2, GPIO_Mode_IPD},
#if(QXW_PRODUCT_ID==116)
	{RCC_APB2Periph_GPIOE, GPIOE, GPIO_Pin_3, GPIO_Mode_IPD},
	{RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_8, GPIO_Mode_IPD},
#endif
	//更多的外部按键定义放这里

};

extern void ExtiKeyHandler(u8 KeyId,u8 KeyStaus);
extern u8 LCD_Light_Counter;
//用于查询外部按键状态
void KeysHandler_Task(void *Task_Parameters )
{
	GPIO_InitTypeDef GPIO_InitStructure;
	u32 KeyMap=0;
	u8 i;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	for(i=0;i<EXTI_KEY_MAX_NUM;i++)
	{
		RCC_APB2PeriphClockCmd(gExtiKeyDefine[i].RccId,ENABLE);
		GPIO_InitStructure.GPIO_Pin = gExtiKeyDefine[i].GpioPin;
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = gExtiKeyDefine[i].GpioMode;
		GPIO_Init(gExtiKeyDefine[i].GpioGroup, &GPIO_InitStructure);
	}

	while(1)
	{
		for(i=0;i<EXTI_KEY_MAX_NUM;i++)
		{
			if(GPIO_ReadInputDataBit(gExtiKeyDefine[i].GpioGroup,gExtiKeyDefine[i].GpioPin)!=ReadBit(KeyMap,i))//有变化
			{
				KeyMap^=(1<<(i));
				if(gExtiKeyDefine[i].GpioMode == GPIO_Mode_IPD)//下拉输入
					ExtiKeyHandler(i,GPIO_ReadInputDataBit(gExtiKeyDefine[i].GpioGroup,gExtiKeyDefine[i].GpioPin));
				else if(gExtiKeyDefine[i].GpioMode == GPIO_Mode_IPU)//上拉输入
					ExtiKeyHandler(i,!GPIO_ReadInputDataBit(gExtiKeyDefine[i].GpioGroup,gExtiKeyDefine[i].GpioPin));
				LCD_Light_Counter=0;
			}
		}
	
		OS_TaskDelayMs(100);
	}
}


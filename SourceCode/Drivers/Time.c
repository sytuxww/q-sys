#include "Drivers.h"

void Tim3_Init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	/* Time Base configuration */
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = 999;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; // 输出模式
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 1000; // 占空比参数
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);

    TIM_ARRPreloadConfig(TIM3, ENABLE); // 这个记得要开
    TIM_Cmd(TIM3, ENABLE);
}

//输入范围0-100
void Tim3_PWM(u8 Value)
{
 	TIM_OCInitTypeDef TIM_OCStructure;	 	//计数器输出比较初始化结构体

	TIM_OCStructure.TIM_OCMode = TIM_OCMode_PWM1;	   //PWM1模式
	TIM_OCStructure.TIM_OutputState = TIM_OutputState_Enable; 
	TIM_OCStructure.TIM_Pulse = Value*10;		//脉冲值 赋给CCR寄存器
	TIM_OCStructure.TIM_OCPolarity = TIM_OCPolarity_High;//比较匹配输出高电平
	TIM_OC1Init(TIM3, &TIM_OCStructure);  //使以上参数有效
}

void Tim2_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	//配置定时器TIM2中断
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TIME2_IRQn_Priority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
}

void Tim4_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	//配置定时器TIM4中断
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TIME4_IRQn_Priority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
}

void Tim5_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	//配置定时器TIM5中断
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TIME5_IRQn_Priority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
}

//uS_Base 表示单位，为1时，单位是us；为100时，单位是100us；最小值1，最大值900
//最终定时值= Val x uS_Base x 1us
//新定时设定会覆盖旧设定
//AutoReload用来设定是一次定时还是循环定时
//val和uS_Base其中任意一个为0，则停止当前定时。
void Tim2_Set(u16 Val,u16 uS_Base,bool AutoReload)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    RCC_ClocksTypeDef RCC_Clocks;

    RCC_GetClocksFreq(&RCC_Clocks);//获取系统频率

    TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
    TIM_Cmd(TIM2, DISABLE);

    if((Val==0)||(uS_Base==0)) return;

    if(uS_Base>900) uS_Base=900;
    
    //定时频率为： 72M/(预分频+1)/预装载
    TIM_TimeBaseStructure.TIM_Period        = (Val-1);
    TIM_TimeBaseStructure.TIM_Prescaler     = ((RCC_Clocks.SYSCLK_Frequency/1000000)*uS_Base - 1);

    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	if(AutoReload) TIM_SelectOnePulseMode(TIM2,TIM_OPMode_Repetitive);
	else TIM_SelectOnePulseMode(TIM2,TIM_OPMode_Single);
	
	TIM_ClearFlag(TIM2, TIM_IT_Update);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);/* 使能TIM2的 向上溢出 中断 */    

	TIM_Cmd(TIM2, ENABLE);/* 使能TIM2 */	 
}

void Tim4_Set(u16 Val, u16 uS_Base, bool AutoReload)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    RCC_ClocksTypeDef RCC_Clocks;

    RCC_GetClocksFreq(&RCC_Clocks);//获取系统频率

    TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);
    TIM_Cmd(TIM4, DISABLE);

    if((Val == 0) || (uS_Base == 0)) return;

    if(uS_Base > 900) uS_Base = 900;
    
    //定时频率为： 72M/(预分频+1)/预装载
    TIM_TimeBaseStructure.TIM_Period        = (Val - 1);
    TIM_TimeBaseStructure.TIM_Prescaler     = ((RCC_Clocks.SYSCLK_Frequency / 1000000) * uS_Base - 1);

    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	if(AutoReload) TIM_SelectOnePulseMode(TIM4, TIM_OPMode_Repetitive);
	else TIM_SelectOnePulseMode(TIM4, TIM_OPMode_Single);
	
	TIM_ClearFlag(TIM4, TIM_IT_Update);
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);/* 使能TIM4的 向上溢出 中断 */    

	TIM_Cmd(TIM4, ENABLE);/* 使能TIM4 */	 
}

void Tim5_Set(u16 Val, u16 uS_Base, bool AutoReload)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    RCC_ClocksTypeDef RCC_Clocks;

    RCC_GetClocksFreq(&RCC_Clocks);//获取系统频率

    TIM_ITConfig(TIM5, TIM_IT_Update, DISABLE);
    TIM_Cmd(TIM5, DISABLE);

    if((Val == 0) || (uS_Base == 0)) return;

    if(uS_Base > 900) uS_Base = 900;
    
    //定时频率为： 72M/(预分频+1)/预装载
    TIM_TimeBaseStructure.TIM_Period        = (Val - 1);
    TIM_TimeBaseStructure.TIM_Prescaler     = ((RCC_Clocks.SYSCLK_Frequency / 1000000) * uS_Base - 1);

    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);

	if(AutoReload) TIM_SelectOnePulseMode(TIM5, TIM_OPMode_Repetitive);
	else TIM_SelectOnePulseMode(TIM5, TIM_OPMode_Single);
	
	TIM_ClearFlag(TIM5, TIM_IT_Update);
    TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);/* 使能TIM5的 向上溢出 中断 */    

	TIM_Cmd(TIM5, ENABLE);/* 使能TIM5 */	 
}



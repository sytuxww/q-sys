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

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; // ���ģʽ
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 1000; // ռ�ձȲ���
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);

    TIM_ARRPreloadConfig(TIM3, ENABLE); // ����ǵ�Ҫ��
    TIM_Cmd(TIM3, ENABLE);
}

//���뷶Χ0-100
void Tim3_PWM(u8 Value)
{
 	TIM_OCInitTypeDef TIM_OCStructure;	 	//����������Ƚϳ�ʼ���ṹ��

	TIM_OCStructure.TIM_OCMode = TIM_OCMode_PWM1;	   //PWM1ģʽ
	TIM_OCStructure.TIM_OutputState = TIM_OutputState_Enable; 
	TIM_OCStructure.TIM_Pulse = Value*10;		//����ֵ ����CCR�Ĵ���
	TIM_OCStructure.TIM_OCPolarity = TIM_OCPolarity_High;//�Ƚ�ƥ������ߵ�ƽ
	TIM_OC1Init(TIM3, &TIM_OCStructure);  //ʹ���ϲ�����Ч
}

void Tim2_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	//���ö�ʱ��TIM2�ж�
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
	
	//���ö�ʱ��TIM4�ж�
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
	
	//���ö�ʱ��TIM5�ж�
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TIME5_IRQn_Priority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
}

//uS_Base ��ʾ��λ��Ϊ1ʱ����λ��us��Ϊ100ʱ����λ��100us����Сֵ1�����ֵ900
//���ն�ʱֵ= Val x uS_Base x 1us
//�¶�ʱ�趨�Ḳ�Ǿ��趨
//AutoReload�����趨��һ�ζ�ʱ����ѭ����ʱ
//val��uS_Base��������һ��Ϊ0����ֹͣ��ǰ��ʱ��
void Tim2_Set(u16 Val,u16 uS_Base,bool AutoReload)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    RCC_ClocksTypeDef RCC_Clocks;

    RCC_GetClocksFreq(&RCC_Clocks);//��ȡϵͳƵ��

    TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
    TIM_Cmd(TIM2, DISABLE);

    if((Val==0)||(uS_Base==0)) return;

    if(uS_Base>900) uS_Base=900;
    
    //��ʱƵ��Ϊ�� 72M/(Ԥ��Ƶ+1)/Ԥװ��
    TIM_TimeBaseStructure.TIM_Period        = (Val-1);
    TIM_TimeBaseStructure.TIM_Prescaler     = ((RCC_Clocks.SYSCLK_Frequency/1000000)*uS_Base - 1);

    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	if(AutoReload) TIM_SelectOnePulseMode(TIM2,TIM_OPMode_Repetitive);
	else TIM_SelectOnePulseMode(TIM2,TIM_OPMode_Single);
	
	TIM_ClearFlag(TIM2, TIM_IT_Update);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);/* ʹ��TIM2�� ������� �ж� */    

	TIM_Cmd(TIM2, ENABLE);/* ʹ��TIM2 */	 
}

void Tim4_Set(u16 Val, u16 uS_Base, bool AutoReload)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    RCC_ClocksTypeDef RCC_Clocks;

    RCC_GetClocksFreq(&RCC_Clocks);//��ȡϵͳƵ��

    TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);
    TIM_Cmd(TIM4, DISABLE);

    if((Val == 0) || (uS_Base == 0)) return;

    if(uS_Base > 900) uS_Base = 900;
    
    //��ʱƵ��Ϊ�� 72M/(Ԥ��Ƶ+1)/Ԥװ��
    TIM_TimeBaseStructure.TIM_Period        = (Val - 1);
    TIM_TimeBaseStructure.TIM_Prescaler     = ((RCC_Clocks.SYSCLK_Frequency / 1000000) * uS_Base - 1);

    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	if(AutoReload) TIM_SelectOnePulseMode(TIM4, TIM_OPMode_Repetitive);
	else TIM_SelectOnePulseMode(TIM4, TIM_OPMode_Single);
	
	TIM_ClearFlag(TIM4, TIM_IT_Update);
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);/* ʹ��TIM4�� ������� �ж� */    

	TIM_Cmd(TIM4, ENABLE);/* ʹ��TIM4 */	 
}

void Tim5_Set(u16 Val, u16 uS_Base, bool AutoReload)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    RCC_ClocksTypeDef RCC_Clocks;

    RCC_GetClocksFreq(&RCC_Clocks);//��ȡϵͳƵ��

    TIM_ITConfig(TIM5, TIM_IT_Update, DISABLE);
    TIM_Cmd(TIM5, DISABLE);

    if((Val == 0) || (uS_Base == 0)) return;

    if(uS_Base > 900) uS_Base = 900;
    
    //��ʱƵ��Ϊ�� 72M/(Ԥ��Ƶ+1)/Ԥװ��
    TIM_TimeBaseStructure.TIM_Period        = (Val - 1);
    TIM_TimeBaseStructure.TIM_Prescaler     = ((RCC_Clocks.SYSCLK_Frequency / 1000000) * uS_Base - 1);

    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);

	if(AutoReload) TIM_SelectOnePulseMode(TIM5, TIM_OPMode_Repetitive);
	else TIM_SelectOnePulseMode(TIM5, TIM_OPMode_Single);
	
	TIM_ClearFlag(TIM5, TIM_IT_Update);
    TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);/* ʹ��TIM5�� ������� �ж� */    

	TIM_Cmd(TIM5, ENABLE);/* ʹ��TIM5 */	 
}



#include "Drivers.h" 

/********************************************************************************
* 名    称：void Adc_init(void)
* 功    能：ADC初始化
* 入口参数：无
* 出口参数：无
* 说    明：
*******************************************************************************/ 
void Adc_Init(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;  
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA| RCC_APB2Periph_ADC1, ENABLE);
 
  	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  	ADC_InitStructure.ADC_NbrOfChannel = 1;
  	ADC_Init(ADC1, &ADC_InitStructure);
  	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);
  	ADC_Cmd(ADC1, ENABLE);
  	ADC_ResetCalibration(ADC1);
  	
	while(ADC_GetResetCalibrationStatus(ADC1));
  	
	ADC_StartCalibration(ADC1);
  	
	while(ADC_GetCalibrationStatus(ADC1));
  	
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}


u16 ReadBatteryVoltage(void)
{
	return ADC_GetConversionValue(ADC1);
}


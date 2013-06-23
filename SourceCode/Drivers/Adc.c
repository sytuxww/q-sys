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
		
	//PA0设置为模拟通道0                     
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA| RCC_APB2Periph_ADC1, ENABLE);
 
    //以下是ADC1的寄存器配置
  	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;//AD模式选为独立模式
  	ADC_InitStructure.ADC_ScanConvMode = ENABLE;//自动扫描模式使能
  	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;//连续转换模式使能
  	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;//没有中断触发转换
  	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//数据向右对齐
  	ADC_InitStructure.ADC_NbrOfChannel = 1;//初始化ADC通道号数1
  	ADC_Init(ADC1, &ADC_InitStructure);//构建ADC1设备
  	//PA0对应ADC1通道是0
  	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);
	//使能ADC1
  	ADC_Cmd(ADC1, ENABLE);
  	//复位ADC1的寄存器  
  	ADC_ResetCalibration(ADC1);
    //等待复位结束 	
  	while(ADC_GetResetCalibrationStatus(ADC1));
  	//开始ADC1校准
  	ADC_StartCalibration(ADC1);
  	//等待ADC1校准结束 	
  	while(ADC_GetCalibrationStatus(ADC1));
    //使能ADC1转换
  	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}


u16 ReadBatteryVoltage(void)
{
	return ADC_GetConversionValue(ADC1);
}


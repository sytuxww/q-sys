#include "Drivers.h" 

/********************************************************************************
* ��    �ƣ�void Adc_init(void)
* ��    �ܣ�ADC��ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵    ����
*******************************************************************************/ 
void Adc_Init(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;  
		
	//PA0����Ϊģ��ͨ��0                     
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA| RCC_APB2Periph_ADC1, ENABLE);
 
    //������ADC1�ļĴ�������
  	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;//ADģʽѡΪ����ģʽ
  	ADC_InitStructure.ADC_ScanConvMode = ENABLE;//�Զ�ɨ��ģʽʹ��
  	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;//����ת��ģʽʹ��
  	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;//û���жϴ���ת��
  	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//�������Ҷ���
  	ADC_InitStructure.ADC_NbrOfChannel = 1;//��ʼ��ADCͨ������1
  	ADC_Init(ADC1, &ADC_InitStructure);//����ADC1�豸
  	//PA0��ӦADC1ͨ����0
  	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);
	//ʹ��ADC1
  	ADC_Cmd(ADC1, ENABLE);
  	//��λADC1�ļĴ���  
  	ADC_ResetCalibration(ADC1);
    //�ȴ���λ���� 	
  	while(ADC_GetResetCalibrationStatus(ADC1));
  	//��ʼADC1У׼
  	ADC_StartCalibration(ADC1);
  	//�ȴ�ADC1У׼���� 	
  	while(ADC_GetCalibrationStatus(ADC1));
    //ʹ��ADC1ת��
  	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}


u16 ReadBatteryVoltage(void)
{
	return ADC_GetConversionValue(ADC1);
}


#ifndef __VS1003B_H__
#define __VS1003B_H__  

#define TCS   (1<<1)  // PA1 
#define RST   (1<<4)  // PC4     			    
#define XDCS   (1<<2)  // PA2
#define VS_CS_SET(x)  GPIOA->ODR=(GPIOA->ODR&~TCS)|(x ? TCS:0)  
#define VS_XRST_SET(x)  GPIOC->ODR=(GPIOC->ODR&~RST)|(x ? RST:0)  
#define VS_XDCS_SET(x)  GPIOA->ODR=(GPIOA->ODR&~XDCS)|(x ? XDCS:0)  

#define VS_DREQ  (GPIOA->IDR & GPIO_Pin_4)

#define VS_WRITE_COMMAND 	0x02
#define VS_READ_COMMAND 	0x03		 		 

//�Ĵ�������
#define REG_MODE        	0x00   //ģʽ����
#define REG_STATUS      	0x01   //״̬�Ĵ���
#define REG_BASS        	0x02   //������������
#define REG_CLOCKF      	0x03   //ʱ������
#define REG_DECODE_TIME 	0x04   // 1��֮�ڽ������
#define REG_AUDATA      	0x05   // 15-1bit�����ʣ�0bitͨ����
#define REG_WRAM        	0x06   //���س����ָ�ram
#define REG_WRAMADDR    	0x07   //��REG_WRAM֮ǰʹ��
#define REG_HDAT0       	0x08   //�ļ���Ϣ
#define REG_HDAT1       	0x09   //�ļ���Ϣ
#define REG_AIADDR      	0x0a   //ָ��Ӧ�ô��뿪ʼ��ַ
#define REG_VOL         	0x0b   //��������
#define REG_AICTRL0     	0x0c   //
#define REG_AICTRL1     	0x0d   //
#define REG_AICTRL2     	0x0e   //
#define REG_AICTRL3     	0x0f   //

//Mode�Ĵ���λ����
#define SM_DIFF         	0x01   //0������λ��1��������ת
#define SM_JUMP         	0x02   //������
#define SM_RESET        	0x04   //��λʱ��λ����λ���Զ�����
#define SM_OUTOFWAV     	0x08   //��λʱֹͣwav����
#define SM_PDOWN        	0x10   // ��λʱ�������ʡ��ģʽ
#define SM_TESTS        	0x20   //��λʱ����SDI����ģʽ
#define SM_STREAM       	0x40   //ʹ��vs103����ģʽ
#define SM_PLUSV        	0x80   //MP3+V����ʹ��
#define SM_DACT         	0x100   // ����sck�����ػ��½�����Ч
#define SM_SDIORD       	0x200   //����sdi�����ֽ�˳��
#define SM_SDISHARE     	0x400   //��λʱ��xDCS��ʡȥ��ֻ��xCS
#define SM_SDINEW       	0x800   //��λ������ģʽ
#define SM_ADPCM        	0x1000   //��λʱADPCM¼��ʹ��
#define SM_ADPCM_HP     	0x2000 //��λʱʹ��ADPCM��ͨ�˲���
#define SM_LINE_IN	0x4000		// 1ѡ��line in��0ѡ����˷�

//Gpio�Ĵ�������
//0xC017 rw 0 DDR[3:0] �趨io�ڷ���1�������0������
//0xC018 r 0 IDATA[3:0] ��io������ֵ
//0xC019 rw 0 ODATA[3:0] �趨ioֵ�������� 
#define VS_GPIO_DDR  0xC017 
#define VS_GPIO_IData  0xC018 
#define VS_GPIO_OData  0xC019 

#define VS_Write_Byte(Data) SPI_WriteByte(SPI1,Data)
#define VS_Read_Byte() SPI_ReadByte(SPI1)

//�õ�mp3&wma�Ĳ�����
u16 VsGetHeadInfo(void);  

//�������ʱ��                          
void VsResetDecodeTime(void);

//�õ�mp3�Ĳ���ʱ��n sec
u16 VsGetDecodeTime(void); 

//����Ƶ�׷����Ĵ��뵽VS1003
void VsLoadPatch(void);

//�õ�Ƶ������
void VsGetSpec(u16 *p);

//����io����
void  VsGpioDDR(u8 ucDDRValue) ; 

//����io��ƽ
void VsGpioWriteData(u8 ucWriteValue) ; 

//��ȡio��ƽ
u8 VsGpioReadData(void) ; 

//��������
void VsSetVolume(u8  left,u8 right) ;

//��λ
void VsSoftReset(void);

//led ����
void VsLedSet(bool LedIO2,bool LedIO3);

//����vs
void VsStart(void);

void VsStop(void);

void VsInit(void);





#endif


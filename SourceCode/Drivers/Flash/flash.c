#include "stm32f10x.h"

//������������ݾ���MCU��FLASH��ַ��������
#define  FLASH_START     0x08000000
#define  FLASH_SIZE	     0x80000	//512K
#define  FLASH_PAGE_SIZE 0x800		//2K

#define  PAGEADDRESS (FLASH_START+FLASH_SIZE-FLASH_PAGE_SIZE) //FLASH���һҳ���׵�ַ,��FLASH��д���������ڸ�ҳ�ڽ���

//����  ��ָ����ַ��FLASH��д����
//����  Buf: д�����׵�ַ  Num���ֽ��� Address: FLASH��ַ
//���� 1: �ɹ� 0: ʧ��
//ע�⣺
//1.FLASH�����һҳ�����г��������
//2.Address����λ��FLASH�����һҳ��
//3.Buf,Num,Address���ܱ�4����
unsigned int FLASH_WriteBuf(void *Buf,unsigned int Num,unsigned int Address)
{
	unsigned int i;
	FLASH_Unlock();	 //�ر�д����
	if( (unsigned int)Buf%4!=0 || (unsigned int)Num%4!=0 || (unsigned int)Address%4!=0) //����������
		return 0;
	if( Address<PAGEADDRESS || Address>(PAGEADDRESS+FLASH_PAGE_SIZE-Num) ) //������Χ���
		return 0;
	if(FLASH_ErasePage(PAGEADDRESS)!=FLASH_COMPLETE)//�߳�
		return 0;
	for(i=0;i<Num/4;i++)
	{
		if( FLASH_ProgramWord(Address+4*i, ((unsigned int *)Buf)[i] ) != FLASH_COMPLETE) 
			return 0;
	}
	FLASH_Lock();   //����д����
	return 1; 
}

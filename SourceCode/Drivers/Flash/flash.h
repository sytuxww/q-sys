#ifndef __FLASH_H__
#define __FLASH_H__

//����  ��ָ����ַ��FLASH��д����
//����  Buf: д�����׵�ַ  Num���ֽ��� Address: ��ַ
//1.FLASH�����һҳ�����г��������
//2.Buf,Num,Address���ܱ�4����
//3.Address����λ��FLASH�����һҳ��
//���� 1: �ɹ� 0: ʧ��
unsigned int FLASH_WriteBuf(void *Buf,unsigned int Num,unsigned int Address)

#endif

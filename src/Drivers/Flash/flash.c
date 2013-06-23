#include "stm32f10x.h"

//下面三个宏根据具体MCU的FLASH地址进行配置
#define  FLASH_START     0x08000000
#define  FLASH_SIZE	     0x80000	//512K
#define  FLASH_PAGE_SIZE 0x800		//2K

#define  PAGEADDRESS (FLASH_START+FLASH_SIZE-FLASH_PAGE_SIZE) //FLASH最后一页的首地址,对FLASH的写操作必须在该页内进行

//功能  向指定地址的FLASH里写数据
//参数  Buf: 写缓存首地址  Num：字节数 Address: FLASH地址
//返回 1: 成功 0: 失败
//注意：
//1.FLASH的最后一页不能有程序或数据
//2.Address必须位于FLASH的最后一页中
//3.Buf,Num,Address均能被4整除
unsigned int FLASH_WriteBuf(void *Buf,unsigned int Num,unsigned int Address)
{
	unsigned int i;
	FLASH_Unlock();	 //关闭写保护
	if( (unsigned int)Buf%4!=0 || (unsigned int)Num%4!=0 || (unsigned int)Address%4!=0) //参数对齐检查
		return 0;
	if( Address<PAGEADDRESS || Address>(PAGEADDRESS+FLASH_PAGE_SIZE-Num) ) //参数范围检查
		return 0;
	if(FLASH_ErasePage(PAGEADDRESS)!=FLASH_COMPLETE)//檫除
		return 0;
	for(i=0;i<Num/4;i++)
	{
		if( FLASH_ProgramWord(Address+4*i, ((unsigned int *)Buf)[i] ) != FLASH_COMPLETE) 
			return 0;
	}
	FLASH_Lock();   //开启写保护
	return 1; 
}

#ifndef __FLASH_H__
#define __FLASH_H__

//功能  向指定地址的FLASH里写数据
//参数  Buf: 写缓存首地址  Num：字节数 Address: 地址
//1.FLASH的最后一页不能有程序或数据
//2.Buf,Num,Address均能被4整除
//3.Address必须位于FLASH的最后一页中
//返回 1: 成功 0: 失败
unsigned int FLASH_WriteBuf(void *Buf,unsigned int Num,unsigned int Address)

#endif

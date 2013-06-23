#ifndef QSYS_PUBLIC_FUNC_H
#define QSYS_PUBLIC_FUNC_H


//2									公用函数	(PublicFunc.c)						

//将utf8字符串StrUtf8转换成c标准字符串存于StrOem中并返回头地址，出错返回NULL。
//StrOemByte指定了StrOem缓存长度，strlen(StrOem)必须不小于strlen(StrUtf8)
u8 *UTF8_To_OEM(const u8 *StrUtf8,u8 *StrOem,u16 StrOemByte);

//字符串转换为整形
//pStr必须是十进制的数字，如果不符合要求，返回0
u32 StrToUint(u8 *pStr);

//浮点型转换为整型，四舍五入
int FloatToInt(float f);


#if 0
void MemSet(void *Dst,u8 C,u16 Byte);
void MemCpy(void *Dst,const void *Src,u16 Byte);
#else
#define MemSet memset
#define MemCpy memcpy
#endif

//比较两个内存块中字节是否相同，返回TRUE表示相同，Len指定比较长度
bool CompareBuf(u8 *Buf1,u8 *Buf2,u16 Len);

//打印buf内容
void DisplayBuf(const u8 *Buf,u16 Len,u8 RawLen);

//截屏
//成功返回TRUE
bool PrtScreen(void);

//保存屏幕的指定区域色彩到文件
bool PrtScreenToBin(u8 *pNewFilePath,u16 x,u16 y,u16 w,u16 h);

//保存spi flash指定页为文件
bool ReadSpiFlashToBin(u8 *pNewFilePath,u32 StartPage,u32 EndPage);

//分析单个后缀名
//suffix内容类似于".mp3"
bool CheckSuffix(u8 *Str,u8 *Suffix,u8 SuffixLen);

//分析多个后缀名
//suffix内容类似于".mp3|.wav|.txt"
bool CheckMultiSuffix(u8 *Str,u8 *SuffixStr);

#ifdef USE_PAGE_FUNC_GUI
//主题1的标题显示
void DrawTitle1(FONT_ACT FontLib,const u8 *pTitle,u16 xStart,u8 ByteLen,COLOR_TYPE Color);

//主题1的框架显示
void DrawFrame1(u16 y_start,u16 h);
#endif

u32 MakeHash33(u8 *pData,u32 Len);

u16 CRC16(const u8 *pData,u16 Len);

u16 Rev16(u16 Data);

u32 Rev32(u32 Data);

u32 Rand(u32 Mask);

u32 GetHwID(void);

#endif


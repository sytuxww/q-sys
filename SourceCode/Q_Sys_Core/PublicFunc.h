#ifndef QSYS_PUBLIC_FUNC_H
#define QSYS_PUBLIC_FUNC_H


//2									���ú���	(PublicFunc.c)						

//��utf8�ַ���StrUtf8ת����c��׼�ַ�������StrOem�в�����ͷ��ַ��������NULL��
//StrOemByteָ����StrOem���泤�ȣ�strlen(StrOem)���벻С��strlen(StrUtf8)
u8 *UTF8_To_OEM(const u8 *StrUtf8,u8 *StrOem,u16 StrOemByte);

//�ַ���ת��Ϊ����
//pStr������ʮ���Ƶ����֣����������Ҫ�󣬷���0
u32 StrToUint(u8 *pStr);

//������ת��Ϊ���ͣ���������
int FloatToInt(float f);


#if 0
void MemSet(void *Dst,u8 C,u16 Byte);
void MemCpy(void *Dst,const void *Src,u16 Byte);
#else
#define MemSet memset
#define MemCpy memcpy
#endif

//�Ƚ������ڴ�����ֽ��Ƿ���ͬ������TRUE��ʾ��ͬ��Lenָ���Ƚϳ���
bool CompareBuf(u8 *Buf1,u8 *Buf2,u16 Len);

//��ӡbuf����
void DisplayBuf(const u8 *Buf,u16 Len,u8 RawLen);

//����
//�ɹ�����TRUE
bool PrtScreen(void);

//������Ļ��ָ������ɫ�ʵ��ļ�
bool PrtScreenToBin(u8 *pNewFilePath,u16 x,u16 y,u16 w,u16 h);

//����spi flashָ��ҳΪ�ļ�
bool ReadSpiFlashToBin(u8 *pNewFilePath,u32 StartPage,u32 EndPage);

//����������׺��
//suffix����������".mp3"
bool CheckSuffix(u8 *Str,u8 *Suffix,u8 SuffixLen);

//���������׺��
//suffix����������".mp3|.wav|.txt"
bool CheckMultiSuffix(u8 *Str,u8 *SuffixStr);

#ifdef USE_PAGE_FUNC_GUI
//����1�ı�����ʾ
void DrawTitle1(FONT_ACT FontLib,const u8 *pTitle,u16 xStart,u8 ByteLen,COLOR_TYPE Color);

//����1�Ŀ����ʾ
void DrawFrame1(u16 y_start,u16 h);
#endif

u32 MakeHash33(u8 *pData,u32 Len);

u16 CRC16(const u8 *pData,u16 Len);

u16 Rev16(u16 Data);

u32 Rev32(u32 Data);

u32 Rand(u32 Mask);

u32 GetHwID(void);

#endif


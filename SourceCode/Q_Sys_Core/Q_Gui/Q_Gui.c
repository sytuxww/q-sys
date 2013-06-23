/*��������:
һ������£��Ҳ�ϲ���ñ����ֳɵ�gui����Ϊͨ�õ�guiһ�㶼�����
�ض���Ӳ�����Ż����������Լ�д�����gui����һ����С�ģ��ܼ�
��gui���Ժ�������ƣ����Ҳ���Զཻ����

stm32��о��320x240����Ļ��ʵ�Ǻܳ����ģ���Ҫ������ͼƬ���ݱ������
ROM�ⲿ,�����ݻ���������ʾ����,���Դ�ҿ��Կ�������ʾͼ
Ƭ��ʱ��������Ե�ˢ����Ϊ�˽�������ˢ������ҿ����������Ż�:
1.����亯�����潥���bmp����
2.��������bmp
3.��bmp�Ƚ��룬��ͼƬ��Ӧ��ɫ����Ϣת����һ������(Image2Lcd����)��
��ŵ��ļ�ϵͳ��spi flash�У�Ȼ���д��Ӧ����溯����

ÿ�������ڲ��ʼ�������Gui_SetRegion���÷�Χ
*/
#include "System.h"
#include "Drivers.h"
#include "Lcd.h"

#define Gui_Debug Debug

extern OS_MutexHandler gLCD_Mutex;			//lcd����������
#define LCD_Lock() //OS_SemaphoreTake(gLCD_Mutex,OS_MAX_DELAY)
#define LCD_UnLock() //OS_SemaphoreGive(gLCD_Mutex)

//static FIL gBmpObj; 
#if GUI_USE_FS_FONT_LIB == 1
static FS_FILE *gpFontObj;
#endif
static BMP_INFO gBmpInfo;
//__align(4) static u8 gBmpColorTableBuf[GUI_BMP_COLOR_TABLE_BUF_SIZE];//��ͼ����
static u8 * gBmpColorTableBuf;//��ͼ�������ָ�뷽ʽ��heap�������ڴ�
static bool gLandScapeMode=FALSE;//true ����

#define LandScapeAddrIncMode	 xDec_yInc	//����ģʽ�µĵ�ַ����ģʽ
#define NormalAddrIncMode xInc_yInc//����ģʽ�µĵ�ַ����ģʽ

//��16λRGB(565)ɫ�ʻ����16λBGR(565)ɫ��
#define RGB2BGR(Color16)	(((Color16>>11)&0x001f)|(Color16&0x07e0)|((Color16<<11)&0xf800))

//���ڻ�ȡspi flash������ֿ�
static void __inline Gui_ReadFontLib(u32 Base,u32 local,u8 *buf,u32 len)
{
#if GUI_USE_FS_FONT_LIB
	UINT ReadByte;

	if(Q_DB_GetStatus(Status_FsInitFinish,NULL))
	{
		if(FS_FSeek(gpFontObj,local,FS_SEEK_SET)!=0)
		{
			Gui_Debug("Font Lib fseek err!\n\r");
			return;
		}	

		ReadByte=FS_FRead((void *)buf,len,1,gpFontObj);
		if(ReadByte==0)
	    {
	    	Gui_Debug("Read Font Lib error!\n\r");
			return;
	    }
	}
	else
		MemSet(buf,0,len);
	//else
	//{
	//	Q_SpiFlashSync(FlashRead,GUI_GBK16_FONT_SPI_FLASH_BASE+local,len,buf);
	//}
#else
	Q_SpiFlashSync(FlashRead,Base+local,len,buf);
#endif
}

//���ڻ�ȡspi�����ͼƬ����
static void __inline Gui_ReadSpiFlash(u32 local,u8 *buf,u32 len)
{
	Q_SpiFlashSync(FlashRead,local,len,buf);
}

void Gui_Init(void)
{
	//SPI_Flash_Init();
	LCD_Init();	
	Gui_Debug("Gui initialize OK!\n\r");
}

void Gui_DeInit(void)
{
	return;
}

//���ñ���
//Value��Χ0-100
void Gui_SetBgLight(u8 Value)
{
	if(Value>100) return;
	LCD_Light_Set(Value);
}

//��ȡ��������
//����ֵ��Χ0-100
u8 Gui_GetBgLightVal(void)
{
	return LCD_Light_State();
}

//LandScapeMode:ture ����,false ����
//ע�⻻�˺���֮��,˼άҲҪ�����
//xy��������Ȼ����,����w��h��Ҫ�Ե���
//��ͼ��д�ֶ��Ǹ�����������ϵ�����
GUI_RESULT Gui_SetLandScapeMode(bool LandScape)
{
	if(LandScape==gLandScapeMode) return Gui_True;
	
	LCD_Lock();
	if(LandScape)
	{	//����
		gLandScapeMode=TRUE;
		LCD_SetAddrIncMode(LandScapeAddrIncMode);
	}
	else
	{	//����
		gLandScapeMode=FALSE;
		LCD_SetAddrIncMode(NormalAddrIncMode);
	}
	LCD_UnLock();
	return Gui_True;
}

bool Gui_GetLandScapeMode(void)
{
	return gLandScapeMode;
}

//������ʾ����,���ú���ģʽ,������귶Χ�Ƿ���ȷ
//�����Ǻ�����������ģʽ,x_start,y_start��Ϊ����������Ͻǵ�����
//����,������뻭һ������0,0-100,100
//����ģʽ��,x_start,y_start=0,0
//����ģʽ��,x_start,y_start=100,0
//w��h����101
//ֻ�ܱ�Gui.c�ڲ��ĺ���ʹ��,��Ϊû�д�����
static GUI_RESULT Gui_SetRegion(u16 x_start,u16 y_start,u16 w,u16 h)
{
	u16 x_end;
	u16 y_end;

	if(gLandScapeMode)
	{	//����ģʽ	
		x_end=x_start;
		x_start=x_end+1-h;
		y_end=y_start+w-1;

		if((x_start>=LCD_WIDTH)||(y_start>=LCD_HIGHT)||(x_end>=LCD_WIDTH)||(y_end>=LCD_HIGHT))
		{
			Gui_Debug("Gui display region overflow!(%d,%d) - (%d,%d)\n\r",x_start,y_start,x_end,y_end);
			return Gui_Out_Of_Range;
		}	

		LCD_SetRegion(x_start,y_start,x_end,y_end,gLandScapeMode);
		LCD_SetAddrIncMode(LandScapeAddrIncMode);
		LCD_SetXY(x_end,y_start);
	}
	else
	{	//����ģʽ
		x_end=x_start+w-1;
		y_end=y_start+h-1;

		if((x_start>=LCD_WIDTH)||(y_start>=LCD_HIGHT)||(x_end>=LCD_WIDTH)||(y_end>=LCD_HIGHT))
		{
			Gui_Debug("Gui display region overflow!(%d,%d) - (%d,%d)\n\r",x_start,y_start,x_end,y_end);
			return Gui_Out_Of_Range;
		}	
	
		LCD_SetRegion(x_start,y_start,x_end,y_end,gLandScapeMode);
		LCD_SetAddrIncMode(NormalAddrIncMode);
		LCD_SetXY(x_start,y_start);
	}

	return Gui_True;
}

//�õ�ɫ���һ������
GUI_RESULT Gui_FillBlock(const GUI_REGION *pRegion)
{
	u32 Temp;  
	COLOR_TYPE Color;

	LCD_Lock();
	if(Gui_SetRegion(pRegion->x,pRegion->y,pRegion->w,pRegion->h)==Gui_Out_Of_Range)
	{
		LCD_UnLock();
		return Gui_Out_Of_Range;
	}

#if USE_32BIT_COLOR_TYPE
	Color=ChColor(pRegion->Color);//ת����ɫ	
#else
	Color=pRegion->Color;
#endif

	LCD_BlukWriteDataStart();
	
	for (Temp=pRegion->w*pRegion->h;Temp;Temp--)
	{
		LCD_BulkWriteData(Color);
	} 
	LCD_UnLock();

	return Gui_True;
}

//��ָ����ɫ�����������Gui_FillScreen(FatColor(0x000000))
GUI_RESULT Gui_FillScreen(COLOR_TYPE Color)
{
	GUI_REGION DrawRegion;
	
	DrawRegion.x=0;
	DrawRegion.y=0;
	DrawRegion.w=LCD_WIDTH;
	DrawRegion.h=LCD_HIGHT;
	DrawRegion.Color=Color;
	return Gui_FillBlock(&DrawRegion);			 
}

//�õ�ɫ���PixelBuf����1��λ��Ӧ�����ص�
//PixelBufÿһ��bit��Ӧһ�����ص����
//���һ���ֽڶ�Ӧ8���㣬����Ҫע���ֽڶ���
//pRegion->x ,yָ��ͼ�����
//pRegion->w ,hָ��ͼ����
GUI_RESULT Gui_PixelFill(const u8 *PixelBuf,const GUI_REGION *pRegion)
{
	u8 RowByte=((pRegion->w-1)>>3);//ÿ��ռ�ֽ���-1
	u8 LastNull=8-(pRegion->w-(RowByte<<3));//���һ���ֽڲ���Ҫ��ʾ�����ݸ���
	u8 Row;
	s8 Bit;
	u32 i,Byte=0;
	COLOR_TYPE Color;
	
	LCD_Lock();
	if(Gui_SetRegion(pRegion->x,pRegion->y,pRegion->w,pRegion->h)==Gui_Out_Of_Range)
	{
		LCD_UnLock();
		return Gui_Out_Of_Range;
	}

#if USE_32BIT_COLOR_TYPE
	Color=ChColor(pRegion->Color);//ת����ɫ	
#else
	Color=pRegion->Color;
#endif

	LCD_BlukWriteDataStart();
	
	for (Row = 0; Row < pRegion->h; Row++)//��
	{
		for(i=0;i<RowByte;i++,Byte++)//ǰ�������ֽ�
		{
			for(Bit=7;Bit>=0;Bit--)//��ȡ��ǰ�ֽ�
			{
				if(PixelBuf[Byte]&(1<<Bit))
				{
					LCD_BulkWriteData(Color);
				}
				else
				{
					LCD_AddrInc();
				}
			}
		}

		for(Bit=7;Bit>=LastNull;Bit--) //���һ���Ƕ����ֽ�
		{
			if(PixelBuf[Byte]&(1<<Bit))
			{
				LCD_BulkWriteData(Color);
			}
			else
			{
				LCD_AddrInc();
			}
		}
		Byte++;
	} 
	
	LCD_UnLock();
	return Gui_True;
}

//�Ӵ洢���ж�ȡ�ֿ���ʾ����
//����ʵ�ʶ�ȡ��byte
//ע��region.space��region.row_space���ڿ����־�,�ұ�������ֵ!!!
static u16 Gui_DrawFont_GBK16(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//�����ַ�ռ��λ�ã������ر�ʾ
	u32 FontLocal;	//�ֿ�ƫ����
	u8 CharBuf[GUI_GBK16_FONT_BUF_SIZE];
	u8 w,h,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	COLOR_TYPE Color;

#if USE_32BIT_COLOR_TYPE
	Color=ChColor(pRegion->Color);//ת����ɫ	
#else
	Color=pRegion->Color;
#endif

	LCD_Lock();
	
	while((Offset=*pStr++)!=0) //��ʼ��ȡ�ַ���
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//��ʾ����
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//Խ�׽���
			if((RowLen+GUI_GBK16_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			FontLocal=((Offset-GUI_GBK_FONT_OFFSET_H)*GUI_GBK_FONT_PAGENUM+
				((*pStr)-GUI_GBK_FONT_OFFSET_L))<<5;	//��ȡƫ����,<<5= *GUI_GBK16_FONT_BUF_SIZE
			if((*pStr++) > GUI_GBK_FONT_OFFSET_P) FontLocal-=GUI_GBK16_FONT_BUF_SIZE; //��ȥ�ֿ���0x7f������
			
			//�Ƶ��ֿ��Ӧλ�ö�ȡһ���ֵĵ�������,32���ֽ�
			Gui_ReadFontLib(GUI_GBK16_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_GBK16_FONT_BUF_SIZE);

			if(Gui_GetLandScapeMode())//����
				Gui_SetRegion(pRegion->x-RowLen,pRegion->y+ColLen,GUI_GBK16_CHAR_WIDTH,GUI_GBK16_FONT_HEIGHT);
			else//����
				Gui_SetRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_GBK16_CHAR_WIDTH,GUI_GBK16_FONT_HEIGHT);

			LCD_BlukWriteDataStart();
			for(h=0;h<GUI_GBK16_FONT_BUF_SIZE;h++)//��ʾһ�����ֹ�32���ֽڵĵ�����Ϣ
			{
				for(w=8;w!=0;w--)//��ȡ��ǰ�ֽ�
				{
					if(CharBuf[h]&0x80)
					{
						LCD_BulkWriteData(Color);
					}
					else
					{
						LCD_AddrInc();
					}
					CharBuf[h]<<=1;
				}
			}
			
			ColLen+=(GUI_GBK16_CHAR_WIDTH+Space);
			//Խ�߽���
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK16_CHAR_WIDTH;
			else w=GUI_GBK16_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//�������Ԥ����
			{				
				ColLen=0;
				RowLen+=(GUI_GBK16_FONT_HEIGHT+RowSpace);
			}

			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//��ʾ����
		{
			//Debug("C:%c ",*(pStr-1));
			//Խ�׽���
			if((RowLen+GUI_GBK16_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//����������
			{
				case 0x0A:	//���лس������
				{
					ColLen=0;
					RowLen+=(GUI_GBK16_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //��ͨ�ַ������
				{
					FontLocal=GUI_GBK16_FONT_END+Offset*GUI_GBK16_FONT_BUF_SIZE;//��ȡ�ֿ���ASCii��ƫ����
					
					//�Ƶ��ֿ��Ӧλ�ö�ȡһ���ֵĵ�������,32���ֽ�
					Gui_ReadFontLib(GUI_GBK16_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_GBK16_FONT_BUF_SIZE);
					
					if(Gui_GetLandScapeMode())//����
						Gui_SetRegion(pRegion->x-RowLen,pRegion->y+ColLen,GUI_GBK16_ASCII_WIDTH,GUI_GBK16_FONT_HEIGHT);
					else//����
						Gui_SetRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_GBK16_ASCII_WIDTH,GUI_GBK16_FONT_HEIGHT);

					LCD_BlukWriteDataStart();
					for(h=0;h<GUI_GBK16_FONT_BUF_SIZE;h+=2)//��ʾһ��ascii��16���ֽڵĵ�����Ϣ
					{
						for(w=GUI_GBK16_ASCII_WIDTH;w!=0;w--)//��ȡ��ǰ�ֽ�
						{
							if(CharBuf[h]&0x80)
							{
								LCD_BulkWriteData(Color);
							}
							else
							{
								LCD_AddrInc();
							}
							CharBuf[h]<<=1;
						}
					}	
					
					ColLen+=(GUI_GBK16_ASCII_WIDTH+Space);
					//Խ�߽���
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK16_CHAR_WIDTH;
					else w=GUI_GBK16_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//�������Ԥ����
					{				
						ColLen=0;
						RowLen+=(GUI_GBK16_FONT_HEIGHT+RowSpace);
					}
					//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
				}
			}
		}
	}

	LCD_UnLock();
	return (u16)(pStr-Str-1);
}

//����ʵ��������Ҫ���ַ���byte��
//����ֵʵ�ʾ���Gui_DrawFont_GBK16�ķ���ֵ��ֻ��������ʾʵ������
static u16 Gui_DrawFont_GBK16_Num(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//�����ַ�ռ��λ�ã������ر�ʾ
	u8 w,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	
	while((Offset=*pStr++)!=0) //��ʼ��ȡ�ַ���
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//��ʾ����
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//Խ�׽���
			if((RowLen+GUI_GBK16_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			pStr++;
			
			ColLen+=(GUI_GBK16_CHAR_WIDTH+Space);
			//Խ�߽���
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK16_CHAR_WIDTH;
			else w=GUI_GBK16_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//�������Ԥ����
			{				
				ColLen=0;
				RowLen+=(GUI_GBK16_FONT_HEIGHT+RowSpace);
			}
			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//��ʾ����
		{
			//Debug("C:%c ",*(pStr-1));
			//Խ�׽���
			if((RowLen+GUI_GBK16_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//����������
			{
				case 0x0A:	//���лس������
				{
					ColLen=0;
					RowLen+=(GUI_GBK16_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //��ͨ�ַ������
				{					
					ColLen+=(GUI_GBK16_ASCII_WIDTH+Space);
					//Խ�߽���
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK16_CHAR_WIDTH;
					else w=GUI_GBK16_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//�������Ԥ����
					{				
						ColLen=0;
						RowLen+=(GUI_GBK16_FONT_HEIGHT+RowSpace);
					}
					//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
				}
			}
		}
	}

	return (u16)(pStr-Str-1);
}

//�Ӵ洢���ж�ȡ�ֿ���ʾ����
//����ʵ�ʶ�ȡ��byte
//ע��region.space��region.row_space���ڿ����־�,�ұ�������ֵ!!!
static u16 Gui_DrawFont_GBK12(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//�����ַ�ռ��λ�ã������ر�ʾ
	u32 FontLocal;	//�ֿ�ƫ����
	u8 CharBuf[GUI_GBK12_FONT_BUF_SIZE];
	u8 w,h,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	COLOR_TYPE Color;

#if USE_32BIT_COLOR_TYPE
	Color=ChColor(pRegion->Color);//ת����ɫ	
#else
	Color=pRegion->Color;
#endif

	LCD_Lock();
	
	while((Offset=*pStr++)!=0) //��ʼ��ȡ�ַ���
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//��ʾ����
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//Խ�׽���
			if((RowLen+GUI_GBK12_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			FontLocal=((Offset-GUI_GBK_FONT_OFFSET_H)*GUI_GBK_FONT_PAGENUM+
				((*pStr)-GUI_GBK_FONT_OFFSET_L))*GUI_GBK12_FONT_BUF_SIZE;	//�ӵ�һ���ֽڵõ�ƫ����
			if((*pStr++) > GUI_GBK_FONT_OFFSET_P) FontLocal-=GUI_GBK12_FONT_BUF_SIZE; //��ȥ�ֿ���0x7f������
			
			//�Ƶ��ֿ��Ӧλ�ö�ȡһ���ֵĵ�������,24���ֽ�
			Gui_ReadFontLib(GUI_GBK12_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_GBK12_FONT_BUF_SIZE);

			if(Gui_GetLandScapeMode())//����
				Gui_SetRegion(pRegion->x-RowLen,pRegion->y+ColLen,GUI_GBK12_CHAR_WIDTH,GUI_GBK12_FONT_HEIGHT);
			else//����
				Gui_SetRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_GBK12_CHAR_WIDTH,GUI_GBK12_FONT_HEIGHT);

			LCD_BlukWriteDataStart();
			for(h=0;h<GUI_GBK12_FONT_BUF_SIZE;h++)//��ʾһ�����ֹ�24���ֽڵĵ�����Ϣ
			{
				for(w=8;w!=0;w--)//��ȡ��ǰ�ֽ�
				{
					if(CharBuf[h]&0x80)
					{
						LCD_BulkWriteData(Color);
					}
					else
					{
						LCD_AddrInc();
					}
					CharBuf[h]<<=1;
				}
				
				for(w=4,h++;w!=0;w--)
				{
					if(CharBuf[h]&0x80)
					{
						LCD_BulkWriteData(Color);
					}
					else
					{
						LCD_AddrInc();
					}
					CharBuf[h]<<=1;
				}
			}
			
			ColLen+=(GUI_GBK12_CHAR_WIDTH+Space);
			//Խ�߽���
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK12_CHAR_WIDTH;
			else w=GUI_GBK12_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//�������Ԥ����
			{				
				ColLen=0;
				RowLen+=(GUI_GBK12_FONT_HEIGHT+RowSpace);
			}

			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//��ʾ����
		{
			//Debug("C:%c ",*(pStr-1));
			//Խ�׽���
			if((RowLen+GUI_GBK12_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//����������
			{
				case 0x0A:	//���лس������
				{
					ColLen=0;
					RowLen+=(GUI_GBK12_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //��ͨ�ַ������
				{
					FontLocal=GUI_GBK12_FONT_END+Offset*GUI_GBK12_FONT_BUF_SIZE;//��ȡ�ֿ���ASCii��ƫ����
					
					//�Ƶ��ֿ��Ӧλ�ö�ȡһ���ֵĵ�������,24���ֽ�
					Gui_ReadFontLib(GUI_GBK12_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_GBK12_FONT_BUF_SIZE);
					
					if(Gui_GetLandScapeMode())//����
						Gui_SetRegion(pRegion->x-RowLen,pRegion->y+ColLen,GUI_GBK12_ASCII_WIDTH,GUI_GBK12_FONT_HEIGHT);
					else//����
						Gui_SetRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_GBK12_ASCII_WIDTH,GUI_GBK12_FONT_HEIGHT);

					LCD_BlukWriteDataStart();
					for(h=0;h<GUI_GBK12_FONT_BUF_SIZE;h+=2)//��ʾһ��ascii��24���ֽڵĵ�����Ϣ
					{
						for(w=GUI_GBK12_ASCII_WIDTH;w!=0;w--)//��ȡ��ǰ�ֽ�
						{
							if(CharBuf[h]&0x80)
							{
								LCD_BulkWriteData(Color);
							}
							else
							{
								LCD_AddrInc();
							}
							CharBuf[h]<<=1;
						}
					}	
					
					ColLen+=(GUI_GBK12_ASCII_WIDTH+Space);
					//Խ�߽���
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK12_CHAR_WIDTH;
					else w=GUI_GBK12_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//�������Ԥ����
					{				
						ColLen=0;
						RowLen+=(GUI_GBK12_FONT_HEIGHT+RowSpace);
					}
					//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
				}
			}
		}
	}

	LCD_UnLock();
	return (u16)(pStr-Str-1);
}

//����ʵ��������Ҫ���ַ���byte��
//����ֵʵ�ʾ���Gui_DrawFont_GBK12�ķ���ֵ��ֻ��������ʾʵ������
static u16 Gui_DrawFont_GBK12_Num(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//�����ַ�ռ��λ�ã������ر�ʾ
	u8 w,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	
	while((Offset=*pStr++)!=0) //��ʼ��ȡ�ַ���
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//��ʾ����
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//Խ�׽���
			if((RowLen+GUI_GBK12_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			pStr++;
			
			ColLen+=(GUI_GBK12_CHAR_WIDTH+Space);
			//Խ�߽���
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK12_CHAR_WIDTH;
			else w=GUI_GBK12_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//�������Ԥ����
			{				
				ColLen=0;
				RowLen+=(GUI_GBK12_FONT_HEIGHT+RowSpace);
			}
			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//��ʾ����
		{
			//Debug("C:%c ",*(pStr-1));
			//Խ�׽���
			if((RowLen+GUI_GBK12_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//����������
			{
				case 0x0A:	//���лس������
				{
					ColLen=0;
					RowLen+=(GUI_GBK12_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //��ͨ�ַ������
				{					
					ColLen+=(GUI_GBK12_ASCII_WIDTH+Space);
					//Խ�߽���
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK12_CHAR_WIDTH;
					else w=GUI_GBK12_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//�������Ԥ����
					{				
						ColLen=0;
						RowLen+=(GUI_GBK12_FONT_HEIGHT+RowSpace);
					}
					//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
				}
			}
		}
	}

	return (u16)(pStr-Str-1);
}

//�Ӵ洢���ж�ȡ�ֿ���ʾ����
//����ʵ�ʶ�ȡ��byte
//ע��region.space��region.row_space���ڿ����־�,�ұ�������ֵ!!!
static u16 Gui_DrawFont_ASC14B(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//�����ַ�ռ��λ�ã������ر�ʾ
	u32 FontLocal;	//�ֿ�ƫ����
	u8 CharBuf[GUI_ASC14B_FONT_BUF_SIZE];
	u8 w,h,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	COLOR_TYPE Color;

#if USE_32BIT_COLOR_TYPE
	Color=ChColor(pRegion->Color);//ת����ɫ	
#else
	Color=pRegion->Color;
#endif

	LCD_Lock();
	while((Offset=*pStr++)!=0) //��ʼ��ȡ�ַ���
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//��ʾ����
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//Խ�׽���
			if((RowLen+GUI_ASC14B_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}

#if 0 //���ֿⲻ֧�ֺ���			
			FontLocal=((Offset-GUI_GBK_FONT_OFFSET_H)*GUI_GBK_FONT_PAGENUM+
				((*pStr)-GUI_GBK_FONT_OFFSET_L))*GUI_ASC14B_FONT_BUF_SIZE;	//�ӵ�һ���ֽڵõ�ƫ����
			if((*pStr++) > GUI_GBK_FONT_OFFSET_P) FontLocal-=GUI_ASC14B_FONT_BUF_SIZE; //��ȥ�ֿ���0x7f������
			
			//�Ƶ��ֿ��Ӧλ�ö�ȡһ���ֵĵ�������,24���ֽ�
			Gui_ReadFontLib(GUI_ASC14B_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_ASC14B_FONT_BUF_SIZE);
#else
			pStr++;
#endif

			if(Gui_GetLandScapeMode())//����
				Gui_SetRegion(pRegion->x-RowLen,pRegion->y+ColLen,GUI_ASC14B_CHAR_WIDTH,GUI_ASC14B_FONT_HEIGHT);
			else//����
				Gui_SetRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_ASC14B_CHAR_WIDTH,GUI_ASC14B_FONT_HEIGHT);

#if 1		//���ڲ�֧�ֺ��֣���ʾ����
			LCD_BlukWriteDataStart();
			for(h=0;h<GUI_ASC14B_FONT_BUF_SIZE;h+=2)//��ʾһ�����ֹ�28���ֽڵĵ�����Ϣ
			{
				for(w=GUI_ASC14B_CHAR_WIDTH;w!=0;w--)//��ȡ��ǰ�ֽ�
				{
					//if(CharBuf[h]&0x80)
					{
						LCD_BulkWriteData(Color);
					}
					//else
					//{
					//	LCD_AddrInc();
					//}
					//CharBuf[h]<<=1;
				}
			}
#endif

			ColLen+=(GUI_ASC14B_CHAR_WIDTH+Space);
			//Խ�߽���
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_ASC14B_CHAR_WIDTH;
			else w=GUI_ASC14B_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//�������Ԥ����
			{				
				ColLen=0;
				RowLen+=(GUI_ASC14B_FONT_HEIGHT+RowSpace);
			}

			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//��ʾ����
		{
			//Debug("C:%c ",*(pStr-1));
			//Խ�׽���
			if((RowLen+GUI_ASC14B_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//����������
			{
				case 0x0A:	//���лس������
				{
					ColLen=0;
					RowLen+=(GUI_ASC14B_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //��ͨ�ַ������
				{
					FontLocal=Offset*GUI_ASC14B_FONT_BUF_SIZE;//��ȡ�ֿ���ASCii��ƫ����
					
					//�Ƶ��ֿ��Ӧλ�ö�ȡһ���ֵĵ�������,28���ֽ�
					Gui_ReadFontLib(GUI_ASC14B_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_ASC14B_FONT_BUF_SIZE);
					
					if(Gui_GetLandScapeMode())//����
						Gui_SetRegion(pRegion->x-RowLen,pRegion->y+ColLen,GUI_ASC14B_ASCII_WIDTH,GUI_ASC14B_FONT_HEIGHT);
					else//����
						Gui_SetRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_ASC14B_ASCII_WIDTH,GUI_ASC14B_FONT_HEIGHT);

					LCD_BlukWriteDataStart();
					for(h=0;h<GUI_ASC14B_FONT_BUF_SIZE;h++)//��ʾһ��ascii��28���ֽڵĵ�����Ϣ
					{
						for(w=8;w!=0;w--)//��ȡ��ǰ�ֽ�
						{
							if(CharBuf[h]&0x80)
							{
								LCD_BulkWriteData(Color);
							}
							else
							{
								LCD_AddrInc();
							}
							CharBuf[h]<<=1;
						}
						
						for(w=GUI_ASC14B_ASCII_WIDTH-8,h++;w!=0;w--)
						{
							if(CharBuf[h]&0x80)
							{
								LCD_BulkWriteData(Color);
							}
							else
							{
								LCD_AddrInc();
							}
							CharBuf[h]<<=1;
						}						
					}	
					
					ColLen+=(GUI_ASC14B_ASCII_WIDTH+Space);
					//Խ�߽���
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_ASC14B_CHAR_WIDTH;
					else w=GUI_ASC14B_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//�������Ԥ����
					{				
						ColLen=0;
						RowLen+=(GUI_ASC14B_FONT_HEIGHT+RowSpace);
					}
					//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
				}
			}
		}
	}

	LCD_UnLock();
	return (u16)(pStr-Str-1);
}

//����ʵ��������Ҫ���ַ���byte��
//����ֵʵ�ʾ���Gui_DrawFont_GBK12�ķ���ֵ��ֻ��������ʾʵ������
static u16 Gui_DrawFont_ASC14B_Num(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//�����ַ�ռ��λ�ã������ر�ʾ
	u8 w,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	
	while((Offset=*pStr++)!=0) //��ʼ��ȡ�ַ���
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//��ʾ����
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//Խ�׽���
			if((RowLen+GUI_ASC14B_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			pStr++;
			
			ColLen+=(GUI_ASC14B_CHAR_WIDTH+Space);
			//Խ�߽���
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_ASC14B_CHAR_WIDTH;
			else w=GUI_ASC14B_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//�������Ԥ����
			{				
				ColLen=0;
				RowLen+=(GUI_ASC14B_FONT_HEIGHT+RowSpace);
			}
			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//��ʾ����
		{
			//Debug("C:%c ",*(pStr-1));
			//Խ�׽���
			if((RowLen+GUI_ASC14B_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//����������
			{
				case 0x0A:	//���лس������
				{
					ColLen=0;
					RowLen+=(GUI_ASC14B_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //��ͨ�ַ������
				{					
					ColLen+=(GUI_ASC14B_ASCII_WIDTH+Space);
					//Խ�߽���
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_ASC14B_CHAR_WIDTH;
					else w=GUI_ASC14B_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//�������Ԥ����
					{				
						ColLen=0;
						RowLen+=(GUI_ASC14B_FONT_HEIGHT+RowSpace);
					}
					//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
				}
			}
		}
	}

	return (u16)(pStr-Str-1);
}

//�Ӵ洢���ж�ȡ�ֿ���ʾ����
//����ʵ�ʶ�ȡ��byte
//ע��region.space��region.row_space���ڿ����־�,�ұ�������ֵ!!!
static u16 Gui_DrawFont_GBK21(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//�����ַ�ռ��λ�ã������ر�ʾ
	u32 FontLocal;	//�ֿ�ƫ����
	u8 CharBuf[GUI_GBK21_FONT_BUF_SIZE];
	u8 w,h,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	COLOR_TYPE Color;

#if USE_32BIT_COLOR_TYPE
	Color=ChColor(pRegion->Color);//ת����ɫ	
#else
	Color=pRegion->Color;
#endif

	LCD_Lock();
	
	while((Offset=*pStr++)!=0) //��ʼ��ȡ�ַ���
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//��ʾ����
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//Խ�׽���
			if((RowLen+GUI_GBK21_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			FontLocal=((Offset-GUI_GBK_FONT_OFFSET_H)*GUI_GBK_FONT_PAGENUM+
				((*pStr)-GUI_GBK_FONT_OFFSET_L))*GUI_GBK21_FONT_BUF_SIZE;	//��ȡƫ����,<<x= *GUI_GBK21_FONT_BUF_SIZE
			if((*pStr++) > GUI_GBK_FONT_OFFSET_P) FontLocal-=GUI_GBK21_FONT_BUF_SIZE; //��ȥ�ֿ���0x7f������
			
			//�Ƶ��ֿ��Ӧλ�ö�ȡһ���ֵĵ�������,63���ֽ�
			Gui_ReadFontLib(GUI_GBK21_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_GBK21_FONT_BUF_SIZE);

			if(Gui_GetLandScapeMode())//����
				Gui_SetRegion(pRegion->x-RowLen,pRegion->y+ColLen,GUI_GBK21_CHAR_WIDTH,GUI_GBK21_FONT_HEIGHT);
			else//����
				Gui_SetRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_GBK21_CHAR_WIDTH,GUI_GBK21_FONT_HEIGHT);

			LCD_BlukWriteDataStart();
			for(h=0;h<GUI_GBK21_FONT_BUF_SIZE;h++)//��ʾһ�����ֹ�63���ֽڵĵ�����Ϣ
			{
				for(w=8;w!=0;w--)//��ȡ��ǰ�ֽ�
				{
					if(CharBuf[h]&0x80)
					{
						LCD_BulkWriteData(Color);
					}
					else
					{
						LCD_AddrInc();
					}
					CharBuf[h]<<=1;
				}

				for(w=8,h++;w!=0;w--)//��ȡ��ǰ�ֽ�
				{
					if(CharBuf[h]&0x80)
					{
						LCD_BulkWriteData(Color);
					}
					else
					{
						LCD_AddrInc();
					}
					CharBuf[h]<<=1;
				}

				for(w=5,h++;w!=0;w--)//��ȡ��ǰ�ֽ�
				{
					if(CharBuf[h]&0x80)
					{
						LCD_BulkWriteData(Color);
					}
					else
					{
						LCD_AddrInc();
					}
					CharBuf[h]<<=1;
				}
			}
			
			ColLen+=(GUI_GBK21_CHAR_WIDTH+Space);
			//Խ�߽���
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK21_CHAR_WIDTH;
			else w=GUI_GBK21_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//�������Ԥ����
			{				
				ColLen=0;
				RowLen+=(GUI_GBK21_FONT_HEIGHT+RowSpace);
			}

			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//��ʾ����
		{
			//Debug("C:%c ",*(pStr-1));
			//Խ�׽���
			if((RowLen+GUI_GBK21_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//����������
			{
				case 0x0A:	//���лس������
				{
					ColLen=0;
					RowLen+=(GUI_GBK21_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //��ͨ�ַ������
				{
					FontLocal=GUI_GBK21_FONT_END+Offset*GUI_GBK21_FONT_BUF_SIZE;//��ȡ�ֿ���ASCii��ƫ����
					
					//�Ƶ��ֿ��Ӧλ�ö�ȡһ���ֵĵ�������,63���ֽ�
					Gui_ReadFontLib(GUI_GBK21_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_GBK21_FONT_BUF_SIZE);
					
					if(Gui_GetLandScapeMode())//����
						Gui_SetRegion(pRegion->x-RowLen,pRegion->y+ColLen,GUI_GBK21_ASCII_WIDTH,GUI_GBK21_FONT_HEIGHT);
					else//����
						Gui_SetRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_GBK21_ASCII_WIDTH,GUI_GBK21_FONT_HEIGHT);

					LCD_BlukWriteDataStart();
					for(h=0;h<GUI_GBK21_FONT_BUF_SIZE;h++)//��ʾһ��ascii��63���ֽڵĵ�����Ϣ
					{
						for(w=8;w!=0;w--)//��ȡ��ǰ�ֽ�
						{
							if(CharBuf[h]&0x80)
							{
								LCD_BulkWriteData(Color);
							}
							else
							{
								LCD_AddrInc();
							}
							CharBuf[h]<<=1;
						}

						for(w=3,h++;w!=0;w--)//��ȡ��ǰ�ֽ�
						{
							if(CharBuf[h]&0x80)
							{
								LCD_BulkWriteData(Color);
							}
							else
							{
								LCD_AddrInc();
							}
							CharBuf[h]<<=1;
						}						
						h++;
					}	
					
					ColLen+=(GUI_GBK21_ASCII_WIDTH+Space);
					//Խ�߽���
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK21_CHAR_WIDTH;
					else w=GUI_GBK21_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//�������Ԥ����
					{				
						ColLen=0;
						RowLen+=(GUI_GBK21_FONT_HEIGHT+RowSpace);
					}
					//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
				}
			}
		}
	}

	LCD_UnLock();
	return (u16)(pStr-Str-1);
}

//����ʵ��������Ҫ���ַ���byte��
//����ֵʵ�ʾ���Gui_DrawFont_GBK16�ķ���ֵ��ֻ��������ʾʵ������
static u16 Gui_DrawFont_GBK21_Num(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//�����ַ�ռ��λ�ã������ر�ʾ
	u8 w,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	
	while((Offset=*pStr++)!=0) //��ʼ��ȡ�ַ���
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//��ʾ����
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//Խ�׽���
			if((RowLen+GUI_GBK21_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			pStr++;
			
			ColLen+=(GUI_GBK21_CHAR_WIDTH+Space);
			//Խ�߽���
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK21_CHAR_WIDTH;
			else w=GUI_GBK21_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//�������Ԥ����
			{				
				ColLen=0;
				RowLen+=(GUI_GBK21_FONT_HEIGHT+RowSpace);
			}
			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//��ʾ����
		{
			//Debug("C:%c ",*(pStr-1));
			//Խ�׽���
			if((RowLen+GUI_GBK21_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//����������
			{
				case 0x0A:	//���лس������
				{
					ColLen=0;
					RowLen+=(GUI_GBK21_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //��ͨ�ַ������
				{					
					ColLen+=(GUI_GBK21_ASCII_WIDTH+Space);
					//Խ�߽���
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK21_CHAR_WIDTH;
					else w=GUI_GBK21_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//�������Ԥ����
					{				
						ColLen=0;
						RowLen+=(GUI_GBK21_FONT_HEIGHT+RowSpace);
					}
					//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
				}
			}
		}
	}

	return (u16)(pStr-Str-1);
}

//ͨ����ʾ�ַ��ĺ���
u16 Gui_DrawFont(FONT_ACT FontLib,const u8 *Str,const GUI_REGION *pRegion)
{
	switch(FontLib)
	{
		case GBK12_FONT:
#if GUI_USE_FS_FONT_LIB//ʹ��sd���ֿ�
		{
			u16 Result;
			
			if(!Q_DB_GetStatus(Status_FsInitFinish,NULL)) return 0;//�ļ�ϵͳδ����
			
			if ((gpFontObj=FS_FOpen("/System/gbk12.a", FA_OPEN_EXISTING | FA_READ)) == 0 ) 
			{
				Gui_Debug("Open Font Lib error!\n\r");
				return 0;
			}

			Result=Gui_DrawFont_GBK12(Str,pRegion);
			
			FS_FClose(gpFontObj);	

			return Result;
		}
#else
			return Gui_DrawFont_GBK12(Str,pRegion);
#endif
		case GBK12_NUM:
			return Gui_DrawFont_GBK12_Num(Str,pRegion);
		case GBK16_FONT:
#if GUI_USE_FS_FONT_LIB//ʹ��sd���ֿ�
		{
			u16 Result;
			
			if(!Q_DB_GetStatus(Status_FsInitFinish,NULL)) return 0;//�ļ�ϵͳδ����
			
			if ((gpFontObj=FS_FOpen("/System/gbk16.a", FA_OPEN_EXISTING | FA_READ)) == 0 ) 
			{
				Gui_Debug("Open Font Lib error!\n\r");
				return 0;
			}

			Result=Gui_DrawFont_GBK16(Str,pRegion);
			
			FS_FClose(gpFontObj);	

			return Result;
		}
#else
			return Gui_DrawFont_GBK16(Str,pRegion);
#endif
		case GBK16_NUM:
			return Gui_DrawFont_GBK16_Num(Str,pRegion);
		case ASC14B_FONT:
#if GUI_USE_FS_FONT_LIB//ʹ��sd���ֿ�
		{
			u16 Result;
			
			if(!Q_DB_GetStatus(Status_FsInitFinish,NULL)) return 0;//�ļ�ϵͳδ����
			
			if ((gpFontObj=FS_FOpen("/System/asc14b.a", FA_OPEN_EXISTING | FA_READ)) == 0 ) 
			{
				Gui_Debug("Open Font Lib error!\n\r");
				return 0;
			}

			Result=Gui_DrawFont_ASC14B(Str,pRegion);
			
			FS_FClose(gpFontObj);	

			return Result;
		}
#else
			return Gui_DrawFont_ASC14B(Str,pRegion);
#endif
		case ASC14B_NUM:
			return Gui_DrawFont_ASC14B_Num(Str,pRegion);
		case GBK21_FONT:
#if GUI_USE_FS_FONT_LIB//ʹ��sd���ֿ�
		{
			u16 Result;
			
			if(!Q_DB_GetStatus(Status_FsInitFinish,NULL)) return 0;//�ļ�ϵͳδ����
			
			if ((gpFontObj=FS_FOpen("/System/gbk21.a", FA_OPEN_EXISTING | FA_READ)) == 0 ) 
			{
				Gui_Debug("Open Font Lib error!\n\r");
				return 0;
			}

			Result=Gui_DrawFont_GBK21(Str,pRegion);
			
			FS_FClose(gpFontObj);	

			return Result;
		}
#else
			return Gui_DrawFont_GBK21(Str,pRegion);
#endif
		case GBK21_NUM:
			return Gui_DrawFont_GBK21_Num(Str,pRegion);

	}
  
  return 0;
}

//��ȡbmp�ļ���ͼƬ�����Ϣ
//��ָ�뷵�ض�ȡֵ
//����ָ�����ΪNULL
GUI_RESULT Gui_ReadBmpInfo(const u8 * pBmpPath,u16 *Width,u16 *Hight)
{
	BMP_INFO *pBmpInfo=&gBmpInfo;
	GUI_RESULT Ret=Gui_True;	
	FS_FILE * fp;
	u8 *ptemp=(u8 *)&gBmpInfo;//for align 4 bytes
	u32 bfOffBits=0;//for align 4 bytes
	u32 biCompression=0;//for align 4 bytes
	u32 biWidth=0;//for align 4 bytes
	u32 biHeight=0;//for align 4 bytes
	//open bmp file
	if ((fp=FS_FOpen((void *)pBmpPath, FA_OPEN_EXISTING | FA_READ)) == 0 ) 
	{
		Gui_Debug("Open bmp error!\n\r");
		return Gui_No_Such_File;
	}

	//read bmp file info
	if((FS_FRead((void *)pBmpInfo, sizeof(BMP_INFO), 1,fp))==0)
	{
		Gui_Debug("Read bmp error!\n\r");
		Ret=Gui_False;
		goto Gui_Return;
	}

	//check bmp file info 
	if(pBmpInfo->bmfHeader.bfType!=0x4d42)
	{
		Gui_Debug("This file is not a bmp!\n\r");
		Ret=Gui_False;
		goto Gui_Return;
	}
	
	//get bfOffBits from gBmpInfo,//for align 4 bytes
	bfOffBits = *((u16 *)(ptemp+10));
	bfOffBits |= ((*((u16 *)(ptemp+12)))<<16);
	
	if(bfOffBits!=0x36)//modify by karlno
	{
		Gui_Debug("Bmp info is error!\n\r");
		Ret=Gui_False;
		goto Gui_Return;
	}

	if(pBmpInfo->bmiHeader.biBitCount!=24)
	{
		Gui_Debug("This bmp is not a 24bit bmp!\n\r");
		Ret=Gui_False;
		goto Gui_Return;
	}
	
	//get biCompression from gBmpInfo,//for align 4 bytes
	biCompression = *((u16 *)(ptemp+30));
	biCompression |= ((*((u16 *)(ptemp+32)))<<16);
	
	if(biCompression!=0)
	{
		Gui_Debug("This bmp compression is error!\n\r");
		Ret=Gui_False;
		goto Gui_Return;
	}
	
	//get biWidth and biHeight from gBmpInfo,//for align 4 bytes
	biWidth = *((u16 *)(ptemp+18));
	biWidth |= ((*((u16 *)(ptemp+20)))<<16);
	biHeight = *((u16 *)(ptemp+22));
	biHeight |= ((*((u16 *)(ptemp+24)))<<16);
	
	//check bmp info success
	if(Width!=NULL)
		*Width=biWidth;//ͼƬ��
	if(Hight!=NULL)
		*Hight=biHeight;//ͼƬ��
	
Gui_Return:
	FS_FClose(fp);

	return Ret;
}

//дһ������bmpʱ��Ϊ172ms
//ֻ��ָ�����x y
//ͼƬ������Χ�Ͳ�����ʾ
//pBmpRegion->TransColorָ��͸��ɫ
GUI_RESULT Gui_Draw24Bmp(const u8 * pBmpPath,const GUI_REGION *pBmpRegion)
{
	BMP_INFO *pBmpInfo=&gBmpInfo;	
	u8 *ptemp=(u8 *)&gBmpInfo;//for align 4 bytes
	u32 bfOffBits=0;   //for align 4 bytes
	u32 biCompression=0;//for align 4 bytes
	u32 biWidth=0;//for align 4 bytes
	u32 biHeight=0;//for align 4 bytes
	GUI_RESULT Ret=Gui_True;
	u32 i;
	u32 OnceReadByte,OnceReadRow;//ÿ����Ҫ��ȡ��byte��ͼƬ��
	UINT ReadByte,ReadRow;//ʵ��ÿ�ζ�ȡ��byte��ͼƬ��
	u32 BmpWidthByte;
	u16 BmpWidth,BmpHight;
	u8 *pBmpRgb;
	bool NeedTrans=FALSE;//�Ƿ���Ҫ͸��ɫ
	u8 BmpMargin;  //  bmp��Ϊ��֤4�ֽڶ������������
	u16 BmpRgb;	//����ת���ֽڵı���
	COLOR_TYPE TransColor=0;
	FS_FILE *fp;	
	u8 SaveData[2];//for align 4 bytes.
	
	//open bmp file
	if ((fp=FS_FOpen((void *)pBmpPath, FA_OPEN_EXISTING | FA_READ)) == 0 ) 
	{
		Gui_Debug("Open bmp error:%s \n\r",pBmpPath);
		return Gui_No_Such_File;
	}

	//read bmp file info
	if((ReadByte=FS_FRead((void *)pBmpInfo, sizeof(BMP_INFO), 1,fp))==0)
	{
		Gui_Debug("Read bmp error!\n\r");
		Ret=Gui_False;
		goto Gui_Return;
	}

	//check bmp file info 
	if(pBmpInfo->bmfHeader.bfType!=0x4d42)
	{
		Gui_Debug("This file is not a bmp!\n\r");
		Ret=Gui_False;
		goto Gui_Return;
	}

	//get bfOffBits from gBmpInfo,//for align 4 bytes
	bfOffBits = *((u16 *)(ptemp+10));
	bfOffBits |= ((*((u16 *)(ptemp+12)))<<16);
	
	if(bfOffBits!=0x36)//modify by karlno
	{
		Gui_Debug("Bmp info is error!\n\r");
		Ret=Gui_False;
		goto Gui_Return;
	}

	if(pBmpInfo->bmiHeader.biBitCount!=24)
	{
		Gui_Debug("This bmp is not a 24bit bmp!\n\r");
		Ret=Gui_False;
		goto Gui_Return;
	}
	
	//get biCompression from gBmpInfo,//for align 4 bytes
	biCompression = *((u16 *)(ptemp+30));
	biCompression |= ((*((u16 *)(ptemp+32)))<<16);
	
	if(biCompression!=0)
	{
		Gui_Debug("This bmp compression is error!\n\r");
		Ret=Gui_False;
		goto Gui_Return;
	}
	//check bmp info success

	SaveData[0]=pBmpInfo->Data[0];
	SaveData[1]=pBmpInfo->Data[1];

	if(FS_FSeek(fp,0x38,FS_SEEK_SET)==-1)//�Ƶ�λͼɫ�ʱ�+2
	{
		Gui_Debug("Bmp fseek err!\r\n");
		Ret=Gui_False;
		goto Gui_Return;
	}	

	//get biWidth and biHeight from gBmpInfo,//for align 4 bytes
	biWidth = *((u16 *)(ptemp+18));
	biWidth |= ((*((u16 *)(ptemp+20)))<<16);
	biHeight = *((u16 *)(ptemp+22));
	biHeight |= ((*((u16 *)(ptemp+24)))<<16);
	
	BmpWidth=biWidth;//ͼƬ��
	BmpHight=biHeight;//ͼƬ��

	BmpMargin=(4-(BmpWidth*3))&0x03;//����bmp���ݱ߰�����ֽ���
	BmpWidthByte=(BmpWidth*3+BmpMargin);
	OnceReadRow=GUI_BMP_COLOR_TABLE_BUF_SIZE/BmpWidthByte;
	OnceReadByte=OnceReadRow*BmpWidthByte;
	
	LCD_Lock();//��ռ��Ļ
	if(Gui_SetRegion(pBmpRegion->x,pBmpRegion->y,BmpWidth,BmpHight)
		==Gui_Out_Of_Range)
	{
		LCD_UnLock();
		Ret=Gui_Out_Of_Range;
		goto Gui_Return;
	}
	
	if(Gui_GetLandScapeMode())//����
	{
		LCD_SetXY(pBmpRegion->x,pBmpRegion->y);
		LCD_SetAddrIncMode(NormalAddrIncMode);
	}
	else//����
	{
		LCD_SetXY(pBmpRegion->x,pBmpRegion->y+BmpHight-1);
		LCD_SetAddrIncMode(xInc_yDec);
	}
	LCD_BgrMode(TRUE);

	if(pBmpRegion->Color!=FatColor(NO_TRANS))//�ж��Ƿ���Ҫ͸��ɫ
	{
#if USE_32BIT_COLOR_TYPE
		TransColor=pBmpRegion->Color;
#else
		TransColor=RGB2BGR(pBmpRegion->Color);//��Ҫת��͸��ɫ��ģʽΪBGR
#endif
		NeedTrans=TRUE;
	}

	gBmpColorTableBuf=Q_Mallco(GUI_BMP_COLOR_TABLE_BUF_SIZE+8);//���仺��
	
	LCD_BlukWriteDataStart();
	while(1)
	{				
		gBmpColorTableBuf[2]=SaveData[0];
	  	gBmpColorTableBuf[3]=SaveData[1];
  	    ReadByte = FS_FRead(&gBmpColorTableBuf[4],OnceReadByte, 1,fp);

  	    if (ReadByte == 0)break;// error or eof
  	    else if(ReadByte==OnceReadByte)
  	    {
	  	    SaveData[0]=gBmpColorTableBuf[OnceReadByte+2];
	  	    SaveData[1]=gBmpColorTableBuf[OnceReadByte+3];  	    
  	    }
  	    else
  	    {
	  	    ReadByte+=2;
  	    }
				
  	    ReadRow=ReadByte/BmpWidthByte;
  	    pBmpRgb=&gBmpColorTableBuf[2];

		if(NeedTrans) //��Ҫ͸��ɫ
		{
			for(;ReadRow;ReadRow--)
			{
				for(i=BmpWidthByte-BmpMargin;i;i-=3)
				{
#if USE_32BIT_COLOR_TYPE
					if(TransColor==((*(COLOR_TYPE *)pBmpRgb)&0xffffff))
					{
						LCD_AddrInc();
						pBmpRgb+=3;
					}
					else
					{
						BmpRgb=((*pBmpRgb++)&0xf8)<<8;
						BmpRgb+=(((*pBmpRgb++)&0xfc)<<3);
						BmpRgb+=(((*pBmpRgb++)&0xf8)>>3);
						//Gui_Debug("%d:%d ",ReadNum,BmpRgb);
						LCD_BulkWriteData(BmpRgb);
					}
#else
					BmpRgb=((*pBmpRgb++)&0xf8)<<8;
					BmpRgb+=(((*pBmpRgb++)&0xfc)<<3);
					BmpRgb+=(((*pBmpRgb++)&0xf8)>>3);
					
					if(TransColor==BmpRgb)
					{
						LCD_AddrInc();
					}
					else
					{
						LCD_BulkWriteData(BmpRgb);
					}
#endif
				}
				pBmpRgb+=BmpMargin;//�������
			}	
		}
		else //����Ҫ͸��ɫ
		{
			for(;ReadRow;ReadRow--)
			{
				for(i=BmpWidthByte-BmpMargin;i;i-=3)
				{
					BmpRgb=((*pBmpRgb++)&0xf8)<<8;
					BmpRgb+=(((*pBmpRgb++)&0xfc)<<3);
					BmpRgb+=(((*pBmpRgb++)&0xf8)>>3);
					//Gui_Debug("%d:%d ",ReadNum,BmpRgb);
					LCD_BulkWriteData(BmpRgb);
				}
				pBmpRgb+=BmpMargin;//�������
			}	
		}
  	}	

  	Q_Free(gBmpColorTableBuf);//�ͷŻ���
  	
	if(Gui_GetLandScapeMode())//����
	{
		LCD_SetAddrIncMode(LandScapeAddrIncMode);
	}
	else//����
	{
		LCD_SetAddrIncMode(NormalAddrIncMode);
	}
	LCD_BgrMode(FALSE);
	LCD_UnLock();//�ͷ���Ļ��Դ
	
Gui_Return:
	if( fp )
	{
		FS_FClose(fp);
	}

	return Ret;
}

//��ȡ���飬��ͼ
//����ȫ��Ϊɫ�����ݣ�ÿ�����ֽڴ���һ������
//�����ָ�����x,y�ͳ���w,h��Ϣ
//дһ������ͼƬʱ��Ϊ32ms����Ȼ������������Rom��
GUI_RESULT Gui_DrawImgArray(const u8 * pImageBuf,const GUI_REGION *pRegion)
{
	u32 i;
	u16 *pColor16=(u16 *)pImageBuf;
	COLOR_TYPE TransColor=0;
	bool NeedTrans=FALSE;
	
	LCD_Lock();//��ռ��Ļ
	if(Gui_SetRegion(pRegion->x,pRegion->y,pRegion->w,pRegion->h)==Gui_Out_Of_Range)
	{
		LCD_UnLock();
		return Gui_Out_Of_Range;
	}

	LCD_BlukWriteDataStart();
	if(pRegion->Color!=FatColor(NO_TRANS))//�ж��Ƿ���Ҫ͸��ɫ
	{
#if USE_32BIT_COLOR_TYPE
		TransColor=ChColor(pRegion->Color);
#else
		TransColor=pRegion->Color;//��Ҫת��͸��ɫ��ģʽΪBGR
#endif
		NeedTrans=TRUE;
	}
	
	if(NeedTrans)
	{
		for(i=(pRegion->w*pRegion->h);i!=0;i--)
	  	{
	  		if(*pColor16==TransColor)
	  		{
				LCD_AddrInc();
				pColor16++;
	  		}
	  		else
	  		{
				LCD_BulkWriteData(*pColor16++);
			}
		}
	}
	else
	{
	  	for(i=(pRegion->w*pRegion->h);i!=0;i--)
	  	{
			LCD_BulkWriteData(*pColor16++);
		}
	}
	
	LCD_UnLock();//�ͷ���Ļ��Դ

	return Gui_True;
}

//��ȡ�ļ�ϵͳbin�ļ�����ͼ
//bin�ļ�ȫ��Ϊɫ�����ݣ�ÿ�����ֽڴ���һ������
//�����ָ�����x,y�ͳ���w,h��Ϣ
//дһ������ͼƬʱ��Ϊ104ms�������Ը���sd���ٶ����
GUI_RESULT Gui_DrawImgBin(const u8 * pPath,const GUI_REGION *pRegion)
{
	GUI_RESULT Ret=Gui_True;
	u32 i;
	UINT ReadByte;//ʵ��ÿ�ζ�ȡ��byte��ͼƬ��
	u16 *pColor16;
	COLOR_TYPE TransColor=0;
	bool NeedTrans=FALSE;
	FS_FILE *fp;
	//open bin file
	if ((fp=FS_FOpen( (void *)pPath, FA_OPEN_EXISTING | FA_READ) )== 0 ) 
	{
		Gui_Debug("Open bmp bin error !:%s\n\r",pPath);
		return Gui_No_Such_File;
	}
	
	LCD_Lock();//��ռ��Ļ
	if(Gui_SetRegion(pRegion->x,pRegion->y,pRegion->w,pRegion->h)==Gui_Out_Of_Range)
	{
		LCD_UnLock();
		Ret=Gui_Out_Of_Range;
		goto Gui_Return;
	}

	if(pRegion->Color!=FatColor(NO_TRANS))//�ж��Ƿ���Ҫ͸��ɫ
	{
#if USE_32BIT_COLOR_TYPE
		TransColor=ChColor(pRegion->Color);
#else
		TransColor=pRegion->Color;//��Ҫת��͸��ɫ��ģʽΪBGR
#endif
		NeedTrans=TRUE;
	}

	gBmpColorTableBuf=Q_Mallco(GUI_BMP_COLOR_TABLE_BUF_SIZE);//���仺��
	
	LCD_BlukWriteDataStart();
	
	while(1)
	{					
  	    ReadByte =FS_FRead(gBmpColorTableBuf,GUI_BMP_COLOR_TABLE_BUF_SIZE,1,fp );

  	    if (ReadByte == 0) // error or eof
  	    {
  	    	break;
  	    }		 

  	    pColor16=(void *)gBmpColorTableBuf;
  	    
		if(NeedTrans)
		{
	  	    for(i=ReadByte>>1;i!=0;i--)
	  	    {
	  	    	if(*pColor16==TransColor)
	  	    	{
					LCD_AddrInc();
					pColor16++;
	  	    	}
	  	    	else
	  	    	{
					LCD_BulkWriteData(*pColor16++);
				}
			}
		}
		else
		{
	  	    for(i=ReadByte>>1;i!=0;i--)
	  	    {
				LCD_BulkWriteData(*pColor16++);
			}
		}
  	}	

  	Q_Free(gBmpColorTableBuf);//�ͷŻ���
  	
	LCD_UnLock();//�ͷ���Ļ��Դ

Gui_Return:	
	if( fp)
		FS_FClose(fp);

	return Ret;
}

//��ȡSpi Flashϵͳbin�ļ�����ͼ
//bin�ļ�ȫ��Ϊɫ�����ݣ�ÿ�����ֽڴ���һ������
//�����ָ�����x,y�ͳ���w,h��Ϣ
GUI_RESULT Gui_DrawImgFlashBin(u32 Page,const GUI_REGION *pRegion)
{
	GUI_RESULT Ret=Gui_True;
	u32 i;
	u32 ReadByte;
	u16 *pColor16;
	u32 sizeByte;
	COLOR_TYPE TransColor=0;
	bool NeedTrans=FALSE;
	
	sizeByte=((pRegion->w*pRegion->h)<<1);

	LCD_Lock();//��ռ��Ļ
	if(Gui_SetRegion(pRegion->x,pRegion->y,pRegion->w,pRegion->h)==Gui_Out_Of_Range)
	{
		LCD_UnLock();
		return Gui_Out_Of_Range;
	}

	if(pRegion->Color!=FatColor(NO_TRANS))//�ж��Ƿ���Ҫ͸��ɫ
	{
#if USE_32BIT_COLOR_TYPE
		TransColor=ChColor(pRegion->Color);
#else
		TransColor=pRegion->Color;//��Ҫת��͸��ɫ��ģʽΪBGR
#endif
		NeedTrans=TRUE;
	}

	gBmpColorTableBuf=Q_Mallco(GUI_BMP_COLOR_TABLE_BUF_SIZE);//���仺��
	
	LCD_BlukWriteDataStart();
	
	while(1)
	{			
		if(sizeByte==0)
		{
			break;
		}
		else if(sizeByte>SPI_FLASH_PAGE_SIZE)
		{
			ReadByte=SPI_FLASH_PAGE_SIZE;
			sizeByte-=SPI_FLASH_PAGE_SIZE;
		}
		else
		{
			ReadByte=sizeByte;
			sizeByte=0;
		}

		Gui_ReadSpiFlash(Page++*SPI_FLASH_PAGE_SIZE,(void *)gBmpColorTableBuf,ReadByte);

  	    pColor16=(void *)gBmpColorTableBuf;

		if(NeedTrans)
		{
	  	    for(i=ReadByte>>1;i>0;i--)		
	  	   	{
	  	   		if(*pColor16==TransColor)
	  	   		{
	  	   			LCD_AddrInc();
					pColor16++;
	  	   		}
	  	   		else
	  	   		{
					LCD_BulkWriteData(*pColor16++);
				}
			}
		}
		else
		{
	  	    for(i=ReadByte>>1;i>0;i--)		
				LCD_BulkWriteData(*pColor16++);
		}
  	}

  	Q_Free(gBmpColorTableBuf);
  	
	LCD_UnLock();//�ͷ���Ļ��Դ
	
	return Ret;
}

//��һ�������ʽ��ͼƬ��䵽ָ������
//pImageBufͨ��Image2Lcd���߻�ȡ,�����Gui_DrawImageArray����
//pRegion��ָ��x y w h������ָ���������
//width��hightָ��ͼ����
GUI_RESULT Gui_FillImgArray(const u8 * pImageBuf,u16 width,u16 hight,const GUI_REGION *pRegion)
{
	u16 *pColor16;
	u16 FillNumH;//������Ҫ�ظ��ĸ���
	u16 FillNumV;//������Ҫ�ظ��ĸ���
	u16 LastOneH;//���һ�εĿ��
	u16 LastOneV;//���һ�εĸ߶�
	u32 i,j,n,m;
	u16 Hstart,Vstart;
	u16 H=0,V;
	bool NeedTrans=FALSE;
	COLOR_TYPE TransColor=0;

	LCD_Lock();//��ռ��Ļ
	
	if(Gui_SetRegion(pRegion->x,pRegion->y,pRegion->w,pRegion->h)==Gui_Out_Of_Range)
	{
		LCD_UnLock();
		return Gui_Out_Of_Range;
	}
	
	if(pRegion->Color!=FatColor(NO_TRANS))//�ж��Ƿ���Ҫ͸��ɫ
	{
#if USE_32BIT_COLOR_TYPE
		TransColor=ChColor(pRegion->Color);//ת����ɫ	
#else
		TransColor=pRegion->Color;
#endif
		NeedTrans=TRUE;
	}
	
	if(gLandScapeMode)	
	{
		Hstart=pRegion->y;
		Vstart=pRegion->x;	
	}
	else
	{
		Hstart=pRegion->x;
		Vstart=pRegion->y;	
	}
	
	FillNumH=pRegion->w/width;
	FillNumV=pRegion->h/hight;
	LastOneH=pRegion->w%width;
	LastOneV=pRegion->h%hight;

	if(NeedTrans)
	{
		for(i=0,V=Vstart;i<FillNumV;i++)//ÿ��
		{
			for(j=0,H=Hstart;j<FillNumH;j++)//ÿ��
			{
				pColor16=(u16 *)pImageBuf;
				if(gLandScapeMode) Gui_SetRegion(V,H,width,hight);
				else Gui_SetRegion(H,V,width,hight);
				LCD_BlukWriteDataStart();
				for(n=width*hight;n;n--)
				{
					if(*pColor16==TransColor){LCD_AddrInc();pColor16++;}
					else LCD_BulkWriteData(*pColor16++);
				}

				H+=width;
			}

			//ÿ�����һ��
			if(LastOneH)
			{
				pColor16=(u16 *)pImageBuf;
				if(gLandScapeMode) Gui_SetRegion(V,H,LastOneH,hight);
				else Gui_SetRegion(H,V,LastOneH,hight);
				LCD_BlukWriteDataStart();
				for(n=hight;n;n--,pColor16+=(width-LastOneH))
					for(m=LastOneH;m;m--)
					{
						if(*pColor16==TransColor){LCD_AddrInc();pColor16++;}
						else LCD_BulkWriteData(*pColor16++);
					}
			}

			if(gLandScapeMode) V-=hight;
			else V+=hight;
		}

		//���һ��
		if(LastOneV)
		{
			H=Hstart;
			for(j=0;j<FillNumH;j++,H+=width)//���һ�е�ÿ��
			{
				pColor16=(u16 *)pImageBuf;
				if(gLandScapeMode) Gui_SetRegion(V,H,width,LastOneV);
				else Gui_SetRegion(H,V,width,LastOneV);
				LCD_BlukWriteDataStart();
				for(n=width*LastOneV;n;n--)
				{
					if(*pColor16==TransColor){LCD_AddrInc();pColor16++;}
					else LCD_BulkWriteData(*pColor16++);
				}
			}
		}

		//���һ�����һ��
		if(LastOneH&&LastOneV)
		{
			pColor16=(u16 *)pImageBuf;
			if(gLandScapeMode) Gui_SetRegion(V,H,LastOneH,LastOneV);
			else Gui_SetRegion(H,V,LastOneH,LastOneV);
			LCD_BlukWriteDataStart();
			for(n=LastOneV;n;n--,pColor16+=(width-LastOneH))
				for(m=LastOneH;m;m--)
				{
					if(*pColor16==TransColor){LCD_AddrInc();pColor16++;}
					else LCD_BulkWriteData(*pColor16++);
				}
		}
	}
	else //����Ҫ͸��ɫ
	{
		for(i=0,V=Vstart;i<FillNumV;i++)//ÿ��
		{
			for(j=0,H=Hstart;j<FillNumH;j++)//ÿ��
			{
				pColor16=(u16 *)pImageBuf;
				if(gLandScapeMode) Gui_SetRegion(V,H,width,hight);
				else Gui_SetRegion(H,V,width,hight);
				LCD_BlukWriteDataStart();
				for(n=width*hight;n;n--)
					LCD_BulkWriteData(*pColor16++);

				H+=width;
			}

			//ÿ�����һ��
			if(LastOneH)
			{
				pColor16=(u16 *)pImageBuf;
				if(gLandScapeMode) Gui_SetRegion(V,H,LastOneH,hight);
				else Gui_SetRegion(H,V,LastOneH,hight);
				LCD_BlukWriteDataStart();
				for(n=hight;n;n--,pColor16+=(width-LastOneH))
					for(m=LastOneH;m;m--)
						LCD_BulkWriteData(*pColor16++);
			}

			if(gLandScapeMode) V-=hight;
			else V+=hight;
		}

		//���һ��
		if(LastOneV)
		{
			H=Hstart;
			for(j=0;j<FillNumH;j++,H+=width)//���һ�е�ÿ��
			{
				pColor16=(u16 *)pImageBuf;
				if(gLandScapeMode) Gui_SetRegion(V,H,width,LastOneV);
				else Gui_SetRegion(H,V,width,LastOneV);
				LCD_BlukWriteDataStart();
				for(n=width*LastOneV;n;n--)
					LCD_BulkWriteData(*pColor16++);
			}
		}

		//���һ�����һ��
		if(LastOneH&&LastOneV)
		{
			pColor16=(u16 *)pImageBuf;
			if(gLandScapeMode) Gui_SetRegion(V,H,LastOneH,LastOneV);
			else Gui_SetRegion(H,V,LastOneH,LastOneV);
			LCD_BlukWriteDataStart();
			for(n=LastOneV;n;n--,pColor16+=(width-LastOneH))
				for(m=LastOneH;m;m--)
					LCD_BulkWriteData(*pColor16++);
		}
	}
	
	LCD_UnLock();//�ͷ���Ļ��Դ

	return Gui_True;
}

//��һ�������ʽ��ͼƬ������䵽ָ������
//pImageBufͨ��Image2Lcd���߻�ȡ,�����Gui_DrawImageArray����
//pRegion��ָ��x y w h����������ͼƬ�����
//widthָ���������Ŀ��
GUI_RESULT Gui_FillImgArray_H(const u8 * pImageBuf,u16 width,const GUI_REGION *pRegion)
{
	u16 *pColor16;
	u16 FillNum=width/pRegion->w;//��Ҫ�ظ��ĸ���
	u8 LastOne=width%pRegion->w*pRegion->h;//���һ�εĿ��
	u32 i,j;

	LCD_Lock();//��ռ��Ļ
	if(Gui_SetRegion(pRegion->x,pRegion->y,pRegion->w,pRegion->h)==Gui_Out_Of_Range)
	{
		LCD_UnLock();
		return Gui_Out_Of_Range;
	}
	
	LCD_SetRegion(pRegion->x,pRegion->y,
		pRegion->x+width-1,pRegion->y+pRegion->h-1,TRUE);//�������ģʽ
	LCD_SetAddrIncMode(NormalAddrIncMode);

	LCD_BlukWriteDataStart();
	for(j=0;j<FillNum;j++)
	{
		pColor16=(u16 *)pImageBuf;
	  	for(i=(pRegion->w*pRegion->h);i!=0;i--)
	  	{
			LCD_BulkWriteData(*pColor16++);
		}
	}

	pColor16=(u16 *)pImageBuf;
	for(i=0;i<LastOne;i++)
	{
		LCD_BulkWriteData(*pColor16++);
	}

	Gui_SetRegion(0,0,LCD_WIDTH-1,LCD_HIGHT-1);
	LCD_UnLock();//�ͷ���Ļ��Դ

	return Gui_True;
}

//��һ�������ʽ��ͼƬ������䵽ָ������
//pImageBufͨ��Image2Lcd���߻�ȡ,�����Gui_DrawImageArray����
//pRegion��ָ��x y w h����������ͼƬ��Ϣ�����
//hightָ���������ĸ߶�
GUI_RESULT Gui_FillImgArray_V(const u8 * pImageBuf,u16 hight,const GUI_REGION *pRegion)
{
	u16 *pColor16;
	u16 FillNum=hight/pRegion->h;//��Ҫ�ظ��ĸ���
	u8 LastOne=hight%pRegion->h*pRegion->w;//���һ�εĿ��
	u32 i,j;

	LCD_Lock();//��ռ��Ļ
	if(Gui_SetRegion(pRegion->x,pRegion->y,pRegion->w,pRegion->h)==Gui_Out_Of_Range)
	{
		LCD_UnLock();
		return Gui_Out_Of_Range;
	}
	
	LCD_SetRegion(pRegion->x,pRegion->y,
		pRegion->x+pRegion->w-1,pRegion->y+hight-1,FALSE);
	LCD_SetAddrIncMode(NormalAddrIncMode);

	LCD_BlukWriteDataStart();
	for(j=0;j<FillNum;j++)
	{
		pColor16=(u16 *)pImageBuf;
	  	for(i=(pRegion->w*pRegion->h);i!=0;i--)
	  	{
			LCD_BulkWriteData(*pColor16++);
		}
	}

	pColor16=(u16 *)pImageBuf;
	for(i=0;i<LastOne;i++)
	{
		LCD_BulkWriteData(*pColor16++);
	}

	LCD_SetRegion(0,0,LCD_WIDTH-1,LCD_HIGHT-1,gLandScapeMode);
	LCD_UnLock();//�ͷ���Ļ��Դ

	return Gui_True;
}

//�������ȱ�����ͼƬ��ȵ�������
//pRegion->xָ�����x����
//pRegion->yָ�����y����
//pRegion->wָ�������
GUI_RESULT Gui_FillBmp_H(const u8 *pBmpPath,const GUI_REGION *pRegion)
{
	GUI_REGION BmpRegion;
	u16 Times;
	u16 BmpW;

	BmpRegion.x=pRegion->x;
	BmpRegion.y=pRegion->y;
	BmpRegion.Color=pRegion->Color;
	Gui_ReadBmpInfo(pBmpPath,&BmpW,NULL);
	Times=BmpRegion.w/BmpW;//������ܱ������������ּ�϶

	for(;Times;Times--,BmpRegion.x+=BmpW)
	{
		Gui_Draw24Bmp(pBmpPath,&BmpRegion);
	}

	return Gui_True;
}

//������߶ȱ�����ͼƬ�߶ȵ�������
//pRegion->xָ�����x����
//pRegion->yָ�����y����
//pRegion->hָ�����߶�
GUI_RESULT Gui_FillBmp_V(const u8 *pBmpPath,const GUI_REGION *pRegion)
{
	GUI_REGION BmpRegion;
	u8 Times;
	u16 BmpH;

	BmpRegion.x=pRegion->x;
	BmpRegion.y=pRegion->y;
	BmpRegion.Color=pRegion->Color;
	Gui_ReadBmpInfo(pBmpPath,NULL,&BmpH);
	Times=BmpRegion.h/BmpH;//������ܱ������������ּ�϶

	for(;Times;Times--,BmpRegion.y+=BmpH)
	{
		Gui_Draw24Bmp(pBmpPath,&BmpRegion);
	}

	return Gui_True;
}

//��ָ��ǰ׺���ɵ�����ͼƬ���������һ������
//pRegion->xָ�����x����
//pRegion->yָ�����y����
//pRegion->wָ�������
GUI_RESULT Gui_DrawFrame_H(const u8 *pFramePathPrefix,const GUI_REGION *pRegion)
{
	u8 PathBuf[64]="";//·����󲻵ó���64���ַ�
	//u8 LeftBmpName[]="L.bmp";
	//u8 MiddleBmpName[]="M.bmp";
	//u8 RightBmpName[]="R.bmp";
	GUI_REGION BmpRegion;
	u16 Lwidth,Rwidth;
	
	if(strlen((void *)pFramePathPrefix)>48)
	{
		Gui_Debug("Frame image path is too long!\n\r");
		return Gui_False;
	}

	BmpRegion.x=pRegion->x;
	BmpRegion.y=pRegion->y;
	BmpRegion.Color=pRegion->Color;
	
	//�Ȼ������ͼ
	strcpy((void *)PathBuf,(void *)pFramePathPrefix);
	strcat((void *)PathBuf,"L.bmp");
	Gui_ReadBmpInfo((void *)PathBuf,&Lwidth,NULL);
	Gui_Draw24Bmp((void *)PathBuf,&BmpRegion);
	
	//�ٸ�����ͼ��С����ͼ
	strcpy((void *)PathBuf,(void *)pFramePathPrefix);
	strcat((void *)PathBuf,"R.bmp");
	Gui_ReadBmpInfo((void *)PathBuf,&Rwidth,NULL);	
	BmpRegion.x+=pRegion->w-Rwidth;
	Gui_Draw24Bmp((void *)PathBuf,&BmpRegion);
	
	//�������м�
	strcpy((void *)PathBuf,(void *)pFramePathPrefix);
	strcat((void *)PathBuf,"M.bmp");
	BmpRegion.x=pRegion->x+Lwidth;
	BmpRegion.w=pRegion->w-Lwidth-Rwidth;
	Gui_FillBmp_H((void *)PathBuf,&BmpRegion);
	
	return Gui_True;
}

//��ָ��ǰ׺���ɵ�����ͼƬ���������һ������
//pRegion->xָ�����x����
//pRegion->yָ�����y����
//pRegion->hָ�����߶�
GUI_RESULT Gui_DrawFrame_V(const u8 *pFramePathPrefix,const GUI_REGION *pRegion)
{
	u8 PathBuf[64]="";//·����󲻵ó���64���ַ�
	//u8 LeftBmpName[]="T.bmp";
	//u8 MiddleBmpName[]="M.bmp";
	//u8 RightBmpName[]="B.bmp";
	GUI_REGION BmpRegion;
	u16 Thight,Bhight;
	
	if(strlen((void *)pFramePathPrefix)>48)
	{
		Gui_Debug("Frame image path is too long!\n\r");
		return Gui_False;
	}

	BmpRegion.x=pRegion->x;
	BmpRegion.y=pRegion->y;
	BmpRegion.Color=pRegion->Color;
	
	//�Ȼ������ͼ
	strcpy((void *)PathBuf,(void *)pFramePathPrefix);
	strcat((void *)PathBuf,"T.bmp");
	Gui_ReadBmpInfo((void *)PathBuf,NULL,&Thight);
	Gui_Draw24Bmp((void *)PathBuf,&BmpRegion);

	//�ٸ�����ͼ��С����ͼ
	strcpy((void *)PathBuf,(void *)pFramePathPrefix);
	strcat((void *)PathBuf,"B.bmp");
	Gui_ReadBmpInfo((void *)PathBuf,NULL,&Bhight);	
	BmpRegion.y+=BmpRegion.h-Bhight;
	Gui_Draw24Bmp((void *)PathBuf,&BmpRegion);
	
	//�������м�
	strcpy((void *)PathBuf,(void *)pFramePathPrefix);
	strcat((void *)PathBuf,"M.bmp");
	BmpRegion.y=pRegion->y+Thight;
	BmpRegion.h=pRegion->h-Thight-Bhight;
	Gui_FillBmp_V((void *)PathBuf,&BmpRegion);
	
	return Gui_True;
}

//���ߺ�����ʹ��Bresenham �����㷨
GUI_RESULT Gui_DrawLine(u16 x0, u16 y0,u16 x1, u16 y1,COLOR_TYPE Color)   
{
s32 dx,             // difference in x's
    dy,             // difference in y's
    dx2,            // dx,dy * 2
    dy2, 
    x_inc,          // amount in pixel space to move during drawing
    y_inc,          // amount in pixel space to move during drawing
    error,          // the discriminant i.e. error i.e. decision variable
    index;          // used for looping	
bool LandMode=Gui_GetLandScapeMode();

#if USE_32BIT_COLOR_TYPE
	Color=ChColor(Color);//ת����ɫ	
#endif

	LCD_Lock();
	Gui_SetLandScapeMode(FALSE);
	Gui_SetRegion(0,0,LCD_WIDTH,LCD_HIGHT);
	
	dx = x1-x0;//����x����
	dy = y1-y0;//����y����

	if (dx>=0)
	{
		x_inc = 1;
	}
	else
	{
		x_inc = -1;
		dx    = -dx;  
	} 
	
	if (dy>=0)
	{
		y_inc = 1;
	} 
	else
	{
		y_inc = -1;
		dy    = -dy; 
	} 

	dx2 = dx << 1;
	dy2 = dy << 1;

	if (dx > dy)//x�������y���룬��ôÿ��x����ֻ��һ���㣬ÿ��y���������ɸ���
	{//���ߵĵ�������x���룬��x���������
		// initialize error term
		error = dy2 - dx; 

		// draw the line
		for (index=0; index <= dx; index++)//Ҫ���ĵ������ᳬ��x����
		{
			//����
			LCD_DrawPoint(x0,y0,Color);
			
			// test if error has overflowed
			if (error >= 0) //�Ƿ���Ҫ����y����ֵ
			{
				error-=dx2;

				// move to next line
				y0+=y_inc;//����y����ֵ
			} // end if error overflowed

			// adjust the error term
			error+=dy2;

			// move to the next pixel
			x0+=x_inc;//x����ֵÿ�λ���󶼵���1
		} // end for
	} // end if |slope| <= 1
	else//y�����x�ᣬ��ÿ��y����ֻ��һ���㣬x�����ɸ���
	{//��y��Ϊ��������
		// initialize error term
		error = dx2 - dy; 

		// draw the line
		for (index=0; index <= dy; index++)
		{
			// set the pixel
			LCD_DrawPoint(x0,y0,Color);

			// test if error overflowed
			if (error >= 0)
			{
				error-=dy2;

				// move to next line
				x0+=x_inc;
			} // end if error overflowed

			// adjust the error term
			error+=dx2;

			// move to the next pixel
			y0+=y_inc;
		} // end for
	} // end else |slope| > 1

	Gui_SetLandScapeMode(LandMode);
	LCD_UnLock();
	return Gui_True;
} // end Draw_Line16

//������,ͬGui_DrawLine
//ColorBuf�������ڴ��16λ��ɫ����Ϣ,ÿ������һ��
//���һ���㲻��
GUI_RESULT Gui_DrawColorLine(u16 x0, u16 y0,u16 x1, u16 y1,const u16 *ColorBuf)   
{
s32 dx,             // difference in x's
    dy,             // difference in y's
    dx2,            // dx,dy * 2
    dy2, 
    x_inc,          // amount in pixel space to move during drawing
    y_inc,          // amount in pixel space to move during drawing
    error,          // the discriminant i.e. error i.e. decision variable
    index;          // used for looping	
bool LandMode=Gui_GetLandScapeMode();

	LCD_Lock();
	Gui_SetLandScapeMode(FALSE);
	Gui_SetRegion(0,0,LCD_WIDTH,LCD_HIGHT);
	
	dx = x1-x0;//����x����
	dy = y1-y0;//����y����

	if (dx>=0)
	{
		x_inc = 1;
	}
	else
	{
		x_inc = -1;
		dx    = -dx;  
	} 
	
	if (dy>=0)
	{
		y_inc = 1;
	} 
	else
	{
		y_inc = -1;
		dy    = -dy; 
	} 

	dx2 = dx << 1;
	dy2 = dy << 1;

	if (dx > dy)//x�������y���룬��ôÿ��x����ֻ��һ���㣬ÿ��y���������ɸ���
	{//���ߵĵ�������x���룬��x���������
		// initialize error term
		error = dy2 - dx; 

		// draw the line
		dx--;//���һ���㲻��Ҫ��
		for (index=0; index <= dx; index++)//Ҫ���ĵ������ᳬ��x����
		{
			//����
			LCD_DrawPoint(x0,y0,ColorBuf[index]);
			
			// test if error has overflowed
			if (error >= 0) //�Ƿ���Ҫ����y����ֵ
			{
				error-=dx2;

				// move to next line
				y0+=y_inc;//����y����ֵ
			} // end if error overflowed

			// adjust the error term
			error+=dy2;

			// move to the next pixel
			x0+=x_inc;//x����ֵÿ�λ���󶼵���1
		} // end for
	} // end if |slope| <= 1
	else//y�����x�ᣬ��ÿ��y����ֻ��һ���㣬x�����ɸ���
	{//��y��Ϊ��������
		// initialize error term
		error = dx2 - dy; 

		// draw the line
		dy--;//���һ���㲻��Ҫ��
		for (index=0; index <= dy; index++)
		{
			// set the pixel
			LCD_DrawPoint(x0,y0,ColorBuf[index]);

			// test if error overflowed
			if (error >= 0)
			{
				error-=dy2;

				// move to next line
				x0+=x_inc;
			} // end if error overflowed

			// adjust the error term
			error+=dx2;

			// move to the next pixel
			y0+=y_inc;
		} // end for
	} // end else |slope| > 1

	Gui_SetLandScapeMode(LandMode);
	LCD_UnLock();
	return Gui_True;
}

//����һ���߾��������е��ɫ����Ϣ
//ColorBuf�������ڴ��16λ��ɫ����Ϣ,ÿ������һ��
//���һ���㲻��
GUI_RESULT Gui_SaveColorLine(u16 x0, u16 y0,u16 x1, u16 y1,u16 *ColorBuf)   
{
s32 dx,             // difference in x's
    dy,             // difference in y's
    dx2,            // dx,dy * 2
    dy2, 
    x_inc,          // amount in pixel space to move during drawing
    y_inc,          // amount in pixel space to move during drawing
    error,          // the discriminant i.e. error i.e. decision variable
    index;          // used for looping	
bool LandMode=Gui_GetLandScapeMode();

	LCD_Lock();
	Gui_SetLandScapeMode(FALSE);
	
	dx = x1-x0;//����x����
	dy = y1-y0;//����y����

	if (dx>=0)
	{
		x_inc = 1;
	}
	else
	{
		x_inc = -1;
		dx    = -dx;  
	} 
	
	if (dy>=0)
	{
		y_inc = 1;
	} 
	else
	{
		y_inc = -1;
		dy    = -dy; 
	} 

	dx2 = dx << 1;
	dy2 = dy << 1;

	if (dx > dy)//x�������y���룬��ôÿ��x����ֻ��һ���㣬ÿ��y���������ɸ���
	{//���ߵĵ�������x���룬��x���������
		// initialize error term
		error = dy2 - dx; 

		// draw the line
		dx--;//���һ���㲻��Ҫ��
		for (index=0; index <= dx; index++)//Ҫ���ĵ������ᳬ��x����
		{
			//���
			ColorBuf[index]=Gui_ReadPixel16Bit(x0,y0);
			
			// test if error has overflowed
			if (error >= 0) //�Ƿ���Ҫ����y����ֵ
			{
				error-=dx2;

				// move to next line
				y0+=y_inc;//����y����ֵ
			} // end if error overflowed

			// adjust the error term
			error+=dy2;

			// move to the next pixel
			x0+=x_inc;//x����ֵÿ�λ���󶼵���1
		} // end for
	} // end if |slope| <= 1
	else//y�����x�ᣬ��ÿ��y����ֻ��һ���㣬x�����ɸ���
	{//��y��Ϊ��������
		// initialize error term
		error = dx2 - dy; 

		// draw the line
		dy--;//���һ���㲻��Ҫ��
		for (index=0; index <= dy; index++)
		{
			// ���
			ColorBuf[index]=Gui_ReadPixel16Bit(x0,y0);

			// test if error overflowed
			if (error >= 0)
			{
				error-=dy2;

				// move to next line
				x0+=x_inc;
			} // end if error overflowed

			// adjust the error term
			error+=dx2;

			// move to the next pixel
			y0+=y_inc;
		} // end for
	} // end else |slope| > 1

	Gui_SetLandScapeMode(LandMode);
	LCD_UnLock();
	return Gui_True;
}

//�������ߺ�����ͬGui_DrawLine,gapָ�����
//Gap����ĸ�λָ���߳�
//Gap�θ��ĸ�λָ����϶��
GUI_RESULT Gui_DrawDashed(u16 x0, u16 y0,u16 x1, u16 y1,u8 Gap,COLOR_TYPE Color)   
{
	s32 dx,             // difference in x's
    dy,             // difference in y's
    dx2,            // dx,dy * 2
    dy2, 
    x_inc,          // amount in pixel space to move during drawing
    y_inc,          // amount in pixel space to move during drawing
    error,          // the discriminant i.e. error i.e. decision variable
    index;          // used for looping	
	u8 LineLen=Gap>>4;
	u8 UnitLen=(Gap&0x0f)+LineLen;
bool LandMode=Gui_GetLandScapeMode();

#if USE_32BIT_COLOR_TYPE
    Color=ChColor(Color);
#endif

	LCD_Lock();
	Gui_SetLandScapeMode(FALSE);
	Gui_SetRegion(0,0,LCD_WIDTH,LCD_HIGHT);
	
	dx = x1-x0;//����x����
	dy = y1-y0;//����y����

	if (dx>=0)
	{
		x_inc = 1;
	}
	else
	{
		x_inc = -1;
		dx    = -dx;  
	} 
	
	if (dy>=0)
	{
		y_inc = 1;
	} 
	else
	{
		y_inc = -1;
		dy    = -dy; 
	} 

	dx2 = dx << 1;
	dy2 = dy << 1;

	if (dx > dy)//x�������y���룬��ôÿ��x����ֻ��һ���㣬ÿ��y���������ɸ���
	{//���ߵĵ�������x���룬��x���������
		// initialize error term
		error = dy2 - dx; 

		// draw the line
		for (index=0; index <= dx; index++)//Ҫ���ĵ������ᳬ��x����
		{
			//����
			if((index%UnitLen)<LineLen)
			{
				LCD_DrawPoint(x0,y0,Color);
			}
			
			// test if error has overflowed
			if (error >= 0) //�Ƿ���Ҫ����y����ֵ
			{
				error-=dx2;

				// move to next line
				y0+=y_inc;//����y����ֵ
			} // end if error overflowed

			// adjust the error term
			error+=dy2;

			// move to the next pixel
			x0+=x_inc;//x����ֵÿ�λ���󶼵���1
		} // end for
	} // end if |slope| <= 1
	else//y�����x�ᣬ��ÿ��y����ֻ��һ���㣬x�����ɸ���
	{//��y��Ϊ��������
		// initialize error term
		error = dx2 - dy; 

		// draw the line
		for (index=0; index <= dy; index++)
		{
			//����
			if((index%UnitLen)<LineLen)
			{
				LCD_DrawPoint(x0,y0,Color);
			}

			// test if error overflowed
			if (error >= 0)
			{
				error-=dy2;

				// move to next line
				x0+=x_inc;
			} // end if error overflowed

			// adjust the error term
			error+=dx2;

			// move to the next pixel
			y0+=y_inc;
		} // end for
	} // end else |slope| > 1

	Gui_SetLandScapeMode(LandMode);
	LCD_UnLock();
	return Gui_True;
}

//�˶Գ���
static void __inline Gui_DrawCircle8(u16 xc, u16 yc, u16 x, u16 y,u8 FillMode,COLOR_TYPE Color) 
{

	if(FillMode&(1<<0)) LCD_DrawPoint(xc + x, yc - y,Color);// 1��
	if(FillMode&(1<<1)) LCD_DrawPoint(xc + y, yc - x,Color);// 1��
	if(FillMode&(1<<2)) LCD_DrawPoint(xc + y, yc + x,Color);// 2��
	if(FillMode&(1<<3)) LCD_DrawPoint(xc + x, yc + y,Color);// 2��	
	if(FillMode&(1<<4)) LCD_DrawPoint(xc - x, yc + y,Color);// 3��
	if(FillMode&(1<<5)) LCD_DrawPoint(xc - y, yc + x,Color);// 3��
	if(FillMode&(1<<6)) LCD_DrawPoint(xc - y, yc - x,Color);// 4��
	if(FillMode&(1<<7)) LCD_DrawPoint(xc - x, yc - y,Color);// 4��
}

//Bresenham��Բ�㷨
//pRegion->x,yָ��Բ������
//pRegion->wָ��Բ�뾶
//pRegion->Colorָ�������ɫ
//pRegion->Spaceָ���������,0xff-���8������
//Cir:TRUE��Բ FALSE����
GUI_RESULT Gui_DrawCircle(const GUI_REGION *pRegion,bool Cir) 
{
	int x = 0, y = pRegion->w, yi, d= 3 - 2 * pRegion->w;
	COLOR_TYPE Color;
	bool LandMode=Gui_GetLandScapeMode();
	
	if (pRegion->x + pRegion->w < 0 || pRegion->x - pRegion->w >= LCD_WIDTH || 
		pRegion->y + pRegion->w < 0 || pRegion->y - pRegion->w >= LCD_HIGHT) 
		return Gui_Out_Of_Range;// ���Բ��ͼƬ�ɼ������⣬ֱ���˳�

#if USE_32BIT_COLOR_TYPE
	Color=ChColor(pRegion->Color);//ת����ɫ	
#else
	Color=pRegion->Color;
#endif

	LCD_Lock();
	Gui_SetLandScapeMode(FALSE);
	Gui_SetRegion(0,0,LCD_WIDTH,LCD_HIGHT);
	
	if (Cir) // �����䣨��ʵ��Բ��
	{
		while (x <= y) 
		{
			for (yi = x; yi <= y; yi ++)
				Gui_DrawCircle8( pRegion->x, pRegion->y, x, yi,pRegion->Space,Color);

			if (d < 0) 
			{
				d = d + 4 * x + 6;
			} 
			else 
			{
				d = d + 4 * (x - y) + 10;
				y --;
			}
			x++;
		}
	} 
	else // �������䣨������Բ��
	{
		while (x <= y) 
		{
			Gui_DrawCircle8( pRegion->x, pRegion->y, x, y,pRegion->Space,Color);

			if (d < 0) 
			{
				d = d + 4 * x + 6;
			} 
			else 
			{
				d = d + 4 * (x - y) + 10;
				y --;
			}
			x ++;
		}
	}

	Gui_SetLandScapeMode(LandMode);
	LCD_UnLock();
	return Gui_True;
}


//��ָ���������ɫ
GUI_RESULT Gui_ReadRegion24Bit(u8 *Buf,const GUI_REGION *pRegion)
{
	u32 i,num=pRegion->w*pRegion->h*3;
	u16 Color;

	LCD_Lock();
	if(Gui_SetRegion(pRegion->x,pRegion->y,pRegion->w,pRegion->h)==Gui_Out_Of_Range)
	{
		LCD_UnLock();
		return Gui_Out_Of_Range;
	}

	LCD_BulkReadDataStart();
	
	for(i=0;i<num;)
	{
		Color=LCD_BulkReadData();
		Buf[i++]=(u8)((Color&0x1f)<<3);
		Buf[i++]=(u8)((Color&0x7e0)>>3);
		Buf[i++]=(u8)((Color&0xf800)>>8);
	}

	LCD_UnLock();
	return Gui_True;
}

//��ָ���������ɫ
GUI_RESULT Gui_ReadRegion16Bit(u16 *Buf,const GUI_REGION *pRegion)
{
	u32 i,num=pRegion->w*pRegion->h;

	LCD_Lock();
	if(Gui_SetRegion(pRegion->x,pRegion->y,pRegion->w,pRegion->h)==Gui_Out_Of_Range)
	{
		LCD_UnLock();
		return Gui_Out_Of_Range;
	}

	LCD_BulkReadDataStart();
	
	for(i=0;i<num;i++)
	{
		Buf[i]=LCD_BulkReadData();
	}

	LCD_UnLock();
	return Gui_True;
}

//��һ�����ɫ��
void Gui_WritePixel(u16 x,u16 y,COLOR_TYPE Color)
{
	if((x>=LCD_WIDTH)||(y>=LCD_HIGHT)) return;
	
#if USE_32BIT_COLOR_TYPE
	Color=ChColor(Color);//ת����ɫ	
#endif

	LCD_Lock();
	LCD_DrawPoint(x,y,Color);
	LCD_UnLock();
}

//��һ�����ɫ����Ϣ
u16 Gui_ReadPixel16Bit(u16 x,u16 y)
{
	u16 Color;

	if((x>=LCD_WIDTH)||(y>=LCD_HIGHT)) return 0;
	
	LCD_Lock();
	Color=LCD_ReadPoint(x,y);
	LCD_UnLock();
	
	return Color;
}


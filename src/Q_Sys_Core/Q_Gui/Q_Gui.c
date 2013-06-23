/*作者声明:
一般情况下，我不喜欢用别人现成的gui，因为通用的gui一般都不会对
特定的硬件做优化。所以我自己写了这个gui，是一个很小的，很简单
的gui，以后会逐步完善，大家也可以多交流。

stm32的芯带320x240的屏幕其实是很吃力的，主要是由于图片数据必须放在
ROM外部,读数据会让整个显示变慢,所以大家可以看到在显示图
片的时候会有明显的刷屏。为了较少这种刷屏，大家可以做如下优化:
1.用填充函数代替渐变的bmp背景
2.背景不用bmp
3.将bmp先解码，将图片对应的色彩信息转换成一个数组(Image2Lcd工具)，
存放到文件系统或spi flash中，然后编写对应的描绘函数。

每个函数内部最开始必须调用Gui_SetRegion设置范围
*/
#include "System.h"
#include "Drivers.h"
#include "Lcd.h"

#define Gui_Debug Debug

extern OS_MutexHandler gLCD_Mutex;			//lcd操作互斥量
#define LCD_Lock() //OS_SemaphoreTake(gLCD_Mutex,OS_MAX_DELAY)
#define LCD_UnLock() //OS_SemaphoreGive(gLCD_Mutex)

//static FIL gBmpObj; 
#if GUI_USE_FS_FONT_LIB == 1
static FS_FILE *gpFontObj;
#endif
static BMP_INFO gBmpInfo;
//__align(4) static u8 gBmpColorTableBuf[GUI_BMP_COLOR_TABLE_BUF_SIZE];//画图缓存
static u8 * gBmpColorTableBuf;//画图缓存改用指针方式从heap里面拿内存
static bool gLandScapeMode=FALSE;//true 横屏

#define LandScapeAddrIncMode	 xDec_yInc	//横屏模式下的地址自增模式
#define NormalAddrIncMode xInc_yInc//正常模式下的地址自增模式

//将16位RGB(565)色彩换算成16位BGR(565)色彩
#define RGB2BGR(Color16)	(((Color16>>11)&0x001f)|(Color16&0x07e0)|((Color16<<11)&0xf800))

//用于获取spi flash里面的字库
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

//用于获取spi里面的图片内容
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

//设置背光
//Value范围0-100
void Gui_SetBgLight(u8 Value)
{
	if(Value>100) return;
	LCD_Light_Set(Value);
}

//读取背光亮度
//返回值范围0-100
u8 Gui_GetBgLightVal(void)
{
	return LCD_Light_State();
}

//LandScapeMode:ture 横屏,false 正常
//注意换了横屏之后,思维也要变过来
//xy坐标轴依然不变,但是w和h是要对掉的
//画图和写字都是给相对区域坐上点坐标
GUI_RESULT Gui_SetLandScapeMode(bool LandScape)
{
	if(LandScape==gLandScapeMode) return Gui_True;
	
	LCD_Lock();
	if(LandScape)
	{	//横屏
		gLandScapeMode=TRUE;
		LCD_SetAddrIncMode(LandScapeAddrIncMode);
	}
	else
	{	//正常
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

//设置显示区域,设置横屏模式,检查坐标范围是否正确
//不管是横屏还是正常模式,x_start,y_start均为相对区域左上角的坐标
//比如,如果我想画一个区域0,0-100,100
//正常模式下,x_start,y_start=0,0
//横屏模式下,x_start,y_start=100,0
//w和h都是101
//只能被Gui.c内部的函数使用,因为没有带互斥
static GUI_RESULT Gui_SetRegion(u16 x_start,u16 y_start,u16 w,u16 h)
{
	u16 x_end;
	u16 y_end;

	if(gLandScapeMode)
	{	//横屏模式	
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
	{	//正常模式
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

//用单色填充一个区域
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
	Color=ChColor(pRegion->Color);//转换颜色	
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

//用指定颜色填充整屏，如Gui_FillScreen(FatColor(0x000000))
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

//用单色填充PixelBuf中置1的位对应的像素点
//PixelBuf每一个bit对应一个像素的填充
//因此一个字节对应8个点，所以要注意字节对齐
//pRegion->x ,y指定图像起点
//pRegion->w ,h指定图像宽高
GUI_RESULT Gui_PixelFill(const u8 *PixelBuf,const GUI_REGION *pRegion)
{
	u8 RowByte=((pRegion->w-1)>>3);//每行占字节数-1
	u8 LastNull=8-(pRegion->w-(RowByte<<3));//最后一个字节不需要显示的数据个数
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
	Color=ChColor(pRegion->Color);//转换颜色	
#else
	Color=pRegion->Color;
#endif

	LCD_BlukWriteDataStart();
	
	for (Row = 0; Row < pRegion->h; Row++)//行
	{
		for(i=0;i<RowByte;i++,Byte++)//前面对齐的字节
		{
			for(Bit=7;Bit>=0;Bit--)//读取当前字节
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

		for(Bit=7;Bit>=LastNull;Bit--) //最后一个非对齐字节
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

//从存储体中读取字库显示汉字
//返回实际读取的byte
//注意region.space和region.row_space用于控制字距,且必须设置值!!!
static u16 Gui_DrawFont_GBK16(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//计算字符占用位置，按像素表示
	u32 FontLocal;	//字库偏移量
	u8 CharBuf[GUI_GBK16_FONT_BUF_SIZE];
	u8 w,h,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	COLOR_TYPE Color;

#if USE_32BIT_COLOR_TYPE
	Color=ChColor(pRegion->Color);//转换颜色	
#else
	Color=pRegion->Color;
#endif

	LCD_Lock();
	
	while((Offset=*pStr++)!=0) //开始读取字符串
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//显示汉字
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//越底界检查
			if((RowLen+GUI_GBK16_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			FontLocal=((Offset-GUI_GBK_FONT_OFFSET_H)*GUI_GBK_FONT_PAGENUM+
				((*pStr)-GUI_GBK_FONT_OFFSET_L))<<5;	//获取偏移量,<<5= *GUI_GBK16_FONT_BUF_SIZE
			if((*pStr++) > GUI_GBK_FONT_OFFSET_P) FontLocal-=GUI_GBK16_FONT_BUF_SIZE; //除去字库中0x7f这条线
			
			//移到字库对应位置读取一个字的点阵数据,32个字节
			Gui_ReadFontLib(GUI_GBK16_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_GBK16_FONT_BUF_SIZE);

			if(Gui_GetLandScapeMode())//横屏
				Gui_SetRegion(pRegion->x-RowLen,pRegion->y+ColLen,GUI_GBK16_CHAR_WIDTH,GUI_GBK16_FONT_HEIGHT);
			else//正常
				Gui_SetRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_GBK16_CHAR_WIDTH,GUI_GBK16_FONT_HEIGHT);

			LCD_BlukWriteDataStart();
			for(h=0;h<GUI_GBK16_FONT_BUF_SIZE;h++)//显示一个汉字共32个字节的点阵信息
			{
				for(w=8;w!=0;w--)//读取当前字节
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
			//越边界检查
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK16_CHAR_WIDTH;
			else w=GUI_GBK16_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//如果超过预设宽度
			{				
				ColLen=0;
				RowLen+=(GUI_GBK16_FONT_HEIGHT+RowSpace);
			}

			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//显示符号
		{
			//Debug("C:%c ",*(pStr-1));
			//越底界检查
			if((RowLen+GUI_GBK16_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//检测符号类型
			{
				case 0x0A:	//换行回车的情况
				{
					ColLen=0;
					RowLen+=(GUI_GBK16_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //普通字符的情况
				{
					FontLocal=GUI_GBK16_FONT_END+Offset*GUI_GBK16_FONT_BUF_SIZE;//获取字库中ASCii的偏移量
					
					//移到字库对应位置读取一个字的点阵数据,32个字节
					Gui_ReadFontLib(GUI_GBK16_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_GBK16_FONT_BUF_SIZE);
					
					if(Gui_GetLandScapeMode())//横屏
						Gui_SetRegion(pRegion->x-RowLen,pRegion->y+ColLen,GUI_GBK16_ASCII_WIDTH,GUI_GBK16_FONT_HEIGHT);
					else//正常
						Gui_SetRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_GBK16_ASCII_WIDTH,GUI_GBK16_FONT_HEIGHT);

					LCD_BlukWriteDataStart();
					for(h=0;h<GUI_GBK16_FONT_BUF_SIZE;h+=2)//显示一个ascii共16个字节的点阵信息
					{
						for(w=GUI_GBK16_ASCII_WIDTH;w!=0;w--)//读取当前字节
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
					//越边界检查
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK16_CHAR_WIDTH;
					else w=GUI_GBK16_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//如果超过预设宽度
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

//返回实际区域需要的字符串byte数
//返回值实际就是Gui_DrawFont_GBK16的返回值，只不过不显示实际内容
static u16 Gui_DrawFont_GBK16_Num(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//计算字符占用位置，按像素表示
	u8 w,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	
	while((Offset=*pStr++)!=0) //开始读取字符串
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//显示汉字
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//越底界检查
			if((RowLen+GUI_GBK16_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			pStr++;
			
			ColLen+=(GUI_GBK16_CHAR_WIDTH+Space);
			//越边界检查
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK16_CHAR_WIDTH;
			else w=GUI_GBK16_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//如果超过预设宽度
			{				
				ColLen=0;
				RowLen+=(GUI_GBK16_FONT_HEIGHT+RowSpace);
			}
			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//显示符号
		{
			//Debug("C:%c ",*(pStr-1));
			//越底界检查
			if((RowLen+GUI_GBK16_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//检测符号类型
			{
				case 0x0A:	//换行回车的情况
				{
					ColLen=0;
					RowLen+=(GUI_GBK16_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //普通字符的情况
				{					
					ColLen+=(GUI_GBK16_ASCII_WIDTH+Space);
					//越边界检查
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK16_CHAR_WIDTH;
					else w=GUI_GBK16_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//如果超过预设宽度
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

//从存储体中读取字库显示汉字
//返回实际读取的byte
//注意region.space和region.row_space用于控制字距,且必须设置值!!!
static u16 Gui_DrawFont_GBK12(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//计算字符占用位置，按像素表示
	u32 FontLocal;	//字库偏移量
	u8 CharBuf[GUI_GBK12_FONT_BUF_SIZE];
	u8 w,h,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	COLOR_TYPE Color;

#if USE_32BIT_COLOR_TYPE
	Color=ChColor(pRegion->Color);//转换颜色	
#else
	Color=pRegion->Color;
#endif

	LCD_Lock();
	
	while((Offset=*pStr++)!=0) //开始读取字符串
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//显示汉字
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//越底界检查
			if((RowLen+GUI_GBK12_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			FontLocal=((Offset-GUI_GBK_FONT_OFFSET_H)*GUI_GBK_FONT_PAGENUM+
				((*pStr)-GUI_GBK_FONT_OFFSET_L))*GUI_GBK12_FONT_BUF_SIZE;	//从第一个字节得到偏移量
			if((*pStr++) > GUI_GBK_FONT_OFFSET_P) FontLocal-=GUI_GBK12_FONT_BUF_SIZE; //除去字库中0x7f这条线
			
			//移到字库对应位置读取一个字的点阵数据,24个字节
			Gui_ReadFontLib(GUI_GBK12_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_GBK12_FONT_BUF_SIZE);

			if(Gui_GetLandScapeMode())//横屏
				Gui_SetRegion(pRegion->x-RowLen,pRegion->y+ColLen,GUI_GBK12_CHAR_WIDTH,GUI_GBK12_FONT_HEIGHT);
			else//正常
				Gui_SetRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_GBK12_CHAR_WIDTH,GUI_GBK12_FONT_HEIGHT);

			LCD_BlukWriteDataStart();
			for(h=0;h<GUI_GBK12_FONT_BUF_SIZE;h++)//显示一个汉字共24个字节的点阵信息
			{
				for(w=8;w!=0;w--)//读取当前字节
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
			//越边界检查
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK12_CHAR_WIDTH;
			else w=GUI_GBK12_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//如果超过预设宽度
			{				
				ColLen=0;
				RowLen+=(GUI_GBK12_FONT_HEIGHT+RowSpace);
			}

			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//显示符号
		{
			//Debug("C:%c ",*(pStr-1));
			//越底界检查
			if((RowLen+GUI_GBK12_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//检测符号类型
			{
				case 0x0A:	//换行回车的情况
				{
					ColLen=0;
					RowLen+=(GUI_GBK12_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //普通字符的情况
				{
					FontLocal=GUI_GBK12_FONT_END+Offset*GUI_GBK12_FONT_BUF_SIZE;//获取字库中ASCii的偏移量
					
					//移到字库对应位置读取一个字的点阵数据,24个字节
					Gui_ReadFontLib(GUI_GBK12_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_GBK12_FONT_BUF_SIZE);
					
					if(Gui_GetLandScapeMode())//横屏
						Gui_SetRegion(pRegion->x-RowLen,pRegion->y+ColLen,GUI_GBK12_ASCII_WIDTH,GUI_GBK12_FONT_HEIGHT);
					else//正常
						Gui_SetRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_GBK12_ASCII_WIDTH,GUI_GBK12_FONT_HEIGHT);

					LCD_BlukWriteDataStart();
					for(h=0;h<GUI_GBK12_FONT_BUF_SIZE;h+=2)//显示一个ascii共24个字节的点阵信息
					{
						for(w=GUI_GBK12_ASCII_WIDTH;w!=0;w--)//读取当前字节
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
					//越边界检查
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK12_CHAR_WIDTH;
					else w=GUI_GBK12_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//如果超过预设宽度
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

//返回实际区域需要的字符串byte数
//返回值实际就是Gui_DrawFont_GBK12的返回值，只不过不显示实际内容
static u16 Gui_DrawFont_GBK12_Num(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//计算字符占用位置，按像素表示
	u8 w,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	
	while((Offset=*pStr++)!=0) //开始读取字符串
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//显示汉字
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//越底界检查
			if((RowLen+GUI_GBK12_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			pStr++;
			
			ColLen+=(GUI_GBK12_CHAR_WIDTH+Space);
			//越边界检查
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK12_CHAR_WIDTH;
			else w=GUI_GBK12_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//如果超过预设宽度
			{				
				ColLen=0;
				RowLen+=(GUI_GBK12_FONT_HEIGHT+RowSpace);
			}
			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//显示符号
		{
			//Debug("C:%c ",*(pStr-1));
			//越底界检查
			if((RowLen+GUI_GBK12_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//检测符号类型
			{
				case 0x0A:	//换行回车的情况
				{
					ColLen=0;
					RowLen+=(GUI_GBK12_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //普通字符的情况
				{					
					ColLen+=(GUI_GBK12_ASCII_WIDTH+Space);
					//越边界检查
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK12_CHAR_WIDTH;
					else w=GUI_GBK12_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//如果超过预设宽度
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

//从存储体中读取字库显示汉字
//返回实际读取的byte
//注意region.space和region.row_space用于控制字距,且必须设置值!!!
static u16 Gui_DrawFont_ASC14B(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//计算字符占用位置，按像素表示
	u32 FontLocal;	//字库偏移量
	u8 CharBuf[GUI_ASC14B_FONT_BUF_SIZE];
	u8 w,h,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	COLOR_TYPE Color;

#if USE_32BIT_COLOR_TYPE
	Color=ChColor(pRegion->Color);//转换颜色	
#else
	Color=pRegion->Color;
#endif

	LCD_Lock();
	while((Offset=*pStr++)!=0) //开始读取字符串
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//显示汉字
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//越底界检查
			if((RowLen+GUI_ASC14B_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}

#if 0 //此字库不支持汉字			
			FontLocal=((Offset-GUI_GBK_FONT_OFFSET_H)*GUI_GBK_FONT_PAGENUM+
				((*pStr)-GUI_GBK_FONT_OFFSET_L))*GUI_ASC14B_FONT_BUF_SIZE;	//从第一个字节得到偏移量
			if((*pStr++) > GUI_GBK_FONT_OFFSET_P) FontLocal-=GUI_ASC14B_FONT_BUF_SIZE; //除去字库中0x7f这条线
			
			//移到字库对应位置读取一个字的点阵数据,24个字节
			Gui_ReadFontLib(GUI_ASC14B_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_ASC14B_FONT_BUF_SIZE);
#else
			pStr++;
#endif

			if(Gui_GetLandScapeMode())//横屏
				Gui_SetRegion(pRegion->x-RowLen,pRegion->y+ColLen,GUI_ASC14B_CHAR_WIDTH,GUI_ASC14B_FONT_HEIGHT);
			else//正常
				Gui_SetRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_ASC14B_CHAR_WIDTH,GUI_ASC14B_FONT_HEIGHT);

#if 1		//由于不支持汉字，显示填充块
			LCD_BlukWriteDataStart();
			for(h=0;h<GUI_ASC14B_FONT_BUF_SIZE;h+=2)//显示一个汉字共28个字节的点阵信息
			{
				for(w=GUI_ASC14B_CHAR_WIDTH;w!=0;w--)//读取当前字节
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
			//越边界检查
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_ASC14B_CHAR_WIDTH;
			else w=GUI_ASC14B_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//如果超过预设宽度
			{				
				ColLen=0;
				RowLen+=(GUI_ASC14B_FONT_HEIGHT+RowSpace);
			}

			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//显示符号
		{
			//Debug("C:%c ",*(pStr-1));
			//越底界检查
			if((RowLen+GUI_ASC14B_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//检测符号类型
			{
				case 0x0A:	//换行回车的情况
				{
					ColLen=0;
					RowLen+=(GUI_ASC14B_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //普通字符的情况
				{
					FontLocal=Offset*GUI_ASC14B_FONT_BUF_SIZE;//获取字库中ASCii的偏移量
					
					//移到字库对应位置读取一个字的点阵数据,28个字节
					Gui_ReadFontLib(GUI_ASC14B_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_ASC14B_FONT_BUF_SIZE);
					
					if(Gui_GetLandScapeMode())//横屏
						Gui_SetRegion(pRegion->x-RowLen,pRegion->y+ColLen,GUI_ASC14B_ASCII_WIDTH,GUI_ASC14B_FONT_HEIGHT);
					else//正常
						Gui_SetRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_ASC14B_ASCII_WIDTH,GUI_ASC14B_FONT_HEIGHT);

					LCD_BlukWriteDataStart();
					for(h=0;h<GUI_ASC14B_FONT_BUF_SIZE;h++)//显示一个ascii共28个字节的点阵信息
					{
						for(w=8;w!=0;w--)//读取当前字节
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
					//越边界检查
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_ASC14B_CHAR_WIDTH;
					else w=GUI_ASC14B_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//如果超过预设宽度
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

//返回实际区域需要的字符串byte数
//返回值实际就是Gui_DrawFont_GBK12的返回值，只不过不显示实际内容
static u16 Gui_DrawFont_ASC14B_Num(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//计算字符占用位置，按像素表示
	u8 w,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	
	while((Offset=*pStr++)!=0) //开始读取字符串
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//显示汉字
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//越底界检查
			if((RowLen+GUI_ASC14B_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			pStr++;
			
			ColLen+=(GUI_ASC14B_CHAR_WIDTH+Space);
			//越边界检查
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_ASC14B_CHAR_WIDTH;
			else w=GUI_ASC14B_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//如果超过预设宽度
			{				
				ColLen=0;
				RowLen+=(GUI_ASC14B_FONT_HEIGHT+RowSpace);
			}
			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//显示符号
		{
			//Debug("C:%c ",*(pStr-1));
			//越底界检查
			if((RowLen+GUI_ASC14B_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//检测符号类型
			{
				case 0x0A:	//换行回车的情况
				{
					ColLen=0;
					RowLen+=(GUI_ASC14B_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //普通字符的情况
				{					
					ColLen+=(GUI_ASC14B_ASCII_WIDTH+Space);
					//越边界检查
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_ASC14B_CHAR_WIDTH;
					else w=GUI_ASC14B_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//如果超过预设宽度
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

//从存储体中读取字库显示汉字
//返回实际读取的byte
//注意region.space和region.row_space用于控制字距,且必须设置值!!!
static u16 Gui_DrawFont_GBK21(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//计算字符占用位置，按像素表示
	u32 FontLocal;	//字库偏移量
	u8 CharBuf[GUI_GBK21_FONT_BUF_SIZE];
	u8 w,h,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	COLOR_TYPE Color;

#if USE_32BIT_COLOR_TYPE
	Color=ChColor(pRegion->Color);//转换颜色	
#else
	Color=pRegion->Color;
#endif

	LCD_Lock();
	
	while((Offset=*pStr++)!=0) //开始读取字符串
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//显示汉字
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//越底界检查
			if((RowLen+GUI_GBK21_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			FontLocal=((Offset-GUI_GBK_FONT_OFFSET_H)*GUI_GBK_FONT_PAGENUM+
				((*pStr)-GUI_GBK_FONT_OFFSET_L))*GUI_GBK21_FONT_BUF_SIZE;	//获取偏移量,<<x= *GUI_GBK21_FONT_BUF_SIZE
			if((*pStr++) > GUI_GBK_FONT_OFFSET_P) FontLocal-=GUI_GBK21_FONT_BUF_SIZE; //除去字库中0x7f这条线
			
			//移到字库对应位置读取一个字的点阵数据,63个字节
			Gui_ReadFontLib(GUI_GBK21_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_GBK21_FONT_BUF_SIZE);

			if(Gui_GetLandScapeMode())//横屏
				Gui_SetRegion(pRegion->x-RowLen,pRegion->y+ColLen,GUI_GBK21_CHAR_WIDTH,GUI_GBK21_FONT_HEIGHT);
			else//正常
				Gui_SetRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_GBK21_CHAR_WIDTH,GUI_GBK21_FONT_HEIGHT);

			LCD_BlukWriteDataStart();
			for(h=0;h<GUI_GBK21_FONT_BUF_SIZE;h++)//显示一个汉字共63个字节的点阵信息
			{
				for(w=8;w!=0;w--)//读取当前字节
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

				for(w=8,h++;w!=0;w--)//读取当前字节
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

				for(w=5,h++;w!=0;w--)//读取当前字节
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
			//越边界检查
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK21_CHAR_WIDTH;
			else w=GUI_GBK21_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//如果超过预设宽度
			{				
				ColLen=0;
				RowLen+=(GUI_GBK21_FONT_HEIGHT+RowSpace);
			}

			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//显示符号
		{
			//Debug("C:%c ",*(pStr-1));
			//越底界检查
			if((RowLen+GUI_GBK21_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//检测符号类型
			{
				case 0x0A:	//换行回车的情况
				{
					ColLen=0;
					RowLen+=(GUI_GBK21_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //普通字符的情况
				{
					FontLocal=GUI_GBK21_FONT_END+Offset*GUI_GBK21_FONT_BUF_SIZE;//获取字库中ASCii的偏移量
					
					//移到字库对应位置读取一个字的点阵数据,63个字节
					Gui_ReadFontLib(GUI_GBK21_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_GBK21_FONT_BUF_SIZE);
					
					if(Gui_GetLandScapeMode())//横屏
						Gui_SetRegion(pRegion->x-RowLen,pRegion->y+ColLen,GUI_GBK21_ASCII_WIDTH,GUI_GBK21_FONT_HEIGHT);
					else//正常
						Gui_SetRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_GBK21_ASCII_WIDTH,GUI_GBK21_FONT_HEIGHT);

					LCD_BlukWriteDataStart();
					for(h=0;h<GUI_GBK21_FONT_BUF_SIZE;h++)//显示一个ascii共63个字节的点阵信息
					{
						for(w=8;w!=0;w--)//读取当前字节
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

						for(w=3,h++;w!=0;w--)//读取当前字节
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
					//越边界检查
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK21_CHAR_WIDTH;
					else w=GUI_GBK21_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//如果超过预设宽度
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

//返回实际区域需要的字符串byte数
//返回值实际就是Gui_DrawFont_GBK16的返回值，只不过不显示实际内容
static u16 Gui_DrawFont_GBK21_Num(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//计算字符占用位置，按像素表示
	u8 w,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	
	while((Offset=*pStr++)!=0) //开始读取字符串
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//显示汉字
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//越底界检查
			if((RowLen+GUI_GBK21_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			pStr++;
			
			ColLen+=(GUI_GBK21_CHAR_WIDTH+Space);
			//越边界检查
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK21_CHAR_WIDTH;
			else w=GUI_GBK21_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//如果超过预设宽度
			{				
				ColLen=0;
				RowLen+=(GUI_GBK21_FONT_HEIGHT+RowSpace);
			}
			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//显示符号
		{
			//Debug("C:%c ",*(pStr-1));
			//越底界检查
			if((RowLen+GUI_GBK21_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//检测符号类型
			{
				case 0x0A:	//换行回车的情况
				{
					ColLen=0;
					RowLen+=(GUI_GBK21_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //普通字符的情况
				{					
					ColLen+=(GUI_GBK21_ASCII_WIDTH+Space);
					//越边界检查
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK21_CHAR_WIDTH;
					else w=GUI_GBK21_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//如果超过预设宽度
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

//通用显示字符的函数
u16 Gui_DrawFont(FONT_ACT FontLib,const u8 *Str,const GUI_REGION *pRegion)
{
	switch(FontLib)
	{
		case GBK12_FONT:
#if GUI_USE_FS_FONT_LIB//使用sd卡字库
		{
			u16 Result;
			
			if(!Q_DB_GetStatus(Status_FsInitFinish,NULL)) return 0;//文件系统未加载
			
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
#if GUI_USE_FS_FONT_LIB//使用sd卡字库
		{
			u16 Result;
			
			if(!Q_DB_GetStatus(Status_FsInitFinish,NULL)) return 0;//文件系统未加载
			
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
#if GUI_USE_FS_FONT_LIB//使用sd卡字库
		{
			u16 Result;
			
			if(!Q_DB_GetStatus(Status_FsInitFinish,NULL)) return 0;//文件系统未加载
			
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
#if GUI_USE_FS_FONT_LIB//使用sd卡字库
		{
			u16 Result;
			
			if(!Q_DB_GetStatus(Status_FsInitFinish,NULL)) return 0;//文件系统未加载
			
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

//读取bmp文件的图片宽高信息
//从指针返回读取值
//允许指针参数为NULL
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
		*Width=biWidth;//图片宽
	if(Hight!=NULL)
		*Hight=biHeight;//图片高
	
Gui_Return:
	FS_FClose(fp);

	return Ret;
}

//写一幅满屏bmp时间为172ms
//只需指定起点x y
//图片超出范围就不会显示
//pBmpRegion->TransColor指定透明色
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
	u32 OnceReadByte,OnceReadRow;//每次需要读取的byte和图片行
	UINT ReadByte,ReadRow;//实际每次读取的byte和图片行
	u32 BmpWidthByte;
	u16 BmpWidth,BmpHight;
	u8 *pBmpRgb;
	bool NeedTrans=FALSE;//是否需要透明色
	u8 BmpMargin;  //  bmp中为保证4字节对齐的数据留空
	u16 BmpRgb;	//保存转换字节的变量
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

	if(FS_FSeek(fp,0x38,FS_SEEK_SET)==-1)//移到位图色彩表+2
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
	
	BmpWidth=biWidth;//图片宽
	BmpHight=biHeight;//图片高

	BmpMargin=(4-(BmpWidth*3))&0x03;//计算bmp数据边白填充字节数
	BmpWidthByte=(BmpWidth*3+BmpMargin);
	OnceReadRow=GUI_BMP_COLOR_TABLE_BUF_SIZE/BmpWidthByte;
	OnceReadByte=OnceReadRow*BmpWidthByte;
	
	LCD_Lock();//独占屏幕
	if(Gui_SetRegion(pBmpRegion->x,pBmpRegion->y,BmpWidth,BmpHight)
		==Gui_Out_Of_Range)
	{
		LCD_UnLock();
		Ret=Gui_Out_Of_Range;
		goto Gui_Return;
	}
	
	if(Gui_GetLandScapeMode())//横屏
	{
		LCD_SetXY(pBmpRegion->x,pBmpRegion->y);
		LCD_SetAddrIncMode(NormalAddrIncMode);
	}
	else//正常
	{
		LCD_SetXY(pBmpRegion->x,pBmpRegion->y+BmpHight-1);
		LCD_SetAddrIncMode(xInc_yDec);
	}
	LCD_BgrMode(TRUE);

	if(pBmpRegion->Color!=FatColor(NO_TRANS))//判断是否需要透明色
	{
#if USE_32BIT_COLOR_TYPE
		TransColor=pBmpRegion->Color;
#else
		TransColor=RGB2BGR(pBmpRegion->Color);//需要转换透明色的模式为BGR
#endif
		NeedTrans=TRUE;
	}

	gBmpColorTableBuf=Q_Mallco(GUI_BMP_COLOR_TABLE_BUF_SIZE+8);//分配缓存
	
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

		if(NeedTrans) //需要透明色
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
				pBmpRgb+=BmpMargin;//跳过填充
			}	
		}
		else //不需要透明色
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
				pBmpRgb+=BmpMargin;//跳过填充
			}	
		}
  	}	

  	Q_Free(gBmpColorTableBuf);//释放缓存
  	
	if(Gui_GetLandScapeMode())//横屏
	{
		LCD_SetAddrIncMode(LandScapeAddrIncMode);
	}
	else//正常
	{
		LCD_SetAddrIncMode(NormalAddrIncMode);
	}
	LCD_BgrMode(FALSE);
	LCD_UnLock();//释放屏幕资源
	
Gui_Return:
	if( fp )
	{
		FS_FClose(fp);
	}

	return Ret;
}

//读取数组，画图
//数组全部为色彩数据，每两个字节代表一个像素
//入口需指点起点x,y和长宽w,h信息
//写一幅满屏图片时间为32ms。当然，你牺牲的是Rom。
GUI_RESULT Gui_DrawImgArray(const u8 * pImageBuf,const GUI_REGION *pRegion)
{
	u32 i;
	u16 *pColor16=(u16 *)pImageBuf;
	COLOR_TYPE TransColor=0;
	bool NeedTrans=FALSE;
	
	LCD_Lock();//独占屏幕
	if(Gui_SetRegion(pRegion->x,pRegion->y,pRegion->w,pRegion->h)==Gui_Out_Of_Range)
	{
		LCD_UnLock();
		return Gui_Out_Of_Range;
	}

	LCD_BlukWriteDataStart();
	if(pRegion->Color!=FatColor(NO_TRANS))//判断是否需要透明色
	{
#if USE_32BIT_COLOR_TYPE
		TransColor=ChColor(pRegion->Color);
#else
		TransColor=pRegion->Color;//需要转换透明色的模式为BGR
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
	
	LCD_UnLock();//释放屏幕资源

	return Gui_True;
}

//读取文件系统bin文件，画图
//bin文件全部为色彩数据，每两个字节代表一个像素
//入口需指点起点x,y和长宽w,h信息
//写一幅满屏图片时间为104ms，还可以根据sd卡速度提高
GUI_RESULT Gui_DrawImgBin(const u8 * pPath,const GUI_REGION *pRegion)
{
	GUI_RESULT Ret=Gui_True;
	u32 i;
	UINT ReadByte;//实际每次读取的byte和图片行
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
	
	LCD_Lock();//独占屏幕
	if(Gui_SetRegion(pRegion->x,pRegion->y,pRegion->w,pRegion->h)==Gui_Out_Of_Range)
	{
		LCD_UnLock();
		Ret=Gui_Out_Of_Range;
		goto Gui_Return;
	}

	if(pRegion->Color!=FatColor(NO_TRANS))//判断是否需要透明色
	{
#if USE_32BIT_COLOR_TYPE
		TransColor=ChColor(pRegion->Color);
#else
		TransColor=pRegion->Color;//需要转换透明色的模式为BGR
#endif
		NeedTrans=TRUE;
	}

	gBmpColorTableBuf=Q_Mallco(GUI_BMP_COLOR_TABLE_BUF_SIZE);//分配缓存
	
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

  	Q_Free(gBmpColorTableBuf);//释放缓存
  	
	LCD_UnLock();//释放屏幕资源

Gui_Return:	
	if( fp)
		FS_FClose(fp);

	return Ret;
}

//读取Spi Flash系统bin文件，画图
//bin文件全部为色彩数据，每两个字节代表一个像素
//入口需指点起点x,y和长宽w,h信息
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

	LCD_Lock();//独占屏幕
	if(Gui_SetRegion(pRegion->x,pRegion->y,pRegion->w,pRegion->h)==Gui_Out_Of_Range)
	{
		LCD_UnLock();
		return Gui_Out_Of_Range;
	}

	if(pRegion->Color!=FatColor(NO_TRANS))//判断是否需要透明色
	{
#if USE_32BIT_COLOR_TYPE
		TransColor=ChColor(pRegion->Color);
#else
		TransColor=pRegion->Color;//需要转换透明色的模式为BGR
#endif
		NeedTrans=TRUE;
	}

	gBmpColorTableBuf=Q_Mallco(GUI_BMP_COLOR_TABLE_BUF_SIZE);//分配缓存
	
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
  	
	LCD_UnLock();//释放屏幕资源
	
	return Ret;
}

//将一个数组格式的图片填充到指定区域
//pImageBuf通过Image2Lcd工具获取,请参照Gui_DrawImageArray函数
//pRegion需指定x y w h参数来指定填充区域
//width，hight指定图像宽高
GUI_RESULT Gui_FillImgArray(const u8 * pImageBuf,u16 width,u16 hight,const GUI_REGION *pRegion)
{
	u16 *pColor16;
	u16 FillNumH;//横着需要重复的个数
	u16 FillNumV;//竖着需要重复的个数
	u16 LastOneH;//最后一次的宽度
	u16 LastOneV;//最后一次的高度
	u32 i,j,n,m;
	u16 Hstart,Vstart;
	u16 H=0,V;
	bool NeedTrans=FALSE;
	COLOR_TYPE TransColor=0;

	LCD_Lock();//独占屏幕
	
	if(Gui_SetRegion(pRegion->x,pRegion->y,pRegion->w,pRegion->h)==Gui_Out_Of_Range)
	{
		LCD_UnLock();
		return Gui_Out_Of_Range;
	}
	
	if(pRegion->Color!=FatColor(NO_TRANS))//判断是否需要透明色
	{
#if USE_32BIT_COLOR_TYPE
		TransColor=ChColor(pRegion->Color);//转换颜色	
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
		for(i=0,V=Vstart;i<FillNumV;i++)//每行
		{
			for(j=0,H=Hstart;j<FillNumH;j++)//每个
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

			//每行最后一个
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

		//最后一行
		if(LastOneV)
		{
			H=Hstart;
			for(j=0;j<FillNumH;j++,H+=width)//最后一行的每个
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

		//最后一行最后一个
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
	else //不需要透明色
	{
		for(i=0,V=Vstart;i<FillNumV;i++)//每行
		{
			for(j=0,H=Hstart;j<FillNumH;j++)//每个
			{
				pColor16=(u16 *)pImageBuf;
				if(gLandScapeMode) Gui_SetRegion(V,H,width,hight);
				else Gui_SetRegion(H,V,width,hight);
				LCD_BlukWriteDataStart();
				for(n=width*hight;n;n--)
					LCD_BulkWriteData(*pColor16++);

				H+=width;
			}

			//每行最后一个
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

		//最后一行
		if(LastOneV)
		{
			H=Hstart;
			for(j=0;j<FillNumH;j++,H+=width)//最后一行的每个
			{
				pColor16=(u16 *)pImageBuf;
				if(gLandScapeMode) Gui_SetRegion(V,H,width,LastOneV);
				else Gui_SetRegion(H,V,width,LastOneV);
				LCD_BlukWriteDataStart();
				for(n=width*LastOneV;n;n--)
					LCD_BulkWriteData(*pColor16++);
			}
		}

		//最后一行最后一个
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
	
	LCD_UnLock();//释放屏幕资源

	return Gui_True;
}

//将一个数组格式的图片横向填充到指定区域
//pImageBuf通过Image2Lcd工具获取,请参照Gui_DrawImageArray函数
//pRegion需指定x y w h参数来描述图片和起点
//width指定填充区域的宽度
GUI_RESULT Gui_FillImgArray_H(const u8 * pImageBuf,u16 width,const GUI_REGION *pRegion)
{
	u16 *pColor16;
	u16 FillNum=width/pRegion->w;//需要重复的个数
	u8 LastOne=width%pRegion->w*pRegion->h;//最后一次的宽度
	u32 i,j;

	LCD_Lock();//独占屏幕
	if(Gui_SetRegion(pRegion->x,pRegion->y,pRegion->w,pRegion->h)==Gui_Out_Of_Range)
	{
		LCD_UnLock();
		return Gui_Out_Of_Range;
	}
	
	LCD_SetRegion(pRegion->x,pRegion->y,
		pRegion->x+width-1,pRegion->y+pRegion->h-1,TRUE);//设置描绘模式
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
	LCD_UnLock();//释放屏幕资源

	return Gui_True;
}

//将一个数组格式的图片纵向填充到指定区域
//pImageBuf通过Image2Lcd工具获取,请参照Gui_DrawImageArray函数
//pRegion需指定x y w h参数来描述图片信息和起点
//hight指定填充区域的高度
GUI_RESULT Gui_FillImgArray_V(const u8 * pImageBuf,u16 hight,const GUI_REGION *pRegion)
{
	u16 *pColor16;
	u16 FillNum=hight/pRegion->h;//需要重复的个数
	u8 LastOne=hight%pRegion->h*pRegion->w;//最后一次的宽度
	u32 i,j;

	LCD_Lock();//独占屏幕
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
	LCD_UnLock();//释放屏幕资源

	return Gui_True;
}

//填充区宽度必须是图片宽度的整数倍
//pRegion->x指定起点x坐标
//pRegion->y指定起点y坐标
//pRegion->w指定填充宽度
GUI_RESULT Gui_FillBmp_H(const u8 *pBmpPath,const GUI_REGION *pRegion)
{
	GUI_REGION BmpRegion;
	u16 Times;
	u16 BmpW;

	BmpRegion.x=pRegion->x;
	BmpRegion.y=pRegion->y;
	BmpRegion.Color=pRegion->Color;
	Gui_ReadBmpInfo(pBmpPath,&BmpW,NULL);
	Times=BmpRegion.w/BmpW;//如果不能被整除，将出现间隙

	for(;Times;Times--,BmpRegion.x+=BmpW)
	{
		Gui_Draw24Bmp(pBmpPath,&BmpRegion);
	}

	return Gui_True;
}

//填充区高度必须是图片高度的整数倍
//pRegion->x指定起点x坐标
//pRegion->y指定起点y坐标
//pRegion->h指定填充高度
GUI_RESULT Gui_FillBmp_V(const u8 *pBmpPath,const GUI_REGION *pRegion)
{
	GUI_REGION BmpRegion;
	u8 Times;
	u16 BmpH;

	BmpRegion.x=pRegion->x;
	BmpRegion.y=pRegion->y;
	BmpRegion.Color=pRegion->Color;
	Gui_ReadBmpInfo(pBmpPath,NULL,&BmpH);
	Times=BmpRegion.h/BmpH;//如果不能被整除，将出现间隙

	for(;Times;Times--,BmpRegion.y+=BmpH)
	{
		Gui_Draw24Bmp(pBmpPath,&BmpRegion);
	}

	return Gui_True;
}

//用指定前缀生成的三个图片来横向填充一个区域
//pRegion->x指定起点x坐标
//pRegion->y指定起点y坐标
//pRegion->w指定填充宽度
GUI_RESULT Gui_DrawFrame_H(const u8 *pFramePathPrefix,const GUI_REGION *pRegion)
{
	u8 PathBuf[64]="";//路径最大不得超过64个字符
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
	
	//先画框框左图
	strcpy((void *)PathBuf,(void *)pFramePathPrefix);
	strcat((void *)PathBuf,"L.bmp");
	Gui_ReadBmpInfo((void *)PathBuf,&Lwidth,NULL);
	Gui_Draw24Bmp((void *)PathBuf,&BmpRegion);
	
	//再根据右图大小画右图
	strcpy((void *)PathBuf,(void *)pFramePathPrefix);
	strcat((void *)PathBuf,"R.bmp");
	Gui_ReadBmpInfo((void *)PathBuf,&Rwidth,NULL);	
	BmpRegion.x+=pRegion->w-Rwidth;
	Gui_Draw24Bmp((void *)PathBuf,&BmpRegion);
	
	//最后填充中间
	strcpy((void *)PathBuf,(void *)pFramePathPrefix);
	strcat((void *)PathBuf,"M.bmp");
	BmpRegion.x=pRegion->x+Lwidth;
	BmpRegion.w=pRegion->w-Lwidth-Rwidth;
	Gui_FillBmp_H((void *)PathBuf,&BmpRegion);
	
	return Gui_True;
}

//用指定前缀生成的三个图片来纵向填充一个区域
//pRegion->x指定起点x坐标
//pRegion->y指定起点y坐标
//pRegion->h指定填充高度
GUI_RESULT Gui_DrawFrame_V(const u8 *pFramePathPrefix,const GUI_REGION *pRegion)
{
	u8 PathBuf[64]="";//路径最大不得超过64个字符
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
	
	//先画框框上图
	strcpy((void *)PathBuf,(void *)pFramePathPrefix);
	strcat((void *)PathBuf,"T.bmp");
	Gui_ReadBmpInfo((void *)PathBuf,NULL,&Thight);
	Gui_Draw24Bmp((void *)PathBuf,&BmpRegion);

	//再根据下图大小画下图
	strcpy((void *)PathBuf,(void *)pFramePathPrefix);
	strcat((void *)PathBuf,"B.bmp");
	Gui_ReadBmpInfo((void *)PathBuf,NULL,&Bhight);	
	BmpRegion.y+=BmpRegion.h-Bhight;
	Gui_Draw24Bmp((void *)PathBuf,&BmpRegion);
	
	//最后填充中间
	strcpy((void *)PathBuf,(void *)pFramePathPrefix);
	strcat((void *)PathBuf,"M.bmp");
	BmpRegion.y=pRegion->y+Thight;
	BmpRegion.h=pRegion->h-Thight-Bhight;
	Gui_FillBmp_V((void *)PathBuf,&BmpRegion);
	
	return Gui_True;
}

//画线函数，使用Bresenham 画线算法
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
	Color=ChColor(Color);//转换颜色	
#endif

	LCD_Lock();
	Gui_SetLandScapeMode(FALSE);
	Gui_SetRegion(0,0,LCD_WIDTH,LCD_HIGHT);
	
	dx = x1-x0;//计算x距离
	dy = y1-y0;//计算y距离

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

	if (dx > dy)//x距离大于y距离，那么每个x轴上只有一个点，每个y轴上有若干个点
	{//且线的点数等于x距离，以x轴递增画点
		// initialize error term
		error = dy2 - dx; 

		// draw the line
		for (index=0; index <= dx; index++)//要画的点数不会超过x距离
		{
			//画点
			LCD_DrawPoint(x0,y0,Color);
			
			// test if error has overflowed
			if (error >= 0) //是否需要增加y坐标值
			{
				error-=dx2;

				// move to next line
				y0+=y_inc;//增加y坐标值
			} // end if error overflowed

			// adjust the error term
			error+=dy2;

			// move to the next pixel
			x0+=x_inc;//x坐标值每次画点后都递增1
		} // end for
	} // end if |slope| <= 1
	else//y轴大于x轴，则每个y轴上只有一个点，x轴若干个点
	{//以y轴为递增画点
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

//画彩线,同Gui_DrawLine
//ColorBuf数组用于存放16位的色彩信息,每个点用一个
//最后一个点不画
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
	
	dx = x1-x0;//计算x距离
	dy = y1-y0;//计算y距离

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

	if (dx > dy)//x距离大于y距离，那么每个x轴上只有一个点，每个y轴上有若干个点
	{//且线的点数等于x距离，以x轴递增画点
		// initialize error term
		error = dy2 - dx; 

		// draw the line
		dx--;//最后一个点不需要了
		for (index=0; index <= dx; index++)//要画的点数不会超过x距离
		{
			//画点
			LCD_DrawPoint(x0,y0,ColorBuf[index]);
			
			// test if error has overflowed
			if (error >= 0) //是否需要增加y坐标值
			{
				error-=dx2;

				// move to next line
				y0+=y_inc;//增加y坐标值
			} // end if error overflowed

			// adjust the error term
			error+=dy2;

			// move to the next pixel
			x0+=x_inc;//x坐标值每次画点后都递增1
		} // end for
	} // end if |slope| <= 1
	else//y轴大于x轴，则每个y轴上只有一个点，x轴若干个点
	{//以y轴为递增画点
		// initialize error term
		error = dx2 - dy; 

		// draw the line
		dy--;//最后一个点不需要了
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

//储存一条线经过的所有点的色彩信息
//ColorBuf数组用于存放16位的色彩信息,每个点用一个
//最后一个点不存
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
	
	dx = x1-x0;//计算x距离
	dy = y1-y0;//计算y距离

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

	if (dx > dy)//x距离大于y距离，那么每个x轴上只有一个点，每个y轴上有若干个点
	{//且线的点数等于x距离，以x轴递增画点
		// initialize error term
		error = dy2 - dx; 

		// draw the line
		dx--;//最后一个点不需要了
		for (index=0; index <= dx; index++)//要画的点数不会超过x距离
		{
			//存点
			ColorBuf[index]=Gui_ReadPixel16Bit(x0,y0);
			
			// test if error has overflowed
			if (error >= 0) //是否需要增加y坐标值
			{
				error-=dx2;

				// move to next line
				y0+=y_inc;//增加y坐标值
			} // end if error overflowed

			// adjust the error term
			error+=dy2;

			// move to the next pixel
			x0+=x_inc;//x坐标值每次画点后都递增1
		} // end for
	} // end if |slope| <= 1
	else//y轴大于x轴，则每个y轴上只有一个点，x轴若干个点
	{//以y轴为递增画点
		// initialize error term
		error = dx2 - dy; 

		// draw the line
		dy--;//最后一个点不需要了
		for (index=0; index <= dy; index++)
		{
			// 存点
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

//画虚线线函数，同Gui_DrawLine,gap指定间隔
//Gap最高四个位指定线长
//Gap次高四个位指定空隙长
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
	
	dx = x1-x0;//计算x距离
	dy = y1-y0;//计算y距离

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

	if (dx > dy)//x距离大于y距离，那么每个x轴上只有一个点，每个y轴上有若干个点
	{//且线的点数等于x距离，以x轴递增画点
		// initialize error term
		error = dy2 - dx; 

		// draw the line
		for (index=0; index <= dx; index++)//要画的点数不会超过x距离
		{
			//画点
			if((index%UnitLen)<LineLen)
			{
				LCD_DrawPoint(x0,y0,Color);
			}
			
			// test if error has overflowed
			if (error >= 0) //是否需要增加y坐标值
			{
				error-=dx2;

				// move to next line
				y0+=y_inc;//增加y坐标值
			} // end if error overflowed

			// adjust the error term
			error+=dy2;

			// move to the next pixel
			x0+=x_inc;//x坐标值每次画点后都递增1
		} // end for
	} // end if |slope| <= 1
	else//y轴大于x轴，则每个y轴上只有一个点，x轴若干个点
	{//以y轴为递增画点
		// initialize error term
		error = dx2 - dy; 

		// draw the line
		for (index=0; index <= dy; index++)
		{
			//画点
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

//八对称性
static void __inline Gui_DrawCircle8(u16 xc, u16 yc, u16 x, u16 y,u8 FillMode,COLOR_TYPE Color) 
{

	if(FillMode&(1<<0)) LCD_DrawPoint(xc + x, yc - y,Color);// 1上
	if(FillMode&(1<<1)) LCD_DrawPoint(xc + y, yc - x,Color);// 1下
	if(FillMode&(1<<2)) LCD_DrawPoint(xc + y, yc + x,Color);// 2上
	if(FillMode&(1<<3)) LCD_DrawPoint(xc + x, yc + y,Color);// 2下	
	if(FillMode&(1<<4)) LCD_DrawPoint(xc - x, yc + y,Color);// 3下
	if(FillMode&(1<<5)) LCD_DrawPoint(xc - y, yc + x,Color);// 3上
	if(FillMode&(1<<6)) LCD_DrawPoint(xc - y, yc - x,Color);// 4下
	if(FillMode&(1<<7)) LCD_DrawPoint(xc - x, yc - y,Color);// 4上
}

//Bresenham画圆算法
//pRegion->x,y指定圆心坐标
//pRegion->w指定圆半径
//pRegion->Color指定填充颜色
//pRegion->Space指定填充区域,0xff-填充8个区域
//Cir:TRUE画圆 FALSE画弧
GUI_RESULT Gui_DrawCircle(const GUI_REGION *pRegion,bool Cir) 
{
	int x = 0, y = pRegion->w, yi, d= 3 - 2 * pRegion->w;
	COLOR_TYPE Color;
	bool LandMode=Gui_GetLandScapeMode();
	
	if (pRegion->x + pRegion->w < 0 || pRegion->x - pRegion->w >= LCD_WIDTH || 
		pRegion->y + pRegion->w < 0 || pRegion->y - pRegion->w >= LCD_HIGHT) 
		return Gui_Out_Of_Range;// 如果圆在图片可见区域外，直接退出

#if USE_32BIT_COLOR_TYPE
	Color=ChColor(pRegion->Color);//转换颜色	
#else
	Color=pRegion->Color;
#endif

	LCD_Lock();
	Gui_SetLandScapeMode(FALSE);
	Gui_SetRegion(0,0,LCD_WIDTH,LCD_HIGHT);
	
	if (Cir) // 如果填充（画实心圆）
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
	else // 如果不填充（画空心圆）
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


//读指定区域的颜色
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

//读指定区域的颜色
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

//画一个点的色彩
void Gui_WritePixel(u16 x,u16 y,COLOR_TYPE Color)
{
	if((x>=LCD_WIDTH)||(y>=LCD_HIGHT)) return;
	
#if USE_32BIT_COLOR_TYPE
	Color=ChColor(Color);//转换颜色	
#endif

	LCD_Lock();
	LCD_DrawPoint(x,y,Color);
	LCD_UnLock();
}

//读一个点的色彩信息
u16 Gui_ReadPixel16Bit(u16 x,u16 y)
{
	u16 Color;

	if((x>=LCD_WIDTH)||(y>=LCD_HIGHT)) return 0;
	
	LCD_Lock();
	Color=LCD_ReadPoint(x,y);
	LCD_UnLock();
	
	return Color;
}


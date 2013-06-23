#ifndef QSYS_Q_GUI_H
#define QSYS_Q_GUI_H

//Gui配置
#define LCD_WIDTH	240//屏幕宽
#define LCD_HIGHT		320//屏幕高
#define GUI_USE_FS_FONT_LIB 0 //选择字库是放文件系统还是放板载flash里
#define GUI_BMP_COLOR_TABLE_BUF_SIZE 512*8  //最大240的图片需要720byte,4k时速度最快

#if 0
//显示相关

#define LCD_DATA_BYTE 	2//传输给lcd一个像素点的数据的字节数
#define BMP_RGB_BYTE 	3//bmp位图的一个像素点的字节数
#define LCD_R_MASK 		0xf8
#define LCD_R_OFFSET 		3 //往右移
#define LCD_G_MASK 		0xfc
#define LCD_G_OFFSET 		3 //往左移
#define LCD_B_MASK			0xf8
#define LCD_B_OFFSET 		8 //往左移
#endif

//字库选择
typedef enum{
	GBK12_FONT=0,//写12x12的字
	GBK12_NUM,//计算区域内可容纳的12x12字数
	GBK16_FONT,//写字
	GBK16_NUM,//计数
	ASC14B_FONT,//写字
	ASC14B_NUM,//计数
	GBK21_FONT,//写字
	GBK21_NUM,//计数
}FONT_ACT;

//GBK字库定义
#define GUI_GBK_FONT_OFFSET_H		0x81
#define GUI_GBK_FONT_OFFSET_L		0x40
#define GUI_GBK_FONT_OFFSET_P		0x7F
#define GUI_GBK_FONT_PAGENUM    	0xBE //=0xFE-0x40+1-1

//GBK16x16字库
#define GUI_GBK16_FONT_HEIGHT	16			//字体高度是16
#define GUI_GBK16_CHAR_WIDTH	16			//汉字字符宽16
#define GUI_GBK16_ASCII_WIDTH	8			//ascii字符宽8
#define GUI_GBK16_FONT_BUF_SIZE 	32 //一个汉字需要多少个字节来存储。=GUI_GBK_FONT_SIZE/8xGUI_GBK_FONT_SIZE
#define GUI_GBK16_FONT_SPI_FLASH_BASE		(682*256)// DOWNLOAD_START_PAGE*DOWNLOAD_PAGE_SIZE存储字库的起始页
#define GUI_GBK16_FONT_END				(23940*GUI_GBK16_FONT_BUF_SIZE) //字库结束点 即Ascii库起点

//GBK12x12字库
#define GUI_GBK12_FONT_HEIGHT	12		//字体高度是12
#define GUI_GBK12_CHAR_WIDTH	12			//汉字字符宽12
#define GUI_GBK12_ASCII_WIDTH	6			//ascii字符宽6
#define GUI_GBK12_FONT_BUF_SIZE 	24 //一个汉字需要多少个字节来存储。=GUI_GBK_FONT_SIZE/8xGUI_GBK_FONT_SIZE
#define GUI_GBK12_FONT_SPI_FLASH_BASE		(3691*256)// DOWNLOAD_START_PAGE*DOWNLOAD_PAGE_SIZE存储字库的起始页
#define GUI_GBK12_FONT_END	(23940*GUI_GBK12_FONT_BUF_SIZE) //字库结束点 即Ascii库起点

//ASCII 14 BOLD 字库定义
#define GUI_ASC14B_FONT_HEIGHT	14		//字体高度是14
#define GUI_ASC14B_CHAR_WIDTH		15			//汉字字符宽15，实际上此字库不支持汉字
#define GUI_ASC14B_ASCII_WIDTH	8			//ascii字符宽8
#define GUI_ASC14B_FONT_BUF_SIZE 	28 //一个字需要多少个字节来存储。=GUI_GBK_FONT_SIZE/8xGUI_GBK_FONT_SIZE
#define GUI_ASC14B_FONT_SPI_FLASH_BASE		(5948*256)// DOWNLOAD_START_PAGE*DOWNLOAD_PAGE_SIZE存储字库的起始页

//GBK21x21字库
#define GUI_GBK21_FONT_HEIGHT	21		//字体高度是12
#define GUI_GBK21_CHAR_WIDTH	21			//汉字字符宽12
#define GUI_GBK21_ASCII_WIDTH	11			//ascii字符宽6
#define GUI_GBK21_FONT_BUF_SIZE 	63 //一个汉字需要多少个字节来存储。=GUI_GBK_FONT_SIZE/8xGUI_GBK_FONT_SIZE
#define GUI_GBK21_FONT_SPI_FLASH_BASE		(8192*256)// DOWNLOAD_START_PAGE*DOWNLOAD_PAGE_SIZE存储字库的起始页
#define GUI_GBK21_FONT_END	0x170380//align 16 =(23940*GUI_GBK21_FONT_BUF_SIZE) //字库结束点 即Ascii库起点

#if 0
//GB2312字库
#define GUI_FONT_GB2312_OFFSET_H			0xA1
#define GUI_FONT_GB2312_OFFSET_L			0xA1
#define GUI_FONT_GB2312_PAGENUM    		0x5E //=0xFE-0xA1+1
#define GUI_FONT_GB2312_SPACE					0		//间距
#define GUI_FONT_GB2312_MARGIN				16		//行距
#define GUI_FONT_GB2312_END						8836*128
#endif

#if 1//bmp info
//bmp文件组成
// 1.文件头
// 2.信息头
// 3.彩色表
typedef long	 LONG;
//BMP头文件
typedef __packed struct
{
    u16  bfType ;     		//文件标志.只对'BM',用来识别BMP位图类型
    u32 bfSize ;           //文件大小,占四个字节
    u16  bfReserved1 ;	//保留
    u16  bfReserved2 ;	//保留
    u32 bfOffBits ;  		//从文件开始到位图数据(bitmap data)开始之间的的偏移量
}BMP_FILE_HEADER;

//BMP信息头
typedef __packed struct 
{
    u32 biSize ;                   //说明BITMAPINFOHEADER结构所需要的字数。
    LONG  biWidth ;                   //说明图象的宽度，以象素为单位
    LONG  biHeight ;           		//说明图象的高度，以象素为单位
    u16  biPlanes ;           		//为目标设备说明位面数，其值将总是被设为1
    u16  biBitCount ;           	//说明比特数/象素，其值为1、4、8、16、24、或32
    u32 biCompression ;  	//说明图象数据压缩的类型。其值可以是下述值之一：
												// 0 :BI_RGB：没有压缩；
												// 1:BI_RLE8：每个象素8比特的RLE压缩编码，压缩格式由2字节组成(重复象素计数和颜色索引)；  
												// 2:BI_RLE4：每个象素4比特的RLE压缩编码，压缩格式由2字节组成
												// 3:BI_BITFIELDS：每个象素的比特由指定的掩码决定。
    u32 biSizeImage ;			//说明图象的大小，以字节为单位。当用BI_RGB格式时，可设置为0  
    LONG  biXPelsPerMeter ;	//说明水平分辨率，用象素/米表示
    LONG  biYPelsPerMeter ;	//说明垂直分辨率，用象素/米表示
    u32 biClrUsed ;           	//说明位图实际使用的彩色表中的颜色索引数
    u32 biClrImportant ; 	//说明对图象显示有重要影响的颜色索引的数目，如果是0，表示都重要。
}BMP_INFO_HEADER;

//彩色表
typedef __packed struct
{
    u8 rgbBlue ;    		//指定蓝色强度
    u8 rgbGreen ;      //指定绿色强度
    u8 rgbRed ;         //指定红色强度
    //u8 rgbReserved ;//保留，设置为0
}RGB_QUAD;

typedef __packed struct
{
        BMP_FILE_HEADER bmfHeader;	// 1.文件头信息块
        BMP_INFO_HEADER bmiHeader;	// 2.图像描述信息块
        //RGB_QUAD bmiColors[1];  			// 3.颜色表
        u8 Data[2];//add by karlno for align 4.
}BMP_INFO;
#endif //bmp info define end

//3	GUI 定义类型

//将24位RGB(888)色彩换算成16位RGB(565)色彩
#define ChColor(Color24) ((u16)((Color24>>8)&0xf800|(Color24>>5)&0x07e0|(Color24>>3)&0x001f)) 

#define USE_32BIT_COLOR_TYPE 0 // 1-用32位来记录颜色值，0-用16位记录颜色值
#if USE_32BIT_COLOR_TYPE
#define COLOR_TYPE u32 //定义颜色值类型
#define FatColor(Color) Color	//如果系统是32位记录色彩，那就没必要转换了
#else
#define COLOR_TYPE u16 //定义颜色值类型
#define FatColor ChColor //将标准的24位色彩换算成16位
#endif

//4	色彩位数说明
//4酷系统中传输的色彩值，如果USE_32BIT_COLOR_TYPE==1，则用u32来传递24bit的色彩值，COLOR_TYPE=u32
//4如果USE_32BIT_COLOR_TYPE==0，则用u16来传递16bit的色彩值，COLOR_TYPE=16，可节省ROM和RAM。
//4酷系统中需要赋色彩值的地方，应该用FatColor(0xffffff)来保证不管USE_32BIT_COLOR_TYPE取值是多少，
//4输入都是正确的。而对于用户而言，只需要使用FatColor，呈现在我们面前的永远都是0xffffff这样的24bit值，
//4方便阅读和从photoshop中取值。
//4ChColor和RGB2BGR都是被系统使用的，用户一般情况下无需触碰

#define NO_TRANS 0x00000f//用此值表示不使用透明色，转成16位色彩后是0x0001
#define TRANS_COR	0xfffff0//用此色彩代替表示透明色，转成16位色彩后是0xfffe
typedef struct { 
	u16 x; 	//区域相对左上角x
	u16 y;	//区域相对左上角y
	u16 w; 	//区域宽
	u16 h;	//区域高
	u16 Space;//用来指定间距，在写字时，4-7位表示字间距,0-3位表示行间距
	COLOR_TYPE Color;//用来指定色彩
}GUI_REGION,*pGUI_REGION;

typedef enum{
	Gui_True=0,
	Gui_False,
	Gui_No_Such_File,//没有读到文件
	Gui_Out_Of_Range//图片显示超出范围
}GUI_RESULT;

//2		GUI API   
//初始化GUI
void Gui_Init(void);

//设置背光
//Value范围0-100
void Gui_SetBgLight(u8 Value);

//读取背光亮度
//返回值范围0-100
u8 Gui_GetBgLightVal(void);

//设置横屏模式,注意成对使用
//Gui_SetLandScapeMode(TRUE);横屏
//Gui_SetLandScapeMode(FALSE);竖屏
GUI_RESULT Gui_SetLandScapeMode(bool LandScape);

//获取当前屏幕模式
//return TRUE - 横屏
//return FALSE - 竖屏
bool Gui_GetLandScapeMode(void);

//填充指定区域
//将pRegion的x y w h四个参数设置好即可
//Color表示填充色彩,0xffffff表示白色
GUI_RESULT Gui_FillBlock(const GUI_REGION *pRegion);

//用指定颜色填充整屏，如Gui_FillScreen(FatColor(0x000000))
GUI_RESULT Gui_FillScreen(COLOR_TYPE Color);

//用指定数组的内容单色填充区域
//数组每个元素的每个bit对应一个像素，8字节对齐
//pRegion->x ,y指定图像起点
//pRegion->w ,h指定图像宽高
GUI_RESULT Gui_PixelFill(const u8 *PixelBuf,const GUI_REGION *pRegion);

#if 0//请用Gui_DrawFont替代
//在指定区域内写字,超出则不显示
//将pRegion的所有参数都要设置,特别是space参数
//Str表示要写的字符串
//Color表示字体颜色
//字体为16x16
u16 Gui_DrawFont_GBK16(const u8 *Str,const GUI_REGION *pRegion);

//返回实际区域需要的字符串byte数
//返回值实际就是Gui_DrawFont_GBK的返回值，只不过不显示实际内容
u16 Gui_DrawFont_GBK16_Num(const u8 *Str,const GUI_REGION *pRegion);

//在指定区域内写字,超出则不显示
//将pRegion的所有参数都要设置,特别是space参数
//Str表示要写的字符串
//Color表示字体颜色
//字体为12x12
u16 Gui_DrawFont_GBK12(const u8 *Str,const GUI_REGION *pRegion);

//返回实际区域需要的字符串byte数
//返回值实际就是Gui_DrawFont_GBK的返回值，只不过不显示实际内容
u16 Gui_DrawFont_GBK12_Num(const u8 *Str,const GUI_REGION *pRegion);

//在指定区域内写字,超出则不显示
//将pRegion的所有参数都要设置,特别是space参数
//Str表示要写的字符串
//Color表示字体颜色
//字体为9x14,不支持中文
u16 Gui_DrawFont_ASC14B(const u8 *Str,const GUI_REGION *pRegion);

//返回实际区域需要的字符串byte数
//返回值实际就是Gui_DrawFont_GBK的返回值，只不过不显示实际内容
u16 Gui_DrawFont_ASC14B_Num(const u8 *Str,const GUI_REGION *pRegion);
#endif

//通用显示字符的函数
u16 Gui_DrawFont(FONT_ACT FontLib,const u8 *Str,const GUI_REGION *pRegion);

//读取指定路径的bmp图片长宽信息
//pBmpRegion的w h带回信息
GUI_RESULT Gui_ReadBmpInfo(const u8 * pBmpPath,u16 *Width,u16 *Hight);

//在pBmpRegion指定的x y点画指定路径的bmp图片
//pBmpRegion的w和h参数不需设置
//pBmpRegion的w和h会带回图片长宽信息
//bmp为24位bmp图片
GUI_RESULT Gui_Draw24Bmp(const u8 * pBmpPath,const GUI_REGION *pBmpRegion);

//将指定图片数组画到指定区域
//图片数组pImageBuf可用工具Image2Lcd获得
//Image2Lcd选项如下:
//  输出数据格式:c语言数组
//  扫描格式:水平扫描
//  输出灰度:16位真色彩,R 5bit, G 6bit, B 5Bit
//  软件左下方五个勾选框都不选
//pRegion需指定x y w h参数
GUI_RESULT Gui_DrawImgArray(const u8 * pImageBuf,const GUI_REGION *pRegion);

//将图片bin文件画到指定区域
//图片bin文件获取方法
//首先由Image2Lcd将要显示的图片转换成数组,如Gui_DrawImageArray
//然后用Text2Bin将数组转换成Bin文件,注意前四个字节不带长宽信息.
//将获取的Bin放到sd卡文件系统,路径赋给pPath即可
//pRegion需要指定x y w h参数
GUI_RESULT Gui_DrawImgBin(const u8 * pPath,const GUI_REGION *pRegion);

//同Gui_DrawImageBin,不过Bin文件存放在Spi Flash中
//Page 为存放起始页
//因为Spi Flash速度比较慢,此函数用处不大
GUI_RESULT Gui_DrawImgFlashBin(u32 Page,const GUI_REGION *pRegion);

//将一个数组格式的图片填充到指定区域
//pImageBuf通过Image2Lcd工具获取,请参照Gui_DrawImageArray函数
//pRegion需指定x y w h参数来指定填充区域
//width，hight指定图像宽高
GUI_RESULT Gui_FillImgArray(const u8 * pImageBuf,u16 width,u16 hight,const GUI_REGION *pRegion);

//将一个数组格式的图片横向填充到指定区域
//pImageBuf通过Image2Lcd工具获取,请参照Gui_DrawImageArray函数
//pRegion中x y 指定起点坐标
//pRegion中w h单元图片的长宽参数
GUI_RESULT Gui_FillImgArray_H(const u8 * pImageBuf,u16 width,const GUI_REGION *pRegion);

//将一个数组格式的图片纵向填充到指定区域
//pImageBuf通过Image2Lcd工具获取,请参照Gui_DrawImageArray函数
//pRegion中x y 指定起点坐标
//pRegion中w h单元图片的长宽参数
GUI_RESULT Gui_FillImgArray_V(const u8 * pImageBuf,u16 hight,const GUI_REGION *pRegion);

//将一个bmp图片从x y开始进行横向填充
//填充区宽度width必须是图片宽度的整数倍
//pRegion->x指定起点x坐标
//pRegion->y指定起点y坐标
//pRegion->w指定填充宽度
GUI_RESULT Gui_FillBmp_H(const u8 *pBmpPath,const GUI_REGION *pRegion);

//将一个bmp图片从x y开始进行纵向填充
//填充区宽度hight必须是图片高度的整数倍
//pRegion->x指定起点x坐标
//pRegion->y指定起点y坐标
//pRegion->w指定填充宽度
GUI_RESULT Gui_FillBmp_H(const u8 *pBmpPath,const GUI_REGION *pRegion);

//指定一个路径前缀,则将此路径前缀代表的三个bmp进行横向画框处理
//如pFramePathPrefix="DirName/Name"
//则会读取三个文件分别为"DirName/NameL.bmp","DirName/NameM.bmp","DirName/NameR.bmp"
//程序会将L.bmp画到最左边,R.bmp画到最右边,中间部分则用M.bmp进行横向填充
//所以width一定要大于三个图片的总宽度.
//bmp为24位bmp图片
//pRegion->x指定起点x坐标
//pRegion->y指定起点y坐标
//pRegion->w指定填充宽度
GUI_RESULT Gui_DrawFrame_H(const u8 *pFramePathPrefix,const GUI_REGION *pRegion);

//指定一个路径前缀,则将此路径前缀代表的三个bmp进行纵向画框处理
//如pFramePathPrefix="DirName/Name"
//则会读取三个文件分别为"DirName/NameT.bmp","DirName/NameM.bmp","DirName/NameB.bmp"
//程序会将T.bmp画到最上边,B.bmp画到最下边,中间部分则用M.bmp进行纵向填充
//所以hight一定要大于三个图片的总高度.
//bmp为24位bmp图片
//pRegion->x指定起点x坐标
//pRegion->y指定起点y坐标
//pRegion->h指定填充高度
GUI_RESULT Gui_DrawFrame_V(const u8 *pFramePathPrefix,const GUI_REGION *pRegion);

//根据指定的起点x0 y0和终点x1 y1画色彩为Color的线
//Color为24位,0xffffff为白色
//线宽为1
GUI_RESULT Gui_DrawLine(u16 x0, u16 y0,u16 x1, u16 y1,COLOR_TYPE Color);   

//根据指定的起点x0 y0和终点x1 y1画色彩为ColorBuf对应位置值的线
//ColorBuf为色彩信息数组,每一个成员代表一个点的色彩
//第1个点的色彩为ColorBuf[0],第2个点的色彩为ColorBuf[1],以此类推
//线宽为1
//需要和Gui_SaveColorLine成对使用
GUI_RESULT Gui_DrawColorLine(u16 x0, u16 y0,u16 x1, u16 y1,const u16 *ColorBuf);

//根据指定的起点x0 y0和终点x1 y1将每个点的色彩保存到ColorBuf
//ColorBuf为色彩信息数组,每一个成员代表一个点的色彩
//第1个点的色彩保存到ColorBuf[0],第2个点的色彩保存到ColorBuf[1],以此类推
//线宽为1
//需要和Gui_DrawColorLine成对使用
GUI_RESULT Gui_SaveColorLine(u16 x0, u16 y0,u16 x1, u16 y1,u16 *ColorBuf);

//画虚线线函数，同Gui_DrawLine
//Gap最高四个位指定线长
//Gap次高四个位指定空隙长
GUI_RESULT Gui_DrawDashed(u16 x0, u16 y0,u16 x1, u16 y1,u8 Gap,COLOR_TYPE Color);

GUI_RESULT Gui_DrawCircle(const GUI_REGION *pRegion,bool Cir);

//读区域函数,将指定区域每个点的色彩信息依次读到Buf中
//存到Buf中的色彩信息为24位,所以每个点占用3个字节
GUI_RESULT Gui_ReadRegion24Bit(u8 *Buf,const GUI_REGION *pRegion);

//读区域函数,将指定区域每个点的色彩信息依次读到Buf中
//存到Buf中的色彩信息为16位,所以每个点占用2个字节
GUI_RESULT Gui_ReadRegion16Bit(u16 *Buf,const GUI_REGION *pRegion);

//画一个点的色彩
void Gui_WritePixel(u16 x,u16 y,COLOR_TYPE Color);

//读指定点的色彩信息
//色彩信息为16位,由返回值传递
u16 Gui_ReadPixel16Bit(u16 x,u16 y);


#endif


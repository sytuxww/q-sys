#ifndef QSYS_Q_GUI_H
#define QSYS_Q_GUI_H

//Gui����
#define LCD_WIDTH	240//��Ļ��
#define LCD_HIGHT		320//��Ļ��
#define GUI_USE_FS_FONT_LIB 0 //ѡ���ֿ��Ƿ��ļ�ϵͳ���ǷŰ���flash��
#define GUI_BMP_COLOR_TABLE_BUF_SIZE 512*8  //���240��ͼƬ��Ҫ720byte,4kʱ�ٶ����

#if 0
//��ʾ���

#define LCD_DATA_BYTE 	2//�����lcdһ�����ص�����ݵ��ֽ���
#define BMP_RGB_BYTE 	3//bmpλͼ��һ�����ص���ֽ���
#define LCD_R_MASK 		0xf8
#define LCD_R_OFFSET 		3 //������
#define LCD_G_MASK 		0xfc
#define LCD_G_OFFSET 		3 //������
#define LCD_B_MASK			0xf8
#define LCD_B_OFFSET 		8 //������
#endif

//�ֿ�ѡ��
typedef enum{
	GBK12_FONT=0,//д12x12����
	GBK12_NUM,//���������ڿ����ɵ�12x12����
	GBK16_FONT,//д��
	GBK16_NUM,//����
	ASC14B_FONT,//д��
	ASC14B_NUM,//����
	GBK21_FONT,//д��
	GBK21_NUM,//����
}FONT_ACT;

//GBK�ֿⶨ��
#define GUI_GBK_FONT_OFFSET_H		0x81
#define GUI_GBK_FONT_OFFSET_L		0x40
#define GUI_GBK_FONT_OFFSET_P		0x7F
#define GUI_GBK_FONT_PAGENUM    	0xBE //=0xFE-0x40+1-1

//GBK16x16�ֿ�
#define GUI_GBK16_FONT_HEIGHT	16			//����߶���16
#define GUI_GBK16_CHAR_WIDTH	16			//�����ַ���16
#define GUI_GBK16_ASCII_WIDTH	8			//ascii�ַ���8
#define GUI_GBK16_FONT_BUF_SIZE 	32 //һ��������Ҫ���ٸ��ֽ����洢��=GUI_GBK_FONT_SIZE/8xGUI_GBK_FONT_SIZE
#define GUI_GBK16_FONT_SPI_FLASH_BASE		(682*256)// DOWNLOAD_START_PAGE*DOWNLOAD_PAGE_SIZE�洢�ֿ����ʼҳ
#define GUI_GBK16_FONT_END				(23940*GUI_GBK16_FONT_BUF_SIZE) //�ֿ������ ��Ascii�����

//GBK12x12�ֿ�
#define GUI_GBK12_FONT_HEIGHT	12		//����߶���12
#define GUI_GBK12_CHAR_WIDTH	12			//�����ַ���12
#define GUI_GBK12_ASCII_WIDTH	6			//ascii�ַ���6
#define GUI_GBK12_FONT_BUF_SIZE 	24 //һ��������Ҫ���ٸ��ֽ����洢��=GUI_GBK_FONT_SIZE/8xGUI_GBK_FONT_SIZE
#define GUI_GBK12_FONT_SPI_FLASH_BASE		(3691*256)// DOWNLOAD_START_PAGE*DOWNLOAD_PAGE_SIZE�洢�ֿ����ʼҳ
#define GUI_GBK12_FONT_END	(23940*GUI_GBK12_FONT_BUF_SIZE) //�ֿ������ ��Ascii�����

//ASCII 14 BOLD �ֿⶨ��
#define GUI_ASC14B_FONT_HEIGHT	14		//����߶���14
#define GUI_ASC14B_CHAR_WIDTH		15			//�����ַ���15��ʵ���ϴ��ֿⲻ֧�ֺ���
#define GUI_ASC14B_ASCII_WIDTH	8			//ascii�ַ���8
#define GUI_ASC14B_FONT_BUF_SIZE 	28 //һ������Ҫ���ٸ��ֽ����洢��=GUI_GBK_FONT_SIZE/8xGUI_GBK_FONT_SIZE
#define GUI_ASC14B_FONT_SPI_FLASH_BASE		(5948*256)// DOWNLOAD_START_PAGE*DOWNLOAD_PAGE_SIZE�洢�ֿ����ʼҳ

//GBK21x21�ֿ�
#define GUI_GBK21_FONT_HEIGHT	21		//����߶���12
#define GUI_GBK21_CHAR_WIDTH	21			//�����ַ���12
#define GUI_GBK21_ASCII_WIDTH	11			//ascii�ַ���6
#define GUI_GBK21_FONT_BUF_SIZE 	63 //һ��������Ҫ���ٸ��ֽ����洢��=GUI_GBK_FONT_SIZE/8xGUI_GBK_FONT_SIZE
#define GUI_GBK21_FONT_SPI_FLASH_BASE		(8192*256)// DOWNLOAD_START_PAGE*DOWNLOAD_PAGE_SIZE�洢�ֿ����ʼҳ
#define GUI_GBK21_FONT_END	0x170380//align 16 =(23940*GUI_GBK21_FONT_BUF_SIZE) //�ֿ������ ��Ascii�����

#if 0
//GB2312�ֿ�
#define GUI_FONT_GB2312_OFFSET_H			0xA1
#define GUI_FONT_GB2312_OFFSET_L			0xA1
#define GUI_FONT_GB2312_PAGENUM    		0x5E //=0xFE-0xA1+1
#define GUI_FONT_GB2312_SPACE					0		//���
#define GUI_FONT_GB2312_MARGIN				16		//�о�
#define GUI_FONT_GB2312_END						8836*128
#endif

#if 1//bmp info
//bmp�ļ����
// 1.�ļ�ͷ
// 2.��Ϣͷ
// 3.��ɫ��
typedef long	 LONG;
//BMPͷ�ļ�
typedef __packed struct
{
    u16  bfType ;     		//�ļ���־.ֻ��'BM',����ʶ��BMPλͼ����
    u32 bfSize ;           //�ļ���С,ռ�ĸ��ֽ�
    u16  bfReserved1 ;	//����
    u16  bfReserved2 ;	//����
    u32 bfOffBits ;  		//���ļ���ʼ��λͼ����(bitmap data)��ʼ֮��ĵ�ƫ����
}BMP_FILE_HEADER;

//BMP��Ϣͷ
typedef __packed struct 
{
    u32 biSize ;                   //˵��BITMAPINFOHEADER�ṹ����Ҫ��������
    LONG  biWidth ;                   //˵��ͼ��Ŀ�ȣ�������Ϊ��λ
    LONG  biHeight ;           		//˵��ͼ��ĸ߶ȣ�������Ϊ��λ
    u16  biPlanes ;           		//ΪĿ���豸˵��λ��������ֵ�����Ǳ���Ϊ1
    u16  biBitCount ;           	//˵��������/���أ���ֵΪ1��4��8��16��24����32
    u32 biCompression ;  	//˵��ͼ������ѹ�������͡���ֵ����������ֵ֮һ��
												// 0 :BI_RGB��û��ѹ����
												// 1:BI_RLE8��ÿ������8���ص�RLEѹ�����룬ѹ����ʽ��2�ֽ����(�ظ����ؼ�������ɫ����)��  
												// 2:BI_RLE4��ÿ������4���ص�RLEѹ�����룬ѹ����ʽ��2�ֽ����
												// 3:BI_BITFIELDS��ÿ�����صı�����ָ�������������
    u32 biSizeImage ;			//˵��ͼ��Ĵ�С�����ֽ�Ϊ��λ������BI_RGB��ʽʱ��������Ϊ0  
    LONG  biXPelsPerMeter ;	//˵��ˮƽ�ֱ��ʣ�������/�ױ�ʾ
    LONG  biYPelsPerMeter ;	//˵����ֱ�ֱ��ʣ�������/�ױ�ʾ
    u32 biClrUsed ;           	//˵��λͼʵ��ʹ�õĲ�ɫ���е���ɫ������
    u32 biClrImportant ; 	//˵����ͼ����ʾ����ҪӰ�����ɫ��������Ŀ�������0����ʾ����Ҫ��
}BMP_INFO_HEADER;

//��ɫ��
typedef __packed struct
{
    u8 rgbBlue ;    		//ָ����ɫǿ��
    u8 rgbGreen ;      //ָ����ɫǿ��
    u8 rgbRed ;         //ָ����ɫǿ��
    //u8 rgbReserved ;//����������Ϊ0
}RGB_QUAD;

typedef __packed struct
{
        BMP_FILE_HEADER bmfHeader;	// 1.�ļ�ͷ��Ϣ��
        BMP_INFO_HEADER bmiHeader;	// 2.ͼ��������Ϣ��
        //RGB_QUAD bmiColors[1];  			// 3.��ɫ��
        u8 Data[2];//add by karlno for align 4.
}BMP_INFO;
#endif //bmp info define end

//3	GUI ��������

//��24λRGB(888)ɫ�ʻ����16λRGB(565)ɫ��
#define ChColor(Color24) ((u16)((Color24>>8)&0xf800|(Color24>>5)&0x07e0|(Color24>>3)&0x001f)) 

#define USE_32BIT_COLOR_TYPE 0 // 1-��32λ����¼��ɫֵ��0-��16λ��¼��ɫֵ
#if USE_32BIT_COLOR_TYPE
#define COLOR_TYPE u32 //������ɫֵ����
#define FatColor(Color) Color	//���ϵͳ��32λ��¼ɫ�ʣ��Ǿ�û��Ҫת����
#else
#define COLOR_TYPE u16 //������ɫֵ����
#define FatColor ChColor //����׼��24λɫ�ʻ����16λ
#endif

//4	ɫ��λ��˵��
//4��ϵͳ�д����ɫ��ֵ�����USE_32BIT_COLOR_TYPE==1������u32������24bit��ɫ��ֵ��COLOR_TYPE=u32
//4���USE_32BIT_COLOR_TYPE==0������u16������16bit��ɫ��ֵ��COLOR_TYPE=16���ɽ�ʡROM��RAM��
//4��ϵͳ����Ҫ��ɫ��ֵ�ĵط���Ӧ����FatColor(0xffffff)����֤����USE_32BIT_COLOR_TYPEȡֵ�Ƕ��٣�
//4���붼����ȷ�ġ��������û����ԣ�ֻ��Ҫʹ��FatColor��������������ǰ����Զ����0xffffff������24bitֵ��
//4�����Ķ��ʹ�photoshop��ȡֵ��
//4ChColor��RGB2BGR���Ǳ�ϵͳʹ�õģ��û�һ����������败��

#define NO_TRANS 0x00000f//�ô�ֵ��ʾ��ʹ��͸��ɫ��ת��16λɫ�ʺ���0x0001
#define TRANS_COR	0xfffff0//�ô�ɫ�ʴ����ʾ͸��ɫ��ת��16λɫ�ʺ���0xfffe
typedef struct { 
	u16 x; 	//����������Ͻ�x
	u16 y;	//����������Ͻ�y
	u16 w; 	//�����
	u16 h;	//�����
	u16 Space;//����ָ����࣬��д��ʱ��4-7λ��ʾ�ּ��,0-3λ��ʾ�м��
	COLOR_TYPE Color;//����ָ��ɫ��
}GUI_REGION,*pGUI_REGION;

typedef enum{
	Gui_True=0,
	Gui_False,
	Gui_No_Such_File,//û�ж����ļ�
	Gui_Out_Of_Range//ͼƬ��ʾ������Χ
}GUI_RESULT;

//2		GUI API   
//��ʼ��GUI
void Gui_Init(void);

//���ñ���
//Value��Χ0-100
void Gui_SetBgLight(u8 Value);

//��ȡ��������
//����ֵ��Χ0-100
u8 Gui_GetBgLightVal(void);

//���ú���ģʽ,ע��ɶ�ʹ��
//Gui_SetLandScapeMode(TRUE);����
//Gui_SetLandScapeMode(FALSE);����
GUI_RESULT Gui_SetLandScapeMode(bool LandScape);

//��ȡ��ǰ��Ļģʽ
//return TRUE - ����
//return FALSE - ����
bool Gui_GetLandScapeMode(void);

//���ָ������
//��pRegion��x y w h�ĸ��������úü���
//Color��ʾ���ɫ��,0xffffff��ʾ��ɫ
GUI_RESULT Gui_FillBlock(const GUI_REGION *pRegion);

//��ָ����ɫ�����������Gui_FillScreen(FatColor(0x000000))
GUI_RESULT Gui_FillScreen(COLOR_TYPE Color);

//��ָ����������ݵ�ɫ�������
//����ÿ��Ԫ�ص�ÿ��bit��Ӧһ�����أ�8�ֽڶ���
//pRegion->x ,yָ��ͼ�����
//pRegion->w ,hָ��ͼ����
GUI_RESULT Gui_PixelFill(const u8 *PixelBuf,const GUI_REGION *pRegion);

#if 0//����Gui_DrawFont���
//��ָ��������д��,��������ʾ
//��pRegion�����в�����Ҫ����,�ر���space����
//Str��ʾҪд���ַ���
//Color��ʾ������ɫ
//����Ϊ16x16
u16 Gui_DrawFont_GBK16(const u8 *Str,const GUI_REGION *pRegion);

//����ʵ��������Ҫ���ַ���byte��
//����ֵʵ�ʾ���Gui_DrawFont_GBK�ķ���ֵ��ֻ��������ʾʵ������
u16 Gui_DrawFont_GBK16_Num(const u8 *Str,const GUI_REGION *pRegion);

//��ָ��������д��,��������ʾ
//��pRegion�����в�����Ҫ����,�ر���space����
//Str��ʾҪд���ַ���
//Color��ʾ������ɫ
//����Ϊ12x12
u16 Gui_DrawFont_GBK12(const u8 *Str,const GUI_REGION *pRegion);

//����ʵ��������Ҫ���ַ���byte��
//����ֵʵ�ʾ���Gui_DrawFont_GBK�ķ���ֵ��ֻ��������ʾʵ������
u16 Gui_DrawFont_GBK12_Num(const u8 *Str,const GUI_REGION *pRegion);

//��ָ��������д��,��������ʾ
//��pRegion�����в�����Ҫ����,�ر���space����
//Str��ʾҪд���ַ���
//Color��ʾ������ɫ
//����Ϊ9x14,��֧������
u16 Gui_DrawFont_ASC14B(const u8 *Str,const GUI_REGION *pRegion);

//����ʵ��������Ҫ���ַ���byte��
//����ֵʵ�ʾ���Gui_DrawFont_GBK�ķ���ֵ��ֻ��������ʾʵ������
u16 Gui_DrawFont_ASC14B_Num(const u8 *Str,const GUI_REGION *pRegion);
#endif

//ͨ����ʾ�ַ��ĺ���
u16 Gui_DrawFont(FONT_ACT FontLib,const u8 *Str,const GUI_REGION *pRegion);

//��ȡָ��·����bmpͼƬ������Ϣ
//pBmpRegion��w h������Ϣ
GUI_RESULT Gui_ReadBmpInfo(const u8 * pBmpPath,u16 *Width,u16 *Hight);

//��pBmpRegionָ����x y�㻭ָ��·����bmpͼƬ
//pBmpRegion��w��h������������
//pBmpRegion��w��h�����ͼƬ������Ϣ
//bmpΪ24λbmpͼƬ
GUI_RESULT Gui_Draw24Bmp(const u8 * pBmpPath,const GUI_REGION *pBmpRegion);

//��ָ��ͼƬ���黭��ָ������
//ͼƬ����pImageBuf���ù���Image2Lcd���
//Image2Lcdѡ������:
//  ������ݸ�ʽ:c��������
//  ɨ���ʽ:ˮƽɨ��
//  ����Ҷ�:16λ��ɫ��,R 5bit, G 6bit, B 5Bit
//  ������·������ѡ�򶼲�ѡ
//pRegion��ָ��x y w h����
GUI_RESULT Gui_DrawImgArray(const u8 * pImageBuf,const GUI_REGION *pRegion);

//��ͼƬbin�ļ�����ָ������
//ͼƬbin�ļ���ȡ����
//������Image2Lcd��Ҫ��ʾ��ͼƬת��������,��Gui_DrawImageArray
//Ȼ����Text2Bin������ת����Bin�ļ�,ע��ǰ�ĸ��ֽڲ���������Ϣ.
//����ȡ��Bin�ŵ�sd���ļ�ϵͳ,·������pPath����
//pRegion��Ҫָ��x y w h����
GUI_RESULT Gui_DrawImgBin(const u8 * pPath,const GUI_REGION *pRegion);

//ͬGui_DrawImageBin,����Bin�ļ������Spi Flash��
//Page Ϊ�����ʼҳ
//��ΪSpi Flash�ٶȱȽ���,�˺����ô�����
GUI_RESULT Gui_DrawImgFlashBin(u32 Page,const GUI_REGION *pRegion);

//��һ�������ʽ��ͼƬ��䵽ָ������
//pImageBufͨ��Image2Lcd���߻�ȡ,�����Gui_DrawImageArray����
//pRegion��ָ��x y w h������ָ���������
//width��hightָ��ͼ����
GUI_RESULT Gui_FillImgArray(const u8 * pImageBuf,u16 width,u16 hight,const GUI_REGION *pRegion);

//��һ�������ʽ��ͼƬ������䵽ָ������
//pImageBufͨ��Image2Lcd���߻�ȡ,�����Gui_DrawImageArray����
//pRegion��x y ָ���������
//pRegion��w h��ԪͼƬ�ĳ������
GUI_RESULT Gui_FillImgArray_H(const u8 * pImageBuf,u16 width,const GUI_REGION *pRegion);

//��һ�������ʽ��ͼƬ������䵽ָ������
//pImageBufͨ��Image2Lcd���߻�ȡ,�����Gui_DrawImageArray����
//pRegion��x y ָ���������
//pRegion��w h��ԪͼƬ�ĳ������
GUI_RESULT Gui_FillImgArray_V(const u8 * pImageBuf,u16 hight,const GUI_REGION *pRegion);

//��һ��bmpͼƬ��x y��ʼ���к������
//��������width������ͼƬ��ȵ�������
//pRegion->xָ�����x����
//pRegion->yָ�����y����
//pRegion->wָ�������
GUI_RESULT Gui_FillBmp_H(const u8 *pBmpPath,const GUI_REGION *pRegion);

//��һ��bmpͼƬ��x y��ʼ�����������
//��������hight������ͼƬ�߶ȵ�������
//pRegion->xָ�����x����
//pRegion->yָ�����y����
//pRegion->wָ�������
GUI_RESULT Gui_FillBmp_H(const u8 *pBmpPath,const GUI_REGION *pRegion);

//ָ��һ��·��ǰ׺,�򽫴�·��ǰ׺���������bmp���к��򻭿���
//��pFramePathPrefix="DirName/Name"
//����ȡ�����ļ��ֱ�Ϊ"DirName/NameL.bmp","DirName/NameM.bmp","DirName/NameR.bmp"
//����ὫL.bmp���������,R.bmp�������ұ�,�м䲿������M.bmp���к������
//����widthһ��Ҫ��������ͼƬ���ܿ��.
//bmpΪ24λbmpͼƬ
//pRegion->xָ�����x����
//pRegion->yָ�����y����
//pRegion->wָ�������
GUI_RESULT Gui_DrawFrame_H(const u8 *pFramePathPrefix,const GUI_REGION *pRegion);

//ָ��һ��·��ǰ׺,�򽫴�·��ǰ׺���������bmp�������򻭿���
//��pFramePathPrefix="DirName/Name"
//����ȡ�����ļ��ֱ�Ϊ"DirName/NameT.bmp","DirName/NameM.bmp","DirName/NameB.bmp"
//����ὫT.bmp�������ϱ�,B.bmp�������±�,�м䲿������M.bmp�����������
//����hightһ��Ҫ��������ͼƬ���ܸ߶�.
//bmpΪ24λbmpͼƬ
//pRegion->xָ�����x����
//pRegion->yָ�����y����
//pRegion->hָ�����߶�
GUI_RESULT Gui_DrawFrame_V(const u8 *pFramePathPrefix,const GUI_REGION *pRegion);

//����ָ�������x0 y0���յ�x1 y1��ɫ��ΪColor����
//ColorΪ24λ,0xffffffΪ��ɫ
//�߿�Ϊ1
GUI_RESULT Gui_DrawLine(u16 x0, u16 y0,u16 x1, u16 y1,COLOR_TYPE Color);   

//����ָ�������x0 y0���յ�x1 y1��ɫ��ΪColorBuf��Ӧλ��ֵ����
//ColorBufΪɫ����Ϣ����,ÿһ����Ա����һ�����ɫ��
//��1�����ɫ��ΪColorBuf[0],��2�����ɫ��ΪColorBuf[1],�Դ�����
//�߿�Ϊ1
//��Ҫ��Gui_SaveColorLine�ɶ�ʹ��
GUI_RESULT Gui_DrawColorLine(u16 x0, u16 y0,u16 x1, u16 y1,const u16 *ColorBuf);

//����ָ�������x0 y0���յ�x1 y1��ÿ�����ɫ�ʱ��浽ColorBuf
//ColorBufΪɫ����Ϣ����,ÿһ����Ա����һ�����ɫ��
//��1�����ɫ�ʱ��浽ColorBuf[0],��2�����ɫ�ʱ��浽ColorBuf[1],�Դ�����
//�߿�Ϊ1
//��Ҫ��Gui_DrawColorLine�ɶ�ʹ��
GUI_RESULT Gui_SaveColorLine(u16 x0, u16 y0,u16 x1, u16 y1,u16 *ColorBuf);

//�������ߺ�����ͬGui_DrawLine
//Gap����ĸ�λָ���߳�
//Gap�θ��ĸ�λָ����϶��
GUI_RESULT Gui_DrawDashed(u16 x0, u16 y0,u16 x1, u16 y1,u8 Gap,COLOR_TYPE Color);

GUI_RESULT Gui_DrawCircle(const GUI_REGION *pRegion,bool Cir);

//��������,��ָ������ÿ�����ɫ����Ϣ���ζ���Buf��
//�浽Buf�е�ɫ����ϢΪ24λ,����ÿ����ռ��3���ֽ�
GUI_RESULT Gui_ReadRegion24Bit(u8 *Buf,const GUI_REGION *pRegion);

//��������,��ָ������ÿ�����ɫ����Ϣ���ζ���Buf��
//�浽Buf�е�ɫ����ϢΪ16λ,����ÿ����ռ��2���ֽ�
GUI_RESULT Gui_ReadRegion16Bit(u16 *Buf,const GUI_REGION *pRegion);

//��һ�����ɫ��
void Gui_WritePixel(u16 x,u16 y,COLOR_TYPE Color);

//��ָ�����ɫ����Ϣ
//ɫ����ϢΪ16λ,�ɷ���ֵ����
u16 Gui_ReadPixel16Bit(u16 x,u16 y);


#endif


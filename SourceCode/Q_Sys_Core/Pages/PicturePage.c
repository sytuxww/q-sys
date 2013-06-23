/**************** Q-OS *******************
 * PageName : 
 * Author : YuanYin
 * Version : 
 * Description :
 * WebSite : Www.Q-ShareWe.Com
 ***************** Q-OS *******************/
#include "User.h"
#include "PicturePage.h"
#include "FileListPage.h"

static SYS_MSG SystemEventHandlerPicturePage(SYS_EVT SysEvent ,int IntParam, void *pSysParam);
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam);
static CO_MSG TouchEventHandlerPicturePage(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo);

//-----------------------本页系统变量及声明-----------------------
enum PictureOID
{
	PicturePage_ExtiKey0=0,//系统默认将外部中断按键发送到第一个键值

	PicturePage_Open=USER_KEY_VALUE_START,
	PicturePage_Return,
	PicturePage_Pre,
	PicturePage_Next
};

//定义页面或应用的触摸区域集，相当于定义按键
static const IMG_BUTTON_OBJ PicturePageTouchRegionSet[]={
	//{key,gLandScapeMode,x,y,width,hight,image x,image y,normal bmp path,release bmp path,press bmp path,transparent color,key name}
	{"Open",PicturePage_Open,RelMsk,0,0,240,60,0,0,"",0},
	{"Return",PicturePage_Return,RelMsk,0,260,240,60,0,0,"",0},
	{"<<",PicturePage_Pre,RelMsk,0,60,120,200,0,0,"",0},
	{">>",PicturePage_Next,RelMsk,120,60,120,200,0,0,"",0}
};

//定义页面或者应用的属性集
const PAGE_ATTRIBUTE PicturePage={
	"PicturePage",
	"YuanYin",
	"Picture page",
	NORMAL_PAGE,
	0,

	{
		sizeof(PicturePageTouchRegionSet)/sizeof(IMG_BUTTON_OBJ),
		0,
	},
	PicturePageTouchRegionSet,
	NULL,

	SystemEventHandlerPicturePage,
	PeripheralsHandler,
	Bit(Perip_UartInput),
	TouchEventHandlerPicturePage,
	
};

//-----------------------本页自定义变量声明-----------------------
#define MAX_PICTURE_NUM 256
#define PICTURE_DIR "/Picture"
#define PICTURE_SUFFIX_STR ".bmp"


static const char PictureHelp[]="图片浏览程序帮助:\n\r1.本程序暂只支持320x240以内的24位Bmp格式图片，请将图片放置在TF卡根目录Picture文件夹下。\n\r2.点击上方60个像素的区域为打开图片文件列表，点击下方60个像素的区域为返回到主界面，点击左侧区域为显示上一图，点击右侧区域为显示下一图。\n\r3.直接在超级终端输入字符串，敲击回车后，自动打开对应图片。按电脑前后键可控制图片播放。";
static u8 *gpPicPath;//当前图片路径


//-----------------------本页自定义函数-----------------------
static void DispPic(void)
{
	GUI_REGION DrawRegion;
	u16 i;

	Debug("Display Picture:%s\n\r",gpPicPath);
	
	for(i=strlen((void *)gpPicPath);i;i--) if(gpPicPath[i]=='/')break;
	i++;
				
	DrawRegion.x=0;
	DrawRegion.y=0;
	DrawRegion.w=240;
	DrawRegion.h=320;
	DrawRegion.Color=FatColor(NO_TRANS);
	if(Gui_Draw24Bmp(gpPicPath,&DrawRegion)!=Gui_True) return;

	//draw title
	DrawRegion.w=234;
	DrawRegion.h=314;
	DrawRegion.Space=0x36;

	DrawRegion.x=2;
	DrawRegion.y=1;
	DrawRegion.Color=FatColor(0xffffff);
	Gui_DrawFont(GBK12_FONT,&gpPicPath[i],&DrawRegion);

	DrawRegion.x=2;
	DrawRegion.y=3;
	DrawRegion.Color=FatColor(0xffffff);
	Gui_DrawFont(GBK12_FONT,&gpPicPath[i],&DrawRegion);

	DrawRegion.x=1;
	DrawRegion.y=2;
	DrawRegion.Color=FatColor(0xffffff);
	Gui_DrawFont(GBK12_FONT,&gpPicPath[i],&DrawRegion);

	DrawRegion.x=3;
	DrawRegion.y=2;
	DrawRegion.Color=FatColor(0xffffff);
	Gui_DrawFont(GBK12_FONT,&gpPicPath[i],&DrawRegion);
	
	DrawRegion.x=2;
	DrawRegion.y=2;
	DrawRegion.Color=FatColor(0);
	Gui_DrawFont(GBK12_FONT,&gpPicPath[i],&DrawRegion);

	return;		
}

//-----------------------本页系统函数----------------------
//在每次本页面变成系统当前页面时会调用的程序
static SYS_MSG SystemEventHandlerPicturePage(SYS_EVT SysEvent ,int IntParam, void *pSysParam)
{
	GUI_REGION DrawRegion;
	
	switch(SysEvent)
	{
		case Sys_PageInit:	
			gpPicPath=(u8 *)Q_PageMallco(MAX_PATH_LEN);

			if(gpPicPath==0)
			{
				Q_ErrorStopScreen("gpPicPath malloc fail !\n\r");
			}

			sprintf((void *)gpPicPath,PICTURE_DIR);
		case Sys_SubPageReturn:
			DrawRegion.x=0;
			DrawRegion.y=0;
			DrawRegion.w=240;
			DrawRegion.h=320;
			DrawRegion.Color=FatColor(0x8b8a8a);
			Gui_FillBlock(&DrawRegion);
			
			DrawRegion.x=4;
			DrawRegion.y=4;
			DrawRegion.w=232;
			DrawRegion.h=320;
			DrawRegion.Space=0x22;
			DrawRegion.Color=FatColor(0xffffff);
			Gui_DrawFont(GBK16_FONT,(void *)PictureHelp,&DrawRegion);
			break;
		case Sys_TouchSetOk:
		case Sys_TouchSetOk_SR:
			if(IntParam==TRUE)//从子页面返回
			{
				if(CheckMultiSuffix(gpPicPath,PICTURE_SUFFIX_STR))
				{//如果是文件，立即显示
					DispPic();
				}
			}
			break;
		case Sys_PageClean:
			Q_PageFree(gpPicPath);
			break;
		/*

		*/
	}
	
	return 0;
}
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam)
{
	FILELIST_SET FLPS;
	switch(PeripEvent)
	{
		case Perip_KeyPress:
			switch(IntParam){
				case PicturePage_ExtiKey0:
					PrtScreen();
					break;
			}break;
		case Perip_UartInput:
		if((IntParam>>16)==1)//串口1
		{
			IntParam&=0xffff;
			if((IntParam==0)&&(((u16 *)pParam)[0]==0x445b)) //previous one
			{
				//Debug("Now:%s\n\r",gpPicPath);
				FLPS.CallBackRid=Q_FindRidByPageName(NULL);
				FLPS.pPathBuf=gpPicPath;
				FLPS.pSuffixStr=PICTURE_SUFFIX_STR;
				if((Q_GotoPage(GotoSubPage,"FileListPage",FL_FindPrevious|FL_NoListBuf,&FLPS)&SM_STATE_MASK)==SM_State_OK)
				{
					if(CheckMultiSuffix(gpPicPath,PICTURE_SUFFIX_STR))
					{
						//Debug("Prev:%s\n\r",gpPicPath);
					}
					else
					{
						if((Q_GotoPage(GotoSubPage,"FileListPage",FL_FindLast|FL_NoListBuf,&FLPS)&SM_STATE_MASK)==SM_State_OK)
						{
							//Debug("Last:%s\n\r",gpPicPath);
						}
					}
					DispPic();
				}			
			}
			else if((IntParam==0)&&(((u16 *)pParam)[0]==0x435b)) //next one
			{
				//Debug("Now:%s\n\r",gpPicPath);
				FLPS.CallBackRid=Q_FindRidByPageName(NULL);
				FLPS.pPathBuf=gpPicPath;
				FLPS.pSuffixStr=PICTURE_SUFFIX_STR;
				if((Q_GotoPage(GotoSubPage,"FileListPage",FL_FindNext|FL_NoListBuf,&FLPS)&SM_STATE_MASK)==SM_State_OK)
				{
					if(CheckMultiSuffix(gpPicPath,PICTURE_SUFFIX_STR))
					{
						//Debug("Next:%s\n\r",gpPicPath);
					}
					else
					{
						if((Q_GotoPage(GotoSubPage,"FileListPage",FL_FindFrist|FL_NoListBuf,&FLPS)&SM_STATE_MASK)==SM_State_OK)
						{
							//Debug("First:%s\n\r",gpPicPath);
						}
					}
					DispPic();
				}			
			}
			else
			{
				sprintf((void *)gpPicPath,"Picture/%s.bmp",(u8 *)pParam);
				DispPic();
			}
		}

		break;
	}

	return 0;
}
static CO_MSG TouchEventHandlerPicturePage(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo)
{	
	FILELIST_SET FLPS;
	
	if(InEvent!=Tch_Release) return 0;
	
	switch(Key)
	{
		
		case PicturePage_Open:
			if(InEvent!=Tch_Release) return 0;
			{
				FLPS.CallBackRid=Q_FindRidByPageName(NULL);
				sprintf((void *)gpPicPath,PICTURE_DIR);
				FLPS.pPathBuf=gpPicPath;
				FLPS.pSuffixStr=PICTURE_SUFFIX_STR;
				Q_GotoPage(GotoSubPage,"FileListPage",FL_SelectOne|FL_NoParent|FL_NoListBuf,&FLPS);
			}
			break;
		case PicturePage_Return:	
			Q_GotoPage(GotoNewPage,"MainPage",0,NULL);
			break;
		case PicturePage_Pre:
			if(InEvent!=Tch_Release) return 0;
			{
				Debug("Now:%s\n\r",gpPicPath);
				FLPS.CallBackRid=Q_FindRidByPageName(NULL);
				FLPS.pPathBuf=gpPicPath;
				FLPS.pSuffixStr=PICTURE_SUFFIX_STR;
				if((Q_GotoPage(GotoSubPage,"FileListPage",FL_FindPrevious|FL_NoListBuf,&FLPS)&SM_STATE_MASK)==SM_State_OK)
				{
					if(CheckMultiSuffix(gpPicPath,PICTURE_SUFFIX_STR))
					{
						Debug("Prev:%s\n\r",gpPicPath);
					}
					else
					{
						if((Q_GotoPage(GotoSubPage,"FileListPage",FL_FindLast|FL_NoListBuf,&FLPS)&SM_STATE_MASK)==SM_State_OK)
						{
							Debug("Last:%s\n\r",gpPicPath);
						}
					}
					DispPic();
				}			
			}
			break;
		case PicturePage_Next:
			if(InEvent!=Tch_Release) return 0;
			{
				Debug("Now:%s\n\r",gpPicPath);
				FLPS.CallBackRid=Q_FindRidByPageName(NULL);
				FLPS.pPathBuf=gpPicPath;
				FLPS.pSuffixStr=PICTURE_SUFFIX_STR;
				if((Q_GotoPage(GotoSubPage,"FileListPage",FL_FindNext|FL_NoListBuf,&FLPS)&SM_STATE_MASK)==SM_State_OK)
				{
					if(CheckMultiSuffix(gpPicPath,PICTURE_SUFFIX_STR))
					{
						Debug("Next:%s\n\r",gpPicPath);
					}
					else
					{
						if((Q_GotoPage(GotoSubPage,"FileListPage",FL_FindFrist|FL_NoListBuf,&FLPS)&SM_STATE_MASK)==SM_State_OK)
						{
							Debug("First:%s\n\r",gpPicPath);
						}
					}
					DispPic();
				}			
			}
			break;			
	}
	
	return 0;
}




/**************** Q-OS *******************
 * PageName : 
 * Author : YuanYin
 * Version : 
 * Description :
 * WebSite : Www.Q-ShareWe.Com
 ***************** Q-OS *******************/
#include "User.h"
#include "Theme.h"
#include "FileListPage.h"

static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,FILELIST_CMD Cmd, FILELIST_SET *pFLS);
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam);
static CO_MSG ButtonHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo);

//-----------------------本页系统变量及声明-----------------------
typedef enum 
{
	ExtiKeyDown=EXTI_KEY_VALUE_START,//系统默认将外部中断按键发送到第一个键值
	ExtiKeyUp,
	ExtiKeyEnter,

	NumKV=USER_KEY_VALUE_START,
	UpKV,
	DownKV,
	ParentDirKV,
	EnterKV,
	ReturnKV
}FileListPage_OID;

//定义页面或应用的触摸区域集，相当于定义按键
static const IMG_BUTTON_OBJ ImgButtonCon[]={
	//{key,gLandScapeMode,x,y,width,hight,image x,image y,normal bmp path,release bmp path,press bmp path,transparent color,key name}
	{"",NumKV,PrsMsk|CotMsk|RelMsk,0,26,240,260,0,0,"",0},
	{"Return",ReturnKV,RelMsk|PathMsk,4,287,39,31,0,0,"Common/Btn/Return",FatColor(NO_TRANS)},
	{"Up",UpKV,RelMsk|PathMsk,52,287,39,31,0,0,"Common/Btn/Up",FatColor(NO_TRANS)},
	{"Down",DownKV,RelMsk|PathMsk,100,287,39,31,0,0,"Common/Btn/Down",FatColor(NO_TRANS)},
	{"ParentDir",ParentDirKV,RelMsk|PathMsk,148,287,39,31,0,0,"Common/Btn/Parent",FatColor(NO_TRANS)},
	{"Enter",EnterKV,RelMsk|PathMsk,196,287,39,31,0,0,"Common/Btn/Yes",FatColor(NO_TRANS)},
};

//定义页面或者应用的属性集
const PAGE_ATTRIBUTE FileListPage={
	"FileListPage",
	"YuanYin",
	"Filelist page",
	NORMAL_PAGE,
	0,
	
	{
		sizeof(ImgButtonCon)/sizeof(IMG_BUTTON_OBJ), //size of touch region array
		0,//sizeof(CharButtonCon)/sizeof(CHAR_BUTTON_OBJ), //size of touch region array,
	},
	ImgButtonCon,
	NULL,
	
	(SystemHandlerFunc)SystemEventHandler,
	PeripheralsHandler,
	Bit(Perip_KeyPress)|Bit(Perip_KeyRelease)|Bit(Perip_UartInput),
	ButtonHandler,

};

//-----------------------本页自定义变量声明-----------------------
#define PAGE_DISPALY_NUM 10	//每页最多显示行数
#define MAX_SUB_DIR_NUM 256//支持的最大数目子目录数目
#define MAX_FILE_NUM 1024//支持的最大数目子文件数目
#define EVERY_LIST_LEN 64	//list.buf中每条list占用的字节数

//显示有关的宏定义
#define DISPLAY_START_X 4+20 //显示列表项目名的x轴起点，不算图标部分
#define DISPLAY_START_Y 21 //显示列表的y轴起点
#define DISPLAY_WIDTH 230-24 //显示项目名列表的宽度,不含图标部分
#define ROW_HIGHT 26	//行高
#define FONT_HIGHT 16 //字高
#define FONT_TOP_MARGIN 5 //字符顶部旁白
#define FONT_SPACE 0x10;// 4-7位表字间距，0-3位无效
#define ICON_WIDTH 16 //icon 宽度
#define ICON_HEIGHT 18 //icon 宽度
#define ICON_RIGHT_MARGIN 4 //icon离文字距离
#define ICON_TOP_MARGIN 4 //icon离顶部距离

#define BG_COLOR_1 0xdddddd
#define BG_COLOR_2 0xeeeeee
#define ICON_TRAN_COLOR 0xff0000	//图标透明色

#define FILE_NAME_COLOR	0x666666//文件名色彩
#define FILE_SELECT_COLOR 0x54920d	//被选择的文件名色彩

#define DIR_NAME_COLOR	0x666666//目录名色彩
#define DIR_SELECT_COLOR		0xff9000//被选择的目录名色彩

//List Buf
#define LIST_BUF_NAME "/list.buf"

//全局变量
typedef struct{
	FILELIST_CMD Cmd;//用来记录本次调用的命令
	bool UseListBuf;
	union{
		struct{
			u8 DirIndex[MAX_SUB_DIR_NUM];//当前目录下的子目录索引,最大只支持256个子目录
			u8 FileIndex[MAX_FILE_NUM];//当前目录下的文件索引,最大只支持256个文件
			u16 CurFileTotal;//当前目录下的匹配文件总数	
			u8 CurSubDirTotal; //当前目录下的子目录个数
		}N;//No List Buf
		struct{
			u16 Header[EVERY_LIST_LEN>>1];//list buf的文件头
			u8 RecordH[EVERY_LIST_LEN*PAGE_DISPALY_NUM*2];// 2页的缓存量
			u8 *pRecord;
			//u8 CurTotal;//当前目录下的匹配文件文件夹总数
		}L;//List Buf
	}Unit;
	u8 *pPath;//用来记录当前目录的绝对路径
	u8 CurSelectName[MAX_LEN_FILENAME];//用来记录当前选择的名字
	u8 SuffixStr[64];//用来记录后缀名字符串
	
	u32 CallerPageRid;//记录要返回的页面
	u16 CurStartNum;//当前显示的起始项
	u16 CurTotal;//=CurFileTotal+CurSubDirTotal,文件夹和文件总数
	u8 CurSelect;//当前选择项,设置为0xff表示没选择任何项
	u8 RootPathLen;//初始目录的路径长度

	//文件系统
	struct dirent  *pd;
	struct stat stat_buf;
	FS_FILE *fp;
	FS_DIR *dirp;
}FLIELIST_PAGE_VARS;//将本页要用到的全局变量全部放入此结构体
static FLIELIST_PAGE_VARS *gpFpVars;//只需要定义一个指针，减少全局变量的使用

//文件系统
typedef struct {//sizeof=680
	struct dirent  *pd;
	struct stat stat_buf;
	FS_DIR *dirp;
}FS_VARS;
//-----------------------本页自定义函数-----------------------
//从一个地址开始的指定Bit位置高
static void SetArrayBit(u16 *pArray,u16 Bit,u8 Value)
{
	u16 i=Bit>>4;

	while(Bit>=16) Bit-=16;

	if(Value) SetBit(pArray[i],Bit);
	else ClrBit(pArray[i],Bit);
}

static bool GetArrayBit(u16 *pArray,u16 Bit)
{
	u16 i=Bit>>4;

	while(Bit>=16) Bit-=16;

	if(ReadBit(pArray[i],Bit)) return TRUE;
	else return FALSE;
}

static bool RecordOneName(FS_FILE *fp,u8 *Name,u8 NameLen,bool IsDir)
{
	unsigned int WriteByte;
	u8 Buf[EVERY_LIST_LEN];
	u8 *pBuf=Buf;
	
	if(NameLen>=EVERY_LIST_LEN) return FALSE;

	if(IsDir==TRUE)//加入目录标识符
	{
		*pBuf++='/';
	}
	
	for(WriteByte=0;WriteByte<NameLen;WriteByte++)
	{
		*pBuf++=Name[WriteByte];
	}
	*pBuf=0;
	
	WriteByte =FS_FWrite(Buf, EVERY_LIST_LEN, 1,fp);		
	if(WriteByte!=sizeof(Buf))
	{
		Q_ErrorStopScreen("Write file error!\n\r");
	}
	
	return TRUE;		
}

static bool FileIsExist(u8 *pPath)
{
	FS_FILE * FObj;

	FObj = FS_FOpen((void *)pPath,FA_OPEN_EXISTING);

	if(FObj!=0)
	{
		FS_FClose(FObj);
		return TRUE;
	}
	else 
	{
		return FALSE;
	}	
}

//在当前目录下新建列表缓存
//返回TRUE表示新建了一个
//返回FALSE表示没有建立新的
//Force表示是否强行重建而不理会已存在的
//pShuffixStr指定要匹配的文件后缀
static bool CreatListBuf(FLIELIST_PAGE_VARS *pVars,bool Force)
{
//	FRESULT Res;
	unsigned int WriteByte;
	u16 i;

	//打开要读的文件夹
	pVars->dirp = FS_OpenDir((void *)pVars->pPath);

	if (pVars->dirp==0)
	{
		Debug("CLB:Open Dir %s error !\n\r",pVars->pPath);			
		return FALSE;
	}

	//建立缓存文件
	i=strlen((void *)pVars->pPath);
	strcat((void *)pVars->pPath,LIST_BUF_NAME);
	
	if((FileIsExist(pVars->pPath)==FALSE)||(Force==TRUE))
	{
		pVars->fp=FS_FOpen((void *)pVars->pPath,FA_CREATE_ALWAYS | FA_WRITE);

		if (pVars->fp==0) 
		{
			Debug("Open txt flie error :%s\n\r",pVars->pPath);
			FS_CloseDir(pVars->dirp);
			return FALSE;
		}
		pVars->pPath[i]=0;

    	for(i=0;i<(EVERY_LIST_LEN>>1);i++) pVars->Unit.L.Header[i]=0;//清空头缓存
		WriteByte=FS_FWrite((void *)pVars->Unit.L.Header, EVERY_LIST_LEN, 1,pVars->fp);
		if(WriteByte!=EVERY_LIST_LEN)
		{
			Debug("CLB:Write file error!%d\n\r",WriteByte);
			Q_ErrorStopScreen("CLB:Write file error!\n\r");
		}

		while( ( pVars->pd=FS_ReadDir(pVars->dirp) ) != 0 )
		{
			if (pVars->pd->d_name[0]==0)
			{
				break;			
			}

			if(S_ISDIR(pVars->pd->st_mode))
			{
					//Debug("**dir %s,%s\n\r",finfo.lfname,finfo.fname);
				pVars->Unit.L.Header[0]++;//文件计数
				SetArrayBit(&(pVars->Unit.L.Header[1]),pVars->Unit.L.Header[0],0);//目录记录为0
				RecordOneName(pVars->fp,(u8 *)pVars->pd->d_name,strlen(pVars->pd->d_name),TRUE);		
			}
			else
			{
				if(CheckMultiSuffix((void *)pVars->pd->d_name,pVars->SuffixStr)) 

				{
					//Debug("**file:%s,%s\n\r",finfo.lfname,finfo.fname);	
					pVars->Unit.L.Header[0]++;//文件计数
					SetArrayBit(&(pVars->Unit.L.Header[1]),pVars->Unit.L.Header[0],1);//文件记录为1
					RecordOneName(pVars->fp,(u8 *)pVars->pd->d_name,strlen(pVars->pd->d_name),FALSE);		
				}
			}
		}


		FS_FSeek(pVars->fp,0,FS_SEEK_SET);

		WriteByte=FS_FWrite((void *)pVars->Unit.L.Header, EVERY_LIST_LEN, 1,pVars->fp);
		if(WriteByte!=EVERY_LIST_LEN)
		{
			Debug("CLB2:Write file error!%d\n\r",WriteByte);
			Q_ErrorStopScreen("CLB2:Write file error!\n\r");
		}

		FS_FClose(pVars->fp);
		FS_CloseDir(pVars->dirp);

		return TRUE;
	}
	else//原来就有此文件，不动它
	{
		return FALSE;
	}
}

static bool PrefetchDirByList(FLIELIST_PAGE_VARS *pVars)
{
	unsigned int ReadByte;
	u16 i;

	pVars->CurTotal=0;
	pVars->CurStartNum=0;//当前显示的起始项
	pVars->CurSelect=0;//当前选择项
	
	i=strlen((void *)pVars->pPath);
	strcat((void *)pVars->pPath,LIST_BUF_NAME);

	if ((pVars->fp=FS_FOpen((void *)pVars->pPath,FA_OPEN_EXISTING | FA_READ))==0  ) 
	{
		Debug("Open txt flie error !:%s\n\r",pVars->pPath);
		return FALSE;
	}
	pVars->pPath[i]=0;

	#if 0
	if(f_lseek(&ListObj,GetSum(gpEBookLocal,Page))!=FR_OK)
	{
		Debug("Txt fseek err!\n");
		return;
	}	
	#endif
	
	ReadByte=FS_FRead(pVars->Unit.L.Header, EVERY_LIST_LEN,1, pVars->fp);
	if(ReadByte==EVERY_LIST_LEN)
    {
    	Debug("header:");DisplayBuf((u8 *)pVars->Unit.L.Header,64,16);
    	pVars->CurTotal=pVars->Unit.L.Header[0];
		return TRUE;
    }
    else
    {
		return FALSE;
    }
}

//pPath必须是不以斜杠结尾的文件夹路径
//预处理目录，不依靠list.buf文件
static bool PrefetchDir(FLIELIST_PAGE_VARS *pVars)
{
//	FRESULT res;
	u16 i;

	pVars->Unit.N.CurSubDirTotal=0; //当前目录下的子目录个数
	pVars->Unit.N.CurFileTotal=0;//当前目录下的匹配文件个数
	pVars->CurTotal=0;
	pVars->CurStartNum=0;//当前显示的起始项
	pVars->CurSelect=0;//当前选择项
	for(i=0;i<MAX_SUB_DIR_NUM;i++)
	{
		pVars->Unit.N.DirIndex[i]=0;
	}
	for(i=0;i<MAX_FILE_NUM;i++)
	{
		pVars->Unit.N.FileIndex[i]=0;
	}

	//打开要读的文件夹
	if ((pVars->dirp=FS_OpenDir((void *)pVars->pPath))== 0) 
	{
		Debug("PFD:Open Dir %s error !\n\r",pVars->pPath);			
		return FALSE;
	}
			
	for(i=0;(pVars->pd=FS_ReadDir(pVars->dirp)) != 0;i++)  
	{
		if(pVars->pd->d_name[0]==0) break;//读完文件退出

		//Debug("scan file:%s\r\n",pVars->pd->d_name);

		if(S_ISDIR(pVars->pd->st_mode)) //读到的是目录
		{
			//Debug("**dir %s,%s\n\r",finfo.lfname,finfo.fname);
			pVars->Unit.N.DirIndex[pVars->Unit.N.CurSubDirTotal]=i;
			pVars->Unit.N.CurSubDirTotal++;
		}
		else  //读到的是文档
		{
			if(CheckMultiSuffix((void *)pVars->pd->d_name,pVars->SuffixStr)) 
			{
				//Debug("**file:%s,%s\n\r",finfo.lfname,finfo.fname);	
				pVars->Unit.N.FileIndex[pVars->Unit.N.CurFileTotal]=i;
				pVars->Unit.N.CurFileTotal++;
			}
		}
				
		if((pVars->Unit.N.CurFileTotal>=MAX_FILE_NUM)||(pVars->Unit.N.CurSubDirTotal>=MAX_SUB_DIR_NUM)) 
		{//本目录文件超过了最大支持的数目
			Debug("Files and subdirs number is most than max support number!\n\r");
			break;
		}
	}

	FS_CloseDir(pVars->dirp);

	pVars->CurTotal=pVars->Unit.N.CurFileTotal+pVars->Unit.N.CurSubDirTotal;//算总数

	Debug("CurToal = %d %d %d!\r\n",pVars->CurTotal,pVars->Unit.N.CurFileTotal,pVars->Unit.N.CurSubDirTotal);

	return TRUE;
}

//显示一个文件夹的当前文件
//返回FALSE表示显示完了
static bool DisplayFiles(FLIELIST_PAGE_VARS *pVars)
{
//	FRESULT Res;
	GUI_REGION DrawRegion,IconDrawReg;
	u8 Row=0;
	u16 i;	

	pVars->CurSelect=0xff;
	pVars->CurSelectName[0]=0;
	
	//显示路径
	if(pVars->pPath[0])
	{
		for(i=strlen((void *)pVars->pPath);i;i--) if(pVars->pPath[i]=='/') break;
		sprintf((void *)pVars->pd->d_name,"%s: %d / %d",&pVars->pPath[++i],pVars->CurStartNum/PAGE_DISPALY_NUM+1,(pVars->CurTotal-1)/PAGE_DISPALY_NUM+1);
	}
	else 
		sprintf((void *)pVars->pd->d_name,"./: %d / %d",pVars->CurStartNum/PAGE_DISPALY_NUM+1,(pVars->CurTotal-1)/PAGE_DISPALY_NUM+1);
	DrawTitle1(GBK12_FONT,(void *)pVars->pd->d_name,4,29,FatColor(0xe0e0e0));
	DrawRegion.x=DISPLAY_START_X;
	DrawRegion.y=DISPLAY_START_Y+FONT_TOP_MARGIN;
	DrawRegion.w=DISPLAY_WIDTH;
	DrawRegion.h=ROW_HIGHT-FONT_TOP_MARGIN;
	DrawRegion.Space=FONT_SPACE;

	IconDrawReg.x=DISPLAY_START_X-ICON_WIDTH-ICON_RIGHT_MARGIN;
	IconDrawReg.y=DISPLAY_START_Y+ICON_TOP_MARGIN;
	IconDrawReg.w=ICON_WIDTH;
	IconDrawReg.h=ICON_HEIGHT;

	if ((pVars->dirp=FS_OpenDir((void *)pVars->pPath) )== 0) 
	{
		Debug("DFL:Open Dir %s error !\n\r",pVars->pPath);			
		return FALSE;
	}

	for(Row=0;Row<PAGE_DISPALY_NUM;Row++,DrawRegion.y+=ROW_HIGHT,IconDrawReg.y+=ROW_HIGHT)
	{
		if((pVars->CurStartNum+Row)>=pVars->CurTotal)
		{//没有文件部分
			//画底色
			if(Row&0x01)
			{
				DrawRegion.Color=FatColor(BG_COLOR_1);
				IconDrawReg.Color=FatColor(BG_COLOR_1);
			}
			else
			{
				DrawRegion.Color=FatColor(BG_COLOR_2);
				IconDrawReg.Color=FatColor(BG_COLOR_2);
			}
			Gui_FillBlock(&DrawRegion);
			Gui_FillBlock(&IconDrawReg);
		}
		else if((pVars->CurStartNum+Row)>=pVars->Unit.N.CurSubDirTotal)
		{//1如果当前选择超过子目录个数,显示文件
			//Debug("readfile %d %d\n\r",gpFpVars->CurrentStartNum+Row-gpFpVars->CurSubDirTotal,gpFpVars->FileIndex[gpFpVars->CurrentStartNum+Row-gpFpVars->CurSubDirTotal]);
			if((pVars->pd= FS_ReadDir_IDX(pVars->dirp,pVars->Unit.N.FileIndex[pVars->CurStartNum+Row-pVars->Unit.N.CurSubDirTotal]))!=0)
			{
				//画底色
				if(Row&0x01)
				{
					DrawRegion.Color=FatColor(BG_COLOR_1);
					IconDrawReg.Color=FatColor(BG_COLOR_1);
				}
				else
				{
					DrawRegion.Color=FatColor(BG_COLOR_2);
					IconDrawReg.Color=FatColor(BG_COLOR_2);
				}
				Gui_FillBlock(&DrawRegion);
				Gui_FillBlock(&IconDrawReg);

				IconDrawReg.Color=FatColor(ICON_TRAN_COLOR);
				Gui_DrawImgArray((u8 *)gImage_GrayDot,&IconDrawReg);
				DrawRegion.Color=FatColor(FILE_NAME_COLOR);
				Gui_DrawFont(GBK16_FONT,(void *)pVars->pd->d_name,&DrawRegion);
			}
		}
		else
		{//显示子目录
			//Debug("readdir %d %d\n\r",gpFpVars->CurrentStartNum+Row,gpFpVars->DirIndex[gpFpVars->CurrentStartNum+Row]);
			if((pVars->pd= FS_ReadDir_IDX(pVars->dirp,pVars->Unit.N.DirIndex[pVars->CurStartNum+Row]))!=0)
			{
				//画底色
				if(Row&0x01)
				{
					DrawRegion.Color=FatColor(BG_COLOR_1);
					IconDrawReg.Color=FatColor(BG_COLOR_1);
				}
				else
				{
					DrawRegion.Color=FatColor(BG_COLOR_2);
					IconDrawReg.Color=FatColor(BG_COLOR_2);
				}
				Gui_FillBlock(&DrawRegion);
				Gui_FillBlock(&IconDrawReg);

				IconDrawReg.Color=FatColor(ICON_TRAN_COLOR);
				Gui_DrawImgArray((u8 *)gImage_GrayDir,&IconDrawReg);
				DrawRegion.Color=FatColor(DIR_NAME_COLOR);
				Gui_DrawFont(GBK16_FONT,(void *)pVars->pd->d_name,&DrawRegion);
			}
		}
	}

	FS_CloseDir(pVars->dirp);

	return TRUE;
}

//显示一个文件夹的当前文件
//返回FALSE表示显示完了
static bool DisplayFilesByListBuf(FLIELIST_PAGE_VARS *pVars)
{
//	FRESULT Res;
	GUI_REGION DrawRegion;
	u8 Row=0;
	u16 i;	

	pVars->CurSelect=0xff;
	pVars->CurSelectName[0]=0;
	
	DrawRegion.x=DISPLAY_START_X;
	DrawRegion.y=DISPLAY_START_Y+FONT_TOP_MARGIN;
	DrawRegion.w=DISPLAY_WIDTH;
	DrawRegion.h=ROW_HIGHT;
	DrawRegion.Space=FONT_SPACE;

	if(pVars->CurTotal)//有文件
	{
		//打开list buf
		i=strlen((void *)pVars->pPath);
		strcat((void *)pVars->pPath,LIST_BUF_NAME);//拼合list.buf路径

		if ((pVars->fp=FS_FOpen((void *)pVars->pPath,FA_OPEN_EXISTING | FA_READ) )== 0 ) 
		{
			Debug("Open txt flie error !:%s\n\r",pVars->pPath);
			return FALSE;
		}

		pVars->pPath[i]=0;//剪去list.buf
		
		for(Row=0;Row<PAGE_DISPALY_NUM;Row++,DrawRegion.y+=ROW_HIGHT)
		{
			if((pVars->CurStartNum+Row)>=pVars->CurTotal)
			{//超过范围
				DrawRegion.Color=FatColor(0);
				Gui_FillBlock(&DrawRegion);
			}
			else if(GetArrayBit(&(pVars->Unit.L.Header[1]),pVars->CurStartNum+Row)==1)
			{//显示文件
				DrawRegion.Color=FatColor(0);
				Gui_FillBlock(&DrawRegion);
				//Gui_DrawFont(GBK16_FONT,(void *)finfo.lfname,&DrawRegion,FILE_NAME_COLOR);
			}
			else
			{//显示子目录
				DrawRegion.Color=FatColor(0);
				Gui_FillBlock(&DrawRegion);
				//Gui_DrawFont(GBK16_FONT,(void *)finfo.lfname,&DrawRegion,DIR_NAME_COLOR);
			}
		}
	}

	if(pVars->pPath[0])
	{
		for(i=strlen((void *)pVars->pPath);i;i--) if(pVars->pPath[i]=='/') break;//得到最后一层目录的名字
		sprintf((void *)pVars->pd->d_name,"%s: %d / %d",&pVars->pPath[++i],pVars->CurStartNum/PAGE_DISPALY_NUM+1,(pVars->CurTotal-1)/PAGE_DISPALY_NUM+1);
	}
	else//如果是根目录
		sprintf((void *)pVars->pd->d_name,"./: %d / %d",pVars->CurStartNum/PAGE_DISPALY_NUM+1,(pVars->CurTotal-1)/PAGE_DISPALY_NUM+1);
	DrawRegion.Color=FatColor(0xffffff);
	Gui_FillBlock(&DrawRegion);
	DrawRegion.Color=FatColor(0x666666);
	Gui_DrawFont(GBK16_FONT,(void *)pVars->pd->d_name,&DrawRegion);

	return TRUE;
}

static bool HandleNewDir(FLIELIST_PAGE_VARS *pVars)
{
	if(pVars->Cmd&FL_NoListBuf)
	{
		pVars->UseListBuf=FALSE;
		PrefetchDir(pVars);	
		return DisplayFiles(pVars);//显示新目录
	}
	else
	{
		pVars->UseListBuf=TRUE;
		
		if(pVars->Cmd&FL_ForceListBuf)
			CreatListBuf(pVars,TRUE);
		else
			CreatListBuf(pVars,FALSE);

		if(PrefetchDirByList(pVars)==FALSE) return FALSE;

		return DisplayFilesByListBuf(pVars);//显示新目录
	}
}

static bool ChangeSelect(u8 NewSelect,FLIELIST_PAGE_VARS *pVars,TCH_EVT InEvent )
{
	GUI_REGION IconDrawReg;
	GUI_REGION NameDrawReg;
//	FRESULT Res;

	Debug("Old %d ,New %d\n\r",pVars->CurSelect,NewSelect);
	if(NewSelect>=PAGE_DISPALY_NUM) return FALSE;

	if(pVars->CurSelect==NewSelect)//如果选择没变 ，即双击了
	{
		if((pVars->CurStartNum+pVars->CurSelect)>=pVars->Unit.N.CurSubDirTotal)
		{//选择的是文件
			return FALSE;
		}
		else //选择的是目录
		{
			if(InEvent==Tch_Release)
			{
				strcat((void *)pVars->pPath,"/");
				strcat((void *)pVars->pPath,(void *)pVars->CurSelectName);//产生新目录路径
				HandleNewDir(pVars);//进入新目录
				return TRUE;
			}
			else
				return FALSE;
		}
	}	

	if ((pVars->dirp=FS_OpenDir((void *)pVars->pPath))== 0) 
	{
		Debug("CS:Open Dir %s error !\n\r",pVars->pPath); 		
		return FALSE;
	}	

	IconDrawReg.x=DISPLAY_START_X-ICON_WIDTH-ICON_RIGHT_MARGIN;
	IconDrawReg.w=ICON_WIDTH;
	IconDrawReg.h=ICON_HEIGHT;

	NameDrawReg.x=DISPLAY_START_X;
	NameDrawReg.w=DISPLAY_WIDTH;
	NameDrawReg.h=FONT_HIGHT;
	NameDrawReg.Space=FONT_SPACE;
	
	//去掉旧亮
	if(pVars->CurSelect<PAGE_DISPALY_NUM)
	{
		if((pVars->CurStartNum+pVars->CurSelect)>=pVars->CurTotal)
		{//超过范围
			return FALSE;
		}
		else if((pVars->CurStartNum+pVars->CurSelect)>=pVars->Unit.N.CurSubDirTotal)
		{//如果当前选择超过子目录个数,显示文件
			if(pVars->CurSelectName[0])
			{
				IconDrawReg.y=DISPLAY_START_Y+ICON_TOP_MARGIN+ROW_HIGHT*pVars->CurSelect;
				NameDrawReg.y=DISPLAY_START_Y+FONT_TOP_MARGIN+ROW_HIGHT*pVars->CurSelect;
				if(pVars->CurSelect&0x01)
					//NameDrawReg.Color=
					IconDrawReg.Color=FatColor(BG_COLOR_1);
				else
					//NameDrawReg.Color=
					IconDrawReg.Color=FatColor(BG_COLOR_2);

				//画图标	
				Gui_FillBlock(&IconDrawReg);
				IconDrawReg.Color=FatColor(ICON_TRAN_COLOR);
				Gui_DrawImgArray((u8 *)gImage_GrayDot,&IconDrawReg);

				//写文件名
				//Gui_FillBlock(&NameDrawReg);
				NameDrawReg.Color=FatColor(FILE_NAME_COLOR);
				Gui_DrawFont(GBK16_FONT,pVars->CurSelectName,&NameDrawReg);
			}
		}
		else
		{//显示子目录
			if(pVars->CurSelectName[0])
			{
				IconDrawReg.y=DISPLAY_START_Y+ICON_TOP_MARGIN+ROW_HIGHT*pVars->CurSelect;
				NameDrawReg.y=DISPLAY_START_Y+FONT_TOP_MARGIN+ROW_HIGHT*pVars->CurSelect;
				if(pVars->CurSelect&0x01)
					//NameDrawReg.Color=
					IconDrawReg.Color=FatColor(BG_COLOR_1);
				else
					//NameDrawReg.Color=
					IconDrawReg.Color=FatColor(BG_COLOR_2);

				//画图标
				Gui_FillBlock(&IconDrawReg);
				IconDrawReg.Color=FatColor(ICON_TRAN_COLOR);
				Gui_DrawImgArray((u8 *)gImage_GrayDir,&IconDrawReg);

				//写路径名
				//Gui_FillBlock(&NameDrawReg);
				NameDrawReg.Color=FatColor(DIR_NAME_COLOR);
				Gui_DrawFont(GBK16_FONT,pVars->CurSelectName,&NameDrawReg);
			}
		}
	}

	pVars->CurSelect=NewSelect;		
    
	//添加新亮
	if((pVars->CurStartNum+NewSelect)>=pVars->CurTotal)
	{//超过范围
		return FALSE;
	}
	else if((pVars->CurStartNum+NewSelect)>=pVars->Unit.N.CurSubDirTotal)
	{//如果当前选择超过子目录个数,显示文件

		if((pVars->pd= FS_ReadDir_IDX(pVars->dirp,pVars->Unit.N.FileIndex[pVars->CurStartNum+NewSelect-pVars->Unit.N.CurSubDirTotal]))!=0)
		{
			strcpy((void *)pVars->CurSelectName,(void *)pVars->pd->d_name);

			IconDrawReg.y=DISPLAY_START_Y+ICON_TOP_MARGIN+ROW_HIGHT*NewSelect;
			NameDrawReg.y=DISPLAY_START_Y+FONT_TOP_MARGIN+ROW_HIGHT*pVars->CurSelect;
			if(NewSelect&0x01)
				//NameDrawReg.Color=
				IconDrawReg.Color=FatColor(BG_COLOR_1);
			else
				//NameDrawReg.Color=
				IconDrawReg.Color=FatColor(BG_COLOR_2);

			//画图标
			Gui_FillBlock(&IconDrawReg);
			IconDrawReg.Color=FatColor(ICON_TRAN_COLOR);
			Gui_DrawImgArray((u8 *)gImage_GreenDot,&IconDrawReg);

			//写文件名
			//Gui_FillBlock(&NameDrawReg);
			NameDrawReg.Color=FatColor(FILE_SELECT_COLOR);
			Gui_DrawFont(GBK16_FONT,pVars->CurSelectName,&NameDrawReg);
		}
	}
	else
	{//显示子目录

		if((pVars->pd= FS_ReadDir_IDX(pVars->dirp,pVars->Unit.N.DirIndex[pVars->CurStartNum+NewSelect]))!=0)
		{
			strcpy((void *)pVars->CurSelectName,(void *)pVars->pd->d_name);
			IconDrawReg.y=DISPLAY_START_Y+ICON_TOP_MARGIN+ROW_HIGHT*NewSelect;
			NameDrawReg.y=DISPLAY_START_Y+FONT_TOP_MARGIN+ROW_HIGHT*pVars->CurSelect;
			if(NewSelect&0x01)
				//NameDrawReg.Color=
				IconDrawReg.Color=FatColor(BG_COLOR_1);
			else
				//NameDrawReg.Color=
				IconDrawReg.Color=FatColor(BG_COLOR_2);

			//画图标
			Gui_FillBlock(&IconDrawReg);
			IconDrawReg.Color=FatColor(ICON_TRAN_COLOR);
			Gui_DrawImgArray((u8 *)gImage_Dir,&IconDrawReg);

			//写路径名
			//Gui_FillBlock(&NameDrawReg);
			NameDrawReg.Color=FatColor(DIR_SELECT_COLOR);
			Gui_DrawFont(GBK16_FONT,pVars->CurSelectName,&NameDrawReg);
		}
	}

	FS_CloseDir(pVars->dirp);
	return TRUE;
}

//------------------------未进入页面之前调用的函数------------------------
//返回路径到DirPath中
static bool GetNextFile(u8 *PathBuf,u8 *FileName,u8 *SuffixStr,bool UseListBuf)
{
	FS_VARS *pFsVars;
//	FRESULT Res;
	bool Flag;
	
	if(UseListBuf)
	{
		
	}
	else
	{
		pFsVars=(FS_VARS *)Q_PageMallco(sizeof(FS_VARS));//申请空间
		
		if(pFsVars==0)
		{
			Q_ErrorStopScreen("pFsVars malloc fail !\n\r");
		}

		Flag=FALSE;
		
		//打开要读的文件夹
		if ((pFsVars->dirp=FS_OpenDir((void *)PathBuf))== 0) 
		{
			Debug("GNF:Open Dir %s error !\n\r",PathBuf);
		}	

		else
		{		    	
			while((pFsVars->pd=FS_ReadDir(pFsVars->dirp))!=0)  
			{
				if(pFsVars->pd->d_name[0]==0) break;//读完文件退出

				if(!S_ISDIR(pFsVars->pd->st_mode)) 
				{
					if(CheckMultiSuffix((void *)pFsVars->pd->d_name,SuffixStr)) 
					{
						//Debug("**File:%s,%s\n\r",pFsVars->FileInfo.lfname,pFsVars->FileInfo.fname);	
						if(Flag==TRUE)
						{
							strcat((void *)PathBuf,"/");
							strcat((void *)PathBuf,(void *)pFsVars->pd->d_name);
							break;
						}
						if(strcmp((void *)FileName,(void *)pFsVars->pd->d_name)==0) Flag=TRUE;//匹配到了，下一个文件就是
					}
				}
			}

			FS_CloseDir(pFsVars->dirp);

		}
		
		Q_PageFree(pFsVars);
	}
	
	return TRUE;
}

//返回路径到DirPath中
static bool GetPrevFile(u8 *PathBuf,u8 *FileName,u8 *SuffixStr,bool UseListBuf)
{
	FS_VARS *pFsVars;
//	FRESULT Res;
	u16 PrevIndex;
	u16 i;
	
	if(UseListBuf)
	{
		
	}
	else
	{
		pFsVars=(FS_VARS *)Q_PageMallco(sizeof(FS_VARS));//申请空间

		if(pFsVars==0)
		{
			Q_ErrorStopScreen("pFsVars malloc fail !\n\r");
		}

		if ((pFsVars->dirp=FS_OpenDir((void *)PathBuf) )== 0) 
		{
			Debug("GNF:Open Dir %s error !\n\r",PathBuf);
		}	

		else
		{	
			for(i=0,PrevIndex=0xffff;(pFsVars->pd=FS_ReadDir(pFsVars->dirp)) !=0;i++)	
			{
				if(!pFsVars->pd->d_name[0])//读完文件
				{
					break;
				}
				
				if(!S_ISDIR(pFsVars->pd->st_mode)) 
				{
					if(CheckMultiSuffix((void *)pFsVars->pd->d_name,SuffixStr)) //后缀匹配
					{
						//Debug("**File:%s,%s\n\r",pFsVars->FileInfo.lfname,pFsVars->FileInfo.fname);	
						if(strcmp((void *)FileName,(void *)pFsVars->pd->d_name)==0) //匹配到了，上一个文件就是
						{
							if(PrevIndex!=0xffff)
							{
								if((pFsVars->pd= FS_ReadDir_IDX(pFsVars->dirp,PrevIndex))!=0)
								{
									strcat((void *)PathBuf,"/");
									strcat((void *)PathBuf,(void *)pFsVars->pd->d_name);
								}
							}
							break;
						}
						PrevIndex=i;
					}
				}
			}
			FS_CloseDir(pFsVars->dirp);
		}
		
		Q_PageFree(pFsVars);
	}
	
	return TRUE;
}

//返回路径到DirPath中
static bool GetFristFile(u8 *PathBuf,u8 *SuffixStr,bool UseListBuf)
{
	FS_VARS *pFsVars;
//	FRESULT Res;
	
	if(UseListBuf)
	{
		
	}
	else
	{
		pFsVars=(FS_VARS *)Q_PageMallco(sizeof(FS_VARS));//申请空间

		if(pFsVars==0)
		{
			Q_ErrorStopScreen("pFsVars malloc fail !\n\r");
		}	
		//打开要读的文件夹
		if ((pFsVars->dirp=FS_OpenDir((void *)PathBuf))== 0) 
		{
			Debug("GNF:Open Dir %s error !\n\r",PathBuf);
		}	
		else
		{		 
			while((pFsVars->pd=FS_ReadDir(pFsVars->dirp)) != 0)  
			{
				if(!pFsVars->pd->d_name[0])//读完文件
				{
					break;
				}
				
				if(!S_ISDIR(pFsVars->pd->st_mode)) 
				{
					if(CheckMultiSuffix((void *)pFsVars->pd->d_name,SuffixStr)) 
					{
						//Debug("**File:%s,%s\n\r",pFsVars->FileInfo.lfname,pFsVars->FileInfo.fname);	
						strcat((void *)PathBuf,"/");
						strcat((void *)PathBuf,(void *)pFsVars->pd->d_name);
						break;
					}
				}
			}
			FS_CloseDir(pFsVars->dirp);
		}
		
		Q_PageFree(pFsVars);
	}
	
	return TRUE;
}

//返回路径到DirPath中
static bool GetLastFile(u8 *PathBuf,u8 *SuffixStr,bool UseListBuf)
{
	FS_VARS *pFsVars;
//	FRESULT Res;
	u8 FirstChar;
	u16 LastIndex;
	u16 i;
	
	if(UseListBuf)
	{
		
	}
	else
	{
		pFsVars=(FS_VARS *)Q_PageMallco(sizeof(FS_VARS));//申请空间

		if(pFsVars==0)
		{
			Q_ErrorStopScreen("pFsVars malloc fail !\n\r");
		}
				
		if ((pFsVars->dirp=FS_OpenDir((void *)PathBuf))== 0) 
		{
			Debug("GNF:Open Dir %s error !\n\r",PathBuf);
			PathBuf[0]=0;
		}	

		else
		{		    	
			FirstChar=PathBuf[0];
			PathBuf[0]=0;

			for(i=0,LastIndex=0xffff;(pFsVars->pd=FS_ReadDir(pFsVars->dirp)) !=0;i++)	
			{
				if(!pFsVars->pd->d_name[0])//读完文件
				{
					if(LastIndex!=0xffff)
					{
						if((pFsVars->pd= FS_ReadDir_IDX(pFsVars->dirp,LastIndex))!=0)
						{
							PathBuf[0]=FirstChar;
							strcat((void *)PathBuf,"/");
							strcat((void *)PathBuf,(void *)pFsVars->pd->d_name);
						}
					}
					break;
				}

				if(!S_ISDIR(pFsVars->pd->st_mode)) 
				{
					if(CheckMultiSuffix((void *)pFsVars->pd->d_name,SuffixStr)) //后缀匹配
					{
						LastIndex=i;
					}
				}
			}
			FS_CloseDir(pFsVars->dirp);

		}
		
		Q_PageFree(pFsVars);
	}
	
	return TRUE;
}

//------------------------未进入页面之前调用的函数结束------------------------

//------------------------本页系统函数------------------------
//在每次本页面变成系统当前页面时会调用的程序
//在每次本页面发生关键事件时会调用的函数
//用以处理页面的lcd显示
//所以本页面大部分显示工作请放到此函数内
//以避免显示混乱
//调用时机及对应事件如下
// 1.当本页面变成当前页面时，事件Sys_Page_Init
// 2.当本页面的所有按钮区域绘画完毕时，事件Sys_TouchRegionSetFinish
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,FILELIST_CMD Cmd, FILELIST_SET *pFLS)
{
	GUI_REGION DrawRegion;
	u8 *pCurName;
	u16 i;
	u8 Row;
	
	switch(SysEvent)
	{
		case Sys_PreGotoPage:
			if(Q_DB_GetStatus(Status_FsInitFinish,NULL)==FALSE) //文件系统没有挂载
			{
				Debug("File system not mount!!!\n\r");
				return SM_State_Faile|SM_NoGoto;
			}			
			
			if(pFLS==NULL)//传入参数不正确
			{		
				Debug("Flie list page param is NULL!!!");
				return SM_State_Faile|SM_NoGoto;
			}			

			//Debug("Cmd:0x%x\n\r",Cmd);

			switch(Cmd&FL_CMD_MASK)
			{
				case FL_SelectOne:
					break;
				case FL_CreatListBuf:
					break;
				case FL_FindNext:
					i=strlen((void *)pFLS->pPathBuf);
					if(i==0) return SM_State_Faile|SM_NoGoto;
					for(;i;i--)//剥离目录和文件名
					{
						if(pFLS->pPathBuf[i]=='/')
						{
							pFLS->pPathBuf[i]=0;
							break;
						}
					}
					pCurName=&pFLS->pPathBuf[i+1];
					
					if(Cmd&FL_NoListBuf) GetNextFile(pFLS->pPathBuf,pCurName,pFLS->pSuffixStr,FALSE);
					else GetNextFile(pFLS->pPathBuf,pCurName,pFLS->pSuffixStr,TRUE);					
					return SM_State_OK|SM_NoGoto;
				case FL_FindPrevious:
					i=strlen((void *)pFLS->pPathBuf);
					if(i==0) return SM_State_Faile|SM_NoGoto;
					for(;i;i--)//剥离目录和文件名
					{
						if(pFLS->pPathBuf[i]=='/')
						{
							pFLS->pPathBuf[i]=0;
							break;
						}
					}
					pCurName=&pFLS->pPathBuf[i+1];
					
					if(Cmd&FL_NoListBuf) GetPrevFile(pFLS->pPathBuf,pCurName,pFLS->pSuffixStr,FALSE);
					else GetPrevFile(pFLS->pPathBuf,pCurName,pFLS->pSuffixStr,TRUE);					
					return SM_State_OK|SM_NoGoto;
				case FL_FindFrist:
					i=strlen((void *)pFLS->pPathBuf);
					if(i==0) return SM_State_Faile|SM_NoGoto;
					i--;
					if(pFLS->pPathBuf[i]=='/') pFLS->pPathBuf[i]=0;//去掉最后一个'/'

					if(Cmd&FL_NoListBuf) GetFristFile(pFLS->pPathBuf,pFLS->pSuffixStr,FALSE);
					else GetFristFile(pFLS->pPathBuf,pFLS->pSuffixStr,TRUE);					
					return SM_State_OK|SM_NoGoto;
				case FL_FindLast:
					i=strlen((void *)pFLS->pPathBuf);
					if(i==0) return SM_State_Faile|SM_NoGoto;
					i--;
					if(pFLS->pPathBuf[i]=='/') pFLS->pPathBuf[i]=0;//去掉最后一个'/'

					if(Cmd&FL_NoListBuf) GetLastFile(pFLS->pPathBuf,pFLS->pSuffixStr,FALSE);
					else GetLastFile(pFLS->pPathBuf,pFLS->pSuffixStr,TRUE);					
					return SM_State_OK|SM_NoGoto;
				default:
					return SM_State_Faile|SM_NoGoto;
			}
			
			Debug("sizeof(FLIELIST_PAGE_VARS)=%d\n\r",sizeof(FLIELIST_PAGE_VARS));
			Debug("Dir:%s\n\r",pFLS->pPathBuf);
			gpFpVars=(FLIELIST_PAGE_VARS *)Q_PageMallco(sizeof(FLIELIST_PAGE_VARS));//申请空间

			if(gpFpVars==0)
			{
				Q_ErrorStopScreen("gpFpVars malloc fail !\n\r");
			}
					
			gpFpVars->Cmd=Cmd;//复制当前命令
			gpFpVars->pPath=pFLS->pPathBuf;//取得路径缓存指针
			gpFpVars->RootPathLen=strlen((void *)gpFpVars->pPath);//取得初始路径长度

			//如果不复制，当函数返回，指针指向的内存就注销了
			if(gpFpVars->pPath[gpFpVars->RootPathLen-1]=='/') 
			{//如果最后一个字符是斜杠
				gpFpVars->pPath[gpFpVars->RootPathLen-1]=0;//去掉最后一个'/'
				gpFpVars->RootPathLen--;
			}
			gpFpVars->CallerPageRid=pFLS->CallBackRid;		//记录返回页面	
			strcpy((void *)gpFpVars->SuffixStr,(void *)pFLS->pSuffixStr);//复制后缀名字符串

			Debug("SuffixStr %s\r\n",gpFpVars->SuffixStr);
			
			//文件系统
			gpFpVars->fp =0 ;
			gpFpVars->dirp = 0;

			if((Cmd&FL_CMD_MASK)==FL_CreatListBuf)
			{
				CreatListBuf(gpFpVars,TRUE);
				Q_PageFree(gpFpVars);
				return SM_State_OK|SM_NoGoto;
			}
			
     		return SM_State_OK;
		case Sys_PageInit:	

			//画标题栏
			DrawRegion.x=DrawRegion.y=0;
			DrawRegion.w=240;
			DrawRegion.h=21;
			DrawRegion.Color=FatColor(NO_TRANS);
			Gui_FillImgArray((u8 *)gImage_StatusBar1,1,21,&DrawRegion);	
			
			//画背景
			DrawRegion.y=DISPLAY_START_Y;
			DrawRegion.w=240;
			DrawRegion.h=ROW_HIGHT;
			for(Row=0;Row<PAGE_DISPALY_NUM;Row++,DrawRegion.y+=ROW_HIGHT)
			{
				if(Row&0x01)
					DrawRegion.Color=FatColor(BG_COLOR_1);
				else
					DrawRegion.Color=FatColor(BG_COLOR_2);
				Gui_FillBlock(&DrawRegion);
			}		

			//画底栏
			DrawRegion.x=0;
			DrawRegion.y=320-39;
			DrawRegion.w=240;
			DrawRegion.h=39;
			DrawRegion.Color=FatColor(NO_TRANS);
			Gui_FillImgArray((u8 *)gImage_BottomBar1,1,39,&DrawRegion);
	
			return SM_State_OK;
		case Sys_TouchSetOk:
			HandleNewDir(gpFpVars);//进入新目录
			return SM_State_OK;
		case Sys_PageClean:
			Q_PageFree(gpFpVars);
			return SM_State_OK;
		default:
			Debug("You should not here! %d\n\r",SysEvent);
			//while(1);
	}
	
	return SM_State_OK;
}
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam)
{
	switch(PeripEvent)
	{
		case Perip_KeyPress:
				switch(IntParam){
					case ExtiKeyEnter:
						PrtScreen();
						break;
					case ExtiKeyUp:
						if(gpFpVars->CurStartNum-PAGE_DISPALY_NUM>=0)
						{
							gpFpVars->CurStartNum-=PAGE_DISPALY_NUM;
							DisplayFiles(gpFpVars);
						}			
						break;
					case ExtiKeyDown:
						if(gpFpVars->CurStartNum+PAGE_DISPALY_NUM<gpFpVars->CurTotal) 
						{
							gpFpVars->CurStartNum+=PAGE_DISPALY_NUM;
							DisplayFiles(gpFpVars);
						}
						break; 
				}
			break;
		case Perip_UartInput:
			if((IntParam>>16)==1)//串口1
			{
				Q_Sh_CmdHandler(IntParam&0xffff,pParam);
			}
			break;
	} 
	return 0;
}
static CO_MSG ButtonHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo)
{	
	int i;

	switch(Key)
	{
		case UpKV:
			if(InEvent!=Tch_Release) return 0;
			if(gpFpVars->CurStartNum-PAGE_DISPALY_NUM>=0)
			{
				gpFpVars->CurStartNum-=PAGE_DISPALY_NUM;
				DisplayFiles(gpFpVars);
			}			
			break;
		case DownKV:
			if(InEvent!=Tch_Release) return 0;
			if(gpFpVars->CurStartNum+PAGE_DISPALY_NUM<gpFpVars->CurTotal) 
			{
				gpFpVars->CurStartNum+=PAGE_DISPALY_NUM;
				DisplayFiles(gpFpVars);
			}
			break;
		case NumKV:
			Key=(pTouchInfo->y+FONT_TOP_MARGIN-DISPLAY_START_Y)/ROW_HIGHT;
			if((Key+gpFpVars->CurStartNum)<gpFpVars->CurTotal)
			{
				ChangeSelect(Key,gpFpVars,InEvent);	
			}
			break;
		case ParentDirKV:
			if(InEvent!=Tch_Release) return 0;
			if(gpFpVars->pPath[0]!=0)
			{
				for(i=strlen((void *)gpFpVars->pPath);i;i--)//除去最后一层目录
				{
					Debug("%c",gpFpVars->pPath[i]);
					if(gpFpVars->pPath[i]=='/') break;
				}

				if((gpFpVars->Cmd&FL_NoParent)&&(i<gpFpVars->RootPathLen))//不准进入初始目录上层
					break;
				else
					gpFpVars->pPath[i]=0;

				HandleNewDir(gpFpVars);//进入新目录
			}
			break;
		case EnterKV:
			if(InEvent!=Tch_Release) return 0;
			if(gpFpVars->CurSelect!=0xff) 
			{
				sprintf((void *)gpFpVars->pPath,"%s/%s",gpFpVars->pPath,gpFpVars->CurSelectName);
				Debug("Select %s\n\r",gpFpVars->pPath);//拼合文件路径
				Q_GotoPage(SubPageReturn,"",TRUE,gpFpVars->pPath);
			}
			else
				Q_GotoPage(SubPageReturn,"",FALSE,gpFpVars->pPath);
			break;		
		case ReturnKV:
			if(InEvent!=Tch_Release) return 0;
			Q_GotoPage(SubPageReturn,"",FALSE,gpFpVars->pPath);
			break;
		default:
			Debug("You should not here! Key:%d\n\r",Key);
			//while(1);
	}
	
	//OS_TaskDelayMs(200);

	return 0;
}




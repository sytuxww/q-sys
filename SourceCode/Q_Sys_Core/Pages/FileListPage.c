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

//-----------------------��ҳϵͳ����������-----------------------
typedef enum 
{
	ExtiKeyDown=EXTI_KEY_VALUE_START,//ϵͳĬ�Ͻ��ⲿ�жϰ������͵���һ����ֵ
	ExtiKeyUp,
	ExtiKeyEnter,

	NumKV=USER_KEY_VALUE_START,
	UpKV,
	DownKV,
	ParentDirKV,
	EnterKV,
	ReturnKV
}FileListPage_OID;

//����ҳ���Ӧ�õĴ������򼯣��൱�ڶ��尴��
static const IMG_BUTTON_OBJ ImgButtonCon[]={
	//{key,gLandScapeMode,x,y,width,hight,image x,image y,normal bmp path,release bmp path,press bmp path,transparent color,key name}
	{"",NumKV,PrsMsk|CotMsk|RelMsk,0,26,240,260,0,0,"",0},
	{"Return",ReturnKV,RelMsk|PathMsk,4,287,39,31,0,0,"Common/Btn/Return",FatColor(NO_TRANS)},
	{"Up",UpKV,RelMsk|PathMsk,52,287,39,31,0,0,"Common/Btn/Up",FatColor(NO_TRANS)},
	{"Down",DownKV,RelMsk|PathMsk,100,287,39,31,0,0,"Common/Btn/Down",FatColor(NO_TRANS)},
	{"ParentDir",ParentDirKV,RelMsk|PathMsk,148,287,39,31,0,0,"Common/Btn/Parent",FatColor(NO_TRANS)},
	{"Enter",EnterKV,RelMsk|PathMsk,196,287,39,31,0,0,"Common/Btn/Yes",FatColor(NO_TRANS)},
};

//����ҳ�����Ӧ�õ����Լ�
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

//-----------------------��ҳ�Զ����������-----------------------
#define PAGE_DISPALY_NUM 10	//ÿҳ�����ʾ����
#define MAX_SUB_DIR_NUM 256//֧�ֵ������Ŀ��Ŀ¼��Ŀ
#define MAX_FILE_NUM 1024//֧�ֵ������Ŀ���ļ���Ŀ
#define EVERY_LIST_LEN 64	//list.buf��ÿ��listռ�õ��ֽ���

//��ʾ�йصĺ궨��
#define DISPLAY_START_X 4+20 //��ʾ�б���Ŀ����x����㣬����ͼ�겿��
#define DISPLAY_START_Y 21 //��ʾ�б��y�����
#define DISPLAY_WIDTH 230-24 //��ʾ��Ŀ���б�Ŀ��,����ͼ�겿��
#define ROW_HIGHT 26	//�и�
#define FONT_HIGHT 16 //�ָ�
#define FONT_TOP_MARGIN 5 //�ַ������԰�
#define FONT_SPACE 0x10;// 4-7λ���ּ�࣬0-3λ��Ч
#define ICON_WIDTH 16 //icon ���
#define ICON_HEIGHT 18 //icon ���
#define ICON_RIGHT_MARGIN 4 //icon�����־���
#define ICON_TOP_MARGIN 4 //icon�붥������

#define BG_COLOR_1 0xdddddd
#define BG_COLOR_2 0xeeeeee
#define ICON_TRAN_COLOR 0xff0000	//ͼ��͸��ɫ

#define FILE_NAME_COLOR	0x666666//�ļ���ɫ��
#define FILE_SELECT_COLOR 0x54920d	//��ѡ����ļ���ɫ��

#define DIR_NAME_COLOR	0x666666//Ŀ¼��ɫ��
#define DIR_SELECT_COLOR		0xff9000//��ѡ���Ŀ¼��ɫ��

//List Buf
#define LIST_BUF_NAME "/list.buf"

//ȫ�ֱ���
typedef struct{
	FILELIST_CMD Cmd;//������¼���ε��õ�����
	bool UseListBuf;
	union{
		struct{
			u8 DirIndex[MAX_SUB_DIR_NUM];//��ǰĿ¼�µ���Ŀ¼����,���ֻ֧��256����Ŀ¼
			u8 FileIndex[MAX_FILE_NUM];//��ǰĿ¼�µ��ļ�����,���ֻ֧��256���ļ�
			u16 CurFileTotal;//��ǰĿ¼�µ�ƥ���ļ�����	
			u8 CurSubDirTotal; //��ǰĿ¼�µ���Ŀ¼����
		}N;//No List Buf
		struct{
			u16 Header[EVERY_LIST_LEN>>1];//list buf���ļ�ͷ
			u8 RecordH[EVERY_LIST_LEN*PAGE_DISPALY_NUM*2];// 2ҳ�Ļ�����
			u8 *pRecord;
			//u8 CurTotal;//��ǰĿ¼�µ�ƥ���ļ��ļ�������
		}L;//List Buf
	}Unit;
	u8 *pPath;//������¼��ǰĿ¼�ľ���·��
	u8 CurSelectName[MAX_LEN_FILENAME];//������¼��ǰѡ�������
	u8 SuffixStr[64];//������¼��׺���ַ���
	
	u32 CallerPageRid;//��¼Ҫ���ص�ҳ��
	u16 CurStartNum;//��ǰ��ʾ����ʼ��
	u16 CurTotal;//=CurFileTotal+CurSubDirTotal,�ļ��к��ļ�����
	u8 CurSelect;//��ǰѡ����,����Ϊ0xff��ʾûѡ���κ���
	u8 RootPathLen;//��ʼĿ¼��·������

	//�ļ�ϵͳ
	struct dirent  *pd;
	struct stat stat_buf;
	FS_FILE *fp;
	FS_DIR *dirp;
}FLIELIST_PAGE_VARS;//����ҳҪ�õ���ȫ�ֱ���ȫ������˽ṹ��
static FLIELIST_PAGE_VARS *gpFpVars;//ֻ��Ҫ����һ��ָ�룬����ȫ�ֱ�����ʹ��

//�ļ�ϵͳ
typedef struct {//sizeof=680
	struct dirent  *pd;
	struct stat stat_buf;
	FS_DIR *dirp;
}FS_VARS;
//-----------------------��ҳ�Զ��庯��-----------------------
//��һ����ַ��ʼ��ָ��Bitλ�ø�
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

	if(IsDir==TRUE)//����Ŀ¼��ʶ��
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

//�ڵ�ǰĿ¼���½��б���
//����TRUE��ʾ�½���һ��
//����FALSE��ʾû�н����µ�
//Force��ʾ�Ƿ�ǿ���ؽ���������Ѵ��ڵ�
//pShuffixStrָ��Ҫƥ����ļ���׺
static bool CreatListBuf(FLIELIST_PAGE_VARS *pVars,bool Force)
{
//	FRESULT Res;
	unsigned int WriteByte;
	u16 i;

	//��Ҫ�����ļ���
	pVars->dirp = FS_OpenDir((void *)pVars->pPath);

	if (pVars->dirp==0)
	{
		Debug("CLB:Open Dir %s error !\n\r",pVars->pPath);			
		return FALSE;
	}

	//���������ļ�
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

    	for(i=0;i<(EVERY_LIST_LEN>>1);i++) pVars->Unit.L.Header[i]=0;//���ͷ����
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
				pVars->Unit.L.Header[0]++;//�ļ�����
				SetArrayBit(&(pVars->Unit.L.Header[1]),pVars->Unit.L.Header[0],0);//Ŀ¼��¼Ϊ0
				RecordOneName(pVars->fp,(u8 *)pVars->pd->d_name,strlen(pVars->pd->d_name),TRUE);		
			}
			else
			{
				if(CheckMultiSuffix((void *)pVars->pd->d_name,pVars->SuffixStr)) 

				{
					//Debug("**file:%s,%s\n\r",finfo.lfname,finfo.fname);	
					pVars->Unit.L.Header[0]++;//�ļ�����
					SetArrayBit(&(pVars->Unit.L.Header[1]),pVars->Unit.L.Header[0],1);//�ļ���¼Ϊ1
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
	else//ԭ�����д��ļ���������
	{
		return FALSE;
	}
}

static bool PrefetchDirByList(FLIELIST_PAGE_VARS *pVars)
{
	unsigned int ReadByte;
	u16 i;

	pVars->CurTotal=0;
	pVars->CurStartNum=0;//��ǰ��ʾ����ʼ��
	pVars->CurSelect=0;//��ǰѡ����
	
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

//pPath�����ǲ���б�ܽ�β���ļ���·��
//Ԥ����Ŀ¼��������list.buf�ļ�
static bool PrefetchDir(FLIELIST_PAGE_VARS *pVars)
{
//	FRESULT res;
	u16 i;

	pVars->Unit.N.CurSubDirTotal=0; //��ǰĿ¼�µ���Ŀ¼����
	pVars->Unit.N.CurFileTotal=0;//��ǰĿ¼�µ�ƥ���ļ�����
	pVars->CurTotal=0;
	pVars->CurStartNum=0;//��ǰ��ʾ����ʼ��
	pVars->CurSelect=0;//��ǰѡ����
	for(i=0;i<MAX_SUB_DIR_NUM;i++)
	{
		pVars->Unit.N.DirIndex[i]=0;
	}
	for(i=0;i<MAX_FILE_NUM;i++)
	{
		pVars->Unit.N.FileIndex[i]=0;
	}

	//��Ҫ�����ļ���
	if ((pVars->dirp=FS_OpenDir((void *)pVars->pPath))== 0) 
	{
		Debug("PFD:Open Dir %s error !\n\r",pVars->pPath);			
		return FALSE;
	}
			
	for(i=0;(pVars->pd=FS_ReadDir(pVars->dirp)) != 0;i++)  
	{
		if(pVars->pd->d_name[0]==0) break;//�����ļ��˳�

		//Debug("scan file:%s\r\n",pVars->pd->d_name);

		if(S_ISDIR(pVars->pd->st_mode)) //��������Ŀ¼
		{
			//Debug("**dir %s,%s\n\r",finfo.lfname,finfo.fname);
			pVars->Unit.N.DirIndex[pVars->Unit.N.CurSubDirTotal]=i;
			pVars->Unit.N.CurSubDirTotal++;
		}
		else  //���������ĵ�
		{
			if(CheckMultiSuffix((void *)pVars->pd->d_name,pVars->SuffixStr)) 
			{
				//Debug("**file:%s,%s\n\r",finfo.lfname,finfo.fname);	
				pVars->Unit.N.FileIndex[pVars->Unit.N.CurFileTotal]=i;
				pVars->Unit.N.CurFileTotal++;
			}
		}
				
		if((pVars->Unit.N.CurFileTotal>=MAX_FILE_NUM)||(pVars->Unit.N.CurSubDirTotal>=MAX_SUB_DIR_NUM)) 
		{//��Ŀ¼�ļ����������֧�ֵ���Ŀ
			Debug("Files and subdirs number is most than max support number!\n\r");
			break;
		}
	}

	FS_CloseDir(pVars->dirp);

	pVars->CurTotal=pVars->Unit.N.CurFileTotal+pVars->Unit.N.CurSubDirTotal;//������

	Debug("CurToal = %d %d %d!\r\n",pVars->CurTotal,pVars->Unit.N.CurFileTotal,pVars->Unit.N.CurSubDirTotal);

	return TRUE;
}

//��ʾһ���ļ��еĵ�ǰ�ļ�
//����FALSE��ʾ��ʾ����
static bool DisplayFiles(FLIELIST_PAGE_VARS *pVars)
{
//	FRESULT Res;
	GUI_REGION DrawRegion,IconDrawReg;
	u8 Row=0;
	u16 i;	

	pVars->CurSelect=0xff;
	pVars->CurSelectName[0]=0;
	
	//��ʾ·��
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
		{//û���ļ�����
			//����ɫ
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
		{//1�����ǰѡ�񳬹���Ŀ¼����,��ʾ�ļ�
			//Debug("readfile %d %d\n\r",gpFpVars->CurrentStartNum+Row-gpFpVars->CurSubDirTotal,gpFpVars->FileIndex[gpFpVars->CurrentStartNum+Row-gpFpVars->CurSubDirTotal]);
			if((pVars->pd= FS_ReadDir_IDX(pVars->dirp,pVars->Unit.N.FileIndex[pVars->CurStartNum+Row-pVars->Unit.N.CurSubDirTotal]))!=0)
			{
				//����ɫ
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
		{//��ʾ��Ŀ¼
			//Debug("readdir %d %d\n\r",gpFpVars->CurrentStartNum+Row,gpFpVars->DirIndex[gpFpVars->CurrentStartNum+Row]);
			if((pVars->pd= FS_ReadDir_IDX(pVars->dirp,pVars->Unit.N.DirIndex[pVars->CurStartNum+Row]))!=0)
			{
				//����ɫ
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

//��ʾһ���ļ��еĵ�ǰ�ļ�
//����FALSE��ʾ��ʾ����
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

	if(pVars->CurTotal)//���ļ�
	{
		//��list buf
		i=strlen((void *)pVars->pPath);
		strcat((void *)pVars->pPath,LIST_BUF_NAME);//ƴ��list.buf·��

		if ((pVars->fp=FS_FOpen((void *)pVars->pPath,FA_OPEN_EXISTING | FA_READ) )== 0 ) 
		{
			Debug("Open txt flie error !:%s\n\r",pVars->pPath);
			return FALSE;
		}

		pVars->pPath[i]=0;//��ȥlist.buf
		
		for(Row=0;Row<PAGE_DISPALY_NUM;Row++,DrawRegion.y+=ROW_HIGHT)
		{
			if((pVars->CurStartNum+Row)>=pVars->CurTotal)
			{//������Χ
				DrawRegion.Color=FatColor(0);
				Gui_FillBlock(&DrawRegion);
			}
			else if(GetArrayBit(&(pVars->Unit.L.Header[1]),pVars->CurStartNum+Row)==1)
			{//��ʾ�ļ�
				DrawRegion.Color=FatColor(0);
				Gui_FillBlock(&DrawRegion);
				//Gui_DrawFont(GBK16_FONT,(void *)finfo.lfname,&DrawRegion,FILE_NAME_COLOR);
			}
			else
			{//��ʾ��Ŀ¼
				DrawRegion.Color=FatColor(0);
				Gui_FillBlock(&DrawRegion);
				//Gui_DrawFont(GBK16_FONT,(void *)finfo.lfname,&DrawRegion,DIR_NAME_COLOR);
			}
		}
	}

	if(pVars->pPath[0])
	{
		for(i=strlen((void *)pVars->pPath);i;i--) if(pVars->pPath[i]=='/') break;//�õ����һ��Ŀ¼������
		sprintf((void *)pVars->pd->d_name,"%s: %d / %d",&pVars->pPath[++i],pVars->CurStartNum/PAGE_DISPALY_NUM+1,(pVars->CurTotal-1)/PAGE_DISPALY_NUM+1);
	}
	else//����Ǹ�Ŀ¼
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
		return DisplayFiles(pVars);//��ʾ��Ŀ¼
	}
	else
	{
		pVars->UseListBuf=TRUE;
		
		if(pVars->Cmd&FL_ForceListBuf)
			CreatListBuf(pVars,TRUE);
		else
			CreatListBuf(pVars,FALSE);

		if(PrefetchDirByList(pVars)==FALSE) return FALSE;

		return DisplayFilesByListBuf(pVars);//��ʾ��Ŀ¼
	}
}

static bool ChangeSelect(u8 NewSelect,FLIELIST_PAGE_VARS *pVars,TCH_EVT InEvent )
{
	GUI_REGION IconDrawReg;
	GUI_REGION NameDrawReg;
//	FRESULT Res;

	Debug("Old %d ,New %d\n\r",pVars->CurSelect,NewSelect);
	if(NewSelect>=PAGE_DISPALY_NUM) return FALSE;

	if(pVars->CurSelect==NewSelect)//���ѡ��û�� ����˫����
	{
		if((pVars->CurStartNum+pVars->CurSelect)>=pVars->Unit.N.CurSubDirTotal)
		{//ѡ������ļ�
			return FALSE;
		}
		else //ѡ�����Ŀ¼
		{
			if(InEvent==Tch_Release)
			{
				strcat((void *)pVars->pPath,"/");
				strcat((void *)pVars->pPath,(void *)pVars->CurSelectName);//������Ŀ¼·��
				HandleNewDir(pVars);//������Ŀ¼
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
	
	//ȥ������
	if(pVars->CurSelect<PAGE_DISPALY_NUM)
	{
		if((pVars->CurStartNum+pVars->CurSelect)>=pVars->CurTotal)
		{//������Χ
			return FALSE;
		}
		else if((pVars->CurStartNum+pVars->CurSelect)>=pVars->Unit.N.CurSubDirTotal)
		{//�����ǰѡ�񳬹���Ŀ¼����,��ʾ�ļ�
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

				//��ͼ��	
				Gui_FillBlock(&IconDrawReg);
				IconDrawReg.Color=FatColor(ICON_TRAN_COLOR);
				Gui_DrawImgArray((u8 *)gImage_GrayDot,&IconDrawReg);

				//д�ļ���
				//Gui_FillBlock(&NameDrawReg);
				NameDrawReg.Color=FatColor(FILE_NAME_COLOR);
				Gui_DrawFont(GBK16_FONT,pVars->CurSelectName,&NameDrawReg);
			}
		}
		else
		{//��ʾ��Ŀ¼
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

				//��ͼ��
				Gui_FillBlock(&IconDrawReg);
				IconDrawReg.Color=FatColor(ICON_TRAN_COLOR);
				Gui_DrawImgArray((u8 *)gImage_GrayDir,&IconDrawReg);

				//д·����
				//Gui_FillBlock(&NameDrawReg);
				NameDrawReg.Color=FatColor(DIR_NAME_COLOR);
				Gui_DrawFont(GBK16_FONT,pVars->CurSelectName,&NameDrawReg);
			}
		}
	}

	pVars->CurSelect=NewSelect;		
    
	//�������
	if((pVars->CurStartNum+NewSelect)>=pVars->CurTotal)
	{//������Χ
		return FALSE;
	}
	else if((pVars->CurStartNum+NewSelect)>=pVars->Unit.N.CurSubDirTotal)
	{//�����ǰѡ�񳬹���Ŀ¼����,��ʾ�ļ�

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

			//��ͼ��
			Gui_FillBlock(&IconDrawReg);
			IconDrawReg.Color=FatColor(ICON_TRAN_COLOR);
			Gui_DrawImgArray((u8 *)gImage_GreenDot,&IconDrawReg);

			//д�ļ���
			//Gui_FillBlock(&NameDrawReg);
			NameDrawReg.Color=FatColor(FILE_SELECT_COLOR);
			Gui_DrawFont(GBK16_FONT,pVars->CurSelectName,&NameDrawReg);
		}
	}
	else
	{//��ʾ��Ŀ¼

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

			//��ͼ��
			Gui_FillBlock(&IconDrawReg);
			IconDrawReg.Color=FatColor(ICON_TRAN_COLOR);
			Gui_DrawImgArray((u8 *)gImage_Dir,&IconDrawReg);

			//д·����
			//Gui_FillBlock(&NameDrawReg);
			NameDrawReg.Color=FatColor(DIR_SELECT_COLOR);
			Gui_DrawFont(GBK16_FONT,pVars->CurSelectName,&NameDrawReg);
		}
	}

	FS_CloseDir(pVars->dirp);
	return TRUE;
}

//------------------------δ����ҳ��֮ǰ���õĺ���------------------------
//����·����DirPath��
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
		pFsVars=(FS_VARS *)Q_PageMallco(sizeof(FS_VARS));//����ռ�
		
		if(pFsVars==0)
		{
			Q_ErrorStopScreen("pFsVars malloc fail !\n\r");
		}

		Flag=FALSE;
		
		//��Ҫ�����ļ���
		if ((pFsVars->dirp=FS_OpenDir((void *)PathBuf))== 0) 
		{
			Debug("GNF:Open Dir %s error !\n\r",PathBuf);
		}	

		else
		{		    	
			while((pFsVars->pd=FS_ReadDir(pFsVars->dirp))!=0)  
			{
				if(pFsVars->pd->d_name[0]==0) break;//�����ļ��˳�

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
						if(strcmp((void *)FileName,(void *)pFsVars->pd->d_name)==0) Flag=TRUE;//ƥ�䵽�ˣ���һ���ļ�����
					}
				}
			}

			FS_CloseDir(pFsVars->dirp);

		}
		
		Q_PageFree(pFsVars);
	}
	
	return TRUE;
}

//����·����DirPath��
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
		pFsVars=(FS_VARS *)Q_PageMallco(sizeof(FS_VARS));//����ռ�

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
				if(!pFsVars->pd->d_name[0])//�����ļ�
				{
					break;
				}
				
				if(!S_ISDIR(pFsVars->pd->st_mode)) 
				{
					if(CheckMultiSuffix((void *)pFsVars->pd->d_name,SuffixStr)) //��׺ƥ��
					{
						//Debug("**File:%s,%s\n\r",pFsVars->FileInfo.lfname,pFsVars->FileInfo.fname);	
						if(strcmp((void *)FileName,(void *)pFsVars->pd->d_name)==0) //ƥ�䵽�ˣ���һ���ļ�����
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

//����·����DirPath��
static bool GetFristFile(u8 *PathBuf,u8 *SuffixStr,bool UseListBuf)
{
	FS_VARS *pFsVars;
//	FRESULT Res;
	
	if(UseListBuf)
	{
		
	}
	else
	{
		pFsVars=(FS_VARS *)Q_PageMallco(sizeof(FS_VARS));//����ռ�

		if(pFsVars==0)
		{
			Q_ErrorStopScreen("pFsVars malloc fail !\n\r");
		}	
		//��Ҫ�����ļ���
		if ((pFsVars->dirp=FS_OpenDir((void *)PathBuf))== 0) 
		{
			Debug("GNF:Open Dir %s error !\n\r",PathBuf);
		}	
		else
		{		 
			while((pFsVars->pd=FS_ReadDir(pFsVars->dirp)) != 0)  
			{
				if(!pFsVars->pd->d_name[0])//�����ļ�
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

//����·����DirPath��
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
		pFsVars=(FS_VARS *)Q_PageMallco(sizeof(FS_VARS));//����ռ�

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
				if(!pFsVars->pd->d_name[0])//�����ļ�
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
					if(CheckMultiSuffix((void *)pFsVars->pd->d_name,SuffixStr)) //��׺ƥ��
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

//------------------------δ����ҳ��֮ǰ���õĺ�������------------------------

//------------------------��ҳϵͳ����------------------------
//��ÿ�α�ҳ����ϵͳ��ǰҳ��ʱ����õĳ���
//��ÿ�α�ҳ�淢���ؼ��¼�ʱ����õĺ���
//���Դ���ҳ���lcd��ʾ
//���Ա�ҳ��󲿷���ʾ������ŵ��˺�����
//�Ա�����ʾ����
//����ʱ������Ӧ�¼�����
// 1.����ҳ���ɵ�ǰҳ��ʱ���¼�Sys_Page_Init
// 2.����ҳ������а�ť����滭���ʱ���¼�Sys_TouchRegionSetFinish
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,FILELIST_CMD Cmd, FILELIST_SET *pFLS)
{
	GUI_REGION DrawRegion;
	u8 *pCurName;
	u16 i;
	u8 Row;
	
	switch(SysEvent)
	{
		case Sys_PreGotoPage:
			if(Q_DB_GetStatus(Status_FsInitFinish,NULL)==FALSE) //�ļ�ϵͳû�й���
			{
				Debug("File system not mount!!!\n\r");
				return SM_State_Faile|SM_NoGoto;
			}			
			
			if(pFLS==NULL)//�����������ȷ
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
					for(;i;i--)//����Ŀ¼���ļ���
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
					for(;i;i--)//����Ŀ¼���ļ���
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
					if(pFLS->pPathBuf[i]=='/') pFLS->pPathBuf[i]=0;//ȥ�����һ��'/'

					if(Cmd&FL_NoListBuf) GetFristFile(pFLS->pPathBuf,pFLS->pSuffixStr,FALSE);
					else GetFristFile(pFLS->pPathBuf,pFLS->pSuffixStr,TRUE);					
					return SM_State_OK|SM_NoGoto;
				case FL_FindLast:
					i=strlen((void *)pFLS->pPathBuf);
					if(i==0) return SM_State_Faile|SM_NoGoto;
					i--;
					if(pFLS->pPathBuf[i]=='/') pFLS->pPathBuf[i]=0;//ȥ�����һ��'/'

					if(Cmd&FL_NoListBuf) GetLastFile(pFLS->pPathBuf,pFLS->pSuffixStr,FALSE);
					else GetLastFile(pFLS->pPathBuf,pFLS->pSuffixStr,TRUE);					
					return SM_State_OK|SM_NoGoto;
				default:
					return SM_State_Faile|SM_NoGoto;
			}
			
			Debug("sizeof(FLIELIST_PAGE_VARS)=%d\n\r",sizeof(FLIELIST_PAGE_VARS));
			Debug("Dir:%s\n\r",pFLS->pPathBuf);
			gpFpVars=(FLIELIST_PAGE_VARS *)Q_PageMallco(sizeof(FLIELIST_PAGE_VARS));//����ռ�

			if(gpFpVars==0)
			{
				Q_ErrorStopScreen("gpFpVars malloc fail !\n\r");
			}
					
			gpFpVars->Cmd=Cmd;//���Ƶ�ǰ����
			gpFpVars->pPath=pFLS->pPathBuf;//ȡ��·������ָ��
			gpFpVars->RootPathLen=strlen((void *)gpFpVars->pPath);//ȡ�ó�ʼ·������

			//��������ƣ����������أ�ָ��ָ����ڴ��ע����
			if(gpFpVars->pPath[gpFpVars->RootPathLen-1]=='/') 
			{//������һ���ַ���б��
				gpFpVars->pPath[gpFpVars->RootPathLen-1]=0;//ȥ�����һ��'/'
				gpFpVars->RootPathLen--;
			}
			gpFpVars->CallerPageRid=pFLS->CallBackRid;		//��¼����ҳ��	
			strcpy((void *)gpFpVars->SuffixStr,(void *)pFLS->pSuffixStr);//���ƺ�׺���ַ���

			Debug("SuffixStr %s\r\n",gpFpVars->SuffixStr);
			
			//�ļ�ϵͳ
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

			//��������
			DrawRegion.x=DrawRegion.y=0;
			DrawRegion.w=240;
			DrawRegion.h=21;
			DrawRegion.Color=FatColor(NO_TRANS);
			Gui_FillImgArray((u8 *)gImage_StatusBar1,1,21,&DrawRegion);	
			
			//������
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

			//������
			DrawRegion.x=0;
			DrawRegion.y=320-39;
			DrawRegion.w=240;
			DrawRegion.h=39;
			DrawRegion.Color=FatColor(NO_TRANS);
			Gui_FillImgArray((u8 *)gImage_BottomBar1,1,39,&DrawRegion);
	
			return SM_State_OK;
		case Sys_TouchSetOk:
			HandleNewDir(gpFpVars);//������Ŀ¼
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
			if((IntParam>>16)==1)//����1
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
				for(i=strlen((void *)gpFpVars->pPath);i;i--)//��ȥ���һ��Ŀ¼
				{
					Debug("%c",gpFpVars->pPath[i]);
					if(gpFpVars->pPath[i]=='/') break;
				}

				if((gpFpVars->Cmd&FL_NoParent)&&(i<gpFpVars->RootPathLen))//��׼�����ʼĿ¼�ϲ�
					break;
				else
					gpFpVars->pPath[i]=0;

				HandleNewDir(gpFpVars);//������Ŀ¼
			}
			break;
		case EnterKV:
			if(InEvent!=Tch_Release) return 0;
			if(gpFpVars->CurSelect!=0xff) 
			{
				sprintf((void *)gpFpVars->pPath,"%s/%s",gpFpVars->pPath,gpFpVars->CurSelectName);
				Debug("Select %s\n\r",gpFpVars->pPath);//ƴ���ļ�·��
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




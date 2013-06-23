/**************** Q-OS *******************
 * PageName : 
 * Author : YuanYin
 * Version : 
 * Description :
 * WebSite : Www.Q-ShareWe.Com
 ***************** Q-OS *******************/ 

#include "User.h"
#include "Theme.h"
#include "MusicHandler.h"
#include "MusicPage.h"
#include "FileListPage.h"

static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam);
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam);
static CO_MSG ButtonHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo);
static SYS_MSG GobalMusicStopHandler(PERIP_EVT PeripEvent,int intParam, void *pParam);

//-----------------------��ҳϵͳ����������-----------------------
enum
{
	MpExtiKeyDown=EXTI_KEY_VALUE_START,//ϵͳĬ�Ͻ��ⲿ�жϰ������͵���һ����ֵ
	MpExtiKeyUp,
	MpExtiKeyEnter,

	MpVolumeUpKV=USER_KEY_VALUE_START,
	MpPrevOneKV,
	MpNextOneKV,
	MpVolumeDownKV,
	MpListKV,
	MpPlayKV,
	MpOffKV,
	MpPlayBarKV,

	MpHomeKV,
	MpMessageKV,
	MpMusicKV,
	MpPepoleKV,
};

//����ҳ���Ӧ�õĴ������򼯣��൱�ڶ��尴��
//֧�ֵ�������������ΪMAX_TOUCH_REGION_NUM
//ϵͳ��ʾ�ʹ�������������ϵ������Ļ���Ͻ�Ϊԭ��(x 0,y 0)�����½�Ϊ(x 320,y 240)
static const IMG_BUTTON_OBJ ImgButtonCon[]={
	//KeyName,ObjID,OptionMask,Tch_x,Tch_y,Tch_w,Tch_h,Img_x,Img_y,BmpPathPrefix,NormalSuf,PressSuf,ReleaseSuf,TransColor},
	{"Off",		MpOffKV,RelMsk,0,271,48,49,0,0,"PowerOff",FatColor(NO_TRANS)},
	{"|<<",		MpPrevOneKV,RelMsk,48,271,48,49,0,0,"PrevOne",FatColor(0xff0000)},
	{"Play",	MpPlayKV,RelMsk,96,271,48,49,0,0,"Play",FatColor(0xff0000)},
	{">>|",		MpNextOneKV,RelMsk,144,271,48,49,0,0,"NextOne",FatColor(0xff0000)},
	{"List",	MpListKV,RelMsk,192,271,48,49,0,0,"FileList",FatColor(NO_TRANS)},
	{"PlayBar",	MpPlayBarKV,PrsMsk|CotMsk|RelMsk|ReVMsk,33,162,174,37,0,0,"",FatColor(NO_TRANS)},

	//Һ�����������ʾ������ĸ���
	{"",MpHomeKV,RelMsk,0,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",MpMessageKV,RelMsk,60,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",MpMusicKV,RelMsk,120,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",MpPepoleKV,RelMsk,180,320,60,30,0,0,"",FatColor(NO_TRANS)},
};

//����ҳ�����Ӧ�õ����Լ�
const PAGE_ATTRIBUTE MusicPage={
	"MusicPage",
	"YuanYin",
	"Music page",
	NORMAL_PAGE,
	0,
	
	{
		sizeof(ImgButtonCon)/sizeof(IMG_BUTTON_OBJ), //size of touch region array
		0,//sizeof(CharButtonCon)/sizeof(CHAR_BUTTON_OBJ), //size of touch region array,
	},
	ImgButtonCon,
	NULL,
	
	SystemEventHandler,
	PeripheralsHandler,
	Bit(Perip_KeyPress)|Bit(Perip_KeyRelease)|Bit(Perip_RtcMin)|Bit(Perip_MscPlay)|Bit(Perip_MscPause)|
	Bit(Perip_MscContinue)|Bit(Perip_Timer)|Bit(Perip_RtcAlarm),
	ButtonHandler,
};

//-----------------------��ҳ�Զ����������-----------------------
#define MAX_VOL_STEP 10
static u8 gVolStep;
static const unsigned char VolumeTable[MAX_VOL_STEP]={0xfe,0x80,0x70,0x60,0x50,0x40,0x30,0x20,0x10,0};

#define MUSIC_DIR_PATH "Music"
#define MAX_MUSIC_NUM 256

static u8 *gpMusicPath;

#define MUSIC_PAGE_BG_COLOR 0x404040 //����ɫ

//������
static const unsigned char ProgBarDot[]={0x38,0x7C,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0x7C,0x38};
#define PRO_BAR_START_X 43
#define PRO_BAR_START_Y 177
#define PRO_BAR_WIDTH 147
#define PRO_BAR_HIGHT 7
#define PRO_BAR_COLOR 0xb9b9b9
#define PRO_BAR_HI_COLOR 0xa7d94f
#define PRO_BAR_DOT_COLOR 0xffffff
#define PRO_BAR_DOT_START_Y PRO_BAR_START_Y-3
#define PRO_BAR_DOT_WIDTH 7
#define PRO_BAR_DOT_HIGHT 13


//-----------------------��ҳ�Զ��庯��-----------------------
//static const char Week[][4]={"һ","��","��","��","��","��","��"};
static void DispTime(void)
{
	RTC_TIME Time;
	char TimeMsg[64];
		
	RTC_GetTime(&Time);
	sprintf(TimeMsg,"%02d:%02d",Time.hour,Time.min);
	//Debug("%04d��%02d��%02d�� ����%d %02d:%02d\n\r",Time.year,Time.mon,Time.day,Time.week+1,Time.hour,Time.min,Time.sec);

	DrawTitle1(ASC14B_FONT,(void *)TimeMsg,240-strlen(TimeMsg)*GUI_ASC14B_ASCII_WIDTH-4,strlen(TimeMsg),FatColor(0xe0e0e0));
}

//���²���ʱ�����ʱ�䣬ֵΪ-1�������¶�Ӧ��
//��λ ��
static void UpdataPlayTime(u16 PlayTime,u16 TotalTime,bool DispTotalTime)
{
	GUI_REGION DrawRegion;
	u8 TimeStr[8];

	DrawRegion.x=PRO_BAR_START_X-36;
	DrawRegion.y=PRO_BAR_START_Y-3;
	DrawRegion.w=30;
	DrawRegion.h=12;
	DrawRegion.Space=0x00;

	if(PlayTime<6000)//д����ʱ��
	{
		if(PlayTime>TotalTime) PlayTime=TotalTime;
		sprintf((void *)TimeStr,"%02d:%02d",PlayTime/60,PlayTime%60);
		DrawRegion.Color=FatColor(MUSIC_PAGE_BG_COLOR);
		Gui_FillBlock(&DrawRegion);
		DrawRegion.Color=FatColor(PRO_BAR_COLOR);
		Gui_DrawFont(GBK12_FONT,TimeStr,&DrawRegion);
	}
	
	if((TotalTime<6000)&&(DispTotalTime))//������ʱ��
	{
		sprintf((void *)TimeStr,"%02d:%02d",TotalTime/60,TotalTime%60);
		DrawRegion.x=PRO_BAR_START_X+PRO_BAR_WIDTH+PRO_BAR_DOT_WIDTH+8;
		DrawRegion.Color=FatColor(MUSIC_PAGE_BG_COLOR);
		Gui_FillBlock(&DrawRegion);
		DrawRegion.Color=FatColor(PRO_BAR_COLOR);
		Gui_DrawFont(GBK12_FONT,TimeStr,&DrawRegion);
	}
}

//���ݲ������ڽ������������ؽ��Ȱٷֱ�
//CMD:
#define PROGBAR_INIT 0//��ʼ����������ʱ��ͽ��ȸ���NewPlayTime������NewPixelX������Ч
#define PROGBAR_TIME_INC 1//ʱ������ģʽ��NewPixelX��PlayTime������Ч��ÿ����һ�Σ�����ʱ��һ�Σ������½��������غͰٷֱ�
#define PROGBAR_USER_IN   2//�û�����ģʽ��NewPixelX����Ϊ���ص�x���꣬�����û���������ص��������ٷֱȺ͸���ʱ�䣬PlayTime������Ч
static u8 DispProgressBar(u8 Cmd,u16 TotalTime,u16 NewPixelX,u32 NewPlayTime)
{
	static u16 OldPixelX;
	static u16 PlayTime;
	GUI_REGION DrawRegion;

	switch(Cmd)
	{
		case PROGBAR_INIT: //��ʼ��������
			//��ʼ������������
			DrawRegion.x=PRO_BAR_START_X;
			DrawRegion.y=PRO_BAR_DOT_START_Y;
			DrawRegion.w=PRO_BAR_WIDTH+PRO_BAR_DOT_WIDTH;
			DrawRegion.h=PRO_BAR_DOT_HIGHT;
			DrawRegion.Color=FatColor(MUSIC_PAGE_BG_COLOR);
			Gui_FillBlock(&DrawRegion);
			
			//��ʼ��������
			DrawRegion.y=PRO_BAR_START_Y;
			DrawRegion.h=PRO_BAR_HIGHT;
			DrawRegion.Color=FatColor(PRO_BAR_COLOR);
			Gui_FillBlock(&DrawRegion);
			OldPixelX=PRO_BAR_START_X;

			//��ʼ����ʱ��
			PlayTime=NewPlayTime;
			UpdataPlayTime(PlayTime,TotalTime,TRUE);//���²���ʱ�����ʱ��Ϊ��ʼֵ
			PlayTime--;
		case PROGBAR_TIME_INC://ʱ��ģʽ������ʱ�����������
			PlayTime++;//����ʱ������
			UpdataPlayTime(PlayTime,TotalTime,FALSE);//���²���ʱ��
			if(TotalTime)
				NewPixelX=PlayTime*PRO_BAR_WIDTH/TotalTime+PRO_BAR_START_X;
			else
				NewPixelX=PRO_BAR_START_X;

			//ȡ��Χ
			if(NewPixelX<PRO_BAR_START_X) NewPixelX=PRO_BAR_START_X;
			else if(NewPixelX>(PRO_BAR_START_X+PRO_BAR_WIDTH)) NewPixelX=(PRO_BAR_START_X+PRO_BAR_WIDTH);
			break;
		case PROGBAR_USER_IN://�û�����ģʽ�������û����ȷ��ʱ��ͽ���
			//ȡ��Χ
			if(NewPixelX<PRO_BAR_START_X) NewPixelX=PRO_BAR_START_X;
			else if(NewPixelX>(PRO_BAR_START_X+PRO_BAR_WIDTH)) NewPixelX=(PRO_BAR_START_X+PRO_BAR_WIDTH);

			PlayTime=(NewPixelX-PRO_BAR_START_X)*TotalTime/PRO_BAR_WIDTH;//����ʱ�����
			UpdataPlayTime(PlayTime,TotalTime,FALSE);//���²���ʱ��
			break;
	}
	
	//���ò����߶�
 	if(((Cmd==PROGBAR_USER_IN)&&(NewPixelX>OldPixelX-2)&&(NewPixelX<OldPixelX+2))||(NewPixelX==OldPixelX))
 		return (NewPixelX-PRO_BAR_START_X)*99/PRO_BAR_WIDTH;
	
	//ȥ���ɽ��ȵ�
	DrawRegion.x=OldPixelX;
	DrawRegion.y=PRO_BAR_DOT_START_Y;
	DrawRegion.w=PRO_BAR_DOT_WIDTH;
	DrawRegion.h=PRO_BAR_DOT_HIGHT;
	DrawRegion.Color=FatColor(MUSIC_PAGE_BG_COLOR);
	Gui_FillBlock(&DrawRegion);
	
	if(OldPixelX<NewPixelX) //����������
	{
		DrawRegion.x=OldPixelX;
		DrawRegion.y=PRO_BAR_START_Y;
		DrawRegion.w=NewPixelX-OldPixelX;
		DrawRegion.h=PRO_BAR_HIGHT;
		DrawRegion.Color=FatColor(PRO_BAR_HI_COLOR);
		Gui_FillBlock(&DrawRegion);
	}
	else if(OldPixelX>NewPixelX) //����������
	{
		DrawRegion.x=NewPixelX+PRO_BAR_DOT_WIDTH;
		DrawRegion.y=PRO_BAR_START_Y;
		DrawRegion.w=OldPixelX-NewPixelX;
		DrawRegion.h=PRO_BAR_HIGHT;
		DrawRegion.Color=FatColor(PRO_BAR_COLOR);
		Gui_FillBlock(&DrawRegion);
	}

	//�����ȵ�
	DrawRegion.x=NewPixelX;
	DrawRegion.y=PRO_BAR_DOT_START_Y;
	DrawRegion.w=PRO_BAR_DOT_WIDTH;
	DrawRegion.h=PRO_BAR_DOT_HIGHT;
	DrawRegion.Color=FatColor(PRO_BAR_DOT_COLOR);
	Gui_PixelFill(ProgBarDot,&DrawRegion);
	
	OldPixelX=NewPixelX;
	
	return (NewPixelX-PRO_BAR_START_X)*99/PRO_BAR_WIDTH;
}

static void DispPlayBtn(void)
{
	if(Q_MusicGetState()==MusicPause)
	{					
		Q_ChangeImgTchImg(MpPlayKV,0);
		Q_PresentTch(MpPlayKV,Tch_Release);
	}
	else
	{
		Q_ChangeImgTchImg(MpPlayKV,'P');
		Q_PresentTch(MpPlayKV,Tch_Release);
	}
}

#define PLAY_NAME_X 10 //�������
#define PLAY_NAME_Y 29
#define INFO_LINES_WIDTH 18 //һ����Ϣ�������
#define INFO_LINES_HIGHT 18 //�и�
#define INFO_LINES_SPACE 0x04 //���

static void DispVolume(u8 Vol)
{
	GUI_REGION DrawRegion;
	u8 Str[16];
	
	//д������ģʽ
	DrawRegion.x=125+60;
	DrawRegion.y=52+90;
	DrawRegion.w=36;
	DrawRegion.h=12;
	DrawRegion.Space=INFO_LINES_SPACE;
	DrawRegion.Color=FatColor(MUSIC_PAGE_BG_COLOR);
	Gui_FillBlock(&DrawRegion);
    DrawRegion.Color=FatColor(0xffffff);
    sprintf((void *)Str,"���� %d",Vol);
	Gui_DrawFont(GBK12_FONT,(void *)Str,&DrawRegion);
}

static void DispPlayInfo(const u8 *Name)
{
	GUI_REGION DrawRegion;
	u8 str[40];
	u8 *pstr;
	u8 DispLines;
	
	//д����
	if(Q_MusicGetInfo()->Title[0]) Name=&Q_MusicGetInfo()->Title[0];

	strcpy((void *)str,(void *)Name);
	pstr=(void *)strstr((void *)str,".mp3"); 
	if(pstr!=NULL) *pstr='\0';
	
	DrawRegion.x=PLAY_NAME_X-1;
	DrawRegion.y=PLAY_NAME_Y-1;
	DrawRegion.w=224;
	DrawRegion.h=14;
	DrawRegion.Space=0;
    DrawRegion.Color=FatColor(MUSIC_PAGE_BG_COLOR);
	Gui_FillBlock(&DrawRegion);

	DrawRegion.x=PLAY_NAME_X-1;
	DrawRegion.y=PLAY_NAME_Y;
    DrawRegion.Color=FatColor(0xffffff);
	Gui_DrawFont(GBK12_FONT,(void *)str,&DrawRegion);

	DrawRegion.x=PLAY_NAME_X+1;
	DrawRegion.y=PLAY_NAME_Y;
	Gui_DrawFont(GBK12_FONT,(void *)str,&DrawRegion);

	DrawRegion.x=PLAY_NAME_X;
	DrawRegion.y=PLAY_NAME_Y-1;
	Gui_DrawFont(GBK12_FONT,(void *)str,&DrawRegion);

	DrawRegion.x=PLAY_NAME_X;
	DrawRegion.y=PLAY_NAME_Y+1;
	Gui_DrawFont(GBK12_FONT,(void *)str,&DrawRegion);
	
	DrawRegion.x=PLAY_NAME_X;
	DrawRegion.y=PLAY_NAME_Y;
    DrawRegion.Color=FatColor(0x000000);
	Gui_DrawFont(GBK12_FONT,(void *)str,&DrawRegion);

	//д��Ϣ����
	DrawRegion.x=0;
	DrawRegion.y=42;
	DrawRegion.w=120;
	DrawRegion.h=128;
	DrawRegion.Color=FatColor(NO_TRANS);
	Gui_Draw24Bmp("Theme/F/MusicPage/Bg/MusicImg.bmp",&DrawRegion);//ר������

	DrawRegion.x=120;
	Gui_Draw24Bmp("Theme/F/MusicPage/Bg/InfoBg.bmp",&DrawRegion);//������Ϣ

	//д������Ϣ
	DrawRegion.x=125;
	DrawRegion.y=52;
	DrawRegion.w=110;
	DrawRegion.h=108-INFO_LINES_HIGHT;
	DrawRegion.Space=INFO_LINES_SPACE;
    DrawRegion.Color=FatColor(0xffffff);
    
    if(Q_MusicGetInfo()->Performer[0])
    {
    	sprintf((void *)str,"�ݳ���:%s",Q_MusicGetInfo()->Performer);
		DispLines=(Gui_DrawFont(GBK12_FONT,(void *)str,&DrawRegion)-1)/INFO_LINES_WIDTH+1;
		DrawRegion.y+=INFO_LINES_HIGHT*DispLines;
		DrawRegion.h-=INFO_LINES_HIGHT*DispLines;
	}
	
    if(Q_MusicGetInfo()->Album[0])
    {
    	sprintf((void *)str,"ר��:%s",Q_MusicGetInfo()->Album);
		DispLines=(Gui_DrawFont(GBK12_FONT,(void *)str,&DrawRegion)-1)/INFO_LINES_WIDTH+1;
		DrawRegion.y+=INFO_LINES_HIGHT*DispLines;
		DrawRegion.h-=INFO_LINES_HIGHT*DispLines;
	}

    if(Q_MusicGetInfo()->Year[0])
    {
    	sprintf((void *)str,"�������:%s",Q_MusicGetInfo()->Year);
		DispLines=(Gui_DrawFont(GBK12_FONT,(void *)str,&DrawRegion)-1)/INFO_LINES_WIDTH+1;
		DrawRegion.y+=INFO_LINES_HIGHT*DispLines;
		DrawRegion.h-=INFO_LINES_HIGHT*DispLines;
	}

	if(Q_MusicGetInfo()->Note[0])
    {
    	sprintf((void *)str,"%s",Q_MusicGetInfo()->Note);
		DispLines=(Gui_DrawFont(GBK12_FONT,(void *)str,&DrawRegion)-1)/INFO_LINES_WIDTH+1;
		DrawRegion.y+=INFO_LINES_HIGHT*DispLines;
		DrawRegion.h-=INFO_LINES_HIGHT*DispLines;
	}

	if(Name[0])	DispVolume(gVolStep);
}

/*************************************************************************************************************************************************************
*�����ʾ
**************************************************************************************************************************************************************/
#define LRC_BOX_X		8
#define LRC_BOX_Y		198
#define LRC_BOX_W		216
#define LRC_BOX_H		(12+4)*4
#define LRC_CHAR_SPACE	0
#define LRC_ROW_SPACE	0
#define LRC_CHAR_COLOR	(FatColor(0xff0000))//�����ɫ
#define MAX_LABEL		128	 //֧�ֵ����ʱ���ǩ��
#define READBUF_SIZE	512	 //����lrc�õ��Ļ����С
#define LRCBUF_SIZE		2048 //����ַ��������С
#define DELAYTIME		1000 //��ʱ����ʱʱ��0.1s
#define MAX_LRCPATH_LEN (MAX_LEN_FILENAME+7)

//��ʶ�������
typedef struct{		
	char LrcPath[MAX_LRCPATH_LEN];//LRC�ļ�·��
	u32 TimeToText[MAX_LABEL][2]; //��һάΪʱ���ǩ��Ӧ��ʱ�䣨��λ�����룩���ڶ�άΪ��ʱ���Ӧ�ĸ��ƫ���������ַ���ָ��
	u16 TimeLabels;				  //��ǰ����ļ�������ʱ���ǩ�� 
	u8  ReadBuf[READBUF_SIZE];    //����lrc�õ��Ķ�����
	u8  LyricBuf[LRCBUF_SIZE];    //����ַ�������
	GUI_REGION LrcRegion;         //��ǰ������ʾ����
}LRCDEF;

typedef struct{
	LRCDEF LRC;
}MUSIC_PAGE_VARS;//����ҳҪ�õ���ȫ�ֱ���ȫ������˽ṹ��
static MUSIC_PAGE_VARS *gpMpVar;

static bool IfReady;
static u32 PauseTime;
static u32 PauseStart;
static u32 PauseEnd;

static void Lrc_Priv_GetLrcFile(void);		   //��õ�ǰ������Ӧ��lrc�ļ�ָ��(����˽�к���)
static bool Lrc_Priv_LrcParse(void);		   //����lrc�ļ���TimeToText(����˽�к���)
static void Lrc_Intf_Init(void); 		       //��ʼ��(����ӿں���)
static bool Lrc_Intf_Show(u32 time);	   	   //��ʾ����ʱ���Ӧ�ĸ��(����ӿں���)

#define MU_CAL_TIME_BEGIN 0
#define MU_CAL_TIME_END   1
//����ʱ�亯�� ����ɶ�ʹ��
static u32 MU_CalTimMs(u32 BeginOrEnd,const char *FunctionName){
	static u32 count;
	if( BeginOrEnd==MU_CAL_TIME_BEGIN ){
		count=OS_GetCurrentSysMs();
		Debug("Begin calculate the time spend by %s\r\n",FunctionName);
		return 0;
	}
	else if( BeginOrEnd==MU_CAL_TIME_END ){
		count = OS_GetCurrentSysMs()-count;
		Debug("%s spend %d ms to run\r\n",FunctionName,count);
		return count;
	}
	return 0;
}
void Lrc_Priv_GetLrcFile(void){//��õ�ǰ����mp3��Ӧ��lrc�ļ�·��
	u8 i=0;	
	if(gpMusicPath[0]==0)
		return;
	while(gpMusicPath[i]!='.'){
		gpMpVar->LRC.LrcPath[i]=gpMusicPath[i];
		i++;
		if(i>(MAX_LEN_FILENAME+7))
			return;
	}
	gpMpVar->LRC.LrcPath[i++]='.';
	gpMpVar->LRC.LrcPath[i++]='l';
	gpMpVar->LRC.LrcPath[i++]='r';
	gpMpVar->LRC.LrcPath[i++]='c';
	gpMpVar->LRC.LrcPath[i]=0;
	Debug("��ǰmp3��Ӧ�ĸ���ļ���%s\r\n",gpMpVar->LRC.LrcPath);
}

bool Lrc_Priv_LrcParse(void){//����lrc�ļ�
	FS_FILE* Fp_Lrc;
	u32      CutPos=0,ReadByte,TempTime,index=0,repeat=0,TempOffset=0,i=0,j,k=0;
	bool     ON=FALSE,IfTimeLabel=FALSE;
	if (( Fp_Lrc = FS_FOpen((void *)_TEXT(gpMpVar->LRC.LrcPath), FA_OPEN_EXISTING | FA_READ) )== 0 ){ 
		Debug("�򿪸���ļ�%sʧ��\n\r",gpMpVar->LRC.LrcPath);
		Gui_DrawFont(GBK12_FONT,"��ǰ�����޶�Ӧ����ļ�\r\n",&gpMpVar->LRC.LrcRegion);
		return FALSE;
	}
	while(1){
		FS_FSeek(Fp_Lrc,CutPos,FS_SEEK_SET);
		if((ReadByte=FS_FRead(gpMpVar->LRC.ReadBuf, 512, 1,Fp_Lrc))==0)
			break;//�����ļ��˳�ѭ��
		index=0;  //���������� 
		
		while(index<ReadByte){//ͨ�����whileѭ����ʱ������Ӧ��ƫ������Ϣ����TimeToText��ά����
			if(gpMpVar->LRC.ReadBuf[index]=='[')
				ON=TRUE;
			if(gpMpVar->LRC.ReadBuf[index]==']'){
				TempOffset = CutPos + index +1;
				ON=FALSE;
			}
			if(ON==TRUE){
				if(gpMpVar->LRC.ReadBuf[index]==':'){
					if( gpMpVar->LRC.ReadBuf[index-2]<='9' && gpMpVar->LRC.ReadBuf[index-2]>='0' && gpMpVar->LRC.ReadBuf[index-1]<='9' && gpMpVar->LRC.ReadBuf[index-1]>='0' && gpMpVar->LRC.ReadBuf[index+1]<='9' && gpMpVar->LRC.ReadBuf[index+1]>='0' && gpMpVar->LRC.ReadBuf[index+2]<='9' && gpMpVar->LRC.ReadBuf[index+2]>='0'){
						IfTimeLabel=TRUE;
						TempTime = (gpMpVar->LRC.ReadBuf[index-2]-'0')*60000 + (gpMpVar->LRC.ReadBuf[index-1]-'0')*6000 + (gpMpVar->LRC.ReadBuf[index+1]-'0')*1000 + (gpMpVar->LRC.ReadBuf[index+2]-'0')*100;
						if( gpMpVar->LRC.ReadBuf[index+3]=='.' || gpMpVar->LRC.ReadBuf[index+3]==':' )
							TempTime = TempTime + (gpMpVar->LRC.ReadBuf[index+4]-'0')*10 + (gpMpVar->LRC.ReadBuf[index+5]-'0');
						gpMpVar->LRC.TimeToText[gpMpVar->LRC.TimeLabels][0]=TempTime;
						gpMpVar->LRC.TimeLabels++;
						repeat++;							
					}			
				}
			}
			if( gpMpVar->LRC.ReadBuf[index]=='\n'){
				if( IfTimeLabel==TRUE ){
					IfTimeLabel=FALSE;
					while(repeat){
						repeat--;
						gpMpVar->LRC.TimeToText[gpMpVar->LRC.TimeLabels-repeat-1][1]=TempOffset;
					}
					repeat=0;
				}
				if(index>=256){
					CutPos+=index+1;
					break;
				}
			}			
			index++;
			if(index==ReadByte)
				CutPos+=(index+1);
		}		
	}
	while( i < gpMpVar->LRC.TimeLabels ){//���whileѭ��������TimeToText��ά���еĵڶ�ά��lrc������ʷ���LyricBuf�в���TimeToText��ά���еĵڶ�ά����ΪLyricBuf��ÿ�����ַ������׵�ַ
		FS_FSeek(Fp_Lrc,gpMpVar->LRC.TimeToText[i][1],FS_SEEK_SET);
		FS_FRead(&gpMpVar->LRC.LyricBuf[k], 100, 1,Fp_Lrc);
		j=0;
		while(gpMpVar->LRC.LyricBuf[k+j]!='\r')		
			j++;
		gpMpVar->LRC.LyricBuf[k+j]=0;
		gpMpVar->LRC.TimeToText[i][1]=(u32)&gpMpVar->LRC.LyricBuf[k];
		Debug("LRC.TimeToText[%d][0] :%d\n\r",i,(u8 *)gpMpVar->LRC.TimeToText[i][0]);
		Debug("LRC.TimeToText[%d][1] :%s\n\r",i,(u8 *)gpMpVar->LRC.TimeToText[i][1]);

		k=k+j+1;
		i++;
	}
	if(FS_FClose(Fp_Lrc)==-1){
		Q_ErrorStopScreen("�ļ�ϵͳ��������!!!\n\r");
	}
	return TRUE;
}

void Lrc_Intf_Init(void){//��ʼ��LRC����
	gpMpVar->LRC.TimeLabels=0;
	gpMpVar->LRC.LrcRegion.x=LRC_BOX_X;
	gpMpVar->LRC.LrcRegion.y=LRC_BOX_Y;
	gpMpVar->LRC.LrcRegion.w=LRC_BOX_W;
	gpMpVar->LRC.LrcRegion.h=12;
	gpMpVar->LRC.LrcRegion.Space=(LRC_CHAR_SPACE<<4)|(LRC_ROW_SPACE);
	gpMpVar->LRC.LrcRegion.Color=LRC_CHAR_COLOR;
	Lrc_Priv_GetLrcFile();
	MU_CalTimMs(MU_CAL_TIME_BEGIN,"Lrc_Priv_LrcParse()");
	Lrc_Priv_LrcParse();
	MU_CalTimMs(MU_CAL_TIME_END,"Lrc_Priv_LrcParse()");
}

static void RtoY(u32 x,u32 y,u32 w,u32 h){//��һ�����ڵĺ�ɫ��Ϊ��ɫ
	u32 x0;
	u32 y0;
	u32 x1;
	u32 y1;
	u16 Color;
	x1=x+w;
	y1=y+h;	
	for(y0=y;y0<y1;y0++){
		for(x0=x;x0<x1;x0++){
			Color=Gui_ReadPixel16Bit(x0,y0);
			if(Color==FatColor(0xff0000))
				Gui_WritePixel(x0,y0,FatColor(0xffe000));
		}
	}
}

bool Lrc_Intf_Show(u32 time){//��ʾ����ʱ���Ӧ�ĸ��
	GUI_REGION DrawRegion;
	u32 i=0,j,n,nn,nnn,TempTime;
	static u32 Len=0;
	static u32 Tim=0xffffffff;
	static u32 k=0;
	if( gpMpVar->LRC.TimeLabels==0 ){
		return FALSE;
	}
	while( gpMpVar->LRC.TimeToText[i][0]/10 != time/100 ){
		if( i>=gpMpVar->LRC.TimeLabels ){
			RtoY(gpMpVar->LRC.LrcRegion.x,gpMpVar->LRC.LrcRegion.y,10*k*Len/Tim,gpMpVar->LRC.LrcRegion.h);
			k++;
			return FALSE;
		}
		i++;	
	}	
	DrawRegion.x=LRC_BOX_X;
	DrawRegion.y=LRC_BOX_Y;
	DrawRegion.w=LRC_BOX_W;
	DrawRegion.h=LRC_BOX_H;
	DrawRegion.Color=FatColor(MUSIC_PAGE_BG_COLOR);
	Gui_FillBlock(&DrawRegion);		
		
	k=0;
	if(*(u8 *)(gpMpVar->LRC.TimeToText[i][1])==0)
		Gui_DrawFont(GBK12_FONT,"......",&gpMpVar->LRC.LrcRegion);
	else
		Gui_DrawFont(GBK12_FONT,(u8 *)gpMpVar->LRC.TimeToText[i][1],&gpMpVar->LRC.LrcRegion);
	Len=strlen((char *)gpMpVar->LRC.TimeToText[i][1])*6;
	Tim=0xffffffff;
	for(j=0,n=0;j<gpMpVar->LRC.TimeLabels;j++){
		if(j==i)
			continue;
		if( gpMpVar->LRC.TimeToText[j][0] < gpMpVar->LRC.TimeToText[i][0] )
			continue;
		if((gpMpVar->LRC.TimeToText[j][0] - gpMpVar->LRC.TimeToText[i][0])<Tim){
			Tim=gpMpVar->LRC.TimeToText[j][0] - gpMpVar->LRC.TimeToText[i][0];
			n=j;
		}
	}
	TempTime=0xffffffff;
	for(j=0,nn=0;j<gpMpVar->LRC.TimeLabels;j++){
		if(j==n)
			continue;
		if( gpMpVar->LRC.TimeToText[j][0] < gpMpVar->LRC.TimeToText[n][0] )
			continue;
		if((gpMpVar->LRC.TimeToText[j][0] - gpMpVar->LRC.TimeToText[n][0])<TempTime){
			TempTime=gpMpVar->LRC.TimeToText[j][0] - gpMpVar->LRC.TimeToText[n][0];
			nn=j;
		}
	}
	TempTime=0xffffffff;
	for(j=0,nnn=0;j<gpMpVar->LRC.TimeLabels;j++){
		if(j==nn)
			continue;
		if( gpMpVar->LRC.TimeToText[j][0] < gpMpVar->LRC.TimeToText[nn][0] )
			continue;
		if((gpMpVar->LRC.TimeToText[j][0] - gpMpVar->LRC.TimeToText[nn][0])<TempTime){
			TempTime=gpMpVar->LRC.TimeToText[j][0] - gpMpVar->LRC.TimeToText[nn][0];
			nnn=j;
		}
	}
	DrawRegion.x=LRC_BOX_X;
	DrawRegion.y=LRC_BOX_Y+16;
	DrawRegion.w=LRC_BOX_W;
	DrawRegion.h=12;
	DrawRegion.Space=(LRC_CHAR_SPACE<<4)|(LRC_ROW_SPACE);
	DrawRegion.Color=FatColor(0xb9b9b9);
	if(n>0){
		if(*(u8 *)(gpMpVar->LRC.TimeToText[n][1])==0)
			Gui_DrawFont(GBK12_FONT,"......",&DrawRegion);
		else
			Gui_DrawFont(GBK12_FONT,(u8 *)gpMpVar->LRC.TimeToText[n][1],&DrawRegion);
	}
	DrawRegion.y=LRC_BOX_Y+16*2;
	if(nn>1){
		if(*(u8 *)(gpMpVar->LRC.TimeToText[nn][1])==0)
			Gui_DrawFont(GBK12_FONT,"......",&DrawRegion);
		else
			Gui_DrawFont(GBK12_FONT,(u8 *)gpMpVar->LRC.TimeToText[nn][1],&DrawRegion);
	}
	DrawRegion.y=LRC_BOX_Y+16*3;
	if(nnn>2){
		if(*(u8 *)(gpMpVar->LRC.TimeToText[nnn][1])==0)
			Gui_DrawFont(GBK12_FONT,"......",&DrawRegion);
		else
			Gui_DrawFont(GBK12_FONT,(u8 *)gpMpVar->LRC.TimeToText[nnn][1],&DrawRegion);
	}
	Debug("[%d%d:%d%d.%d0]%s\n\r",time/600000,time%600000/60000,time%60000/10000,time%10000/1000,time%1000/100,(u8 *)gpMpVar->LRC.TimeToText[i][1]);
	return TRUE;
}

bool Lrc_Intf_ObscShow(u32 time){//ģ��ƥ�䲢��ʾ����ʱ���Ӧ�ĸ��
	GUI_REGION DrawRegion;
	u32 i=0,j,n,nn,nnn,TempTime;
	if( gpMpVar->LRC.TimeLabels==0 ){
		return FALSE;
	}
	TempTime=0xffffffff;
	for(j=0;j<gpMpVar->LRC.TimeLabels;j++){
		if( time/10 < gpMpVar->LRC.TimeToText[j][0] )
			continue;
		if( time/10 - gpMpVar->LRC.TimeToText[j][0] < TempTime){
			TempTime = time/10 - gpMpVar->LRC.TimeToText[j][0];
			i=j;
		}
	}	
	
	DrawRegion.x=LRC_BOX_X;
	DrawRegion.y=LRC_BOX_Y;
	DrawRegion.w=LRC_BOX_W;
	DrawRegion.h=LRC_BOX_H;
	DrawRegion.Color=FatColor(MUSIC_PAGE_BG_COLOR);
	Gui_FillBlock(&DrawRegion);		
	
	if(*(u8 *)(gpMpVar->LRC.TimeToText[i][1])==0)
		Gui_DrawFont(GBK12_FONT,"......",&gpMpVar->LRC.LrcRegion);
	else
		Gui_DrawFont(GBK12_FONT,(u8 *)gpMpVar->LRC.TimeToText[i][1],&gpMpVar->LRC.LrcRegion);
	TempTime=0xffffffff;
	for(j=0,n=0;j<gpMpVar->LRC.TimeLabels;j++){
		if(j==i)
			continue;
		if( gpMpVar->LRC.TimeToText[j][0] < gpMpVar->LRC.TimeToText[i][0] )
			continue;
		if((gpMpVar->LRC.TimeToText[j][0] - gpMpVar->LRC.TimeToText[i][0])<TempTime){
			TempTime=gpMpVar->LRC.TimeToText[j][0] - gpMpVar->LRC.TimeToText[i][0];
			n=j;
		}
	}
	TempTime=0xffffffff;
	for(j=0,nn=0;j<gpMpVar->LRC.TimeLabels;j++){
		if(j==n)
			continue;
		if( gpMpVar->LRC.TimeToText[j][0] < gpMpVar->LRC.TimeToText[n][0] )
			continue;
		if((gpMpVar->LRC.TimeToText[j][0] - gpMpVar->LRC.TimeToText[n][0])<TempTime){
			TempTime=gpMpVar->LRC.TimeToText[j][0] - gpMpVar->LRC.TimeToText[n][0];
			nn=j;
		}
	}
	TempTime=0xffffffff;
	for(j=0,nnn=0;j<gpMpVar->LRC.TimeLabels;j++){
		if(j==nn)
			continue;
		if( gpMpVar->LRC.TimeToText[j][0] < gpMpVar->LRC.TimeToText[nn][0] )
			continue;
		if((gpMpVar->LRC.TimeToText[j][0] - gpMpVar->LRC.TimeToText[nn][0])<TempTime){
			TempTime=gpMpVar->LRC.TimeToText[j][0] - gpMpVar->LRC.TimeToText[nn][0];
			nnn=j;
		}
	}
	DrawRegion.x=LRC_BOX_X;
	DrawRegion.y=LRC_BOX_Y+16;
	DrawRegion.w=LRC_BOX_W;
	DrawRegion.h=12;
	DrawRegion.Space=(LRC_CHAR_SPACE<<4)|(LRC_ROW_SPACE);
	DrawRegion.Color=FatColor(0xb9b9b9);
	if(n>0){
		if(*(u8 *)(gpMpVar->LRC.TimeToText[n][1])==0)
			Gui_DrawFont(GBK12_FONT,"......",&DrawRegion);
		else
			Gui_DrawFont(GBK12_FONT,(u8 *)gpMpVar->LRC.TimeToText[n][1],&DrawRegion);
	}
	DrawRegion.y=LRC_BOX_Y+16*2;
	if(nn>1){
		if(*(u8 *)(gpMpVar->LRC.TimeToText[nn][1])==0)
			Gui_DrawFont(GBK12_FONT,"......",&DrawRegion);
		else
			Gui_DrawFont(GBK12_FONT,(u8 *)gpMpVar->LRC.TimeToText[nn][1],&DrawRegion);
	}
	DrawRegion.y=LRC_BOX_Y+16*3;
	if(nnn>2){
		if(*(u8 *)(gpMpVar->LRC.TimeToText[nnn][1])==0)
			Gui_DrawFont(GBK12_FONT,"......",&DrawRegion);
		else
			Gui_DrawFont(GBK12_FONT,(u8 *)gpMpVar->LRC.TimeToText[nnn][1],&DrawRegion);
	}
	Debug("[%d%d:%d%d.%d0]%s\n\r",time/600000,time%600000/60000,time%60000/10000,time%10000/1000,time%1000/100,(u8 *)gpMpVar->LRC.TimeToText[i][1]);
	return TRUE;
}
/*************************************************************************************************************************************************************
*�����ʾ
**************************************************************************************************************************************************************/

//-----------------------��ҳϵͳ����----------------------
//��ÿ�α�ҳ����ϵͳ��ǰҳ��ʱ����õĳ���
//IntParam:TRUE:pSysParam������,FALSE:pSysParam������
//pSysParam:�����Ҫ���ŵĸ���·��
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam)
{
	GUI_REGION DrawRegion;
	MUSIC_EVENT MusicEvent;
	int i;
	switch(SysEvent)
	{
		case Sys_PreGotoPage:
			break;
		case Sys_PageInit:	
			gpMpVar=(MUSIC_PAGE_VARS *)Q_PageMallco(sizeof(MUSIC_PAGE_VARS));//����ռ�	
			if(gpMpVar==0){
				Q_ErrorStopScreen("gpMpVar malloc fail !\n\r");
			}	
		case Sys_SubPageReturn:
			if(Q_GetPageEntries()==1)  //ҳ���״δ���ʱ��Ҫ��������
			{
				Q_EnableGobalPeripEvent(Perip_MscStop,GobalMusicStopHandler);
				gVolStep=5;
				IfReady=FALSE;
			}
			gpMusicPath=Q_MusicGetPath();
			if(IfReady==TRUE){
				Lrc_Intf_Init();
				Q_TimSet(Q_TIM1,DELAYTIME,100,(bool)1);
				Q_SetPeripEvt(PRID_Current,Bit(Perip_RtcSec));//�����¼���Ӧ
				Lrc_Intf_ObscShow(Q_MusicGetPlayMs()-PauseTime);
			}
			//��������
			DrawRegion.x=DrawRegion.y=0;
			DrawRegion.w=240;
			DrawRegion.h=21;
			DrawRegion.Color=FatColor(NO_TRANS);
			Gui_FillImgArray((u8 *)gImage_StatusBar1,1,21,&DrawRegion);	
			DrawTitle1(ASC14B_FONT,"Q-Player",4,strlen("Q-Player"),FatColor(0xe0e0e0));		//д����	
			
			//������
			DrawRegion.x=0;
			DrawRegion.y=21;
			DrawRegion.w=240;
			DrawRegion.h=320-21-49;
			DrawRegion.Color=FatColor(MUSIC_PAGE_BG_COLOR);
			Gui_FillBlock(&DrawRegion);

			DispTime();
			break;	
		case Sys_TouchSetOk:
		case Sys_TouchSetOk_SR:
			DrawRegion.x=0;
			DrawRegion.y=42;
			DrawRegion.w=120;
			DrawRegion.h=128;
			DrawRegion.Color=FatColor(NO_TRANS);
			Gui_Draw24Bmp("Theme/F/MusicPage/Bg/MusicImg.bmp",&DrawRegion);//ר������

			DrawRegion.x=120;
			Gui_Draw24Bmp("Theme/F/MusicPage/Bg/InfoBg.bmp",&DrawRegion);//������Ϣ
			
			if((SysEvent==Sys_TouchSetOk_SR)&&(IntParam==TRUE))//����ҳ�淵��
			{
				Debug("Select %s\n\r",gpMusicPath);
				
				for(i=strlen((void *)gpMusicPath);i;i--) if(gpMusicPath[i]=='/')break;

				if(CheckMultiSuffix(&gpMusicPath[++i],".mp3"))
				{//������ļ�����������
				    MusicEvent.Cmd=MusicPlay;
				    MusicEvent.ParamU16=0;
					MusicEvent.pFilePath=(u8 *)gpMusicPath;
					MusicEvent.pParam=NULL;
					Q_MusicSync(&MusicEvent);

					//OS_TaskDelayMs(100);
					MusicEvent.Cmd=MusicSetIo;
					MusicEvent.ParamU16=0x01;
					Q_MusicSync(&MusicEvent);
					IfReady=TRUE;
				}
			}
			else //�Ǵ���ҳ�淵��
			{
				for(i=strlen((void *)gpMusicPath);i;i--) if(gpMusicPath[i]=='/')break;
				DispPlayInfo(&gpMusicPath[++i]);
				DispPlayBtn();
				if(Q_MusicGetState()!=MusicNothing) 
				{
					DispProgressBar(PROGBAR_INIT,Q_MusicGetInfo()->Duration,0,Q_MusicGetPlayMs()/1000);
				}
			}
			break;
		case Sys_PageClean:
			Q_TimSet(Q_TIM1,0,0,(bool)1);
			Q_PageFree(gpMpVar);
			break;

		case Sys_PreSubPage:
			Q_TimSet(Q_TIM1,0,0,(bool)1);
			break;
		
		default:
			Debug("You should not here! %d\n\r",SysEvent);
			//while(1);
	}
	
	return 0;
}
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam)
{
	MUSIC_EVENT MusicEvent;
	GUI_REGION DrawRegion;
	int i;
	switch(PeripEvent)
	{
	   case Perip_KeyPress:
			switch(IntParam){
				case MpExtiKeyEnter:
					break;
				case MpExtiKeyUp:
					Q_PresentTch(MpVolumeUpKV,Tch_Press);
					break;
				case MpExtiKeyDown:
					Q_PresentTch(MpVolumeDownKV,Tch_Press);
					break; 
			}break;
		case Perip_KeyRelease:
			switch(IntParam){
				case MpExtiKeyEnter:
					PrtScreen();
					break;
				case MpExtiKeyUp:
				    Q_PresentTch(MpVolumeUpKV,Tch_Release);
					if(gVolStep<MAX_VOL_STEP-1)
					{
						gVolStep++;			
						//WaitMusicHandlerIdle(100);
						MusicEvent.Cmd=MusicChangeVol;
						MusicEvent.ParamU16=(VolumeTable[gVolStep]<<8)|VolumeTable[gVolStep];
						Q_MusicSync(&MusicEvent);		
						DispVolume(gVolStep);
					}	
					break;
				case MpExtiKeyDown:
					Q_PresentTch(MpVolumeDownKV,Tch_Release);
					if(gVolStep>0)
					{
						gVolStep--;			
						//WaitMusicHandlerIdle(100);
						MusicEvent.Cmd=MusicChangeVol;
						MusicEvent.ParamU16=(VolumeTable[gVolStep]<<8)|VolumeTable[gVolStep];
						Q_MusicSync(&MusicEvent);
						DispVolume(gVolStep);
					}
					break; 
			}break;


		case Perip_RtcMin:
			DispTime();
		case Perip_RtcSec:
			if(Q_MusicGetState()==MusicContinue) DispProgressBar(PROGBAR_TIME_INC,Q_MusicGetInfo()->Duration,0,0);
			break;
		case Perip_MscPlay:
			Debug("Perip_MscPlay\n\r");
			DrawRegion.x=LRC_BOX_X;
			DrawRegion.y=LRC_BOX_Y;
			DrawRegion.w=LRC_BOX_W;
			DrawRegion.h=LRC_BOX_H;
			DrawRegion.Color=FatColor(MUSIC_PAGE_BG_COLOR);
			Gui_FillBlock(&DrawRegion);		
			
			Lrc_Intf_Init();
			PauseTime=0;
			DispProgressBar(PROGBAR_INIT,Q_MusicGetInfo()->Duration,0,0);
			for(i=strlen((void *)gpMusicPath);i;i--) if(gpMusicPath[i]=='/')break;
			DispPlayInfo(&gpMusicPath[++i]);
			DispPlayBtn();
			Q_TimSet(Q_TIM1,DELAYTIME,100,(bool)1);
			Q_SetPeripEvt(PRID_Current,Bit(Perip_RtcSec));//�����¼���Ӧ
			break;
		case Perip_MscPause:
			Debug("Perip_MscPause\n\r");
			PauseStart=Q_MusicGetPlayMs();
			DispPlayBtn();
			Q_TimSet(Q_TIM1,0,0,(bool)1);
			break;
		case Perip_MscContinue:
			Debug("Perip_MscContinue\n\r");
			PauseEnd=Q_MusicGetPlayMs();
			PauseTime+=(PauseEnd-PauseStart);
			DispPlayBtn();
			Q_TimSet(Q_TIM1,DELAYTIME,100,(bool)1);
			break;
		case Perip_Timer:
			//MU_CalTimMs(MU_CAL_TIME_BEGIN,"Lrc_Intf_Show()");
			Lrc_Intf_Show(Q_MusicGetPlayMs()-PauseTime);
			//MU_CalTimMs(MU_CAL_TIME_END,"Lrc_Intf_Show()");
			break;	
		case Perip_RtcAlarm:
			Debug("Alarm\n\r");
			break;
	}

	return 0;
}
static SYS_MSG GobalMusicStopHandler(PERIP_EVT PeripEvent,int intParam, void *pParam)
{
	MUSIC_EVENT MusicEvent;
	FILELIST_SET FLPS;
	
	Debug("Perip_MscStop\n\r");//break;
	if(Q_MusicGetPlayMs()>1000)
	{
		Debug("Now:%s\n\r",gpMusicPath);
		FLPS.CallBackRid=Q_FindRidByPageName(NULL);
		FLPS.pPathBuf=gpMusicPath;
		FLPS.pSuffixStr=".mp3";
		if((Q_GotoPage(GotoSubPage,"FileListPage",FL_FindNext|FL_NoListBuf,&FLPS)&SM_STATE_MASK)==SM_State_OK)
		{
			if(CheckMultiSuffix(gpMusicPath,".mp3"))
			{
				Debug("Next:%s\n\r",gpMusicPath);

			    MusicEvent.Cmd=MusicPlay;
				MusicEvent.pFilePath=(u8 *)gpMusicPath;
				MusicEvent.ParamU16=0;
				Q_MusicSync(&MusicEvent);

				//OS_TaskDelayMs(100);
				MusicEvent.Cmd=MusicSetIo;
				MusicEvent.ParamU16=0x01;
				Q_MusicSync(&MusicEvent);
			}
			else
			{
				if((Q_GotoPage(GotoSubPage,"FileListPage",FL_FindFrist|FL_NoListBuf,&FLPS)&SM_STATE_MASK)==SM_State_OK)
				{
					Debug("First:%s\n\r",gpMusicPath);

				    MusicEvent.Cmd=MusicPlay;
					MusicEvent.pFilePath=(u8 *)gpMusicPath;
					MusicEvent.ParamU16=0;
					Q_MusicSync(&MusicEvent);

					//OS_TaskDelayMs(100);
					MusicEvent.Cmd=MusicSetIo;
					MusicEvent.ParamU16=0x01;
					Q_MusicSync(&MusicEvent);
				}
			}
		}			
	}

	return 0;
}

static CO_MSG ButtonHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo)
{
	MUSIC_EVENT MusicEvent;
	FILELIST_SET FLPS;
	//u16 i;
	
	switch(Key)
	{
		case MpVolumeDownKV:		
			if(InEvent!=Tch_Release) return CO_State_OK;
			if(gVolStep>0)
			{
				gVolStep--;			
				//WaitMusicHandlerIdle(100);
				MusicEvent.Cmd=MusicChangeVol;
				MusicEvent.ParamU16=(VolumeTable[gVolStep]<<8)|VolumeTable[gVolStep];
				Q_MusicSync(&MusicEvent);
				DispVolume(gVolStep);
			}
			break;
		case MpVolumeUpKV:
			if(InEvent!=Tch_Release) return CO_State_OK;
			if(gVolStep<MAX_VOL_STEP-1)
			{
				gVolStep++;			
				//WaitMusicHandlerIdle(100);
				MusicEvent.Cmd=MusicChangeVol;
				MusicEvent.ParamU16=(VolumeTable[gVolStep]<<8)|VolumeTable[gVolStep];
				Q_MusicSync(&MusicEvent);		
				DispVolume(gVolStep);
			}	
			break;
		case MpPrevOneKV:
			if(InEvent!=Tch_Release) return CO_State_OK;
			{
				Q_DisableGobalPeripEvent(Perip_MscStop,GobalMusicStopHandler);
				Debug("Now:%s\n\r",gpMusicPath);
				FLPS.CallBackRid=Q_FindRidByPageName(NULL);
				FLPS.pPathBuf=gpMusicPath;
				FLPS.pSuffixStr=".mp3";
				if((Q_GotoPage(GotoSubPage,"FileListPage",FL_FindPrevious|FL_NoListBuf,&FLPS)&SM_STATE_MASK)==SM_State_OK)
				{
					if(CheckMultiSuffix(gpMusicPath,".mp3"))
					{
						Debug("Prev:%s\n\r",gpMusicPath);
					    MusicEvent.Cmd=MusicPlay;
						MusicEvent.pFilePath=(u8 *)gpMusicPath;
						MusicEvent.ParamU16=0;
						Q_MusicSync(&MusicEvent);

						//OS_TaskDelayMs(100);
						MusicEvent.Cmd=MusicSetIo;
						MusicEvent.ParamU16=0x01;
						Q_MusicSync(&MusicEvent);
					}
					else
					{
						if((Q_GotoPage(GotoSubPage,"FileListPage",FL_FindLast|FL_NoListBuf,&FLPS)&SM_STATE_MASK)==SM_State_OK)
						{
							Debug("Last:%s\n\r",gpMusicPath);
						    MusicEvent.Cmd=MusicPlay;
							MusicEvent.pFilePath=(u8 *)gpMusicPath;
							MusicEvent.ParamU16=0;
							Q_MusicSync(&MusicEvent);

							//OS_TaskDelayMs(100);
							MusicEvent.Cmd=MusicSetIo;
							MusicEvent.ParamU16=0x01;
							Q_MusicSync(&MusicEvent);
						}
					}
				}
				Q_EnableGobalPeripEvent(Perip_MscStop,GobalMusicStopHandler);
			}
			break;
		case MpNextOneKV:
			if(InEvent!=Tch_Release) return CO_State_OK;
			{			
				Q_DisableGobalPeripEvent(Perip_MscStop,GobalMusicStopHandler);
				Debug("Now:%s\n\r",gpMusicPath);
				FLPS.CallBackRid=Q_FindRidByPageName(NULL);
				FLPS.pPathBuf=gpMusicPath;
				FLPS.pSuffixStr=".mp3";

				if((Q_GotoPage(GotoSubPage,"FileListPage",FL_FindNext|FL_NoListBuf,&FLPS)&SM_STATE_MASK)==SM_State_OK)
				{
					if(CheckMultiSuffix(gpMusicPath,".mp3"))
					{
						Debug("Next:%s\n\r",gpMusicPath);
						//g/AutoNextFlag=FALSE;
					    MusicEvent.Cmd=MusicPlay;
						MusicEvent.pFilePath=(u8 *)gpMusicPath;
						MusicEvent.ParamU16=0;
						Q_MusicSync(&MusicEvent);

						//OS_TaskDelayMs(100);
						MusicEvent.Cmd=MusicSetIo;
						MusicEvent.ParamU16=0x01;
						Q_MusicSync(&MusicEvent);
					}
					else
					{
						if((Q_GotoPage(GotoSubPage,"FileListPage",FL_FindFrist|FL_NoListBuf,&FLPS)&SM_STATE_MASK)==SM_State_OK)
						{
							Debug("First:%s\n\r",gpMusicPath);
						    MusicEvent.Cmd=MusicPlay;
							MusicEvent.pFilePath=(u8 *)gpMusicPath;
							MusicEvent.ParamU16=0;
							Q_MusicSync(&MusicEvent);
							
							//OS_TaskDelayMs(100);
							MusicEvent.Cmd=MusicSetIo;
							MusicEvent.ParamU16=0x01;
							Q_MusicSync(&MusicEvent);
						}
					}
				}
				Q_EnableGobalPeripEvent(Perip_MscStop,GobalMusicStopHandler);
			}
			break;		
		case MpListKV:
			if(InEvent!=Tch_Release) return 0;
			{
				FLPS.CallBackRid=Q_FindRidByPageName(NULL);
				sprintf((void *)gpMusicPath,"/Music");
				FLPS.pPathBuf=gpMusicPath;
				FLPS.pSuffixStr=".mp3";
				Q_GotoPage(GotoSubPage,"FileListPage",FL_SelectOne|FL_NoListBuf,&FLPS);
			}
			break;	
		case MpPlayKV:
			if(InEvent==Tch_Release)
			{
				if(Q_MusicGetState()==MusicPause)
				{					
				    MusicEvent.Cmd=MusicContinue;
					MusicEvent.pFilePath=(u8 *)gpMusicPath;
					MusicEvent.ParamU16=0;
					Q_MusicSync(&MusicEvent);

					//OS_TaskDelayMs(100);
					MusicEvent.Cmd=MusicSetIo;
					MusicEvent.ParamU16=0x01;
					Q_MusicSync(&MusicEvent);
				}
				else
				{
					MusicEvent.Cmd=MusicPause;
					MusicEvent.pFilePath=(u8 *)gpMusicPath;
					MusicEvent.ParamU16=0;
					Q_MusicSync(&MusicEvent);

					//OS_TaskDelayMs(100);
					MusicEvent.Cmd=MusicSetIo;
					MusicEvent.ParamU16=0x10;
					Q_MusicSync(&MusicEvent);
				}
			}
			break;
		case MpOffKV:
			if(InEvent!=Tch_Release) return 0;
			Q_GotoPage(GotoNewPage,"MainPage",0,NULL);
			break;
		case MpPlayBarKV:
			if(gpMusicPath[0]==0) break;
			if((Q_MusicGetState()!=MusicPlay)&&(Q_MusicGetState()!=MusicPause)&&(Q_MusicGetState()!=MusicContinue)) break;
			if((InEvent==Tch_Release)||(InEvent==Tch_ReleaseVain))
			{
				MusicEvent.Cmd=MusicJump;
				MusicEvent.ParamU16=DispProgressBar(PROGBAR_USER_IN,Q_MusicGetInfo()->Duration,pTouchInfo->x,0);
				Q_MusicSync(&MusicEvent);
				
			}
			else DispProgressBar(PROGBAR_USER_IN,Q_MusicGetInfo()->Duration,pTouchInfo->x,0);
			OS_TaskDelayMs(100);
			Lrc_Intf_ObscShow(Q_MusicGetPlayMs()-PauseTime-100);
			break;
		//default:
			
			//while(1);
	}
	
	return 0;
}




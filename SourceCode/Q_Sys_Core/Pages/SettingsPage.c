/**************** Q-SYS *******************
 * PageName : SettingsPage
 * Author : YourName
 * Version : 
 * Base Q-Sys Version :
 * Description :
 * WebSite : Www.Q-ShareWe.Com
 ***************** Q-SYS *******************/
 
#include "User.h"
#include "Theme.h"
#include "SettingsPage.h"

//��������
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam);
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam);
static CO_MSG ButtonHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo);

//-----------------------��ҳ�Զ����������-----------------------
#define SETTINGS_PAGE_BGCOLOR 0xe0e0e0
#define SETTINGS_PAGE_LINECOLOR 0x979797
#define SETTINGS_PAGE_TITLE_COLOR 0x000000
#define SETTINGS_PAGE_NOTE_COLOR 0x888888
#define SETTINGS_PAGE_NUM_COLOR 0x333333

#define SETTINGS_PAGE_OPTION_STARTX 6
#define SETTINGS_PAGE_OPTION_ENDX 234
#define SETTINGS_PAGE_OPTION_STARTY 34
#define SETTINGS_PAGE_OPTION_ENDY 268
#define SETTINGS_PAGE_OPTION_MARGIN 2
#define SETTINGS_PAGE_OPTION_NAME_H 22
#define SETTINGS_PAGE_OPTION_NOTE_H 14
#define SETTINGS_PAGE_OPTION_LINE_H 1

#define SETTINGS_PAGE_MAX_OPTION_NUM 64 //��ҳ��֧�ֵ����ѡ����
#define SETTINGS_PAGE_MAX_PAGE_NUM 16  //��ҳ��֧�ֵ����ѡ��ҳ��
#define SETTINGS_PAGE_MAX_PAGE_OPTION_NUM 12 //��ҳ�����12��

//-----------------------��ҳϵͳ����������-----------------------
//����ҳ�水����Ҫ�õ���ö�٣�����������Ψһ�ĺ궨��
typedef enum
{
	ExtiKeyDown=EXTI_KEY_VALUE_START,//ϵͳĬ�Ͻ��ⲿ�жϰ������͵���һ����ֵ
	ExtiKeyUp,
	ExtiKeyEnter,
	
	//�����Ӳ���汾��֧�ָ����ⲿ�жϰ����������߼��̣�
	//���Դ�����ֵ��USER_KEY_VALUE_START��ʼ����ǰ��ļ�ֵ��������
	BackKV=USER_KEY_VALUE_START,
	LeftArrowKV,
	DotKV,
	RightArrowKV,
	DoneKV,
	MainZoneKV,
	
	Option1KV,
	Option2KV,
	Option3KV,
	Option4KV,
	Option5KV,
	Option6KV,
	Option7KV,
	Option8KV,
	Option9KV,
	Option10KV,
	Option11KV,
	Option12KV,
	
}SettingsPage_OID;

//����ҳ���Ӧ�õĴ������򼯣��൱�ڶ��尴��
//֧�ֵ�������������ΪMAX_TOUCH_REGION_NUM
//ϵͳ��ʾ�ʹ�������������ϵ������Ļ���Ͻ�Ϊԭ��(x 0,y 0)�����½�Ϊ(x 320,y 240)
static const IMG_BUTTON_OBJ ImgButtonCon[]={
	//KeyName,ObjID,OptionMask,Tch_x,Tch_y,Tch_w,Tch_h,Img_x,Img_y,BmpPathPrefix,NormalSuf,PressSuf,ReleaseSuf,TransColor},
	{"Back",	BackKV,RelMsk|PathMsk,3,287,54,31,0,0,"Common/Btn/Back",FatColor(NO_TRANS)},
	{"<<",		LeftArrowKV,RelMsk|PathMsk,65,287,39,31,0,0,"Common/Btn/LeftArr",FatColor(NO_TRANS)},
	//{"Dot",		DotKV,PathMsk,109,292,22,22,0,0,"Common/Btn/Dot",FatColor(NO_TRANS)},
	{">>",		RightArrowKV,RelMsk|PathMsk,136,287,39,31,0,0,"Common/Btn/RightArr",FatColor(NO_TRANS)},
	{"Done",	DoneKV,RelMsk|PathMsk,183,287,54,31,0,0,"Common/Btn/Done",FatColor(NO_TRANS)},
};

//���ڶ�̬������Ĭ��ֵ
static const IMG_BUTTON_OBJ DefaultTch={"",Option1KV,RelMsk,0,0,0,0,0,0,"",FatColor(NO_TRANS)};

//����ҳ�����Ӧ�õ����Լ�
const PAGE_ATTRIBUTE SettingsPage={
	"SettingsPage",
	"YuanYin",
	"Page Description",
	NORMAL_PAGE,
	0,//

	{
		sizeof(ImgButtonCon)/sizeof(IMG_BUTTON_OBJ), //size of touch region array
		0,//sizeof(CharButtonCon)/sizeof(CHAR_BUTTON_OBJ), //size of touch region array,
		SETTINGS_PAGE_MAX_PAGE_OPTION_NUM,
		0
	},
	ImgButtonCon, //touch region array
	NULL,
	
	SystemEventHandler,
	PeripheralsHandler,
	Bit(Perip_KeyPress)|Bit(Perip_KeyRelease),
	ButtonHandler,
};

//-----------------------��ҳ�Զ�������-----------------------
#if 1
typedef struct {
	u16 ChkNum;//���ڷ���
	u16 BufSize;//���������ֽ���
	u16 UsedSize;//ʹ���˵��ֽ���
	u16 OptionTotal;//��Ŀ��
	void *pLastOpt;//���һ�����ָ��
	u8 *Name;
	u16 LocalTab[SETTINGS_PAGE_MAX_OPTION_NUM+1];//[0]û��
}OPS_HEADER;

typedef enum {
	OPT_Null=0,//������
	OPT_YesOrNo,//yes or no ѡ��
	OPT_Num,//�����������������
	OPT_NumList,//���ֵݱ��б�
	OPT_NumEnum,//����ö���б�
	OPT_StrsEnum,//����
}OP_TYPE;

typedef struct {
	u8 Id;
	OP_TYPE Type;
	u8 *Name;
	u8 *Note;
	u8 Total;//only for NumList and Strs options.
}OP_HEADER;

typedef struct {
	u8 Id;
	OP_TYPE Type;
	u8 *Name;
	u8 *Note;
	bool DefValue;
	bool Value;
}OP_YESORNO;

typedef struct {
	u8 Id;
	OP_TYPE Type;
	u8 *Name;
	u8 *Note;
	s32 DefValue;
	s32 Value;
}OP_NUM;

typedef struct{
	u8 Id;
	OP_TYPE Type;
	u8 *Name;
	u8 *Note;
	s32 DefValue;
	s32 Value;
	s32 Max;
	s32 Min;
	s32 Step;
}OP_NUMLIST;

typedef struct {
	u8 Id;
	OP_TYPE Type;
	u8 *Name;
	u8 *Note;
	u8 EnumTotal;
	u8 DefNum;//sequence number,start form 1
	u8 Num;
	s32 FirstEnum;
	//u32 Enum ...
}OP_NUMENUM_HEADER;

typedef struct {
	u8 Id;
	OP_TYPE Type;
	u8 *Name;
	u8 *Note;
	u8 StrsTotal;
	u8 DefNum;//sequence number,start form 1
	u8 Num;
	u8 *pStr[1];//�ַ���ָ������,[0]δʹ��
}OP_STRENUM_HEADER;

typedef enum{
	OpSurVeyFlag,//�����κ���ʾ���޸ģ�ֻ����ռ����
	OpInitFlag,//����ֵ���޸ģ�ֻ����ʾ����������
	OpNewValFlag,//ֱ�Ӱ�������ֵ��������ʾ
	OpIncFlag,//����ԭֵ����������ʾ
	OpDecFlag,//�Լ�ԭֵ����������ʾ
	OpIncPressFlag,//��ס�������Ӱ���ʱ
	OpDecPressFlag,//��ס���ż��ٰ���ʱ
	OpReloadBtnFlag,//����Ч�����ͷŰ���ʱ����ԭԭ��״̬
}OP_UPDATA_FLAG;
#endif

#if 1 //ȫ�ֱ���
static u16 gCurChkNum;//������¼��ǰ��Ŀ���Ψһ�룬��ֹ����

typedef struct {
	OPS_HEADER *OpsBuf;//��¼��ǰѡ����Ļ���
	u8 PageFirstOpIdx[SETTINGS_PAGE_MAX_PAGE_NUM+1];//��¼ÿҳ�ĵ�һ������ֵ��[0]������
	u8 PageTotal;//��¼ҳ����������СֵΪ1
	u8 CurrPage;//��¼��ǰҳ����1��ʼ

	u8 DynImgTchIdx;
	IMG_BUTTON_OBJ OptionsTch[SETTINGS_PAGE_MAX_PAGE_OPTION_NUM];
}SETTINGS_PAGE_VARS;//����ҳҪ�õ���ȫ�ֱ���ȫ������˽ṹ��
static SETTINGS_PAGE_VARS *gOpsVars;//ֻ��Ҫ����һ��ָ�룬����ȫ�ֱ�����ʹ��
#endif

//-----------------------��ҳ���⺯��-----------------------
#if 1
//����ѡ�����ͷ�ֶ�
//OptsID����������ѡ���飬����Ϊ0
void SP_AddOptionsHeader(void *pBuf,u16 BufSize,u8 *Name,u8 OptsID)
{
	OPS_HEADER *pOptionsBuf=pBuf;

	gCurChkNum=Rand(0xff00)+OptsID;
	MemSet(pOptionsBuf,0,BufSize);
	
	if(BufSize<=sizeof(OPS_HEADER)) 
	{
		Debug("SettingsPage %s:Buffer Size is too small!Must > %d\n\r",__FUNCTION__,sizeof(OPS_HEADER));
		return;
	}
	
	pOptionsBuf->ChkNum=gCurChkNum;
	Debug("SPID %d\n\r",pOptionsBuf->ChkNum);
	pOptionsBuf->BufSize=BufSize;//���滺������С
	pOptionsBuf->UsedSize=sizeof(OPS_HEADER);//�ڶ���Ԫ�����ڱ������ֽ���
	pOptionsBuf->OptionTotal=0;//��Ŀ����
	pOptionsBuf->Name=Name;
	pOptionsBuf->pLastOpt=(void *)((u32)pOptionsBuf+sizeof(OPS_HEADER));//������Ԫ�����ڱ������һ��Option��ͷ��ַ
}

//���yes noѡ��
void SP_AddYesOrNoOption(void *pBuf,u8 Id,bool DefValue,u8 *Name,u8 *Note)
{
	OPS_HEADER *pOptionsBuf=pBuf;
	OP_YESORNO *pYesOrNo;

	if(pOptionsBuf->ChkNum!=gCurChkNum)
	{
		Debug("SettingsPage %s:ID is error!!!\n\r",__FUNCTION__);
		return;
	}
	
	if((pOptionsBuf->UsedSize+sizeof(OP_YESORNO))>pOptionsBuf->BufSize)//��黺���Ƿ���
	{
		Debug("SettingsPage %s:Buffer Size is too small!\n\r",__FUNCTION__);
		pOptionsBuf->ChkNum=0;
		return;
	}
	
	pYesOrNo=(void *)((u32)pOptionsBuf+pOptionsBuf->UsedSize);//�ҵ���λ��

	pYesOrNo->Id=Id;
	pYesOrNo->Type=OPT_YesOrNo;
	pYesOrNo->Name=Name;
	pYesOrNo->Note=Note;
 	pYesOrNo->DefValue=pYesOrNo->Value=DefValue;
 	
 	if(++pOptionsBuf->OptionTotal>SETTINGS_PAGE_MAX_OPTION_NUM)
 	{
		Debug("SettingsPage %s:LocalTab Size is too small!\n\r",__FUNCTION__);
		pOptionsBuf->ChkNum=0;
		return;
 	}
 	pOptionsBuf->LocalTab[pOptionsBuf->OptionTotal]=pOptionsBuf->UsedSize;
 	pOptionsBuf->UsedSize+=sizeof(OP_YESORNO);
 	pOptionsBuf->pLastOpt=pYesOrNo; 	
}

//�������ѡ��
void SP_AddNumOption(void *pBuf,u8 Id,s32 DefValue,u8 *Name,u8 *Note)
{
	OPS_HEADER *pOptionsBuf=pBuf;
	OP_NUM *pNum;
	
	if(pOptionsBuf->ChkNum!=gCurChkNum)
	{
		Debug("SettingsPage %s:ID is error!!!\n\r",__FUNCTION__);
		return;
	}
	
	if((pOptionsBuf->UsedSize+sizeof(OP_NUM))>pOptionsBuf->BufSize) //��黺���Ƿ���
	{
		Debug("SettingsPage %s:Buffer Size is too small!\n\r",__FUNCTION__);
		pOptionsBuf->ChkNum=0;
		return;
	}

	pNum=(void *)((u32)pOptionsBuf+pOptionsBuf->UsedSize);//�ҵ���λ��
	
	pNum->Id=Id;
	pNum->Type=OPT_Num;
	pNum->Name=Name;
	pNum->Note=Note;
	pNum->DefValue=pNum->Value=DefValue;

 	if(++pOptionsBuf->OptionTotal>SETTINGS_PAGE_MAX_OPTION_NUM)
 	{
		Debug("SettingsPage %s:LocalTab Size is too small!\n\r",__FUNCTION__);
		pOptionsBuf->ChkNum=0;
		return;
 	}
 	pOptionsBuf->LocalTab[pOptionsBuf->OptionTotal]=pOptionsBuf->UsedSize;
 	pOptionsBuf->UsedSize+=sizeof(OP_NUM);
 	pOptionsBuf->pLastOpt=pNum;
}

//��������б�ѡ��
void SP_AddNumListOption(void *pBuf,u8 Id,s32 DefValue,s32 Min,s32 Max,s32 Step,u8 *Name,u8 *Note)
{
	OPS_HEADER *pOptionsBuf=pBuf;
	OP_NUMLIST *pNumList;

	if(pOptionsBuf->ChkNum!=gCurChkNum)
	{
		Debug("SettingsPage %s:ID is error!!!\n\r",__FUNCTION__);
		return;
	}
	
	if((pOptionsBuf->UsedSize+sizeof(OP_NUMLIST))>pOptionsBuf->BufSize) //��黺���Ƿ���
	{
		Debug("SettingsPage %s:Buffer Size is too small!\n\r",__FUNCTION__);
		pOptionsBuf->ChkNum=0;
		return;
	}

	if(Min>=Max)
	{
		Debug("SettingsPage %s:Min and Max value is error!\n\r",__FUNCTION__);
		return;
	}
	
	pNumList=(void *)((u32)pOptionsBuf+pOptionsBuf->UsedSize);//�ҵ���λ��

	pNumList->Id=Id;
	pNumList->Type=OPT_NumList;
	pNumList->Name=Name;
	pNumList->Note=Note;
	pNumList->DefValue=pNumList->Value=DefValue;
	pNumList->Max=Max;
	pNumList->Min=Min;
	pNumList->Step=Step;

 	if(++pOptionsBuf->OptionTotal>SETTINGS_PAGE_MAX_OPTION_NUM)
 	{
		Debug("SettingsPage %s:LocalTab Size is too small!\n\r",__FUNCTION__);
		pOptionsBuf->ChkNum=0;
		return;
 	}
 	pOptionsBuf->LocalTab[pOptionsBuf->OptionTotal]=pOptionsBuf->UsedSize;
 	pOptionsBuf->UsedSize+=sizeof(OP_NUMLIST);
 	pOptionsBuf->pLastOpt=pNumList;
 }

//�������ö��ѡ��
void SP_AddNumEnumOption(void *pBuf,u8 Id,bool IsDef,s32 Value,u8 *Name,u8 *Note)
{
	OPS_HEADER *pOptionsBuf=pBuf;
	OP_NUMENUM_HEADER *pCurrNumEnum=pOptionsBuf->pLastOpt;
	s32 *pEnumItem;
	bool IsNewOne=FALSE;

	if(pOptionsBuf->ChkNum!=gCurChkNum)
	{
		Debug("SettingsPage %s:ID is error!!!\n\r",__FUNCTION__);
		return;
	}
	
	//����Ƿ�Ҫ�¼�һ��
	if(pCurrNumEnum->Type!=OPT_NumEnum) IsNewOne=TRUE;
	else if(pCurrNumEnum->Id!=Id) IsNewOne=TRUE;

	if(IsNewOne)//Ҫ�¼�һ��NumEnum��
	{
		if((pOptionsBuf->UsedSize+sizeof(OP_NUMENUM_HEADER))>pOptionsBuf->BufSize) //��黺���Ƿ���
		{
			Debug("SettingsPage %s:Buffer Size is too small!\n\r",__FUNCTION__);
			pOptionsBuf->ChkNum=0;
			return;
		}
		
		pCurrNumEnum=(void *)((u32)pOptionsBuf+pOptionsBuf->UsedSize);//�ҵ���λ��

		//��ʼ��
		pCurrNumEnum->Id=Id;
		pCurrNumEnum->Type=OPT_NumEnum;
		pCurrNumEnum->Name=Name;
		pCurrNumEnum->Note=Note;
		pCurrNumEnum->EnumTotal=1;
		pCurrNumEnum->DefNum=pCurrNumEnum->Num=1;
		pCurrNumEnum->FirstEnum=Value;

	 	if(++pOptionsBuf->OptionTotal>SETTINGS_PAGE_MAX_OPTION_NUM)
	 	{
			Debug("SettingsPage %s:LocalTab Size is too small!\n\r",__FUNCTION__);
			pOptionsBuf->ChkNum=0;
			return;
	 	}
	 	pOptionsBuf->LocalTab[pOptionsBuf->OptionTotal]=pOptionsBuf->UsedSize;
		pOptionsBuf->UsedSize+=sizeof(OP_NUMENUM_HEADER);
		pOptionsBuf->pLastOpt=pCurrNumEnum;
	}
	else //��ӵ�ԭ����NumEnum��num�б�ĩβ
	{
		if((pOptionsBuf->UsedSize+sizeof(u32))>pOptionsBuf->BufSize)//��黺���Ƿ���
		{
			Debug("SettingsPage %s:Buffer Size is too small!\n\r",__FUNCTION__);
			pOptionsBuf->ChkNum=0;
			return;
		}

		pCurrNumEnum=pOptionsBuf->pLastOpt;//����Դ��ڵ�option��ָ��

		pCurrNumEnum->EnumTotal++;
		if(IsDef) pCurrNumEnum->DefNum=pCurrNumEnum->Num=pCurrNumEnum->EnumTotal;

		pEnumItem=(void *)((u32)pOptionsBuf+pOptionsBuf->UsedSize);
		*pEnumItem=Value;

		pOptionsBuf->UsedSize+=sizeof(u32);
	}		
}

//����ַ���ö��ѡ��
void SP_AddStrEnumOption(void *pBuf,u8 Id,bool IsDef,u8 *pStr,u8 *Name,u8 *Note)
{
	OPS_HEADER *pOptionsBuf=pBuf;
	OP_STRENUM_HEADER *pCurrStrs=pOptionsBuf->pLastOpt;
	u8 StrLen=strlen((void *)pStr);
	bool IsNewOne=FALSE;
	u8 **pNewStr;

	if(pOptionsBuf->ChkNum!=gCurChkNum)
	{
		Debug("SettingsPage %s:ID is error!!!\n\r",__FUNCTION__);
		return;
	}
	
	//����Ƿ�Ҫ�¼�һ��
	if(pCurrStrs->Type!=OPT_StrsEnum) IsNewOne=TRUE;
	else if(pCurrStrs->Id!=Id) IsNewOne=TRUE;

	if(IsNewOne)//Ҫ�¼�һ��Strs��
	{
		if((pOptionsBuf->UsedSize+sizeof(OP_STRENUM_HEADER))>pOptionsBuf->BufSize)//��黺���Ƿ���
		{
			Debug("SettingsPage %s:Buffer Size is too small!\n\r",__FUNCTION__);
			pOptionsBuf->ChkNum=0;
			return;
		}
	
		pCurrStrs=(void *)((u32)pOptionsBuf+pOptionsBuf->UsedSize);//�ҵ���λ��
		
		//��ʼ��
		pCurrStrs->Id=Id;
		pCurrStrs->Type=OPT_StrsEnum;
		pCurrStrs->Name=Name;
		pCurrStrs->Note=Note;
		pCurrStrs->StrsTotal=1;
		pCurrStrs->DefNum=pCurrStrs->Num=1;
		pCurrStrs->pStr[1]=pStr;

	 	if(++pOptionsBuf->OptionTotal>SETTINGS_PAGE_MAX_OPTION_NUM)
	 	{
			Debug("SettingsPage %s:LocalTab Size is too small!\n\r",__FUNCTION__);
			pOptionsBuf->ChkNum=0;
			return;
	 	}
	 	pOptionsBuf->LocalTab[pOptionsBuf->OptionTotal]=pOptionsBuf->UsedSize;
		pOptionsBuf->UsedSize+=sizeof(OP_STRENUM_HEADER);
		pOptionsBuf->pLastOpt=pCurrStrs;
	}
	else //��ӵ�ԭ����NumEnum��num�б�ĩβ
	{
		if((pOptionsBuf->UsedSize+StrLen+1)>pOptionsBuf->BufSize) //��黺���Ƿ���
		{
			Debug("SettingsPage %s:Buffer Size is too small!\n\r",__FUNCTION__);
			pOptionsBuf->ChkNum=0;
			return;
		}
		
		pCurrStrs=pOptionsBuf->pLastOpt;//����Ѵ��ڵ�option��ָ��

		pCurrStrs->StrsTotal++;
		if(IsDef) pCurrStrs->DefNum=pCurrStrs->Num=pCurrStrs->StrsTotal;

		pNewStr=(void *)((u32)pOptionsBuf+pOptionsBuf->UsedSize);
		*pNewStr=pStr;

		pOptionsBuf->UsedSize+=sizeof(pNewStr);
	}		
}

//��ȡѡ������
bool SP_GetOptionsTotal(void *pOptionsBuf,u8 *Total)
{
	OPS_HEADER *pOpsHeader=pOptionsBuf;

	if(pOpsHeader->UsedSize>pOpsHeader->BufSize) return FALSE;
	*Total=pOpsHeader->OptionTotal;
	return TRUE;
}

//��ѯ��ǰ���������Ƿ��޸�
bool SP_IsModify(void *pOptionsBuf,u8 Idx)
{
	OPS_HEADER *pOpsHeader=pOptionsBuf;
	OP_HEADER *pHeader;

	pHeader=(void *)((u32)pOpsHeader+pOpsHeader->LocalTab[Idx]);
	if(pHeader!=NULL)
	switch(pHeader->Type)
	{
		case OPT_YesOrNo:
			{
				OP_YESORNO *pYesOrNo=(void *)pHeader;
				if(pYesOrNo->Value!=pYesOrNo->DefValue) return TRUE;
			}
			break;
		case OPT_Num:
			{
				OP_NUM *pNum=(void *)pHeader;
				if(pNum->Value!=pNum->DefValue) return TRUE;
			}
			break;
 		case OPT_NumList:
			{
				OP_NUMLIST *pNumList=(void *)pHeader;
				if(pNumList->Value!=pNumList->DefValue) return TRUE;
			}
			break;
		case OPT_NumEnum:
			{
				OP_NUMENUM_HEADER *pNumEnum=(void *)pHeader;
				if(pNumEnum->Num!=pNumEnum->DefNum) return TRUE;
			}
			break;
		case OPT_StrsEnum:
			{
				OP_STRENUM_HEADER *pStrs=(void *)pHeader;
				if(pStrs->Num!=pStrs->DefNum) return TRUE;
			}
			break;
	}

	return FALSE;
}

//��ȡָ�������������
OP_RET_TYPE SP_GetType(void *pOptionsBuf,u8 Idx)
{
	OPS_HEADER *pOpsHeader=pOptionsBuf;
	OP_HEADER *pHeader;

	pHeader=(void *)((u32)pOpsHeader+pOpsHeader->LocalTab[Idx]);
	if(pHeader!=NULL)
		switch(pHeader->Type)
		{
			case OPT_YesOrNo:
				return OPRT_YesOrNo;
			case OPT_Num:
			case OPT_NumList:
		 	case OPT_NumEnum:
		 		return OPRT_Num;
			case OPT_StrsEnum:
				return OPRT_Str;			
		}
	return OPRT_Null;
}

//��ȡ���id
bool SP_GetOptionID(void *pOptionsBuf,u8 Idx,u8 *Id)
{
	OPS_HEADER *pOpsHeader=pOptionsBuf;
	OP_HEADER *pHeader;

	pHeader=(void *)((u32)pOpsHeader+pOpsHeader->LocalTab[Idx]);
	if(pHeader!=NULL)
	{
		*Id=pHeader->Id;
		return TRUE;
	}
	else return FALSE;
}

//��ȡ��������ַ���
bool SP_GetOptionName(void *pOptionsBuf,u8 Idx,u8 **ppName)
{
	OPS_HEADER *pOpsHeader=pOptionsBuf;
	OP_HEADER *pHeader;

	pHeader=(void *)((u32)pOpsHeader+pOpsHeader->LocalTab[Idx]);
	if(pHeader!=NULL)
	{
		*ppName=pHeader->Name;
		return TRUE;
	}
	else return FALSE;
}

//��ȡ�����ַ���
bool SP_GetOptionNote(void *pOptionsBuf,u8 Idx,u8 **ppNote)
{
	OPS_HEADER *pOpsHeader=pOptionsBuf;
	OP_HEADER *pHeader;

	pHeader=(void *)((u32)pOpsHeader+pOpsHeader->LocalTab[Idx]);
	if(pHeader!=NULL)
	{
		*ppNote=pHeader->Note;
		return TRUE;
	}
	else return FALSE;
}

//����yes no���ֵ
bool SP_GetYesOrNoOption(void *pOptionsBuf,u8 Idx,bool *pYesNo)
{
	OPS_HEADER *pOpsHeader=pOptionsBuf;
	OP_YESORNO *pYesOrNo;

	pYesOrNo=(void *)((u32)pOpsHeader+pOpsHeader->LocalTab[Idx]);
	if((pYesOrNo!=NULL)&&(pYesOrNo->Type==OPT_YesOrNo))
	{
		*pYesNo=pYesOrNo->Value;
		return TRUE;
	}
	return FALSE;
}

//�����������ֵ
bool SP_GetNumOption(void *pOptionsBuf,u8 Idx,s32 *pVal)
{
	OPS_HEADER *pOpsHeader=pOptionsBuf;
	OP_NUM *pNum;
	OP_NUMLIST *pNumList;
	OP_NUMENUM_HEADER *pNumEnumHeadr;

	pNum=(void *)((u32)pOpsHeader+pOpsHeader->LocalTab[Idx]);
	pNumList=(void *)((u32)pOpsHeader+pOpsHeader->LocalTab[Idx]);
	pNumEnumHeadr=(void *)((u32)pOpsHeader+pOpsHeader->LocalTab[Idx]);
	if(pNum!=NULL)
	{
		switch(pNum->Type)
		{
			case OPT_Num:
				*pVal=pNum->Value;
				return TRUE;
			case OPT_NumList:
				*pVal=pNumList->Value;
				return TRUE;
			case OPT_NumEnum:
			{
				s32 *pS32=&pNumEnumHeadr->FirstEnum;
				*pVal=pS32[pNumList->Value];
				return TRUE;
			}
		}
	}

	return FALSE;
}

//unfinish ��ȡ�ַ������ֵ
bool SP_GetStrOption(void *pOptionsBuf,u8 Idx,u8 **ppStr)
{
	return TRUE;
}
#endif
//--------------------------------------------------------

//--------------------��ҳ�������ͺͺ���------------------------------------
#if 1
//ͨ��������ȡѡ�ע��OpIdx��1��ʼ
//����NULL��ʾδ��ȡ��
static OP_HEADER *GetOptionHeader(OPS_HEADER *pOpsHeader,u8 OpIdx)
{
	if(pOpsHeader->ChkNum!=gCurChkNum)
	{
		Debug("SettingsPage %s:ID is error!!!\n\r",__FUNCTION__);
		return NULL;
	}

	if((OpIdx<=pOpsHeader->OptionTotal)&&(OpIdx<=SETTINGS_PAGE_MAX_OPTION_NUM)&&(OpIdx>0))
		return (void *)((u32)pOpsHeader+pOpsHeader->LocalTab[OpIdx]);

	return NULL;
}

//��ȡ��ҳָ��������ѡ��ͷ
//PageItemIdx��СֵΪ1
static OP_HEADER *GetPagesOption(u8 PageItemIdx)
{
	if(gOpsVars->OpsBuf!=NULL)
		return GetOptionHeader(gOpsVars->OpsBuf,gOpsVars->PageFirstOpIdx[gOpsVars->CurrPage]+PageItemIdx-1);
	return NULL;
}

//���ڻ���yes or no ѡ��
//������һ��ѡ�������Y��ַ
static u16 DrawYesOrNoOption(OP_UPDATA_FLAG Flag,void *pOptionBuf,bool NewVal,u16 StartY,IMG_BUTTON_OBJ *Reg)
{
	OP_YESORNO *pYesNo=pOptionBuf;
	GUI_REGION DrawRegion;
	u8 NoteLineNum;

	if(pYesNo->Type!=OPT_YesOrNo) return StartY;
	
	if((pYesNo->Note==NULL)||(pYesNo->Note[0]==0)) 	NoteLineNum=0;	
	else 	NoteLineNum=(strlen((void *)pYesNo->Note)-1)/38+1;//����������һ��38����
	
	switch(Flag)
	{
		case OpSurVeyFlag:
			return StartY+SETTINGS_PAGE_OPTION_NAME_H+SETTINGS_PAGE_OPTION_NOTE_H*NoteLineNum+SETTINGS_PAGE_OPTION_LINE_H;
		case OpInitFlag:
#if 0
			DrawRegion.x=SETTINGS_PAGE_OPTION_STARTX;
			DrawRegion.y=StartY;
			DrawRegion.w=SETTINGS_PAGE_OPTION_ENDX-SETTINGS_PAGE_OPTION_STARTX;
			DrawRegion.h=SETTINGS_PAGE_OPTION_NAME_H+SETTINGS_PAGE_OPTION_NOTE_H*NoteLineNum;
			DrawRegion.Color=FatColor(SETTINGS_PAGE_BGCOLOR);
			Gui_FillBlock(&DrawRegion);//����ɫ
#endif

			DrawRegion.x=SETTINGS_PAGE_OPTION_STARTX;
			DrawRegion.y=StartY;
			DrawRegion.w=SETTINGS_PAGE_OPTION_ENDX-SETTINGS_PAGE_OPTION_STARTX;
			DrawRegion.h=16;
			DrawRegion.Color=FatColor(SETTINGS_PAGE_TITLE_COLOR);
			DrawRegion.Space=0x00;
			Gui_DrawFont(GBK16_FONT,pYesNo->Name,&DrawRegion);//��Ŀ��

			DrawRegion.x=SETTINGS_PAGE_OPTION_STARTX;
			DrawRegion.y=StartY+SETTINGS_PAGE_OPTION_NAME_H;
			DrawRegion.w=SETTINGS_PAGE_OPTION_ENDX-SETTINGS_PAGE_OPTION_STARTX;
			DrawRegion.h=SETTINGS_PAGE_OPTION_NOTE_H*NoteLineNum;
			DrawRegion.Color=FatColor(SETTINGS_PAGE_NOTE_COLOR);
			DrawRegion.Space=0x02;
			Gui_DrawFont(GBK12_FONT,pYesNo->Note,&DrawRegion);//˵��

			//name��ѡ���ļ��
			DrawRegion.x=SETTINGS_PAGE_OPTION_ENDX-58-SETTINGS_PAGE_OPTION_MARGIN;//58��yes or noͼƬ���
			DrawRegion.y=StartY;
			DrawRegion.w=SETTINGS_PAGE_OPTION_MARGIN;
			DrawRegion.h=18;
			DrawRegion.Color=FatColor(SETTINGS_PAGE_BGCOLOR);
			Gui_FillBlock(&DrawRegion);
	
			//�ָ���
			DrawRegion.x=SETTINGS_PAGE_OPTION_STARTX;
			DrawRegion.y=StartY+SETTINGS_PAGE_OPTION_NAME_H+SETTINGS_PAGE_OPTION_NOTE_H*NoteLineNum;
			DrawRegion.w=SETTINGS_PAGE_OPTION_ENDX-SETTINGS_PAGE_OPTION_STARTX;
			DrawRegion.h=SETTINGS_PAGE_OPTION_LINE_H;
			DrawRegion.Color=FatColor(SETTINGS_PAGE_LINECOLOR);
			Gui_FillBlock(&DrawRegion);

			Reg->x=SETTINGS_PAGE_OPTION_ENDX-58;
			Reg->y=StartY;
			Reg->w=58;
			Reg->h=20;//�������򣬱���ѡ��
			Reg->OptionsMask=PrsMsk|RelMsk|ReVMsk;
			Q_SetDynamicImgTch(gOpsVars->DynImgTchIdx++,Reg);
			break;
		case OpIncFlag:
		case OpDecFlag:
			if(pYesNo->Value) pYesNo->Value=FALSE;
			else pYesNo->Value=TRUE;
			break;
		case OpNewValFlag:
			pYesNo->Value=NewVal;
			break;	
	}
	StartY=Reg->y;

	if((Flag==OpIncPressFlag)||(Flag==OpDecPressFlag))
	{
		DrawRegion.x=SETTINGS_PAGE_OPTION_ENDX-58;//58��yes or noͼƬ���
		DrawRegion.y=StartY;
		DrawRegion.w=58;
		DrawRegion.h=18;
		DrawRegion.Color=FatColor(NO_TRANS);
		Gui_DrawImgArray(gImage_OnOff,&DrawRegion);//��ס����ʱ��ͼ��
	}
	else
	{
		if(pYesNo->Value)
		{
			DrawRegion.x=SETTINGS_PAGE_OPTION_ENDX-58;//58��yes or noͼƬ���
			DrawRegion.y=StartY;
			DrawRegion.w=58;
			DrawRegion.h=18;
			DrawRegion.Color=FatColor(NO_TRANS);
			Gui_DrawImgArray(gImage_On,&DrawRegion);
		}
		else
		{
			DrawRegion.x=SETTINGS_PAGE_OPTION_ENDX-58;//58��yes or noͼƬ���
			DrawRegion.y=StartY;
			DrawRegion.w=58;
			DrawRegion.h=18;
			DrawRegion.Color=FatColor(NO_TRANS);
			Gui_DrawImgArray(gImage_Off,&DrawRegion);
		}
	}
	
	return StartY+SETTINGS_PAGE_OPTION_NAME_H+SETTINGS_PAGE_OPTION_NOTE_H*NoteLineNum+SETTINGS_PAGE_OPTION_LINE_H;
}

//����NumOption�Ĵ���
//����ĩβ��yֵ
static u16 DrawNumOption(OP_UPDATA_FLAG Flag,void *pOptionBuf,s32 NewVal,u16 StartY,IMG_BUTTON_OBJ *Reg)
{
	OP_NUM *pNum=pOptionBuf;
	OP_NUMLIST *pNumList=pOptionBuf;
	OP_NUMENUM_HEADER *pNumEnumHeadr=pOptionBuf;
	GUI_REGION DrawRegion;
	u8 NoteLineNum;

	if((pNum->Type!=OPT_Num)&&(pNumList->Type!=OPT_NumList)&&(pNumEnumHeadr->Type!=OPT_NumEnum)) 
		return StartY;

	if((pNum->Note==NULL)||(pNum->Note[0]==0)) 	NoteLineNum=0;	
	else 	NoteLineNum=(strlen((void *)pNum->Note)-1)/38+1;//����������һ��38����
	
	switch(Flag)
	{
		case OpSurVeyFlag:
			return StartY+SETTINGS_PAGE_OPTION_NAME_H+SETTINGS_PAGE_OPTION_NOTE_H*NoteLineNum+SETTINGS_PAGE_OPTION_LINE_H;
		case OpInitFlag:
#if 0
			DrawRegion.x=SETTINGS_PAGE_OPTION_STARTX;
			DrawRegion.y=StartY;
			DrawRegion.w=SETTINGS_PAGE_OPTION_ENDX-SETTINGS_PAGE_OPTION_STARTX;
			DrawRegion.h=SETTINGS_PAGE_OPTION_NAME_H+SETTINGS_PAGE_OPTION_NOTE_H*NoteLineNum;
			DrawRegion.Color=FatColor(SETTINGS_PAGE_BGCOLOR);
			Gui_FillBlock(&DrawRegion);//����ɫ
#endif

			DrawRegion.x=SETTINGS_PAGE_OPTION_STARTX;
			DrawRegion.y=StartY;
			DrawRegion.w=SETTINGS_PAGE_OPTION_ENDX-SETTINGS_PAGE_OPTION_STARTX;
			DrawRegion.h=16;
			DrawRegion.Color=FatColor(SETTINGS_PAGE_TITLE_COLOR);
			DrawRegion.Space=0x00;
			Gui_DrawFont(GBK16_FONT,pNum->Name,&DrawRegion);//��Ŀ��

			DrawRegion.x=SETTINGS_PAGE_OPTION_STARTX;
			DrawRegion.y=StartY+SETTINGS_PAGE_OPTION_NAME_H;
			DrawRegion.w=SETTINGS_PAGE_OPTION_ENDX-SETTINGS_PAGE_OPTION_STARTX;
			DrawRegion.h=SETTINGS_PAGE_OPTION_NOTE_H*NoteLineNum;
			DrawRegion.Color=FatColor(SETTINGS_PAGE_NOTE_COLOR);
			DrawRegion.Space=0x02;
			Gui_DrawFont(GBK12_FONT,pNum->Note,&DrawRegion);//˵��

			//�ָ���
			DrawRegion.x=SETTINGS_PAGE_OPTION_STARTX;
			DrawRegion.y=StartY+SETTINGS_PAGE_OPTION_NAME_H+SETTINGS_PAGE_OPTION_NOTE_H*NoteLineNum;
			DrawRegion.w=SETTINGS_PAGE_OPTION_ENDX-SETTINGS_PAGE_OPTION_STARTX;
			DrawRegion.h=SETTINGS_PAGE_OPTION_LINE_H;
			DrawRegion.Color=FatColor(SETTINGS_PAGE_LINECOLOR);
			Gui_FillBlock(&DrawRegion);

			break;
		case OpIncFlag:
			if(pNum->Type==OPT_Num) pNum->Value++;
			else if(pNum->Type==OPT_NumList) 
			{
				if((pNumList->Value+pNumList->Step)<=pNumList->Max)
					pNumList->Value+=pNumList->Step;
				else pNumList->Value=pNumList->Min;
			}
			else if(pNum->Type==OPT_NumEnum)
			{
				if((pNumEnumHeadr->Num+1)<=pNumEnumHeadr->EnumTotal) pNumEnumHeadr->Num++;
				else pNumEnumHeadr->Num=1;
			}
			break;
		case OpDecFlag:
			if(pNum->Type==OPT_Num) pNum->Value--;
			else if(pNum->Type==OPT_NumList) 
			{
				if((pNumList->Value-pNumList->Step)>=pNumList->Min)
					pNumList->Value-=pNumList->Step;
				else pNumList->Value=pNumList->Max;
			}
			else if(pNum->Type==OPT_NumEnum)
			{
				if((pNumEnumHeadr->Num-1)>=1) pNumEnumHeadr->Num--;
				else pNumEnumHeadr->Num=pNumEnumHeadr->EnumTotal;
			}
			break;
		case OpNewValFlag:
			if(pNum->Type==OPT_Num) pNum->Value=NewVal;
			else if(pNum->Type==OPT_NumList)
			{
				if((NewVal>=pNumList->Min)&&(NewVal<=pNumList->Max))
					pNumList->Value=NewVal;
				else Debug("SettingsPage:Vain Value!\n\r");
			}
			else if(pNum->Type==OPT_NumEnum)
			{
				u8 i;
				s32 *pNum=&pNumEnumHeadr->FirstEnum;
				for(i=0;i<pNumEnumHeadr->EnumTotal;i++)
				{
					if(pNum[i]==NewVal)
					{
						pNumEnumHeadr->Num=i+1;
						break;
					}
				}
			}
			break;
	}
	
	{
		u8 NumStr[16];
		u8 StrW;

		if(pNum->Type==OPT_Num)
		{
			sprintf((void *)NumStr,"%d",pNum->Value);
		}
		else if(pNum->Type==OPT_NumList)
		{
			sprintf((void *)NumStr,"%d",pNumList->Value);
		}
		else if(pNum->Type==OPT_NumEnum)
		{
			s32 *pNum=&pNumEnumHeadr->FirstEnum;
			sprintf((void *)NumStr,"%d",pNum[pNumEnumHeadr->Num-1]);
		}
		
		StrW=strlen((void *)NumStr)*8+8;

		//ע����߸ı䰴������
		Reg->x=SETTINGS_PAGE_OPTION_ENDX-StrW-23*2;
		Reg->h=20;//�Ӹ����򣬷�����
		Reg->OptionsMask=PrsMsk|RelMsk|ReVMsk;
		if(Flag==OpInitFlag) 
		{
			Reg->y=StartY;
			Reg->w=StrW+23*2;
			Q_SetDynamicImgTch(gOpsVars->DynImgTchIdx++,Reg);
		}
		else
		{
			StartY=Reg->y;
			if((StrW+23*2)<Reg->w)//��Ҫ����д������Name
			{
				DrawRegion.x=SETTINGS_PAGE_OPTION_ENDX-Reg->w-SETTINGS_PAGE_OPTION_MARGIN;
				DrawRegion.y=StartY;
				DrawRegion.w=Reg->w+SETTINGS_PAGE_OPTION_MARGIN;
				DrawRegion.h=19;
				DrawRegion.Color=FatColor(SETTINGS_PAGE_BGCOLOR);
				Gui_FillBlock(&DrawRegion);

				DrawRegion.x=SETTINGS_PAGE_OPTION_STARTX;
				DrawRegion.y=StartY;
				DrawRegion.w=SETTINGS_PAGE_OPTION_ENDX-SETTINGS_PAGE_OPTION_STARTX;
				DrawRegion.h=16;
				DrawRegion.Color=FatColor(SETTINGS_PAGE_TITLE_COLOR);
				DrawRegion.Space=0x00;
				Gui_DrawFont(GBK16_FONT,pNum->Name,&DrawRegion);//��Ŀ��
			}
			Reg->w=StrW+23*2;
		}

		//name��ѡ���ļ��
		DrawRegion.x=SETTINGS_PAGE_OPTION_ENDX-StrW-23*2-SETTINGS_PAGE_OPTION_MARGIN;
		DrawRegion.y=StartY;
		DrawRegion.w=SETTINGS_PAGE_OPTION_MARGIN;
		DrawRegion.h=19;
		DrawRegion.Color=FatColor(SETTINGS_PAGE_BGCOLOR);
		Gui_FillBlock(&DrawRegion);
		
		//�¼�ͷ
		DrawRegion.x=SETTINGS_PAGE_OPTION_ENDX-StrW-23*2;//23�����¼�ͷ�Ŀ��
		DrawRegion.y=StartY;
		DrawRegion.w=23;
		DrawRegion.h=19;
		DrawRegion.Color=FatColor(NO_TRANS);
		if(Flag==OpDecPressFlag)
			Gui_DrawImgArray(gImage_DownArrowHL,&DrawRegion);
		else 
			Gui_DrawImgArray(gImage_DownArrowLL,&DrawRegion);

		//���ֱ���
		DrawRegion.x+=23;//23�����¼�ͷ�Ŀ��
		DrawRegion.w=1;//��䱳��ͼ�Ŀ��
		DrawRegion.h=19;//��䱳��ͼ�ĸ߶�		
		Gui_FillImgArray_H(gImage_NumFrame,StrW,&DrawRegion);

		//����
		DrawRegion.x+=4;
		DrawRegion.y+=4;
		DrawRegion.w=StrW;
		DrawRegion.h=16;
		DrawRegion.Color=FatColor(SETTINGS_PAGE_NUM_COLOR);
		DrawRegion.Space=0x00;
		Gui_DrawFont(ASC14B_FONT,NumStr,&DrawRegion);
		DrawRegion.x-=4;
		DrawRegion.y-=4;

		//�¼�ͷ
		DrawRegion.x+=StrW;
		DrawRegion.w=23;
		DrawRegion.h=19;
		DrawRegion.Color=FatColor(NO_TRANS);
		if(Flag==OpIncPressFlag)
			Gui_DrawImgArray(gImage_UpArrowHL,&DrawRegion);
		else
			Gui_DrawImgArray(gImage_UpArrowLL,&DrawRegion);
	}

	return StartY+SETTINGS_PAGE_OPTION_NAME_H+SETTINGS_PAGE_OPTION_NOTE_H*NoteLineNum+SETTINGS_PAGE_OPTION_LINE_H;
}

static u16 DrawStrOption(OP_UPDATA_FLAG Flag,void *pOptionBuf,u8 *pNewVal,u16 StartY,IMG_BUTTON_OBJ *Reg)
{
	return StartY;
}

//����ҳ��ʱ����ѡ����ÿҳ��Ŀ�����洢��ȫ�ֱ�����
static bool OptionsParse(OPS_HEADER *pOpsHeader)
{
	u8 i;
	u16 y;
	OP_HEADER *pHeader;
	
	if(pOpsHeader->ChkNum!=gCurChkNum)
	{
		Debug("SettingsPage %s:ID is error!!!\n\r",__FUNCTION__);
		return FALSE;
	}
	if(pOpsHeader->BufSize<pOpsHeader->UsedSize) return FALSE;
	if(pOpsHeader->OptionTotal==0) return FALSE;

	gOpsVars->PageTotal=1;
	gOpsVars->PageFirstOpIdx[0]=0;
	gOpsVars->PageFirstOpIdx[1]=1;
	y=SETTINGS_PAGE_OPTION_STARTY-SETTINGS_PAGE_OPTION_MARGIN;
	for(i=1;i<=pOpsHeader->OptionTotal;i++)//��1��ʼ����Ϊ[0]δʹ��
	{
		y+=SETTINGS_PAGE_OPTION_MARGIN;
		pHeader=(void *)((u32)pOpsHeader+pOpsHeader->LocalTab[i]);

		switch(pHeader->Type)//����߶�
		{
			case OPT_YesOrNo://yes or no ѡ��
				y=DrawYesOrNoOption(OpSurVeyFlag,pHeader,TRUE,y,NULL);
				break;
			case OPT_Num://�����������������
			case OPT_NumList://���ֵݱ��б�
			case OPT_NumEnum://����ö���б�
				y=DrawNumOption(OpSurVeyFlag,pHeader,0,y,NULL);
				break;
			case OPT_StrsEnum://����
				y=DrawStrOption(OpSurVeyFlag,pHeader,"",y,NULL);
				break;
		}

		if(y>SETTINGS_PAGE_OPTION_ENDY)
		{
			gOpsVars->PageFirstOpIdx[++gOpsVars->PageTotal]=i;
			y=SETTINGS_PAGE_OPTION_STARTY-SETTINGS_PAGE_OPTION_MARGIN;
			//Debug("Page %d:%d\n\r",PageTotal,i);
		}
		else
			y+=SETTINGS_PAGE_OPTION_MARGIN;
	}

	//Debug("PageTotal:%d\n\r",PageTotal);
	return TRUE;
}

//����ҳ����������ʾҳ�棬PageIdx��1��ʼ
static void DrawPage(OPS_HEADER *pOpsHeader,u8 PageIdx)
{
	u8 OptionStartIdx;
	u8 OptionEndIdx;
	u8 i;
	OP_HEADER *OpHeader;
	u16 y=SETTINGS_PAGE_OPTION_STARTY-SETTINGS_PAGE_OPTION_MARGIN;
	GUI_REGION DrawRegion;
	
	if(pOpsHeader->ChkNum!=gCurChkNum)
	{
		Debug("SettingsPage %s:ID is error!!!\n\r",__FUNCTION__);
		return;
	}
	
	if((PageIdx<1)||(PageIdx>gOpsVars->PageTotal)) return;
	
	OptionStartIdx=gOpsVars->PageFirstOpIdx[PageIdx];
	if((PageIdx+1)>gOpsVars->PageTotal)
		OptionEndIdx=pOpsHeader->OptionTotal;
	else
		OptionEndIdx=gOpsVars->PageFirstOpIdx[PageIdx+1]-1;

	//������ж�̬����
	gOpsVars->DynImgTchIdx=1;
	for(i=1;i<SETTINGS_PAGE_MAX_PAGE_OPTION_NUM;i++)
	{
		Q_SetDynamicImgTch(i,NULL);
	}

	//������
	DrawRegion.x=SETTINGS_PAGE_OPTION_STARTX;
	DrawRegion.y=SETTINGS_PAGE_OPTION_STARTY;
	DrawRegion.w=SETTINGS_PAGE_OPTION_ENDX-SETTINGS_PAGE_OPTION_STARTX;
	DrawRegion.h=SETTINGS_PAGE_OPTION_ENDY-SETTINGS_PAGE_OPTION_STARTY;
	DrawRegion.Color=FatColor(SETTINGS_PAGE_BGCOLOR);
	Gui_FillBlock(&DrawRegion);

	//��ʼ��ѡ��
	for(i=0;(OptionStartIdx<=OptionEndIdx)&&(i<SETTINGS_PAGE_MAX_PAGE_OPTION_NUM);OptionStartIdx++,i++)
	{
		OpHeader=GetOptionHeader(pOpsHeader,OptionStartIdx);
		if(OpHeader==NULL)
		{
			Debug("Unexpected error!\n\r");
			return;
		}
		y+=SETTINGS_PAGE_OPTION_MARGIN;
		
		switch(OpHeader->Type)
		{
			case OPT_Null://������
				break;
			case OPT_YesOrNo://yes or no ѡ��
				y=DrawYesOrNoOption(OpInitFlag,OpHeader,TRUE,y,&gOpsVars->OptionsTch[i]);
				break;
			case OPT_Num://�����������������
			case OPT_NumList://���ֵݱ��б�
			case OPT_NumEnum://����ö���б�
				y=DrawNumOption(OpInitFlag,OpHeader,0,y,&gOpsVars->OptionsTch[i]);
				break;
			case OPT_StrsEnum://����
				break;
		}		
	}
}

//����һ���Ĭ��ֵ��ͬ������TRUE
bool CheckModify(void *pOptionsBuf)
{
	OP_HEADER *pHeader;
	u8 i=1;

	while((pHeader=GetOptionHeader(pOptionsBuf,i++))!=NULL)
	switch(pHeader->Type)
	{
		case OPT_YesOrNo:
			{
				OP_YESORNO *pYesOrNo=(void *)pHeader;
				if(pYesOrNo->Value!=pYesOrNo->DefValue) return TRUE;
			}
			break;
		case OPT_Num:
			{
				OP_NUM *pNum=(void *)pHeader;
				if(pNum->Value!=pNum->DefValue) return TRUE;
			}
			break;
 		case OPT_NumList:
			{
				OP_NUMLIST *pNumList=(void *)pHeader;
				if(pNumList->Value!=pNumList->DefValue) return TRUE;
			}
			break;
		case OPT_NumEnum:
			{
				OP_NUMENUM_HEADER *pNumEnum=(void *)pHeader;
				if(pNumEnum->Num!=pNumEnum->DefNum) return TRUE;
			}
			break;
		case OPT_StrsEnum:
			{
				OP_STRENUM_HEADER *pStrs=(void *)pHeader;
				if(pStrs->Num!=pStrs->DefNum) return TRUE;
			}
			break;
	}

	return FALSE;
}
#endif

static void DrawSettingsBg(u8 *pName)
{
	GUI_REGION DrawRegion;
	
	//��������
	DrawRegion.x=DrawRegion.y=0;
	DrawRegion.w=240;
	DrawRegion.h=21;
	DrawRegion.Color=FatColor(NO_TRANS);
	Gui_FillImgArray((u8 *)gImage_StatusBar1,1,21,&DrawRegion);	
	DrawTitle1(ASC14B_FONT,(void *)pName,(240-strlen((void *)pName)*GUI_ASC14B_ASCII_WIDTH)>>1,strlen((void *)pName),FatColor(0xe0e0e0));//д����
	
	//������
	DrawRegion.x=0;
	DrawRegion.y=21;
	DrawRegion.w=240;
	DrawRegion.h=320-21-39;
	DrawRegion.Color=FatColor(0x8b8a8a);
	Gui_FillBlock(&DrawRegion);

	//������
	DrawRegion.x=0;
	DrawRegion.y=320-39;
	DrawRegion.w=240;
	DrawRegion.h=39;
	DrawRegion.Color=FatColor(NO_TRANS);
	Gui_FillImgArray((u8 *)gImage_BottomBar1,1,39,&DrawRegion);

	//����
	DrawFrame1(25,252);	

	//����
	{GUI_REGION DrawRegTmp={109,292,22,22,0,FatColor(NO_TRANS)};
	Gui_Draw24Bmp("Theme/F/Common/Btn/DotN.bmp",&DrawRegTmp);}
}

//-----------------------��ҳϵͳ����----------------------
//����ĳЩ�¼�ʱ���ᴥ���ĺ���
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam)
{
	OP_HEADER *pHeader;
	//GUI_REGION DrawRegion;
	OPS_HEADER *pOptionsBuf=pSysParam;
	
	switch(SysEvent)
	{
		case Sys_PreGotoPage:
			if(strcmp((void *)Q_GetCurrPageName(),"KeyBoardPage")==0) break;//�Ӽ���ҳ�淵��
			if(pOptionsBuf->ChkNum!=gCurChkNum)//��黺���Ƿ���ȷ
			{
				Debug("SettingsPage %s:ID is error!!!\n\r",__FUNCTION__);
				return SM_NoGoto;
			}
			break;
		case Sys_PageInit:		//ϵͳÿ�δ����ҳ�棬�ᴦ������¼�				
			DrawSettingsBg(pOptionsBuf->Name);

			if(IntParam==TRUE)
			{
				u8 i;
				
				gOpsVars=Q_PageMallco(sizeof(SETTINGS_PAGE_VARS));
				for(i=0;i<SETTINGS_PAGE_MAX_PAGE_OPTION_NUM;i++)//��ʼ����̬��������
				{
					gOpsVars->DynImgTchIdx=1;
					memcpy(&gOpsVars->OptionsTch[i],&DefaultTch,sizeof(IMG_BUTTON_OBJ));
					gOpsVars->OptionsTch[i].ObjID=Option1KV+i;
				}
				
				OptionsParse(pSysParam);

				i=1;
				while((pHeader=GetOptionHeader(pSysParam,i++))!=NULL)//��ȡѡ��
				{
					Debug("Id %d,Name:%s\n\r",pHeader->Id,pHeader->Name);
				}
			}
			break;
		case Sys_SubPageReturn:	//�������ҳ�淵��,�Ͳ��ᴥ��Sys_Page_Init�¼�,����Sys_SubPage_Return
			DrawSettingsBg(gOpsVars->OpsBuf->Name);
			break;
		case Sys_TouchSetOk:
			gOpsVars->OpsBuf=pSysParam;
			gOpsVars->CurrPage=1;
		case Sys_TouchSetOk_SR:
			DrawPage(gOpsVars->OpsBuf,gOpsVars->CurrPage);		
			break;
		case Sys_PageClean:
			gCurChkNum=0;
			Q_PageFree(gOpsVars);
			break;
		case Sys_PreSubPage://����ҳ��֮ǰ
			break;
		default:
			//��Ҫ��Ӧ���¼�δ����
			Debug("%s SystemEventHandler:This System Event Handler case unfinish! SysEvent:%d\n\r",Q_GetCurrPageName(),SysEvent);
			//while(1);
	}
	
	return 0;
}
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam)
{
	switch(PeripEvent)
	{	
		case Perip_KeyPress:
			switch(IntParam){
				case ExtiKeyEnter:
					break;
				case ExtiKeyUp:
					Q_PresentTch(LeftArrowKV,Tch_Press);
					break;
				case ExtiKeyDown:
					Q_PresentTch(RightArrowKV,Tch_Press);
					break; 
			}break;
		case Perip_KeyRelease:
			switch(IntParam){
				case ExtiKeyEnter:
			#if 0
				{
					void *p;
					p=Q_PageMallco(128);
					sprintf(p,"<Q-SYS Input Page>\n\r�汾: 2.1\n\r����: ChenYu\n\rQQ: 409262796");
					Q_GotoPage(GotoSubPage,"KeyBoardPage",128,p);
				}break;
			#endif
					PrtScreen();
					break;
				case ExtiKeyUp:
				    Q_PresentTch(LeftArrowKV,Tch_Release);
					if(gOpsVars->CurrPage-1>=1)
						DrawPage(gOpsVars->OpsBuf,--gOpsVars->CurrPage);	
					break;
				case ExtiKeyDown:
					Q_PresentTch(RightArrowKV,Tch_Release);
					if(gOpsVars->CurrPage+1<=gOpsVars->PageTotal)
						DrawPage(gOpsVars->OpsBuf,++gOpsVars->CurrPage);
					break; 
			}break;
	}

	return 0;
}
//��ʹ���߰��±�ҳTouchRegionSet�ﶨ��İ���ʱ���ᴥ�����������Ķ�Ӧ�¼�
static CO_MSG ButtonHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo)
{		
	//GUI_REGION DrawRegion;
	OP_HEADER *pHeader;
	u8 TchIdx;
	
	switch(Key)
	{
		
		case BackKV:
			if(InEvent!=Tch_Release) return 0;
				Q_GotoPage(SubPageReturn,"",0,gOpsVars->OpsBuf);//�����淵��
			break;
		case DoneKV:
			if(InEvent!=Tch_Release) return 0;
			if(CheckModify(gOpsVars->OpsBuf)==TRUE)//����޸�������ֵ
				Q_GotoPage(SubPageReturn,"",gCurChkNum&0xff,gOpsVars->OpsBuf);
			else
				Q_GotoPage(SubPageReturn,"",0,gOpsVars->OpsBuf);
			break;
		
		case LeftArrowKV:
			if(gOpsVars->CurrPage-1>=1)
				DrawPage(gOpsVars->OpsBuf,--gOpsVars->CurrPage);	
			break;
		//case DotKV:
			//break;
		
		case RightArrowKV:
			if(gOpsVars->CurrPage+1<=gOpsVars->PageTotal)
				DrawPage(gOpsVars->OpsBuf,++gOpsVars->CurrPage);
			break;
		case Option1KV:
		case Option2KV:
		case Option3KV:
		case Option4KV:
		case Option5KV:
		case Option6KV:
		case Option7KV:
		case Option8KV:
		case Option9KV:
		case Option10KV:
		case Option11KV:
		case Option12KV:
			TchIdx=Key-Option1KV;
			if((pHeader=GetPagesOption(TchIdx+1))!=NULL)//��ȡoption item buf
				switch(pHeader->Type)
				{
					case OPT_YesOrNo:
						if(InEvent==Tch_Press)
							DrawYesOrNoOption(OpIncPressFlag,pHeader,TRUE,0,&gOpsVars->OptionsTch[TchIdx]);
						else if(InEvent==Tch_Release) DrawYesOrNoOption(OpIncFlag,pHeader,TRUE,0,&gOpsVars->OptionsTch[TchIdx]);
						else DrawYesOrNoOption(OpReloadBtnFlag,pHeader,TRUE,0,&gOpsVars->OptionsTch[TchIdx]);
						break;
					case OPT_Num:
					case OPT_NumList:
					case OPT_NumEnum:
						if(InEvent==Tch_Press)
						{
							if(pTouchInfo->x<gOpsVars->OptionsTch[TchIdx].x+(gOpsVars->OptionsTch[TchIdx].w>>1))
								DrawNumOption(OpDecPressFlag,pHeader,0,0,&gOpsVars->OptionsTch[TchIdx]);
							else
								DrawNumOption(OpIncPressFlag,pHeader,0,0,&gOpsVars->OptionsTch[TchIdx]);
						}
						else if(InEvent==Tch_Release)
						{
							if(pTouchInfo->x<gOpsVars->OptionsTch[TchIdx].x+(gOpsVars->OptionsTch[TchIdx].w>>1))
								DrawNumOption(OpDecFlag,pHeader,0,0,&gOpsVars->OptionsTch[TchIdx]);
							else
								DrawNumOption(OpIncFlag,pHeader,0,0,&gOpsVars->OptionsTch[TchIdx]);
						}
						else 
						{
							DrawNumOption(OpReloadBtnFlag,pHeader,0,0,&gOpsVars->OptionsTch[TchIdx]);
						}
						break;
				}
				
			if(InEvent!=Tch_Press)
			if(CheckModify(gOpsVars->OpsBuf)==TRUE)//����޸�������ֵ�������޸ı�־
			{
				GUI_REGION DrawRegion={109,292,22,22,0,FatColor(NO_TRANS)};
				Gui_Draw24Bmp("Theme/F/Common/Btn/DotP.bmp",&DrawRegion);
			}
			else
			{
				GUI_REGION DrawRegion={109,292,22,22,0,FatColor(NO_TRANS)};
				Gui_Draw24Bmp("Theme/F/Common/Btn/DotN.bmp",&DrawRegion);
			}
			break;
		default:
			//��Ҫ��Ӧ���¼�δ����
			Debug("%s ButtonHandler:This Touch Event Handler case unfinish! Key:%d\n\r",Q_GetCurrPageName(),Key);
			///while(1);
	}
	
	return 0;
}



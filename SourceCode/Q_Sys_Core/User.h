/*																										
 * FileName:        	User.h													       
 * Author:          	YuanYin															  
 * Hardware Version:	QXW-Summer-V1.x												
 * Date: 				2010-11-04																
 * Description:		���ļ����������ͺͺ�������ֻ�ᱻҳ��ʹ��			
 * ҳ��Ҳֻ��ʹ�ñ�ҳ����������ͺͺ�������										
 * �Ժ�ϵͳ��ܸ�����																			
 * version:				Q-OS V1.1												   			 */
 
/*****************************************************
 *  ��ѧ��	Q-Share We	����-��Դ-����						
 *  						   													
 *  ���������ǵĲ�Ʒ���κν������������ϵ����! 	
 *						   														
 *  �Ա�רӪ��Q-ShareWe.TaoBao.Com	    						
 *  ������̳��Www.Q-ShareWe.Com									
 *****************************************************/

#ifndef QSYS_USER_H
#define QSYS_USER_H

/* Ports Macro*/
#define PACKED	__packed//������֧�ֵİ������������ṹ��
//#define NULL ((void *)0)
#define Bit(bit)		(1<<bit)
#define SetBit(reg,bit)  reg|=(1<<bit)
#define ClrBit(reg,bit) reg&=~(1<<bit)
#define ReadBit(reg,bit) ((reg>>bit)&1)

//1																�궨��                               
//2												ϵͳ�趨												
//3						ҳ�津��������						
#define MAX_IMG_KEY_NUM	32//ÿ��ҳ���Զ��������̬ͼƬ������Ŀ
#define MAX_CHAR_KEY_NUM	32//ÿ��ҳ���Զ��������̬���ְ�����Ŀ
#define MAX_DYN_CTRL_OBJ_NUM	24//ÿ��ҳ������Ķ�̬�ؼ�������Ŀ��������̬������yesno��numbox...�ȿؼ�
#define EXTI_KEY_VALUE_START 0x00 //ÿ��ҳ�涨����ⲿ����ö�٣�����Ӵ����ֿ�ʼ
#define USER_KEY_VALUE_START 0x40	//ÿ��ҳ�涨��İ���ö�٣�����Ӵ����ֿ�ʼ
#define MAX_TOUCH_PRESS_MS	5000 //�����������Ч����ʱ��
#define IMG_KEY_NOR_DEF_SUFFIX 'N'//������ͨ״̬ͼ��Ĭ�Ϻ�׺
#define IMG_KEY_PRS_DEF_SUFFIX 'P' //��������״̬ͼ��Ĭ�Ϻ�׺
#define IMG_KEY_REL_DEF_SUFFIX 'R' //�����ͷ�״̬ͼ��Ĭ�Ϻ�׺
//3					ҳ�津��������	����					

//3								������									

//3								������									

//3							����											
#define EXTI_KEY_MAX_NUM	3 	//�ⲿ��������
#define MAX_PATH_LEN		512 //ϵͳ�·���ֽ���Ϊ1024
#define MAX_PAGE_TRACK_NUM 32//����ҳ��ۼ���¼��
#define MAX_PAGE_LAYER_NUM 8//����ҳ�������¼��
#define MAX_GOBAL_SYSEVT	32 //ϵͳ֧�ֵ����ȫ���¼���Ŀ
#define UART_BUF_LEN 128 //���ڻ����С
#define MAX_BMP_PATH_LEN  64//֧�����bmp��Դ·������
#define MAX_SAME_KEY_VALUE_NUM 8//֧�����8����������ļ�ֵ��ͬ

//��������ͼ���޷�����ʱ��ʹ���������ɫ
#define REGION_BG_COLOR		0xe0e0e0
#define NORMAL_REPLACE_COLOR 	0xbbbbbb
#define PRESS_REPLACE_COLOR		0x999999
#define RELEASE_REPLACE_COLOR	0xbbbbbb
#define KEY_NAME_COLOR	0x333333

//3						����		����									
//2										ϵͳ�趨	����											


//2													ϵͳ��												
//#define MAX_LEN_FILENAME (_MAX_LFN * (_DF1S ? 2 : 1) + 1)//�ļ�ϵͳ���ļ�������
#define MAX_LEN_FILENAME (NAME_MAX + 1)//�ļ�ϵͳ���ļ�������

//2											ϵͳ��		����										



//1														ͷ�ļ�����																	
//2									 Standard includes.										 
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>

//2									 RTOS includes.												
#include "Os_Wrap.h"

//2							 FileSystem includes 											
#include "FsWrap.h"

//2								Drivers includes.												
#include "Drivers.h"
#include "Debug.h"
#include "Rtc.h"

//2								 Q-Sys includes													
#include "Uart.h"
#include "Debug.h"
#include "Q_Heap.h"
#include "Q_Gui.h"
#include "Q_Database.h"
#include "Q_Shell.h"
#include "QWebApi.h"
#include "Resources.h"
#include "PublicFunc.h"
#include "ControlObj.h"
//1													ͷ�ļ�����			����													




//2											ȫ���߳����ȼ�										
#if OS_USE_UCOS										
//����ucos���ԣ�����ԽС���ȼ�Խ��
typedef enum{
	SYSTEM_TASK_PRIORITY=OS_HIGHEST_PRIORITIES,
	SYS_MUTEX_MAX_PRIORITY,
	MUSIC_TASK_PRIORITY=SYS_MUTEX_MAX_PRIORITY+5,//֧�����5��������
	TOUCH_TASK_PRIORITY,
	INPUT_TASK_PRIORITY,
	RF_DATA_TASK_PRIORITY,
	KEYS_TASK_PRIORITY,
	//�û�������������ȼ�������
	WAVE_DISP_TASK_PRIORITY,
	
}TASK_PRIORITY;
#endif
#if OS_USE_FREERTOS
//����ucos���ԣ�����Խ�����ȼ�Խ��
#define SYSTEM_TASK_PRIORITY   22
#define SYS_MUTEX_MAX_PRIORITY 21
#define MUSIC_TASK_PRIORITY    20
#define TOUCH_TASK_PRIORITY	   19
#define INPUT_TASK_PRIORITY	   18
#define RF_DATA_TASK_PRIORITY  17
#define KEYS_TASK_PRIORITY	   16

#endif
//2									ȫ���߳����ȼ�	����									

//1														�궨�� ����                              



//1														��������																		
typedef enum{
	PRID_Null=-1,
	PRID_Current=0,
	PRID_MainPage,
	PRID_MusicPage,
	PRID_FileListPage,
	PRID_NumCtrlObjPage,
	PRID_StrCtrlObjPage,
	PRID_EBookPage,
	PRID_PicturePage,
	PRID_SettingsPage,
	PRID_TestPage,
	PRID_KeyBoardPage,
	PRID_QWebPage,
	PRID_ChatPage,
	PRID_NewsPage,
	PRID_AppListPage,
	PRID_SnakePage,
	PRID_TouchCheckPage,
	PRID_WaveDispPage,
	//��ҳ���RID���嵽����
	
}PAGE_RID;//4 	ҳ��RID

typedef enum {
	// ��ͨҳ��
	NORMAL_PAGE=0,

	//����ҳ�棬����ͨҳ����������²�ͬ
	// 1.���뵯��ҳ�棬ֻ������ҳ�����ݽ��룬��ͨ������GotoPage(GotoSubPage)����
	// 2.�ڵ���ҳ���ڣ�������GotoPage�����κ�����ҳ�棬ֻ����GotoPage(SubPageReturn)����֮ǰ����ҳ��
	// 3.���뵯��ҳ��ʱ�����ᴥ����ҳ���Clean�¼�
	// 4.�ӵ���ҳ�淵��ʱ�����ᴥ����ҳ���Sys_PreGotoPage��Sys_SubPageReturn��case
	// 5.�ӵ���ҳ�淵��ʱ���������ҳ��Ŀؼ����л滭�����ỹԭ�ؼ������������Ч�ԣ���˲��ᴥ��Sys_TouchSetOk��Sys_TouchSetOk_SR��case
	// 6.�ӵ���ҳ�淵��ʱ��ֻ�ᴥ����ҳ���Sys_PopPageReturn�¼�
	// 7.ʵ���ϣ����Խ�����ҳ�濴������ҳ������Կؼ�����ʱʧЧ
	POP_PAGE,
}PAGE_TYPE;//4		ҳ������

typedef enum {
	GotoNewPage=0,//����һ����ҳ�棬ԭ������ҳ�漰��������ҳ������Ƚ�����ķ�ʽִ��Sys_SubPageReturn��Sys_PageClean
	GotoSubPage,//����һ����ҳ�棬ҳ��㼶����һ�㣬��ҳ���ִ��Sys_PreSubPage
	SubPageReturn,//���ص���һ��ҳ�棬ҳ��㼶��һ�㣬����ҳ���ִ��Sys_PageClean�����ص���ҳ���ִ��Sys_SubPageReturn
	SubPageTranslate,//��һ����ҳ����ת������һ��ͬ������ҳ�棬�㼶���䣬��ǰҳ���ִ��Sys_PageClean����ҳ���ִ��Sys_PageInit
}PAGE_ACTION;

//4	����ҳ���SystemHandler����ֵ��ּ�ڸ���ϵͳ��һЩ����
#define SYS_MSG u32
//bit0-bit15 �����û��Զ��巵��ֵ
#define SM_RET_MASK 0xffff
//bit16-30���ڻش������ϵͳ������ֻ֧��15���ش�����
#define SM_CMD_MASK 0x7fff0000
#define SM_CMD_OFFSET 16
#define SM_NoGoto (1<<16)//��Sys_GotoPage���ص�Q_GotoPage�����Բ�����GotoPage����
#define SM_NoPageClean (1<<17)//��Sys_GotoPage���ص�Q_GotoPage�����Բ���������ҳ���Sys_PageClean��Sys_PreSubPage�¼�������!!!
#define SM_NoPageInit (1<<18)	//��Sys_GotoPage/Sys_PageClean���أ����Բ���������ҳ���Sys_PageInit��Sys_SubPageReturn�¼�������!!!
#define SM_NoTouchInit (1<<19) //��Sys_GotoPage/Sys_PageClean/Sys_Page_Init/Sys_SubPage_Return����·��ش�ֵ������ֹͣҳ�水������Ļ���
#define SM_NoPopReturn (1<<20) //��Sys_GotoPage/Sys_PageClean/Sys_Page_Init/Sys_SubPage_Return/Sys_TouchSetOk����·��ش�ֵ������ֹͣҳ����ӦSys_PopPageReturn�¼�
#define SM_TouchOff (1<<21)//��SystemEventHandler���ش�ֵ����������ҳ��Ĵ�����Ӧ
#define SM_TouchOn (1<<22)//��SystemEventHandler���ش�ֵ����������ҳ��Ĵ�����Ӧ
#define SM_ExtiKeyOff (1<<23) //��SystemEventHandler���ش�ֵ���ر��ⲿ��������
#define SM_ExtiKeyOn (1<<24) //��SystemEventHandler���ش�ֵ�������ⲿ��������

//bit31 ���ڻش�״̬��ϵͳ
#define SM_STATE_MASK 0x80000000
#define SM_STATE_OFFSET 31
#define SM_State_OK 0
#define SM_State_Faile 0x80000000

typedef enum {
	Sys_PreGotoPage=0,//��һ��ҳ�����GotoPage����ʱ���ᴥ����Ҫ����ҳ��Ĵ��¼���
									//Sys_PreGotoPage���case���൱����ǰ��ҳ����Ӻ����������ǵ�ǰҳ��ĺ��������Ǻ�Sys_PageInit�ı�������
									//�����case����SM_NoGoto����ô���൱��ֻ�����˴����ڵ�ǰҳ��Դ���е�һ���������ѣ��������ᴥ����ҳ��Ķ�����
									
	Sys_PageInit,	//ÿ�δ�ҳ�����Ȼᴥ�����¼���
	Sys_SubPageReturn,//����ҳ���أ���Sys_PageInit��ԡ�
	
	Sys_TouchSetOk,//ҳ��򿪺󣬰������������ᴥ�����¼���
	Sys_TouchSetOk_SR,//��Sys_TouchSetOk��ԣ�ֻ������ҳ�Ǵ���ҳ�淵�صġ�

	Sys_PopPageReturn,//����ҳ�淵��
	
	Sys_PageSync,		//���û��̷߳���ͬ����Ϣʱ���ᴥ�����¼���
	
	Sys_PageClean,	//ҳ���ɵ�ǰҳ���ɷǵ�ǰҳ��ʱ����,���˳�ʱ���á�ParamTable(SYS_EVT,NewPageRegID,NewPagePtr)
	Sys_PreSubPage,//��Ҫ������ҳ�棬��Sys_PageClean����Եġ�ParamTable(SYS_EVT,NewPageRegID,NewPagePtr)
	
	Sys_Error,//����

	SysEvt_MaxNum,
}SYS_EVT;//4	����SystemEventHandler���¼�

typedef enum{
	Perip_KeyPress,//�ⲿ�������¡�ParamTable(PERIP_EVT,KeyValue,EXIT_KEY_INFO*)
	Perip_KeyRelease,//�ⲿ�����ͷš�ParamTable(PERIP_EVT,KeyValue,EXIT_KEY_INFO*)
	
	Perip_RtcSec,		//�������֣�ÿ�붼�ᴥ�����¼����������Σ�Ĭ�Ϲرա�ParamTable(PERIP_EVT,(u16)RTC_Counter,NULL)
	Perip_RtcMin,	//ÿ�ֶ��ᷢ�����¼����������Σ�Ĭ�Ϲرա�ParamTable(PERIP_EVT,(u16)RTC_Counter,NULL)
	Perip_RtcAlarm,//RTC���ڱ������������Σ�Ĭ�Ϲرա�ParamTable(PERIP_EVT,(u16)RTC_Counter,NULL)
	Perip_Timer,		//��ʱ�����ڣ�����֧�ֶ����ʱ������IntParam����ͬ��ʱ�������˳�ҳ��ʱ����رյ�ǰҳ�Ķ�ʱ��!!!ParamTable(PERIP_EVT,(Q_TIM_ID)TIM ID,NULL)
	
	Perip_LcdOff,//lcd��ʱϨ�𴥷����¼����������Σ�Ĭ�Ϲرա�ParamTable(PERIP_EVT,(u16)RTC_Counter,NULL)
	Perip_LcdOn,//lcd Ϩ��󣬱��������µ����ᴥ�����¼����������Σ�Ĭ�Ϲرա�ParamTable(PERIP_EVT,(u16)RTC_Counter,NULL)

	Perip_UartInput,//�������롣ParamTable(PERIP_EVT,(ComNum<<16)|StrBytes,u8 *Str)

	Perip_MscPlay,//��ʼ����һ�������ļ�ʱ������ParamTable(PERIP_EVT,0,u8 *FilePath)
	Perip_MscPause,//�����ļ����ű���ͣʱ������ParamTable(PERIP_EVT,PlayTime,u8 *FilePath)
	Perip_MscContinue,//�����ļ�����ͣ�����������ʱ��������ParamTable(PERIP_EVT,PlayTime,u8 *FilePath)
	Perip_MscStop,//�����ļ��������ʱ��������ParamTable(PERIP_EVT,PlayTime,u8 *FilePath)

	Perip_QWebJoin,//(����)�´ӻ�����;(�ӻ�)��ȡ���ӻ���ַ.ParamTable(PERIP_EVT,Addr,u8 *DeviceName)
	Perip_QWebRecv,//�յ�Q������.ParamTable(PERIP_EVT,(Addr<<24)|DataLen,u8 *pData)
	Perip_QWebSendOk,//����Q���������.ParamTable(PERIP_EVT,(Addr<<24)|DataLen,u8 *pData)
	Perip_QWebSendFailed,//����Q������ʧ��.ParamTable(PERIP_EVT,(Addr<<24)|DataLen,u8 *pData)
	Perip_QWebHostConflict,//������ͻ
	Perip_QWebQueryName,//�յ�һ��queryӦ��.ParamTable(PERIP_EVT,Addr,u8 *DeviceName)
	Perip_QWebError,//����
}PERIP_EVT;//4	����PeripheralsHandler���¼�

typedef struct{//sizeof=8
	s32 IntParam;
	void *pParam;
}SYNC_INFO;//ͬ���������ͣ�ҳ��ͬ���������������Ҫ�õ�������

typedef struct{
	u16 Id;	//ֻ�а��²Żᵼ��id++,������ҳ�����ƥ��id���ж��ǲ���ͬһ�δ�������
	u16 TimeStamp;//ÿ��������ʱ�䣬���ֵ����λms
}EXIT_KEY_INFO;//��������ⲿ�����¼���Ϣ��Perip_Key�¼�����ʱ���ᴫ��PeripheralsHandlerFunc�ĵ���������

typedef enum {
	NoHasSysEvt=0,//���账��
	HasPagePeripEvt,//ҳ���¼�
	HasGobalSysEvt//ȫ���¼�
}INSPECT_SYSEVT_RET;//InspectSysEvt��������ֵ

//��һ��������ϵͳ�¼��������������û��Զ��塣
//�û����ݵ�һ�������ж�Ҫ�����ĸ���֧��
typedef SYS_MSG (*SystemHandlerFunc)(SYS_EVT ,int , void *);

typedef SYS_MSG (*PeripheralsHandlerFunc)(PERIP_EVT, int, void *);

//4	ҳ���OptionMask����ֵ(���֧��16������)
//#define ThemeMsk (1<<0)  //һ����������ʾʹ������Ŀ¼����ô��ť����ʱ�����Բ���·��ǰ׺

typedef struct PAGE_ATTRIBUTE{
	u8 * Name; //ҳ�����ƣ�����Ϊ�ַ�����������GotoPage�����õ�
	u8 * Author; //����
	u8 * Description;//����
	PAGE_TYPE Type;//ҳ������
	u16 OptionsMask;//ҳ�湦�����룬�����ã�����չ
	
	PAGE_CONTROL_NUM CtrlObjNum;//�ؼ�������¼
	
	const IMG_BUTTON_OBJ *pImgButtonCon;//ҳ��İ�ť������������ҳ�����е�ͼƬ����
	const CHAR_BUTTON_OBJ *pCharButtonCon;//ҳ��İ�ť������������ҳ�����е����ְ���

	SystemHandlerFunc SysEvtHandler;//ҳ���SystemEventHandlerʵ�壬����ϵͳ�¼�����Ҫ��ҳ���л�������Ϊ��
	PeripheralsHandlerFunc PeripEvtHandler;//ҳ��PeripheralsHandlerFuncʵ�壬������Χ�¼�������Ϊ��
	u32 PeripEvtInitMask;
	
	ButtonHandlerFunc ButtonHandler;//ҳ���TouchEventHandlerʵ�塣����Ϊ��
	YesNoHandlerFunc YesNoHandler;//����YesNo�ؼ�
	NumCtrlObjHanderFunc NumCtrlObjHander;//����Num�ؼ�
	StrCtrlObjHandlerFunc StrCtrlObjHandler;//����Str�ؼ�
}PAGE_ATTRIBUTE;//4	ÿ��ҳ�涼�ᶨ������Խṹ��

//4	spi flash��
typedef enum{
	FlashNothing=0,
	FlashRead,
	FlashWrite,
	FlashSectorEarse,
	FlashBlukEarse
}FLASH_CMD;
//4	spi flash�����

//4		��ʱ����
typedef enum{//�󲿷�ҳ��ֻ����һ����ʱ����Q_TIM1��������ֲʱ����ֻ��ֲ1����ʱ��
	Q_TIM1=0,//ʵ��ʹ��Tim2
	Q_TIM2,//ʵ��ʹ��Tim4
	Q_TIM3,//ʵ��ʹ��Tim5
}Q_TIM_ID;
//4��ʱ�������

//1												��������		����																

//1												ȫ�ֱ�������																		


//1												ȫ�ֱ�������		����														

//1													��������																			
//2										��ϵͳ����											

#if Q_HEAP_TRACK_DEBUG ==1
#define Q_PageMallco(n) _Q_PageMallco(n,(void *)__func__,__LINE__)
#define Q_PageFree(p) _Q_PageFree(p,(void *)__func__,__LINE__)
void *_Q_PageMallco(u16 Size,u8 *pFuncName,u32 Lines);
void _Q_PageFree(void *Ptr,u8 *pFuncName,u32 Lines);
#else
#define Q_PageMallco _Q_PageMallco //ҳ��ʹ�õ��ڴ���亯�� 
#define Q_PageFree _Q_PageFree  //ҳ��ʹ�õ��ڴ��ͷź���
void *_Q_PageMallco(u16 Size);
void _Q_PageFree(void *Ptr);
#endif

//ҳ����ת����,��ڲ���NameΪҳ������
//����ҳ�棬ҲҪѸ�ٸ������ָ��ָ�������
//��Ϊ���ָ��ָ����ڴ���ʱ�ᱻע��
//�ɹ��᷵����ҳ��Sys_PreGotoPage���case�����ص�SYS_MSGֵ
//ֻ����ҳ���SystemEventHandler��TouchEventHandler�����е��ã�������ܻ�����һЩ��Ԥ�ϵĴ���
SYS_MSG Q_GotoPage(PAGE_ACTION PageAction, u8 *Name, int IntParam, void *pSysParam);

//��ȡҳ������
u8 Q_GetPageTotal(void);

//ָ����Ե�ǰҳ��ĺۼ�ƫ��ֵ������ҳ��ָ��
//��GetPageByTrack(0)���ص�ǰҳ��ָ��
//Q_GetPageByTrack(1)����ǰһҳ��ָ��
const PAGE_ATTRIBUTE *Q_GetPageByTrack(u8 Local);

//LayerOffset=0,���ص�ǰҳ��ָ��
//LayerOffset=1,������һ��ҳ��ָ��
const PAGE_ATTRIBUTE *Q_GetPageByLayerOffset(u8 LayerOffset);

//�õ�ָ�����ҳ��ָ��
//LayerNum=1,�õ�����
//LayerNum=2,�õ��ڶ���
const PAGE_ATTRIBUTE *Q_GetPageByLayer(u8 LayerNum);

//����һ��ҳ�����ƣ����Է������Ӧ��RID
//���û�ҵ�������0
//���Name==NULL����Name[0]=0���򷵻ص�ǰҳ���RID
PAGE_RID Q_FindRidByPageName(u8 *PageName);

//���ص�ǰҳ�������
u8 *Q_GetCurrPageName(void);

//��ȡ��ǰҳ��������
u16 Q_GetPageEntries(void);

//�������������룬�ⲿ��������
void Q_EnableInput(void);

//�رմ��������룬�ⲿ��������
void Q_DisableInput(void);

//������һ�麯�����ڿ�����Ӧ�¼�����
void Q_SetPeripEvt(PAGE_RID RegID,u32 SysEvtCon);//��������¼���Ӧ����SetSysEvt((1<<Sys_TouchSetOk)|(1<<Perip_RtcSec));
void Q_ClrPeripEvt(PAGE_RID RegID,u32 SysEvtCon);//�رն���¼���Ӧ����ClrSysEvt((1<<Sys_TouchSetOk)|(1<<Perip_RtcSec));
void Q_EnablePeripEvt(PAGE_RID RegID,PERIP_EVT SysEvent);//����һ���¼���Ӧ����EnableSysEvt(Perip_RtcSec);
void Q_DisablePeripEvt(PAGE_RID RegID,PERIP_EVT SysEvent);//�ر�һ���¼���Ӧ����DisableSysEvt(Perip_RtcSec);
INSPECT_SYSEVT_RET Q_InspectPeripEvt(PAGE_RID RegID,PERIP_EVT SysEvent);//���ָ���¼��Ƿ���Ա���Ӧ����InspectSysEvt(Perip_RtcSec);

//����ȫ���¼����κ�ҳ���£����ᴥ���¼��Ĵ�����PeripEvtHandler
void Q_EnableGobalPeripEvent(PERIP_EVT PeripEvt,PeripheralsHandlerFunc PeripEvtHandler);

//ע��ȫ���¼�
void Q_DisableGobalPeripEvent(PERIP_EVT PeripEvt,PeripheralsHandlerFunc PeripEvtHandler);

//���ڴ���ֹͣ���˺������ܷ��أ�ֱ����ѭ��
#define Q_ErrorStopScreen(Msg) Q_ErrorStop(__FILE__,__func__,__LINE__,Msg);
void Q_ErrorStop(const char *FileName,const char *pFuncName,const u32 Line,const char *Msg);

//��Ϊsummer v1.4�İ汾�vs��spi flash����һ��spi�ߣ�Ϊ��ֹ����
//ֻ�ܵ��ô˺�����ȡflash������
bool Q_SpiFlashSync(FLASH_CMD cmd,u32 addr,u32 len,u8 *buf);

//�����û��Զ�ʱ�������ã������ö����ʱ������ֲʱ���ɿ���ֻ��ֲ1������
void Q_TimSet(Q_TIM_ID TimId,u16 Val,u16 uS_Base, bool AutoReload);

//����ҳ��Perip_UartInput�¼��£���������������
void Q_UartCmdHandler(u16 Len,u8 *pStr);

//ҳ��Run�̺߳�ҳ��ͬ���ĺ���
//�ɹ�����TRUE
bool Q_PageSync(int IntParam,void *pSysParam,u32 TicksToWait);

//2									��ϵͳ����	����									



//1													��������		����															

#endif


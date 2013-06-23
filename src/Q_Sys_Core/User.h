/*																										
 * FileName:        	User.h													       
 * Author:          	YuanYin															  
 * Hardware Version:	QXW-Summer-V1.x												
 * Date: 				2010-11-04																
 * Description:		本文件的数据类型和函数声明只会被页面使用			
 * 页面也只能使用本页面的数据类型和函数声明										
 * 以和系统框架隔绝。																			
 * version:				Q-OS V1.1												   			 */
 
/*****************************************************
 *  酷学玩	Q-Share We	快乐-开源-分享						
 *  						   													
 *  如果你对我们的产品有任何建议或不满，请联系我们! 	
 *						   														
 *  淘宝专营：Q-ShareWe.TaoBao.Com	    						
 *  技术论坛：Www.Q-ShareWe.Com									
 *****************************************************/

#ifndef QSYS_USER_H
#define QSYS_USER_H

/* Ports Macro*/
#define PACKED	__packed//编译器支持的按定义紧密排序结构体
//#define NULL ((void *)0)
#define Bit(bit)		(1<<bit)
#define SetBit(reg,bit)  reg|=(1<<bit)
#define ClrBit(reg,bit) reg&=~(1<<bit)
#define ReadBit(reg,bit) ((reg>>bit)&1)

//1																宏定义                               
//2												系统设定												
//3						页面触摸区域定义						
#define MAX_IMG_KEY_NUM	32//每个页面自定义的最大固态图片按键数目
#define MAX_CHAR_KEY_NUM	32//每个页面自定义的最大固态文字按键数目
#define MAX_DYN_CTRL_OBJ_NUM	24//每个页面允许的动态控件的总数目，包含动态按键，yesno，numbox...等控件
#define EXTI_KEY_VALUE_START 0x00 //每个页面定义的外部按键枚举，必须从此数字开始
#define USER_KEY_VALUE_START 0x40	//每个页面定义的按键枚举，必须从此数字开始
#define MAX_TOUCH_PRESS_MS	5000 //触摸区域最长有效按下时间
#define IMG_KEY_NOR_DEF_SUFFIX 'N'//按键普通状态图标默认后缀
#define IMG_KEY_PRS_DEF_SUFFIX 'P' //按键按下状态图标默认后缀
#define IMG_KEY_REL_DEF_SUFFIX 'R' //按键释放状态图标默认后缀
//3					页面触摸区域定义	结束					

//3								主题项									

//3								主题项									

//3							杂项											
#define EXTI_KEY_MAX_NUM	3 	//外部按键个数
#define MAX_PATH_LEN		512 //系统最长路径字节数为1024
#define MAX_PAGE_TRACK_NUM 32//最大的页面痕迹记录数
#define MAX_PAGE_LAYER_NUM 8//最大的页面层数记录数
#define MAX_GOBAL_SYSEVT	32 //系统支持的最多全局事件数目
#define UART_BUF_LEN 128 //串口缓冲大小
#define MAX_BMP_PATH_LEN  64//支持最长的bmp资源路径长度
#define MAX_SAME_KEY_VALUE_NUM 8//支持最大8个触摸区域的键值相同

//触摸区域图标无法访问时，使用如下替代色
#define REGION_BG_COLOR		0xe0e0e0
#define NORMAL_REPLACE_COLOR 	0xbbbbbb
#define PRESS_REPLACE_COLOR		0x999999
#define RELEASE_REPLACE_COLOR	0xbbbbbb
#define KEY_NAME_COLOR	0x333333

//3						杂项		结束									
//2										系统设定	结束											


//2													系统宏												
//#define MAX_LEN_FILENAME (_MAX_LFN * (_DF1S ? 2 : 1) + 1)//文件系统长文件名长度
#define MAX_LEN_FILENAME (NAME_MAX + 1)//文件系统长文件名长度

//2											系统宏		结束										



//1														头文件包含																	
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
//1													头文件包含			结束													




//2											全局线程优先级										
#if OS_USE_UCOS										
//对于ucos而言，数字越小优先级越高
typedef enum{
	SYSTEM_TASK_PRIORITY=OS_HIGHEST_PRIORITIES,
	SYS_MUTEX_MAX_PRIORITY,
	MUSIC_TASK_PRIORITY=SYS_MUTEX_MAX_PRIORITY+5,//支持最多5个互斥量
	TOUCH_TASK_PRIORITY,
	INPUT_TASK_PRIORITY,
	RF_DATA_TASK_PRIORITY,
	KEYS_TASK_PRIORITY,
	//用户定义的任务优先级放这里
	WAVE_DISP_TASK_PRIORITY,
	
}TASK_PRIORITY;
#endif
#if OS_USE_FREERTOS
//对于ucos而言，数字越大优先级越高
#define SYSTEM_TASK_PRIORITY   22
#define SYS_MUTEX_MAX_PRIORITY 21
#define MUSIC_TASK_PRIORITY    20
#define TOUCH_TASK_PRIORITY	   19
#define INPUT_TASK_PRIORITY	   18
#define RF_DATA_TASK_PRIORITY  17
#define KEYS_TASK_PRIORITY	   16

#endif
//2									全局线程优先级	结束									

//1														宏定义 结束                              



//1														类型声明																		
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
	//新页面的RID定义到这里
	
}PAGE_RID;//4 	页面RID

typedef enum {
	// 普通页面
	NORMAL_PAGE=0,

	//弹出页面，和普通页面相比有如下不同
	// 1.进入弹出页面，只能以子页面的身份进入，即通过调用GotoPage(GotoSubPage)进入
	// 2.在弹出页面内，不能用GotoPage进入任何其他页面，只能用GotoPage(SubPageReturn)返回之前的主页面
	// 3.进入弹出页面时，不会触发子页面的Clean事件
	// 4.从弹出页面返回时，不会触发主页面的Sys_PreGotoPage、Sys_SubPageReturn等case
	// 5.从弹出页面返回时，不会对主页面的控件进行绘画，但会还原控件触碰区域的有效性，因此不会触发Sys_TouchSetOk、Sys_TouchSetOk_SR等case
	// 6.从弹出页面返回时，只会触发主页面的Sys_PopPageReturn事件
	// 7.实际上，可以将弹出页面看做是主页面的所以控件的临时失效
	POP_PAGE,
}PAGE_TYPE;//4		页面类型

typedef enum {
	GotoNewPage=0,//进入一个新页面，原来的主页面及其所有子页面会以先进后出的方式执行Sys_SubPageReturn或Sys_PageClean
	GotoSubPage,//进入一个子页面，页面层级深入一层，主页面会执行Sys_PreSubPage
	SubPageReturn,//返回到上一个页面，页面层级减一层，此子页面会执行Sys_PageClean，返回到的页面会执行Sys_SubPageReturn
	SubPageTranslate,//从一个子页面跳转到另外一个同级的子页面，层级不变，当前页面会执行Sys_PageClean，新页面会执行Sys_PageInit
}PAGE_ACTION;

//4	用于页面的SystemHandler返回值，旨在告诉系统做一些事情
#define SYS_MSG u32
//bit0-bit15 用于用户自定义返回值
#define SM_RET_MASK 0xffff
//bit16-30用于回传命令给系统，所以只支持15个回传命令
#define SM_CMD_MASK 0x7fff0000
#define SM_CMD_OFFSET 16
#define SM_NoGoto (1<<16)//从Sys_GotoPage返回到Q_GotoPage，可以不触发GotoPage动作
#define SM_NoPageClean (1<<17)//从Sys_GotoPage返回到Q_GotoPage，可以不触发调用页面的Sys_PageClean或Sys_PreSubPage事件，慎用!!!
#define SM_NoPageInit (1<<18)	//从Sys_GotoPage/Sys_PageClean返回，可以不触发调用页面的Sys_PageInit或Sys_SubPageReturn事件，慎用!!!
#define SM_NoTouchInit (1<<19) //从Sys_GotoPage/Sys_PageClean/Sys_Page_Init/Sys_SubPage_Return情况下返回此值，可以停止页面按键区域的绘制
#define SM_NoPopReturn (1<<20) //从Sys_GotoPage/Sys_PageClean/Sys_Page_Init/Sys_SubPage_Return/Sys_TouchSetOk情况下返回此值，可以停止页面响应Sys_PopPageReturn事件
#define SM_TouchOff (1<<21)//在SystemEventHandler返回此值，将不开启页面的触摸响应
#define SM_TouchOn (1<<22)//在SystemEventHandler返回此值，将不开启页面的触摸响应
#define SM_ExtiKeyOff (1<<23) //在SystemEventHandler返回此值，关闭外部按键输入
#define SM_ExtiKeyOn (1<<24) //在SystemEventHandler返回此值，开启外部按键输入

//bit31 用于回传状态给系统
#define SM_STATE_MASK 0x80000000
#define SM_STATE_OFFSET 31
#define SM_State_OK 0
#define SM_State_Faile 0x80000000

typedef enum {
	Sys_PreGotoPage=0,//当一个页面调用GotoPage函数时，会触发将要进入页面的此事件。
									//Sys_PreGotoPage这个case就相当于是前个页面的子函数，而不是当前页面的函数，这是和Sys_PageInit的本质区别。
									//如果此case返回SM_NoGoto，那么就相当于只调用了存在于当前页面源码中的一个函数而已，根本不会触发新页面的动作。
									
	Sys_PageInit,	//每次打开页面首先会触发此事件。
	Sys_SubPageReturn,//从子页返回，和Sys_PageInit相对。
	
	Sys_TouchSetOk,//页面打开后，按键描绘结束，会触发此事件。
	Sys_TouchSetOk_SR,//和Sys_TouchSetOk相对，只不过本页是从子页面返回的。

	Sys_PopPageReturn,//弹出页面返回
	
	Sys_PageSync,		//当用户线程发出同步信息时，会触发此事件。
	
	Sys_PageClean,	//页面由当前页面变成非当前页面时调用,即退出时调用。ParamTable(SYS_EVT,NewPageRegID,NewPagePtr)
	Sys_PreSubPage,//将要进入子页面，和Sys_PageClean是相对的。ParamTable(SYS_EVT,NewPageRegID,NewPagePtr)
	
	Sys_Error,//错误

	SysEvt_MaxNum,
}SYS_EVT;//4	传入SystemEventHandler的事件

typedef enum{
	Perip_KeyPress,//外部按键按下。ParamTable(PERIP_EVT,KeyValue,EXIT_KEY_INFO*)
	Perip_KeyRelease,//外部按键释放。ParamTable(PERIP_EVT,KeyValue,EXIT_KEY_INFO*)
	
	Perip_RtcSec,		//除了整分，每秒都会触发此事件。可以屏蔽，默认关闭。ParamTable(PERIP_EVT,(u16)RTC_Counter,NULL)
	Perip_RtcMin,	//每分都会发出此事件。可以屏蔽，默认关闭。ParamTable(PERIP_EVT,(u16)RTC_Counter,NULL)
	Perip_RtcAlarm,//RTC到期报警。可以屏蔽，默认关闭。ParamTable(PERIP_EVT,(u16)RTC_Counter,NULL)
	Perip_Timer,		//定时器到期，可以支持多个定时器，以IntParam区别不同定时器。在退出页面时，请关闭当前页的定时器!!!ParamTable(PERIP_EVT,(Q_TIM_ID)TIM ID,NULL)
	
	Perip_LcdOff,//lcd超时熄灭触发此事件。可以屏蔽，默认关闭。ParamTable(PERIP_EVT,(u16)RTC_Counter,NULL)
	Perip_LcdOn,//lcd 熄灭后，被触碰重新点亮会触发此事件。可以屏蔽，默认关闭。ParamTable(PERIP_EVT,(u16)RTC_Counter,NULL)

	Perip_UartInput,//串口输入。ParamTable(PERIP_EVT,(ComNum<<16)|StrBytes,u8 *Str)

	Perip_MscPlay,//开始播放一个音乐文件时触发。ParamTable(PERIP_EVT,0,u8 *FilePath)
	Perip_MscPause,//音乐文件播放被暂停时触发。ParamTable(PERIP_EVT,PlayTime,u8 *FilePath)
	Perip_MscContinue,//音乐文件从暂停进入继续播放时被触发。ParamTable(PERIP_EVT,PlayTime,u8 *FilePath)
	Perip_MscStop,//音乐文件播放完毕时被触发。ParamTable(PERIP_EVT,PlayTime,u8 *FilePath)

	Perip_QWebJoin,//(主机)新从机加入;(从机)获取到从机地址.ParamTable(PERIP_EVT,Addr,u8 *DeviceName)
	Perip_QWebRecv,//收到Q网数据.ParamTable(PERIP_EVT,(Addr<<24)|DataLen,u8 *pData)
	Perip_QWebSendOk,//发送Q网数据完成.ParamTable(PERIP_EVT,(Addr<<24)|DataLen,u8 *pData)
	Perip_QWebSendFailed,//发送Q网数据失败.ParamTable(PERIP_EVT,(Addr<<24)|DataLen,u8 *pData)
	Perip_QWebHostConflict,//主机冲突
	Perip_QWebQueryName,//收到一个query应答.ParamTable(PERIP_EVT,Addr,u8 *DeviceName)
	Perip_QWebError,//错误
}PERIP_EVT;//4	传入PeripheralsHandler的事件

typedef struct{//sizeof=8
	s32 IntParam;
	void *pParam;
}SYNC_INFO;//同步参数类型，页面同步，串口输入等需要用到此类型

typedef struct{
	u16 Id;	//只有按下才会导致id++,所以在页面可以匹配id以判断是不是同一次触碰动作
	u16 TimeStamp;//每个动作的时间，相对值，单位ms
}EXIT_KEY_INFO;//用来存放外部按键事件信息，Perip_Key事件发生时，会传入PeripheralsHandlerFunc的第三个参数

typedef enum {
	NoHasSysEvt=0,//无需处理
	HasPagePeripEvt,//页面事件
	HasGobalSysEvt//全局事件
}INSPECT_SYSEVT_RET;//InspectSysEvt函数返回值

//第一参数传递系统事件，后两个参数用户自定义。
//用户根据第一参数来判断要进入哪个分支。
typedef SYS_MSG (*SystemHandlerFunc)(SYS_EVT ,int , void *);

typedef SYS_MSG (*PeripheralsHandlerFunc)(PERIP_EVT, int, void *);

//4	页面的OptionMask掩码值(最大支持16个掩码)
//#define ThemeMsk (1<<0)  //一旦开启，表示使用主题目录，那么按钮定义时，可以不加路径前缀

typedef struct PAGE_ATTRIBUTE{
	u8 * Name; //页面名称，必须为字符串，可以在GotoPage里面用到
	u8 * Author; //作者
	u8 * Description;//描述
	PAGE_TYPE Type;//页面类型
	u16 OptionsMask;//页面功能掩码，暂无用，留扩展
	
	PAGE_CONTROL_NUM CtrlObjNum;//控件个数记录
	
	const IMG_BUTTON_OBJ *pImgButtonCon;//页面的按钮集，用来定义页面所有的图片按键
	const CHAR_BUTTON_OBJ *pCharButtonCon;//页面的按钮集，用来定义页面所有的文字按键

	SystemHandlerFunc SysEvtHandler;//页面的SystemEventHandler实体，处理系统事件，主要是页面切换。不能为空
	PeripheralsHandlerFunc PeripEvtHandler;//页面PeripheralsHandlerFunc实体，处理外围事件。不能为空
	u32 PeripEvtInitMask;
	
	ButtonHandlerFunc ButtonHandler;//页面的TouchEventHandler实体。不能为空
	YesNoHandlerFunc YesNoHandler;//处理YesNo控件
	NumCtrlObjHanderFunc NumCtrlObjHander;//处理Num控件
	StrCtrlObjHandlerFunc StrCtrlObjHandler;//处理Str控件
}PAGE_ATTRIBUTE;//4	每个页面都会定义的属性结构体

//4	spi flash项
typedef enum{
	FlashNothing=0,
	FlashRead,
	FlashWrite,
	FlashSectorEarse,
	FlashBlukEarse
}FLASH_CMD;
//4	spi flash项结束

//4		定时器项
typedef enum{//大部分页面只用了一个定时器，Q_TIM1，所以移植时可以只移植1个定时器
	Q_TIM1=0,//实际使用Tim2
	Q_TIM2,//实际使用Tim4
	Q_TIM3,//实际使用Tim5
}Q_TIM_ID;
//4定时器项结束

//1												类型声明		结束																

//1												全局变量声明																		


//1												全局变量声明		结束														

//1													函数声明																			
//2										酷系统函数											

#if Q_HEAP_TRACK_DEBUG ==1
#define Q_PageMallco(n) _Q_PageMallco(n,(void *)__func__,__LINE__)
#define Q_PageFree(p) _Q_PageFree(p,(void *)__func__,__LINE__)
void *_Q_PageMallco(u16 Size,u8 *pFuncName,u32 Lines);
void _Q_PageFree(void *Ptr,u8 *pFuncName,u32 Lines);
#else
#define Q_PageMallco _Q_PageMallco //页面使用的内存分配函数 
#define Q_PageFree _Q_PageFree  //页面使用的内存释放函数
void *_Q_PageMallco(u16 Size);
void _Q_PageFree(void *Ptr);
#endif

//页面跳转函数,入口参数Name为页面名称
//在新页面，也要迅速复制入口指针指向的内容
//因为入口指针指向的内存随时会被注销
//成功会返回新页面Sys_PreGotoPage这个case所返回的SYS_MSG值
//只能在页面的SystemEventHandler和TouchEventHandler函数中调用，否则可能会引起一些非预料的错误
SYS_MSG Q_GotoPage(PAGE_ACTION PageAction, u8 *Name, int IntParam, void *pSysParam);

//获取页面总数
u8 Q_GetPageTotal(void);

//指定相对当前页面的痕迹偏移值，返回页面指针
//如GetPageByTrack(0)返回当前页面指针
//Q_GetPageByTrack(1)返回前一页面指针
const PAGE_ATTRIBUTE *Q_GetPageByTrack(u8 Local);

//LayerOffset=0,返回当前页面指针
//LayerOffset=1,返回上一层页面指针
const PAGE_ATTRIBUTE *Q_GetPageByLayerOffset(u8 LayerOffset);

//得到指定层的页面指针
//LayerNum=1,得到顶层
//LayerNum=2,得到第二层
const PAGE_ATTRIBUTE *Q_GetPageByLayer(u8 LayerNum);

//输入一个页面名称，可以返回其对应的RID
//如果没找到，返回0
//如果Name==NULL或者Name[0]=0，则返回当前页面的RID
PAGE_RID Q_FindRidByPageName(u8 *PageName);

//返回当前页面的名字
u8 *Q_GetCurrPageName(void);

//获取当前页面进入次数
u16 Q_GetPageEntries(void);

//开启触摸屏输入，外部按键输入
void Q_EnableInput(void);

//关闭触摸屏输入，外部按键输入
void Q_DisableInput(void);

//下面这一组函数用于控制响应事件开关
void Q_SetPeripEvt(PAGE_RID RegID,u32 SysEvtCon);//开启多个事件响应，如SetSysEvt((1<<Sys_TouchSetOk)|(1<<Perip_RtcSec));
void Q_ClrPeripEvt(PAGE_RID RegID,u32 SysEvtCon);//关闭多个事件响应，如ClrSysEvt((1<<Sys_TouchSetOk)|(1<<Perip_RtcSec));
void Q_EnablePeripEvt(PAGE_RID RegID,PERIP_EVT SysEvent);//开启一个事件响应，如EnableSysEvt(Perip_RtcSec);
void Q_DisablePeripEvt(PAGE_RID RegID,PERIP_EVT SysEvent);//关闭一个事件响应，如DisableSysEvt(Perip_RtcSec);
INSPECT_SYSEVT_RET Q_InspectPeripEvt(PAGE_RID RegID,PERIP_EVT SysEvent);//检查指定事件是否可以被响应，如InspectSysEvt(Perip_RtcSec);

//设置全局事件，任何页面下，都会触发事件的处理函数PeripEvtHandler
void Q_EnableGobalPeripEvent(PERIP_EVT PeripEvt,PeripheralsHandlerFunc PeripEvtHandler);

//注销全局事件
void Q_DisableGobalPeripEvent(PERIP_EVT PeripEvt,PeripheralsHandlerFunc PeripEvtHandler);

//用于错误停止，此函数不能返回，直接死循环
#define Q_ErrorStopScreen(Msg) Q_ErrorStop(__FILE__,__func__,__LINE__,Msg);
void Q_ErrorStop(const char *FileName,const char *pFuncName,const u32 Line,const char *Msg);

//因为summer v1.4的版本里，vs和spi flash共用一个spi线，为防止出错
//只能调用此函数获取flash的数据
bool Q_SpiFlashSync(FLASH_CMD cmd,u32 addr,u32 len,u8 *buf);

//用于用户对定时器的设置，可设置多个定时器，移植时，可考虑只移植1个即可
void Q_TimSet(Q_TIM_ID TimId,u16 Val,u16 uS_Base, bool AutoReload);

//置于页面Perip_UartInput事件下，用来处理串口命令
void Q_UartCmdHandler(u16 Len,u8 *pStr);

//页面Run线程和页面同步的函数
//成功返回TRUE
bool Q_PageSync(int IntParam,void *pSysParam,u32 TicksToWait);

//2									酷系统函数	结束									



//1													函数声明		结束															

#endif


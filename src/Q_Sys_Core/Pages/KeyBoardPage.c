#include "User.h"
#include "KeyBoardPage.h"
#include "PinYinForKeyBoardPage.h"

//-------------------------------------------宏定义&类型声明-------------------------------------------------------
#define KB_WHITE				    (FatColor(0xFFFFFF))
#define KB_BLACK					(FatColor(0x000000))
#define	KB_ORANGE					(FatColor(0xE47833))

#define	KB_TEXTBOXCROSS_LINE			5		 //无回车全英文情况下一次翻页所翻的行数
#define	KB_TEXTBOX_POS_X				7        //文本框x坐标
#define	KB_TEXTBOX_POS_Y				5		 //文本框y坐标
#define	KB_TEXTBOX_W					213		 //文本框宽度
#define	KB_TEXTBOX_H					110		 //文本框高度
#define	KB_TEXTBOX_CHAR_SPACE			1		 //文本框字间距
#define	KB_TEXTBOX_ROW_SPACE			1		 //文本框行间距
#define	KB_TEXTBOXCHAR_COLOUR			KB_BLACK    //文本框字体颜色
#define	KB_TEXTBOXBG_COLOUR				KB_WHITE	 //文本框背景颜色
#define	KB_CROSSBAR_COLOUR				KB_ORANGE     //滚动条颜色
#define KB_CROSSBOT_SIZE				14	 	 //滚动条按钮大小
#define	KB_TEXTBOXCROSS_NUM				(KB_TEXTBOX_W/(8+KB_TEXTBOX_CHAR_SPACE)*KB_TEXTBOXCROSS_LINE)
#define	KB_CROSSBAR_INI_X				(KB_TEXTBOX_POS_X+KB_TEXTBOX_W+5)						
#define	KB_CROSSBAR_INI_Y				(KB_TEXTBOX_POS_Y+KB_CROSSBOT_SIZE)
#define	KB_CROSSBAR_INI_W				(KB_CROSSBOT_SIZE-2)						
#define	KB_CROSSBAR_INI_H				(KB_TEXTBOX_H-KB_CROSSBOT_SIZE*2+1)
#define	KB_PINYINBUF_MAX  	 	 	 	6      	//拼音框最多可容纳的字符数
#define	KB_PINYINBOXCHAR_COLOUR			KB_BLACK   //拼音颜色
#define	KB_PINYINBOXCHARBG_COLOUR		KB_WHITE   //拼音背景颜色
#define	KB_MAYBEBOXCHAR_COLOUR			KB_BLACK	//疑似汉字颜色
#define	KB_MAYBEBOXCHARBG_COLOUR		KB_WHITE   //疑似汉字背景颜色
#define	KB_PINYINBOX_POS_X				1   	//拼音框x坐标
#define	KB_PINYINBOX_POS_Y				122		//拼音框y坐标
#define	KB_PINYINBOX_W					49		//拼音框宽度
#define	KB_PINYINBOX_H					16		//拼音框高度
#define	KB_PINYINBOX_CHAR_SPACE	 		0		//拼音框字间距
#define	KB_PINYINBOX_ROW_SPACE		 	0		//拼音框行间距
#define	KB_MAYBEBOX_POS_X				51  	//疑似汉字子框x坐标
#define	KB_MAYBEBOX_POS_Y				122		//疑似汉字子框y坐标
#define	KB_MAYBEBOX_W					187		//疑似汉字子框宽度
#define	KB_MAYBEBOX_H					34		//疑似汉字子框高度
#define	KB_MAYBEBOX_CHAR_SPACE	 	 	3		//疑似汉字子框字间距
#define	KB_MAYBEBOX_ROW_SPACE		 	2		//疑似汉字子框行间距
#define KB_NUM_MODE	   					0x1		//数字模式
#define KB_EN_MODE	   					0x2		//英文模式
#define KB_PINYIN_MODE					0x4		//拼音模式
#define KB_CAP_MODE						0x10	//大写子模式
#define	KB_TEMPBUF_MAX  	 	 	 	10      //缓存大小 
#define KB_DKTIME						5000    //用来判断多击的时延

typedef enum{
	//键盘按键
	KB_PRESS_1=USER_KEY_VALUE_START,  
	KB_PRESS_2,  KB_PRESS_3,  KB_PRESS_4,  KB_PRESS_5,  KB_PRESS_6,  
	KB_PRESS_7,  KB_PRESS_8,  KB_PRESS_9,  KB_PRESS_10, KB_PRESS_11, 
	KB_PRESS_12, KB_PRESS_13, KB_PRESS_14, KB_PRESS_15, KB_PRESS_16, 
	KB_PRESS_17, KB_PRESS_18, KB_PRESS_19, KB_PRESS_20, KB_PRESS_21, 
	//翻页键
	KB_PRESS_22,	KB_PRESS_23,
	//拼音选择区域
	KB_PY_SELECT,
	//拼音翻页
	KB_PRESS_24,	KB_PRESS_25
}KeyBoardPage_OID;
//------------------------对象声明-------------------------------
//注:由对象自身调用，而且只改变对象自身的状态的函数被归为私有函数
//	 由对象自身调用，而且将改变其他对象的状态的函数被归为操纵函数
//	 接口函数是其他对象访问该对象的唯一方式
//---------------------------------------------------------------
//文本框对象类型声明
typedef struct{	
	//对象属性
	u8  *elem;      					//缓存区
	u16	OUTBUF_SIZE;					//输出缓冲区大小
	s32 tailnextnext;	     		    //缓存区最后一个元素的下下一个元素的位置
	s32 ShowBegin;               		//从第ShowBegin个字节开始显示
	u32 TotalPage;						//总页数 
	u32 CurPage;                 		//当前是第几页
	GUI_REGION TextBoxRegion;    		//文本显示区域
	GUI_REGION ScrollbarRegion;  		//滚动条显示区域
	//私有函数
	void (*ShowFromPos) (void);	    	//从ShowBegin位置开始在文本框内显示字符串
	void (*RefuCrossBar)(void);			//刷新滚动条
	//操纵函数 无
	//接口函数
	void (*Init)(u8 *p,s32 bufsizemsg); //对象初始化
	bool (*InsTail)(u8 inchar);			//在尾部插入字符
	bool (*InsTailChain)(u8 *str);		//在尾部插入字符串
	void (*ClrTail)(void);				//删除尾部的字符
	void (*ClearAll)(void);				//删除所有字符
	void (*UpPage)(void);				//上翻页
	void (*DownPage)(void);				//下翻页
	void (*ShowPrepa)(u8 *str);			//在尾部显示预备字符串
	void (*EndEdit)(void);				//结束编辑
}KB_TextBoxDef;

//拼音框对象类型声明
typedef struct{
	u8  elem[KB_PINYINBUF_MAX+1];       	//缓存区
	u8	tailnext;					 	//缓存区最后一个元素的下一个元素的位置
	const u8 *pmbstr;	 				//指向疑似字符串
	u8 curbase;	 						//疑似字符串显示基点位置
	u8 strnum; 							//疑似字符串的长度
	GUI_REGION PinYinRegion;    		//拼音显示区域
	GUI_REGION MayBeRegion;  			//疑似汉字条显示区域
	//私有函数
	void (*ClrPinYinShow)(void);	 	//清除拼音显示
	void (*ClrMayBeShow)(void);		 	//清除疑似汉字显示
	void (*ShowPinYinBox)(void);	 	//显示缓存区的内容
	void (*ShowMayBeFromPos)(void);	 	//从curbase位置开始在疑似汉字框内显示字符串
	void (*ClrMayBe)(void);			 	//清除疑似字符串
	//操纵函数 	
	void (*Upload)(KB_TextBoxDef *ptextbox,u8 order);//将选中的汉字上传到文本框对象
	void (*UploadHalf)(KB_TextBoxDef *ptextbox,u8 order);//将选中的半角字符上传到文本框对象	
	//接口函数
	void (*Init)(void);   			 	//对象初始化
	void (*Clr)(void);   		     	//对象清空
	void (*LinkMayBe)(const u8* str);	//链接全角到疑似字符串
	void (*LinkMayBeHalf)(const u8* str);//链接半角到疑似字符串
	bool (*InsTailChain)(u8 *str);	 	//在尾部插入字符串
	void (*ClrTail)(void);			 	//删除尾部的字符
	void (*NextMayBePage)(void);	 	//下翻页疑似全角字符框
	void (*NextMayBePageHalf)(void);	 //下翻页疑似半角字符框
	void (*PreMayBePage)(void);	 		//上翻页疑似全角字符框
	void (*PreMayBePageHalf)(void);	 	//上翻页疑似半角字符框

	void (*PleaseUpload)(KB_TextBoxDef *ptextbox,u8 order);//请求对象将选中的汉字上传到文本框对象
	void (*PleaseUploadHalf)(KB_TextBoxDef *ptextbox,u8 order);//请求对象将选中的半角字符上传到文本框对象
	bool (*IfEditPinYin)(void);			//是否在编辑拼音
	void (*ShowPrepa)(u8 *str);			//在尾部显示预备字符串				
}KB_PinYinBoxDef;

//临时缓存区对象类型声明
typedef struct{
	u8  elem[KB_TEMPBUF_MAX+1];       		//缓存区
	u8  decodebuf[KB_TEMPBUF_MAX+1];       //解码缓存区
	u8	tailnext;				   		//缓存区最后一个元素的下一个元素的位置
	//私有函数
	void (*Decode)(void);   	   		//对对象缓存区进行解码，从而区分出单击双击多击
	//操纵函数
	void (*UploadText)(KB_TextBoxDef *ptextbox);//将临时缓存区的字符串上传到文本框对象													  
	void (*UploadPinYin)(KB_PinYinBoxDef *ppinyinbox);//将临时缓存区的字符串上传到拼音框对象			
	//接口函数
	void (*Init)(void);   		   		//初始化对象
	void (*Clr)(void);   		   		//清空对象
	bool (*InsTail)(u8 inchar);	   		//在对象尾部插入字符	
	void (*PleaseUploadText)(KB_TextBoxDef *ptextbox);//请求对象将临时缓存区的字符串上传到文本框对象													  
	void (*PleaseUploadPinYin)(KB_PinYinBoxDef *ppinyinbox);//请求对象将临时缓存区的字符串上传到拼音框对象	
}KB_TempBufDef;

//模式框对象类型声明
typedef struct{
	u8  ModeFlag;   					 //模式标志
	bool IfOffPinYin;					 //是否关闭拼音模式标志
	bool IfOffNum;						 //是否关闭数字模式标志
	bool IfOffEn;						 //是否关闭英文模式标志
	bool IfOffPunc;						 //是否关闭标点
	//私有函数
	void (*ShowMode)(void);			  	 //模式框显示当前模式
	//操纵函数
	void (*ChangeModeAct)(KB_PinYinBoxDef *ppinyinbox,u8 newmode);	//执行模式切换
	//接口函数
	void (*Init)(s32 modemsg);   		  //初始化对象
	u8 (*GetMode)(void);				  //获得当前模式
	void (*Capital)(void);				  //置大写
	void (*UnCapital)(void);			  //取消大写
	bool (*IfCapital)(void);			  //判断当前是否大写
	void (*PleaseChangeMode)(KB_PinYinBoxDef *ppinyinbox,u8 newmode); //请求对象发送切换到新模式	
}KB_ModeBoxDef;

//------------全局变量类型声明-------------			
typedef struct{
   KB_TextBoxDef   KB_TextBox;   //文本框对象
   KB_TempBufDef   KB_TempBuf;   //临时缓存对象
   KB_PinYinBoxDef KB_PinYinBox; //拼音框对象
   KB_ModeBoxDef   KB_ModeBox;   //模式对象
}KEYBOARD_PAGE_VARS;

//---------------------------------------------函数声明-----------------------------------------------------------
void KB_TextBox_Init(u8 *p,s32 bufsizemsg);   			    //初始化文本框对象
void KB_TextBox_ShowFromPos(void);	    					//文本框对象从指定位置开始在显示字符串
void KB_TextBox_RefuCrossBar(void);						//文本框对象刷新滚动条
bool KB_TextBox_InsTail(u8 inchar);						//在文本框对象尾部插入字符
bool KB_TextBox_InsTailChain(u8 *str);						//在文本框尾部对象插入字符串
void KB_TextBox_ClrTail(void);								//删除文本框对象尾部的字符
void KB_TextBox_ClearAll(void);								//删除文本框对象内所有字符
void KB_TextBox_UpPage(void);								//上翻页文本框对象
void KB_TextBox_DownPage(void);							//下翻页文本框对象
void KB_TextBox_ShowPrepa(u8 *str);						//在尾部显示预备字符串
void KB_TextBox_EndEdit(void);								//结束编辑
void KB_TempBuf_Init(void);   		   						//初始化临时缓存区对象
void KB_TempBuf_Decode(void);   	   						//对缓存区进行解码，从而区分出单击双击多击
void KB_TempBuf_Clr(void);   		   						//清空临时缓存区对象
bool KB_TempBuf_InsTail(u8 inchar);	   					//在临时缓存区尾部插入字符
void KB_TempBuf_UploadText(KB_TextBoxDef *ptextbox);			//临时缓存区将临时缓存区的字符串上传到文本框对象
void KB_TempBuf_UploadPinYin(KB_PinYinBoxDef *ppinyinbox);	//临时缓存区将临时缓存区的字符串上传到文本框对象
void KB_TempBuf_PleaseUploadText(KB_TextBoxDef *ptextbox);	//请求对象将临时缓存区的字符串上传到文本框对象													  
void KB_TempBuf_PleaseUploadPinYin(KB_PinYinBoxDef *ppinyinbox);//请求对象将临时缓存区的字符串上传到拼音框对象
void KB_PinYinBox_ClrPinYinShow(void);		     			//拼音框对象清除疑似汉字显示
void KB_PinYinBox_ClrMayBeShow(void);		     			//拼音框对象清除疑似汉字显示
void KB_PinYinBox_ShowPinYinBox(void);	 					//拼音框对象显示拼音
void KB_PinYinBox_ShowMayBeFromPos(void);	 				//拼音框对象从curbase位置开始在疑似汉字框内显示字符串
void KB_PinYinBox_ClrMayBe(void);			 				//拼音框对象清除疑似字符串
void KB_PinYinBox_LinkMayBe(const u8* str);				//拼音框对象将全角字符链接到疑似字符串
void KB_PinYinBox_LinkMayBeHalf(const u8* str);			//拼音框对象将半角字符链接到疑似字符串
void KB_PinYinBox_NextMayBePage(void);	 					//下翻页疑似全角字符框
void KB_PinYinBox_NextMayBePageHalf(void);	 					//下翻页疑似全角字符框
void KB_PinYinBox_PreMayBePage(void);	 					//上翻页疑似半角字符框
void KB_PinYinBox_PreMayBePageHalf(void);	 					//上翻页疑似半角字符框
void KB_PinYinBox_Init(void);   			 				//初始化拼音框对象
void KB_PinYinBox_Clr(void);   		     				//清空拼音框对象
bool KB_PinYinBox_InsTailChain(u8 *str);	 				//在拼音框对象尾部插入字符串
void KB_PinYinBox_ClrTail(void);			 				//删除拼音框对象尾部的字符
bool KB_PinYinBox_IfEditPinYin(void);						//是否在编辑拼音		
void KB_PinYinBox_Upload(KB_TextBoxDef *ptextbox,u8 order);	//拼音框对象将选中的全角字符上传到文本框对象
void KB_PinYinBox_UploadHalf(KB_TextBoxDef *ptextbox,u8 order);//拼音框对象将选中的半角字符上传到文本框对象
void KB_PinYinBox_PleaseUpload(KB_TextBoxDef *ptextbox,u8 order);//请求对象将选中的全角字符上传到文本框对象
void KB_PinYinBox_PleaseUploadHalf(KB_TextBoxDef *ptextbox,u8 order);//请求对象将选中的半角字符上传到文本框对象
void KB_PinYinBox_ShowPrepa(u8 *str);						//在尾部显示预备字符串
void KB_ModeBox_ShowMode(void);			  				//模式框显示当前模式
void KB_ModeBox_Init(s32 modemsg);   		   	  				    //对象初始化
void KB_ModeBox_PleaseChangeMode(KB_PinYinBoxDef *ppinyinbox,u8 newmode); //切换到新模式
u8 	 KB_ModeBox_GetMode(void);				  				//获得当前模式
void KB_ModeBox_ChangeModeAct(KB_PinYinBoxDef *ppinyinbox,u8 newmode);//执行模式切换
void KB_ModeBox_Capital(void);				  				//置大写
void KB_ModeBox_UnCapital(void);			 			 	//取消大写
bool KB_ModeBox_IfCapital(void);			  				//判断当前是否大写
static void KB_DrawBG(void);								//画背景
static void KB_DrawBlock(void);							//画格线
static void KB_DrawPress(u8 xx,u8 yy);							//选字提示
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam);
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam);
static CO_MSG ButtonHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo);
//---------------------------------------------页面资源定义-----------------------------------------------------------

static const IMG_BUTTON_OBJ ImgButtonCon[]={
	{"\n 逗号",KB_PRESS_1,			RelMsk,0,157,39,31,0,0,"Comma",FatColor(NO_TRANS)},	
	{"\n 句号",KB_PRESS_6,			RelMsk,0,190,39,31,0,0,"Period",FatColor(NO_TRANS)},
	{"\n省略号",KB_PRESS_11,		RelMsk,0,223,39,31,0,0,"Points",FatColor(NO_TRANS)},
	{"\n感叹号",KB_PRESS_16,		RelMsk,0,256,39,31,0,0,"Excla",FatColor(NO_TRANS)},
	{"\n 问号",KB_PRESS_21,		RelMsk,0,289,39,31,0,0,"Ask",FatColor(NO_TRANS)},

	{"\n 1",KB_PRESS_2,			RelMsk,40,157,52,40,0,0,"1",FatColor(NO_TRANS)},
	{"\n 2 abc",KB_PRESS_3,		RelMsk,93,157,52,40,0,0,"2",FatColor(NO_TRANS)},
	{"\n 3 def",KB_PRESS_4,		RelMsk,146,157,52,40,0,0,"3",FatColor(NO_TRANS)},
	{"\n 删除",KB_PRESS_5,			RelMsk,199,157,40,40,0,0,"Delete",FatColor(NO_TRANS)},
	
	{"\n 4 ghi",KB_PRESS_7,		RelMsk,40,198,52,40,0,0,"4",FatColor(NO_TRANS)},
	{"\n 5 jkl",KB_PRESS_8,		RelMsk,93,198,52,40,0,0,"5",FatColor(NO_TRANS)},
	{"\n 6 mno",KB_PRESS_9,		RelMsk,146,198,52,40,0,0,"6",FatColor(NO_TRANS)},
	{"\n 数字\n 模式",KB_PRESS_10,	RelMsk,199,198,40,40,0,0,"123",FatColor(NO_TRANS)},

	{"\n 7 pqrs",KB_PRESS_12,		RelMsk,40,239,52,40,0,0,"7",FatColor(NO_TRANS)},
	{"\n 8 tuv",KB_PRESS_13,		RelMsk,93,239,52,40,0,0,"8",FatColor(NO_TRANS)},
	{"\n 9 wxyz",KB_PRESS_14,		RelMsk,146,239,52,40,0,0,"9",FatColor(NO_TRANS)},
	{"\n 英文\n 模式",KB_PRESS_15,	RelMsk,199,239,40,40,0,0,"abc",FatColor(NO_TRANS)},

	{"\n 完成",KB_PRESS_17,		RelMsk,40,280,52,40,0,0,"Ok",FatColor(NO_TRANS)},
	{"\n 0 空格",KB_PRESS_18,		RelMsk,93,280,52,40,0,0,"0",FatColor(NO_TRANS)},
	{"\n 回车",KB_PRESS_19,		RelMsk,146,280,52,40,0,0,"Enter",FatColor(NO_TRANS)},
	{"\n 拼音\n 模式",KB_PRESS_20,	RelMsk,199,280,40,40,0,0,"PinYin",FatColor(NO_TRANS)},

	{"∧",KB_PRESS_22,RelMsk,KB_CROSSBAR_INI_X,KB_CROSSBAR_INI_Y-KB_CROSSBOT_SIZE-3,14,14,0,0,"UpArrow",FatColor(NO_TRANS)},
	{"∨",KB_PRESS_23,RelMsk,KB_CROSSBAR_INI_X-1,KB_CROSSBAR_INI_Y+KB_CROSSBAR_INI_H+3,14,14,0,0,"DownArrow",FatColor(NO_TRANS)},

	{">>",KB_PRESS_24,RelMsk,KB_MAYBEBOX_POS_X-25,KB_MAYBEBOX_POS_Y+17,24,18,0,0,"MBDownArrow",FatColor(NO_TRANS)},
	{"<<",KB_PRESS_25,RelMsk,KB_MAYBEBOX_POS_X-50,KB_MAYBEBOX_POS_Y+17,24,18,0,0,"MBUPArrow",FatColor(NO_TRANS)},

	{"",KB_PY_SELECT,RelMsk|PrsMsk,KB_MAYBEBOX_POS_X,KB_MAYBEBOX_POS_Y,KB_MAYBEBOX_W,KB_MAYBEBOX_H,0,0,"",FatColor(NO_TRANS)}
};

//定义页面或者应用的属性集
const PAGE_ATTRIBUTE KeyBoardPage={
	"KeyBoardPage",
	"Author:ChenYu","keyboard",
	NORMAL_PAGE,
	0,
	{
		sizeof(ImgButtonCon)/sizeof(IMG_BUTTON_OBJ), //size of touch region array
		0,//sizeof(CharButtonCon)/sizeof(CHAR_BUTTON_OBJ), //size of touch region array,
	},
	ImgButtonCon, 
	0,
	SystemEventHandler,
	PeripheralsHandler,
	Bit(Perip_Timer)|Bit(Perip_UartInput),
	ButtonHandler,

};

//---------------------------------------全局变量指针----------------------------------------------
static KEYBOARD_PAGE_VARS *gpKbpVars;					
//-------------------------------------最重要的两个系统函数----------------------------------------
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam){
	switch(SysEvent){
		case Sys_PreGotoPage:
			break;
		case Sys_PageInit:		
			KB_DrawBG();//画页面背景			
			gpKbpVars=(KEYBOARD_PAGE_VARS *)Q_PageMallco(sizeof(KEYBOARD_PAGE_VARS));//申请空间				
			 if(gpKbpVars==0){
				Q_ErrorStopScreen("gpKbpVars malloc fail !\n\r");
			 }
			MemSet(gpKbpVars,0,sizeof(KEYBOARD_PAGE_VARS));
			//初始化所有对象
			gpKbpVars->KB_TextBox.Init=KB_TextBox_Init;
			gpKbpVars->KB_PinYinBox.Init=KB_PinYinBox_Init;
			gpKbpVars->KB_ModeBox.Init=KB_ModeBox_Init;
			gpKbpVars->KB_TempBuf.Init=KB_TempBuf_Init;
			gpKbpVars->KB_TextBox.Init(pSysParam,IntParam);			
			gpKbpVars->KB_ModeBox.Init(IntParam);
			gpKbpVars->KB_PinYinBox.Init();
			gpKbpVars->KB_TempBuf.Init();			
			break;
		case Sys_SubPageReturn:	//如果从子页面返回,就不会触发Sys_Page_Init事件,而是Sys_SubPage_Return
			break;

		case Sys_PageClean:
			Q_PageFree(gpKbpVars);
			break;
		default:
			Debug("%s SystemEventHandler:This System Event Handler case unfinish! SysEvent:%d\n\r",Q_GetCurrPageName(),SysEvent);
			//while(1);
	}
	return 0;
}
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam)
{
	u8 mainmode;
	switch(PeripEvent)
	{
		case Perip_Timer:
		{	 //在英文模式下向文本框发送字符
			mainmode=gpKbpVars->KB_ModeBox.GetMode() & 0x0f;
			switch(mainmode){
				case KB_EN_MODE:
				   gpKbpVars->KB_TempBuf.PleaseUploadText(&gpKbpVars->KB_TextBox);
				   break;
				case KB_PINYIN_MODE:
				   gpKbpVars->KB_TempBuf.PleaseUploadPinYin(&gpKbpVars->KB_PinYinBox);
				   break;
			}							
		}break;
		case Perip_UartInput:
		{
			gpKbpVars->KB_TextBox.ClearAll();
			gpKbpVars->KB_TextBox.InsTailChain((u8 *)pParam);
		}break;
	}

	return 0;
}
//当使用者按下本页TouchRegionSet里定义的按键时，会触发这个函数里的对应事件
static CO_MSG ButtonHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo){
	u8 mainmode;

	mainmode = gpKbpVars->KB_ModeBox.GetMode() & 0xf;

	if(mainmode&KB_NUM_MODE){
		switch(Key){
				case KB_PRESS_1:
					if(gpKbpVars->KB_ModeBox.IfOffPunc==FALSE)
						gpKbpVars->KB_TextBox.InsTail(',');
					break;			
				case KB_PRESS_2:gpKbpVars->KB_TextBox.InsTail('1');break;									
				case KB_PRESS_3:gpKbpVars->KB_TextBox.InsTail('2');break;							
				case KB_PRESS_4:gpKbpVars->KB_TextBox.InsTail('3');break;			 					 
				case KB_PRESS_5:gpKbpVars->KB_TextBox.ClrTail();   break;									
				case KB_PRESS_6:
					if(gpKbpVars->KB_ModeBox.IfOffPunc==FALSE)
						gpKbpVars->KB_TextBox.InsTail('.');
					break;		
				case KB_PRESS_7:gpKbpVars->KB_TextBox.InsTail('4');break;				 				   	
				case KB_PRESS_8:gpKbpVars->KB_TextBox.InsTail('5');break;			 							
				case KB_PRESS_9:gpKbpVars->KB_TextBox.InsTail('6');break;				 					
				case KB_PRESS_10:break;			
				case KB_PRESS_11:
					if(gpKbpVars->KB_ModeBox.IfOffPunc==FALSE)
						gpKbpVars->KB_TextBox.InsTailChain("...");
					break;				
				case KB_PRESS_12:gpKbpVars->KB_TextBox.InsTail('7');break;			 						 
				case KB_PRESS_13:gpKbpVars->KB_TextBox.InsTail('8');break;	
				case KB_PRESS_14:gpKbpVars->KB_TextBox.InsTail('9');break;	
				case KB_PRESS_15:gpKbpVars->KB_ModeBox.PleaseChangeMode(&gpKbpVars->KB_PinYinBox,KB_EN_MODE);break;
				case KB_PRESS_16:
					if(gpKbpVars->KB_ModeBox.IfOffPunc==FALSE)
						gpKbpVars->KB_TextBox.InsTail('!');
					break;	
				case KB_PRESS_17:gpKbpVars->KB_TextBox.EndEdit();break;
				case KB_PRESS_18:gpKbpVars->KB_TextBox.InsTail('0');break;	
				case KB_PRESS_19:
					if(gpKbpVars->KB_ModeBox.IfOffPunc==FALSE)
						gpKbpVars->KB_TextBox.InsTailChain("\r\n");
					break;
				case KB_PRESS_20:gpKbpVars->KB_ModeBox.PleaseChangeMode(&gpKbpVars->KB_PinYinBox,KB_PINYIN_MODE);break;
				case KB_PRESS_21:
					if(gpKbpVars->KB_ModeBox.IfOffPunc==FALSE)
						gpKbpVars->KB_TextBox.InsTail('?');
					break;			
				case KB_PRESS_22:gpKbpVars->KB_TextBox.UpPage();    break;
				case KB_PRESS_23:gpKbpVars->KB_TextBox.DownPage();  break;
	    }	
	}else if(mainmode&KB_EN_MODE){
		switch(Key){
				case KB_PRESS_1:
					if(gpKbpVars->KB_ModeBox.IfOffPunc==FALSE)
						gpKbpVars->KB_TextBox.InsTail(',');
					break;		
				case KB_PRESS_2:
					if(gpKbpVars->KB_ModeBox.IfOffPunc==FALSE)
						gpKbpVars->KB_PinYinBox.LinkMayBeHalf (":@;.+-*/%=#$&()<>{}~");
					break;
				case KB_PRESS_3:{
					if(gpKbpVars->KB_ModeBox.IfCapital())
						gpKbpVars->KB_TempBuf.InsTail('A');
					else
						gpKbpVars->KB_TempBuf.InsTail('a');
				}break;		
				case KB_PRESS_4:{
					if(gpKbpVars->KB_ModeBox.IfCapital())
						gpKbpVars->KB_TempBuf.InsTail('D');
					else
						gpKbpVars->KB_TempBuf.InsTail('d');
				}break;			
				case KB_PRESS_5:
					if(gpKbpVars->KB_PinYinBox.IfEditPinYin()){
					 	gpKbpVars->KB_PinYinBox.ClrTail();						
						break;								 	
					}else{
						gpKbpVars->KB_TextBox.ClrTail(); 													
						break;
					}		
				case KB_PRESS_6:
					if(gpKbpVars->KB_ModeBox.IfOffPunc==FALSE)
						gpKbpVars->KB_TextBox.InsTail('.');
					break;		
				case KB_PRESS_7:{
					if(gpKbpVars->KB_ModeBox.IfCapital())
						gpKbpVars->KB_TempBuf.InsTail('G');
					else
						gpKbpVars->KB_TempBuf.InsTail('g');
				}break;				
				case KB_PRESS_8:{
					if(gpKbpVars->KB_ModeBox.IfCapital())
						gpKbpVars->KB_TempBuf.InsTail('J');
					else
						gpKbpVars->KB_TempBuf.InsTail('j');
				}break;			
				case KB_PRESS_9:{
					if(gpKbpVars->KB_ModeBox.IfCapital())
						gpKbpVars->KB_TempBuf.InsTail('M');
					else
						gpKbpVars->KB_TempBuf.InsTail('m');
				}break;			
				case KB_PRESS_10:gpKbpVars->KB_ModeBox.PleaseChangeMode(&gpKbpVars->KB_PinYinBox,KB_NUM_MODE);break;
				case KB_PRESS_11:
					if(gpKbpVars->KB_ModeBox.IfOffPunc==FALSE)
						gpKbpVars->KB_TextBox.InsTailChain("...");
					break;	
				case KB_PRESS_12:{
					if(gpKbpVars->KB_ModeBox.IfCapital())
						gpKbpVars->KB_TempBuf.InsTail('P');
					else
						gpKbpVars->KB_TempBuf.InsTail('p');
				}break;				
				case KB_PRESS_13:{
					if(gpKbpVars->KB_ModeBox.IfCapital())
						gpKbpVars->KB_TempBuf.InsTail('T');
					else
						gpKbpVars->KB_TempBuf.InsTail('t');
				}break;				
				case KB_PRESS_14:{
					if(gpKbpVars->KB_ModeBox.IfCapital())
						gpKbpVars->KB_TempBuf.InsTail('W');
					else
						gpKbpVars->KB_TempBuf.InsTail('w');
				}break;				
				case KB_PRESS_15:{
					if(gpKbpVars->KB_ModeBox.IfCapital())
						gpKbpVars->KB_ModeBox.UnCapital();
					else
						gpKbpVars->KB_ModeBox.Capital();
				}break;
				case KB_PRESS_16:
					if(gpKbpVars->KB_ModeBox.IfOffPunc==FALSE)
						gpKbpVars->KB_TextBox.InsTail('!');
					break;		
				case KB_PRESS_17:gpKbpVars->KB_TextBox.EndEdit();break;
				case KB_PRESS_18:
					if(gpKbpVars->KB_ModeBox.IfOffPunc==FALSE)
						gpKbpVars->KB_TextBox.InsTail(' ');
					break;		
				case KB_PRESS_19:
					if(gpKbpVars->KB_ModeBox.IfOffPunc==FALSE)
						gpKbpVars->KB_TextBox.InsTailChain("\r\n");
					break;
				case KB_PRESS_20:gpKbpVars->KB_ModeBox.PleaseChangeMode(&gpKbpVars->KB_PinYinBox,KB_PINYIN_MODE);break;
				case KB_PRESS_21:
					if(gpKbpVars->KB_ModeBox.IfOffPunc==FALSE)
						gpKbpVars->KB_TextBox.InsTail('?');
					break;		
				case KB_PRESS_22:gpKbpVars->KB_TextBox.UpPage();    break;
				case KB_PRESS_23:gpKbpVars->KB_TextBox.DownPage();  break;
				case KB_PY_SELECT:{
						static u8 i=0,j=0;
						Debug("i=%d j=%d \r\n",i,j);
						if(gpKbpVars->KB_PinYinBox.IfEditPinYin()==FALSE)
							break;						
						if( InEvent!=Tch_Release){
							i=(pTouchInfo->x-KB_MAYBEBOX_POS_X)/19;
							j=(pTouchInfo->y-KB_MAYBEBOX_POS_Y)/18;
							if(gpKbpVars->KB_PinYinBox.curbase+(i+10*j)<gpKbpVars->KB_PinYinBox.strnum)
								KB_DrawPress(KB_MAYBEBOX_POS_X+19*i,KB_MAYBEBOX_POS_Y+18*j);
							else
								break;	
						}										
						else{	
							gpKbpVars->KB_PinYinBox.PleaseUploadHalf(&gpKbpVars->KB_TextBox,i+10*j);
							i=0;
							j=0;
						}
				}break;
				case KB_PRESS_24:gpKbpVars->KB_PinYinBox.NextMayBePageHalf();break;
				case KB_PRESS_25:gpKbpVars->KB_PinYinBox.PreMayBePageHalf();break;		  
		}
	}else if(mainmode&KB_PINYIN_MODE){
		switch(Key){
				case KB_PRESS_1:
					if(gpKbpVars->KB_ModeBox.IfOffPunc==FALSE)
						gpKbpVars->KB_TextBox.InsTailChain("，");
					break;
				case KB_PRESS_2:
					if(gpKbpVars->KB_ModeBox.IfOffPunc==FALSE)
						gpKbpVars->KB_PinYinBox.LinkMayBe ("、；“”：～＠＆﹪＃﹝﹞［］｛｝《》〈〉＋－×÷／＝℃≤≥＜＞≮≯≈≡≠‰☆★○●◎◇◆□■△▲※→←↑↓〓¤︿＿￣―♂♀");
					break;
				case KB_PRESS_3:gpKbpVars->KB_TempBuf.InsTail('a');break;		
				case KB_PRESS_4:gpKbpVars->KB_TempBuf.InsTail('d');break;		
				case KB_PRESS_5:{
					if(gpKbpVars->KB_PinYinBox.IfEditPinYin()){
					 	gpKbpVars->KB_PinYinBox.ClrTail();						
						break;
					}else{
						gpKbpVars->KB_TextBox.ClrTail(); 													
						break;
					}	
				}																			 
				case KB_PRESS_6:
					if(gpKbpVars->KB_ModeBox.IfOffPunc==FALSE)
						gpKbpVars->KB_TextBox.InsTailChain("。");
					break;
				case KB_PRESS_7:gpKbpVars->KB_TempBuf.InsTail('g');break;		
				case KB_PRESS_8:gpKbpVars->KB_TempBuf.InsTail('j');break;		
				case KB_PRESS_9:gpKbpVars->KB_TempBuf.InsTail('m');break;		
				case KB_PRESS_10:gpKbpVars->KB_ModeBox.PleaseChangeMode(&gpKbpVars->KB_PinYinBox,KB_NUM_MODE);break;
				case KB_PRESS_11:
					if(gpKbpVars->KB_ModeBox.IfOffPunc==FALSE)
						gpKbpVars->KB_TextBox.InsTailChain("......");
					break;
				case KB_PRESS_12:gpKbpVars->KB_TempBuf.InsTail('p');break;		
				case KB_PRESS_13:gpKbpVars->KB_TempBuf.InsTail('t');break;		
				case KB_PRESS_14:gpKbpVars->KB_TempBuf.InsTail('w');break;
				case KB_PRESS_15:gpKbpVars->KB_ModeBox.PleaseChangeMode(&gpKbpVars->KB_PinYinBox,KB_EN_MODE);break;
				case KB_PRESS_16:
					if(gpKbpVars->KB_ModeBox.IfOffPunc==FALSE)
						gpKbpVars->KB_TextBox.InsTailChain("！");
					break;
				case KB_PRESS_17:gpKbpVars->KB_TextBox.EndEdit();break;
				case KB_PRESS_18:
					if(gpKbpVars->KB_ModeBox.IfOffPunc==FALSE)
						gpKbpVars->KB_TextBox.InsTailChain("　");
					break;
				case KB_PRESS_19:
					if(gpKbpVars->KB_ModeBox.IfOffPunc==FALSE)
						gpKbpVars->KB_TextBox.InsTailChain("\r\n");
					break;
				case KB_PRESS_20:;break;
				case KB_PRESS_21:
					if(gpKbpVars->KB_ModeBox.IfOffPunc==FALSE)
						gpKbpVars->KB_TextBox.InsTailChain("？");
					break;
				case KB_PRESS_22:gpKbpVars->KB_TextBox.UpPage();    break;
				case KB_PRESS_23:gpKbpVars->KB_TextBox.DownPage();  break;
				case KB_PY_SELECT:{
						static u8 i=0,j=0;
						Debug("i=%d j=%d \r\n",i,j);
						if(gpKbpVars->KB_PinYinBox.IfEditPinYin()==FALSE)
							break;						
						if( InEvent!=Tch_Release){
							i=(pTouchInfo->x-KB_MAYBEBOX_POS_X)/19;
							j=(pTouchInfo->y-KB_MAYBEBOX_POS_Y)/18;
							if(gpKbpVars->KB_PinYinBox.curbase+(i+10*j)*2<gpKbpVars->KB_PinYinBox.strnum)
								KB_DrawPress(KB_MAYBEBOX_POS_X+19*i,KB_MAYBEBOX_POS_Y+18*j);
							else
								break;	
						}										
						else{	
							gpKbpVars->KB_PinYinBox.PleaseUpload(&gpKbpVars->KB_TextBox,i+10*j);
							i=0;
							j=0;
						}
				}break;
				case KB_PRESS_24:gpKbpVars->KB_PinYinBox.NextMayBePage();break;
				case KB_PRESS_25:gpKbpVars->KB_PinYinBox.PreMayBePage();break;		  
			}
	}	
	return 0;
}
static void KB_DrawBG(void){//画背景
	GUI_REGION DrawRegion;
	DrawRegion.x=0;DrawRegion.y=0;
	DrawRegion.w=240;DrawRegion.h=320;
	DrawRegion.Color=FatColor(0x809090);
	Gui_FillBlock(&DrawRegion);
	DrawRegion.Color=FatColor(NO_TRANS);
	Gui_DrawImgBin("Theme/F/KeyBoardPage/Bg/Bg.bin",&DrawRegion);	
}
static void KB_DrawBlock(void){//画格线
	Gui_DrawLine(KB_MAYBEBOX_POS_X,KB_MAYBEBOX_POS_Y+16,KB_MAYBEBOX_POS_X+KB_MAYBEBOX_W, KB_MAYBEBOX_POS_Y+16,KB_ORANGE);
	Gui_DrawLine(KB_MAYBEBOX_POS_X-2+1*19,KB_MAYBEBOX_POS_Y,KB_MAYBEBOX_POS_X-2+1*19, KB_MAYBEBOX_POS_Y+34,KB_ORANGE);
	Gui_DrawLine(KB_MAYBEBOX_POS_X-2+2*19,KB_MAYBEBOX_POS_Y,KB_MAYBEBOX_POS_X-2+2*19, KB_MAYBEBOX_POS_Y+34,KB_ORANGE);
	Gui_DrawLine(KB_MAYBEBOX_POS_X-2+3*19,KB_MAYBEBOX_POS_Y,KB_MAYBEBOX_POS_X-2+3*19, KB_MAYBEBOX_POS_Y+34,KB_ORANGE);
	Gui_DrawLine(KB_MAYBEBOX_POS_X-2+4*19,KB_MAYBEBOX_POS_Y,KB_MAYBEBOX_POS_X-2+4*19, KB_MAYBEBOX_POS_Y+34,KB_ORANGE);
	Gui_DrawLine(KB_MAYBEBOX_POS_X-2+5*19,KB_MAYBEBOX_POS_Y,KB_MAYBEBOX_POS_X-2+5*19, KB_MAYBEBOX_POS_Y+34,KB_ORANGE);
	Gui_DrawLine(KB_MAYBEBOX_POS_X-2+6*19,KB_MAYBEBOX_POS_Y,KB_MAYBEBOX_POS_X-2+6*19, KB_MAYBEBOX_POS_Y+34,KB_ORANGE);
	Gui_DrawLine(KB_MAYBEBOX_POS_X-2+7*19,KB_MAYBEBOX_POS_Y,KB_MAYBEBOX_POS_X-2+7*19, KB_MAYBEBOX_POS_Y+34,KB_ORANGE);
	Gui_DrawLine(KB_MAYBEBOX_POS_X-2+8*19,KB_MAYBEBOX_POS_Y,KB_MAYBEBOX_POS_X-2+8*19, KB_MAYBEBOX_POS_Y+34,KB_ORANGE);
	Gui_DrawLine(KB_MAYBEBOX_POS_X-2+9*19,KB_MAYBEBOX_POS_Y,KB_MAYBEBOX_POS_X-2+9*19, KB_MAYBEBOX_POS_Y+34,KB_ORANGE);
}
static void KB_DrawPress(u8 xx,u8 yy){//选字提示
	u8 x,y,x0,y0,x1,y1;
	u16 Color;
	x0=xx;
	y0=yy;
	x1=xx+18;
	y1=yy+17;	
	for(y=y0;y<y1;y++)
		for(x=x0;x<x1;x++)
		{
			Color=Gui_ReadPixel16Bit(x,y);
			if(Color==KB_WHITE)
				Gui_WritePixel(x,y,KB_ORANGE);
		}	
}
//-----------------------------------------对象成员函数定义---------------------------------------------	
//----------文本框对象成员函数定义--------
void KB_TextBox_Init(u8 *p,s32 bufsizemsg){
	u16 bufsize;
	u32 i=0;
	bufsize=bufsizemsg&0xFFFF;
	gpKbpVars->KB_TextBox.elem=p;
	gpKbpVars->KB_TextBox.OUTBUF_SIZE=bufsize;
	while(*p++)
		i++;						   			
	gpKbpVars->KB_TextBox.elem[i]='_';
	gpKbpVars->KB_TextBox.elem[i+1]=0; 
	gpKbpVars->KB_TextBox.tailnextnext=i+1;
	gpKbpVars->KB_TextBox.ShowBegin=0;
	gpKbpVars->KB_TextBox.TotalPage=(i/KB_TEXTBOXCROSS_NUM)+1;
	gpKbpVars->KB_TextBox.CurPage=1;		
	gpKbpVars->KB_TextBox.TextBoxRegion.x=KB_TEXTBOX_POS_X;
	gpKbpVars->KB_TextBox.TextBoxRegion.y=KB_TEXTBOX_POS_Y;
	gpKbpVars->KB_TextBox.TextBoxRegion.w=KB_TEXTBOX_W;
	gpKbpVars->KB_TextBox.TextBoxRegion.h=KB_TEXTBOX_H;
	gpKbpVars->KB_TextBox.TextBoxRegion.Space=(KB_TEXTBOX_CHAR_SPACE<<4)|(KB_TEXTBOX_ROW_SPACE);
	gpKbpVars->KB_TextBox.TextBoxRegion.Color=KB_TEXTBOXCHAR_COLOUR;			 
	gpKbpVars->KB_TextBox.ScrollbarRegion.x=KB_CROSSBAR_INI_X;
	gpKbpVars->KB_TextBox.ScrollbarRegion.y=KB_CROSSBAR_INI_Y;
	gpKbpVars->KB_TextBox.ScrollbarRegion.w=KB_CROSSBAR_INI_W;
	gpKbpVars->KB_TextBox.ScrollbarRegion.h=KB_CROSSBAR_INI_H;
	gpKbpVars->KB_TextBox.ScrollbarRegion.Color=KB_CROSSBAR_COLOUR;
	gpKbpVars->KB_TextBox.ShowFromPos=KB_TextBox_ShowFromPos; 
	gpKbpVars->KB_TextBox.RefuCrossBar=KB_TextBox_RefuCrossBar;
	gpKbpVars->KB_TextBox.InsTail=KB_TextBox_InsTail;
	gpKbpVars->KB_TextBox.InsTailChain=KB_TextBox_InsTailChain;
	gpKbpVars->KB_TextBox.ClrTail=KB_TextBox_ClrTail;
	gpKbpVars->KB_TextBox.ClearAll=KB_TextBox_ClearAll;
	gpKbpVars->KB_TextBox.UpPage=KB_TextBox_UpPage;
	gpKbpVars->KB_TextBox.DownPage=KB_TextBox_DownPage;
	gpKbpVars->KB_TextBox.ShowPrepa=KB_TextBox_ShowPrepa;
	gpKbpVars->KB_TextBox.EndEdit=KB_TextBox_EndEdit;
	gpKbpVars->KB_TextBox.ShowFromPos();
	gpKbpVars->KB_TextBox.RefuCrossBar();		
}
void KB_TextBox_ShowFromPos(void){
	gpKbpVars->KB_TextBox.TextBoxRegion.Color=KB_TEXTBOXBG_COLOUR;
	Gui_FillBlock(&gpKbpVars->KB_TextBox.TextBoxRegion);		
	gpKbpVars->KB_TextBox.TextBoxRegion.Color=KB_TEXTBOXCHAR_COLOUR;
	Gui_DrawFont(GBK16_FONT,&(gpKbpVars->KB_TextBox.elem[gpKbpVars->KB_TextBox.ShowBegin]),&gpKbpVars->KB_TextBox.TextBoxRegion);
}
void KB_TextBox_RefuCrossBar(void){
	gpKbpVars->KB_TextBox.ScrollbarRegion.Color=KB_WHITE;
	Gui_FillBlock(&gpKbpVars->KB_TextBox.ScrollbarRegion);		
	gpKbpVars->KB_TextBox.ScrollbarRegion.Color=KB_CROSSBAR_COLOUR;			
	gpKbpVars->KB_TextBox.ScrollbarRegion.h = KB_CROSSBAR_INI_H/gpKbpVars->KB_TextBox.TotalPage;
	gpKbpVars->KB_TextBox.ScrollbarRegion.y=KB_CROSSBAR_INI_Y+KB_CROSSBAR_INI_H/gpKbpVars->KB_TextBox.TotalPage*(gpKbpVars->KB_TextBox.CurPage-1);	
	Gui_FillBlock(&gpKbpVars->KB_TextBox.ScrollbarRegion);		
}
bool KB_TextBox_InsTail(u8 inchar){
	if(gpKbpVars->KB_TextBox.tailnextnext==gpKbpVars->KB_TextBox.OUTBUF_SIZE-1){
		Debug("TextBuf is Full!!!\n\r");
		return(FALSE);
	}
 	gpKbpVars->KB_TextBox.elem[gpKbpVars->KB_TextBox.tailnextnext-1]=inchar;
	gpKbpVars->KB_TextBox.elem[gpKbpVars->KB_TextBox.tailnextnext]='_';
	gpKbpVars->KB_TextBox.elem[gpKbpVars->KB_TextBox.tailnextnext+1]=0;
	gpKbpVars->KB_TextBox.tailnextnext++;
	gpKbpVars->KB_TextBox.ShowFromPos();
	return(TRUE);
}
bool KB_TextBox_InsTailChain(u8 *str){
	while(*str!=0){
		if(gpKbpVars->KB_TextBox.tailnextnext==gpKbpVars->KB_TextBox.OUTBUF_SIZE-1){
			gpKbpVars->KB_TextBox.elem[gpKbpVars->KB_TextBox.OUTBUF_SIZE-1]=0;
			gpKbpVars->KB_TextBox.ShowFromPos();
			Debug("TextBuf is Full\n\r");
			return(FALSE);
		}
		gpKbpVars->KB_TextBox.elem[gpKbpVars->KB_TextBox.tailnextnext-1]=*str;
		gpKbpVars->KB_TextBox.tailnextnext++;
		str++;		
	}
	gpKbpVars->KB_TextBox.elem[gpKbpVars->KB_TextBox.tailnextnext-1]='_';
	gpKbpVars->KB_TextBox.elem[gpKbpVars->KB_TextBox.tailnextnext]=0;
	gpKbpVars->KB_TextBox.ShowFromPos();
	return(TRUE);
}
void KB_TextBox_ClrTail(void){
	if(gpKbpVars->KB_TextBox.tailnextnext-1==0)
		return;
	if(gpKbpVars->KB_TextBox.tailnextnext-2==0){
		gpKbpVars->KB_TextBox.elem[gpKbpVars->KB_TextBox.tailnextnext-2]='_';
		gpKbpVars->KB_TextBox.elem[gpKbpVars->KB_TextBox.tailnextnext-1]=0;
		gpKbpVars->KB_TextBox.tailnextnext--;
		gpKbpVars->KB_TextBox.ShowFromPos();
		return;
	}
	if(gpKbpVars->KB_TextBox.elem[gpKbpVars->KB_TextBox.tailnextnext-2]<126){
		gpKbpVars->KB_TextBox.elem[gpKbpVars->KB_TextBox.tailnextnext-2]='_';
		gpKbpVars->KB_TextBox.elem[gpKbpVars->KB_TextBox.tailnextnext-1]=0;
		gpKbpVars->KB_TextBox.tailnextnext--;

	}else{
		gpKbpVars->KB_TextBox.elem[gpKbpVars->KB_TextBox.tailnextnext-3]='_';
		gpKbpVars->KB_TextBox.elem[gpKbpVars->KB_TextBox.tailnextnext-2]=0;
		gpKbpVars->KB_TextBox.tailnextnext--;
		gpKbpVars->KB_TextBox.tailnextnext--;
	}
	gpKbpVars->KB_TextBox.ShowFromPos();
}
void KB_TextBox_ClearAll(void)
{
	gpKbpVars->KB_TextBox.elem[0]='_';
	gpKbpVars->KB_TextBox.elem[1]=0; 
	gpKbpVars->KB_TextBox.tailnextnext=1;
	gpKbpVars->KB_TextBox.ShowBegin=0;
	gpKbpVars->KB_TextBox.TotalPage=1;
	gpKbpVars->KB_TextBox.CurPage=1;	
	gpKbpVars->KB_TextBox.ShowFromPos();
	gpKbpVars->KB_TextBox.RefuCrossBar();	
}
void KB_TextBox_UpPage(void){
	if(gpKbpVars->KB_TextBox.ShowBegin<KB_TEXTBOXCROSS_NUM)
		return;
	gpKbpVars->KB_TextBox.ShowBegin-=KB_TEXTBOXCROSS_NUM;
	gpKbpVars->KB_TextBox.ShowFromPos();
	gpKbpVars->KB_TextBox.CurPage-=1;
	gpKbpVars->KB_TextBox.RefuCrossBar();
}
void KB_TextBox_DownPage(void){
	if(gpKbpVars->KB_TextBox.ShowBegin>=gpKbpVars->KB_TextBox.tailnextnext-1-KB_TEXTBOXCROSS_NUM)		
		return;
	gpKbpVars->KB_TextBox.ShowBegin+=KB_TEXTBOXCROSS_NUM;
	gpKbpVars->KB_TextBox.ShowFromPos();
	gpKbpVars->KB_TextBox.CurPage+=1;
	if( gpKbpVars->KB_TextBox.CurPage > gpKbpVars->KB_TextBox.TotalPage )		
		gpKbpVars->KB_TextBox.TotalPage = gpKbpVars->KB_TextBox.CurPage;
	gpKbpVars->KB_TextBox.RefuCrossBar();
}
void KB_TextBox_ShowPrepa(u8 *str){
	u32 index;
	index=gpKbpVars->KB_TextBox.tailnextnext;	
	while(*str!=0){
		if(index==gpKbpVars->KB_TextBox.OUTBUF_SIZE-1){
			Debug("TextBuf will be Full\n\r");
			return;
		}
		gpKbpVars->KB_TextBox.elem[index-1]=*str;
		index++;
		str++;		
	}
	gpKbpVars->KB_TextBox.elem[index-1]='_';
	gpKbpVars->KB_TextBox.elem[index]=0;
	gpKbpVars->KB_TextBox.ShowFromPos();
}
void KB_TextBox_EndEdit(void){
	gpKbpVars->KB_TextBox.elem[gpKbpVars->KB_TextBox.tailnextnext-1]=0;
	Q_GotoPage(SubPageReturn,"",sizeof(gpKbpVars->KB_TextBox.elem),gpKbpVars->KB_TextBox.elem);
}
//--------------临时缓存区对象成员函数定义-----------
void KB_TempBuf_Init(void){
	gpKbpVars->KB_TempBuf.elem[0]=0;
	gpKbpVars->KB_TempBuf.tailnext=0;
	gpKbpVars->KB_TempBuf.Decode=KB_TempBuf_Decode;
	gpKbpVars->KB_TempBuf.Clr=KB_TempBuf_Clr;
	gpKbpVars->KB_TempBuf.InsTail=KB_TempBuf_InsTail;
	gpKbpVars->KB_TempBuf.UploadText=KB_TempBuf_UploadText;
	gpKbpVars->KB_TempBuf.UploadPinYin=KB_TempBuf_UploadPinYin;
	gpKbpVars->KB_TempBuf.PleaseUploadText=KB_TempBuf_PleaseUploadText;
	gpKbpVars->KB_TempBuf.PleaseUploadPinYin=KB_TempBuf_PleaseUploadPinYin;		
}
void KB_TempBuf_Decode(void){
	u8 compare,index=1,realnum=0,repeat=0;
	compare=gpKbpVars->KB_TempBuf.elem[0];
	if(gpKbpVars->KB_TempBuf.tailnext==0)
		return;
	while(index<=gpKbpVars->KB_TempBuf.tailnext){			
		if(gpKbpVars->KB_TempBuf.elem[index]!=compare){
			gpKbpVars->KB_TempBuf.decodebuf[realnum]=gpKbpVars->KB_TempBuf.elem[index-1]+repeat;
			realnum++;
			compare=gpKbpVars->KB_TempBuf.elem[index];
			repeat=0;
		}else{
			repeat++;
			if( compare=='p' || compare=='w' )
				repeat=repeat%4;
			else
				repeat=repeat%3;
		}
		index++;		
	}
	gpKbpVars->KB_TempBuf.decodebuf[realnum]=0;		
}
void KB_TempBuf_Clr(void){
	 gpKbpVars->KB_TempBuf.elem[0]=0;
	 gpKbpVars->KB_TempBuf.tailnext=0;
}
bool KB_TempBuf_InsTail(u8 inchar){
	u8 mainmode;
	if(gpKbpVars->KB_TempBuf.tailnext==KB_TEMPBUF_MAX){
		Debug("KB_TempBuf is Full\n\r");
		return(FALSE);
	}
 	gpKbpVars->KB_TempBuf.elem[gpKbpVars->KB_TempBuf.tailnext]=inchar;
	gpKbpVars->KB_TempBuf.elem[gpKbpVars->KB_TempBuf.tailnext+1]=0;
	gpKbpVars->KB_TempBuf.tailnext++;
	gpKbpVars->KB_TempBuf.Decode();
	mainmode=gpKbpVars->KB_ModeBox.GetMode() & 0x0f;
	switch(mainmode){
		case KB_EN_MODE:
		   gpKbpVars->KB_TextBox.ShowPrepa(gpKbpVars->KB_TempBuf.decodebuf);
		   break;
		case KB_PINYIN_MODE:
		   gpKbpVars->KB_PinYinBox.ShowPrepa(gpKbpVars->KB_TempBuf.decodebuf);	
		   break;
	}					 
	//打开定时器
	Q_TimSet(Q_TIM1,KB_DKTIME,100,(bool)1);
	return(TRUE);
}
void KB_TempBuf_PleaseUploadText(KB_TextBoxDef *ptextbox){
	KB_TempBuf_UploadText(ptextbox);
}	
void KB_TempBuf_PleaseUploadPinYin(KB_PinYinBoxDef *ppinyinbox){
	KB_TempBuf_UploadPinYin(ppinyinbox);
}
void KB_TempBuf_UploadText(KB_TextBoxDef *ptextbox){	
	ptextbox->InsTailChain(gpKbpVars->KB_TempBuf.decodebuf);
	gpKbpVars->KB_TempBuf.Clr();  		 
	//关闭定时器
	Q_TimSet(Q_TIM1,0,0,(bool)1);	
}	
void KB_TempBuf_UploadPinYin(KB_PinYinBoxDef *ppinyinbox){ 
	ppinyinbox->InsTailChain(gpKbpVars->KB_TempBuf.decodebuf);
	gpKbpVars->KB_TempBuf.Clr();	 		 
	//关闭定时器
	Q_TimSet(Q_TIM1,0,0,(bool)1);	
}
//----------------拼音框对象成员函数定义----------------
void KB_PinYinBox_ClrPinYinShow(void){	
	gpKbpVars->KB_PinYinBox.PinYinRegion.Color=KB_PINYINBOXCHARBG_COLOUR;
	Gui_FillBlock(&gpKbpVars->KB_PinYinBox.PinYinRegion);	
	gpKbpVars->KB_PinYinBox.PinYinRegion.Color=KB_PINYINBOXCHAR_COLOUR;
}
void KB_PinYinBox_ClrMayBeShow(void){
	gpKbpVars->KB_PinYinBox.MayBeRegion.Color=KB_MAYBEBOXCHARBG_COLOUR;
	Gui_FillBlock(&gpKbpVars->KB_PinYinBox.MayBeRegion);	
	gpKbpVars->KB_PinYinBox.MayBeRegion.Color=KB_MAYBEBOXCHAR_COLOUR;
}
void KB_PinYinBox_ShowPinYinBox(void){
	gpKbpVars->KB_PinYinBox.ClrPinYinShow();
	Gui_DrawFont(GBK16_FONT,gpKbpVars->KB_PinYinBox.elem,&gpKbpVars->KB_PinYinBox.PinYinRegion);
}
void KB_PinYinBox_ShowMayBeFromPos(void){
	gpKbpVars->KB_PinYinBox.ClrMayBeShow();
	if(gpKbpVars->KB_PinYinBox.pmbstr!=NULL){
		Gui_DrawFont(GBK16_FONT,gpKbpVars->KB_PinYinBox.pmbstr+gpKbpVars->KB_PinYinBox.curbase,&gpKbpVars->KB_PinYinBox.MayBeRegion);
		KB_DrawBlock();
	}
}
void KB_PinYinBox_ClrMayBe(void){
	gpKbpVars->KB_PinYinBox.pmbstr=NULL;
	gpKbpVars->KB_PinYinBox.curbase=0;
	gpKbpVars->KB_PinYinBox.strnum=0;
	gpKbpVars->KB_PinYinBox.ClrMayBeShow();	
}
void KB_PinYinBox_LinkMayBe(const u8* str){
	gpKbpVars->KB_PinYinBox.ClrMayBeShow();
	gpKbpVars->KB_PinYinBox.pmbstr=str;
	gpKbpVars->KB_PinYinBox.curbase=0;
	gpKbpVars->KB_PinYinBox.strnum=0;
	while(*str++)
		 gpKbpVars->KB_PinYinBox.strnum++;
	gpKbpVars->KB_PinYinBox.ShowMayBeFromPos();
}
void KB_PinYinBox_LinkMayBeHalf(const u8* str){	
	gpKbpVars->KB_PinYinBox.ClrMayBeShow();
	gpKbpVars->KB_PinYinBox.pmbstr=str;
	gpKbpVars->KB_PinYinBox.curbase=0;
	gpKbpVars->KB_PinYinBox.strnum=0;
	while(*str++)
		 gpKbpVars->KB_PinYinBox.strnum++;
	gpKbpVars->KB_PinYinBox.MayBeRegion.Space=((KB_MAYBEBOX_CHAR_SPACE+8)<<4)|(KB_MAYBEBOX_ROW_SPACE);
	gpKbpVars->KB_PinYinBox.ShowMayBeFromPos();
	gpKbpVars->KB_PinYinBox.MayBeRegion.Space=(KB_MAYBEBOX_CHAR_SPACE<<4)|(KB_MAYBEBOX_ROW_SPACE);
}
void KB_PinYinBox_Init(void){
	gpKbpVars->KB_PinYinBox.elem[0]=0;
	gpKbpVars->KB_PinYinBox.tailnext=0;					
	gpKbpVars->KB_PinYinBox.pmbstr=0;	 		
	gpKbpVars->KB_PinYinBox.curbase=0;	 				
	gpKbpVars->KB_PinYinBox.strnum=0; 					
	gpKbpVars->KB_PinYinBox.PinYinRegion.x=KB_PINYINBOX_POS_X;
	gpKbpVars->KB_PinYinBox.PinYinRegion.y=KB_PINYINBOX_POS_Y;
	gpKbpVars->KB_PinYinBox.PinYinRegion.w=KB_PINYINBOX_W;
	gpKbpVars->KB_PinYinBox.PinYinRegion.h=KB_PINYINBOX_H;
	gpKbpVars->KB_PinYinBox.PinYinRegion.Color=KB_PINYINBOXCHAR_COLOUR;
	gpKbpVars->KB_PinYinBox.PinYinRegion.Space=(KB_PINYINBOX_CHAR_SPACE<<4)|(KB_PINYINBOX_ROW_SPACE);	
	gpKbpVars->KB_PinYinBox.MayBeRegion.x=KB_MAYBEBOX_POS_X;
	gpKbpVars->KB_PinYinBox.MayBeRegion.y=KB_MAYBEBOX_POS_Y;
	gpKbpVars->KB_PinYinBox.MayBeRegion.w=KB_MAYBEBOX_W;
	gpKbpVars->KB_PinYinBox.MayBeRegion.h=KB_MAYBEBOX_H;
	gpKbpVars->KB_PinYinBox.MayBeRegion.Color=KB_MAYBEBOXCHAR_COLOUR;
	gpKbpVars->KB_PinYinBox.MayBeRegion.Space=(KB_MAYBEBOX_CHAR_SPACE<<4)|(KB_MAYBEBOX_ROW_SPACE);
	gpKbpVars->KB_PinYinBox.ClrPinYinShow=KB_PinYinBox_ClrPinYinShow;
	gpKbpVars->KB_PinYinBox.ClrMayBeShow=KB_PinYinBox_ClrMayBeShow;	
	gpKbpVars->KB_PinYinBox.ShowPinYinBox=KB_PinYinBox_ShowPinYinBox;	
	gpKbpVars->KB_PinYinBox.ShowMayBeFromPos=KB_PinYinBox_ShowMayBeFromPos;
	gpKbpVars->KB_PinYinBox.ClrMayBe=KB_PinYinBox_ClrMayBe;		
	gpKbpVars->KB_PinYinBox.LinkMayBe=KB_PinYinBox_LinkMayBe;
	gpKbpVars->KB_PinYinBox.LinkMayBeHalf=KB_PinYinBox_LinkMayBeHalf;
	gpKbpVars->KB_PinYinBox.NextMayBePage=KB_PinYinBox_NextMayBePage;
	gpKbpVars->KB_PinYinBox.NextMayBePageHalf=KB_PinYinBox_NextMayBePageHalf;	
	gpKbpVars->KB_PinYinBox.PreMayBePage=KB_PinYinBox_PreMayBePage;
	gpKbpVars->KB_PinYinBox.PreMayBePageHalf=KB_PinYinBox_PreMayBePageHalf;			
	gpKbpVars->KB_PinYinBox.Clr=KB_PinYinBox_Clr; 		    
	gpKbpVars->KB_PinYinBox.InsTailChain=KB_PinYinBox_InsTailChain; 
	gpKbpVars->KB_PinYinBox.ClrTail=KB_PinYinBox_ClrTail;	
	gpKbpVars->KB_PinYinBox.IfEditPinYin=KB_PinYinBox_IfEditPinYin;		
	gpKbpVars->KB_PinYinBox.Upload=KB_PinYinBox_Upload;
	gpKbpVars->KB_PinYinBox.UploadHalf=KB_PinYinBox_UploadHalf;
	gpKbpVars->KB_PinYinBox.PleaseUpload=KB_PinYinBox_PleaseUpload;
	gpKbpVars->KB_PinYinBox.PleaseUploadHalf=KB_PinYinBox_PleaseUploadHalf;
	gpKbpVars->KB_PinYinBox.ShowPrepa=KB_PinYinBox_ShowPrepa;
}
void KB_PinYinBox_Clr(void){
	gpKbpVars->KB_PinYinBox.ClrMayBe();
	gpKbpVars->KB_PinYinBox.elem[0]=0;
	gpKbpVars->KB_PinYinBox.tailnext=0;
	gpKbpVars->KB_PinYinBox.ClrPinYinShow();
}
bool KB_PinYinBox_InsTailChain(u8 *str){
	while(*str!=0){
		if(gpKbpVars->KB_PinYinBox.tailnext==KB_PINYINBUF_MAX){
			gpKbpVars->KB_PinYinBox.elem[KB_PINYINBUF_MAX]=0;
			Debug("KB_PinYinBox is Full!!!\n\r");
			return(FALSE);
		}
		gpKbpVars->KB_PinYinBox.elem[gpKbpVars->KB_PinYinBox.tailnext]=*str;		
		gpKbpVars->KB_PinYinBox.tailnext++;
		gpKbpVars->KB_PinYinBox.elem[gpKbpVars->KB_PinYinBox.tailnext]=0;	
		gpKbpVars->KB_PinYinBox.ShowPinYinBox();
		str++;		
	}
	gpKbpVars->KB_PinYinBox.LinkMayBe(py_to_str(gpKbpVars->KB_PinYinBox.elem));	
	return(TRUE);
}
void KB_PinYinBox_ClrTail(void){
   if(gpKbpVars->KB_PinYinBox.tailnext==0){
   		gpKbpVars->KB_PinYinBox.ClrMayBe();
		return;
	}
	gpKbpVars->KB_PinYinBox.elem[gpKbpVars->KB_PinYinBox.tailnext-1]=0;
	gpKbpVars->KB_PinYinBox.tailnext--;
	gpKbpVars->KB_PinYinBox.ShowPinYinBox();
	gpKbpVars->KB_PinYinBox.LinkMayBe(py_to_str(gpKbpVars->KB_PinYinBox.elem));	
}
void KB_PinYinBox_NextMayBePage(void){
	 if(gpKbpVars->KB_PinYinBox.pmbstr==NULL)
		return;
	gpKbpVars->KB_PinYinBox.ClrMayBeShow();
	if(gpKbpVars->KB_PinYinBox.strnum>gpKbpVars->KB_PinYinBox.curbase+40)
		gpKbpVars->KB_PinYinBox.curbase+=40;	
	gpKbpVars->KB_PinYinBox.ShowMayBeFromPos();
}
void KB_PinYinBox_NextMayBePageHalf(void){
	 if(gpKbpVars->KB_PinYinBox.pmbstr==NULL)
		return;
	gpKbpVars->KB_PinYinBox.ClrMayBeShow();
	if(gpKbpVars->KB_PinYinBox.strnum>gpKbpVars->KB_PinYinBox.curbase+20)
		gpKbpVars->KB_PinYinBox.curbase+=20;
	gpKbpVars->KB_PinYinBox.MayBeRegion.Space=((KB_MAYBEBOX_CHAR_SPACE+8)<<4)|(KB_MAYBEBOX_ROW_SPACE);	
	gpKbpVars->KB_PinYinBox.ShowMayBeFromPos();
	gpKbpVars->KB_PinYinBox.MayBeRegion.Space=(KB_MAYBEBOX_CHAR_SPACE<<4)|(KB_MAYBEBOX_ROW_SPACE);
}
void KB_PinYinBox_PreMayBePage(void){
	 if(gpKbpVars->KB_PinYinBox.pmbstr==NULL)
		return;
	gpKbpVars->KB_PinYinBox.ClrMayBeShow();
	if(gpKbpVars->KB_PinYinBox.curbase>=40)
		gpKbpVars->KB_PinYinBox.curbase-=40;			
	gpKbpVars->KB_PinYinBox.ShowMayBeFromPos();	
}
void KB_PinYinBox_PreMayBePageHalf(void){
	 if(gpKbpVars->KB_PinYinBox.pmbstr==NULL)
		return;
	gpKbpVars->KB_PinYinBox.ClrMayBeShow();
	if(gpKbpVars->KB_PinYinBox.curbase>=20)
		gpKbpVars->KB_PinYinBox.curbase-=20;
	gpKbpVars->KB_PinYinBox.MayBeRegion.Space=((KB_MAYBEBOX_CHAR_SPACE+8)<<4)|(KB_MAYBEBOX_ROW_SPACE);	
	gpKbpVars->KB_PinYinBox.ShowMayBeFromPos();
	gpKbpVars->KB_PinYinBox.MayBeRegion.Space=(KB_MAYBEBOX_CHAR_SPACE<<4)|(KB_MAYBEBOX_ROW_SPACE);
}
bool KB_PinYinBox_IfEditPinYin(void){
	if(gpKbpVars->KB_PinYinBox.pmbstr==NULL && gpKbpVars->KB_PinYinBox.elem[0]!='i' && gpKbpVars->KB_PinYinBox.elem[0]!='u' && gpKbpVars->KB_PinYinBox.elem[0]!='v')
		return 	FALSE;
	else
		return TRUE;
}
void KB_PinYinBox_PleaseUpload(KB_TextBoxDef *ptextbox,u8 order){
	 KB_PinYinBox_Upload(ptextbox,order);
}
void KB_PinYinBox_PleaseUploadHalf(KB_TextBoxDef *ptextbox,u8 order){
	 KB_PinYinBox_UploadHalf(ptextbox,order);
}	
void KB_PinYinBox_Upload(KB_TextBoxDef *ptextbox,u8 order){
	u8 *p;
	if(gpKbpVars->KB_PinYinBox.pmbstr==NULL)
		return;
	if(gpKbpVars->KB_PinYinBox.curbase+order*2>=gpKbpVars->KB_PinYinBox.strnum)
		return;	
	p=(u8 *)gpKbpVars->KB_PinYinBox.pmbstr+gpKbpVars->KB_PinYinBox.curbase+order*2;	
	ptextbox->InsTail(*p++);
	ptextbox->InsTail(*p);
	gpKbpVars->KB_PinYinBox.Clr();		
}
void KB_PinYinBox_UploadHalf(KB_TextBoxDef *ptextbox,u8 order){
	u8 *p;
	if(gpKbpVars->KB_PinYinBox.pmbstr==NULL)
		return;
	if(gpKbpVars->KB_PinYinBox.curbase+order>=gpKbpVars->KB_PinYinBox.strnum)
		return;	
	p=(u8 *)gpKbpVars->KB_PinYinBox.pmbstr+gpKbpVars->KB_PinYinBox.curbase+order;
	ptextbox->InsTail(*p);
	gpKbpVars->KB_PinYinBox.Clr();		
}

void KB_PinYinBox_ShowPrepa(u8 *str){
	u8 index;
	index=gpKbpVars->KB_PinYinBox.tailnext;
	while(*str!=0){
		if(index==KB_PINYINBUF_MAX){
			Debug("KB_PinYinBox will be Full!!!\n\r");
			return;
		}
		gpKbpVars->KB_PinYinBox.elem[index]=*str;		
		index++;
		gpKbpVars->KB_PinYinBox.elem[index]=0;	
		gpKbpVars->KB_PinYinBox.ShowPinYinBox();
		str++;		
	}
}		
//----------------模式框对象成员函数定义----------------
void KB_ModeBox_ShowMode(void){
	u8 mainmode;
	mainmode=(gpKbpVars->KB_ModeBox.ModeFlag)&0xf;
	switch(mainmode){
		case  KB_NUM_MODE:{
			Q_ChangeImgTchImg(KB_PRESS_15,'T');
			Q_PresentTch(KB_PRESS_15,Tch_Release);
			Q_ChangeImgTchImg(KB_PRESS_20,'T');
			Q_PresentTch(KB_PRESS_20,Tch_Release);
			Q_ChangeImgTchImg(KB_PRESS_10,'S');
			Q_PresentTch(KB_PRESS_10,Tch_Release);
			break;
		}
		case  KB_EN_MODE:{			
			Q_ChangeImgTchImg(KB_PRESS_10,'T');
			Q_PresentTch(KB_PRESS_10,Tch_Release);
			Q_ChangeImgTchImg(KB_PRESS_20,'T');
			Q_PresentTch(KB_PRESS_20,Tch_Release);
			if( (gpKbpVars->KB_ModeBox.ModeFlag) & KB_CAP_MODE){
				Q_ChangeImgTchImg(KB_PRESS_15,'U');
				Q_PresentTch(KB_PRESS_15,Tch_Release);

			}else{
				Q_ChangeImgTchImg(KB_PRESS_15,'S');
				Q_PresentTch(KB_PRESS_15,Tch_Release);
			}
			break;
		}
		case  KB_PINYIN_MODE:{		
			Q_ChangeImgTchImg(KB_PRESS_10,'T');
			Q_PresentTch(KB_PRESS_10,Tch_Release);
			Q_ChangeImgTchImg(KB_PRESS_15,'T');
			Q_PresentTch(KB_PRESS_15,Tch_Release);
			Q_ChangeImgTchImg(KB_PRESS_20,'S');
			Q_PresentTch(KB_PRESS_20,Tch_Release);
			break;
		}
	}
}
void KB_ModeBox_Init(s32 modemsg){
	if(modemsg&KBMSK_NUM_DEF){
		gpKbpVars->KB_ModeBox.ModeFlag=KB_NUM_MODE;
		Q_ChangeImgTchImg(KB_PRESS_10,'S');
	}
	else if(modemsg&KBMSK_LETTER_DEF){
	  	gpKbpVars->KB_ModeBox.ModeFlag=KB_EN_MODE;
		Q_ChangeImgTchImg(KB_PRESS_15,'S');
	}
	else if(modemsg&KBMSK_PINYIN_DEF){
		gpKbpVars->KB_ModeBox.ModeFlag=KB_PINYIN_MODE;
		Q_ChangeImgTchImg(KB_PRESS_20,'S');
	}
	else{
		gpKbpVars->KB_ModeBox.ModeFlag=KB_PINYIN_MODE;
		Q_ChangeImgTchImg(KB_PRESS_20,'S');
	}

	if(modemsg&KBMSK_PINYIN_DIS)
		gpKbpVars->KB_ModeBox.IfOffPinYin=TRUE;
	else
		gpKbpVars->KB_ModeBox.IfOffPinYin=FALSE;
	
	if(modemsg&KBMSK_LETTER_DIS)
		gpKbpVars->KB_ModeBox.IfOffEn=TRUE;
	else
		gpKbpVars->KB_ModeBox.IfOffEn=FALSE;

	if(modemsg&KBMSK_NUM_DIS)
		gpKbpVars->KB_ModeBox.IfOffNum=TRUE;
	else
		gpKbpVars->KB_ModeBox.IfOffNum=FALSE;
	
	if(modemsg&KBMSK_NAME_MODE)
		gpKbpVars->KB_ModeBox.IfOffPunc=TRUE;
	else
		gpKbpVars->KB_ModeBox.IfOffPunc=FALSE;
									     
	gpKbpVars->KB_ModeBox.ShowMode=KB_ModeBox_ShowMode;		 	   	 
	gpKbpVars->KB_ModeBox.PleaseChangeMode=KB_ModeBox_PleaseChangeMode; 
	gpKbpVars->KB_ModeBox.GetMode=KB_ModeBox_GetMode;				 
	gpKbpVars->KB_ModeBox.ChangeModeAct=KB_ModeBox_ChangeModeAct;	
	gpKbpVars->KB_ModeBox.Capital=KB_ModeBox_Capital;
	gpKbpVars->KB_ModeBox.UnCapital=KB_ModeBox_UnCapital;
	gpKbpVars->KB_ModeBox.IfCapital=KB_ModeBox_IfCapital;
	gpKbpVars->KB_ModeBox.ShowMode();
}
void KB_ModeBox_PleaseChangeMode(KB_PinYinBoxDef *ppinyinbox,u8 newmode){
	if( gpKbpVars->KB_ModeBox.IfOffPinYin && newmode==KB_PINYIN_MODE )
		return;	
	if( gpKbpVars->KB_ModeBox.IfOffNum && newmode==KB_NUM_MODE )
		return;
	if( gpKbpVars->KB_ModeBox.IfOffEn && newmode==KB_EN_MODE )
		return;	
	gpKbpVars->KB_ModeBox.ModeFlag=newmode;
	gpKbpVars->KB_ModeBox.ChangeModeAct(ppinyinbox,newmode);
}
u8 	KB_ModeBox_GetMode(void){
	return gpKbpVars->KB_ModeBox.ModeFlag;
}
void KB_ModeBox_ChangeModeAct(KB_PinYinBoxDef *ppinyinbox,u8 newmode){
	ppinyinbox->Clr();
	gpKbpVars->KB_ModeBox.ShowMode();
}
void KB_ModeBox_Capital(void){
	(gpKbpVars->KB_ModeBox.ModeFlag) |= KB_CAP_MODE;
	gpKbpVars->KB_ModeBox.ShowMode();
}
void KB_ModeBox_UnCapital(void){
	(gpKbpVars->KB_ModeBox.ModeFlag) &= (~KB_CAP_MODE);
	gpKbpVars->KB_ModeBox.ShowMode();
}
bool KB_ModeBox_IfCapital(void){
	if( (gpKbpVars->KB_ModeBox.ModeFlag) & KB_CAP_MODE)
		return TRUE;
	else
		return FALSE;
}


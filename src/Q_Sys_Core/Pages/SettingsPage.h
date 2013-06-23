#ifndef QSYS_SETTINGS_PAGE_H   //这两句是为了让编译器不重复调用此文件，每个h文件最好都有这么一句，但是不要用一样的宏哦！！！
#define QSYS_SETTINGS_PAGE_H

//每个页面只需要申明一个全局变量和两个函数
//一般情况下不要申明别的东西，除非你的页面是一个子页面，会被别的页面调用
//否则你就失去了你的页面的独立性，这是很忌讳的
extern const PAGE_ATTRIBUTE SettingsPage;

typedef enum {
	OPRT_Null=0,//被废弃
	OPRT_YesOrNo,//TRUE or FALSE
	OPRT_Num,//数字
	OPRT_Str,//字符串数组
}OP_RET_TYPE;

//建立options,Id必须从1开始
void SP_AddOptionsHeader(void *pBuf,u16 BufSize,u8 *Name,u8 OptsID);
void SP_AddYesOrNoOption(void *pBuf,u8 Id,bool DefValue,u8 *Name,u8 *Note);
void SP_AddNumOption(void *pBuf,u8 Id,s32 DefValue,u8 *Name,u8 *Note);
void SP_AddNumListOption(void *pBuf,u8 Id,s32 DefValue,s32 Min,s32 Max,s32 Step,u8 *Name,u8 *Note);
void SP_AddNumEnumOption(void *pBuf,u8 Id,bool IsDef,s32 Value,u8 *Name,u8 *Note);
void SP_AddStrEnumOption(void *pBuf,u8 Id,bool IsDef,u8 *pStr,u8 *Name,u8 *Note);

//获取返回的options
bool SP_GetOptionsTotal(void *pOptionsBuf,u8 *Total);
bool SP_IsModify(void *pOptionsBuf,u8 Idx);
OP_RET_TYPE SP_GetType(void *pOptionsBuf,u8 Idx);
bool SP_GetOptionID(void *pOptionsBuf,u8 Idx,u8 *Id);
bool SP_GetOptionName(void *pOptionsBuf,u8 Idx,u8 **ppName);
bool SP_GetOptionNote(void *pOptionsBuf,u8 Idx,u8 **ppNote);
bool SP_GetYesOrNoOption(void *pOptionsBuf,u8 Idx,bool *pYesNo);
bool SP_GetNumOption(void *pOptionsBuf,u8 Idx,s32 *pVal);
bool SP_GetStrOption(void *pOptionsBuf,u8 Idx,u8 **ppStr);

#endif


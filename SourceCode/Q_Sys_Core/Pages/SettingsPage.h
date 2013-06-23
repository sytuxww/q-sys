#ifndef QSYS_SETTINGS_PAGE_H   //��������Ϊ���ñ��������ظ����ô��ļ���ÿ��h�ļ���ö�����ôһ�䣬���ǲ�Ҫ��һ���ĺ�Ŷ������
#define QSYS_SETTINGS_PAGE_H

//ÿ��ҳ��ֻ��Ҫ����һ��ȫ�ֱ�������������
//һ������²�Ҫ������Ķ������������ҳ����һ����ҳ�棬�ᱻ���ҳ�����
//�������ʧȥ�����ҳ��Ķ����ԣ����Ǻܼɻ��
extern const PAGE_ATTRIBUTE SettingsPage;

typedef enum {
	OPRT_Null=0,//������
	OPRT_YesOrNo,//TRUE or FALSE
	OPRT_Num,//����
	OPRT_Str,//�ַ�������
}OP_RET_TYPE;

//����options,Id�����1��ʼ
void SP_AddOptionsHeader(void *pBuf,u16 BufSize,u8 *Name,u8 OptsID);
void SP_AddYesOrNoOption(void *pBuf,u8 Id,bool DefValue,u8 *Name,u8 *Note);
void SP_AddNumOption(void *pBuf,u8 Id,s32 DefValue,u8 *Name,u8 *Note);
void SP_AddNumListOption(void *pBuf,u8 Id,s32 DefValue,s32 Min,s32 Max,s32 Step,u8 *Name,u8 *Note);
void SP_AddNumEnumOption(void *pBuf,u8 Id,bool IsDef,s32 Value,u8 *Name,u8 *Note);
void SP_AddStrEnumOption(void *pBuf,u8 Id,bool IsDef,u8 *pStr,u8 *Name,u8 *Note);

//��ȡ���ص�options
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


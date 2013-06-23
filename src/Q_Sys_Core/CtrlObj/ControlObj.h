#ifndef QSYS_CONTROL_OBJ_H
#define QSYS_CONTROL_OBJ_H

typedef enum{
	COT_NULL=0,
	COT_ImgBtn,
	COT_CharBtn,
	COT_DynImgBtn,
	COT_DynCharBtn,
	COT_YesNo,
	COT_Num,
	COT_Str,
}CTRL_OBJ_TYPE;

typedef struct{
	u8 ImgBtnNum;//ͼƬ������Ŀ�������ҳ��ֻ�õ���ͼƬ��ť
	u8 CharBtnNum;//���ְ�����Ŀ�������ҳ��ֻ�õ���ͼƬ��ť�����ֵ����Ϊ0����
	u8 DynImgBtnNum;//��̬ͼƬ�����ĸ���
	u8 DynCharBtnNum;//��̬���ְ����ĸ���
	u8 YesNoNum;//yes or no ѡ��ĸ���
	u8 NumCtrlObjNum;//���ֿ�ĸ���
	u8 StrCtrlObjNum;//�ַ�����ĸ���
}PAGE_CONTROL_NUM;//ҳ��Ķ�̬�ؼ�����

//4	����ҳ���UserEventHandler����ֵ��ּ�ڸ���ϵͳ��һЩ����
#define CO_MSG u32
//bit0-bit15 �����û��Զ��巵��ֵ
#define CO_RET_MASK 0xffff
//bit16-30���ڻش������ϵͳ������ֻ֧��15���ش�����
#define CO_CMD_MASK 0x7fff0000
#define CO_CMD_OFFSET 16
#define CO_TouchOff (1<<16)//��TouchEventHandler���ش�ֵ���رմ�����Ӧ
#define CO_TouchOn (1<<17)//��TouchEventHandler���ش�ֵ������������Ӧ
#define CO_ExtiKeyOff (1<<18)//��TouchEventHandler���ش�ֵ���ر��ⲿ��������
#define CO_ExtiKeyOn (1<<19)//��TouchEventHandler���ش�ֵ�������ⲿ��������
//bit31 ���ڻش�״̬��ϵͳ
#define CO_STATE_MASK 0x80000000
#define CO_STATE_OFFSET 31
#define CO_State_OK 0
#define CO_State_Faile 0x80000000

typedef enum {
	Tch_Press=0,//��������
	Tch_Continue,//���ִ���״̬�����ڴ��ݳ���ʱ��ʵʱ����
	Tch_Release,//����Ч�������ɿ�,��Ч����ָPressʱ����ע������
	Tch_ReleaseVain,//�ڷ���Ч�����ɿ�
	Tch_Normal,//���ڸı�ͼ�꣬��ʵ��case
}TCH_EVT;//4	����TouchEventHandler���¼�

typedef struct {//sizeof=8
	u16 x;	//����x����
	u16 y;	//����y����
	u16 Id;	//ֻ�а��²Żᵼ��id++,������ҳ�����ƥ��id���ж��ǲ���ͬһ�δ�������
	u16 TimeStamp;//ÿ��������ʱ�䣬���ֵ����λms
}TOUCH_INFO;//4		����TouchEventHandler�Ĳ���

//��һ����Ϊ��ֵ(��ʶ��)���ڶ�����Ϊ�����¼�����������Ϊ������Ϣ
typedef CO_MSG (*ButtonHandlerFunc)(u8 ,TCH_EVT , TOUCH_INFO *);
//��һ������Ϊ��ֵ(��ʶ��)���ڶ�������Ϊ��ǰֵ����ֵ�仯ʱ����
typedef CO_MSG (*YesNoHandlerFunc)(u8 ,bool );
//��һ������Ϊ��ֵ(��ʶ��)���ڶ�������Ϊ��ǰֵ��������ֵΪ�ؼ�����ָ�룬���ؼ�����ʱ����
typedef CO_MSG (*NumCtrlObjHanderFunc)(u8 ,s32,void *);
//��һ������Ϊ��ֵ(��ʶ��)���ڶ�������Ϊid������������Ϊ��ǰ�ַ��������ĸ�����Ϊ�ؼ�����ָ��
typedef CO_MSG (*StrCtrlObjHandlerFunc)(u8 ,u8 ,u8 *,void *);

//4	���������OptionMask����ֵ(���֧��16������)
//ע:��ΪTouch�̺߳�Input�߳���Դ�������޵�Ե�ʣ������¼����������ڵ�8λ
#define PrsMsk (1<<0) //Press�¼����룬���ھ��������Ƿ���ӦPress�¼���������ͬ
#define CotMsk (1<<1) //Continue�¼�����
#define RelMsk (1<<2)  //Release�¼�����
#define ReVMsk (1<<3) //ReleaseVain�¼�����

//�ǰ����¼�����ɷŸ�λ
#define LandMsk (1<<4) //���ھ��������Ƿ��ú���ģʽ��ʾ���ֻ���ͼƬ
#define BinMsk		(1<<5) //������ʱ������ʹ��bmp�ļ�������ʹ��bin�ļ���ͼ�꣬��ʱͼ��ߴ����w-2*ImgX,h-2*ImgY
#define DbgMsk	(1<<7) //����ģʽ��������ʾ��������ͼƬ�����򲻻���ʾͼƬ��ֻ��ʾ����
#define F16Msk	(1<<8) //�ı����ִ�СΪ16x16
#define B14Msk	(1<<9) //�ı�����ΪB14���壬����Ascii�ַ���Ч
#define PathMsk (1<<10) //������λ��Чʱ��BmpPathPrefixָ�����ַ�����������ͼƬ��·��ǰ׺����λ��Чʱ��BmpPathPrefix���theme·����Ϊͼ��·����
#define RoueMsk (1<<11) //Round Edge Բ��
//#define CirbMsk 	(1<<11) //Circular Bead Բ�ǣ��ݲ�֧��

typedef struct {
	u8 *Name;//������ͼ�갴�������ڲ�����ʾͼ��ʱ����ʾ�����ְ���������������ʾ��
	
	//��ֵ,�����������İ���
	// 1.С��USER_KEY_VALUE_START�ļ�ֵ������ϵͳʹ�ã����������������ⲿ���������İ���
	// 2.���������ͼƬ�����������ʣ����ֵ��ΧΪUSER_KEY_VALUE_START-0xff��ע�ⲻ��ʾͼ��Ҳ������ͼƬ�������͡�
	// 3.��������ļ�ֵ������ͬ������ͼƬ���ʺ���������֮�䶼��������ͬ��ֵ�İ�����ͬһ��ֵ�İ�����������Ӧ��
	u8 ObjID;

	//���ڶ���һЩ���������ԣ������¼���
	// 1.�����¼������������ͺ�ͼƬ�Ͷ���Ч�����ھ�����������Ҫ��Ӧ��Щ�¼���ȡֵ��PrsMsk|CotMsk|RelMsk|ReVMsk
	// 2.����ѡ����������ͺ�ͼƬ�Ͷ���Ч�����ھ����������Ƿ������ʾ��ע�����ʱ��˼άת��
	// 3.����ģʽ�����������ͺ�ͼƬ�Ͷ���Ч��������ʾ�������򣬷������ʱʹ��
	// 4.���ִ�Сѡ����������ͺ�ͼƬ�Ͷ���Ч�����ھ����Ƿ����16x16�����ִ�С��Ĭ��Ϊ12x12
	// 5.Բ�Ǳ�����ֻ������������Ч���������ֱ����Ľ�ΪԲ��
	// 6.Բ�߱�����ֻ������������Ч���������ֱ�������ΪԲ��
	// ��������ѡ��ɹ��棬��ֵʱʹ��"|"���ţ���PrsMsk|LandMsk|F16Msk
	u16 OptionsMask;
	
	u16 x; 	//������������xֵ����Ļ���ϵ�Ϊ0��0���ꡣ����Ǻ���ģʽ������Ҫ�����ʾ����ָ����ʾ��������ϵ㡣
	u16 y;	//������������yֵ
	u16 w; //����������,��������Ǻ���ģʽ,ע������ʾ��������
	u16 h;	//��������߶�,��������Ǻ���ģʽ,ע������ʾ��������	
	
	u16 ImgX;// ��������ͼƬ����ʾ���xֵ,������������Ͻǵ����ֵ
	u16 ImgY;//��������ͼƬ����ʾ���yֵ,������������Ͻǵ����ֵ

	//����ͼ��·��ǰ׺��Ϊ�˽�ʡrom�ռ䣬1.2��֮��ָ��
	//����ͼ�����ͬǰ׺����ָ��ÿ�ֶ�����Ӧ�ĺ�׺��ĸ
	//����:���BmpPathprefix="Dir/SubDir/Button",NormalSuffix='N',
	//��õ���ͨ״̬����·��Ϊ"Dir/SubDir/ButtonN.bmp"
	u8 *ImgPathPrefix;//���Ҫ����60���ַ�
	//u8 NormalSuffix;//��ͨ״̬��׺�����ֽڣ����ָ��Ϊ0������ʾ
	//u8 PressSuffix;//����״̬��׺�����ֽڣ����ָ��Ϊ0������ʾ
	//u8 ReleaseSuffix;//�ͷ�״̬��׺�����ֽڣ����ָ��Ϊ0������ʾ
	//u8 ReserveU8;//����
	COLOR_TYPE TransColor; //32λ͸��ɫ��NO_TRANSP��ʾ����͸��ɫ
}IMG_BUTTON_OBJ;//4		��������������

typedef struct {
	u8 *Name;//������ͼ�갴�������ڲ�����ʾͼ��ʱ����ʾ�����ְ���������������ʾ��
	
	//��ֵ,�����������İ���
	// 1.С��0x40�ļ�ֵ������ϵͳʹ�ã����������������ⲿ���������İ���
	// 2.���������ͼƬ�����������ʣ����ֵ��ΧΪ0x40-0xff��ע�ⲻ��ʾͼ��Ҳ������ͼƬ�������͡�
	// 3.��������ļ�ֵ������ͬ������ͼƬ���ʺ���������֮�䶼��������ͬ��ֵ�İ�����ͬһ��ֵ�İ�����������Ӧ��
	u8 ObjID;

	//���ڶ���һЩ���������ԣ������¼���
	// 1.�����¼������������ͺ�ͼƬ�Ͷ���Ч�����ھ�����������Ҫ��Ӧ��Щ�¼���ȡֵ��PrsMsk|CotMsk|RelMsk|ReVMsk
	// 2.����ѡ����������ͺ�ͼƬ�Ͷ���Ч�����ھ����������Ƿ������ʾ��ע�����ʱ��˼άת��
	// 3.����ģʽ�����������ͺ�ͼƬ�Ͷ���Ч��������ʾ�������򣬷������ʱʹ��
	// 4.���ִ�Сѡ����������ͺ�ͼƬ�Ͷ���Ч�����ھ����Ƿ����16x16�����ִ�С��Ĭ��Ϊ12x12
	// 5.Բ�Ǳ�����ֻ������������Ч���������ֱ����Ľ�ΪԲ��
	// 6.Բ�߱�����ֻ������������Ч���������ֱ�������ΪԲ��
	// ��������ѡ��ɹ��棬��ֵʱʹ��"|"���ţ���PrsMsk|CotMsk|RelMsk|ReVMsk��
	u16 OptionsMask;
	
	u16 x; 	//������������xֵ����Ļ���ϵ�Ϊ0��0���ꡣ����Ǻ���ģʽ������Ҫ�����ʾ����ָ����ʾ��������ϵ㡣
	u16 y;	//������������yֵ
	u16 w; //����������,��������Ǻ���ģʽ,ע������ʾ��������
	u16 h;	//��������߶�,��������Ǻ���ģʽ,ע������ʾ��������	
	
	u8 CharX;// �����������ֵ���ʾ���xֵ,������������Ͻǵ����ֵ
	u8 CharY;//�����������ֵ���ʾ���yֵ,������������Ͻǵ����ֵ
	u8 Margin;//����λ��ʾ�������ҵ������԰ף�����λ��ʾ�������µ������԰ס�����ֵ��Ϊ���ô���������ڱ�������h��w��ȥ��Ӧ��Margin���ڱ���ɫʵ�ʵ�h��w��
	u8 Space;//����λ��ʾ�ּ�࣬����λ��ʾ�м��

	//TRANSPARENT��ʾʹ��͸��ɫ
	COLOR_TYPE NormalColor;//16λɫ�ʣ�ָ����ͨ״̬������ɫ
	COLOR_TYPE NormalBG;//16λɫ�ʣ�ָ����ͨ״̬����ɫ
	COLOR_TYPE PressColor;//16λɫ�ʣ�ָ������״̬������ɫ
	COLOR_TYPE PressBG;//16λɫ�ʣ�ָ������״̬����ɫ	
	COLOR_TYPE ReleaseColor;//16λɫ�ʣ�ָ���ͷ�״̬������ɫ
	COLOR_TYPE ReleaseBG;//16λɫ�ʣ�ָ���ͷ�״̬����ɫ
}CHAR_BUTTON_OBJ;//4		��������������

typedef struct {		
	u8 ObjID;//��ʶ����ҳ���ڱ���Ψһ�������������ؼ�����

	bool DefVal;//Ĭ��ֵ��TRUE����FALSE
	
	u16 x; 	//������������xֵ����Ļ���ϵ�Ϊ0��0���ꡣ����Ǻ���ģʽ������Ҫ�����ʾ����ָ����ʾ��������ϵ㡣
	u16 y;	//������������yֵ
}YES_NO_OBJ;//4		yes no�ؼ�����

typedef enum{
	NCOT_NumBox,
	NCOT_NumList,
	NCOT_NumEnum,
}NUM_CTRL_OBJ_TYPE;

//ע:NUM_BOX_OBJ��NUM_LIST_OBJ��NUM_ENUM_OBJ����ǰ�漸��Ԫ�صĶ������һ��!!!
typedef struct{
	u8 ObjID;//��ʶ����ҳ���ڱ���Ψһ�������������ؼ�����
	NUM_CTRL_OBJ_TYPE Type;//num box����

	u16 x; 	//������������xֵ����Ļ���ϵ�Ϊ0��0���ꡣ����Ǻ���ģʽ������Ҫ�����ʾ����ָ����ʾ��������ϵ㡣
	u16 y;	//������������yֵ
	u16 w; //����������,��������Ǻ���ģʽ,ע������ʾ��������
	
	s32 Value;//��ǰֵ

	s32 Max;
	s32 Min;
}NUM_BOX_OBJ;//4		���������

typedef NUM_BOX_OBJ NUM_CTRL_OBJ;//��NUM_CTRL_OBJ��������num�ؼ�ǰ�β��֣���������num�ؼ�ʵ��ǰ�γ�Ա�����붨��һ��

typedef struct{
	u8 ObjID;//��ʶ����ҳ���ڱ���Ψһ�������������ؼ�����
	NUM_CTRL_OBJ_TYPE Type;//num box����

	u16 x; 	//�ؼ�����ͷ����������xֵ����Ļ���ϵ�Ϊ0��0���ꡣ����Ǻ���ģʽ������Ҫ�����ʾ����ָ����ʾ��������ϵ㡣
	u16 y;	//�ؼ�����ͷ������������yֵ
	u16 w; //�ؼ�����ͷ������Ŀ��,��������Ǻ���ģʽ,ע������ʾ��������

	s32 Value;//��ǰֵ
	
	s32 Max;//���ֵ
	s32 Min;//��Сֵ
	s32 Step;//����
}NUM_LIST_OBJ;//4		���������

typedef struct{
	u8 ObjID;//��ʶ����ҳ���ڱ���Ψһ�������������ؼ�����
	NUM_CTRL_OBJ_TYPE Type;//num box����

	u16 x; 	//������������xֵ����Ļ���ϵ�Ϊ0��0���ꡣ����Ǻ���ģʽ������Ҫ�����ʾ����ָ����ʾ��������ϵ㡣
	u16 y;	//������������yֵ
	u16 w; //����������,��������Ǻ���ģʽ,ע������ʾ��������

	s32 Value;//��ǰֵ
	
	u8 Idx;//��ǰֵ����
	u8 Num;//��ǰֵ����
	u8 Total;//����������
	s32 *pEnumList;//ָ��ö�����Ĵ洢�ռ䣬�ڿؼ���Чʱ����ָ��ָ����ڴ�ҲӦ��Ч���˿ռ��迪�������з����ȡ
}NUM_ENUM_OBJ;//4		���������

typedef enum{
	SCOT_StrBox,
	SCOT_StrEnum,
}STR_CTRL_OBJ_TYPE;

typedef struct{
	u8 ObjID;//��ʶ����ҳ���ڱ���Ψһ�������������ؼ�����
	STR_CTRL_OBJ_TYPE Type;//str�ؼ�����

	u16 x; 	//�ؼ����߿����������xֵ����Ļ���ϵ�Ϊ0��0���ꡣ����Ǻ���ģʽ������Ҫ�����ʾ����ָ����ʾ��������ϵ㡣
	u16 y;	//�ؼ����߿������������yֵ
	u16 w; //�ؼ����߿������Ŀ��,��������Ǻ���ģʽ,ע������ʾ��������
	u16 h;  //�ؼ����߿������ĸ߶�

	u16 TotalSize;//�����ܴ�С
	u8 *pStrBuf;//�ַ�������
}STR_BOX_OBJ;//4		�ַ��������

typedef STR_BOX_OBJ STR_CTRL_OBJ;

typedef struct{
	u8 ObjID;//��ʶ����ҳ���ڱ���Ψһ�������������ؼ�����
	STR_CTRL_OBJ_TYPE Type;//str�ؼ�����
	
	u16 x; 	//������������xֵ����Ļ���ϵ�Ϊ0��0���ꡣ����Ǻ���ģʽ������Ҫ�����ʾ����ָ����ʾ��������ϵ㡣
	u16 y;	//������������yֵ
	u16 w; //����������,��������Ǻ���ģʽ,ע������ʾ��������
	
	u16 Idx;//��ǰ�ַ���λ��
	u16 Size;//��ǰ�����ַ���ռ�ó��ȣ��������һ���ַ����Ľ�����
	u16 TotalSize;//����������ܳ���
	u8 *pStrEnumBuf;//ָ���ַ������棬ÿ���ַ�����0������˻����迪�������з����ȡ
}STR_ENUM_OBJ;//4		�ַ���ö��ѡ���

typedef u8 REP_IMG_SUFX;//img����ļ���׺����

//api

//���ĵ�ǰ������ĳ����������ʾͼƬ
// 1.ֻ�Ե�ǰҳ����Ч,ת��ҳ�����ҳ���ʧЧ
// 2.ֻ��ָ����׺������ԭ������Դͼ����"MusicN.bmp"
//    ָ��Suffix='T'����ͼ����Դ���"MusicT.bmp"
//���Suffix=0����ָ�ԭʼͼ��
void Q_ChangeImgTchImg(u8 OID,u8 Suffix);

//��ChangeKeyImg��ԣ���ȡ��ǰ��ͼ���滻��׺ֵ
//����0��ʾ��Ĭ��ֵ
//���򷵻��滻�ĺ�׺
u8 Q_ReadImgTchImg(u8 OID);

//���ĵ�ǰ������ĳ�����ְ�������ʾ����
// 1.ֻ�Ե�ǰҳ����Ч,ת��ҳ�����ҳ���ʧЧ
//���NewName=NULL����ָ�ԭʼ����
void Q_ChangeCharTchName(u8 OID,u8 *NewName);

//��ChangeKeyName��ԣ���ȡ��ǰ��ͼ���滻��׺ֵ
//����NULL��ʾ��Ĭ��ֵ����key������Χ
u8 *Q_ReadCharTchName(u8 OID);

//���̳���ָ��������ֵ��ͼ������֣������д˼�ֵ�İ�������Ч
void Q_PresentTch(u8 OID,TCH_EVT Type);

//���������µĶ�̬ͼ�갴��
//pTchRegΪNULLʱ����ʾע���˿ؼ���Ч��
bool Q_SetDynamicImgTch(u8 Idx,IMG_BUTTON_OBJ *pBtnObj);

//���������µĶ�̬���ְ���
//pTchRegΪNULLʱ����ʾע���˿ؼ���Ч��
bool Q_SetDynamicCharTch(u8 Idx,CHAR_BUTTON_OBJ *pBtnObj);

//����yes no�ؼ���pYesNoָ����ڴ��ڵ����꺯���󲻿�ע��
//һ�����ã�������ҳ��ʱ�����õ����ڴ�
//���Ե�ҳ�滹����ʱ�����뱣֤���ڴ����
//Idx��1��ʼ
//pYesNoΪNULLʱ����ʾע���˿ؼ���Ч��
bool Q_SetYesNo(u8 Idx,YES_NO_OBJ *pYesNo);

//����num�ؼ���pNumBoxָ����ڴ��ڵ����꺯���󲻿�ע��
//һ�����ã�������ҳ��ʱ�����õ����ڴ�
//���Ե�ҳ�滹����ʱ�����뱣֤���ڴ����
//Idx��1��ʼ
//pNumCtrlObjΪNULLʱ����ʾע���˿ؼ���Ч��
bool Q_SetNumCtrlObj(u8 Idx,NUM_CTRL_OBJ *pNumCtrlObj);
#define Q_SetNumBox(Idx,pNumBox) Q_SetNumCtrlObj(Idx,(NUM_CTRL_OBJ *)pNumBox)
#define Q_SetNumList(Idx,pNumList) Q_SetNumCtrlObj(Idx,(NUM_CTRL_OBJ *)pNumList)
#define Q_SetNumEnum(Idx,pNumEnum) Q_SetNumCtrlObj(Idx,(NUM_CTRL_OBJ *)pNumEnum)

//����str�ؼ�,pStrCtrlObj��pStrCtrlObj->pBuf��������Ч���ڵ����꺯������ע��
//���Ե�ҳ�滹����ʱ�����뱣֤���ڴ����
//Idx��1��ʼ
//pStrCtrlObjΪNULLʱ����ʾע���˿ؼ���Ч��
bool Q_SetStrCtrlObj(u8 Idx,STR_CTRL_OBJ *pStrCtrlObj);
#define Q_SetStrBox(Idx,pStrBox) Q_SetStrCtrlObj(Idx,(STR_CTRL_OBJ *)pStrBox)
#define Q_SetStrEnum(Idx,pStrEnum) Q_SetStrCtrlObj(Idx,(STR_CTRL_OBJ *)pStrEnum)

bool Q_StrEnumDisplayOne(STR_ENUM_OBJ *pStrEnumObj,u8 StrID);
bool Q_StrEnumAddOne(STR_ENUM_OBJ *pStrEnumObj,u8 StrID,u8 *Str);
bool Q_StrEnumDeleteOne(STR_ENUM_OBJ *pStrEnumObj,u8 StrID);

#endif


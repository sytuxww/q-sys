#ifndef QSYS_CTRL_OBJ_IMG_H
#define QSYS_CTRL_OBJ_IMG_H


//yes no �ؼ�
#define CO_YES_NO_W	58 //yes no�ؼ��Ŀ��
#define CO_YES_NO_H	18 //yes no�ؼ��ĸ߶�
#define CO_YES_NO_TRAN_COLOR	FatColor(0x0000ff)//yes no�ؼ���͸��ɫ
extern const unsigned char gCtrlObj_On[]; //yes no�ؼ���onͼƬ����
extern const unsigned char gCtrlObj_Off[]; //yes no�ؼ���yesͼƬ����
extern const unsigned char gCtrlObj_OnOff[]; //yes no�ؼ���yesno����

//num�ؼ� 
#define CO_NUM_H 18 //num�ؼ��߶�
#define CO_NUM_FRAME_W 4 //num�ؼ����ұ߿���
#define CO_NUM_ARROW_W 18 //num�ؼ���ͷ���
#define CO_NUM_MIDDLE_W	1 //num�ؼ��м���䲿�ֵĵ�λ���
#define CO_NUM_TRAN_COLOR FatColor(0xff0000) //num�ؼ�͸��ɫ
#define CO_NUM_FONT_STYLE 	ASC14B_FONT //num�ؼ���ʾ����
#define CO_NUM_FONT_W	8 //num�ؼ�ascii������
#define CO_NUM_FONT_SPACE 0 //num�ؼ�����������
#define CO_NUM_FONT_COLOR FatColor(0x333333) //num�ؼ�������ɫ
#define CO_NUM_FONT_COLOR_H FatColor(0xf1aa00) //num�ؼ���������ɫ
extern const unsigned char gCtrlObj_NumLeft[]; //num�ؼ���߿�
extern const unsigned char gCtrlObj_NumLeftArrow[]; //num�ؼ����ͷ
extern const unsigned char gCtrlObj_NumMiddle[]; //num�ؼ��м����
extern const unsigned char gCtrlObj_NumRight[]; //num�ؼ��ұ߿�
extern const unsigned char gCtrlObj_NumRightArrow[]; //num�ؼ��Ҽ�ͷ
extern const unsigned char gCtrlObj_NumLeftH[]; //num�ؼ���߿����
extern const unsigned char gCtrlObj_NumLeftArrowH[]; //num�ؼ����ͷ����
extern const unsigned char gCtrlObj_NumMiddleH[]; //num�ؼ��м�������
extern const unsigned char gCtrlObj_NumRightH[]; //num�ؼ��ұ߿����
extern const unsigned char gCtrlObj_NumRightArrowH[]; //num�ؼ��Ҽ�ͷ����

//str Enum�ؼ�(��ԴͬNum�ؼ�)
#define CO_STR_ENUM_H 18
#define CO_STR_ENUM_FRAME_W 4 //str opt�ؼ����ұ߿���
#define CO_STR_ENUM_ARROW_W 18 //str opt�ؼ���ͷ���
#define CO_STR_ENUM_MIDDLE_W	1 //str opt�ؼ��м���䲿�ֵĵ�λ���
#define CO_STR_ENUM_TRAN_COLOR FatColor(0xff0000) //str opt�ؼ�͸��ɫ
#define CO_STR_ENUM_FONT_STYLE GBK12_FONT //str opt�ؼ���ʾ����
#define CO_STR_ENUM_FONT_W	6 //str opt�ؼ�ascii������
#define CO_STR_ENUM_FONT_SPACE 0 //str opt�ؼ�����������
#define CO_STR_ENUM_FONT_COLOR FatColor(0x333333) //str opt�ؼ�������ɫ
#define CO_STR_ENUM_FONT_COLOR_H FatColor(0xf1aa00) //str opt�ؼ���������ɫ
#define gCtrlObj_StrEnumLeft gCtrlObj_NumLeft //str opt�ؼ���߿�
#define gCtrlObj_StrEnumLeftArrow gCtrlObj_NumLeftArrow //str opt�ؼ����ͷ
#define gCtrlObj_StrEnumMiddle gCtrlObj_NumMiddle //str opt�ؼ��м����
#define gCtrlObj_StrEnumRight gCtrlObj_NumRight //str opt�ؼ��ұ߿�
#define gCtrlObj_StrEnumRightArrow gCtrlObj_NumRightArrow //str opt�ؼ��Ҽ�ͷ
#define gCtrlObj_StrEnumLeftH gCtrlObj_NumLeftH //str opt�ؼ���߿����
#define gCtrlObj_StrEnumLeftArrowH gCtrlObj_NumLeftArrowH //str opt�ؼ����ͷ����
#define gCtrlObj_StrEnumMiddleH gCtrlObj_NumMiddleH//str opt�ؼ��м�������
#define gCtrlObj_StrEnumRightH gCtrlObj_NumRightH //str opt�ؼ��ұ߿����
#define gCtrlObj_StrEnumRightArrowH gCtrlObj_NumRightArrowH //str opt�ؼ��Ҽ�ͷ����

//str box�ؼ�


#endif


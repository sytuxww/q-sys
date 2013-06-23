#ifndef QSYS_TOUCH_HANDLER_H
#define QSYS_TOUCH_HANDLER_H

typedef struct{
	u16 x;//��������x
	u16 y;//��������y
	u16 w;//��������w
	u16 h;//��������h
	u8 ObjID;//�������������ؼ���id�ţ��û���ҳ����ָ������Ψһ
	CTRL_OBJ_TYPE Type;//�������������ؼ����ͣ���CTRL_OBJ_TYPE
	u8 Index;//���������������ؼ����е����������������
	u8 OptionsMask;//������Ӧ����
}TOUCH_REGION;
//����touch handler���ԣ���ֻ�����Ӧ�����㵽ע���������
//����ҵ���ǰ�����Ӧ�����򣬲������뿪�ţ����Ͱ������ͺ�����ֵ���¼����ں�

extern u8 gTouchRegionNum;//��¼����ע����������ı���
extern TOUCH_REGION *gpTouchRegions;//��¼����ע�������ָ�����


#endif


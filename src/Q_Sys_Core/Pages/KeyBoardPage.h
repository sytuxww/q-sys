#ifndef QSYS_KEYBOARD_PAGE_H   //��������Ϊ���ñ��������ظ����ô��ļ���ÿ��h�ļ���ö�����ôһ�䣬���ǲ�Ҫ��һ���ĺ�Ŷ������
#define QSYS_KEYBOARD_PAGE_H

#define KBMSK_NAME_MODE    (1<<24)	//��ֹ���,�س�,�ո�
#define KBMSK_NUM_DIS		(1<<25)	//��ֹ����ģʽ
#define KBMSK_LETTER_DIS	(1<<26)	//��ֹ��ĸģʽ
#define KBMSK_PINYIN_DIS	(1<<27) //��ֹƴ��ģʽ
#define KBMSK_NUM_DEF		(1<<28)	//Ĭ������ģʽ
#define KBMSK_LETTER_DEF	(1<<29)	//Ĭ����ĸģʽ
#define KBMSK_PINYIN_DEF	(1<<30)	//Ĭ��ƴ��ģʽ

//ע:��ʹ��Q_GotoPage�����ҳ��ʱ������pSysParam���ݴ��༭���������׵�ַ������IntParam�ĵ�16λ���ݴ��༭�������Ĵ�С����16λ������������
//ÿ��ҳ��ֻ��Ҫ����һ��ȫ�ֱ�������������
//һ������²�Ҫ������Ķ������������ҳ����һ����ҳ�棬�ᱻ���ҳ�����
//�������ʧȥ�����ҳ��Ķ����ԣ����Ǻܼɻ��
extern const PAGE_ATTRIBUTE KeyBoardPage;



#endif


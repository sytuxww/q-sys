#ifndef QSYS_QWEB_PAGE_H   //��������Ϊ���ñ��������ظ����ô��ļ���ÿ��h�ļ���ö�����ôһ�䣬���ǲ�Ҫ��һ���ĺ�Ŷ������
#define QSYS_QWEB_PAGE_H

typedef struct {
	u8 PageTitle[32];
	PAGE_ACTION GotoAct_List;//�����б�������Ժ��ַ�ʽ�������ҳ��
	u8 GotoName_List[20];//�����б����������ҳ��
	PAGE_ACTION GotoAct_Back;//����back�����Ժ��ַ�ʽ�������ҳ��
	u8 GotoName_Back[20];//����back��������ҳ��
	PAGE_ACTION GotoAct_Done;//����done�����Ժ��ַ�ʽ�������ҳ��
	u8 GotoName_Done[20];//����done��������ҳ��
}QWEB_PAGE_SET;

//ÿ��ҳ��ֻ��Ҫ����һ��ȫ�ֱ�������������
//һ������²�Ҫ������Ķ������������ҳ����һ����ҳ�棬�ᱻ���ҳ�����
//�������ʧȥ�����ҳ��Ķ����ԣ����Ǻܼɻ��
extern const PAGE_ATTRIBUTE QWebPage;

u8 *QWP_GetNameByAddr(u8 Addr);


#endif


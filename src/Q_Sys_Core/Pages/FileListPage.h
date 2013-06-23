#ifndef QSYS_FILE_LIST_PAGE_H
#define QSYS_FILE_LIST_PAGE_H

typedef u32 FILELIST_CMD;
//��8λ���ڴ�������
#define FL_CreatListBuf 1 //����һ��Ŀ¼·������׺���ַ��������������ҳ�棬�ؽ���Ŀ¼�µ��б������ļ�list.buf
#define FL_SelectOne 2//����һ��Ŀ¼·������׺���ַ�����������ҳ�棬�г�Ŀ¼��ѡȡһ���ļ��󷵻�
#define FL_FindNext 3//����һ���ļ�·������׺���ַ��������������ҳ�棬ֱ�ӷ�����һ���ļ���������Ŀ¼·����ʾ�Ѿ�����Ŀ¼�ײ�
#define FL_FindPrevious 4//����һ���ļ�·������׺���ַ��������������ҳ�棬ֱ�ӷ�����һ���ļ���������Ŀ¼·����ʾ�Ѿ�����Ŀ¼ͷ��
#define FL_FindFrist 5//����һ��Ŀ¼·������׺���ַ��������������ҳ�棬���ص�һ���ļ���������Ŀ¼·����ʾû���ļ�
#define FL_FindLast 6//����һ��Ŀ¼·������׺���ַ��������������ҳ�棬�������һ���ļ���������Ŀ¼·����ʾû���ļ�
#define FL_CMD_MASK 0xff

//8-15λ�������ݲ���
#define FL_NoListBuf	(1<<8)//ǿ�Ʋ������κ�List.buf�ļ������ܺ�FL_ForceListBufͬʱʹ��
#define FL_ForceListBuf	(1<<9)//ǿ�ƽ���ÿ��Ŀ¼������List.buf�ļ�,���ȼ�����FL_NoListBuf
#define FL_NoParent  (1<<10)//��ֹ�����ʼ��Ŀ¼���ϲ�Ŀ¼


typedef struct {
	PAGE_RID CallBackRid;//��ɺ󷵻ص�ҳ��RID
	
	//ͨ�����ڴ洫�ݶ�ȡ·����������ѡ���ļ���·�������ڴ�
	//���Դ�ָ��ָ����ڴ���FileListҳ��û�з���֮ǰһ�����ܱ�ע��
	u8 *pPathBuf;//��ʼĿ¼·��������Ϊ1024
	u8 *pSuffixStr;//��׺���ַ�������".mp3|.txt|.bmp"
}FILELIST_SET;

extern const PAGE_ATTRIBUTE FileListPage;

#endif


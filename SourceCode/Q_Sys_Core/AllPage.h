/* ��ϵͳ ˵��
�����ϵͳ���κ�ҳ������ȵ����ļ���ע�ᣬע����2��:
1.��ҳ���ͷ�ļ������ڴ��ļ���
2.��ҳ������Խṹ��PAGE_ATTRIBUTE������AllPage�ṹ����

��Ҫע�����:
1.AllPage��ÿ��ҳ������е�����id������һ����Ա��
��id���ǳ�֮Ϊע��id(RegID or RID)��RegID��Ψһ
���þ���������ע�Ͳ���ҳ�棬����ֻ��ҪΨһ�Լ��ɡ�
RegID����Ϊ����ֵ��

2.ϵͳ��ÿ��ҳ�涼�����Ƕ����ģ���һ��ҳ���Դ�벻�ܵ�����һ��
ҳ��ĺ����������ͱ�֤�˾������ҳ��ɾ����Ҳ����Ӱ������ҳ�档
��������Կ�����ϵͳ�ĺܶ�ҳ���ﶼ�ظ�����ͬ�����ܵĺ�����
���⣬ҳ���е��Զ��庯����ȫ�ֱ����������staticǰ׺����������Ϊ��
���õı�����Դ��ѧ��c++�����ѿ��Խ�ҳ�濴�ɶ���

3.��ϵͳ�У�ĳЩҳ����Ҫ���ظ����ã����ļ��б�ҳ�棬��ʱ������
���Խ���ҳ������Ͷ���ΪREG_SUB_PAGE_TYPE������REG_PAGE_TYPE�����ʾ���
ҳ���ǿ��Ա����ҳ�����(����)�ġ���ʵ���ϣ���ҳ��Ҳֻ����������
ҳ��ʹ�����䶨������ͣ���û���ñ��ҳ��ʹ���亯�������ô�ҳ��
�Ĳ������ݣ�ͨ��GotPage��ɡ�

4.�ڴ��ļ����ҳ�����ע��󣬲�����ʾϵͳ������ʾ���¼ӵ�ҳ�档
����Ҫ������ҳ���е���GotoPage()�����ſ��Խ����ҳ��
*/

#ifndef QSYS_ALL_PAGE_H
#define QSYS_ALL_PAGE_H

#include "MainPage.h"
#include "MusicPage.h"
#include "FileListPage.h"
#include "EBookPage.h"
#include "PicturePage.h"
#include "SettingsPage.h"
#include "QWebPage.h"
#include "TestPage.h"
#include "AppListPage.h"
#include "KeyBoardPage.h"
#include "SnakePage.h"
#include "TouchCheckPage.h"
#include "ChatPage.h"
#include "NewsPage.h"
#include "NumCtrlObjPage.h"
#include "StrCtrlObjPage.h"
//#include "WaveDispPage.h"

const PAGE_CONGREGATION AllPage[]={//ҳ���������ô���255��RID����Ϊ��
	{PRID_MainPage,&MainPage},
	{PRID_MusicPage,&MusicPage},
	{PRID_FileListPage,&FileListPage},
	{PRID_NumCtrlObjPage,&NumCtrlObjPage},
	{PRID_StrCtrlObjPage,&StrCtrlObjPage},
	{PRID_EBookPage,&EBookPage},
	{PRID_PicturePage,&PicturePage},
	{PRID_SettingsPage,&SettingsPage},
	{PRID_TestPage,&TestPage},
	{PRID_KeyBoardPage,&KeyBoardPage},
	{PRID_QWebPage,&QWebPage},
	{PRID_ChatPage,&ChatPage},
	{PRID_NewsPage,&NewsPage},
	{PRID_AppListPage,&AppListPage},
	{PRID_SnakePage,&SnakePage},
	{PRID_TouchCheckPage,&TouchCheckPage},
	//{PRID_WaveDispPage,&WaveDispPage}
};

#define PAGE_TOTAL (sizeof(AllPage)/sizeof(PAGE_CONGREGATION))//ҳ��������������255
const u8 gPageTotal=PAGE_TOTAL;

#endif


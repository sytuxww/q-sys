/* 酷系统 说明
加入酷系统的任何页面必须先到此文件中注册，注册有2步:
1.将页面的头文件包含在此文件中
2.将页面的属性结构体PAGE_ATTRIBUTE包含到AllPage结构体中

需要注意的是:
1.AllPage中每个页面必须有单独的id，即第一个成员，
此id我们称之为注册id(RegID or RID)，RegID的唯一
作用就是用来标注和查找页面，所以只需要唯一性即可。
RegID必须为非零值。

2.系统中每个页面都必须是独立的，即一个页面的源码不能调用另一个
页面的函数，这样就保证了就算你的页面删除，也不会影响其他页面。
所以你可以看到在系统的很多页面里都重复定义同样功能的函数。
另外，页面中的自定义函数和全局变量都必须加static前缀，这样做是为了
更好的保护资源。学过c++的朋友可以将页面看成对象

3.在系统中，某些页面需要被重复调用，如文件列表页面，这时，我们
可以将此页面的类型定义为REG_SUB_PAGE_TYPE而不是REG_PAGE_TYPE。这表示这个
页面是可以被别的页面调用(依赖)的。但实际上，此页面也只不过让其他
页面使用了其定义的类型，而没有让别的页面使用其函数。调用此页面
的参数传递，通过GotPage完成。

4.在此文件里对页面进行注册后，并不表示系统就能显示你新加的页面。
你需要在其他页面中调用GotoPage()函数才可以进入此页面
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

const PAGE_CONGREGATION AllPage[]={//页面总数不得大于255，RID不能为零
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

#define PAGE_TOTAL (sizeof(AllPage)/sizeof(PAGE_CONGREGATION))//页面总数，不大于255
const u8 gPageTotal=PAGE_TOTAL;

#endif


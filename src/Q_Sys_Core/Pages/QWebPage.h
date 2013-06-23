#ifndef QSYS_QWEB_PAGE_H   //这两句是为了让编译器不重复调用此文件，每个h文件最好都有这么一句，但是不要用一样的宏哦！！！
#define QSYS_QWEB_PAGE_H

typedef struct {
	u8 PageTitle[32];
	PAGE_ACTION GotoAct_List;//按下列表项后需以何种方式进入的新页面
	u8 GotoName_List[20];//按下列表项后需进入的页面
	PAGE_ACTION GotoAct_Back;//按下back后需以何种方式进入的新页面
	u8 GotoName_Back[20];//按下back后需进入的页面
	PAGE_ACTION GotoAct_Done;//按下done后需以何种方式进入的新页面
	u8 GotoName_Done[20];//按下done后需进入的页面
}QWEB_PAGE_SET;

//每个页面只需要申明一个全局变量和两个函数
//一般情况下不要申明别的东西，除非你的页面是一个子页面，会被别的页面调用
//否则你就失去了你的页面的独立性，这是很忌讳的
extern const PAGE_ATTRIBUTE QWebPage;

u8 *QWP_GetNameByAddr(u8 Addr);


#endif


#ifndef QSYS_KEYBOARD_PAGE_H   //这两句是为了让编译器不重复调用此文件，每个h文件最好都有这么一句，但是不要用一样的宏哦！！！
#define QSYS_KEYBOARD_PAGE_H

#define KBMSK_NAME_MODE    (1<<24)	//禁止标点,回车,空格
#define KBMSK_NUM_DIS		(1<<25)	//禁止数字模式
#define KBMSK_LETTER_DIS	(1<<26)	//禁止字母模式
#define KBMSK_PINYIN_DIS	(1<<27) //禁止拼音模式
#define KBMSK_NUM_DEF		(1<<28)	//默认数字模式
#define KBMSK_LETTER_DEF	(1<<29)	//默认字母模式
#define KBMSK_PINYIN_DEF	(1<<30)	//默认拼音模式

//注:在使用Q_GotoPage进入此页面时，参数pSysParam传递待编辑缓冲区的首地址，参数IntParam的低16位传递待编辑缓冲区的大小，高16位用作功能掩码
//每个页面只需要申明一个全局变量和两个函数
//一般情况下不要申明别的东西，除非你的页面是一个子页面，会被别的页面调用
//否则你就失去了你的页面的独立性，这是很忌讳的
extern const PAGE_ATTRIBUTE KeyBoardPage;



#endif


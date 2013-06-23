#ifndef QSYS_NUM_CTRL_OBJ_PAGE_H   //这两句是为了让编译器不重复调用此文件，每个h文件最好都有这么一句，但是不要用一样的宏哦！！！
#define QSYS_NUM_CTRL_OBJ_PAGE_H

//每个页面只需要申明一个全局变量和两个函数
//一般情况下不要申明别的东西，除非你的页面是一个子页面，会被别的页面调用
//否则你就失去了你的页面的独立性，这是很忌讳的
extern const PAGE_ATTRIBUTE NumCtrlObjPage;



#endif


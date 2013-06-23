#ifndef QSYS_FILE_LIST_PAGE_H
#define QSYS_FILE_LIST_PAGE_H

typedef u32 FILELIST_CMD;
//低8位用于传递命令
#define FL_CreatListBuf 1 //输入一个目录路径及后缀名字符串，不会进入子页面，重建此目录下的列表索引文件list.buf
#define FL_SelectOne 2//输入一个目录路径及后缀名字符串，进入子页面，列出目录并选取一个文件后返回
#define FL_FindNext 3//输入一个文件路径及后缀名字符串，不会进入子页面，直接返回下一个文件名，返回目录路径表示已经到达目录底部
#define FL_FindPrevious 4//输入一个文件路径及后缀名字符串，不会进入子页面，直接返回上一个文件名，返回目录路径表示已经到达目录头部
#define FL_FindFrist 5//输入一个目录路径及后缀名字符串，不会进入子页面，返回第一个文件名，返回目录路径表示没有文件
#define FL_FindLast 6//输入一个目录路径及后缀名字符串，不会进入子页面，返回最后一个文件名，返回目录路径表示没有文件
#define FL_CMD_MASK 0xff

//8-15位用来传递参数
#define FL_NoListBuf	(1<<8)//强制不生成任何List.buf文件，不能和FL_ForceListBuf同时使用
#define FL_ForceListBuf	(1<<9)//强制进入每个目录都生成List.buf文件,优先级低于FL_NoListBuf
#define FL_NoParent  (1<<10)//禁止进入初始化目录的上层目录


typedef struct {
	PAGE_RID CallBackRid;//完成后返回的页面RID
	
	//通过此内存传递读取路径，并返回选择文件的路径到此内存
	//所以此指针指向的内存在FileList页面没有返回之前一定不能被注销
	u8 *pPathBuf;//初始目录路径，长度为1024
	u8 *pSuffixStr;//后缀名字符串，如".mp3|.txt|.bmp"
}FILELIST_SET;

extern const PAGE_ATTRIBUTE FileListPage;

#endif


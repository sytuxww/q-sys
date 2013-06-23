#ifndef QSYS_PIN_YIN_H
#define QSYS_PIN_YIN_H

//查询索引类型
typedef const struct{//用于第二次查表 用两次查表比一次查表效率要高
	const unsigned char *matchstr;//第二个字符开始的匹配字符串，
	const unsigned char *pmaybe;//指向相应的疑似字符串数组的指针
}PY_indexdef;
 
//拼音查询函数 
//输入:拼音字符串 
//输出:返回最后一次匹配成功的疑似汉字数组 若连第一个字母都不匹配则查询失败返回NULL
extern const unsigned char * py_to_str(unsigned char input_py_val[]);

#endif


#ifndef QSYS_TOUCH_HANDLER_H
#define QSYS_TOUCH_HANDLER_H

typedef struct{
	u16 x;//触摸区域x
	u16 y;//触摸区域y
	u16 w;//触摸区域w
	u16 h;//触摸区域h
	u8 ObjID;//触摸区域所属控件的id号，用户在页面中指定，需唯一
	CTRL_OBJ_TYPE Type;//触摸区域所属控件类型，见CTRL_OBJ_TYPE
	u8 Index;//触摸区域在所属控件组中的相对索引，暂无用
	u8 OptionsMask;//功能响应掩码
}TOUCH_REGION;
//对于touch handler而言，它只负责对应触摸点到注册的区域中
//如果找到当前触点对应的区域，并且掩码开放，则发送包含类型和索引值的事件给内核

extern u8 gTouchRegionNum;//记录所有注册区域个数的变量
extern TOUCH_REGION *gpTouchRegions;//记录所有注册区域的指针变量


#endif


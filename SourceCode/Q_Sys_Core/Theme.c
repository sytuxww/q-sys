#include "System.h"

//主题相关
static const u8 *SysThemes[]={"Theme/F/"};
//主题相关结束

//获取当前主题的路径
const u8 *Q_GetNowThemePath(void)
{
	return SysThemes[0];
}

void DrawTitle1(FONT_ACT FontLib,const u8 *pTitle,u16 xStart,u8 ByteLen,COLOR_TYPE Color)
{
	GUI_REGION DrawRegion;
	
	switch(FontLib)
	{
		case GBK12_FONT:
		case GBK12_NUM:
			DrawRegion.w=ByteLen*6;
			break;
		case GBK16_FONT:
		case GBK16_NUM:
			DrawRegion.w=ByteLen*8;
			break;
		case ASC14B_FONT:
		case ASC14B_NUM:
			DrawRegion.w=ByteLen*8;
			break;
		default:
			return;
	}
	DrawRegion.x=xStart;
	DrawRegion.y=0;
	DrawRegion.h=21;
	DrawRegion.Color=FatColor(NO_TRANS);
	Gui_FillImgArray((u8 *)gImage_StatusBar1,1,21,&DrawRegion);//画状态栏

	//写标题
	DrawRegion.Color=Color;
	DrawRegion.Space=0x00;

	switch(FontLib)
	{
		case GBK12_FONT:
		case GBK12_NUM:
			DrawRegion.y=4;
			Gui_DrawFont(GBK12_FONT,pTitle,&DrawRegion);
			break;
		case GBK16_FONT:
		case GBK16_NUM:
			DrawRegion.y=2;
			Gui_DrawFont(GBK16_FONT,pTitle,&DrawRegion);
			break;
		case ASC14B_FONT:
		case ASC14B_NUM:
			DrawRegion.y=3;
			Gui_DrawFont(ASC14B_FONT,pTitle,&DrawRegion);
			break;
	}	
}

void DrawFrame1(u16 y_start,u16 h)
{
	GUI_REGION DrawRegion;
	u16 wc=12,hc=13;
	u16 x=0,w=240;
	
	DrawRegion.x=x;
	DrawRegion.y=y_start;
	DrawRegion.w=wc;
	DrawRegion.h=hc;
	DrawRegion.Color=FatColor(NO_TRANS);
	Gui_DrawImgArray((u8 *)gImage_Frame1LT,&DrawRegion);//left top

	DrawRegion.x=x+w-wc;
	DrawRegion.y=y_start;
	DrawRegion.w=wc;
	DrawRegion.h=hc;
	DrawRegion.Color=FatColor(NO_TRANS);
	Gui_DrawImgArray((u8 *)gImage_Frame1RT,&DrawRegion);//right top

	DrawRegion.x=x;
	DrawRegion.y=y_start+h-hc;
	DrawRegion.w=wc;
	DrawRegion.h=hc;
	DrawRegion.Color=FatColor(NO_TRANS);
	Gui_DrawImgArray((u8 *)gImage_Frame1LB,&DrawRegion);//left bottom

	DrawRegion.x=x+w-wc;
	DrawRegion.y=y_start+h-hc;
	DrawRegion.w=wc;
	DrawRegion.h=hc;
	DrawRegion.Color=FatColor(NO_TRANS);
	Gui_DrawImgArray((u8 *)gImage_Frame1RB,&DrawRegion);//right bottom

	DrawRegion.x=x;
	DrawRegion.y=y_start+hc;
	DrawRegion.w=w;
	DrawRegion.h=h-2*hc;
	DrawRegion.Color=FatColor(0xe0e0e0);
	Gui_FillBlock(&DrawRegion);//中间填充

	DrawRegion.x=x+wc;
	DrawRegion.y=y_start+1;
	DrawRegion.w=w-2*wc;
	DrawRegion.h=hc-1;
	DrawRegion.Color=FatColor(0xe0e0e0);
	Gui_FillBlock(&DrawRegion);//上填充

	DrawRegion.x=x+wc;
	DrawRegion.y=y_start+h-hc;
	DrawRegion.w=w-2*wc;
	DrawRegion.h=hc-1;
	DrawRegion.Color=FatColor(0xe0e0e0);
	Gui_FillBlock(&DrawRegion);			//下填充
	
	Gui_DrawLine(x+wc,y_start,x+w-wc,y_start,FatColor(0x2a2a2a));//上横线
	Gui_DrawLine(x+wc,y_start+h-1,x+w-wc,y_start+h-1,FatColor(0x2a2a2a));//下横线	
}



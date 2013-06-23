#ifndef QSYS_THEME_H
#define QSYS_THEME_H

const u8 *Q_GetNowThemePath(void);
void DrawTitle1(FONT_ACT FontLib,const u8 *pTitle,u16 xStart,u8 ByteLen,COLOR_TYPE Color);
void DrawFrame1(u16 y_start,u16 h);

#endif


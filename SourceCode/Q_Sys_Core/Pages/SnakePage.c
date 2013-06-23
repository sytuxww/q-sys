/**************** Q-SYS *******************
 * PageName : SnakePage
 * Author : ChenYu
 * Version : 1.0 
 * Base Q-Sys Version : 2.x
 * Description : greedy snake
 * WebSite : Www.Q-ShareWe.Com
 ***************** Q-SYS *******************/
 
#include "User.h"
#include "Theme.h"
#include "SnakePage.h"
#include "SettingsPage.h"
//---------------------本页自定义类型及宏定义------------------
//游戏色彩
#define WHITE			(FatColor(0xFFFFFF))
#define BLUE			(FatColor(0x0000FF))
#define	GREEN			(FatColor(0x00FF00))
#define RED				(FatColor(0xFF0000))
#define	PURPLE			(FatColor(0xDB70DB))
#define	ORANGE			(FatColor(0xE47833))
#define BLACK			(FatColor(0x000000))

//默认颜色
#define DEFA_SNAKE_COLOURLEV 3
#define DEFA_FOOD_COLOURLEV	 1
#define DEFA_BG_COLOURLEV	 2 
#define DEFA_SNAKE_COLOUR	BLUE
#define DEFA_FOOD_COLOUR	RED
#define DEFA_BG_COLOUR		GREEN 
 
//蛇的快慢选择
#define SLOW			6000
#define SWIFT			3000
#define LIGHTNING		1500

//蛇的大小选择
#define SMALL			5
#define NORMAL			10
#define BIG				20

//默认大小和形状
#define DEFA_CUR_SPEEDLEV	2
#define DEFA_CUR_SIZELEV	2
#define DEFA_SPEED			SWIFT
#define DEFA_SIZE			NORMAL

//场地边界的像素
#define GROUND_W_PIX	0
#define GROUND_E_PIX	239
#define GROUND_N_PIX	21
#define GROUND_S_PIX	280
//场地边界的坐标	
#define WESTWALL		0
#define EASTWALL		(((GROUND_E_PIX-GROUND_W_PIX+1)/gpSpVars->CUR_SIZE)-1)
#define NORTHWALL		0
#define SOUTHWALL		(((GROUND_S_PIX-GROUND_N_PIX+1)/gpSpVars->CUR_SIZE)-1)
//循环队列至少需保存的元素数目
#define MAXQSIZE		((EASTWALL-WESTWALL+1)*(SOUTHWALL-NORTHWALL+1))	

typedef enum{ //绝对方向 屏幕上方对应NOUTH
	EAST, 
	SOUTH,
	WEST,
	NORTH
}SnakeDir;	

typedef struct{	 //循环队列类型声明
	u16  *base;
	u16  front;
	u16  rear;
}SqQueue;          
    
typedef struct{  //蛇的结构体声明
	s8 hposx;	 //当前蛇头坐标
	s8 hposy;	 
	s8 tposx;	 //当前蛇尾坐标
	s8 tposy;	 
	s8 nhposx;	 //预计下次的蛇头坐标
	s8 nhposy;	 
	SnakeDir dir;//当前蛇头的运动方向
	u16 eatnum;	 //当前得分
}snakedef,*psnake;

//注：“当前”是指在下次定时触发前的这段时间
//“预计”是根据当前蛇头的坐标和方向推断(不管是否死亡或吃到食物)出的下次蛇头的位置
typedef struct{  //食物类型声明
	u8 x;		 //食物坐标
	u8 y;
}fooddef,*pfood; 

//需要setting的项目
typedef enum{
	OSID_SNAKE=1,
}OPTIONS_ID;
typedef enum{
	SNAKEOP_SPEED=1,
	SNAKEOP_SIZE,
	SNAKEOP_SNAKE_COLOUR,
	SNAKEOP_FOOD_COLOUR,
	SNAKEOP_BG_COLOUR
}SNAKE_SETTINGS_ITEM;
//----------------------本页自定义函数声明---------------------
//创建循环队列
static void InitQueue(SqQueue *Q);
//销毁循环队列
static void DestroyQueue(SqQueue *Q);
//清空循环队列
static void ClearQueue(SqQueue *Q);
//插入元素为队尾元素
static void EnQueue(SqQueue *Q,u16 e);
//获得队头元素并删除队头 
static void DeQueue(SqQueue *Q,u16 *e);
//获取队列长度
static u16 QueueLength(SqQueue *Q);
//将一组坐标压入队尾
static void InQ_xy(u8 x,u8 y);
//从队头取出一组坐标
static void OutQ_xy(u8 *x,u8 *y);
//画坐标对应蛇身
static void DrBody_xy(u8 x,u8 y);
//画坐标对应食物
static void DrFood_xy(u8 x,u8 y);
//清坐标对应蛇身
static void ClBody_xy(u8 x,u8 y);
//读坐标对应方块颜色
static u16 GetColo_xy(u8 x,u8 y);
//清屏
static void ClearLCD(void);
//画游戏背景
static void DrawBg(void);
//蛇复位
static void ResetSnake(psnake ps);
//蛇向左转
static void Turn_left(psnake ps);
//蛇向右转
static void Turn_right(psnake ps);
//蛇保持直行
static void Turn_ahead(psnake ps);
//蛇蠕动
static void Go(psnake ps);
//死亡判断
static u8 IfDie(psnake ps);
//死亡处理
static void Die(psnake ps);
//开始游戏
static void StartGame(void);
//小延时
static void delay(void);
//食物地址生成器
static void GetFood_xy(u8 *x,u8 *y);
//食物生成器
static void CreatFood(pfood pf);
//判断是否吃到食物
static u8 IfEat(psnake ps,pfood pf);
//消化食物
static void digest(psnake ps);
//编辑蛇的速度和大小
static void ModifySnakeSettings(void *OptionsBuf);
//-----------------------本页自定义变量定义-----------------------
typedef struct{
	SqQueue  SNAKE_QUEUE;//定义循环队列指针
	snakedef SNAKE;		//定义蛇类型结构体
	fooddef SNAKEFOOD;	//定义食物类型结构体
	
	u16 CUR_SPEED;				//存储蛇当前速度
	u16 CUR_SIZE;					//存储蛇当前大小
	u16 SNAKE_COLOUR;		//存储蛇当前颜色
	u16 FOOD_COLOUR;		//存储食物当前颜色
	u16 BG_COLOUR;			//存储场地当前颜色
	
	u32 CUR_SPEEDLEV;		//存储蛇当前速度（用作seting）
	u32 CUR_SIZELEV;		//存储蛇当前大小（用作seting）
	u16 SNAKE_COLOURLEV;//存储蛇当前颜色（用作seting）
	u16 FOOD_COLOURLEV;	//存储食物当前颜色（用作seting）
	u16 BG_COLOURLEV;		//存储场地当前颜色（用作seting）

	u8  CtrlFlag;//用来保证在定时器两次触发之间只响应第一次动作
}SNAKE_PAGE_VARS;//将本页要用到的全局变量全部放入此结构体

static SNAKE_PAGE_VARS *gpSpVars;//只需要定义一个指针，减少全局变量的使用
//-----------------------本页自定义函数定义-----------------------
//创建一个循环队列
static void InitQueue(SqQueue *Q){
	Q->base=(u16 *)Q_PageMallco(MAXQSIZE*2);//申请堆内存
	Q->front=0;
	Q->rear=0;
}
//清空循环队列
static void ClearQueue(SqQueue *Q){
	Q->front=0;
	Q->rear=0;
}
//销毁循环队列
static void DestroyQueue(SqQueue *Q){
	Q_PageFree(Q->base);	//释放堆内存
	Q->base=NULL;
	Q->front=0;
	Q->rear=0;
}
//插入元素为队尾元素
static void EnQueue(SqQueue *Q,u16 e){
	Q->base[Q->rear]=e;
	Q->rear=(Q->rear+1)%MAXQSIZE;
}
//获得队头元素并删除队头 
static void DeQueue(SqQueue *Q,u16 *e){
	(*e)=Q->base[Q->front];
	Q->front=(Q->front+1)%MAXQSIZE;
}
//获取循环队列长度
static u16 QueueLength(SqQueue *Q){
	return ( Q->rear-Q->front+MAXQSIZE )%MAXQSIZE;
}
//将一对坐标压入队尾
static void InQ_xy(u8 x,u8 y){
	u16 temp=0;
	temp=y;
	temp|=x<<8;
	EnQueue(&gpSpVars->SNAKE_QUEUE,temp);		
}
//从队头取出一组坐标
static void OutQ_xy(u8 *x,u8 *y){
	u16 temp=0;
	DeQueue(&gpSpVars->SNAKE_QUEUE,&temp);
	*x=temp>>8;
	*y=temp&0xff;
}	
//画坐标对应方块
static void DrBody_xy(u8 x,u8 y){
	GUI_REGION DrawRegion;
	DrawRegion.x=GROUND_W_PIX+gpSpVars->CUR_SIZE*x;
	DrawRegion.y=GROUND_N_PIX+gpSpVars->CUR_SIZE*y;
	DrawRegion.w=gpSpVars->CUR_SIZE;
	DrawRegion.h=gpSpVars->CUR_SIZE;
	DrawRegion.Color=gpSpVars->SNAKE_COLOUR;
	Gui_FillBlock(&DrawRegion);
}
//画坐标对应食物
static void DrFood_xy(u8 x,u8 y){
	GUI_REGION DrawRegion;
	DrawRegion.x=(GROUND_W_PIX+gpSpVars->CUR_SIZE*x)+gpSpVars->CUR_SIZE/2;
	DrawRegion.y=(GROUND_N_PIX+gpSpVars->CUR_SIZE*y)+gpSpVars->CUR_SIZE/2;
	DrawRegion.w=(gpSpVars->CUR_SIZE-1)/2;
	DrawRegion.Color=gpSpVars->FOOD_COLOUR;
	DrawRegion.Space=0xff;
	Gui_DrawCircle(&DrawRegion,(bool)1); 
}
//清坐标对应方块
static void ClBody_xy(u8 x,u8 y){
	GUI_REGION DrawRegion;
	DrawRegion.x=GROUND_W_PIX+gpSpVars->CUR_SIZE*x;
	DrawRegion.y=GROUND_N_PIX+gpSpVars->CUR_SIZE*y;
	DrawRegion.w=gpSpVars->CUR_SIZE;
	DrawRegion.h=gpSpVars->CUR_SIZE;

	DrawRegion.Color=gpSpVars->BG_COLOUR;
	Gui_FillBlock(&DrawRegion);
}
//读坐标对应方块颜色
static u16 GetColo_xy(u8 x,u8 y){
	x=GROUND_W_PIX+gpSpVars->CUR_SIZE*x;
	y=GROUND_N_PIX+gpSpVars->CUR_SIZE*y;
	return Gui_ReadPixel16Bit(x,y);
}
//清屏
static void ClearLCD(void){
	GUI_REGION DrawRegion;
	DrawRegion.x=GROUND_W_PIX;
	DrawRegion.y=GROUND_N_PIX;
	DrawRegion.w=GROUND_E_PIX-GROUND_W_PIX+1;
	DrawRegion.h=GROUND_S_PIX-GROUND_N_PIX+1;
	DrawRegion.Color=WHITE;
	Gui_FillBlock(&DrawRegion);
}
//画游戏背景
static void DrawBg(void){
	GUI_REGION DrawRegion;
	DrawRegion.x=GROUND_W_PIX;
	DrawRegion.y=GROUND_N_PIX;
	DrawRegion.w=GROUND_E_PIX-GROUND_W_PIX+1;
	DrawRegion.h=GROUND_S_PIX-GROUND_N_PIX+1;
	DrawRegion.Color=gpSpVars->BG_COLOUR;
	Gui_FillBlock(&DrawRegion);
}
//复位蛇的所有状态
static void ResetSnake(psnake ps){
	ps->hposx=(EASTWALL-WESTWALL)/2;
	ps->hposy=(SOUTHWALL-NORTHWALL)*2/3;
	ps->tposx=ps->hposx;
	ps->tposy=ps->hposy+1;
	ps->nhposx=ps->hposx;
	ps->nhposy=ps->hposy-1;
	ps->dir=NORTH;
	ps->eatnum=0;
	DrBody_xy(ps->hposx,ps->hposy);
	DrBody_xy(ps->tposx,ps->tposy);
	InQ_xy(ps->hposx,ps->hposy);	
}
//蛇向左转
static void Turn_left(psnake ps){
	switch(ps->dir){
		case EAST:
			ps->nhposy=(ps->hposy)-1;
			ps->dir=NORTH;
			break;
		case SOUTH:
			ps->nhposx=(ps->hposx)+1;
			ps->dir=EAST;
			break;
		case WEST:
			ps->nhposy=(ps->hposy)+1;
			ps->dir=SOUTH;
			break;
		case NORTH:
			ps->nhposx=(ps->hposx)-1;
			ps->dir=WEST;
			break;
	}	
}
//蛇向右转
static void Turn_right(psnake ps){
	switch(ps->dir){
		case EAST:
			ps->nhposy=(ps->hposy)+1;
			ps->dir=SOUTH;
			break;
		case SOUTH:
			ps->nhposx=(ps->hposx)-1;
			ps->dir=WEST;
			break;
		case WEST:
			ps->nhposy=(ps->hposy)-1;
			ps->dir=NORTH;
			break;
		case NORTH:
			ps->nhposx=(ps->hposx)+1;
			ps->dir=EAST;
			break;
	}
}
//蛇保持直行
static void Turn_ahead(psnake ps){
	switch(ps->dir){
		case EAST:
			ps->nhposx=(ps->hposx)+1;
			ps->dir=EAST;
			break;
		case SOUTH:
			ps->nhposy=(ps->hposy)+1;
			ps->dir=SOUTH;
			break;
		case WEST:
			ps->nhposx=(ps->hposx)-1;
			ps->dir=WEST;
			break;
		case NORTH:
			ps->nhposy=(ps->hposy)-1;
			ps->dir=NORTH;
			break;
	}
}
//蛇蠕动
static void Go(psnake ps){	
	u8 x,y;
	DrBody_xy(ps->nhposx,ps->nhposy);
	ClBody_xy(ps->tposx,ps->tposy);
	ps->hposx=ps->nhposx;
	ps->hposy=ps->nhposy;
	InQ_xy(ps->hposx,ps->hposy);
	OutQ_xy(&x,&y);
	ps->tposx=x;
	ps->tposy=y;
}
//死亡判断
static u8 IfDie(psnake ps){
	u16 i,j;
	u8  x,y;
	if(ps->nhposx<WESTWALL||ps->nhposx>EASTWALL||ps->nhposy<NORTHWALL||ps->nhposy>SOUTHWALL)
		return 1;
	j=QueueLength(&gpSpVars->SNAKE_QUEUE);
	for(i=0;i<j;i++){
		 OutQ_xy(&x,&y);
		 InQ_xy(x,y);
		 if( ((ps->nhposx)==x) && ((ps->nhposy)==y) )
		 	return 1;
	}
	return 0;
}
//死亡处理
static void Die(psnake ps){
	u8 buf[5];	
	GUI_REGION DrawRegion;	
	//关定时器
	Q_TimSet(Q_TIM1,0,0,(bool)1);
	//输出game over！
	DrawRegion.x=80;
	DrawRegion.y=60;
	DrawRegion.w=200;
	DrawRegion.h=16;
	DrawRegion.Color=0x0;
	DrawRegion.Space=0x00;
	Gui_DrawFont(GBK16_FONT,"GAME OVER!",&DrawRegion);
	//显示游戏分数	
	DrawRegion.x=55;
	DrawRegion.y=100;
	Gui_DrawFont(GBK16_FONT,"YOUR SCORE:",&DrawRegion);	
	DrawRegion.x=150;
	buf[0]=((ps->eatnum)/1000+0x30);
	buf[1]=((ps->eatnum)%1000/100+0x30);
	buf[2]=((ps->eatnum)%100/10+0x30);
	buf[3]=((ps->eatnum)%10+0x30);
	buf[4]=0;
	Gui_DrawFont(GBK16_FONT,buf,&DrawRegion);
}
//开始游戏
static void StartGame(void){
	//清空队列
	ClearQueue(&gpSpVars->SNAKE_QUEUE);
	//画背景
	DrawBg();
	//复位蛇的所有状态
	ResetSnake(&gpSpVars->SNAKE);
	//生成第一个食物
	CreatFood(&gpSpVars->SNAKEFOOD);
	//开定时器
	Q_TimSet(Q_TIM1,gpSpVars->CUR_SPEED,100,(bool)1);
}
//小延时保证读取的颜色正确
static void delay(){
	u32 i=0x10;
	while(i--);
}
//食物地址生成器
static void GetFood_xy(u8 *x,u8 *y){
	u16 temp;
	u16 curcolo;
	u8 tempx=0,tempy=0;
	while(1){
		curcolo=gpSpVars->SNAKE_COLOUR;
		temp=Rand(0xffff);//产生十六位随机数 
		tempx=(temp>>8)%(EASTWALL-WESTWALL+1);
		tempy=(temp&0xff)%(SOUTHWALL-NORTHWALL+1);
		curcolo=GetColo_xy(tempx,tempy);
		if(curcolo!=gpSpVars->SNAKE_COLOUR)//若生成的食物地址不在蛇身上则采用，否则重取
			break;								  
		else
			delay();
	}
	*x=tempx;
	*y=tempy;
}
//食物生成器
static void CreatFood(pfood pf){
	u8 x,y;
	GetFood_xy(&x,&y);
	DrFood_xy(x,y);
	pf->x=x;
	pf->y=y;
}
//判断是否吃到食物
static u8 IfEat(psnake ps,pfood pf){
	if((ps->nhposx==pf->x) && (ps->nhposy==pf->y))
		return 1;
	else
		return 0;
}
//消化食物 越大越快的蛇得分越多
static void digest(psnake ps){
	DrBody_xy(ps->nhposx,ps->nhposy);
	ps->hposx=ps->nhposx;
	ps->hposy=ps->nhposy;
	InQ_xy(ps->hposx,ps->hposy);
	ps->eatnum+=(gpSpVars->CUR_SIZELEV*gpSpVars->CUR_SPEEDLEV);//加分规则
}
//根据用户的设定调整蛇的速度和大小
static void ModifySnakeSettings(void *OptionsBuf){
	u8 i,id,Total;
	s32 Val;

	if(SP_GetOptionsTotal(OptionsBuf,&Total)==FALSE) return;
	for(i=1;i<=Total;i++)//必须从1开始索引
	{
		SP_GetOptionID(OptionsBuf,i,&id);
		switch(id)
		{
			case SNAKEOP_SPEED:
				if(SP_IsModify(OptionsBuf,i)==FALSE) continue;//未修改
				SP_GetNumOption(OptionsBuf,i,&Val);
				if(Val==1){
					gpSpVars->CUR_SPEED=SLOW;
					gpSpVars->CUR_SPEEDLEV=1;
					break;
				}
				else if(Val==2){
					gpSpVars->CUR_SPEED=SWIFT;
					gpSpVars->CUR_SPEEDLEV=2;
					break;
				}
				else if(Val==3){
				    gpSpVars->CUR_SPEED=LIGHTNING;
					gpSpVars->CUR_SPEEDLEV=3;
					break;
				}
				else
					break;

			case SNAKEOP_SIZE:
				if(SP_IsModify(OptionsBuf,i)==FALSE) continue;//未修改
				SP_GetNumOption(OptionsBuf,i,&Val);
				if(Val==1){
					gpSpVars->CUR_SIZE=SMALL;
					gpSpVars->CUR_SIZELEV=1;
					break;
				}
				else if(Val==2){
					gpSpVars->CUR_SIZE=NORMAL;
					gpSpVars->CUR_SIZELEV=2;
					break;
				}
				else if(Val==3){
				    gpSpVars->CUR_SIZE=BIG;
					gpSpVars->CUR_SIZELEV=3;
					break;
				}
				else
					break;

			   case SNAKEOP_SNAKE_COLOUR:
				if(SP_IsModify(OptionsBuf,i)==FALSE) continue;//未修改
				SP_GetNumOption(OptionsBuf,i,&Val);
				if(Val==1){
					gpSpVars->SNAKE_COLOUR=RED;
					gpSpVars->SNAKE_COLOURLEV=1;
					break;
				}
				else if(Val==2){
					gpSpVars->SNAKE_COLOUR=GREEN;
					gpSpVars->SNAKE_COLOURLEV=2;
					break;
				}
				else if(Val==3){
					gpSpVars->SNAKE_COLOUR=BLUE;
					gpSpVars->SNAKE_COLOURLEV=3;
					break;
				}
				else if(Val==4){
					gpSpVars->SNAKE_COLOUR=ORANGE;
					gpSpVars->SNAKE_COLOURLEV=4;
					break;
				}
				else if(Val==5){
					gpSpVars->SNAKE_COLOUR=PURPLE;
					gpSpVars->SNAKE_COLOURLEV=5;
					break;
				}
				else
					break;

			   case SNAKEOP_FOOD_COLOUR:
				if(SP_IsModify(OptionsBuf,i)==FALSE) continue;//未修改
				SP_GetNumOption(OptionsBuf,i,&Val);
				if(Val==1){
					gpSpVars->FOOD_COLOUR=RED;
					gpSpVars->FOOD_COLOURLEV=1;
					break;
				}
				else if(Val==2){
					gpSpVars->FOOD_COLOUR=GREEN;
					gpSpVars->FOOD_COLOURLEV=2;
					break;
				}
				else if(Val==3){
					gpSpVars->FOOD_COLOUR=BLUE;
					gpSpVars->FOOD_COLOURLEV=3;
					break;
				}
				else if(Val==4){
					gpSpVars->FOOD_COLOUR=ORANGE;
					gpSpVars->FOOD_COLOURLEV=4;
					break;
				}
				else if(Val==5){
					gpSpVars->FOOD_COLOUR=PURPLE;
					gpSpVars->FOOD_COLOURLEV=5;
					break;
				}
				else
					break;

			   case SNAKEOP_BG_COLOUR:
				if(SP_IsModify(OptionsBuf,i)==FALSE) continue;//未修改
				SP_GetNumOption(OptionsBuf,i,&Val);
				if(Val==1){
					gpSpVars->BG_COLOUR=RED;
					gpSpVars->BG_COLOURLEV=1;
					break;
				}
				else if(Val==2){
					gpSpVars->BG_COLOUR=GREEN;
					gpSpVars->BG_COLOURLEV=2;
					break;
				}
				else if(Val==3){
					gpSpVars->BG_COLOUR=BLUE;
					gpSpVars->BG_COLOURLEV=3;
					break;
				}
				else if(Val==4){
					gpSpVars->BG_COLOUR=ORANGE;
					gpSpVars->BG_COLOURLEV=4;
					break;
				}
				else if(Val==5){
					gpSpVars->BG_COLOUR=PURPLE;
					gpSpVars->BG_COLOURLEV=5;
					break;
				}
				else
					break;
		}					
	}
}
//-----------------------本页系统函数声明-------------------------
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam);
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam);
static CO_MSG ButtonHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo);
//-----------------------本页系统变量定义及声明-----------------------
//定义页面按键需要用到的枚举，类似于有序唯一的宏定义
typedef enum{
	ExtiKeyDown=EXTI_KEY_VALUE_START,//系统默认将外部中断按键发送到第一个键值
	ExtiKeyUp,
	ExtiKeyEnter,	
	//后面的硬件版本将支持更多外部中断按键或者无线键盘，
	//所以触摸键值从USER_KEY_VALUE_START开始，将前面的键值都留下来
	BackKV=USER_KEY_VALUE_START,
	LeftArrowKV,
	DotKV,
	RightArrowKV,
	DoneKV,
	HomeKV,
	MessageKV,
	MusicKV,
	PepoleKV,
}SnakePage_OID;
//定义页面或应用的触摸区域集，相当于定义按键
//支持的最大触摸区域个数为MAX_TOUCH_REGION_NUM
//系统显示和触摸的所有坐标系均以屏幕左上角为原点(x 0,y 0)，右下角为(x 320,y 240)
static const IMG_BUTTON_OBJ ImgButtonCon[]={
	//KeyName,ObjID,OptionMask,Tch_x,Tch_y,Tch_w,Tch_h,Img_x,Img_y,BmpPathPrefix,NormalSuf,PressSuf,ReleaseSuf,TransColor},
	{"Back",	BackKV,RelMsk|PathMsk,3,287,54,31,0,0,"Common/Btn/Back",FatColor(NO_TRANS)},
	{"<<",		LeftArrowKV,RelMsk|PathMsk,65,287,39,31,0,0,"Common/Btn/LeftArr",FatColor(NO_TRANS)},
	{"Dot",		DotKV,RelMsk|PathMsk,109,292,22,22,0,0,"Common/Btn/Dot",FatColor(NO_TRANS)},
	{">>",		RightArrowKV,RelMsk|PathMsk,136,287,39,31,0,0,"Common/Btn/RightArr",FatColor(NO_TRANS)},
	{"Done",	DoneKV,RelMsk|PathMsk,183,287,54,31,0,0,"Common/Btn/Done",FatColor(NO_TRANS)},
	
	//液晶屏下面非显示区域的四个键
	{"",HomeKV,RelMsk,0,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",MessageKV,RelMsk,60,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",MusicKV,RelMsk,120,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",PepoleKV,RelMsk,180,320,60,30,0,0,"",FatColor(NO_TRANS)},
};
//定义页面或者应用的属性集
const PAGE_ATTRIBUTE SnakePage={
	"SnakePage",
	"Chenyu",
	"greedy snake 1.0",
	NORMAL_PAGE,
	0,//
	{
		sizeof(ImgButtonCon)/sizeof(IMG_BUTTON_OBJ), //size of touch region array
		0,//sizeof(CharButtonCon)/sizeof(CHAR_BUTTON_OBJ), //size of touch region array,
	},
	ImgButtonCon, //touch region array
	0,	
	SystemEventHandler,
	PeripheralsHandler,
	Bit(Perip_Timer),
	ButtonHandler,
};
//-----------------------本页系统函数定义--------------------------
//发生某些事件时，会触发的函数
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam){
	GUI_REGION DrawRegion;	
	switch(SysEvent)
	{
		case Sys_PreGotoPage:
			break;
		case Sys_PageInit:		//系统每次打开这个页面，会处理这个事件
		
			gpSpVars=(SNAKE_PAGE_VARS *)Q_PageMallco(sizeof(SNAKE_PAGE_VARS));//申请空间	
			
			if(gpSpVars==0)
			{
				Q_ErrorStopScreen("gpSpVars malloc fail !\n\r");
			}
						
			//全局变量初始化
			gpSpVars->CUR_SPEED=DEFA_SPEED,				//存储蛇当前速度
			gpSpVars->CUR_SIZE=DEFA_SIZE,					//存储蛇当前大小
			gpSpVars->SNAKE_COLOUR=DEFA_SNAKE_COLOUR,		//存储蛇当前颜色
			gpSpVars->FOOD_COLOUR=DEFA_FOOD_COLOUR,		//存储食物当前颜色
			gpSpVars->BG_COLOUR=DEFA_BG_COLOUR,			//存储场地当前颜色
	
			gpSpVars->CUR_SPEEDLEV=DEFA_CUR_SPEEDLEV,		//存储蛇当前速度（用作seting）
			gpSpVars->CUR_SIZELEV=DEFA_CUR_SIZELEV,		//存储蛇当前大小（用作seting）
			gpSpVars->SNAKE_COLOURLEV=DEFA_SNAKE_COLOURLEV,//存储蛇当前颜色（用作seting）
			gpSpVars->FOOD_COLOURLEV=DEFA_FOOD_COLOURLEV,	//存储食物当前颜色（用作seting）
			gpSpVars->BG_COLOURLEV=DEFA_BG_COLOURLEV,		//存储场地当前颜色（用作seting）

			gpSpVars->CtrlFlag=0;//用来保证在定时器两次触发之间只响应第一次动作

			//画标题栏
			DrawRegion.x=DrawRegion.y=0;
			DrawRegion.w=240;
			DrawRegion.h=21;
			DrawRegion.Color=FatColor(NO_TRANS);
			Gui_FillImgArray((u8 *)gImage_StatusBar1,1,21,&DrawRegion);	
			DrawTitle1(ASC14B_FONT,"SnakePage",(240-strlen("SnakePage")*GUI_ASC14B_ASCII_WIDTH)>>1,strlen("SnakePage"),FatColor(0xe0e0e0));//写标题
			
			//清屏
			ClearLCD();
			
			//画游戏启动画面
			DrawRegion.x=75;
			DrawRegion.y=25;
			DrawRegion.w=89;
			DrawRegion.h=139;
			DrawRegion.Color=FatColor(NO_TRANS);
			Gui_DrawImgBin("Theme/F/SnakePage/Bg/snakebg.bin",&DrawRegion);		

			//输出游戏提示
			DrawRegion.x=35;
			DrawRegion.y=170;
			DrawRegion.w=200;
			DrawRegion.h=16;
			DrawRegion.Color=0x0;
			DrawRegion.Space=0x00;
			Gui_DrawFont(GBK16_FONT,"BACK: 返回应用列表",&DrawRegion);
			DrawRegion.y+=16;
			Gui_DrawFont(GBK16_FONT,"<<  : 蛇向左转",&DrawRegion);
			DrawRegion.y+=16;
			Gui_DrawFont(GBK16_FONT,">>  : 蛇向右转",&DrawRegion);
			DrawRegion.y+=16;
			Gui_DrawFont(GBK16_FONT,"DOT : 游戏设置",&DrawRegion);
			DrawRegion.y+=16;			
			Gui_DrawFont(GBK16_FONT,"DONE: 开始或重新开始",&DrawRegion);
			DrawRegion.y+=16;			
			Gui_DrawFont(GBK16_FONT,"规则: 得分=蛇大小*蛇速度",&DrawRegion);
			
			//画底栏
			DrawRegion.x=0;
			DrawRegion.y=320-39;
			DrawRegion.w=240;
			DrawRegion.h=39;
			DrawRegion.Color=FatColor(NO_TRANS);
			Gui_FillImgArray((u8 *)gImage_BottomBar1,1,39,&DrawRegion);
			
			//第一次初始化循环队列
			InitQueue(&gpSpVars->SNAKE_QUEUE);
			break;						  

		case Sys_SubPageReturn:	//如果从子页面返回,就不会触发Sys_Page_Init事件,而是Sys_SubPage_Return
			if(strcmp((void *)Q_GetPageByTrack(1)->Name,"SettingsPage")==0)//从设置页面返回
			{
				if(IntParam!=0)//设置页面里修改了任意值
				{
					switch(IntParam)
					{
						case OSID_SNAKE:
							ModifySnakeSettings(pSysParam);
							break;
					}
				}
				Q_PageFree(pSysParam);//从设置页面返回必须释放当初进入时分配的内存，否则会造成泄漏
			}						
			DrawBg();//画背景			
			ResetSnake(&gpSpVars->SNAKE);////复位蛇的所有状态
			DestroyQueue(&gpSpVars->SNAKE_QUEUE);//销毁循环队列并释放堆内存				
			InitQueue(&gpSpVars->SNAKE_QUEUE);//重新初始化循环队列			
			break;

		case Sys_PageClean:
		
			//释放页面资源
			Q_PageFree(gpSpVars);
			DestroyQueue(&gpSpVars->SNAKE_QUEUE);//销毁循环队列并释放堆内存			
			Q_TimSet(Q_TIM1,0,0,(bool)1);//tm2停止计数
			
			break;

		default:
			//需要响应的事件未定义
			Debug("%s SystemEventHandler:This System Event Handler case unfinish! SysEvent:%d\n\r",Q_GetCurrPageName(),SysEvent);
	}	
	return 0;
}
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam)
{
	switch(PeripEvent)
	{
		case Perip_KeyPress:
			switch(IntParam){
				case ExtiKeyEnter:
					break;
				case ExtiKeyUp:
					Q_PresentTch(LeftArrowKV,Tch_Press);
					break;
				case ExtiKeyDown:
					Q_PresentTch(RightArrowKV,Tch_Press);
					break; 
			}break;
		case Perip_KeyRelease:
			switch(IntParam){
				case ExtiKeyEnter:
					PrtScreen();
					break;
				case ExtiKeyUp:
				    Q_PresentTch(LeftArrowKV,Tch_Release);
					if(gpSpVars->CtrlFlag==0)
						Turn_left(&gpSpVars->SNAKE);
					gpSpVars->CtrlFlag=1;
					break;
				case ExtiKeyDown:
					Q_PresentTch(RightArrowKV,Tch_Release);
					if(gpSpVars->CtrlFlag==0)
						Turn_right(&gpSpVars->SNAKE);
					gpSpVars->CtrlFlag=1;
					break; 
			}break;

		case Perip_Timer://游戏的脉搏
		
			 //若没有左转或右转则保持前行
			 if(gpSpVars->CtrlFlag==0)
				Turn_ahead(&gpSpVars->SNAKE);
			 gpSpVars->CtrlFlag=0;
			
			 //若将死亡则进行死亡处理
			 if(IfDie(&gpSpVars->SNAKE))
			 	Die(&gpSpVars->SNAKE);
			 
			 //若吃到食物则进行消化
			 else if(IfEat(&gpSpVars->SNAKE,&gpSpVars->SNAKEFOOD)){
			 	digest(&gpSpVars->SNAKE);
			 	CreatFood(&gpSpVars->SNAKEFOOD);
			 }
			 
			 //若下一步不会死，而且不会吃到食物则蠕动一下
			 else	
			 	Go(&gpSpVars->SNAKE);	
				
			break;
			
	}

	return 0;
}
//当使用者按下本页TouchRegionSet里定义的按键时，会触发这个函数里的对应事件
static CO_MSG ButtonHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo){		
	switch(Key){	
		case BackKV:
			Q_GotoPage(GotoNewPage,"AppListPage",0,NULL);			
			break;
		case DoneKV:
				StartGame();//开始游戏
			break;
		
		case LeftArrowKV:
			//蛇向左转
			if(gpSpVars->CtrlFlag==0)
				Turn_left(&gpSpVars->SNAKE);
			gpSpVars->CtrlFlag=1;
			break;
		case DotKV:{
				void *p;
				Q_TimSet(Q_TIM1,0,0,(bool)1);//tm2停止计数
				p=Q_PageMallco(1000);
				SP_AddOptionsHeader(p,1000,"Set Snake",OSID_SNAKE);	
				SP_AddNumListOption(p,SNAKEOP_SPEED,gpSpVars->CUR_SPEEDLEV,1,3,1,"选择蛇的速度","1：慢蛇 2：快蛇 3：闪电蛇");
				SP_AddNumListOption(p,SNAKEOP_SIZE,gpSpVars->CUR_SIZELEV,1,3,1,"选择蛇的大小","1：小蛇 2：中蛇 3：大蛇");
				SP_AddNumListOption(p,SNAKEOP_SNAKE_COLOUR,gpSpVars->SNAKE_COLOURLEV,1,5,1,"选择蛇的颜色","1红 2绿 3蓝 4橙 5紫");
				SP_AddNumListOption(p,SNAKEOP_FOOD_COLOUR,gpSpVars->FOOD_COLOURLEV,1,5,1,"选择食物的颜色","1红 2绿 3蓝 4橙 5紫");
				SP_AddNumListOption(p,SNAKEOP_BG_COLOUR,gpSpVars->BG_COLOURLEV,1,5,1,"选择场地的颜色","1红 2绿 3蓝 4橙 5紫");
				Q_GotoPage(GotoSubPage,"SettingsPage",TRUE,p);				
			}			
			break;	
		case RightArrowKV:
		
			//蛇向右转
			if(gpSpVars->CtrlFlag==0)
				Turn_right(&gpSpVars->SNAKE);
			gpSpVars->CtrlFlag=1;
			break;
		case HomeKV:
		case MessageKV:
		case MusicKV:
		case PepoleKV:
			break;
		default:
			//需要响应的事件未定义
			Debug("%s ButtonHandler:This Touch Event Handler case unfinish! Key:%d\n\r",Q_GetCurrPageName(),Key);
	}	
	return 0;
}


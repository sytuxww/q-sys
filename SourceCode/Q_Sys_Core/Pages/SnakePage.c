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
//---------------------��ҳ�Զ������ͼ��궨��------------------
//��Ϸɫ��
#define WHITE			(FatColor(0xFFFFFF))
#define BLUE			(FatColor(0x0000FF))
#define	GREEN			(FatColor(0x00FF00))
#define RED				(FatColor(0xFF0000))
#define	PURPLE			(FatColor(0xDB70DB))
#define	ORANGE			(FatColor(0xE47833))
#define BLACK			(FatColor(0x000000))

//Ĭ����ɫ
#define DEFA_SNAKE_COLOURLEV 3
#define DEFA_FOOD_COLOURLEV	 1
#define DEFA_BG_COLOURLEV	 2 
#define DEFA_SNAKE_COLOUR	BLUE
#define DEFA_FOOD_COLOUR	RED
#define DEFA_BG_COLOUR		GREEN 
 
//�ߵĿ���ѡ��
#define SLOW			6000
#define SWIFT			3000
#define LIGHTNING		1500

//�ߵĴ�Сѡ��
#define SMALL			5
#define NORMAL			10
#define BIG				20

//Ĭ�ϴ�С����״
#define DEFA_CUR_SPEEDLEV	2
#define DEFA_CUR_SIZELEV	2
#define DEFA_SPEED			SWIFT
#define DEFA_SIZE			NORMAL

//���ر߽������
#define GROUND_W_PIX	0
#define GROUND_E_PIX	239
#define GROUND_N_PIX	21
#define GROUND_S_PIX	280
//���ر߽������	
#define WESTWALL		0
#define EASTWALL		(((GROUND_E_PIX-GROUND_W_PIX+1)/gpSpVars->CUR_SIZE)-1)
#define NORTHWALL		0
#define SOUTHWALL		(((GROUND_S_PIX-GROUND_N_PIX+1)/gpSpVars->CUR_SIZE)-1)
//ѭ�����������豣���Ԫ����Ŀ
#define MAXQSIZE		((EASTWALL-WESTWALL+1)*(SOUTHWALL-NORTHWALL+1))	

typedef enum{ //���Է��� ��Ļ�Ϸ���ӦNOUTH
	EAST, 
	SOUTH,
	WEST,
	NORTH
}SnakeDir;	

typedef struct{	 //ѭ��������������
	u16  *base;
	u16  front;
	u16  rear;
}SqQueue;          
    
typedef struct{  //�ߵĽṹ������
	s8 hposx;	 //��ǰ��ͷ����
	s8 hposy;	 
	s8 tposx;	 //��ǰ��β����
	s8 tposy;	 
	s8 nhposx;	 //Ԥ���´ε���ͷ����
	s8 nhposy;	 
	SnakeDir dir;//��ǰ��ͷ���˶�����
	u16 eatnum;	 //��ǰ�÷�
}snakedef,*psnake;

//ע������ǰ����ָ���´ζ�ʱ����ǰ�����ʱ��
//��Ԥ�ơ��Ǹ��ݵ�ǰ��ͷ������ͷ����ƶ�(�����Ƿ�������Ե�ʳ��)�����´���ͷ��λ��
typedef struct{  //ʳ����������
	u8 x;		 //ʳ������
	u8 y;
}fooddef,*pfood; 

//��Ҫsetting����Ŀ
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
//----------------------��ҳ�Զ��庯������---------------------
//����ѭ������
static void InitQueue(SqQueue *Q);
//����ѭ������
static void DestroyQueue(SqQueue *Q);
//���ѭ������
static void ClearQueue(SqQueue *Q);
//����Ԫ��Ϊ��βԪ��
static void EnQueue(SqQueue *Q,u16 e);
//��ö�ͷԪ�ز�ɾ����ͷ 
static void DeQueue(SqQueue *Q,u16 *e);
//��ȡ���г���
static u16 QueueLength(SqQueue *Q);
//��һ������ѹ���β
static void InQ_xy(u8 x,u8 y);
//�Ӷ�ͷȡ��һ������
static void OutQ_xy(u8 *x,u8 *y);
//�������Ӧ����
static void DrBody_xy(u8 x,u8 y);
//�������Ӧʳ��
static void DrFood_xy(u8 x,u8 y);
//�������Ӧ����
static void ClBody_xy(u8 x,u8 y);
//�������Ӧ������ɫ
static u16 GetColo_xy(u8 x,u8 y);
//����
static void ClearLCD(void);
//����Ϸ����
static void DrawBg(void);
//�߸�λ
static void ResetSnake(psnake ps);
//������ת
static void Turn_left(psnake ps);
//������ת
static void Turn_right(psnake ps);
//�߱���ֱ��
static void Turn_ahead(psnake ps);
//���䶯
static void Go(psnake ps);
//�����ж�
static u8 IfDie(psnake ps);
//��������
static void Die(psnake ps);
//��ʼ��Ϸ
static void StartGame(void);
//С��ʱ
static void delay(void);
//ʳ���ַ������
static void GetFood_xy(u8 *x,u8 *y);
//ʳ��������
static void CreatFood(pfood pf);
//�ж��Ƿ�Ե�ʳ��
static u8 IfEat(psnake ps,pfood pf);
//����ʳ��
static void digest(psnake ps);
//�༭�ߵ��ٶȺʹ�С
static void ModifySnakeSettings(void *OptionsBuf);
//-----------------------��ҳ�Զ����������-----------------------
typedef struct{
	SqQueue  SNAKE_QUEUE;//����ѭ������ָ��
	snakedef SNAKE;		//���������ͽṹ��
	fooddef SNAKEFOOD;	//����ʳ�����ͽṹ��
	
	u16 CUR_SPEED;				//�洢�ߵ�ǰ�ٶ�
	u16 CUR_SIZE;					//�洢�ߵ�ǰ��С
	u16 SNAKE_COLOUR;		//�洢�ߵ�ǰ��ɫ
	u16 FOOD_COLOUR;		//�洢ʳ�ﵱǰ��ɫ
	u16 BG_COLOUR;			//�洢���ص�ǰ��ɫ
	
	u32 CUR_SPEEDLEV;		//�洢�ߵ�ǰ�ٶȣ�����seting��
	u32 CUR_SIZELEV;		//�洢�ߵ�ǰ��С������seting��
	u16 SNAKE_COLOURLEV;//�洢�ߵ�ǰ��ɫ������seting��
	u16 FOOD_COLOURLEV;	//�洢ʳ�ﵱǰ��ɫ������seting��
	u16 BG_COLOURLEV;		//�洢���ص�ǰ��ɫ������seting��

	u8  CtrlFlag;//������֤�ڶ�ʱ�����δ���֮��ֻ��Ӧ��һ�ζ���
}SNAKE_PAGE_VARS;//����ҳҪ�õ���ȫ�ֱ���ȫ������˽ṹ��

static SNAKE_PAGE_VARS *gpSpVars;//ֻ��Ҫ����һ��ָ�룬����ȫ�ֱ�����ʹ��
//-----------------------��ҳ�Զ��庯������-----------------------
//����һ��ѭ������
static void InitQueue(SqQueue *Q){
	Q->base=(u16 *)Q_PageMallco(MAXQSIZE*2);//������ڴ�
	Q->front=0;
	Q->rear=0;
}
//���ѭ������
static void ClearQueue(SqQueue *Q){
	Q->front=0;
	Q->rear=0;
}
//����ѭ������
static void DestroyQueue(SqQueue *Q){
	Q_PageFree(Q->base);	//�ͷŶ��ڴ�
	Q->base=NULL;
	Q->front=0;
	Q->rear=0;
}
//����Ԫ��Ϊ��βԪ��
static void EnQueue(SqQueue *Q,u16 e){
	Q->base[Q->rear]=e;
	Q->rear=(Q->rear+1)%MAXQSIZE;
}
//��ö�ͷԪ�ز�ɾ����ͷ 
static void DeQueue(SqQueue *Q,u16 *e){
	(*e)=Q->base[Q->front];
	Q->front=(Q->front+1)%MAXQSIZE;
}
//��ȡѭ�����г���
static u16 QueueLength(SqQueue *Q){
	return ( Q->rear-Q->front+MAXQSIZE )%MAXQSIZE;
}
//��һ������ѹ���β
static void InQ_xy(u8 x,u8 y){
	u16 temp=0;
	temp=y;
	temp|=x<<8;
	EnQueue(&gpSpVars->SNAKE_QUEUE,temp);		
}
//�Ӷ�ͷȡ��һ������
static void OutQ_xy(u8 *x,u8 *y){
	u16 temp=0;
	DeQueue(&gpSpVars->SNAKE_QUEUE,&temp);
	*x=temp>>8;
	*y=temp&0xff;
}	
//�������Ӧ����
static void DrBody_xy(u8 x,u8 y){
	GUI_REGION DrawRegion;
	DrawRegion.x=GROUND_W_PIX+gpSpVars->CUR_SIZE*x;
	DrawRegion.y=GROUND_N_PIX+gpSpVars->CUR_SIZE*y;
	DrawRegion.w=gpSpVars->CUR_SIZE;
	DrawRegion.h=gpSpVars->CUR_SIZE;
	DrawRegion.Color=gpSpVars->SNAKE_COLOUR;
	Gui_FillBlock(&DrawRegion);
}
//�������Ӧʳ��
static void DrFood_xy(u8 x,u8 y){
	GUI_REGION DrawRegion;
	DrawRegion.x=(GROUND_W_PIX+gpSpVars->CUR_SIZE*x)+gpSpVars->CUR_SIZE/2;
	DrawRegion.y=(GROUND_N_PIX+gpSpVars->CUR_SIZE*y)+gpSpVars->CUR_SIZE/2;
	DrawRegion.w=(gpSpVars->CUR_SIZE-1)/2;
	DrawRegion.Color=gpSpVars->FOOD_COLOUR;
	DrawRegion.Space=0xff;
	Gui_DrawCircle(&DrawRegion,(bool)1); 
}
//�������Ӧ����
static void ClBody_xy(u8 x,u8 y){
	GUI_REGION DrawRegion;
	DrawRegion.x=GROUND_W_PIX+gpSpVars->CUR_SIZE*x;
	DrawRegion.y=GROUND_N_PIX+gpSpVars->CUR_SIZE*y;
	DrawRegion.w=gpSpVars->CUR_SIZE;
	DrawRegion.h=gpSpVars->CUR_SIZE;

	DrawRegion.Color=gpSpVars->BG_COLOUR;
	Gui_FillBlock(&DrawRegion);
}
//�������Ӧ������ɫ
static u16 GetColo_xy(u8 x,u8 y){
	x=GROUND_W_PIX+gpSpVars->CUR_SIZE*x;
	y=GROUND_N_PIX+gpSpVars->CUR_SIZE*y;
	return Gui_ReadPixel16Bit(x,y);
}
//����
static void ClearLCD(void){
	GUI_REGION DrawRegion;
	DrawRegion.x=GROUND_W_PIX;
	DrawRegion.y=GROUND_N_PIX;
	DrawRegion.w=GROUND_E_PIX-GROUND_W_PIX+1;
	DrawRegion.h=GROUND_S_PIX-GROUND_N_PIX+1;
	DrawRegion.Color=WHITE;
	Gui_FillBlock(&DrawRegion);
}
//����Ϸ����
static void DrawBg(void){
	GUI_REGION DrawRegion;
	DrawRegion.x=GROUND_W_PIX;
	DrawRegion.y=GROUND_N_PIX;
	DrawRegion.w=GROUND_E_PIX-GROUND_W_PIX+1;
	DrawRegion.h=GROUND_S_PIX-GROUND_N_PIX+1;
	DrawRegion.Color=gpSpVars->BG_COLOUR;
	Gui_FillBlock(&DrawRegion);
}
//��λ�ߵ�����״̬
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
//������ת
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
//������ת
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
//�߱���ֱ��
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
//���䶯
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
//�����ж�
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
//��������
static void Die(psnake ps){
	u8 buf[5];	
	GUI_REGION DrawRegion;	
	//�ض�ʱ��
	Q_TimSet(Q_TIM1,0,0,(bool)1);
	//���game over��
	DrawRegion.x=80;
	DrawRegion.y=60;
	DrawRegion.w=200;
	DrawRegion.h=16;
	DrawRegion.Color=0x0;
	DrawRegion.Space=0x00;
	Gui_DrawFont(GBK16_FONT,"GAME OVER!",&DrawRegion);
	//��ʾ��Ϸ����	
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
//��ʼ��Ϸ
static void StartGame(void){
	//��ն���
	ClearQueue(&gpSpVars->SNAKE_QUEUE);
	//������
	DrawBg();
	//��λ�ߵ�����״̬
	ResetSnake(&gpSpVars->SNAKE);
	//���ɵ�һ��ʳ��
	CreatFood(&gpSpVars->SNAKEFOOD);
	//����ʱ��
	Q_TimSet(Q_TIM1,gpSpVars->CUR_SPEED,100,(bool)1);
}
//С��ʱ��֤��ȡ����ɫ��ȷ
static void delay(){
	u32 i=0x10;
	while(i--);
}
//ʳ���ַ������
static void GetFood_xy(u8 *x,u8 *y){
	u16 temp;
	u16 curcolo;
	u8 tempx=0,tempy=0;
	while(1){
		curcolo=gpSpVars->SNAKE_COLOUR;
		temp=Rand(0xffff);//����ʮ��λ����� 
		tempx=(temp>>8)%(EASTWALL-WESTWALL+1);
		tempy=(temp&0xff)%(SOUTHWALL-NORTHWALL+1);
		curcolo=GetColo_xy(tempx,tempy);
		if(curcolo!=gpSpVars->SNAKE_COLOUR)//�����ɵ�ʳ���ַ��������������ã�������ȡ
			break;								  
		else
			delay();
	}
	*x=tempx;
	*y=tempy;
}
//ʳ��������
static void CreatFood(pfood pf){
	u8 x,y;
	GetFood_xy(&x,&y);
	DrFood_xy(x,y);
	pf->x=x;
	pf->y=y;
}
//�ж��Ƿ�Ե�ʳ��
static u8 IfEat(psnake ps,pfood pf){
	if((ps->nhposx==pf->x) && (ps->nhposy==pf->y))
		return 1;
	else
		return 0;
}
//����ʳ�� Խ��Խ����ߵ÷�Խ��
static void digest(psnake ps){
	DrBody_xy(ps->nhposx,ps->nhposy);
	ps->hposx=ps->nhposx;
	ps->hposy=ps->nhposy;
	InQ_xy(ps->hposx,ps->hposy);
	ps->eatnum+=(gpSpVars->CUR_SIZELEV*gpSpVars->CUR_SPEEDLEV);//�ӷֹ���
}
//�����û����趨�����ߵ��ٶȺʹ�С
static void ModifySnakeSettings(void *OptionsBuf){
	u8 i,id,Total;
	s32 Val;

	if(SP_GetOptionsTotal(OptionsBuf,&Total)==FALSE) return;
	for(i=1;i<=Total;i++)//�����1��ʼ����
	{
		SP_GetOptionID(OptionsBuf,i,&id);
		switch(id)
		{
			case SNAKEOP_SPEED:
				if(SP_IsModify(OptionsBuf,i)==FALSE) continue;//δ�޸�
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
				if(SP_IsModify(OptionsBuf,i)==FALSE) continue;//δ�޸�
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
				if(SP_IsModify(OptionsBuf,i)==FALSE) continue;//δ�޸�
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
				if(SP_IsModify(OptionsBuf,i)==FALSE) continue;//δ�޸�
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
				if(SP_IsModify(OptionsBuf,i)==FALSE) continue;//δ�޸�
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
//-----------------------��ҳϵͳ��������-------------------------
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam);
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam);
static CO_MSG ButtonHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo);
//-----------------------��ҳϵͳ�������弰����-----------------------
//����ҳ�水����Ҫ�õ���ö�٣�����������Ψһ�ĺ궨��
typedef enum{
	ExtiKeyDown=EXTI_KEY_VALUE_START,//ϵͳĬ�Ͻ��ⲿ�жϰ������͵���һ����ֵ
	ExtiKeyUp,
	ExtiKeyEnter,	
	//�����Ӳ���汾��֧�ָ����ⲿ�жϰ����������߼��̣�
	//���Դ�����ֵ��USER_KEY_VALUE_START��ʼ����ǰ��ļ�ֵ��������
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
//����ҳ���Ӧ�õĴ������򼯣��൱�ڶ��尴��
//֧�ֵ�������������ΪMAX_TOUCH_REGION_NUM
//ϵͳ��ʾ�ʹ�������������ϵ������Ļ���Ͻ�Ϊԭ��(x 0,y 0)�����½�Ϊ(x 320,y 240)
static const IMG_BUTTON_OBJ ImgButtonCon[]={
	//KeyName,ObjID,OptionMask,Tch_x,Tch_y,Tch_w,Tch_h,Img_x,Img_y,BmpPathPrefix,NormalSuf,PressSuf,ReleaseSuf,TransColor},
	{"Back",	BackKV,RelMsk|PathMsk,3,287,54,31,0,0,"Common/Btn/Back",FatColor(NO_TRANS)},
	{"<<",		LeftArrowKV,RelMsk|PathMsk,65,287,39,31,0,0,"Common/Btn/LeftArr",FatColor(NO_TRANS)},
	{"Dot",		DotKV,RelMsk|PathMsk,109,292,22,22,0,0,"Common/Btn/Dot",FatColor(NO_TRANS)},
	{">>",		RightArrowKV,RelMsk|PathMsk,136,287,39,31,0,0,"Common/Btn/RightArr",FatColor(NO_TRANS)},
	{"Done",	DoneKV,RelMsk|PathMsk,183,287,54,31,0,0,"Common/Btn/Done",FatColor(NO_TRANS)},
	
	//Һ�����������ʾ������ĸ���
	{"",HomeKV,RelMsk,0,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",MessageKV,RelMsk,60,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",MusicKV,RelMsk,120,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",PepoleKV,RelMsk,180,320,60,30,0,0,"",FatColor(NO_TRANS)},
};
//����ҳ�����Ӧ�õ����Լ�
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
//-----------------------��ҳϵͳ��������--------------------------
//����ĳЩ�¼�ʱ���ᴥ���ĺ���
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam){
	GUI_REGION DrawRegion;	
	switch(SysEvent)
	{
		case Sys_PreGotoPage:
			break;
		case Sys_PageInit:		//ϵͳÿ�δ����ҳ�棬�ᴦ������¼�
		
			gpSpVars=(SNAKE_PAGE_VARS *)Q_PageMallco(sizeof(SNAKE_PAGE_VARS));//����ռ�	
			
			if(gpSpVars==0)
			{
				Q_ErrorStopScreen("gpSpVars malloc fail !\n\r");
			}
						
			//ȫ�ֱ�����ʼ��
			gpSpVars->CUR_SPEED=DEFA_SPEED,				//�洢�ߵ�ǰ�ٶ�
			gpSpVars->CUR_SIZE=DEFA_SIZE,					//�洢�ߵ�ǰ��С
			gpSpVars->SNAKE_COLOUR=DEFA_SNAKE_COLOUR,		//�洢�ߵ�ǰ��ɫ
			gpSpVars->FOOD_COLOUR=DEFA_FOOD_COLOUR,		//�洢ʳ�ﵱǰ��ɫ
			gpSpVars->BG_COLOUR=DEFA_BG_COLOUR,			//�洢���ص�ǰ��ɫ
	
			gpSpVars->CUR_SPEEDLEV=DEFA_CUR_SPEEDLEV,		//�洢�ߵ�ǰ�ٶȣ�����seting��
			gpSpVars->CUR_SIZELEV=DEFA_CUR_SIZELEV,		//�洢�ߵ�ǰ��С������seting��
			gpSpVars->SNAKE_COLOURLEV=DEFA_SNAKE_COLOURLEV,//�洢�ߵ�ǰ��ɫ������seting��
			gpSpVars->FOOD_COLOURLEV=DEFA_FOOD_COLOURLEV,	//�洢ʳ�ﵱǰ��ɫ������seting��
			gpSpVars->BG_COLOURLEV=DEFA_BG_COLOURLEV,		//�洢���ص�ǰ��ɫ������seting��

			gpSpVars->CtrlFlag=0;//������֤�ڶ�ʱ�����δ���֮��ֻ��Ӧ��һ�ζ���

			//��������
			DrawRegion.x=DrawRegion.y=0;
			DrawRegion.w=240;
			DrawRegion.h=21;
			DrawRegion.Color=FatColor(NO_TRANS);
			Gui_FillImgArray((u8 *)gImage_StatusBar1,1,21,&DrawRegion);	
			DrawTitle1(ASC14B_FONT,"SnakePage",(240-strlen("SnakePage")*GUI_ASC14B_ASCII_WIDTH)>>1,strlen("SnakePage"),FatColor(0xe0e0e0));//д����
			
			//����
			ClearLCD();
			
			//����Ϸ��������
			DrawRegion.x=75;
			DrawRegion.y=25;
			DrawRegion.w=89;
			DrawRegion.h=139;
			DrawRegion.Color=FatColor(NO_TRANS);
			Gui_DrawImgBin("Theme/F/SnakePage/Bg/snakebg.bin",&DrawRegion);		

			//�����Ϸ��ʾ
			DrawRegion.x=35;
			DrawRegion.y=170;
			DrawRegion.w=200;
			DrawRegion.h=16;
			DrawRegion.Color=0x0;
			DrawRegion.Space=0x00;
			Gui_DrawFont(GBK16_FONT,"BACK: ����Ӧ���б�",&DrawRegion);
			DrawRegion.y+=16;
			Gui_DrawFont(GBK16_FONT,"<<  : ������ת",&DrawRegion);
			DrawRegion.y+=16;
			Gui_DrawFont(GBK16_FONT,">>  : ������ת",&DrawRegion);
			DrawRegion.y+=16;
			Gui_DrawFont(GBK16_FONT,"DOT : ��Ϸ����",&DrawRegion);
			DrawRegion.y+=16;			
			Gui_DrawFont(GBK16_FONT,"DONE: ��ʼ�����¿�ʼ",&DrawRegion);
			DrawRegion.y+=16;			
			Gui_DrawFont(GBK16_FONT,"����: �÷�=�ߴ�С*���ٶ�",&DrawRegion);
			
			//������
			DrawRegion.x=0;
			DrawRegion.y=320-39;
			DrawRegion.w=240;
			DrawRegion.h=39;
			DrawRegion.Color=FatColor(NO_TRANS);
			Gui_FillImgArray((u8 *)gImage_BottomBar1,1,39,&DrawRegion);
			
			//��һ�γ�ʼ��ѭ������
			InitQueue(&gpSpVars->SNAKE_QUEUE);
			break;						  

		case Sys_SubPageReturn:	//�������ҳ�淵��,�Ͳ��ᴥ��Sys_Page_Init�¼�,����Sys_SubPage_Return
			if(strcmp((void *)Q_GetPageByTrack(1)->Name,"SettingsPage")==0)//������ҳ�淵��
			{
				if(IntParam!=0)//����ҳ�����޸�������ֵ
				{
					switch(IntParam)
					{
						case OSID_SNAKE:
							ModifySnakeSettings(pSysParam);
							break;
					}
				}
				Q_PageFree(pSysParam);//������ҳ�淵�ر����ͷŵ�������ʱ������ڴ棬��������й©
			}						
			DrawBg();//������			
			ResetSnake(&gpSpVars->SNAKE);////��λ�ߵ�����״̬
			DestroyQueue(&gpSpVars->SNAKE_QUEUE);//����ѭ�����в��ͷŶ��ڴ�				
			InitQueue(&gpSpVars->SNAKE_QUEUE);//���³�ʼ��ѭ������			
			break;

		case Sys_PageClean:
		
			//�ͷ�ҳ����Դ
			Q_PageFree(gpSpVars);
			DestroyQueue(&gpSpVars->SNAKE_QUEUE);//����ѭ�����в��ͷŶ��ڴ�			
			Q_TimSet(Q_TIM1,0,0,(bool)1);//tm2ֹͣ����
			
			break;

		default:
			//��Ҫ��Ӧ���¼�δ����
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

		case Perip_Timer://��Ϸ������
		
			 //��û����ת����ת�򱣳�ǰ��
			 if(gpSpVars->CtrlFlag==0)
				Turn_ahead(&gpSpVars->SNAKE);
			 gpSpVars->CtrlFlag=0;
			
			 //���������������������
			 if(IfDie(&gpSpVars->SNAKE))
			 	Die(&gpSpVars->SNAKE);
			 
			 //���Ե�ʳ�����������
			 else if(IfEat(&gpSpVars->SNAKE,&gpSpVars->SNAKEFOOD)){
			 	digest(&gpSpVars->SNAKE);
			 	CreatFood(&gpSpVars->SNAKEFOOD);
			 }
			 
			 //����һ�������������Ҳ���Ե�ʳ�����䶯һ��
			 else	
			 	Go(&gpSpVars->SNAKE);	
				
			break;
			
	}

	return 0;
}
//��ʹ���߰��±�ҳTouchRegionSet�ﶨ��İ���ʱ���ᴥ�����������Ķ�Ӧ�¼�
static CO_MSG ButtonHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo){		
	switch(Key){	
		case BackKV:
			Q_GotoPage(GotoNewPage,"AppListPage",0,NULL);			
			break;
		case DoneKV:
				StartGame();//��ʼ��Ϸ
			break;
		
		case LeftArrowKV:
			//������ת
			if(gpSpVars->CtrlFlag==0)
				Turn_left(&gpSpVars->SNAKE);
			gpSpVars->CtrlFlag=1;
			break;
		case DotKV:{
				void *p;
				Q_TimSet(Q_TIM1,0,0,(bool)1);//tm2ֹͣ����
				p=Q_PageMallco(1000);
				SP_AddOptionsHeader(p,1000,"Set Snake",OSID_SNAKE);	
				SP_AddNumListOption(p,SNAKEOP_SPEED,gpSpVars->CUR_SPEEDLEV,1,3,1,"ѡ���ߵ��ٶ�","1������ 2������ 3��������");
				SP_AddNumListOption(p,SNAKEOP_SIZE,gpSpVars->CUR_SIZELEV,1,3,1,"ѡ���ߵĴ�С","1��С�� 2������ 3������");
				SP_AddNumListOption(p,SNAKEOP_SNAKE_COLOUR,gpSpVars->SNAKE_COLOURLEV,1,5,1,"ѡ���ߵ���ɫ","1�� 2�� 3�� 4�� 5��");
				SP_AddNumListOption(p,SNAKEOP_FOOD_COLOUR,gpSpVars->FOOD_COLOURLEV,1,5,1,"ѡ��ʳ�����ɫ","1�� 2�� 3�� 4�� 5��");
				SP_AddNumListOption(p,SNAKEOP_BG_COLOUR,gpSpVars->BG_COLOURLEV,1,5,1,"ѡ�񳡵ص���ɫ","1�� 2�� 3�� 4�� 5��");
				Q_GotoPage(GotoSubPage,"SettingsPage",TRUE,p);				
			}			
			break;	
		case RightArrowKV:
		
			//������ת
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
			//��Ҫ��Ӧ���¼�δ����
			Debug("%s ButtonHandler:This Touch Event Handler case unfinish! Key:%d\n\r",Q_GetCurrPageName(),Key);
	}	
	return 0;
}


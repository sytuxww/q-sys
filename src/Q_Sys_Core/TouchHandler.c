#include "System.h"

extern OS_SemaphoreHandle gTouchHandler_Sem;
extern OS_SemaphoreHandle gAllowTchHandler_Sem;

#define Touch_Debug(x,y...) //Debug
#define TOUCH_WAIT_MS		100
#define TOUCH_SAMPLE_NUM 18   //每次触摸采用数目

u8 gTouchRegionNum=0;//记录所有注册区域个数的变量
TOUCH_REGION *gpTouchRegions;//记录所有注册区域的指针变量，包括全部静态和动态控件

float X_RATIO=1.0;
float Y_RATIO=1.0;
s32 X_OFFSET=0.0;
s32 Y_OFFSET=0.0;

TOUCH_REGION *GetTouchInfoByIdx(u8 Idx)
{
	if(Idx>=gTouchRegionNum) return NULL;
	else return &gpTouchRegions[Idx];
}

//用于主线程允许本线程发送触摸信息
//和Wait_Touch_Allow_Input一起，主要起到的作用是防止主线程未处理完事件响应前又来了新的触摸事件
void Allow_Touch_Input(void)
{
	//if(!gAllowTchHandler_Sem->OSEventCnt)
	OS_SemaphoreGive(gAllowTchHandler_Sem);
}

//等待中断信号
static void Wait_Touch_Allow_Input(void)
{
	OS_SemaphoreTake(gAllowTchHandler_Sem,OS_MAX_DELAY); 
}

//获取并计算触摸坐标
static int Get_Touch_Coordinate(u16 *x,u16 *y)
{
	u16 local_x[TOUCH_SAMPLE_NUM],local_y[TOUCH_SAMPLE_NUM],x_min,x_max,y_min,y_max;
	int x_sum,y_sum;
	u8 num;
		
	if(HasTouch())
	{
		GetTouchX();GetTouchY();		//先读一次数据，去掉干扰
		
		x_min=y_min=0xffff;
		x_max=y_max=x_sum=y_sum=0;
		
		for(num=0;num<TOUCH_SAMPLE_NUM;)
		{
			OS_TaskDelayMs(2);
			local_x[num]=GetTouchX();
			local_y[num]=GetTouchY();

			if((local_x[num]==0)||(local_y[num]==4095)) return FALSE;
			
			if(x_min>local_x[num]) x_min=local_x[num];
			if(x_max<local_x[num]) x_max=local_x[num];

			if(y_min>local_y[num]) y_min=local_y[num];
			if(y_max<local_y[num]) y_max=local_y[num];

			x_sum+=local_x[num];
			y_sum+=local_y[num];
			
			//Touch_Debug("%4d , %4d\n\r",local_x[num],local_y[num]);
			if(++num==TOUCH_SAMPLE_NUM) 
			{
				x_sum=(x_sum-(x_min+x_max))>>4;//!!!divide TOUCH_SAMPLE_NUM optimize
				y_sum=(y_sum-(y_min+y_max))>>4;//!!!divide TOUCH_SAMPLE_NUM optimize

				//Debug("Touch %4d , %4d\n\r",x_sum,y_sum);
#if QXW_LCM_ID == 212 || QXW_LCM_ID == 211 || QXW_LCM_ID == 210
				*x=(4*x_sum-1760)/55;
				*y=(15200-4*y_sum)/43;
#elif QXW_LCM_ID == 220
				*x=(x_sum-295)*5/74;
				*y=(y_sum-170)*4/43;
#elif QXW_LCM_ID == 221
				*x=(y_sum-230)*24/367;
				*y=345-8*x_sum/91;
#endif

				//Debug("Touch %4d , %4d\n\r",(int)*x,(int)*y);
								
				*x=(u16)((float)(*x)*X_RATIO)+X_OFFSET;
				*y=(u16)((float)(*y)*Y_RATIO)+Y_OFFSET;	 
				
				if((*x>240)||(*y>350)) return FALSE;
				
				return TRUE;
			}
		}
	}
	
	return FALSE;
}

//检测输入点是否在注册区域之内
static u8 Check_Touch_Input_Region(u16 x,u16 y,TOUCH_REGION *pRegion)
{
	if(pRegion->OptionsMask&LandMsk)//此按键是横屏按键
	{
		//Touch_Debug("(%d,%d) ?in (%d,%d) - (%d,%d)\n\r",x,y,pRegion->x,pRegion->y,pRegion->x-pRegion->h+1,pRegion->y+pRegion->w-1);
		if(x<=pRegion->x)
		{
			if((pRegion->x-x)<pRegion->h)
			{
				if(y>=pRegion->y)
				{
					if((y-pRegion->y)<pRegion->w) return TRUE;
					else return FALSE;
				}else return FALSE;
			}else return FALSE;
		}else return FALSE;	
	}

	//Touch_Debug("(%d,%d) ?in (%d,%d) - (%d,%d)\n\r",x,y,pRegion->x,pRegion->y,pRegion->x+pRegion->w-1,pRegion->y+pRegion->h-1);
	//竖屏按键
	if(x>=pRegion->x)
	{
		if((x-pRegion->x)<pRegion->w)
		{
			if(y>=pRegion->y)
			{
				if((y-pRegion->y)<pRegion->h) return TRUE;
				else return FALSE;
			}else return FALSE;
		}else return FALSE;
	}else return FALSE;	
}

//检查输入是否在拥有此按键键值的区域内
//仅对按键控件有效
//其他类型直接返回当前区域是否点击
static bool Chk_SameID_Touch(u16 x,u16 y,u8 TchRegIdx)
{
	u8 i;
	u8 ObjID=gpTouchRegions[TchRegIdx].ObjID;

	if(gpTouchRegions[TchRegIdx].Type<=COT_DynCharBtn)
	{
		for(i=0;i<gTouchRegionNum;i++)//找相同键值的键
		{
			if(gpTouchRegions[i].ObjID==ObjID)
				if(Check_Touch_Input_Region(x,y,&gpTouchRegions[i]))
					return TRUE;
		}	
	}
	else //其他类型直接返回当前区域是否点击
	{
		if(Check_Touch_Input_Region(x,y,&gpTouchRegions[TchRegIdx]))
			return TRUE;
	}

	return FALSE;
}

//本任务负责从中断获取触摸信号，然后读出触摸点。
//对比触摸点和当前注册区域，并发出事件给InputHandler任务处理。
void TouchHandler_Task( void *Task_Parameters )
{
	INPUT_EVENT KeyEvtParam;
	TOUCH_INFO  *pTouchInfo=&KeyEvtParam.Info.TouchInfo;
	u32 TimeStamp;
	u8 Idx,TchEvtMsk;
	u8 Error;
	
	Error=Error;//for no warning
	pTouchInfo->Id=0;
	KeyEvtParam.uType=Touch_Type;
	Allow_Touch_Input();
	while(1)
	{
WaitTouch:
		Wait_Touch_Allow_Input();
		Touch_Debug("Wait touch interrupt...\n\r");
		//Debug("WT\n\r");
		OS_SemaphoreTake(gTouchHandler_Sem,OS_MAX_DELAY); 
		//OS_TaskSuspend(TOUCH_TASK_PRIORITY);//等待触摸事件

		pTouchInfo->Id++;
		TimeStamp=OS_GetCurrentSysMs();		
		if(HasTouch())	//表示有点击
		{
			OS_TaskDelayMs(10);
			if(Get_Touch_Coordinate(&pTouchInfo->x,&pTouchInfo->y))//获取点击坐标
			{//开始检查是否点击了图片按键区域
				for(Idx=0;Idx<gTouchRegionNum;Idx++)
				{
					if(gpTouchRegions[Idx].Type!=COT_NULL)
						if(Check_Touch_Input_Region(pTouchInfo->x,pTouchInfo->y,&gpTouchRegions[Idx]))
						{//发送press事件，因为要触发坐标，所以必须发送，不需要验证TchEvtMsk
							TchEvtMsk=gpTouchRegions[Idx].OptionsMask;//将选项掩码的低8位重要信息传递出去
							pTouchInfo->TimeStamp=OS_GetCurrentSysMs()-TimeStamp;
							KeyEvtParam.Num=Idx;//区域索引传过去
							KeyEvtParam.EventType=Input_TchPress;
							Touch_Debug("Send Press Touch Info %3d %3d@%4d\n\r",pTouchInfo->x,pTouchInfo->y,pTouchInfo->TimeStamp);
							Error=OS_MsgBoxSend(gInputHandler_Queue,&KeyEvtParam,TOUCH_WAIT_MS,FALSE);
							//Debug("@TchType:Presss[%d]\n\r",Error);
							goto HavePressTouch;//一旦检测到，就不再检测其他按键区域了
						}
				}
				
				Touch_Debug("↓Touch not be register!(x:%3d y:%3d)\n\r",pTouchInfo->x,pTouchInfo->y);
				Allow_Touch_Input();
				goto WaitTouch;

HavePressTouch:

				while(HasTouch())//&&(OS_GetCurrentSysMs()-TimeStamp<MAX_TOUCH_PRESS_MS))	//等待按键释放
				{
					if(Get_Touch_Coordinate(&pTouchInfo->x,&pTouchInfo->y))//获取点击坐标
					{
						if(TchEvtMsk&CotMsk)//检查事件掩码
						{
							if(Chk_SameID_Touch(pTouchInfo->x,pTouchInfo->y,Idx))
							{//在键值区域之内 发送continue事件		
								pTouchInfo->TimeStamp=OS_GetCurrentSysMs()-TimeStamp;
								KeyEvtParam.EventType=Input_TchContinue;
								Touch_Debug("Send Continue Touch Info %3d %3d@%4d\n\r",pTouchInfo->x,pTouchInfo->y,pTouchInfo->TimeStamp);
								Error=OS_MsgBoxSend(gInputHandler_Queue,&KeyEvtParam,TOUCH_WAIT_MS,FALSE);
								//Debug("@TchType:Continue[%d]\n\r",Error);
							}	
							else OS_TaskDelayMs(20);//没有发送就延时
						}
						else OS_TaskDelayMs(20);//没有发送就延时
					}
					else //获取坐标失败
					{
						Touch_Debug("Continue Get Touch Coordinate error!\n\r");
						break;//跳出，这里跳出或者继续循环都可以
					}
				}

				//发送release事件
				pTouchInfo->TimeStamp=OS_GetCurrentSysMs()-TimeStamp;
				if(Chk_SameID_Touch(pTouchInfo->x,pTouchInfo->y,Idx))
				{//最后一次接触依然在键值区域
					KeyEvtParam.EventType=Input_TchRelease;
				}
				else	//最后一次接触在无效区域
				{
					KeyEvtParam.EventType=Input_TchReleaseVain;
				}
				Touch_Debug("Send Release Touch Info %3d %3d@%4d\n\r",pTouchInfo->x,pTouchInfo->y,pTouchInfo->TimeStamp);
				Error=OS_MsgBoxSend(gInputHandler_Queue,&KeyEvtParam,TOUCH_WAIT_MS,FALSE);
				//Debug("@TchType:Relse[%d]\n\r",Error);

				OS_TaskDelayMs(20);//一次触碰完毕，要多等些时间
			}
			else //貌似有点击，但是没获取到坐标信息
			{
				Touch_Debug("Press Get Touch Coordinate error!\n\r");
				OS_TaskDelayMs(10);
				Allow_Touch_Input();
			}
		}
		else
		{
			OS_TaskDelayMs(10);//延时
			Allow_Touch_Input();
		}
	}

}




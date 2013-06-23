#include "System.h"

extern OS_SemaphoreHandle gTouchHandler_Sem;
extern OS_SemaphoreHandle gAllowTchHandler_Sem;

#define Touch_Debug(x,y...) //Debug
#define TOUCH_WAIT_MS		100
#define TOUCH_SAMPLE_NUM 18   //ÿ�δ���������Ŀ

u8 gTouchRegionNum=0;//��¼����ע����������ı���
TOUCH_REGION *gpTouchRegions;//��¼����ע�������ָ�����������ȫ����̬�Ͷ�̬�ؼ�

float X_RATIO=1.0;
float Y_RATIO=1.0;
s32 X_OFFSET=0.0;
s32 Y_OFFSET=0.0;

TOUCH_REGION *GetTouchInfoByIdx(u8 Idx)
{
	if(Idx>=gTouchRegionNum) return NULL;
	else return &gpTouchRegions[Idx];
}

//�������߳������̷߳��ʹ�����Ϣ
//��Wait_Touch_Allow_Inputһ����Ҫ�𵽵������Ƿ�ֹ���߳�δ�������¼���Ӧǰ�������µĴ����¼�
void Allow_Touch_Input(void)
{
	//if(!gAllowTchHandler_Sem->OSEventCnt)
	OS_SemaphoreGive(gAllowTchHandler_Sem);
}

//�ȴ��ж��ź�
static void Wait_Touch_Allow_Input(void)
{
	OS_SemaphoreTake(gAllowTchHandler_Sem,OS_MAX_DELAY); 
}

//��ȡ�����㴥������
static int Get_Touch_Coordinate(u16 *x,u16 *y)
{
	u16 local_x[TOUCH_SAMPLE_NUM],local_y[TOUCH_SAMPLE_NUM],x_min,x_max,y_min,y_max;
	int x_sum,y_sum;
	u8 num;
		
	if(HasTouch())
	{
		GetTouchX();GetTouchY();		//�ȶ�һ�����ݣ�ȥ������
		
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

//���������Ƿ���ע������֮��
static u8 Check_Touch_Input_Region(u16 x,u16 y,TOUCH_REGION *pRegion)
{
	if(pRegion->OptionsMask&LandMsk)//�˰����Ǻ�������
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
	//��������
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

//��������Ƿ���ӵ�д˰�����ֵ��������
//���԰����ؼ���Ч
//��������ֱ�ӷ��ص�ǰ�����Ƿ���
static bool Chk_SameID_Touch(u16 x,u16 y,u8 TchRegIdx)
{
	u8 i;
	u8 ObjID=gpTouchRegions[TchRegIdx].ObjID;

	if(gpTouchRegions[TchRegIdx].Type<=COT_DynCharBtn)
	{
		for(i=0;i<gTouchRegionNum;i++)//����ͬ��ֵ�ļ�
		{
			if(gpTouchRegions[i].ObjID==ObjID)
				if(Check_Touch_Input_Region(x,y,&gpTouchRegions[i]))
					return TRUE;
		}	
	}
	else //��������ֱ�ӷ��ص�ǰ�����Ƿ���
	{
		if(Check_Touch_Input_Region(x,y,&gpTouchRegions[TchRegIdx]))
			return TRUE;
	}

	return FALSE;
}

//����������жϻ�ȡ�����źţ�Ȼ����������㡣
//�Աȴ�����͵�ǰע�����򣬲������¼���InputHandler������
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
		//OS_TaskSuspend(TOUCH_TASK_PRIORITY);//�ȴ������¼�

		pTouchInfo->Id++;
		TimeStamp=OS_GetCurrentSysMs();		
		if(HasTouch())	//��ʾ�е��
		{
			OS_TaskDelayMs(10);
			if(Get_Touch_Coordinate(&pTouchInfo->x,&pTouchInfo->y))//��ȡ�������
			{//��ʼ����Ƿ�����ͼƬ��������
				for(Idx=0;Idx<gTouchRegionNum;Idx++)
				{
					if(gpTouchRegions[Idx].Type!=COT_NULL)
						if(Check_Touch_Input_Region(pTouchInfo->x,pTouchInfo->y,&gpTouchRegions[Idx]))
						{//����press�¼�����ΪҪ�������꣬���Ա��뷢�ͣ�����Ҫ��֤TchEvtMsk
							TchEvtMsk=gpTouchRegions[Idx].OptionsMask;//��ѡ������ĵ�8λ��Ҫ��Ϣ���ݳ�ȥ
							pTouchInfo->TimeStamp=OS_GetCurrentSysMs()-TimeStamp;
							KeyEvtParam.Num=Idx;//������������ȥ
							KeyEvtParam.EventType=Input_TchPress;
							Touch_Debug("Send Press Touch Info %3d %3d@%4d\n\r",pTouchInfo->x,pTouchInfo->y,pTouchInfo->TimeStamp);
							Error=OS_MsgBoxSend(gInputHandler_Queue,&KeyEvtParam,TOUCH_WAIT_MS,FALSE);
							//Debug("@TchType:Presss[%d]\n\r",Error);
							goto HavePressTouch;//һ����⵽���Ͳ��ټ����������������
						}
				}
				
				Touch_Debug("��Touch not be register!(x:%3d y:%3d)\n\r",pTouchInfo->x,pTouchInfo->y);
				Allow_Touch_Input();
				goto WaitTouch;

HavePressTouch:

				while(HasTouch())//&&(OS_GetCurrentSysMs()-TimeStamp<MAX_TOUCH_PRESS_MS))	//�ȴ������ͷ�
				{
					if(Get_Touch_Coordinate(&pTouchInfo->x,&pTouchInfo->y))//��ȡ�������
					{
						if(TchEvtMsk&CotMsk)//����¼�����
						{
							if(Chk_SameID_Touch(pTouchInfo->x,pTouchInfo->y,Idx))
							{//�ڼ�ֵ����֮�� ����continue�¼�		
								pTouchInfo->TimeStamp=OS_GetCurrentSysMs()-TimeStamp;
								KeyEvtParam.EventType=Input_TchContinue;
								Touch_Debug("Send Continue Touch Info %3d %3d@%4d\n\r",pTouchInfo->x,pTouchInfo->y,pTouchInfo->TimeStamp);
								Error=OS_MsgBoxSend(gInputHandler_Queue,&KeyEvtParam,TOUCH_WAIT_MS,FALSE);
								//Debug("@TchType:Continue[%d]\n\r",Error);
							}	
							else OS_TaskDelayMs(20);//û�з��;���ʱ
						}
						else OS_TaskDelayMs(20);//û�з��;���ʱ
					}
					else //��ȡ����ʧ��
					{
						Touch_Debug("Continue Get Touch Coordinate error!\n\r");
						break;//�����������������߼���ѭ��������
					}
				}

				//����release�¼�
				pTouchInfo->TimeStamp=OS_GetCurrentSysMs()-TimeStamp;
				if(Chk_SameID_Touch(pTouchInfo->x,pTouchInfo->y,Idx))
				{//���һ�νӴ���Ȼ�ڼ�ֵ����
					KeyEvtParam.EventType=Input_TchRelease;
				}
				else	//���һ�νӴ�����Ч����
				{
					KeyEvtParam.EventType=Input_TchReleaseVain;
				}
				Touch_Debug("Send Release Touch Info %3d %3d@%4d\n\r",pTouchInfo->x,pTouchInfo->y,pTouchInfo->TimeStamp);
				Error=OS_MsgBoxSend(gInputHandler_Queue,&KeyEvtParam,TOUCH_WAIT_MS,FALSE);
				//Debug("@TchType:Relse[%d]\n\r",Error);

				OS_TaskDelayMs(20);//һ�δ�����ϣ�Ҫ���Щʱ��
			}
			else //ò���е��������û��ȡ��������Ϣ
			{
				Touch_Debug("Press Get Touch Coordinate error!\n\r");
				OS_TaskDelayMs(10);
				Allow_Touch_Input();
			}
		}
		else
		{
			OS_TaskDelayMs(10);//��ʱ
			Allow_Touch_Input();
		}
	}

}




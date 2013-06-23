#include "Os_Wrap.h"
#include "User.h" //for debug
#define OS_Debug //Debug

#include "Os_Select.h"

#if OS_USE_UCOS

#define QS_TASK_RECORD 1//开启酷系统任务记录功能
#define QS_TASK_RECORD_NUM	16 //最大记录任务数


#if QS_TASK_RECORD
//任务堆栈记录
typedef struct{
	const u8 *Name;
	void *Stk;
	u16 StkSize;
	u8 Priority;
}QSYS_TASK_RECORD;
static u8 QsTaskTotalNum=0;
static QSYS_TASK_RECORD QsTaskRecord[QS_TASK_RECORD_NUM];
#endif

void OS_WrapInit(void)
{
	OSInit(); //Initialize "uC/OS-II, The Real-Time Kernel".    
	QS_HeapInit();
}

void OS_StartRun(void)
{
	OSTimeSet(0);
	OSStart();  //Start multitasking (i.e. give control to uC/OS-II).  
}

//任务创建部分
u8 OS_TaskCreate(TASK_FUNC TaskFunc, const u8 *TaskName, u16 StackSizeByte, void *pParam, u8 Priority,void **pTaskHandle)
{
	u8 error;
	void *ptr=Q_Mallco(StackSizeByte+4);//从mallco出来一定是4byte align，我们需要8byte align，所以多分配4个

#if QS_TASK_RECORD
	MemSet(ptr,0x5a,StackSizeByte);//记录堆栈
#endif

	error=OSTaskCreate(TaskFunc,pParam,(OS_STK *)(((((u32)ptr)+StackSizeByte+4)&0xfffffff8)-4),Priority);
	
	if(OS_ERR_NONE!=error)
	{
		Debug("!!!Create Task error %d\n\r",error);
		OS_SchedLock();
		while(1);
	}

	OSTaskNameSet(Priority,(void *)TaskName,&error);

#if QS_TASK_RECORD	
	QsTaskRecord[QsTaskTotalNum].Name=TaskName;
	QsTaskRecord[QsTaskTotalNum].Priority=Priority;
	QsTaskRecord[QsTaskTotalNum].Stk=ptr;
	QsTaskRecord[QsTaskTotalNum++].StkSize=StackSizeByte;	
	OS_Debug("Create Task %s @ STK 0x%x-0x%x\n\r",TaskName,(u32)ptr,(u32)ptr+StackSizeByte);
#endif
	if(pTaskHandle!=NULL)
		*pTaskHandle=(void *)Priority;
	return error;
}

void OS_TaskStkCheck(bool Display)
{
#if QS_TASK_RECORD	
	u8 i;
	u8 *p;

	if(Display)
	{
		Debug("----------------------Task Record----------------------\n\r");
		for(i=0;i<QsTaskTotalNum;i++)
		{
			for(p=(u8 *)QsTaskRecord[i].Stk;*p==0x5a;p++);
			Debug("Task %s,Stk 0x%x,Remain %d\n\r",QsTaskRecord[i].Name,QsTaskRecord[i].Stk,
				(u32)p-(u32)QsTaskRecord[i].Stk);
		}
		Debug("----------------------Task Record----------------------\n\r");
	}
	else
	{
		for(i=0;i<QsTaskTotalNum;i++)
		{
			if(*(u32 *)QsTaskRecord[i].Stk!=0x5a5a5a5a) 
			{
				Debug("!!!Task %s stk is overflow!Stk 0x%x\n\r",QsTaskRecord[i].Name,(u32)QsTaskRecord[i].Stk);
				OS_SchedLock();
				Debug("----------------------Task Record----------------------\n\r");
				for(i=0;i<QsTaskTotalNum;i++)
				{
					for(p=(u8 *)QsTaskRecord[i].Stk;*p==0x5a;p++);
					Debug("Task %s,Stk 0x%x,Remain %d\n\r",QsTaskRecord[i].Name,QsTaskRecord[i].Stk,
						(u32)p-(u32)QsTaskRecord[i].Stk);
				}
				Debug("----------------------Task Record----------------------\n\r");
				while(1);
			}
		}
	}
#endif
}

u8 OS_TaskDelete(void *TaskHandle)
{
	if(TaskHandle==NULL)
		return OS_ERR_TASK_NOT_EXIST;
	return (OSTaskDel((u32)TaskHandle));
}

void OS_TaskDelay (u16 Ticks)
{
	OSTimeDly(Ticks);
}

void OS_TaskDelayMs(u16 Ms)
{
	OSTimeDly(Ms/OS_TICK_RATE_MS);
}

u8 OS_TaskSuspend(void *TaskHandle)
{
	if(TaskHandle==NULL)
		return OS_ERR_TASK_NOT_EXIST;
	return (OSTaskSuspend((u32)TaskHandle));
}

u8 OS_TaskResume(void *TaskHandle)
{
	if(TaskHandle==NULL)
		return OS_ERR_TASK_NOT_EXIST;
	return (OSTaskResume((u32)TaskHandle));
}

#if 0
OS_QueueHandle OS_QueueCreate(void **Start, u16 Size)
{
	return OSQCreate(Start,Size);
}

u8 OS_QueueSend(OS_QueueHandle pQueue, void *pMsg, u16 WaitTicks)
{
	return OSQPost(pQueue,pMsg);
}

u8 OS_QueueSendToFront(OS_QueueHandle pQueue, void *pMsg, u16 WaitTicks)
{
	return OSQPostFront(pQueue,pMsg);
}
 
u8 OS_QueueReceive(OS_QueueHandle pQueue, void *pItem,u8 ItemSize,u16 WaitTicks)
{
	u8 error;
	u8 *ptr=pItem,*pTemp;

	if(WaitTicks==OS_NO_DELAY)
	{
		pTemp=(u8 *)OSQAccept(pQueue,&error);

		if(pTemp==(u8 *)NULL)
		{
			if(error!=OS_ERR_Q_EMPTY) OS_Debug("QueueAccept error:%d\n\r",error);
			return error;
		}
	}
	else
	{
		pTemp=(u8 *)OSQPend(pQueue,WaitTicks,&error);
	}

	for(WaitTicks=0;WaitTicks<ItemSize;WaitTicks++) 
		ptr[WaitTicks]=pTemp[WaitTicks];

	if(error)OS_Debug("QueueReceive error:%d\n\r",error);
	return error;
}
#endif

/********************************************
*  功能      创建邮箱内存
*  入参      blksize：邮件大小 
*			 nblks：邮件数量
*  返回      成功：邮箱内存管理控制块指针
*  		     失败：返回空指针
*  可重入性  可重入
*********************************************/
QSYS_MSG_BOX_MEM *QMBMemCreat(u16 blksize,u8 nblks)
{
	QSYS_MSG_BOX_MEM    *pmem;//用来暂存邮箱内存管理控制块地址
	void   *addr;             //用来暂存邮箱内存地址
    u8     *pblk;			  //用来暂存邮件头地址
    void   **plink;			  //用来链接各邮件
	u16    ablksize;          //对齐后的邮件大小
    u32    i;
	if( blksize==0 || nblks==0 )//参数检验
	{
		Debug("QMBMemCreat parameter error!blksize and nblks must >0\r\n");
		return NULL;
	}
	ablksize = (((blksize-1)>>2)+1)<<2;//保证每封邮件的首地址都按四字节对齐
	addr = Q_Mallco(ablksize*nblks);//获得邮箱内存
	if(addr==NULL)
	{
		Debug("QMBMemCreat can't get mem from heap to creat QSYS_MSG_BOX_MEM\r\n");
		return NULL;
	}
    pmem = Q_Mallco(sizeof(QSYS_MSG_BOX_MEM));//获得邮箱内存管理控制块内存                             
    if (pmem == NULL)
	{ 
		Debug("QMBMemCreat can't get mem from heap to creat QSYS_MSG_BOX_MEM\r\n");
		Q_Free(addr);//释放之前申请的内存
        return NULL;
	}
	//加工邮箱内存
    plink = (void **)addr;                
    pblk  = (u8 *)((u32)addr + ablksize);  
    for (i = 0; i < (nblks - 1); i++)
	{    
       *plink = (void *)pblk;                //当前块首存入下一块地址
        plink = (void **)pblk;               //当前块向后顺移
        pblk  = (u8 *)((u32)pblk + ablksize); //获得下一块地址
    }
    *plink               = NULL;        //末块块首存入空指针
	//给邮箱内存控制块赋值
    pmem->QMBMemAddr     = addr;            
    pmem->QMBMemFreeList = addr;            
    pmem->QMBMemNFree    = nblks;                     
    pmem->QMBMemBlks     = nblks;
    pmem->QMBMemBlkSize  = blksize; 
    return pmem;//成功返回邮箱内存控制块的地址
}

/* *******************************************
*	功能	  获取空闲邮件
*   入参	  pQMBMem：邮箱内存管理控制块指针
*   返回      成功：指向一个空闲邮件的指针
*   		  失败：返回空指针
*   可重入性  可重入
*********************************************/
void *QMBMemGet(QSYS_MSG_BOX_MEM *pQMBMem)
{
	void      *pblk;
    OS_CPU_SR  cpu_sr = 0;
	if(pQMBMem==NULL) return NULL;
    OS_ENTER_CRITICAL();//关中断
    if (pQMBMem->QMBMemNFree > 0) {                       //查看是否有空邮件
        pblk                    = pQMBMem->QMBMemFreeList; //从空闲链表获取空闲邮件指针
        pQMBMem->QMBMemFreeList = *(void **)pblk;         //调整空闲链表
        pQMBMem->QMBMemNFree--;                           //空闲邮件数减一
        OS_EXIT_CRITICAL();//开中断
        return pblk;                                      //返回空闲邮件指针
    }
	else
	{
		OS_EXIT_CRITICAL();//开中断
		Debug("QMBMemGet can't get get free block\r\n");
		return NULL;         
	}                   
}

/******************************************** 
*   功能	 放回用完了的邮件
*	入参	 pQMBMem：邮箱内存管理控制块指针 
*			 pblk：废弃邮件指针
*	返回	 成功：返回1
*			 失败：返回0
*	可重入性 可重入
*********************************************/  
u8 QMBMemPut(QSYS_MSG_BOX_MEM *pQMBMem,void *pblk)
{
    OS_CPU_SR  cpu_sr = 0;
	if(pQMBMem==NULL || pblk==NULL) return 0;
    OS_ENTER_CRITICAL();//关中断
    if (pQMBMem->QMBMemNFree >= pQMBMem->QMBMemBlks) 
	{  
        OS_EXIT_CRITICAL();//开中断
		Debug("QMBMemPut can't put pblk,the pblk maybe illegal\r\n");
        return 0;
    }
    *(void **)pblk          = pQMBMem->QMBMemFreeList;   //将废弃邮件链接到空闲链表
    pQMBMem->QMBMemFreeList = pblk;
    pQMBMem->QMBMemNFree++;                             //空闲邮件数加一
    OS_EXIT_CRITICAL();//开中断
    return 1;                       
}

/********************************************* 
*	功能	  创建酷系统邮箱
*	入参	  Name：邮箱名 
*			  ItemSize：邮箱中邮件的大小 
*			  ItemNum：邮箱中邮件的数量
*   返回	  成功：酷系统邮箱控制块指针
*			  失败：返回空指针
*	可重入性  可重入
*********************************************/
OS_MsgBoxHandle OS_MsgBoxCreate(const u8 *Name,u16 ItemSize,u8 ItemNum)
{
	QSYS_MSG_BOX *pMsgBox;
	void *ptr;
	if(ItemNum==0 || ItemSize==0 || Name==NULL)
	{ 
		Debug("OS_MsgBoxCreate parameter error!blksize and nblks must >0\r\n");
		return NULL;
	}
	if((pMsgBox=(QSYS_MSG_BOX *)Q_Mallco(sizeof(QSYS_MSG_BOX)))==NULL)
	{
		Debug("OS_MsgBoxCreate can't get mem from heap to creat QSYS_MSG_BOX\r\n");
		return NULL; 
	}
	if((pMsgBox->Mem = QMBMemCreat(ItemSize, ItemNum))==NULL)
	{
		Debug("OS_MsgBoxCreate failed because QMBMemCreat failed\r\n");
		Q_Free(pMsgBox);
		return NULL; 
	}
	if((ptr = Q_Mallco(ItemNum*4))==NULL)
	{
		Debug("OS_MsgBoxCreate can't get mem from heap to creat OSMsg\r\n");
		Q_Free(pMsgBox);
	}
	if((pMsgBox->Queue = OSQCreate (ptr,ItemNum))== (OS_EVENT *)0)
	{
		Debug("OS_MsgBoxCreate failed because OSQCreate failed\r\n");
		Q_Free(pMsgBox);
		Q_Free(ptr);
		return NULL; 
	}
	if((pMsgBox->Sem = OSSemCreate (ItemNum)) == (OS_EVENT *)0 )
	{
		Debug("OS_MsgBoxCreate failed because OSSemCreate failed\r\n");
		Q_Free(pMsgBox);
		Q_Free(ptr);
		return NULL; 
	}
	pMsgBox->Name = Name;
	return 	pMsgBox;
}
/*************************************************** 
*	功能	  向酷系统邮箱发送邮件
*	入参	  pMsgBox：酷系统邮箱控制块指针 
*		      Msg：消息指针 
*		      WaitTicks：
*					OS_NO_DELAY  不等待 
*					OS_MAX_DELAY 一直等待
*					1-0xfffe     等待节拍数
*			  IfPostFront:
*					TRUE  发送至队头(适用于紧急事件)
*					FALSE 发送至队尾(适用于一般事件)
*	返回	  成功：OS_NO_ERR
*			  失败：返回OS_ERR_MBOX_FULL
*	可重入性  可重入
*	注意      若在中断中调用此函数，WaitTicks必须为OS_NO_DELAY
****************************************************/
u8 OS_MsgBoxSend(OS_MsgBoxHandle pMsgBox,void *Msg, u16 WaitTicks,bool IfPostFront)
{
	u8 error;
	void *pTempMsg;
	if(WaitTicks==OS_NO_DELAY)
	{
		if(OSSemAccept(pMsgBox->Sem)==0) 
		{
			Debug("OS_MsgBoxSend failed when WaitTicks==OS_NO_DELAY\r\n");
			return OS_ERR_MBOX_FULL;
		}
		else
		{
			pTempMsg = QMBMemGet(pMsgBox->Mem);//从邮箱内存获得一封空闲邮件
			MemCpy( pTempMsg, Msg, pMsgBox->Mem->QMBMemBlkSize );//将所传的信息复制到邮件中
			if(IfPostFront==FALSE)
				OSQPostOpt (pMsgBox->Queue, pTempMsg, OS_POST_OPT_NONE  );//将邮件放入邮箱队尾
			else
				OSQPostOpt (pMsgBox->Queue, pTempMsg, OS_POST_OPT_FRONT  );//将邮件放入邮箱队头
			//Debug("$$%s send\r\n",pMsgBox->Name);
			return OS_NO_ERR;
		}
	}
	else
	{
		if(OSIntNesting>0)
		{
			Debug("in ISR OS_MsgBoxSend's WaitTicks must be OS_NO_DELAY\r\n");
			return 	OS_ERR_MBOX_FULL;
		}
		OSSemPend (pMsgBox->Sem, WaitTicks, &error);
		if(error!=OS_ERR_NONE)
		{
			Debug("OS_MsgBoxSend failed when timeout\r\n");
			return OS_ERR_MBOX_FULL;
		}
		else
		{
			pTempMsg = QMBMemGet(pMsgBox->Mem);//从邮箱内存获得一封空闲邮件
			MemCpy( pTempMsg, Msg, pMsgBox->Mem->QMBMemBlkSize );//将所传的信息复制到邮件中
			if(IfPostFront==FALSE)
				OSQPostOpt (pMsgBox->Queue, pTempMsg, OS_POST_OPT_NONE  );//将邮件放入邮箱队尾
			else
				OSQPostOpt (pMsgBox->Queue, pTempMsg, OS_POST_OPT_FRONT  );//将邮件放入邮箱队头
			//Debug("$$%s send\r\n",pMsgBox->Name);
			return OS_NO_ERR;
		}
	}
}
/************************************************** 
*	功能	  从酷系统邮箱接收邮件
*	入参	  pMsgBox：酷系统邮箱控制块指针 
*		  	  Msg：接收消息的指针 
*		  	  WaitTicks：
*			  	   OS_NO_DELAY不等待 
*				   OS_MAX_DELAY一直等待
*				   1-0xfffe 等待节拍数
*	返回	  成功：OS_NO_ERR
*			  失败：返回OS_TIMEOUT
*	可重入性  可重入
**************************************************/
u8 OS_MsgBoxReceive(OS_MsgBoxHandle pMsgBox,void *Msg, u16 WaitTicks)
{
	u8 error;
	void *pTempMsg;
	if(WaitTicks==OS_NO_DELAY)
	{
		pTempMsg = OSQAccept (pMsgBox->Queue, &error);
		if(pTempMsg==NULL)
		{
		//	Debug("OS_MsgBoxReceive failed when WaitTicks==OS_NO_DELAY\r\n");
			return OS_TIMEOUT;
		}
		else
		{
			MemCpy( Msg ,pTempMsg , pMsgBox->Mem->QMBMemBlkSize );//将邮件内容拷贝到消息容器
			QMBMemPut (pMsgBox->Mem, pTempMsg);//释放一封邮件至空闲邮件链表
			OSSemPost (pMsgBox->Sem);//释放掉一个信号量
			//Debug("$$%s receive\r\n",pMsgBox->Name);
			return OS_NO_ERR;
		}
	}
	else
	{
		pTempMsg = OSQPend (pMsgBox->Queue, WaitTicks, &error);
		if(pTempMsg==NULL)
		{
			Debug("OS_MsgBoxReceive failed when timeout\r\n");
			return OS_TIMEOUT;
		}
		else
		{
			MemCpy( Msg ,pTempMsg , pMsgBox->Mem->QMBMemBlkSize );//将邮件内容拷贝到消息容器
			QMBMemPut (pMsgBox->Mem, pTempMsg);//释放一封邮件至空闲邮件链表
			OSSemPost (pMsgBox->Sem);//释放掉一个信号量
			//Debug("$$%s receive\r\n",pMsgBox->Name);
			return OS_NO_ERR;
		}
	}
}

#if 0
//Query msg box idle num
u8 OS_MsgBoxQueryIdleNum(OS_MsgBoxHandle pMsgBox)
{
	if(pMsgBox->Flag==MSG_BOX_FULL) return 0;
	else if(pMsgBox->Flag==MSG_BOX_IDLE) return pMsgBox->ItemNum;
	else
	{
		u8 i=pMsgBox->Idx;
		u8 n=0;
		CPU_SR cpu_sr;
		u8 *pItem;
	
		OS_EnterCritical();
		while(1)//找第一个
		{
			pItem=(u8 *)((u32)pMsgBox->Msg+pMsgBox->ItemSize*i);
			if(pItem[0]) break;
			if(++i==pMsgBox->ItemNum) i=0;
			if(i==pMsgBox->Idx) OS_Debug("Find MsgBox Item Error!\n\r");
			n++;
		}		
		OS_ExitCritical();

		return n;
	}
}
#endif
/******************************************************
*   功能    建立酷系统信号量(初值为0或1的二值信号量)
*   入参    Cnt	: 0   初值为0
				  >0  初值为1
*   返回    成功 : 酷系统信号量句柄
*           失败 : NULL
*******************************************************/
OS_SemaphoreHandle OS_SemaphoreCreate(u8 Cnt)
{
	if(Cnt==0)
		return  OSMboxCreate(NULL);
	else
		return  OSMboxCreate((void *)1);
}
/******************************************************
*   功能    获取酷系统信号量
*   入参    Sem :酷系统信号量句柄
*           WaitTicks :
*                  OS_NO_DELAY  不等待 
*				   OS_MAX_DELAY 一直等待
*				   1-0xfffe     等待节拍数
*   返回    成功 : 0
*           失败 : !0
*	注意    若在中断中调用此函数，WaitTicks必须为OS_NO_DELAY
*******************************************************/
u8 OS_SemaphoreTake(OS_SemaphoreHandle Sem, u16 WaitTicks)
{
	u8 error;
	if(WaitTicks==OS_NO_DELAY)
	{
		if( OSMboxAccept(Sem)!=NULL )
			return 0;
		else
		{
			OS_Debug("SemaphoreTake error:%d\n\r",error);
			return 1;
		}
	}
	else if(WaitTicks==OS_MAX_DELAY)
	{
		if( OSMboxPend(Sem,0,&error)!=NULL )
			return 0;
		else
		{
			OS_Debug("SemaphoreTake error:%d\n\r",error);
			return 1;
		}
	}
	else
	{
		if( OSMboxPend(Sem,WaitTicks,&error)!=NULL )
			return 0;
		else
		{
			OS_Debug("SemaphoreTake error:%d\n\r",error);
			return 1;
		}
	}
}
/******************************************************
*   功能    释放酷系统信号量
*   入参    Sem :酷系统信号量句柄
*   返回    成功 : 0
*           失败 : !0
*******************************************************/
u8 OS_SemaphoreGive(OS_SemaphoreHandle Sem)
{
	return OSMboxPost( Sem,(void *)1 );
}

OS_MutexHandler OS_MutexCreate(void)
{
	u8 error;
	static u8 Prio=SYS_MUTEX_MAX_PRIORITY;

	OS_MutexHandler pEvent=OSMutexCreate(Prio++,&error);

	if(error) Debug("Mutex Create %d\n\r",error);
	
	return pEvent;
}

u8 OS_MutexTake(OS_MutexHandler Mutex, u16 WaitTicks)
{
	u8 error;
	
	OSMutexPend(Mutex,WaitTicks,&error);

	if(error) OS_Debug("MutexTake error:%d\n\r",error);
	return error;
}

u8 OS_MutexGive(OS_MutexHandler Mutex)
{
	return OSMutexPost(Mutex);
}

u32 OS_GetCurrentTick(void)
{
	return OSTimeGet();
}

u32 OS_GetCurrentSysMs(void)	
{
	return (OSTimeGet()*OS_TICK_RATE_MS);
}


#elif OS_USE_FREERTOS

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#define QS_TASK_RECORD 1//酷系统任务记录功能开关

#define QS_TASK_RECORD_NUM	16 //最大记录任务数 注意此数必须大于等于当前任务数
static u8 QsTaskTotalNum=0;
static xTaskHandle QsTaskRecord[QS_TASK_RECORD_NUM];
#if QS_TASK_RECORD
signed char pcTaskInfoBuffer[QS_TASK_RECORD_NUM*40]={0};
#endif

void OS_WrapInit(void)
{
	QS_HeapInit();
}

void OS_StartRun(void)
{
	vTaskStartScheduler();
}

u8 OS_TaskCreate(TASK_FUNC TaskFunc, const u8 *TaskName, u16 StackSizeByte, void *pParam, u8 Priority,void **pTaskHandle)
{
	u32 error;
	xTaskHandle xHandle;
	error=xTaskCreate( TaskFunc, (const signed char *)TaskName, StackSizeByte/4, pParam, Priority, &xHandle );
	if(QsTaskTotalNum<QS_TASK_RECORD_NUM)
		QsTaskRecord[QsTaskTotalNum++]=xHandle;
	if(pTaskHandle!=NULL)
		(*pTaskHandle)=xHandle;	
	return error;
}

void OS_TaskStkCheck(bool Display)
{
	#if QS_TASK_RECORD
	u8 i;
	if(Display)
	{		
		vTaskList(pcTaskInfoBuffer);
		Debug("----------------------Task Record----------------------\r\n");
		//格式：任务名,任务状态,优先级，堆栈剩余量,TCB号
		Debug("              State   Prio  StkRemain  TCBNum");
		Debug("%s",pcTaskInfoBuffer);
		Debug("----------------------Task Record----------------------\r\n");
	}
	else
	{
		for(i=0;i<QsTaskTotalNum;i++)
		{
			if(uxTaskGetStackHighWaterMark(QsTaskRecord[QsTaskTotalNum])<8) 
			{
				Debug("!!!Task stk is overflow\n\r");
				vTaskSuspendAll();
				Debug("----------------------Task Record----------------------");
				//格式：任务名,任务状态,优先级，堆栈剩余量,TCB号
				vTaskList(pcTaskInfoBuffer);
				Debug("----------------------Task Record----------------------\n\r");
				while(1);
			}
		}
	}
	#endif
}
u8 OS_TaskDelete(void *TaskHandle)
{
	vTaskDelete(TaskHandle);
	return 0;
}
u8 OS_TaskSuspend(void *TaskHandle)
{
	vTaskSuspend(TaskHandle);
	return 0;
}
u8 OS_TaskResume(void *TaskHandle)
{
	vTaskResume(TaskHandle);
	return 0;
}
void OS_TaskDelay (u16 Ticks)
{
	vTaskDelay(Ticks); 
}
void OS_TaskDelayMs(u16 Ms)
{
	vTaskDelay(Ms/portTICK_RATE_MS); 
}
/********************************************* 
*	功能	    创建酷系统邮箱
*	入参	    Name：邮箱名 
*			    ItemSize：邮箱中邮件的大小 
*			    ItemNum：邮箱中邮件的数量
*   返回	    成功：酷系统邮箱控制块指针
*			    失败：返回空指针
*	线程安全性   安全
*********************************************/
OS_MsgBoxHandle OS_MsgBoxCreate(const u8 *Name,u16 ItemSize,u8 ItemNum)
{
	return (xQueueCreate( ItemNum, ItemSize ));	
}

/*************************************************** 
*	功能	    向酷系统邮箱发送邮件
*	入参	    pMsgBox：酷系统邮箱控制块指针 
*		        Msg：消息指针 
*		        WaitTicks：
*					OS_NO_DELAY  不等待 
*					OS_MAX_DELAY 一直等待
*					1-0xfffe     等待节拍数
*			    IfPostFront:
*					TRUE  发送至队头(适用于紧急事件)
*					FALSE 发送至队尾(适用于一般事件)
*	返回	    成功：0
*			    失败：1
*	线程安全性   安全
*	注意        若在中断中调用此函数，WaitTicks必须为OS_NO_DELAY
****************************************************/
u8 OS_MsgBoxSend(OS_MsgBoxHandle pMsgBox,void *Msg, u16 WaitTicks,bool IfPostFront)
{
	portBASE_TYPE xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE; 
	if(WaitTicks==OS_NO_DELAY)
	{
		if((*(u8 *)0xe000ed04)!=0)//判断是否在中断中
		{
			if(IfPostFront==FALSE)
			{
				if(xQueueSendToBackFromISR(pMsgBox,Msg,&xHigherPriorityTaskWoken)==pdTRUE)
				{
					if( xHigherPriorityTaskWoken )
					{
						taskYIELD();
					}
					return 0;
				}
				else
				{
					return 1;
				}
			}
			else
			{
				if(xQueueSendToFrontFromISR(pMsgBox,Msg,&xHigherPriorityTaskWoken)==pdTRUE)
				{
					if( xHigherPriorityTaskWoken )
					{
						taskYIELD();
					}
					return 0;
				}
				else
				{
					return 1;
				}
			}
		}
		else
		{
			if(IfPostFront==FALSE)
			{
				if(xQueueSendToBack( pMsgBox, Msg, 0)==pdTRUE)
					return 0;
				else
					return 1;
			} 
			else
			{
				if(xQueueSendToFront( pMsgBox, Msg, 0)==pdTRUE)
					return 0;
				else
					return 1; 
			}
		}	
	}
	else if(WaitTicks==OS_MAX_DELAY)
	{
		if(IfPostFront==FALSE)
		{
			if(xQueueSendToBack( pMsgBox, Msg, portMAX_DELAY)==pdTRUE)
				return 0;
			else
				return 1;
		} 
		else
		{
			if(xQueueSendToFront( pMsgBox, Msg, portMAX_DELAY)==pdTRUE)
				return 0;
			else
				return 1; 
		}	
	}
	else
	{	 
		if(IfPostFront==FALSE)
		{
			if(xQueueSendToBack( pMsgBox, Msg, WaitTicks)==pdTRUE)
				return 0;
			else
				return 1;
		} 
		else
		{
			if(xQueueSendToFront( pMsgBox, Msg, WaitTicks)==pdTRUE)
				return 0;
			else
				return 1; 
		}
	}
}

/************************************************** 
*	功能	    从酷系统邮箱接收邮件
*	入参	    pMsgBox：酷系统邮箱控制块指针 
*		  	    Msg：接收消息的指针 
*		  	    WaitTicks：
*			  	   OS_NO_DELAY不等待 
*				   OS_MAX_DELAY一直等待
*				   1-0xfffe 等待节拍数
*	返回	    成功：0
*			    失败：1
*	线程安全性   安全
*	注意        若在中断中调用此函数，WaitTicks必须为OS_NO_DELAY
**************************************************/
u8 OS_MsgBoxReceive(OS_MsgBoxHandle pMsgBox,void *Msg, u16 WaitTicks)
{
	portBASE_TYPE xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE; 
	if(WaitTicks==OS_NO_DELAY)
	{
		if((*(u8 *)0xe000ed04)!=0)//判断是否在中断中
		{
			if(xQueueReceiveFromISR(pMsgBox,Msg,&xHigherPriorityTaskWoken)==pdTRUE)
			{
				if( xHigherPriorityTaskWoken )
				{
					taskYIELD();
				}
				return 0;
			}
			else
			{
				return 1;
			}
		}
		else
		{
			if(xQueueReceive( pMsgBox, Msg, 0)==pdTRUE)
				return 0;
			else
				return 1;
		}	
	}
	else if(WaitTicks==OS_MAX_DELAY)
	{
		if(xQueueReceive( pMsgBox, Msg, portMAX_DELAY)==pdTRUE)
			return 0;
		else
			return 1;
	}
	else
	{	 
		if(xQueueReceive( pMsgBox, Msg, WaitTicks)==pdTRUE)
			return 0;
		else
			return 1;
	}

}
/******************************************************
*   功能    建立酷系统信号量(初值为0或1的二值信号量)
*   入参    Cnt	: 0   初值为0
				  >0  初值为1
*   返回    成功 : 酷系统信号量句柄
*           失败 : NULL
*******************************************************/
OS_SemaphoreHandle OS_SemaphoreCreate(u8 Cnt)
{
	OS_SemaphoreHandle xSemaphore;
	vSemaphoreCreateBinary( xSemaphore );
	if(Cnt==0)
		xSemaphoreTake( xSemaphore, 0 );
	return xSemaphore;
}
/******************************************************
*   功能    获取酷系统信号量
*   入参    Sem :酷系统信号量句柄
*           WaitTicks :
*                  OS_NO_DELAY  不等待 
*				   OS_MAX_DELAY 一直等待
*				   1-0xfffe     等待节拍数
*   返回    成功 : 0
*           失败 : !0
*	注意    若在中断中调用此函数，WaitTicks必须为OS_NO_DELAY
*******************************************************/
u8 OS_SemaphoreTake(OS_SemaphoreHandle Sem, u16 WaitTicks)
{
	if( WaitTicks==OS_NO_DELAY || (*(u8 *)0xe000ed04)!=0 )
	{
		if( xSemaphoreTake( Sem, 0 )==pdTRUE )
			return 0;
		else
			return 1;
	}
	else if(WaitTicks==OS_MAX_DELAY)
	{
		if( xSemaphoreTake( Sem, portMAX_DELAY )==pdTRUE )
			return 0;
		else
			return 1;
	}
	else
	{
		if( xSemaphoreTake( Sem, WaitTicks )==pdTRUE )
			return 0;
		else
			return 1;	
	}

	
}
/******************************************************
*   功能    释放酷系统信号量
*   入参    Sem :酷系统信号量句柄
*   返回    成功 : 0
*           失败 : !0
*******************************************************/
u8 OS_SemaphoreGive(OS_SemaphoreHandle Sem)
{
	portBASE_TYPE xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE; 
	if((*(u8 *)0xe000ed04)!=0)//判断是否在中断中	
	{
		if(xSemaphoreGiveFromISR( Sem, &xHigherPriorityTaskWoken )==pdTRUE)
		{
			if( xHigherPriorityTaskWoken )
			{
				taskYIELD();
			}
			return 0;	
		}
		else
		{
			return 1;
		}
	}
	else
	{
		if(xSemaphoreGive( Sem )==pdTRUE)
			return 0;
		else
			return 1;
	}
}
/******************************************************
*   功能    创建酷系统互斥量
*   入参    无
*   返回    成功 : 酷系统互斥量句柄
*           失败 : NULL
*******************************************************/
OS_MutexHandler OS_MutexCreate(void)
{
	return xSemaphoreCreateMutex();
}
/******************************************************
*   功能    获取酷系统互斥量
*   入参    Mutex :酷系统互斥量句柄 
*           WaitTicks :
*                  OS_NO_DELAY  不等待 
*				   OS_MAX_DELAY 一直等待
*				   1-0xfffe     等待节拍数
*   返回    成功 : 0
*           失败 : !0
*	注意    禁止在中断中调用此函数
*******************************************************/
u8 OS_MutexTake(OS_MutexHandler Mutex, u16 WaitTicks)
{
	if((*(u8 *)0xe000ed04)!=0)//判断是否在中断中	
		return OS_ERR_TIMEOUT;
	if( WaitTicks==OS_NO_DELAY )
	{
		if( xSemaphoreTake( Mutex, 0 )==pdTRUE )
			return 0;
		else
			return OS_ERR_TIMEOUT;
	}
	else if(WaitTicks==OS_MAX_DELAY)
	{
		if( xSemaphoreTake( Mutex, portMAX_DELAY )==pdTRUE )
			return 0;
		else
			return OS_ERR_TIMEOUT;
	}
	else
	{
		if( xSemaphoreTake( Mutex, WaitTicks )==pdTRUE )
			return 0;
		else
			return OS_ERR_TIMEOUT;	
	}	
}
/******************************************************
*   功能    释放酷系统互斥量
*   入参    Mutex :酷系统互斥量句柄 
*   返回    成功 : 0
*           失败 : !0
*	注意    禁止在中断中调用此函数
*******************************************************/
u8 OS_MutexGive(OS_MutexHandler Mutex)
{
	if((*(u8 *)0xe000ed04)!=0)//判断是否在中断中	
		return 1;
	if(xSemaphoreGive( Mutex )==pdTRUE)
			return 0;
		else
			return 1;
}
u32 OS_GetCurrentTick(void)
{
	return xTaskGetTickCount();
}

u32 OS_GetCurrentSysMs(void)	
{
	return (xTaskGetTickCount()*OS_TICK_RATE_MS);
}
void OS_SchedLock(void)
{
	vTaskSuspendAll(); 	
}
void OS_SchedUnlock(void)
{
	xTaskResumeAll();
}
void OS_IntEnter(void){}		
void OS_IntExit(void){}
void OS_CPU_SysTickInit(void){}
#endif


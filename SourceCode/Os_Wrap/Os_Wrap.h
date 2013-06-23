/*
本文件的目的是将操作系统包裹起来，让酷系统和具
体的操作系统无关。试想一下，当酷系统使用的操作
系统由freeRTOS变为uC/OS时，我们只需要改变此文件里相
关的宏就行了。如果没有本文件，那么我们需要逐个
去修改酷系统的文件。
实际工程项目中，我们将此文件形容为操作系统外衣。
*/
#ifndef QSYS_OS_WRAP_H
#define QSYS_OS_WRAP_H

#include "stm32f10x.h"
#include "Q_Heap.h"

#include "Os_Select.h"

#if OS_USE_UCOS
#include  "ucos_ii.h"
typedef void (*TASK_FUNC)(void *);

typedef struct{
	void *QMBMemAddr;     //整个邮箱内存地址
	void *QMBMemFreeList; //下一块空闲块地址
	u16  QMBMemBlkSize;   //邮件大小
	u8   QMBMemBlks;      //邮件数量
	u8   QMBMemNFree;	  //空闲邮件数量
}QSYS_MSG_BOX_MEM;//邮箱内存管理控制块

typedef struct{
	OS_EVENT *Queue;//消息队列
	OS_EVENT *Sem;  //计数式信号量
	QSYS_MSG_BOX_MEM *Mem;	//邮箱内存管理控制块
	const u8 *Name;
}QSYS_MSG_BOX;//酷系统邮箱控制块
typedef void * OS_QueueHandle;
typedef QSYS_MSG_BOX * OS_MsgBoxHandle;
typedef void * OS_SemaphoreHandle;
typedef void * OS_MutexHandler;

//os config
#define OS_MINIMAL_STACK_SIZE		128
#define OS_HIGHEST_PRIORITIES		6
#define OS_LOWEST_PRIORITIES		OS_LOWEST_PRIO+2
#define OS_TICK_RATE_HZ			OS_TICKS_PER_SEC			
#define OS_TICK_RATE_MS			(1000/OS_TICKS_PER_SEC)
#define OS_MAX_DELAY					0
#define OS_NO_DELAY					0xffff
#define OS_RETURN_OK 	OS_ERR_NONE
#define OS_Ms2Tick(M) (M/(1000/OS_TICKS_PER_SEC))

//mem function
void MemSet(void *Dst,u8 C,u16 Byte);
void MemCpy(void *Dst,const void *Src,u16 Byte);

//os api wrap
void OS_WrapInit(void);
void OS_StartRun(void);
u8 OS_TaskCreate(TASK_FUNC TaskFunc, const u8 *TaskName, u16 StackSizeByte, void *pParam, u8 Priority,void **pTaskHandle);
void OS_TaskStkCheck(bool Display);

u8 OS_TaskDelete(void *TaskHandle);
u8 OS_TaskSuspend(void *TaskHandle);
u8 OS_TaskResume(void *TaskHandle);

void OS_TaskDelay (u16 Ticks);
void OS_TaskDelayMs(u16 Ms);

#if 0
OS_QueueHandle OS_QueueCreate(u8 Num,u8 ItemSize);
u8 OS_QueueSend(OS_QueueHandle pQueue, void *pItem, u16 WaitTicks);
u8 OS_QueueReceive(OS_QueueHandle pQueue,void *pItem,u16 WaitTicks);
#endif

OS_MsgBoxHandle OS_MsgBoxCreate(const u8 *Name,u16 ItemSize,u8 ItemNum);
u8 OS_MsgBoxSend(OS_MsgBoxHandle pMsgBox, void *Msg, u16 WaitTicks, bool IfPostFront);
u8 OS_MsgBoxReceive(OS_MsgBoxHandle pMsgBox, void *Msg, u16 WaitTicks);
//u8 OS_MsgBoxQueryIdleNum(OS_MsgBoxHandle pMsgBox);

OS_SemaphoreHandle OS_SemaphoreCreate(u8 Cnt);
u8 OS_SemaphoreTake(OS_SemaphoreHandle Sem, u16 WaitTicks);
u8 OS_SemaphoreGive(OS_SemaphoreHandle Sem);

OS_MutexHandler OS_MutexCreate(void);
u8 OS_MutexTake(OS_MutexHandler Mutex, u16 WaitTicks);
u8 OS_MutexGive(OS_MutexHandler Mutex);

u32 OS_GetCurrentTick(void);
u32 OS_GetCurrentSysMs(void)	;

//os marco
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
#define OS_IntEnter()		{CPU_SR cpu_sr;CPU_CRITICAL_ENTER();OSIntNesting++;CPU_CRITICAL_EXIT();}
#else
#define OS_IntEnter()		{CPU_CRITICAL_ENTER();OSIntNesting++;CPU_CRITICAL_EXIT();}
#endif
#define OS_IntExit()	    OSIntExit()	

#define OS_DeclareCritical()	CPU_SR cpu_sr
#define OS_EnterCritical 	OS_ENTER_CRITICAL
#define OS_ExitCritical 	OS_EXIT_CRITICAL

#define OS_SchedLock OSSchedLock
#define OS_SchedUnlock OSSchedUnlock

#define OS_DebugHeap DebugHeap
#define OS_HeapMonitor QS_MonitorFragment

#elif OS_USE_FREERTOS
typedef void (*TASK_FUNC)(void *);
typedef void *OS_MsgBoxHandle;
typedef void *OS_SemaphoreHandle;
typedef void *OS_MutexHandler;
//os config

#define OS_MINIMAL_STACK_SIZE		128
#define ISR_HIGHEST_PRIORITIES		1
#define ISR_LOWEST_PRIORITIES		15
#define OS_HIGHEST_PRIORITIES		64
#define OS_TICK_RATE_HZ				500			
#define OS_TICK_RATE_MS				(1000/OS_TICK_RATE_HZ)
#define OS_MAX_DELAY				0
#define OS_NO_DELAY					0xffff
#define OS_RETURN_OK 				0
#define OS_ERR_NONE                 0u
#define OS_ERR_TIMEOUT              10u
#define OS_Ms2Tick(M) (M/(1000/OS_TICK_RATE_HZ))

//os api wrap
void OS_WrapInit(void);
void OS_StartRun(void);
void OS_CPU_SysTickInit(void);
u8 OS_TaskCreate(TASK_FUNC TaskFunc, const u8 *TaskName, u16 StackSizeByte, void *pParam, u8 Priority,void **pTaskHandle);
void OS_TaskStkCheck(bool Display);
u8 OS_TaskDelete(void *TaskHandle);
u8 OS_TaskSuspend(void *TaskHandle);
u8 OS_TaskResume(void *TaskHandle);
void OS_TaskDelay (u16 Ticks);
void OS_TaskDelayMs(u16 Ms);

OS_MsgBoxHandle OS_MsgBoxCreate(const u8 *Name,u16 ItemSize,u8 ItemNum);
u8 OS_MsgBoxSend(OS_MsgBoxHandle pMsgBox,void *Msg, u16 WaitTicks,bool IfPostFront);
u8 OS_MsgBoxReceive(OS_MsgBoxHandle pMsgBox,void *Msg, u16 WaitTicks);

OS_SemaphoreHandle OS_SemaphoreCreate(u8 Cnt);
u8 OS_SemaphoreTake(OS_SemaphoreHandle Sem, u16 WaitTicks);
u8 OS_SemaphoreGive(OS_SemaphoreHandle Sem);

OS_MutexHandler OS_MutexCreate(void);
u8 OS_MutexTake(OS_MutexHandler Mutex, u16 WaitTicks);
u8 OS_MutexGive(OS_MutexHandler Mutex);
u32 OS_GetCurrentTick(void);
u32 OS_GetCurrentSysMs(void);
//os marco	   
extern void vPortEnterCritical( void );
extern void vPortExitCritical( void );
extern void OS_IntEnter(void);		
extern void OS_IntExit(void);
void OS_SchedLock(void);
void OS_SchedUnlock(void);
#define OS_DeclareCritical()	
#define OS_EnterCritical 	vPortEnterCritical
#define OS_ExitCritical 	vPortExitCritical

#define OS_DebugHeap DebugHeap
#define OS_HeapMonitor QS_MonitorFragment

#define CPU_SR u32


#endif				 
#endif


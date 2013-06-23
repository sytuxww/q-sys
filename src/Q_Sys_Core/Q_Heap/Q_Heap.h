#ifndef QSYS_Q_HEAP_H
#define QSYS_Q_HEAP_H

#define Q_HEAP_TRACK_DEBUG 0//��1��򿪼�¼mallco�����ߵĹ��ܣ��������Ӷ����ڴ濪��

void DebugHeap(void);
void QS_HeapInit(void);
void QS_MonitorFragment(void);
bool IsHeapRam(void *p);

#if Q_HEAP_TRACK_DEBUG == 1
void *QS_Mallco(u16 Size,u8 *pFuncName,u32 Lines);
bool QS_Free(void *Ptr,u8 *pFuncName,u32 Lines);
#else
void *QS_Mallco(u16 Size);
bool QS_Free(void *Ptr);
#endif

#if Q_HEAP_TRACK_DEBUG ==1
#define Q_Mallco(n) QS_Mallco(n,(void *)__func__,__LINE__)
#define Q_Free(p) QS_Free(p,(void *)__func__,__LINE__)
#else
#define Q_Mallco QS_Mallco
#define Q_Free QS_Free
#endif




#endif


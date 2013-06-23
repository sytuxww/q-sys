#include "System.h"
#include "Q_Heap.h"

//����:����һ���򵥵Ķѹ������
//����Ҫ˼���ǽ��ڴ����Ϊ���ɸ���С��Ԫ�������ڴ�ʱ���������������С��Ԫ���ڴ�
//��Ϊ���ڴ����Ϊ��С��Ԫ�ˣ�����������¼�ڴ�������ı���ֻ��Ҫ16λ����
//�����С��Ԫ��4byte����ô�˴�����Թ���ĶѴ�СΪ0xffff��4��
//��ʵ���ϣ�16bit���棬���1λ������ʾ�ڴ���Ƿ�ʹ�ã�����ʵ�ʴ�С��0x7fff��4����
//��ԼΪ130k�Ŀռ䡣�����Ҫ�������ռ䣬ֻ��Ҫ�����С��Ԫ��Byte��Ŀ���ɡ�
//�˶ѹ�����ƵĶ��⿪����2����
// 1.��Ҫһ����¼��������¼�ڴ����Ϣ(��������HEAP_RECORD)��ÿһ���ڴ��Ӧһ����¼���Ա
//   �����¼��Ĵ�СMAX_RECORD_NUM���Ը���ʵ������޸�
// 2.������Ŀռ䲻����С��Ԫ��������+2ʱ�����ж�����ڴ����ģ����2���ֽ����ڴ�ŷ����ǡ�
//   ���統��С��Ԫ��4byte����������СΪ7����ʵ�ʷ�����ڴ���12��Ϊ4����������
//�ڶѷ���ʱ��Ϊ��ʡ����ʱ�䣬�˻��Ʋ���������ռ�����
//�öѹ������û�н��������:�ڴ���Ƭ

//�˶ѹ�������ڲ���ϵͳ��Ҳ���������㱼������
//�������㱼�������޸����¼���
// 1.�޸ĶѴ�С���������ʵ���������ֻ��Ҫ�޸�Q_HEAP_SIZE_BYTE����������
// 2.�޸ļ�¼���С��ÿ����һ���ڴ棬��ķ�һ����¼�飬�ɸ���ʵ������޸�MAX_RECORD_NUM�꼴��
// 3.�����������ٽ����ĺ궨��ɿյľ�����:Q_HEAP_CRIT_SAVE_DATA_DEFINE��Q_HEAP_ENTER_CRIT��Q_HEAP_EXIT_CRIT
// 4.�����Ҫ����Ŀռ�ܴ���ô����Ҫ�޸���С��Ԫ����Ӧ����غ�Q_HEAP_MINI_UNIT��Q_HEAP_UNIT_OFFSET

#if Q_HEAP_TRACK_DEBUG == 1
#define Need_Debug 1
#else
#define Need_Debug 0
#endif

#if Need_Debug
#define QH_Debug Debug
#else
#define QH_Debug(x,y...)
#endif

//Ϊ��֤���߳��¶ѷ����ԭ�Ӳ����������������ϵͳ�ٽ���
//����˻��Ʋ������ڶ��̣߳�����Զ���������������Ϊ�հ�
#define Q_HEAP_CRIT_SAVE_DATA_DEFINE 	OS_DeclareCritical()
#define Q_HEAP_ENTER_CRIT OS_EnterCritical()
#define Q_HEAP_EXIT_CRIT OS_ExitCritical()

//������֤�ڴ��Ƿ񱻳�ٵı�ʶ
//ռ��2���ֽڣ������ڴ�ǰ�󱻳�ٶ����ܲ���ĸ���Ϊ65535��֮1
#define Q_HEAP_UNIT_REWRITE_ID 0xa55a

#define Q_HEAP_SIZE_BYTE (24*1024)//��ջ�ܴ�С����Ϊֻ��16bit��ʾ�ڴ������������ʵ�ʴ�С����С��(0xffff*�ڴ浥Ԫ��С)

static struct
{
	u32 dDummy;//align
	u16 Heap[ Q_HEAP_SIZE_BYTE / 2 ];//mem
}QHS;

//�������������ͬʱ�޸�
#define Q_HEAP_MINI_UNIT 4 //�ڴ浥Ԫ��С����λByte�����ǵ�32λ��������4�ֽڶ��룬��ֵ����Ϊ4����
#define Q_HEAP_UNIT_OFFSET 2 // 2��Q_HEAP_UNIT_OFFSET�η��������Q_HEAP_MINI_UNIT

#define QH_UNIT_MAX_NUM (Q_HEAP_SIZE_BYTE/Q_HEAP_MINI_UNIT)//��Ԫ����

//�ڴ������
#define QH_UnitIdxMask 0x7fff //�õ�15λ��ʾ�ڴ����������Ե�ԪΪ��λ
#define QH_GetUnitIdx(x) (gQHeapRecord[x].UnitIdx&QH_UnitIdxMask)//xΪ��¼����ţ������ڴ��ĵ�һ����Ԫ����
#define QH_SetUnitIdx(x,Index) {gQHeapRecord[x].UnitIdx=((gQHeapRecord[x].UnitIdx&(~QH_UnitIdxMask))|((Index)&QH_UnitIdxMask));}
#define QH_GetUnitSize(x) (QH_GetUnitIdx(QH_GetNext((x)))-QH_GetUnitIdx((x)))//xΪ��¼����ţ������ڴ��ռ�õ�Ԫ��
//#define UnitIdxEnd QH_UNIT_MAX_NUM

//����ָ����Ķ���־����ڲ���xΪ��¼�����
#define QH_UsedMask 0x8000
#define QH_UNUSE 0x0000
#define QH_USED 0x8000
#define QH_GetUsedFlag(x) (gQHeapRecord[x].UnitIdx&(QH_UsedMask))//���ط�0��ʾ��ʹ�ã�����0��ʾ����
#define QH_SetUsedFlag(x,v) {gQHeapRecord[x].UnitIdx=((gQHeapRecord[x].UnitIdx&(~QH_UsedMask))|(v?QH_USED:QH_UNUSE));}

//�����������ݼ�¼����Ż�ȡ��һ�����Ӧ��¼������
#define QH_GetNext(x) (gQHeapRecord[x].Next)
#define QH_SetNext(x,v) {gQHeapRecord[x].Next=(v);}
#define QH_GetPrev(x) (gQHeapRecord[x].Prev)
#define QH_SetPrev(x,v) {gQHeapRecord[x].Prev=(v);}

//���ָ����ļ�¼
#define QH_CleanRecord(x) {gQHeapRecord[x].Prev=gQHeapRecord[x].Next=gQHeapRecord[x].UnitIdx=0;}

//For Track
#if Q_HEAP_TRACK_DEBUG == 1
#define QH_SetRecdName(x,p) gQHeapRecord[x].pFuncName=p
#define QH_SetRecdLines(x,s) gQHeapRecord[x].Lines=s
#define QH_GetRecdName(x) gQHeapRecord[x].pFuncName
#define QH_GetRecdLines(x) gQHeapRecord[x].Lines
#else
#define QH_SetRecdName(x,p)
#define QH_SetRecdLines(x,s)
#define QH_GetRecdName(x)
#define QH_GetRecdLines(x)
#endif

//��Ԫ���ֽڵ�ת��
#define QH_Unit2Byte(U) ((U)<<Q_HEAP_UNIT_OFFSET) //ת����Ԫ�����ֽ���,��λ�ƴ���˷�
#define QH_Byte2Unit(B) ((B)?((((B)-1)>>Q_HEAP_UNIT_OFFSET)+1):0) //ת���ֽ�����ռ�õ�Ԫ������λ�ƴ������

typedef struct {
	u8 Prev;//��¼��һ���ڴ��ļ�¼�����
	u8 Next;//��¼��һ���ڴ��ļ�¼�����
	u16 UnitIdx;//��¼�ڴ����ʼ��Ԫλ��
#if Q_HEAP_TRACK_DEBUG == 1
	u8 *pFuncName;
	u32 Lines;
#endif
}Q_HEAP_RECORD;
#define Q_HEAP_MAX_RECORD_NUM 256
#define QH_RECORD_START 0
#define QH_RECORD_END (Q_HEAP_MAX_RECORD_NUM-1)
static Q_HEAP_RECORD gQHeapRecord[Q_HEAP_MAX_RECORD_NUM];

//���ڵ���
void DebugHeap(void)
{
	u16 Index;//��ʾ�����ʼ��Ԫ
	Q_HEAP_CRIT_SAVE_DATA_DEFINE;
	
	Debug("--------------Heap Record--0x%08x----R:0x%08x-0x%08x--------\n\r",(u32)QHS.Heap,(u32)gQHeapRecord,(u32)gQHeapRecord+Q_HEAP_MAX_RECORD_NUM*sizeof(Q_HEAP_RECORD));
	Q_HEAP_ENTER_CRIT;
	for(Index=QH_RECORD_START;;Index=QH_GetNext(Index))
	{
		Debug("%3d:Used %d,Unit 0x%04x-0x%04x, Addr 0x%08x-0x%08x, Size %5dBytes, Prev %3d,Next %3d",
			Index,QH_GetUsedFlag(Index)?1:0,
			QH_GetUnitIdx(Index),QH_GetUnitIdx(QH_GetNext(Index)),
			(u32)QHS.Heap+QH_Unit2Byte(QH_GetUnitIdx(Index)),(u32)QHS.Heap+QH_Unit2Byte(QH_GetUnitIdx(QH_GetNext(Index))),
			QH_Unit2Byte(QH_GetUnitSize(Index)),
			QH_GetPrev(Index),QH_GetNext(Index));
#if Q_HEAP_TRACK_DEBUG == 1
		Debug(",Function %s@%d",QH_GetRecdName(Index),QH_GetRecdLines(Index));
#endif
		Debug("\n\r");

		if(QH_GetNext(Index)==QH_RECORD_END) break;
	}
	Q_HEAP_EXIT_CRIT;
	Debug("--------------Heap Record End--0x%08x---------\n\r",(u32)QHS.Heap+Q_HEAP_SIZE_BYTE);
}

void QS_HeapInit(void)
{
	u16 Index;
	Q_HEAP_CRIT_SAVE_DATA_DEFINE;

	if( (QH_Unit2Byte(1)!=Q_HEAP_MINI_UNIT)||(QH_Byte2Unit(Q_HEAP_MINI_UNIT)!=1) )
	{
		Debug("Q_HEAP_MINI_UNIT & Q_HEAP_UNIT_OFFSET Marco Define Error!!!\n\r");
		while(1);
	}
		
	Q_HEAP_ENTER_CRIT;
	for(Index=0;Index<(sizeof(gQHeapRecord)/sizeof(Q_HEAP_RECORD));Index++)
	{
		QH_CleanRecord(Index);
	}

	//����ͷ��¼
	QH_SetPrev(QH_RECORD_START,QH_RECORD_END);
	QH_SetNext(QH_RECORD_START,QH_RECORD_END);
	QH_SetUsedFlag(QH_RECORD_START,QH_UNUSE);
	QH_SetUnitIdx(QH_RECORD_START,0);
#if Q_HEAP_TRACK_DEBUG == 1
	QH_SetRecdName(QH_RECORD_START,"<Idle>");
	QH_SetRecdLines(QH_RECORD_START,0);
#endif

	//����β��¼
	QH_SetPrev(QH_RECORD_END,QH_RECORD_START);
	QH_SetNext(QH_RECORD_END,QH_RECORD_START);
	QH_SetUsedFlag(QH_RECORD_END,QH_USED);
	QH_SetUnitIdx(QH_RECORD_END,QH_UNIT_MAX_NUM);
#if Q_HEAP_TRACK_DEBUG == 1
	QH_SetRecdName(QH_RECORD_END,"<Idle>");
	QH_SetRecdLines(QH_RECORD_END,0);
#endif

	//�������ڴ����ͷ���
	QHS.dDummy=Q_HEAP_UNIT_REWRITE_ID;
	QHS.dDummy<<=16;
	
	DebugHeap();
	Q_HEAP_EXIT_CRIT;
}

//����һ���洢���¼����¼����
//Index:�����뵽�˼�¼,
//Index�����Ǹ���¼���ڴ�ļ�¼��
static bool InsertRecord(u16 Index,u16 NeedUnit)
{	
	u16 i;

	//QH_Debug("##Insert UnitNum 0x%x @ %d\n\r",NeedUnit,Index);

	if(NeedUnit<QH_GetUnitSize(Index))//��Ҫ�Ŀռ�С�ڿ��пռ�
	{
		//���½���һ���ڴ��¼��
		for(i=(QH_RECORD_START+1);i<QH_RECORD_END;i++)
		{
			if(QH_GetUnitIdx(i)==0)
			{
				//�������ڴ�飬����һ�����п�
				QH_SetPrev(i,Index);//prev����
				QH_SetNext(i,QH_GetNext(Index));//nextת��
				QH_SetUnitIdx(i,QH_GetUnitIdx(Index)+NeedUnit);
				QH_SetUsedFlag(i,QH_UNUSE);

				//����ԭ�����е��ڴ��֮����ڴ��
				QH_SetPrev(QH_GetNext(Index),i);
				
				//����Ҫʹ�õ��ڴ��
				QH_SetUsedFlag(Index,QH_USED);//���ʹ�ñ�־
				QH_SetNext(Index,i);//nextת��
				
#if Need_Debug
				//DebugHeap();
#endif
				return TRUE;
			}
		}
		
		if(i==QH_RECORD_END)
		{
			Debug("!!!Record Num is not enough!!!\n\r");
		}
		return FALSE;
	}
	else //�����Ҫ������ڴ��Ϳ����ڴ��պ����
	{
		QH_SetUsedFlag(Index,QH_USED);//��Ǳ�ʹ��
		return TRUE;
	}
}

//UnitIdxΪ��ʼ��Ԫ��������HEAP_MINI_UNITΪ��λ
//����0��ʾ���󣬷��򷵻�free���ڴ���С
#if Q_HEAP_TRACK_DEBUG == 1
static u16 DeleteRecord(u16 UnitIdx,u8 *pFuncName,u32 Lines)
#else
static u16 DeleteRecord(u16 UnitIdx)
#endif
{
	u16 Index,Size=0;
	u16 i;

	//Debug("DeleteRecord Unit 0x%x\n\r",UnitIdx);

	for(Index=QH_RECORD_START;;Index=QH_GetNext(Index))
	{
		if(QH_GetUnitIdx(Index)==UnitIdx)//�ҵ�ƥ������
		{
#if Q_HEAP_TRACK_DEBUG == 1
			Debug("##Free   Func:%s Line:%d (For %s %d)\n\r",pFuncName,Lines,QH_GetRecdName(Index),QH_GetRecdLines(Index));
			QH_SetRecdName(Index,"<Idle>");
			QH_SetRecdLines(Index,0);
#endif
			Size=QH_Unit2Byte(QH_GetUnitSize(Index));//��ȡ�ڴ���С
			
			QH_SetUsedFlag(Index,QH_UNUSE);//���õ�ǰΪδʹ��
			
			//�ȿ�ǰ��һ���ǲ��ǿյ�
			i=QH_GetPrev(Index);
			if(QH_GetUsedFlag(i)==QH_UNUSE)//ǰ��һ���ǿյģ��ϲ�ǰ��һ��
			{
				QH_SetNext(i,QH_GetNext(Index));
				QH_SetPrev(QH_GetNext(Index),i);
				QH_CleanRecord(Index);		
				Index=i;
			}

			//�ٿ�����һ���ǲ��ǿյ�
			i=QH_GetNext(Index);
			if(QH_GetUsedFlag(i)==QH_UNUSE)
			{
				QH_SetNext(Index,QH_GetNext(i));
				QH_SetPrev(QH_GetNext(i),Index);
				QH_CleanRecord(i);
			}

#if Need_Debug
			//DebugHeap();
#endif
			return Size;
		}
		
		if(QH_GetNext(Index)==QH_RECORD_END) break;//�������һ�����ˣ��˳���
	}

	return 0;	
}

#if Q_HEAP_TRACK_DEBUG == 1
void *QS_Mallco(u16 Size,u8 *pFuncName,u32 Lines)
#else
void *QS_Mallco(u16 Size)
#endif
{
	u16 UnitNum,Index;
	u16 *Mem=NULL;
	Q_HEAP_CRIT_SAVE_DATA_DEFINE;

	if(Size==0) return NULL;
	
	UnitNum=QH_Byte2Unit(Size);//����Ҫ������Ԫ
	
	//���һ����Ԫ��������֤�ڴ�ͷβ�Ƿ񱻳�١�
	//���Size�����ͱ���С��Ԫ��������С2����С3���Ͳ��ö����һ����Ԫ�ˡ�
	Size%=Q_HEAP_MINI_UNIT;
	if(Size==0||(Q_HEAP_MINI_UNIT-Size)<2) UnitNum++;

	Q_HEAP_ENTER_CRIT;
	for(Index=QH_RECORD_START;;Index=QH_GetNext(Index))
	{
		if(QH_GetUsedFlag(Index)==QH_UNUSE)//�ҵ�һ���յĵ�Ԫ
		{
			if(QH_GetUnitSize(Index)>=UnitNum)
			{
				if(InsertRecord(Index,UnitNum)==TRUE)
				{
					Mem=(void *)((u32)QHS.Heap+QH_Unit2Byte(QH_GetUnitIdx(Index)));
					Mem[(QH_Unit2Byte(UnitNum)>>1)-1]=Q_HEAP_UNIT_REWRITE_ID;
					QH_Debug("##Mallco Size %d, Addr 0x%x,Unit 0x%x\n\r",QH_Unit2Byte(UnitNum),(u32)Mem,QH_GetUnitIdx(Index));
#if Q_HEAP_TRACK_DEBUG == 1
					QH_SetRecdName(Index,pFuncName);
					QH_SetRecdLines(Index,Lines);
					Debug("##Mallco Func:%s Line:%d\n\n\r",pFuncName,Lines);
#endif
					Q_HEAP_EXIT_CRIT;		
					return (void *)Mem;
				}
				else 
					break;
			}
		}			
		
		if(QH_GetNext(Index)==QH_RECORD_END) 
			break;
	}
	Q_HEAP_EXIT_CRIT;
	
	DebugHeap();
	QS_MonitorFragment();
	Debug("!!!No Get Heap!!!\n\r");
	return 0;
//	OS_SchedLock();
//	while(1);
}

#if Q_HEAP_TRACK_DEBUG == 1
bool QS_Free(void *Ptr,u8 *pFuncName,u32 Lines)
#else
bool QS_Free(void *Ptr)
#endif
{	
	u16 *Mem=Ptr;
	u16 Size;
	Q_HEAP_CRIT_SAVE_DATA_DEFINE;

	if(Ptr)
	{
		Q_HEAP_ENTER_CRIT;

#if Q_HEAP_TRACK_DEBUG == 1
		Size=DeleteRecord(QH_Byte2Unit((u32)Ptr-(u32)QHS.Heap),pFuncName,Lines);
#else
		Size=DeleteRecord(QH_Byte2Unit((u32)Ptr-(u32)QHS.Heap));
#endif

		if(Size==0)
		{
			Debug("##Free Error!!!Can not find the match memory!!!0x%x\n\r",(u32)Ptr);
#if Q_HEAP_TRACK_DEBUG == 1
			Debug("##Free   Func:%s Line:%d\n\r",pFuncName,Lines);
#endif
			while(1);
		}
		else 
			QH_Debug("##Free   Size %d, Addr 0x%x,Unit 0x%x\n\n\r",Size,(u32)Ptr,QH_Byte2Unit((u32)Ptr-(u32)QHS.Heap));
			
		//����ڴ������
		if(Mem[(Size>>1)-1]!=Q_HEAP_UNIT_REWRITE_ID)
		{
			Debug("Memory end be rewrited!!!%x %d\n\r",Mem[(Size>>1)-1],Size);
#if Q_HEAP_TRACK_DEBUG == 1
			Debug("##Free   Func:%s Line:%d\n\r",pFuncName,Lines);
#endif
			while(1);
		}

		Mem--;
		if(Mem[0]!=Q_HEAP_UNIT_REWRITE_ID)
		{
			Debug("Memory header be rewrited!!!%x\n\r",Mem[0]);
#if Q_HEAP_TRACK_DEBUG == 1
			Debug("##FuncName:%s Lines:%d\n\r",pFuncName,Lines);
#endif
			while(1);
		}
				
		Q_HEAP_EXIT_CRIT;
		return TRUE;
	}
	else
		return FALSE;
}

//�����ڴ���Ƭ���
/*Ϊ�������ڴ���Ƭ�ж��٣�������һ��������fragindices����������
fragindices������Ϊ��
fragindices = (float)MaxSize/(float)TotalSize
����MaxSize��ʾ�������ɷ�������������ڴ��С
TotalSize��ʾ�������п����ڴ���ܺ�
�ɼ���ϵͳ����Ƭʱfragindices = 1
����ϵͳ���ڴ�ɷ���ʱfragindices = 0 
fragindicesԽС��ʾ��ƬԽ��*/
void QS_MonitorFragment(void)
{
	u16 Index,Size,MaxSize=0,TotalSize=0;
	
	for(Index=QH_RECORD_START;;Index=QH_GetNext(Index))
	{ 
		if(QH_GetUsedFlag(Index)==QH_UNUSE)
		{
			Size=QH_Unit2Byte(QH_GetUnitSize(Index));//��ȡ�ڴ���С
			TotalSize+=Size;
			if(Size>MaxSize)
				MaxSize=Size;
		}
		if(QH_GetNext(Index)==QH_RECORD_END) break;
	}

	Debug("********** Heap Monitor **********\n\r");
	Debug(" TotalFreeMem=%d Byte\n\r",TotalSize);
	Debug(" MaxFreeMem=%d Byte\n\r",MaxSize);
	Debug(" Fragindices=%.2f\n\r",(float)MaxSize/(float)TotalSize);
	Debug("**********************************\n\r");
}

//���������ڴ���Ƭ
bool QS_HeapArrange(void)
{//unfinish
	return TRUE;
}

//���õ�ַ�Ƿ����ڶѿռ�
bool IsHeapRam(void *p)
{
	if(((u32)p >= (u32)QHS.Heap)&&((u32)p < ((u32)QHS.Heap+Q_HEAP_SIZE_BYTE)))
		return TRUE;

	return FALSE;
}


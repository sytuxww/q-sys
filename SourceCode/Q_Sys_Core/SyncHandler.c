#include "System.h"

bool Q_PageSync(int IntParam,void *pSysParam,u32 TicksToWait)
{
	INPUT_EVENT SyncEvent;

	SyncEvent.uType=Sync_Type;
	SyncEvent.EventType=Input_PageSync;
	SyncEvent.Info.SyncInfo.IntParam=IntParam;
	SyncEvent.Info.SyncInfo.pParam=pSysParam;
	if(OS_ERR_NONE!=OS_MsgBoxSend(gInputHandler_Queue,&SyncEvent,TicksToWait,FALSE))
	{
		Debug("Page Sync Queue Send Faile!\n\r");
		return FALSE;
	}
	return TRUE;
}


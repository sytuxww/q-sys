
//����ҳ����AllPage�е�������ȡҳ��
const PAGE_ATTRIBUTE *GetPageByIdx(u8 PageIdx)
{
	return AllPage[PageIdx].pPage;
}

//����ҳ����AllPage�е�������ȡҳ��RID
PAGE_RID GetRegIdByIdx(u8 PageIdx)
{
	return AllPage[PageIdx].RegID;
}

//�õ���Ե�ǰҳ��Local����ҳ������
//Local=0,���ص�ǰҳ������
//Local=1,����ǰһҳ������
//Local=2,3,4...�Դ�����
u8 GetPageIdxByTrack(u8 Local)
{
	if(Local>=MAX_PAGE_TRACK_NUM)//��鷶Χ
	{
		Debug("GetPageIdxByTrack error!(%d>=?%d)\n\r",Local,MAX_PAGE_TRACK_NUM);
		Q_ErrorStopScreen("GetPageIdxByTrack error!");
	}
	
	Local=sizeof(PageTracks)+gCurTrackIdx-Local;
	while(Local>=sizeof(PageTracks))
	{
		Local-=sizeof(PageTracks);
	}
	return PageTracks[Local];
}

//����һ��ҳ����������¼
static void InsertPageTrack(u8 PageIdx)
{
	OS_DeclareCritical();
	
	OS_EnterCritical();
	if(++gCurTrackIdx==sizeof(PageTracks)) gCurTrackIdx=0;
	
	PageTracks[gCurTrackIdx]=PageIdx;
	OS_ExitCritical();
}

//��ȡ��ǰ�㼶��
u8 GetCurLayerNum(void)
{
	return gCurLayerNum;
}

//�õ���Ե�ǰ���ָ����ҳ������
//LayerOffset=0,���ص�ǰҳ������
//LayerOffset=1,������һ��ҳ������
//�Դ�����
u8 GetPageIdxByLayerOffset(u8 LayerOffset)
{
	if(LayerOffset>gCurLayerNum)//��鷶Χ
	{
		Debug("GetPageIdxByLayerOffset error!(%d>?%d)\n\r",LayerOffset,gCurLayerNum);
		while(1);
	}
	
	return PageLayers[gCurLayerNum-LayerOffset];
}

//�õ�ָ�����ҳ������
//LayerNum=1,�õ�����
//LayerNum=2,�õ��ڶ���
u8 GetPageIdxByLayer(u8 LayerNum)
{
	if(LayerNum>MAX_PAGE_LAYER_NUM)
	{
		Debug("GetPageIdxByLayer error! LayerNum=%d\n\r",LayerNum);
		Q_ErrorStopScreen("GetPageIdxByLayer error! ");
	}

	return PageLayers[LayerNum];
}

//�½�һ��
static void InsertPageLayer(u8 PageIdx)
{
	OS_DeclareCritical();
	
	OS_EnterCritical();
	if(++gCurLayerNum<=MAX_PAGE_LAYER_NUM)//�жϴ洢�ռ乻����
		PageLayers[gCurLayerNum]=PageIdx;
	else
	{
		Q_ErrorStopScreen("Layer is overflow,pls set marco MAX_PAGE_LAYER_NUM\n\r");
	}
	OS_ExitCritical();
}

//ɾ����
static void DeletePageLayer(u8 Num)
{
	OS_DeclareCritical();
	
	OS_EnterCritical();

	if(Num>=gCurLayerNum) //������ڵ�ǰ��������ɾ�����в�
		gCurLayerNum=0;
	else
		gCurLayerNum-=Num;
	OS_ExitCritical();
}

//ͨ��pPageIdx�����ҵ���ҳ����AllPage�е�����
SYS_MSG FindPage(u8 *Name, PAGE_RID RegID,u8 *pPageIdx)
{
	u8 PageIdx;
	
	if(Name&&Name[0])
	{
		//��page name���в���		
		for(PageIdx=0;PageIdx<PAGE_TOTAL;PageIdx++)
		{
			if(strcmp((void *)Name,(void *)GetPageByIdx(PageIdx)->Name)) continue;
			else  break;//�ҵ�ָ����ҳ����
		}

		if(PageIdx==PAGE_TOTAL) //û�ҵ�ָ����ҳ��
		{
			Debug("No Such Page Name:%s ,may be you don't push it to AllPage.h\n\r",Name);
			return SM_State_Faile;
		}
	}
	else	if(RegID)
	{
		//��Ψһ��RegID���в���
		for(PageIdx=0;PageIdx<PAGE_TOTAL;PageIdx++)
		{
			if(RegID!=GetRegIdByIdx(PageIdx)) continue;
			else break;//�ҵ���
		}

		if(PageIdx==PAGE_TOTAL) //û�ҵ�ָ����ҳ��
		{
			Debug("No Such Page RegID:%d\n\r",RegID);
			return SM_State_Faile;
		}
	} 
	else return SM_State_Faile;

	*pPageIdx=PageIdx;
	return SM_State_OK;
}


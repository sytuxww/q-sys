
//根据页面在AllPage中的索引获取页面
const PAGE_ATTRIBUTE *GetPageByIdx(u8 PageIdx)
{
	return AllPage[PageIdx].pPage;
}

//根据页面在AllPage中的索引获取页面RID
PAGE_RID GetRegIdByIdx(u8 PageIdx)
{
	return AllPage[PageIdx].RegID;
}

//得到相对当前页面Local步的页面索引
//Local=0,返回当前页面索引
//Local=1,返回前一页面索引
//Local=2,3,4...以此类推
u8 GetPageIdxByTrack(u8 Local)
{
	if(Local>=MAX_PAGE_TRACK_NUM)//检查范围
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

//插入一个页面索引到记录
static void InsertPageTrack(u8 PageIdx)
{
	OS_DeclareCritical();
	
	OS_EnterCritical();
	if(++gCurTrackIdx==sizeof(PageTracks)) gCurTrackIdx=0;
	
	PageTracks[gCurTrackIdx]=PageIdx;
	OS_ExitCritical();
}

//获取当前层级数
u8 GetCurLayerNum(void)
{
	return gCurLayerNum;
}

//得到相对当前层的指定层页面索引
//LayerOffset=0,返回当前页面索引
//LayerOffset=1,返回上一层页面索引
//以此类推
u8 GetPageIdxByLayerOffset(u8 LayerOffset)
{
	if(LayerOffset>gCurLayerNum)//检查范围
	{
		Debug("GetPageIdxByLayerOffset error!(%d>?%d)\n\r",LayerOffset,gCurLayerNum);
		while(1);
	}
	
	return PageLayers[gCurLayerNum-LayerOffset];
}

//得到指定层的页面索引
//LayerNum=1,得到顶层
//LayerNum=2,得到第二层
u8 GetPageIdxByLayer(u8 LayerNum)
{
	if(LayerNum>MAX_PAGE_LAYER_NUM)
	{
		Debug("GetPageIdxByLayer error! LayerNum=%d\n\r",LayerNum);
		Q_ErrorStopScreen("GetPageIdxByLayer error! ");
	}

	return PageLayers[LayerNum];
}

//新建一层
static void InsertPageLayer(u8 PageIdx)
{
	OS_DeclareCritical();
	
	OS_EnterCritical();
	if(++gCurLayerNum<=MAX_PAGE_LAYER_NUM)//判断存储空间够不够
		PageLayers[gCurLayerNum]=PageIdx;
	else
	{
		Q_ErrorStopScreen("Layer is overflow,pls set marco MAX_PAGE_LAYER_NUM\n\r");
	}
	OS_ExitCritical();
}

//删除层
static void DeletePageLayer(u8 Num)
{
	OS_DeclareCritical();
	
	OS_EnterCritical();

	if(Num>=gCurLayerNum) //如果大于当前层数，则删除所有层
		gCurLayerNum=0;
	else
		gCurLayerNum-=Num;
	OS_ExitCritical();
}

//通过pPageIdx返回找到的页面在AllPage中的索引
SYS_MSG FindPage(u8 *Name, PAGE_RID RegID,u8 *pPageIdx)
{
	u8 PageIdx;
	
	if(Name&&Name[0])
	{
		//用page name进行查找		
		for(PageIdx=0;PageIdx<PAGE_TOTAL;PageIdx++)
		{
			if(strcmp((void *)Name,(void *)GetPageByIdx(PageIdx)->Name)) continue;
			else  break;//找到指定的页面了
		}

		if(PageIdx==PAGE_TOTAL) //没找到指定的页面
		{
			Debug("No Such Page Name:%s ,may be you don't push it to AllPage.h\n\r",Name);
			return SM_State_Faile;
		}
	}
	else	if(RegID)
	{
		//用唯一的RegID进行查找
		for(PageIdx=0;PageIdx<PAGE_TOTAL;PageIdx++)
		{
			if(RegID!=GetRegIdByIdx(PageIdx)) continue;
			else break;//找到了
		}

		if(PageIdx==PAGE_TOTAL) //没找到指定的页面
		{
			Debug("No Such Page RegID:%d\n\r",RegID);
			return SM_State_Faile;
		}
	} 
	else return SM_State_Faile;

	*pPageIdx=PageIdx;
	return SM_State_OK;
}


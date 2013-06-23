#include "System.h"  
#include "MusicHandler.h"

extern void MusicHandler_Task(void *Task_Parameters);
extern void TouchHandler_Task(void *Task_Parameters );
extern void QWebHandler_Task(void *Task_Parameters);
extern void KeysHandler_Task(void *Task_Parameters);
extern bool DB_Init(void);

//文件系统

//所有的事件机制必须定义到此处并初始化在本文件
//这样方便统计
OS_MutexHandler gLCD_Mutex=NULL;//用于液晶屏互斥
//OS_MutexHandler gBmpObj_Mutex=NULL;//用于gui访问文件系统互斥
//OS_MutexHandler SD_ReadMutex=NULL;  //karlno add

OS_SemaphoreHandle gTouchHandler_Sem=NULL;//触摸屏有点击的信号量
OS_SemaphoreHandle gAllowTchHandler_Sem=NULL;//酷系统内核允许按键输入的信号量
OS_SemaphoreHandle gVsDreq_Sem=NULL;//vs准备好的信号量
OS_SemaphoreHandle gRfRecvHandler_Sem=NULL;//射频接收数据的信号量

//OS_MutexHandler gSD_Mutex=NULL;  //karlno add

OS_MsgBoxHandle gInputHandler_Queue=NULL;//通过此队列传递触发的事件
OS_MsgBoxHandle gMusicHandler_Queue=NULL;//通过此队列传递命令给Music处理线程

/*******************************************************************************
* 函数: void GPIO_Config(void)
* 功能: gpio配置，针对每个模块的gpio配置推荐放到模块初始化文件中
* 参数: 无
* 返回: 无
*******************************************************************************/
void GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	//使能按键GPIO端口
	//同时也要使能AFIO时钟，使用引脚的重映射功能和外部中断时需要使能AFIO的时钟。
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
												  RCC_APB2Periph_GPIOE | 
												  RCC_APB2Periph_AFIO, ENABLE);

#if 1//initailize pc13 for rtc hardware bug
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
#endif
}

/*******************************************************************************
* 函数: void NVIC_Config(void)
* 功能: 中断矢量配置，针对每个模块的中断配置推荐放到模块初始化文件中
* 参数: 无
* 返回: 无
*******************************************************************************/
void NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);	 //将中断矢量放到Flash的0地址
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);		//只适用抢占优先级PreemptionPriority
  
	//配置sd中断
	NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = SDIO_IRQn_Priority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/*******************************************************************************
* 函数: void EXTI_Config(void)
* 功能: 外部中断的配置函数
* 参数: 无
* 返回: 无
*******************************************************************************/
void EXTI_Config(void)
{
#if 0
	EXTI_InitTypeDef EXTI_InitStructure;  

	//配置中断线2为边降沿触发 
	EXTI_InitStructure.EXTI_Line = EXTI_Line2;             //外部中断线 ，使用第2根
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;    //中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;//中断触发方式，双沿触发
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;              //打开中断
	EXTI_Init(&EXTI_InitStructure);    //调用库函数给寄存器复制

	//连接IO口到中断线
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource2); //选择 GPIOE_2管脚用作外部中断线路
#endif
}

static void SetupHardware( void )
{	
	u8 buf[32];
#if 0//debug by karlno
	u8 *ptr1,*ptr2,*ptr3,*ptr4;
#endif

	GPIO_Config();//针对gpio的一些配置
	NVIC_Config();//针对中断向量的一些配置
	EXTI_Config();//外部中断配置
	
#if 0//debug by karlno
	Debug("#### 123\n\r");

	ptr1=Q_Mallco(200);
	ptr2=Q_Mallco(68);

	Q_Free(ptr1);
	ptr3=Q_Mallco(68);

	ptr3=Q_Mallco(68);

while(1);
#endif
	
	
	Debug("\n\n\n\r************************************\n\r");
	Debug("* Q-SYS %s               *\n\r",QSYS_VERSION);
	Debug("* Start up our dreams!             *\n\r");
#if OS_USE_FREERTOS
	Debug("* Base on FreeRTOS                 *\n\r");
#elif OS_USE_UCOS
	Debug("* Base on uC/OS                    *\n\r");
#endif
	Debug("* Hardware PID : %d %d           *\n\r",QXW_PRODUCT_ID,QXW_LCM_ID);
	Debug("* Hardware ID : %X           *\n\r",GetHwID());
	Debug("* Www.Q-ShareWe.Com                *\n\r");
	Debug("************************************\n\r\n\r");	

	M25P16_Init();//spi flash

	M25P16_Read_Id(buf);
	Debug("SPI Flash ID:\n\r");
	DisplayBuf(buf,20,8);
	Debug("\n\r");
	
	SPI_Touch_Init();//触摸屏

	VsInit();//Vs1003的配置

	//sd卡文件系统初始化
	if(disk_initialize(0))
	{
		Debug("Disk Initialize error!\n\r");
	}
	else
	{
		Debug("Disk Initialize OK!\n\r");
#if 0//debug
	{
	FIL fsrc;            // file objects
	FRESULT res;         // FatFs function common result code
	UINT br;   
	u8 *p=Q_Mallco(1024);
	u32 time=QW_GetNowTimeMs();


	res = f_mount(0,&FS);	//  初始化分区结构，它不初始化SD卡
	if(res == FR_OK)
    {
		Debug("\n\nATA mounrt OK, fs_type =  %d\n\r",FS.fs_type);
	}
    else
    {
		Debug("\nATA mounrt Error!!!\n%d\n\r",res);	 
	}

	res = f_open(&fsrc,"1.rar",FA_READ) ;	
	Debug("Open %d\n\r",res);
	while(1)
	{
	res = f_read(&fsrc,p,fsrc.fsize,&br); //  读文件数据，长度file.fsize，
		if(br==0)break;
	}
    f_close(&fsrc);
    Debug("Time Gap:%d\n\r",QW_GetNowTimeMs()-time);
    while(1);
	}
#endif
		FS_Init();
		Debug("File system mount OK!\n\r");
		Q_DB_SetStatus(Status_FsInitFinish,TRUE,NULL,0);
	}

	Adc_Init();
	
	Tim2_Init();//用户定时器
	Tim3_Init();//背光pwm初始化
	Tim4_Init();//用户定时器
	Tim5_Init();//用户定时器
	
	USB_SetHw();
    USB_Init();
}



//根据指定的配置文件烧写sd卡中的文件到flash
//ForceDownload表示是否强制下载而不比对头16字节的内容
//CfgPath指定配置文件路径
//自定义内容请放到15扇区之后
//配置项内容请勿超过256字节
#define CfgFileSize	256 //配置内容不得大于这个数的字节
#define MaxSecItem	8	//支持8个扇区配置项
#define MaxDownItem	8	//每个扇区配置项可以包含8个下载配置
void SpiFlashDownFromSD(bool ForceDownload,const u8 *CfgPath,u8 *FileBuf,u8 *PageBuf)
{
	u8 Buffer2[16];
	u8 *pTmp;
	u8 *pSecStart[MaxSecItem];
	u8 *pSecEnd[MaxSecItem];
	u8 *pAddr[MaxDownItem];
	u8 *pBinPath[MaxDownItem];
	u8 EarseSecFlag;	//存储每个文件的匹配结果
	u8 CurSecItem=0;//当前解析的扇区配置项
	u8 CurDownItemNum=0;//当前扇区项下面的下载项个数
	u8 *pCfg;
	int i,j;
	UINT ReadByte;
	u32 SecStart,SecEnd;
	FS_FILE *pFileObj;

	//打开配置文件
	if((CfgPath[0]==0)||(CfgPath==NULL))	return;
	if ((pFileObj=FS_FOpen((void *)CfgPath, FA_OPEN_EXISTING | FA_READ)) == 0 ) 
	{
		Debug("Cannot open file \"%s\"\n\r",CfgPath);
		return;
	}	

	//读取配置文件
	if((ReadByte=FS_FRead((void *)FileBuf, CfgFileSize,1,pFileObj ))==0)
	{
		Debug("Read cfg file error!\n\r");
		FS_FClose(pFileObj);
		return;		
	}	
	//Debug("Cfg content:\n\r%s\n\r",Cfg);
	
	//关闭配置文件
	if( FS_FClose(pFileObj)  == -1 )
	{
		Debug("Close file error\n");
		return;
	}	

	pCfg=FileBuf;
	
	//检查配置文件版本
	if((pTmp=(void *)strstr((void *)pCfg,"#"))==NULL)	return;
	if((pCfg=(void *)strstr((void *)pTmp,";"))==NULL)	return;
	pTmp++;
	*pCfg++=0;

	if(strcmp((void *)pTmp,"m25p16 1.0"))
	{
		Debug("Cfg file %s version is error!(%s!=%s)\n\r",CfgPath,"m25p16 1.0",pTmp);
		return;
	}

	//检查配置项完整性
	if((pTmp=(void *)strstr((void *)pCfg,"$"))==NULL) return;
	*pTmp++=0;

	//开始提取配置文件内容
	while(1)
	{
		*pTmp='%';//恢复上一个扇区项
		
		//先获取扇区项
		if((pSecStart[CurSecItem]=(void *)strstr((void *)pCfg,"%"))==NULL)	return;
		if((pSecEnd[CurSecItem]=(void *)strstr((void *)pSecStart[CurSecItem],"-"))==NULL)	return;

		if((pCfg=(void *)strstr((void *)pSecEnd[CurSecItem],";"))==NULL)	return;

		*pSecStart[CurSecItem]++=0;
		*pSecEnd[CurSecItem]++=0;
		*pCfg++=0;

		Debug("Current SectorItem:%d,SecStart:%s,SecEnd:%s\n\r",CurSecItem,pSecStart[CurSecItem],pSecEnd[CurSecItem]);
		SecStart=StrToUint((void *)pSecStart[CurSecItem]);
		SecEnd=StrToUint((void *)pSecEnd[CurSecItem]);
		if(SecStart>SecEnd)	return;//扇区错误
		if(SecEnd>31)	return;//扇区错误
		EarseSecFlag=0;//置0擦除标志

		if((pTmp=(void *)strstr((void *)pCfg,"%"))!=NULL)	 *pTmp=0;//先屏蔽下一个扇区项
		else pTmp=FileBuf;

		//再获取下载项
		for(i=0;i<MaxDownItem;i++)
		{
			if((pBinPath[i]=(void *)strstr((void *)pCfg,"="))==NULL)	return;//找到文件路径
			if((pAddr[i]=(void *)strstr((void *)pBinPath[i],"@"))==NULL)	return;//找到起始页数
			if((pCfg=(void *)strstr((void *)pAddr[i],";"))==NULL)	return;//找到本条结尾位置

			*pBinPath[i]++=0;
			*pAddr[i]++=0;
			*pCfg++=0;
			Debug("=%s download to page %s\n\r",pBinPath[i],pAddr[i]);

			if(strstr((void *)pCfg,"=")==NULL)	//也没有下一个下载项了
			{
				i++;
				break;
			}
		}
		CurDownItemNum=i;
		Debug("Current DownItem Max Num:%d\n\r",CurDownItemNum);

		if(!ForceDownload)
		{
			for(i=0;i<CurDownItemNum;i++)
			{
				//打开bin文件
				if ((pFileObj=FS_FOpen((void *)pBinPath[i], FA_OPEN_EXISTING | FA_READ)) ==0 ) 
				{
					Debug("Cannot open file %s,Cancle download\n\r",pBinPath[i]);
					return;
				}	
			
				//对比每个下载项文件和flash page的头16字节内容
				if((ReadByte=FS_FRead((void *)PageBuf, sizeof(Buffer2), 1,pFileObj)) != 0)
				{
					if(ReadByte)
					{
						//读取spi flash内容进行匹配
						Q_SpiFlashSync(FlashRead,SPI_FLASH_PAGE_SIZE*StrToUint((void *)pAddr[i]),sizeof(Buffer2),Buffer2);
						for(j=0;j<sizeof(Buffer2);j++)
						{
							//Debug("%x?=%x\n\r",Buffer2[j],Buffer[j]);
							if(Buffer2[j]!=PageBuf[j])
							{
								EarseSecFlag=1;
								Debug("We need erase sectors becase of file %s first %d bytes.\n\r",pBinPath[i],sizeof(Buffer2));
								if( FS_FClose(pFileObj)  == -1 )	Debug("Close file %s error\n",pBinPath[i]);
								goto Erase;
							}
						}

						//if(j==sizeof(Buffer2))
						//{
						//	Debug("We needn't download file %s to spi flash\n\r",pBinPath[i]);
						//}
					}
					else //没有读到内容
					{
						Debug("File %s is NULL,cancle download\n\r",pBinPath[i]);
						return;
					}
				}

				//对比每个下载项文件和flash page的尾16字节内容
				FS_FSeek(pFileObj,-sizeof(Buffer2),FS_SEEK_END);
				if((ReadByte=FS_FRead((void *)PageBuf, sizeof(Buffer2), 1,pFileObj)) != 0)
				{
					if(ReadByte)
					{
						//读取spi flash内容进行匹配
						Q_SpiFlashSync(FlashRead,
							SPI_FLASH_PAGE_SIZE*StrToUint((void *)pAddr[i])+FS_GetFileSize(pFileObj)-sizeof(Buffer2),
							sizeof(Buffer2),Buffer2);
						for(j=0;j<sizeof(Buffer2);j++)
						{
							//Debug("%x?=%x\n\r",Buffer2[j],Buffer[j]);
							if(Buffer2[j]!=PageBuf[j])
							{
								EarseSecFlag=1;
								Debug("We need erase sectors becase of file %s last %d byte.\n\r",pBinPath[i],sizeof(Buffer2));
								if( FS_FClose(pFileObj)  == -1 )	Debug("Close file %s error\n",pBinPath[i]);
								goto Erase;
							}
						}

						if(j==sizeof(Buffer2))
						{
							Debug("We needn't download file %s to spi flash\n\r",pBinPath[i]);
						}
					}
					else //没有读到内容
					{
						Debug("File %s is NULL,cancle download\n\r",pBinPath[i]);
						return;
					}
				}
				
				if( FS_FClose(pFileObj)  == -1 )	Debug("Close file %s error\n",pBinPath[i]);
			}						
		}

Erase:
		//擦除扇区
		if(EarseSecFlag||ForceDownload)
		{
			for(i=SecStart;i<=SecEnd;i++) 
			{
				Debug("Erase sector %d(page %d to %d)\n\r",i,i<<8,(i<<8)+256);
				Q_SpiFlashSync(FlashSectorEarse,i<<16,0,NULL);
			}
		
			//烧录文件
			for(i=0;i<CurDownItemNum;i++)
			{
				Debug("#Download %s ",pBinPath[i]);
				if ((pFileObj=FS_FOpen((void *)pBinPath[i], FA_OPEN_EXISTING | FA_READ)) == 0 ) 
				{
					Debug("Cannot open file %s\n\r",pBinPath[i]);
					return;
				}	
				
				for(ReadByte=0,j=StrToUint((void *)pAddr[i]);;j++)
				{					
					if((ReadByte=FS_FRead((void *)PageBuf, SPI_FLASH_PAGE_SIZE, 1,pFileObj)) != 0)
					{//读到非0个数据
						Q_SpiFlashSync(FlashWrite,j*SPI_FLASH_PAGE_SIZE,ReadByte,PageBuf);
						Debug("."); //显示进度条
					}
					else //读到0个数据，说明读到文件末了
					{
						break;
					}
				}
				Debug("\n\rHave Download \"%s\" to spi flash,from page %d to page %d\n\r",pBinPath[i],StrToUint((void *)pAddr[i]),j-1);

				if( FS_FClose(pFileObj)  == -1 )	Debug("Close file %s error\n",pBinPath[i]);
			}
		}

		if(++CurSecItem==MaxSecItem) return;//下一个扇区项
	}
}
void *MusicHandler_Task_Handle=NULL;
void *TouchHandler_Task_Handle=NULL;
void *KeysHandler_Task_Handle=NULL;
void *QWebHandler_Task_Handle=NULL;
void QSYS_Init(void)
{	
	u8 *ptr1;
	u8 *ptr2;
//	FIL *ptr3;
	
	OS_CPU_SysTickInit();//Initialize the SysTick.
#if OS_USE_UCOS
	CPU_IntSrcPrioSet(CPU_INT_PENDSV,15);
	CPU_IntSrcPrioSet(CPU_INT_SYSTICK,15);
#endif
	SetupHardware();

	Debug("sizeof(INPUT_EVENT)=%d\n\r",sizeof(INPUT_EVENT));//for debug by karlno
	Debug("sizeof(PAGE_ATTRIBUTE)=%d\n\r",sizeof(PAGE_ATTRIBUTE));//for debug by karlno
	Debug("sizeof(IMG_BUTTON_OBJ)=%d\n\r",sizeof(IMG_BUTTON_OBJ));//for debug by karlno
	Debug("sizeof(CHAR_BUTTON_OBJ)=%d\n\r",sizeof(CHAR_BUTTON_OBJ));//for debug by karlno
	Debug("sizeof(MUSIC_EVENT)=%d\n\r",sizeof(MUSIC_EVENT));//for debug by karlno
	//Debug("sizeof(QSYS_MSG_BOX)=%d\n\r",sizeof(QSYS_MSG_BOX));//for debug by karlno

	if(SysEvt_MaxNum>32) 
	{
		Debug("Error:SysEvt_MaxNum(%d) is too big!!!\n\r",SysEvt_MaxNum);
		Q_ErrorStopScreen("Error:SysEvt_MaxNum is too big!!!\n\r");
	}
	
	gLCD_Mutex=OS_MutexCreate();

	//创建触摸输入中断发生信号量
	gTouchHandler_Sem=OS_SemaphoreCreate(0);
	gAllowTchHandler_Sem=OS_SemaphoreCreate(0);
	gVsDreq_Sem=OS_SemaphoreCreate(0);
	gRfRecvHandler_Sem=OS_SemaphoreCreate(0);
	
	//创建事件传递数据
	gInputHandler_Queue=OS_MsgBoxCreate("Input Event",sizeof(INPUT_EVENT),16);
	gMusicHandler_Queue=OS_MsgBoxCreate("MusicKV Event",sizeof(MUSIC_EVENT),8);

#if 0//debug

	OS_TaskCreate((void (*) (void *)) T1_Task,(void *) 0,
		(OS_STK *) &T1_TaskStack[OS_MINIMAL_STACK_SIZE - 1],T1_TASK_pRIORITY);

	//OS_TaskCreate(T2_Task , ( signed OS_CHAR * ) "T2 Handler", OS_MINI_STACK_SIZE, NULL, MUSIC_HANDLER_TASK_PRIORITY, NULL );
	OS_TaskDelete(SYSTEM_TASK_PRIORITY);
#endif

#if 1 //首次下载完成后可关闭此处代码，防止误下载
	Debug("\n\r-------------------SPI FLASH DOWNLOAD FROM SD------------------\n\r");
	ptr1=(u8 *)Q_Mallco(CfgFileSize);
	ptr2=(u8 *)Q_Mallco(SPI_FLASH_PAGE_SIZE);
	SpiFlashDownFromSD(FALSE,"System/Down.cfg",ptr1,ptr2);
	Q_Free(ptr2);
	Q_Free(ptr1);
	Debug("-------------------SPI FLASH DOWNLOAD FROM SD------------------\n\r\n\r");
#endif

	Debug("----------------DATABASE SETTING INITIALIZATION----------------\n\r");
	DB_Init();
	Debug("----------------DATABASE SETTING INITIALIZATION----------------\n\r\n\r");
	
	RTC_SetUp();	

	Gui_Init();	//图像库初始化
	Gui_SetBgLight(Q_DB_GetValue(Setting_BgLightScale,NULL));//设置背光亮度

	OS_TaskCreate(MusicHandler_Task,"Music",OS_MINIMAL_STACK_SIZE*8,NULL,MUSIC_TASK_PRIORITY,&MusicHandler_Task_Handle);
	OS_TaskCreate(TouchHandler_Task,"Touch",OS_MINIMAL_STACK_SIZE*3,NULL,TOUCH_TASK_PRIORITY,&TouchHandler_Task_Handle);
	OS_TaskCreate(KeysHandler_Task,"Keys",OS_MINIMAL_STACK_SIZE*2,NULL,KEYS_TASK_PRIORITY,&KeysHandler_Task_Handle);
	OS_TaskCreate(QWebHandler_Task,"QWeb",OS_MINIMAL_STACK_SIZE*8,NULL,RF_DATA_TASK_PRIORITY,&QWebHandler_Task_Handle);
	
	OS_TaskDelay(100);
	OS_TaskStkCheck(FALSE);

	if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)==0) //如果没按下Key-PE2，就正常启动串口中断
	{
#if !QXW_RELEASE_VER//for debug
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//release版本请关掉此句，免得不懂的用户说板卡老死机。
		USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
#endif
	}
}

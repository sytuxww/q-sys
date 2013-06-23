/*
 * FileName:        Rtc.c      
 * Author:          YuanYin  Version: QXW-Summer-V1.x  Date: 2010-4-29
 * Description:     Rtc函数库 
 */
 
/*****************************************************
 *  酷享科技		Q-Share We	快乐-开源-分享		
 *  						   	
 *  如果你对我们的产品有任何建议或不满，请联系我们! 	
 *						   	
 *  淘宝专营：Q-ShareWe.TaoBao.Com			
 *  技术论坛：Www.Q-ShareWe.Com				
 ****************************************************/

#include "Drivers.h"
#include "Rtc.h"
#include "Debug.h"

#define RTC_START_YEAR 1912  //可从1904年后的任何一个闰年开始,如果改变此值，星期初值要改变
#define DAY_SECONDS 86400	//一天的总秒数

//闰年的逐月秒数计算
const int Leap_Month_Seconds[13]={
	0,
	DAY_SECONDS*31,
	DAY_SECONDS*(31+29),
	DAY_SECONDS*(31+29+31),
	DAY_SECONDS*(31+29+31+30),
	DAY_SECONDS*(31+29+31+30+31),
	DAY_SECONDS*(31+29+31+30+31+30),
	DAY_SECONDS*(31+29+31+30+31+30+31),
	DAY_SECONDS*(31+29+31+30+31+30+31+31),
	DAY_SECONDS*(31+29+31+30+31+30+31+31+30),
	DAY_SECONDS*(31+29+31+30+31+30+31+31+30+31),
	DAY_SECONDS*(31+29+31+30+31+30+31+31+30+31+30),
	DAY_SECONDS*(31+29+31+30+31+30+31+31+30+31+30+31),
};

//非闰年的逐月秒数计算
const int Month_Seconds[13]={
	0,
	DAY_SECONDS*31,
	DAY_SECONDS*(31+28),
	DAY_SECONDS*(31+28+31),
	DAY_SECONDS*(31+28+31+30),
	DAY_SECONDS*(31+28+31+30+31),
	DAY_SECONDS*(31+28+31+30+31+30),
	DAY_SECONDS*(31+28+31+30+31+30+31),
	DAY_SECONDS*(31+28+31+30+31+30+31+31),
	DAY_SECONDS*(31+28+31+30+31+30+31+31+30),
	DAY_SECONDS*(31+28+31+30+31+30+31+31+30+31),
	DAY_SECONDS*(31+28+31+30+31+30+31+31+30+31+30),
	DAY_SECONDS*(31+28+31+30+31+30+31+31+30+31+30+31),
};

#define RTCClockSource_LSE // 使用外部时钟， 32.768KHz 

extern unsigned int RTC_Counter;


/*******************************************************************************
* Function Name  : RTC_Configuration
* Description    : Configures the rtc.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RTC_Configuration(void)
{
    u32 rtcintcnt=0x200000;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);  //打开电源管理和备份寄存器时钟
    PWR_BackupAccessCmd(ENABLE);            //使能RTC和备份寄存器的访问(复位默认关闭)
    BKP_DeInit();                           //BKP外设复位
    RCC_LSEConfig(RCC_LSE_ON);              //打开外部低速晶体
    
    while((RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) && (--rtcintcnt));//等待LSE准备好
    if(rtcintcnt!=0)//内部晶振
    {
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE); //选择LSE位RTC时钟
        RCC_RTCCLKCmd(ENABLE);                  //使能RTC时钟
        RTC_WaitForSynchro();                   //等待RTC寄存器和APB时钟同步
        RTC_WaitForLastTask();                  //等待RTC寄存器写操作完成(必须在对RTC寄存器写操作钱调用)
        RTC_ITConfig(RTC_IT_SEC, ENABLE);       //使能RTC秒中断
        RTC_WaitForLastTask();                  //等待RTC寄存器写操作完成
        RTC_ITConfig(RTC_IT_ALR, ENABLE);
        RTC_WaitForLastTask();                  //等待RTC寄存器写操作完成
        RTC_SetPrescaler(32767);                //设置RTC预分频器值产生1秒信号计算公式 fTR_CLK = fRTCCLK/(PRL+1)
        RTC_WaitForLastTask();   
    }
    else//用外部晶振
    {
    	Debug("!!!RTC LSE NO WORK!!!\n\r");
        rtcintcnt=0x200000;    
        RCC_HSEConfig(RCC_HSE_ON);/* Enable HSE */
        while ( (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET) && (--rtcintcnt) );/* Wait till HSE is ready */
        if ( rtcintcnt == 0 )
        {
            return;
        }
        RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);/* Select HSE/128 as RTC Clock Source */
    
	    RCC_RTCCLKCmd(ENABLE);/* Enable RTC Clock */    
	    RTC_WaitForSynchro();/* Wait for RTC registers synchronization */
	    RTC_WaitForLastTask();/* Wait until last write operation on RTC registers has finished */
	    
	    /* Set RTC prescaler: set RTC period to 1sec */
	    RTC_SetPrescaler(8000000/128-1); /* RTC period = RTCCLK/RTC_PR = (8MHz/128)/(8000000/128) */
	    
	    RTC_WaitForLastTask();/* Wait until last write operation on RTC registers has finished */
    }
}

/*******************************************************************************
* Function Name  : RTC_Config
* Description    : 上电时调用本函数，自动检查是否需要RTC初始化， 
*                       若需要重新初始化RTC，则调用RTC_Configuration()完成相应操作
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RTC_SetUp(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	RTC_TIME NowTime;

	/* 使能rtc中断 */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = RTC_Priority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable the RTC Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = RTCAlarm_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = RTCAlarm_Priority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	//检查备用寄存器的值是否改变了，如果改变了，说明battery掉电了，需要重新配置rtc
	if(BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
	{
		/* Backup data register value is not correct or not yet programmed (when
		 	the first time the program is executed) */
		Debug("RTC need configure!\n\r");
		
		/* RTC Configuration */
		RTC_Configuration();

		NowTime.year=2011;
		NowTime.mon=4;
		NowTime.day=21;
		NowTime.hour=0;
		NowTime.min=0;
		NowTime.sec=0;
		if(RTC_Adjust(&NowTime,RtcOp_SetTime)==TRUE)
		{
			Debug("RTC set sucess!\n\r");
		}
		else
		{
			Debug("RTC set error!\n\r");
		}

		//备用寄存器写入一个值，供下次做掉电检查
		BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);    
	}
	else	//如果备用寄存器值不一致，说明掉电了。
	{
		if(RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)//检查是否上电复位
	    {
	      Debug("System power on reset!\n\r");
	    }
	    else if(RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)//检查是否手动复位
	    {
	      Debug("System reset!\n\r");
	    }	   
	    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	    PWR_BackupAccessCmd(ENABLE);	
	    
		RTC_WaitForSynchro();//等待RTC寄存器和APB时钟同步
	    RTC_ITConfig(RTC_IT_SEC, ENABLE);//使能RTC秒中断
	    RTC_WaitForLastTask();//等待写操作完成
	    RTC_ITConfig(RTC_IT_ALR, ENABLE);
	    RTC_WaitForLastTask();//等待写操作完成
	}
	RCC_ClearFlag();//清除复位标志
	
	RTC_WaitForLastTask();
	RTC_Counter=(RTC_GetCounter()+10)/60+1;//让RTC counter变成下一分钟的值
	RTC_Counter*=60;
	Debug("RTC configured finished\n\r");
	RTC_WaitForLastTask();
}

//获取当前时间的函数
//只需要定义一个结构体实体，将地址赋予给pTime，就可以得到当前的日期时间
//算法比较繁琐，学习的人要耐心看
//在arm构架里，除法需要比较长的时间，所以尽量避免少用除法和余法
void RTC_GetTime(RTC_TIME *pTime)
{ 
  u32 NowTime;
  u32 tmp,i;

	RTC_WaitForLastTask();
	NowTime = RTC_GetCounter();//获取当前计时器值
	RTC_WaitForLastTask();

  //计算周期年，考虑到闰年的存在，以4年一个周期
  tmp=NowTime%(DAY_SECONDS*366+DAY_SECONDS*365*3);
  if(tmp<DAY_SECONDS*366) pTime->year=RTC_START_YEAR+NowTime/(DAY_SECONDS*366+DAY_SECONDS*365*3)*4+0;
  else if(tmp<DAY_SECONDS*366+DAY_SECONDS*365*1) pTime->year=RTC_START_YEAR+NowTime/(DAY_SECONDS*366+DAY_SECONDS*365*3)*4+1;
  else if(tmp<DAY_SECONDS*366+DAY_SECONDS*365*2) pTime->year=RTC_START_YEAR+NowTime/(DAY_SECONDS*366+DAY_SECONDS*365*3)*4+2;
  else pTime->year=RTC_START_YEAR+NowTime/(DAY_SECONDS*366+DAY_SECONDS*365*3)*4+3;

  if(tmp<DAY_SECONDS*366) //闰年
  {
		for(i=1;i<13;i++)
	    {
			if(tmp<Leap_Month_Seconds[i])
			{
				pTime->mon=i;
				tmp-=Leap_Month_Seconds[i-1];//用数组查询代替复杂的计算
				break;
		  	}
	   }
  }
  else if(tmp<DAY_SECONDS*366+DAY_SECONDS*365*1)
  {
		tmp-=DAY_SECONDS*366;

	  	for(i=1;i<13;i++)
		{
			if(tmp<Month_Seconds[i])
			{
				pTime->mon=i;
				tmp-=Month_Seconds[i-1];
				break;
			}
		}		
  }
  else if(tmp<DAY_SECONDS*366+DAY_SECONDS*365*2)
  {
  		tmp-=DAY_SECONDS*366+DAY_SECONDS*365*1;

	  	for(i=1;i<13;i++)
		{
			if(tmp<Month_Seconds[i])
			{
				pTime->mon=i;
				tmp-=Month_Seconds[i-1];
				break;
			}
		}	  		
  }
  else
  {
		tmp-=DAY_SECONDS*366+DAY_SECONDS*365*2;
		
	  	for(i=1;i<13;i++)
		{
			if(tmp<Month_Seconds[i])
			{
				pTime->mon=i;
				tmp-=Month_Seconds[i-1];
				break;
			}
		}			
  }

  pTime->week=NowTime/DAY_SECONDS%7;//因为1912.1.1是星期一，所以不用偏移值
  //注意获取的值范围为0-6，这样是为了方便数组查阅，从而翻译成中文显示

  pTime->day=tmp/DAY_SECONDS+1;
  tmp=tmp%DAY_SECONDS;

  pTime->hour = tmp/3600;
  tmp=tmp%3600;

  pTime->min = tmp/60;

  pTime->sec = tmp%60;
}

//时间调整函数
//参数传递用结构体地址，不需要设定星期值，注意要定义结构体实体！
bool RTC_Adjust(RTC_TIME *pTime,RTC_OPERATE Op)
{
	u32 tmp1,tmp2;
	u8 Leap_Flag=0;

	tmp1=pTime->year-RTC_START_YEAR;
	if((tmp1>135)||(pTime->year<RTC_START_YEAR)) return FALSE; //年份检测
	
	if(tmp1)	tmp2=(tmp1-1)/4+1;
	else tmp2=0;
	Leap_Flag=(tmp1%4)?0:1;
	tmp1=(tmp1*365+tmp2)*DAY_SECONDS;//年换算成的秒数

	if((pTime->mon<1)||(pTime->mon>12)) return FALSE;  //月份检查
	
	if(Leap_Flag)
	{
		if(pTime->day>((Leap_Month_Seconds[pTime->mon]-Leap_Month_Seconds[pTime->mon-1])/DAY_SECONDS)) return FALSE; //日检查
		tmp2=Leap_Month_Seconds[pTime->mon-1];
	}
	else
	{
		if(pTime->day>((Month_Seconds[pTime->mon]-Month_Seconds[pTime->mon-1])/DAY_SECONDS)) return FALSE;//日检查
		tmp2=Month_Seconds[pTime->mon-1];
	}

	if(pTime->hour>23) return FALSE; //小时检查
	if(pTime->min>59) return FALSE;	 //分钟检查
	if(pTime->sec>59) return FALSE;	 //秒检查

	tmp1+=(tmp2+(pTime->day-1)*DAY_SECONDS);

	tmp1+=(pTime->hour*3600 + pTime->min*60 + pTime->sec);

	switch(Op)
	{	
		case RtcOp_SetTime:
			RTC_WaitForLastTask(); 
			RTC_SetCounter(tmp1);	
			RTC_WaitForLastTask();

			RTC_WaitForLastTask();
			RTC_Counter=(RTC_GetCounter()+10)/60+1;//让RTC counter变成下一分钟的值
			RTC_Counter*=60;
			RTC_WaitForLastTask();
			break;
		case RtcOp_SetAlarm:
			RTC_WaitForLastTask(); 
			RTC_SetAlarm(tmp1);
			RTC_WaitForLastTask();
			break;
	}
	
	return TRUE;
}

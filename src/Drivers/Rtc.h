#ifndef __RTC_H__
#define __RTC_H__

typedef struct {
	u16 year;
	u8 mon;
	u8 day;
	u8 week;
	u8 hour;
	u8 min;
	u8 sec;
}RTC_TIME;

typedef enum {
	RtcOp_CheckVal,
	RtcOp_SetTime,
	RtcOp_SetAlarm,
}RTC_OPERATE;


void RTC_SetUp(void);
void RTC_GetTime(RTC_TIME *pTime);
bool RTC_Adjust(RTC_TIME *pTime,RTC_OPERATE Op);


#endif


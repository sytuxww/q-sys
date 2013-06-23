#ifndef QSYS_MUSIC_HANDLER_H
#define QSYS_MUSIC_HANDLER_H

//4	音乐控制项
typedef enum{
	//状态项
	MusicNothing=0,//可接受用户状态输入
	MusicPlay,
	MusicPause,//可接受用户状态输入
	MusicContinue,//可接受用户状态输入
	MusicJump,
	MusicStop,

	//功能项
	MusicChangeVol,
	MusicGetFFT,
	MusicSetIo,
}MUSIC_CMD;

typedef struct{	
	MUSIC_CMD Cmd;
	u16 ParamU16;
	u8 *pFilePath;	
	void *pParam;
}MUSIC_EVENT;

typedef struct{
	u8 Title[48];
	u8 Performer[32];
	u8 Album[32];
	u8 Year[6];
	u8 Note[32];

	u32 AudioByte;//音频部分大小
	u32 Duration;//单位 秒
	u16 BitRate;//比特率
	u16 SampleRate;//采样率
	u16 AudioOffset;//音频部分偏移值
	bool HasToc;//是否有toc table
	u8 Toc[100];//toc table
}MUSIC_INFO;

//4	音乐控制项结束

//通过此函数来向音乐控制线程发命令
//成功返回TRUE
bool Q_MusicSync(MUSIC_EVENT *pMusicEvent);

//等待Music线程空闲
//PerWaitMs为最小等待周期
//void WaitMusicHandlerIdle(u16 PerWaitMs);

//返回音乐播放线程的当前状态
//返回值PlayOne,PlayDir,PlayList,Nothing,Pause
MUSIC_CMD Q_MusicGetState(void);

//返回当前文件位置
u32 Q_MusicGetFilePtr(void);

//返回当前音乐路径头指针
u8 * Q_MusicGetPath(void);

//返回当前音乐文件信息
MUSIC_INFO *Q_MusicGetInfo(void);

//返回音乐播放时间，单位毫秒
u32 Q_MusicGetPlayMs(void);

#endif


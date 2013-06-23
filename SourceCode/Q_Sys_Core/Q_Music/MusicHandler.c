#include "System.h"
#include "Drivers.h"

#define Music_Debug Debug
#define MUSIC_BUF_SIZE 512
#define MUSIC_STATE_WAIT_TICK 1000 // ticks

extern OS_MsgBoxHandle gMusicHandler_Queue;//通过此队列传递命令给Music处理线程
extern OS_SemaphoreHandle gVsDreq_Sem;//vs准备好的信号量

extern void VsWriteReg(u8 addr,u16 data);
extern u16 VsReadReg(u8 addr);

static u8 gMusicBuf[MUSIC_BUF_SIZE];//用于存放音乐文件内容
static u8 gNowMusicPath[MAX_PATH_LEN];//用于存放当前播放文件路径
static MUSIC_INFO gNowMusicInfo;//用于存放当前播放文件信息
static MUSIC_CMD gNowState;//PlayOne,PlayDir,PlayList,Nothing,Pause
//static FIL gMusicObj;//文件操作
static u32 gStartPlayTime;//当前音乐开始播放的时间，单位毫秒

typedef struct{
	u16 ClockF;
	u16 AuData;
	u16 Vol;
	u16 Bass;	
}MUSIC_PARAM;
MUSIC_PARAM gMusicParam={0X9800,0xBB81,0x4040,0x0000};


#if 1//mp3解析
typedef struct
{
	u32 Emphasis   : 2;	// M 
	u32 Original   : 1;	// L
	u32 CopyRight  : 1;	// K
	u32 ModeExt    : 2;	// J
	u32 ChanMode   : 2;	// I
	u32 PrivBit    : 1;	// H
	u32 Padding    : 1;	// G
	u32 SampleRate : 2;	// F
	u32 BitRate    : 4;	// E
	u32 HasCrc     : 1;	// D
	u32 MpegLayer  : 2;	// C
	u32 MpegVer    : 2;	// B
	u32 FrameSync  : 11;	// A
}FRAME_HEADER;

#define		MP3_HEADER_MASK		0xFFFE0C00			// 同步头匹配, 忽略比特率

const u8 nMpegVer  [] = { 3, 0, 2, 1 };
const u8 nMpegLayer[] = { 0, 3, 2, 1 };
const s16 nBitrateTable[3][3][16] =
{
	// V1 L1
	0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, -1,
	// V1 L2
	0, 32, 48, 56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 384, -1,
	// V1 L3
	0, 32, 40, 48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, -1,
	// V2 L1
	0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, -1,
	// V2 L2
	0, 32, 48, 56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 384, -1,
	// V2 L3
//	0,  8, 16, 24,  32,  64,  80,  56,  64, 128, 160, 112, 128, 256, 320, -1,
	0,  8, 16, 24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160, -1,
	// V2.5 L1
	0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, -1,
	// V2.5 L2
	0, 32, 48, 56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 384, -1,
	// V2.5 L3
//	0,  8, 16, 24,  32,  64,  80,  56,  64, 128, 160, 112, 128, 256, 320, -1
	0,  8, 16, 24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160, -1
};

const u16 nSamplingRateTable[3][4] =
{
	// MPEG1				// MPEG 2				// MPEG 2.5
	44100, 48000, 32000, 0, 22050, 24000, 16000, 0, 11025, 12000,  8000, 0
};

// Samples per Frame: 1. index = LSF, 2. index = Layer
const u16 dwSamplesPerFrames[][3] =
{
	{	// MPEG 1
		384,	// Layer1
		1152,	// Layer2	
		1152	// Layer3
	},
	{	// MPEG 2
		384,	// Layer1
		1152,	// Layer2
		576		// Layer3
	},
	{	// MPEG 2.5
		384,	// Layer1
		1152,	// Layer2
		576		// Layer3
	}
};
#endif//mp3解析end

//系统和页面通过调用此函数向music控制线程发命令
bool Q_MusicSync(MUSIC_EVENT *pMusicEvent)
{
	u8 Ret=OS_MsgBoxSend(gMusicHandler_Queue,pMusicEvent,1000,FALSE);//OS_NO_DELAY);
	Debug("Send Music Msg\n\r");
	if(OS_ERR_NONE!=Ret)
	{
		Debug("Music Handler Queue Send Faile!Error ID:%d\n\r",Ret);
		return FALSE;
	}
	
	return TRUE;
}

//返回值PlayOne,PlayDir,PlayList,Nothing,Pause
MUSIC_CMD Q_MusicGetState(void)
{
	return gNowState;
}

//返回已读文件字节数
/*u32 Q_MusicGetFilePtr(void)
{
	return gMusicObj.fptr;
}
*/
//返回当前音乐路径头指针
u8 * Q_MusicGetPath(void)
{
	return gNowMusicPath;
}

MUSIC_INFO *Q_MusicGetInfo(void)
{
	return &gNowMusicInfo;
}

u32 Q_MusicGetPlayMs(void)
{
	//Debug("Q_MusicGetPlayMs %d\n\r",(OS_GetCurrentSysMs()-gStartPlayTime));
	return (OS_GetCurrentSysMs()-gStartPlayTime);
}

void SetMusicParam(MUSIC_PARAM *pMP)
{
	VsWriteReg(REG_CLOCKF, pMP->ClockF);//设置vs1003的时钟,3倍频
	VsWriteReg (REG_AUDATA, pMP->AuData); //采样率48k，立体声
	VsWriteReg(REG_BASS, pMP->Bass);//设置重音
	VsWriteReg(REG_VOL,pMP->Vol);
}

#if 1 //mp3解析
static u32 RevU32p(void* p)
{
	u8	 b0, b1, b2, b3;

	b0 = *((u8*)p + 0);
	b1 = *((u8*)p + 1);
	b2 = *((u8*)p + 2);
	b3 = *((u8*)p + 3);

	*((u8*)p + 0) = b3;
	*((u8*)p + 1) = b2;
	*((u8*)p + 2) = b1;
	*((u8*)p + 3) = b0;

	return *((u32 *)p);
}

// 验证是否为有效帧头
bool IsValidFrmHdr(FRAME_HEADER* pFH)
{
	// 11 bit
	if (0x7FF != pFH->FrameSync) return FALSE;
	// 2bit
	if (0x01 == pFH->MpegVer) return FALSE;
	// 2bit
	if (0x00 == pFH->MpegLayer) return FALSE;
	// 4bit
	if (0x00 == pFH->BitRate || 0x0F == pFH->BitRate) return FALSE;
	// 2bit
	if (0x03 == pFH->SampleRate) return FALSE;

	return TRUE;
}

// calc frame size
long CalcMp3FrmSize(FRAME_HEADER* pFH)
{
	u8 Ver   = nMpegVer[pFH->MpegVer];
	u8	 Layer = nMpegLayer[pFH->MpegLayer];
	u32 FrameSize;
	u32 BitRate;
	u32 SampleRate;

	BitRate    = nBitrateTable[Ver - 1][Layer - 1][pFH->BitRate];
	SampleRate = nSamplingRateTable[Ver - 1][pFH->SampleRate];

	if (1 == Ver)	// MPEG 1
		FrameSize = 144 * 1000 * BitRate / SampleRate + pFH->Padding;
	else
		FrameSize = 72 * 1000 * BitRate / SampleRate + pFH->Padding;

	return FrameSize;
}

//解析ID3标签，返回音频帧在文件的位置，返回-1表示解析失败
//解析原则，以ID3V2为优先
//返回值低16位表示音频帧位置，最高位表示是否有ID3V1信息块
//修改全局变量gNowMusicInfo
static u32 Mp3Id3Parse(u8 *Path)
{
	UINT ReadByte=0;
	u8 *pBuf=gMusicBuf;
	u16 ID3V2_Len=0;
	u16 TagLen;
	u8 Header[10];
	u32 FrameStart=0;
	u8 CopyNum;
	FS_FILE *fp;

  	if ((fp=FS_FOpen((void *)_TEXT(Path), FA_OPEN_EXISTING | FA_READ)) == 0 ) 
	{
		Debug("Open music flie error !:%s\n\r",Path);
		return (u32)-1;
	}

	ReadByte=FS_FRead(Header, 10, 1,fp);//读取标签头
	if( (ReadByte!=10)) 
	{
		FS_FClose(fp);
		return (u32)-1;
	}
	
	//ID3V2标签头解析
	if((Header[0]!='I')||(Header[1]!='D')||(Header[2]!='3'))
		goto ID3V1_Parse;//没有id3v2，便解析id3v1
	
	//计算ID3V2长度
	ID3V2_Len=((Header[6]&0x7F)<<21)+((Header[7]&0x7F)<<14)+((Header[8]&0x7F)<<7)+(Header[9]&0x7F);	
	FrameStart=ID3V2_Len+10;
	Music_Debug("#%s: Get a ID3v2 header! Len:%d\n\r",Path,ID3V2_Len);

	//解析ID3V2
	while(ID3V2_Len)
	{
		ReadByte=FS_FRead(Header, 10, 1,fp);//读取标签头
		if((ReadByte!=10)) goto Ret;
		ID3V2_Len-=10;

		TagLen=(Header[4]<<24)+(Header[5]<<16)+(Header[6]<<8)+Header[7];//长度计算
		if(TagLen==0) break;
		ID3V2_Len-=TagLen;
		Music_Debug("#ID3v2 Tag:%c%c%c%c Len:%d\n\r",Header[0],Header[1],Header[2],Header[3],TagLen);
		if(TagLen>MUSIC_BUF_SIZE) 
		{
			FS_FSeek(fp,TagLen,FS_SEEK_CUR);
			continue;
		}
		
		ReadByte=FS_FRead(pBuf, TagLen,1,fp);//读取标签内容
		if( (ReadByte!=TagLen)) goto Ret;
		pBuf[TagLen]=0;

		if(CompareBuf(Header,"APIC",4)==TRUE) //附加图片
		{
			//unfinish
		}
		else if((CompareBuf(Header,"TIT2",4)==TRUE)//标题
					||(CompareBuf(Header,"TPE1",4)==TRUE)//演唱者
					||(CompareBuf(Header,"TALB",4)==TRUE)//专辑
					||(CompareBuf(Header,"TYER",4)==TRUE)//年份
					||(CompareBuf(Header,"COMM",4)==TRUE)//评注
					)
		{
			if(pBuf[0]==0x00)//ascii类型
			{
				Debug("%s\n\r",&pBuf[1]);
			}
			else if(pBuf[0]==0x03)//utf8类型
			{
				if(UTF8_To_OEM(&pBuf[1],&pBuf[1],MUSIC_BUF_SIZE)!=NULL)//转码
				{
					Debug("%s\n\r",pBuf);
				}
			}
			else //其他类型字符串
			{
				Music_Debug("UNFINISH\n\r");
				DisplayBuf(pBuf,TagLen,16);
				pBuf[1]=0;
			}

			if(CompareBuf(Header,"TIT2",4)==TRUE)//标题
			{
				CopyNum=TagLen>sizeof(gNowMusicInfo.Title)?sizeof(gNowMusicInfo.Title)-1:TagLen-1;
				MemCpy(gNowMusicInfo.Title,&pBuf[1],CopyNum);
				gNowMusicInfo.Title[CopyNum]=0;
			}
			else if(CompareBuf(Header,"TPE1",4)==TRUE)//演唱者
			{
				CopyNum=TagLen>sizeof(gNowMusicInfo.Performer)?sizeof(gNowMusicInfo.Performer)-1:TagLen-1;
				MemCpy(gNowMusicInfo.Performer,&pBuf[1],CopyNum);
				gNowMusicInfo.Performer[CopyNum]=0;
			}
			else if(CompareBuf(Header,"TALB",4)==TRUE)//专辑
			{
				CopyNum=TagLen>sizeof(gNowMusicInfo.Album)?sizeof(gNowMusicInfo.Album)-1:TagLen-1;
				MemCpy(gNowMusicInfo.Album,&pBuf[1],CopyNum);
				gNowMusicInfo.Album[CopyNum]=0;
			}
			else if(CompareBuf(Header,"TYER",4)==TRUE)//年份
			{
				CopyNum=4;
				MemCpy(gNowMusicInfo.Year,&pBuf[1],CopyNum);
				gNowMusicInfo.Year[CopyNum]=0;
			}
			else if(CompareBuf(Header,"COMM",4)==TRUE)//评注
			{
				if((CompareBuf(&pBuf[1],"eng",3)==TRUE)||(CompareBuf(&pBuf[1],"chi",3)==TRUE))
				{
					CopyNum=TagLen>sizeof(gNowMusicInfo.Note)?sizeof(gNowMusicInfo.Note)-1:TagLen-1;
					MemCpy(gNowMusicInfo.Note,&pBuf[5],CopyNum);
					gNowMusicInfo.Note[CopyNum]=0;
				}
				else
				{
					CopyNum=TagLen>sizeof(gNowMusicInfo.Note)?sizeof(gNowMusicInfo.Note)-1:TagLen-1;
					MemCpy(gNowMusicInfo.Note,&pBuf[1],CopyNum);
					gNowMusicInfo.Note[CopyNum]=0;
				}
			}
		}
		else //其他标签帧
		{
			//unhandle
		}
	}

ID3V1_Parse:
	//解析ID3V1
	FS_FSeek(fp,-128,FS_SEEK_CUR);
	ReadByte=FS_FRead(pBuf, 128, 1,fp);//读取标签头
	if( (ReadByte!=128)) goto Ret;

	if(CompareBuf(pBuf,"TAG",3))//有ID3V1信息块
	{
		FrameStart|=0x80000000;//置高标志位

		if(!gNowMusicInfo.Title[0]&&pBuf[3])//标题
		{
			CopyNum=30>sizeof(gNowMusicInfo.Title)?sizeof(gNowMusicInfo.Title)-1:29;
			MemCpy(gNowMusicInfo.Title,&pBuf[3],CopyNum);
			gNowMusicInfo.Title[CopyNum]=0;
		}
		if(!gNowMusicInfo.Performer[0]&&pBuf[33])//作者
		{
			CopyNum=30>sizeof(gNowMusicInfo.Performer)?sizeof(gNowMusicInfo.Performer)-1:29;
			MemCpy(gNowMusicInfo.Performer,&pBuf[33],CopyNum);
			gNowMusicInfo.Performer[CopyNum]=0;
		}
		if(!gNowMusicInfo.Album[0]&&pBuf[63])//专辑
		{
			CopyNum=30>sizeof(gNowMusicInfo.Album)?sizeof(gNowMusicInfo.Album)-1:29;
			MemCpy(gNowMusicInfo.Album,&pBuf[63],CopyNum);
			gNowMusicInfo.Album[CopyNum]=0;
		}
		if(!gNowMusicInfo.Year[0]&&pBuf[93])//年份
		{
			CopyNum=4;
			MemCpy(gNowMusicInfo.Year,&pBuf[93],CopyNum);
			gNowMusicInfo.Year[CopyNum]=0;
		}
		if(!gNowMusicInfo.Note[0]&&pBuf[97])//备注
		{
			CopyNum=30>sizeof(gNowMusicInfo.Note)?sizeof(gNowMusicInfo.Note)-1:29;
			MemCpy(gNowMusicInfo.Note,&pBuf[97],CopyNum);
			gNowMusicInfo.Note[CopyNum]=0;
		}
	}
	
Ret:	
	if(FS_FClose(fp)!=0)
	{
		Debug("Mp3Id3Parse:Close music file error!\n\r");
	}

	return FrameStart;
}

#define VBRI_SUPPORT 0
bool Mp3InfoParse(u8 * Mp3Path,MUSIC_INFO *pMusicInfo)
{
//	FRESULT Res;
	UINT ReadByte;
	bool HasID3V1=FALSE;
	u32 AudioOffset;			// 第一个音频帧在文件内偏移
	u32 FrameSize;				// 帧大小
	FRAME_HEADER FrameHeader;//帧头

	u32 Id;
#if VBRI_SUPPORT == 1
	u32 VbriTocCnt;			// (VBRI)
	u32 VbriTocFrm;			// (VBRI)
	u32 i;
#endif
	u8 IsVbrFormat;			// 是否为VBR格式

	u32 AudioByte;			//音频帧总长度
	u32 FrameNum;				// 帧总数
	u32 FrameDuration;  //每一帧的持续时间

	FS_FILE * fp=NULL;
	//解析ID3
	AudioOffset=Mp3Id3Parse((void *)Mp3Path);

	//打开文件
  	if ((fp=FS_FOpen((void *)_TEXT(Mp3Path), FA_OPEN_EXISTING | FA_READ) )== 0 ) 
	{
		Debug("Open music flie error !:%s\n\r",Mp3Path);
		return FALSE;
	}

	//找第一个音频帧
	if (AudioOffset!=(u32)-1)
	{
		if(AudioOffset&0x80000000) HasID3V1=(bool)1;
		AudioOffset&=0x7fffffff;
		Music_Debug("Find the first frame:%d ,%shas ID3V1!\n\r",AudioOffset,HasID3V1?"":"No ");

		FS_FSeek(fp,0,FS_SEEK_END);

		AudioByte=FS_FTell(fp)-AudioOffset-(HasID3V1?128:0);

		FS_FSeek(fp,AudioOffset,FS_SEEK_SET);

		while (1)
		{// 读取帧头
			ReadByte=FS_FRead(&FrameHeader, 4,1, fp);//
			if((ReadByte!=4)){FS_FClose(fp); return FALSE;}

			RevU32p(&FrameHeader);//大小端转换
			if(IsValidFrmHdr(&FrameHeader)) break;//是否有效

			FS_FSeek(fp,-3,FS_SEEK_CUR);//回退三个字节，搜下一个字符
			Music_Debug(".");//此处可优化，消耗资源
		}
	}
	else
	{
		Music_Debug("Not find first frame!\n\r");
		return FALSE;
	}

	// 第一帧已经找到了
	AudioOffset = FS_FTell(fp)- 4;//第一帧位置
	FrameSize   = CalcMp3FrmSize(&FrameHeader);//帧长度
	Music_Debug("AudioOffset:%d,FrameSize:%d\n\r",AudioOffset,FrameSize);

//--------------------------------------开始处理帧信息-----------------------------------------------

	//检查编码，计算边位置
	if (3 == FrameHeader.MpegVer)		// MPEG 1
	{
		if (3 == FrameHeader.ChanMode)	// mono
			FS_FSeek(fp, (21-4),FS_SEEK_CUR);
		else					// stereo
			FS_FSeek(fp, (36-4),FS_SEEK_CUR);
	}
	else if (2 == FrameHeader.MpegVer)	// MPEG 2
	{
		if (3 == FrameHeader.ChanMode)	// mono
			FS_FSeek(fp, (13-4),FS_SEEK_CUR);
		else					// stereo
			FS_FSeek(fp, (21-4),FS_SEEK_CUR);
	}

	ReadByte=FS_FRead(&Id, 4, 1,fp);//读取vbr或cbr标签
	if((ReadByte!=4)){FS_FClose(fp); return FALSE;}

#if VBRI_SUPPORT == 1	
	VbriTocCnt = 0;
#endif
	FrameNum = 0;
	IsVbrFormat = (0x676e6958 == Id) ||	// gaiX
					      (0x6f666e49 == Id) ||	// ofnI
					      (0x49524256 == Id);	// IRBV

	if(IsVbrFormat)
	{
		if (0x49524256 == Id)	// 'IRBV'
		{Music_Debug("Is VBRI format\n\r");
#if VBRI_SUPPORT == 1
			u16 TocCnt;
			u16 TocScale;
			u16 TocSize;
			u16 TocFrames;
			
			
			f_lseek(&gMusicObj, gMusicObj.fptr+6);

			//文件长度
			Res=f_read(&gMusicObj, &AudioByte, 4, &ReadByte);
			if(Res || (ReadByte!=4)){f_close(&gMusicObj); return FALSE;}
			RevU32p(&AudioByte);
			Debug("AudioByte:%d\n\r",AudioByte);

			//帧总数
			Res=f_read(&gMusicObj, &FrameNum, 4, &ReadByte);
			if(Res || (ReadByte!=4)){f_close(&gMusicObj); return FALSE;}
			RevU32p(&FrameNum);
			Debug("FrameNum:%d\n\r",FrameNum);

			// Number of entries within TOC table as Big-Endian WORD
			Res=f_read(&gMusicObj, &TocCnt, 2, &ReadByte);
			if(Res || (ReadByte!=2)){f_close(&gMusicObj); return FALSE;}
			TocCnt = Rev16(TocCnt);
			Debug("Toc Count:%d\n\r",TocCnt);
			
			VbriTocCnt = TocCnt + 1;	// !!!

			// Scale factor of TOC table entries as Big-Endian WORD
			Res=f_read(&gMusicObj, &TocScale, 2, &ReadByte);
			if(Res || (ReadByte!=2)){f_close(&gMusicObj); return FALSE;}
			TocScale = Rev16(TocScale);
			Debug("TocScale:%d\n\r",TocScale);

			// Size per table entry in bytes (max 4) as Big-Endian WORD
			Res=f_read(&gMusicObj, &TocSize, 2, &ReadByte);
			if(Res || (ReadByte!=2)){f_close(&gMusicObj); return FALSE;}
			TocSize = Rev16(TocSize);
			Debug("TocSize:%d\n\r",TocSize);
			
			if (TocSize > 4 || TocSize < 1) {Debug("TocSize error!\n\r");f_close(&gMusicObj); return FALSE;}

			// Frames per table entry as Big-Endian WORD
			Res=f_read(&gMusicObj, &TocFrames, 2, &ReadByte);
			if(Res || (ReadByte!=2)){f_close(&gMusicObj); return FALSE;}
			TocFrames = Rev16(TocFrames);
			VbriTocFrm = TocFrames;
			Debug("TocFrames:%d\n\r",TocFrames);

			// read toc
			{
				u32*	p;
				u32 Pos = 0;			// must be unsigned
				float fScale;
				u32 TocEntry = 0;	// must be unsigned
	
				i = VbriTocCnt + 1;
				p = gNowMusicInfo.Toc;

				while (--i)
				{
					Res=f_read(&gMusicObj, &TocEntry, TocSize, &ReadByte);
					if(Res || (ReadByte!=TocSize)){f_close(&gMusicObj); return FALSE;}			
					RevU32p(&TocEntry);
					Debug("TocEntry:%d\n\r",TocEntry);
					
					TocEntry >>= 8 * (4 - TocSize);
					Pos += TocEntry * TocScale;
					*p++ = Pos;
				}

				i = VbriTocCnt + 1;
				p = gNowMusicInfo.Toc;
				fScale = (float)((double)AudioByte / (double)Pos);
				while (--i)
				{
					*p *= (u32)fScale;
					p++;
				}
			}
#endif
		}
		else		// 'Xing'
		{
			u32	Flag;
      		Music_Debug("Is VBR format\n\r");
			
			ReadByte=FS_FRead( &Flag, 4, 1,fp);
			if( (ReadByte!=4)){FS_FClose(fp); return FALSE;}
			RevU32p(&Flag);

			if (Flag & 1)		// Frames Flag
			{
				ReadByte=FS_FRead(&FrameNum, 4, 1,fp);
				if((ReadByte!=4)){FS_FClose(fp); return FALSE;}
				RevU32p(&FrameNum);
			}

			if (Flag & 2)		// Bytes Flag
			{
				ReadByte=FS_FRead( &AudioByte, 4,1,fp);
				if( (ReadByte!=4)){FS_FClose(fp); return FALSE;}
				RevU32p(&AudioByte);
			}

			if (Flag & 4)		// TOC Flag
			{
				pMusicInfo->HasToc=TRUE;
				ReadByte=FS_FRead( pMusicInfo->Toc, 100,1,fp);
				if( (ReadByte!=100)){FS_FClose(fp); return FALSE;}
			}
			
			//for (i = 0; i < 100; i++)
			//	Debug("Toc:%d-%d-%d\n\r",i,gNowMusicInfo.Toc[i],AudioByte*gNowMusicInfo.Toc[i]/256); 
		}
	}
	else   //CBR格式
	{Music_Debug("Is CBR format\n\r");
	#if 0
		u32	LastHeader;
    	
		// check next frame...
		f_lseek(&gMusicObj, AudioOffset + FrameSize);

		while (1)
		{
			LastHeader = *(long*)&FrameHeader;

			// search sync
			while (1)
			{
				// read frame header
				Res=f_read(&gMusicObj, &FrameHeader, 4, &ReadByte);
				if(Res || (ReadByte!=4)){f_close(&gMusicObj); return -1;}
				RevU32p(&FrameHeader);
				if (IsValidFrmHdr(&FrameHeader)) break;

				f_lseek(&gMusicObj, gMusicObj.fptr-3);
				Debug(".");
			}
			Debug("\n\r");

			// found it
			if ((*(long*)&FrameHeader & MP3_HEADER_MASK) == (LastHeader & MP3_HEADER_MASK))
			{
				FrameSize += CalcMp3FrmSize(&FrameHeader);
				break;
			}

			AudioOffset = gMusicObj.fptr - 4;
			FrameSize = CalcMp3FrmSize(&FrameHeader);
			f_lseek(&gMusicObj, gMusicObj.fptr+(FrameSize-4));
		}
#endif
	}

	//开始计算并赋值
	{
		u8 Ver   = nMpegVer[FrameHeader.MpegVer];
		u8 Layer = nMpegLayer[FrameHeader.MpegLayer];

		// only Layer 3 is supportted
		if (3 != Layer)
		{
			FS_FClose(fp);
			return FALSE;
		}

		pMusicInfo->BitRate = nBitrateTable[Ver - 1][Layer - 1][FrameHeader.BitRate];
		pMusicInfo->SampleRate = nSamplingRateTable[Ver - 1][FrameHeader.SampleRate];

		if (IsVbrFormat)
		{
			if (FrameNum==0) FrameNum = AudioByte / FrameSize;
			FrameDuration = ((u32)dwSamplesPerFrames[Ver - 1][Layer - 1]) * 1000 / pMusicInfo->SampleRate;
			pMusicInfo->Duration = FrameNum * FrameDuration;
			pMusicInfo->Duration=pMusicInfo->Duration/1000+1;
		}
		else
		{
			pMusicInfo->Duration = AudioByte * 8 / pMusicInfo->BitRate;
			pMusicInfo->Duration=pMusicInfo->Duration/1000;
		}

		pMusicInfo->AudioByte=AudioByte;
		pMusicInfo->AudioOffset=AudioOffset;
		Music_Debug("#AudioOffset:%d\n\r#AudioByte:%d\n\r#BitRate:%d\n\r#SampleRate:%d\n\r#Duration:%d\n\r",
			pMusicInfo->AudioOffset,pMusicInfo->AudioByte,pMusicInfo->BitRate, pMusicInfo->SampleRate,pMusicInfo->Duration);
	}	

	FS_FClose(fp);

	return TRUE;
}
#endif//MP3解析结束

void MusicHandler_Task(void *Task_Parameters)
{
	MUSIC_EVENT ME;
	INPUT_EVENT EventParam;
//	FRESULT Res;
	UINT ReadByte=0;
	u32 SendByte=0,i;
	u8 *pBuf=NULL;
	u32 TimeSnap=0;
	FS_FILE *fp=NULL;
#if(QXW_PRODUCT_ID==114)
	OS_DeclareCritical();
#endif
	
////ad for fs test
/*{
	int	Time;
	GUI_REGION DrawRegion;
	int count =30;
	
	DrawRegion.x=DrawRegion.y=0;
	DrawRegion.w=240;
	DrawRegion.h=320;	
	DrawRegion.Color=FatColor(0x555555);

	Time = OS_GetCurrentTick();

	while(count--)
	{
		Gui_FillBlock(&DrawRegion); 
	}

	Debug("write lcd Time:%d \n\r",(OS_GetCurrentTick()-Time)<<1);
}
*/
///
	while(1)
	{
		gNowState=MusicNothing;
		
WaitEvent:
		OS_MsgBoxReceive(gMusicHandler_Queue,&ME,OS_MAX_DELAY);//等待有效的输入

HandleEvent:		
		//if(ME.Cmd==ReadSpiFlash) 	Music_Debug("@MusicHandler: %d, CMD %d,Param %d\n\r",ME.pFilePath,ME.Cmd,ME.ParamS32);
		//Music_Debug("#MusicHandler: %s, CMD %d,Param %x\n\r",ME.pFilePath,ME.Cmd,ME.ParamU16);

		//if(gNowState!=ME.Cmd) Debug("Music State %d -> %d\n\r",gNowState,ME.Cmd);
		switch(ME.Cmd)
		{
			//播放控制部分
			case MusicPlay:
				if((gNowState==MusicNothing)||(gNowState==MusicStop))//从空闲状态或停止状态到播放单曲
				{
					gNowState=MusicPlay;

					MemSet(&gNowMusicInfo,0,sizeof(gNowMusicInfo));//初始化信息
					if(Mp3InfoParse((void *)ME.pFilePath,&gNowMusicInfo)==FALSE) 
					{
						MemSet(&gNowMusicInfo,0,sizeof(gNowMusicInfo));
					}
					
			    	if ((fp=FS_FOpen((void *)_TEXT(ME.pFilePath), FA_OPEN_EXISTING | FA_READ) )== 0 ) 
					{
						Debug("Open music flie error :%s\n\r",ME.pFilePath);
						break;
					}

					FS_FSeek(fp,gNowMusicInfo.AudioOffset,FS_SEEK_SET);//移到音频处

					//发送事件
					if(Q_InspectPeripEvt(PRID_Current,Perip_MscPlay))//检查是否需要触发
					{
						EventParam.uType=SingleNum_Type;
						EventParam.EventType=Input_MscPlay;
						TimeSnap=OS_GetCurrentSysMs();
						EventParam.Num=0;
						OS_MsgBoxSend(gInputHandler_Queue,&EventParam,MUSIC_STATE_WAIT_TICK,FALSE);
					}

					gStartPlayTime=OS_GetCurrentSysMs();//归零时间
					
					ME.Cmd=MusicContinue;
					goto HandleEvent;
				}
				else if((gNowState==MusicContinue)||(gNowState==MusicPause)) //从持续播放状态直接到播放
				{//先停止，再打开文件
					gNowState=MusicPlay;
		
					ME.Cmd=MusicStop;
					goto HandleEvent;
				}
				break;
			case MusicContinue:
				if(gNowState==MusicPlay)//从播放单曲到持续播放
				{
					gNowState=MusicContinue;
					VsStart();
					SetMusicParam(&gMusicParam);
					VsResetDecodeTime();
					SendByte=ReadByte=0;
					ME.Cmd=MusicContinue;
					goto HandleEvent;
				}
				else if(gNowState==MusicPause)//从暂停到持续播放
				{
					gNowState=MusicContinue;
					OS_SemaphoreTake(gVsDreq_Sem,OS_MAX_DELAY);
					
					if(Q_InspectPeripEvt(PRID_Current,Perip_MscContinue))
					{
						EventParam.uType=SingleNum_Type;
						EventParam.EventType=Input_MscContinue;
						EventParam.Num=OS_GetCurrentSysMs()-TimeSnap;
						OS_MsgBoxSend(gInputHandler_Queue,&EventParam,MUSIC_STATE_WAIT_TICK,FALSE);
					}
					
					ME.Cmd=MusicContinue;
					goto HandleEvent;
				}
				else if(gNowState==MusicContinue)//从持续播放到持续播放
				{
					//gNowState=MusicContinue;
					if(SendByte>=ReadByte)
					{
						ReadByte=FS_FRead(gMusicBuf, MUSIC_BUF_SIZE, 1,fp);
						if( ReadByte)//文件有效
			            {
							pBuf=gMusicBuf;		
							SendByte=0;
						}
			   			else	//音乐文件读完了.
			   			{		
	 		   				ME.Cmd=MusicStop;
							goto HandleEvent;
			   			}	  
					}
				
#if(QXW_PRODUCT_ID==114)
					OS_EnterCritical();				
#endif
					VS_XDCS_SET(0);
					for(i=32;i>0;i--)//每次送32个数据
					{
						VS_Write_Byte(*pBuf++);	
					}
					VS_XDCS_SET(1);
#if(QXW_PRODUCT_ID==114)							
					OS_ExitCritical();
#endif
					SendByte+=32;
					
					while(!VS_DREQ)	//等待vs就绪
					{	
						//检查是否有新事件到来
						if(OS_ERR_NONE==OS_MsgBoxReceive(gMusicHandler_Queue,&ME,OS_NO_DELAY))
						{//有新的命令来了			
							goto HandleEvent;
						}
						OS_SemaphoreTake(gVsDreq_Sem,OS_MAX_DELAY);		
					}

					goto HandleEvent;
				}
				else if((gNowState==MusicJump)||(gNowState==MusicChangeVol)||(gNowState==MusicSetIo))
				{
					gNowState=MusicContinue;
					OS_SemaphoreTake(gVsDreq_Sem,OS_MAX_DELAY);
	
					ME.Cmd=MusicContinue;
					goto HandleEvent;
				}
				else	//如果是从未经预料的状态进入持续播放，则直接停止。
				{
					Debug("No Handler,stop it!\n\r");
					gNowState=MusicContinue;
					ME.Cmd=MusicStop;
					goto HandleEvent;
				}
				//break;
			case MusicJump:
				if((gNowState==MusicContinue)||(gNowState==MusicPause))//跳转
				{
					u32 FileLocal;

					ME.Cmd=gNowState;//储存当前状态
					gNowState=MusicJump;//改变当前状态

					Debug("Jump %dpct\n\r",ME.ParamU16);
					if(ME.ParamU16>99) ME.ParamU16=99;					
					if(gNowMusicInfo.HasToc) 
						FileLocal=((((u32)gNowMusicInfo.Toc[ME.ParamU16])*gNowMusicInfo.AudioByte)>>8)+(u32)gNowMusicInfo.AudioOffset;
					else 
						FileLocal=((u32)ME.ParamU16)*gNowMusicInfo.AudioByte/100+(u32)gNowMusicInfo.AudioOffset;

					Debug("File Local:%d\n\r",FileLocal);
					FS_FSeek(fp,FileLocal,FS_SEEK_SET);

					gStartPlayTime=OS_GetCurrentSysMs()-ME.ParamU16*gNowMusicInfo.Duration*10;//计算时间，注意单位换算
					
					goto HandleEvent;
				}
				else
				{
				
				}
				break;
			case MusicPause:
				if((gNowState==MusicContinue)||(gNowState==MusicJump)||(gNowState==MusicChangeVol)||(gNowState==MusicSetIo))//从持续播放到暂停
				{
					gNowState=MusicPause;
					
					if(Q_InspectPeripEvt(PRID_Current,Perip_MscPause))
					{
						EventParam.uType=SingleNum_Type;
						EventParam.EventType=Input_MscPause;
						EventParam.Num=OS_GetCurrentSysMs()-TimeSnap;
						OS_MsgBoxSend(gInputHandler_Queue,&EventParam,MUSIC_STATE_WAIT_TICK,FALSE);
					}
					
					goto WaitEvent;//等待下一个事件到来
				}
				break;	
			
			case MusicStop:
				if((gNowState==MusicContinue)||(gNowState==MusicPlay))//从持续播放到停止播放
				{
					if(gNowState==MusicPlay) ME.Cmd=MusicPlay;//如果是从MusicPlayOne来的，说明还要回去。
					gNowState=MusicStop;

	   				if(FS_FClose(fp)==-1)
					{
						Debug("Close music file error!\n\r");
					}

					for(SendByte=64;SendByte;SendByte--)
					{ 		
#if(QXW_PRODUCT_ID==114)
						OS_EnterCritical();	
#endif							
						VS_XDCS_SET(0);
						for(i=32;i>0;i--)//每次送32个数据
						{
							VS_Write_Byte(0);	
						}
						VS_XDCS_SET(1);
#if(QXW_PRODUCT_ID==114)
						OS_ExitCritical();
#endif
						while(!VS_DREQ)	//等待vs就绪
						{
							OS_SemaphoreTake(gVsDreq_Sem,OS_MAX_DELAY);
						}
	      			} 
	      			VsStop();
	      			
					if(Q_InspectPeripEvt(PRID_Current,Perip_MscStop))//发送停止事件
					{
						EventParam.uType=SingleNum_Type;
						EventParam.EventType=Input_MscStop;
						EventParam.Num=OS_GetCurrentSysMs()-TimeSnap;
						OS_MsgBoxSend(gInputHandler_Queue,&EventParam,MUSIC_STATE_WAIT_TICK,FALSE);
					}

					if(ME.Cmd==MusicPlay) goto HandleEvent;//如果是从MusicPlayOne来的，说明还要回去。
				}
				break;
				
			//修改音量部分
			case MusicChangeVol:
				ME.Cmd=gNowState;//储存当前状态
				gNowState=MusicChangeVol;//改变当前状态
				gMusicParam.Vol=(u16)ME.ParamU16;
				VsWriteReg(REG_VOL,gMusicParam.Vol);

				//从持续播放到修改音量
				if(ME.Cmd==MusicContinue)	goto HandleEvent;
				else if(ME.Cmd==MusicPause)	goto HandleEvent;
				break;
			case MusicSetIo:
				ME.Cmd=gNowState;//储存当前状态
				gNowState=MusicSetIo;//改变当前状态

				VsLedSet((ME.ParamU16&0x0f)?TRUE:FALSE,(ME.ParamU16>>4)?TRUE:FALSE);

				if(ME.Cmd==MusicContinue)	goto HandleEvent;
				else if(ME.Cmd==MusicPause)	goto HandleEvent;
				break;
			case MusicGetFFT:
				VsGetSpec(ME.pParam);
				break;
			default:
				Debug("No Finish State! State:%d -> %d\n\r",gNowState,ME.Cmd);
				while(1);
		}
	}
}








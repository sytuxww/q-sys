#include "System.h"
#include "Drivers.h"

#define Music_Debug Debug
#define MUSIC_BUF_SIZE 512
#define MUSIC_STATE_WAIT_TICK 1000 // ticks

extern OS_MsgBoxHandle gMusicHandler_Queue;//ͨ���˶��д��������Music�����߳�
extern OS_SemaphoreHandle gVsDreq_Sem;//vs׼���õ��ź���

extern void VsWriteReg(u8 addr,u16 data);
extern u16 VsReadReg(u8 addr);

static u8 gMusicBuf[MUSIC_BUF_SIZE];//���ڴ�������ļ�����
static u8 gNowMusicPath[MAX_PATH_LEN];//���ڴ�ŵ�ǰ�����ļ�·��
static MUSIC_INFO gNowMusicInfo;//���ڴ�ŵ�ǰ�����ļ���Ϣ
static MUSIC_CMD gNowState;//PlayOne,PlayDir,PlayList,Nothing,Pause
//static FIL gMusicObj;//�ļ�����
static u32 gStartPlayTime;//��ǰ���ֿ�ʼ���ŵ�ʱ�䣬��λ����

typedef struct{
	u16 ClockF;
	u16 AuData;
	u16 Vol;
	u16 Bass;	
}MUSIC_PARAM;
MUSIC_PARAM gMusicParam={0X9800,0xBB81,0x4040,0x0000};


#if 1//mp3����
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

#define		MP3_HEADER_MASK		0xFFFE0C00			// ͬ��ͷƥ��, ���Ա�����

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
#endif//mp3����end

//ϵͳ��ҳ��ͨ�����ô˺�����music�����̷߳�����
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

//����ֵPlayOne,PlayDir,PlayList,Nothing,Pause
MUSIC_CMD Q_MusicGetState(void)
{
	return gNowState;
}

//�����Ѷ��ļ��ֽ���
/*u32 Q_MusicGetFilePtr(void)
{
	return gMusicObj.fptr;
}
*/
//���ص�ǰ����·��ͷָ��
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
	VsWriteReg(REG_CLOCKF, pMP->ClockF);//����vs1003��ʱ��,3��Ƶ
	VsWriteReg (REG_AUDATA, pMP->AuData); //������48k��������
	VsWriteReg(REG_BASS, pMP->Bass);//��������
	VsWriteReg(REG_VOL,pMP->Vol);
}

#if 1 //mp3����
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

// ��֤�Ƿ�Ϊ��Ч֡ͷ
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

//����ID3��ǩ��������Ƶ֡���ļ���λ�ã�����-1��ʾ����ʧ��
//����ԭ����ID3V2Ϊ����
//����ֵ��16λ��ʾ��Ƶ֡λ�ã����λ��ʾ�Ƿ���ID3V1��Ϣ��
//�޸�ȫ�ֱ���gNowMusicInfo
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

	ReadByte=FS_FRead(Header, 10, 1,fp);//��ȡ��ǩͷ
	if( (ReadByte!=10)) 
	{
		FS_FClose(fp);
		return (u32)-1;
	}
	
	//ID3V2��ǩͷ����
	if((Header[0]!='I')||(Header[1]!='D')||(Header[2]!='3'))
		goto ID3V1_Parse;//û��id3v2�������id3v1
	
	//����ID3V2����
	ID3V2_Len=((Header[6]&0x7F)<<21)+((Header[7]&0x7F)<<14)+((Header[8]&0x7F)<<7)+(Header[9]&0x7F);	
	FrameStart=ID3V2_Len+10;
	Music_Debug("#%s: Get a ID3v2 header! Len:%d\n\r",Path,ID3V2_Len);

	//����ID3V2
	while(ID3V2_Len)
	{
		ReadByte=FS_FRead(Header, 10, 1,fp);//��ȡ��ǩͷ
		if((ReadByte!=10)) goto Ret;
		ID3V2_Len-=10;

		TagLen=(Header[4]<<24)+(Header[5]<<16)+(Header[6]<<8)+Header[7];//���ȼ���
		if(TagLen==0) break;
		ID3V2_Len-=TagLen;
		Music_Debug("#ID3v2 Tag:%c%c%c%c Len:%d\n\r",Header[0],Header[1],Header[2],Header[3],TagLen);
		if(TagLen>MUSIC_BUF_SIZE) 
		{
			FS_FSeek(fp,TagLen,FS_SEEK_CUR);
			continue;
		}
		
		ReadByte=FS_FRead(pBuf, TagLen,1,fp);//��ȡ��ǩ����
		if( (ReadByte!=TagLen)) goto Ret;
		pBuf[TagLen]=0;

		if(CompareBuf(Header,"APIC",4)==TRUE) //����ͼƬ
		{
			//unfinish
		}
		else if((CompareBuf(Header,"TIT2",4)==TRUE)//����
					||(CompareBuf(Header,"TPE1",4)==TRUE)//�ݳ���
					||(CompareBuf(Header,"TALB",4)==TRUE)//ר��
					||(CompareBuf(Header,"TYER",4)==TRUE)//���
					||(CompareBuf(Header,"COMM",4)==TRUE)//��ע
					)
		{
			if(pBuf[0]==0x00)//ascii����
			{
				Debug("%s\n\r",&pBuf[1]);
			}
			else if(pBuf[0]==0x03)//utf8����
			{
				if(UTF8_To_OEM(&pBuf[1],&pBuf[1],MUSIC_BUF_SIZE)!=NULL)//ת��
				{
					Debug("%s\n\r",pBuf);
				}
			}
			else //���������ַ���
			{
				Music_Debug("UNFINISH\n\r");
				DisplayBuf(pBuf,TagLen,16);
				pBuf[1]=0;
			}

			if(CompareBuf(Header,"TIT2",4)==TRUE)//����
			{
				CopyNum=TagLen>sizeof(gNowMusicInfo.Title)?sizeof(gNowMusicInfo.Title)-1:TagLen-1;
				MemCpy(gNowMusicInfo.Title,&pBuf[1],CopyNum);
				gNowMusicInfo.Title[CopyNum]=0;
			}
			else if(CompareBuf(Header,"TPE1",4)==TRUE)//�ݳ���
			{
				CopyNum=TagLen>sizeof(gNowMusicInfo.Performer)?sizeof(gNowMusicInfo.Performer)-1:TagLen-1;
				MemCpy(gNowMusicInfo.Performer,&pBuf[1],CopyNum);
				gNowMusicInfo.Performer[CopyNum]=0;
			}
			else if(CompareBuf(Header,"TALB",4)==TRUE)//ר��
			{
				CopyNum=TagLen>sizeof(gNowMusicInfo.Album)?sizeof(gNowMusicInfo.Album)-1:TagLen-1;
				MemCpy(gNowMusicInfo.Album,&pBuf[1],CopyNum);
				gNowMusicInfo.Album[CopyNum]=0;
			}
			else if(CompareBuf(Header,"TYER",4)==TRUE)//���
			{
				CopyNum=4;
				MemCpy(gNowMusicInfo.Year,&pBuf[1],CopyNum);
				gNowMusicInfo.Year[CopyNum]=0;
			}
			else if(CompareBuf(Header,"COMM",4)==TRUE)//��ע
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
		else //������ǩ֡
		{
			//unhandle
		}
	}

ID3V1_Parse:
	//����ID3V1
	FS_FSeek(fp,-128,FS_SEEK_CUR);
	ReadByte=FS_FRead(pBuf, 128, 1,fp);//��ȡ��ǩͷ
	if( (ReadByte!=128)) goto Ret;

	if(CompareBuf(pBuf,"TAG",3))//��ID3V1��Ϣ��
	{
		FrameStart|=0x80000000;//�ø߱�־λ

		if(!gNowMusicInfo.Title[0]&&pBuf[3])//����
		{
			CopyNum=30>sizeof(gNowMusicInfo.Title)?sizeof(gNowMusicInfo.Title)-1:29;
			MemCpy(gNowMusicInfo.Title,&pBuf[3],CopyNum);
			gNowMusicInfo.Title[CopyNum]=0;
		}
		if(!gNowMusicInfo.Performer[0]&&pBuf[33])//����
		{
			CopyNum=30>sizeof(gNowMusicInfo.Performer)?sizeof(gNowMusicInfo.Performer)-1:29;
			MemCpy(gNowMusicInfo.Performer,&pBuf[33],CopyNum);
			gNowMusicInfo.Performer[CopyNum]=0;
		}
		if(!gNowMusicInfo.Album[0]&&pBuf[63])//ר��
		{
			CopyNum=30>sizeof(gNowMusicInfo.Album)?sizeof(gNowMusicInfo.Album)-1:29;
			MemCpy(gNowMusicInfo.Album,&pBuf[63],CopyNum);
			gNowMusicInfo.Album[CopyNum]=0;
		}
		if(!gNowMusicInfo.Year[0]&&pBuf[93])//���
		{
			CopyNum=4;
			MemCpy(gNowMusicInfo.Year,&pBuf[93],CopyNum);
			gNowMusicInfo.Year[CopyNum]=0;
		}
		if(!gNowMusicInfo.Note[0]&&pBuf[97])//��ע
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
	u32 AudioOffset;			// ��һ����Ƶ֡���ļ���ƫ��
	u32 FrameSize;				// ֡��С
	FRAME_HEADER FrameHeader;//֡ͷ

	u32 Id;
#if VBRI_SUPPORT == 1
	u32 VbriTocCnt;			// (VBRI)
	u32 VbriTocFrm;			// (VBRI)
	u32 i;
#endif
	u8 IsVbrFormat;			// �Ƿ�ΪVBR��ʽ

	u32 AudioByte;			//��Ƶ֡�ܳ���
	u32 FrameNum;				// ֡����
	u32 FrameDuration;  //ÿһ֡�ĳ���ʱ��

	FS_FILE * fp=NULL;
	//����ID3
	AudioOffset=Mp3Id3Parse((void *)Mp3Path);

	//���ļ�
  	if ((fp=FS_FOpen((void *)_TEXT(Mp3Path), FA_OPEN_EXISTING | FA_READ) )== 0 ) 
	{
		Debug("Open music flie error !:%s\n\r",Mp3Path);
		return FALSE;
	}

	//�ҵ�һ����Ƶ֡
	if (AudioOffset!=(u32)-1)
	{
		if(AudioOffset&0x80000000) HasID3V1=(bool)1;
		AudioOffset&=0x7fffffff;
		Music_Debug("Find the first frame:%d ,%shas ID3V1!\n\r",AudioOffset,HasID3V1?"":"No ");

		FS_FSeek(fp,0,FS_SEEK_END);

		AudioByte=FS_FTell(fp)-AudioOffset-(HasID3V1?128:0);

		FS_FSeek(fp,AudioOffset,FS_SEEK_SET);

		while (1)
		{// ��ȡ֡ͷ
			ReadByte=FS_FRead(&FrameHeader, 4,1, fp);//
			if((ReadByte!=4)){FS_FClose(fp); return FALSE;}

			RevU32p(&FrameHeader);//��С��ת��
			if(IsValidFrmHdr(&FrameHeader)) break;//�Ƿ���Ч

			FS_FSeek(fp,-3,FS_SEEK_CUR);//���������ֽڣ�����һ���ַ�
			Music_Debug(".");//�˴����Ż���������Դ
		}
	}
	else
	{
		Music_Debug("Not find first frame!\n\r");
		return FALSE;
	}

	// ��һ֡�Ѿ��ҵ���
	AudioOffset = FS_FTell(fp)- 4;//��һ֡λ��
	FrameSize   = CalcMp3FrmSize(&FrameHeader);//֡����
	Music_Debug("AudioOffset:%d,FrameSize:%d\n\r",AudioOffset,FrameSize);

//--------------------------------------��ʼ����֡��Ϣ-----------------------------------------------

	//�����룬�����λ��
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

	ReadByte=FS_FRead(&Id, 4, 1,fp);//��ȡvbr��cbr��ǩ
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

			//�ļ�����
			Res=f_read(&gMusicObj, &AudioByte, 4, &ReadByte);
			if(Res || (ReadByte!=4)){f_close(&gMusicObj); return FALSE;}
			RevU32p(&AudioByte);
			Debug("AudioByte:%d\n\r",AudioByte);

			//֡����
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
	else   //CBR��ʽ
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

	//��ʼ���㲢��ֵ
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
#endif//MP3��������

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
		OS_MsgBoxReceive(gMusicHandler_Queue,&ME,OS_MAX_DELAY);//�ȴ���Ч������

HandleEvent:		
		//if(ME.Cmd==ReadSpiFlash) 	Music_Debug("@MusicHandler: %d, CMD %d,Param %d\n\r",ME.pFilePath,ME.Cmd,ME.ParamS32);
		//Music_Debug("#MusicHandler: %s, CMD %d,Param %x\n\r",ME.pFilePath,ME.Cmd,ME.ParamU16);

		//if(gNowState!=ME.Cmd) Debug("Music State %d -> %d\n\r",gNowState,ME.Cmd);
		switch(ME.Cmd)
		{
			//���ſ��Ʋ���
			case MusicPlay:
				if((gNowState==MusicNothing)||(gNowState==MusicStop))//�ӿ���״̬��ֹͣ״̬�����ŵ���
				{
					gNowState=MusicPlay;

					MemSet(&gNowMusicInfo,0,sizeof(gNowMusicInfo));//��ʼ����Ϣ
					if(Mp3InfoParse((void *)ME.pFilePath,&gNowMusicInfo)==FALSE) 
					{
						MemSet(&gNowMusicInfo,0,sizeof(gNowMusicInfo));
					}
					
			    	if ((fp=FS_FOpen((void *)_TEXT(ME.pFilePath), FA_OPEN_EXISTING | FA_READ) )== 0 ) 
					{
						Debug("Open music flie error :%s\n\r",ME.pFilePath);
						break;
					}

					FS_FSeek(fp,gNowMusicInfo.AudioOffset,FS_SEEK_SET);//�Ƶ���Ƶ��

					//�����¼�
					if(Q_InspectPeripEvt(PRID_Current,Perip_MscPlay))//����Ƿ���Ҫ����
					{
						EventParam.uType=SingleNum_Type;
						EventParam.EventType=Input_MscPlay;
						TimeSnap=OS_GetCurrentSysMs();
						EventParam.Num=0;
						OS_MsgBoxSend(gInputHandler_Queue,&EventParam,MUSIC_STATE_WAIT_TICK,FALSE);
					}

					gStartPlayTime=OS_GetCurrentSysMs();//����ʱ��
					
					ME.Cmd=MusicContinue;
					goto HandleEvent;
				}
				else if((gNowState==MusicContinue)||(gNowState==MusicPause)) //�ӳ�������״ֱ̬�ӵ�����
				{//��ֹͣ���ٴ��ļ�
					gNowState=MusicPlay;
		
					ME.Cmd=MusicStop;
					goto HandleEvent;
				}
				break;
			case MusicContinue:
				if(gNowState==MusicPlay)//�Ӳ��ŵ�������������
				{
					gNowState=MusicContinue;
					VsStart();
					SetMusicParam(&gMusicParam);
					VsResetDecodeTime();
					SendByte=ReadByte=0;
					ME.Cmd=MusicContinue;
					goto HandleEvent;
				}
				else if(gNowState==MusicPause)//����ͣ����������
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
				else if(gNowState==MusicContinue)//�ӳ������ŵ���������
				{
					//gNowState=MusicContinue;
					if(SendByte>=ReadByte)
					{
						ReadByte=FS_FRead(gMusicBuf, MUSIC_BUF_SIZE, 1,fp);
						if( ReadByte)//�ļ���Ч
			            {
							pBuf=gMusicBuf;		
							SendByte=0;
						}
			   			else	//�����ļ�������.
			   			{		
	 		   				ME.Cmd=MusicStop;
							goto HandleEvent;
			   			}	  
					}
				
#if(QXW_PRODUCT_ID==114)
					OS_EnterCritical();				
#endif
					VS_XDCS_SET(0);
					for(i=32;i>0;i--)//ÿ����32������
					{
						VS_Write_Byte(*pBuf++);	
					}
					VS_XDCS_SET(1);
#if(QXW_PRODUCT_ID==114)							
					OS_ExitCritical();
#endif
					SendByte+=32;
					
					while(!VS_DREQ)	//�ȴ�vs����
					{	
						//����Ƿ������¼�����
						if(OS_ERR_NONE==OS_MsgBoxReceive(gMusicHandler_Queue,&ME,OS_NO_DELAY))
						{//���µ���������			
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
				else	//����Ǵ�δ��Ԥ�ϵ�״̬����������ţ���ֱ��ֹͣ��
				{
					Debug("No Handler,stop it!\n\r");
					gNowState=MusicContinue;
					ME.Cmd=MusicStop;
					goto HandleEvent;
				}
				//break;
			case MusicJump:
				if((gNowState==MusicContinue)||(gNowState==MusicPause))//��ת
				{
					u32 FileLocal;

					ME.Cmd=gNowState;//���浱ǰ״̬
					gNowState=MusicJump;//�ı䵱ǰ״̬

					Debug("Jump %dpct\n\r",ME.ParamU16);
					if(ME.ParamU16>99) ME.ParamU16=99;					
					if(gNowMusicInfo.HasToc) 
						FileLocal=((((u32)gNowMusicInfo.Toc[ME.ParamU16])*gNowMusicInfo.AudioByte)>>8)+(u32)gNowMusicInfo.AudioOffset;
					else 
						FileLocal=((u32)ME.ParamU16)*gNowMusicInfo.AudioByte/100+(u32)gNowMusicInfo.AudioOffset;

					Debug("File Local:%d\n\r",FileLocal);
					FS_FSeek(fp,FileLocal,FS_SEEK_SET);

					gStartPlayTime=OS_GetCurrentSysMs()-ME.ParamU16*gNowMusicInfo.Duration*10;//����ʱ�䣬ע�ⵥλ����
					
					goto HandleEvent;
				}
				else
				{
				
				}
				break;
			case MusicPause:
				if((gNowState==MusicContinue)||(gNowState==MusicJump)||(gNowState==MusicChangeVol)||(gNowState==MusicSetIo))//�ӳ������ŵ���ͣ
				{
					gNowState=MusicPause;
					
					if(Q_InspectPeripEvt(PRID_Current,Perip_MscPause))
					{
						EventParam.uType=SingleNum_Type;
						EventParam.EventType=Input_MscPause;
						EventParam.Num=OS_GetCurrentSysMs()-TimeSnap;
						OS_MsgBoxSend(gInputHandler_Queue,&EventParam,MUSIC_STATE_WAIT_TICK,FALSE);
					}
					
					goto WaitEvent;//�ȴ���һ���¼�����
				}
				break;	
			
			case MusicStop:
				if((gNowState==MusicContinue)||(gNowState==MusicPlay))//�ӳ������ŵ�ֹͣ����
				{
					if(gNowState==MusicPlay) ME.Cmd=MusicPlay;//����Ǵ�MusicPlayOne���ģ�˵����Ҫ��ȥ��
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
						for(i=32;i>0;i--)//ÿ����32������
						{
							VS_Write_Byte(0);	
						}
						VS_XDCS_SET(1);
#if(QXW_PRODUCT_ID==114)
						OS_ExitCritical();
#endif
						while(!VS_DREQ)	//�ȴ�vs����
						{
							OS_SemaphoreTake(gVsDreq_Sem,OS_MAX_DELAY);
						}
	      			} 
	      			VsStop();
	      			
					if(Q_InspectPeripEvt(PRID_Current,Perip_MscStop))//����ֹͣ�¼�
					{
						EventParam.uType=SingleNum_Type;
						EventParam.EventType=Input_MscStop;
						EventParam.Num=OS_GetCurrentSysMs()-TimeSnap;
						OS_MsgBoxSend(gInputHandler_Queue,&EventParam,MUSIC_STATE_WAIT_TICK,FALSE);
					}

					if(ME.Cmd==MusicPlay) goto HandleEvent;//����Ǵ�MusicPlayOne���ģ�˵����Ҫ��ȥ��
				}
				break;
				
			//�޸���������
			case MusicChangeVol:
				ME.Cmd=gNowState;//���浱ǰ״̬
				gNowState=MusicChangeVol;//�ı䵱ǰ״̬
				gMusicParam.Vol=(u16)ME.ParamU16;
				VsWriteReg(REG_VOL,gMusicParam.Vol);

				//�ӳ������ŵ��޸�����
				if(ME.Cmd==MusicContinue)	goto HandleEvent;
				else if(ME.Cmd==MusicPause)	goto HandleEvent;
				break;
			case MusicSetIo:
				ME.Cmd=gNowState;//���浱ǰ״̬
				gNowState=MusicSetIo;//�ı䵱ǰ״̬

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








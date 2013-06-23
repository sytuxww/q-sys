/************************************************************************
    Project        :  
    Module         :  file system api
    File Name      :  fs_api.c
    Author         :  dragon_zhang
    Start Date     :  2011.4.25
    Language       :  C
    Target         :  
    Summary        :
    Change Notes   :  2011.4.25 V1.0 creat file
**************************************************************************/

#include "Debug.h"//for debug
#include "ff.h"
#include "FsWrap.h"
#include "OS_Wrap.h"
#include <string.h>

#define FS_Debug Debug

#define MemSet memset
#define MemCpy memcpy

static FATFS FileSystemArea[_VOLUMES];		 /* Work area (file system object) for logical drives */
static WCHAR LenFileName[_MAX_LFN+1];

typedef struct {
	FRESULT error;
	union 
	{
		FIL file;
		DIR dir;
	}fs;
} _FS_PTR;

FS_FILE* FS_FOpen(const TCHAR *pFileName, const FS_i32 Mode) 
{
	_FS_PTR * fp;

	fp = (_FS_PTR*)Q_Mallco(sizeof(_FS_PTR));
	MemSet((void*)fp,0,sizeof(_FS_PTR));

	fp->error = f_open(&fp->fs.file,pFileName,Mode);

	if(fp->error!=FR_OK) 
	{
		Q_Free((void *)fp );
		fp = 0;
	}

	return (FS_FILE*)fp;
}

FS_i32 FS_FClose(FS_FILE* pFile) 
{
	_FS_PTR * fp = (_FS_PTR *)pFile;

	if (!pFile)	return -1;  /* No pointer to a FS_FILE structure */

	fp->error = f_close(&fp->fs.file);

	if(fp->error!=FR_OK) return -1;

	Q_Free((void *)pFile);

	return 0;
}

FS_size_t FS_FWrite(const void *pData, FS_size_t Size, FS_size_t N, FS_FILE *pFile) 
{
	UINT  i;
	_FS_PTR * fp = (_FS_PTR *)pFile;

	 if (!pFile) return 0; /* No pointer to a FS_FILE structure */

	fp->error = f_write(&fp->fs.file,pData,Size*N,&i);

	if(fp->error!=FR_OK) return 0;

	return i;  
}

FS_size_t FS_FRead(void *pData, FS_size_t Size, FS_size_t N, FS_FILE *pFile) 
{
	UINT i=0;
	_FS_PTR * fp = (_FS_PTR *)pFile;

	if (!pFile) return 0; /* No pointer to a FS_FILE structure */

	fp->error = f_read(&fp->fs.file,pData,Size*N,&i);

	if(fp->error!=FR_OK) return 0;

	return i;  
}

FS_i32 FS_FSeek(FS_FILE *pFile, FS_i32 Offset, FS_i32 Whence) 
{
	FS_i32 value;
	_FS_PTR * fp = (_FS_PTR *)pFile;
	  
	if (!pFile) return -1;

	if(FS_SEEK_SET==Whence) value = 0;
	else if(FS_SEEK_CUR==Whence)	value = fp->fs.file.fptr;
	else if(FS_SEEK_END==Whence)	value = fp->fs.file.fsize;
	else	return -1;
  	
	fp->error = f_lseek(&fp->fs.file,value+Offset);

	if(fp->error!=FR_OK) return -1;

	return 0;
}


FS_i32 FS_FTell(FS_FILE *pFile) 
{
	_FS_PTR * fp = (_FS_PTR *)pFile;

	if (!pFile) return -1;

	return fp->fs.file.fptr;
}

FS_DIR *FS_OpenDir( const TCHAR *dirname )
{
	_FS_PTR * dir;

	dir = (_FS_PTR *)Q_Mallco(sizeof(_FS_PTR));
	MemSet((void*)dir,0,sizeof(_FS_PTR));

	dir->error= f_opendir(&dir->fs.dir,dirname);

	if(dir->error!=FR_OK)
	{
		Q_Free((void *)dir );
		dir= 0;
	}

	return (FS_DIR *)dir;
}


#if _MAX_LFN > (NAME_MAX/2)
#error " _MAX_LFN must less than NAME_MAX/2"
#endif

struct dirent * FS_ReadDir( FS_DIR *dirp)
{
	_FS_PTR * dir = (_FS_PTR *) dirp;

	FILINFO FileInfo;
	static struct dirent ent;
	char *fn;
#if _USE_LFN
	FileInfo.lfname = (TCHAR*)LenFileName;
	FileInfo.lfsize = sizeof(LenFileName);
#endif

	ent.d_name[0]=0;

	if (dirp == 0) {
  		return 0;  /* Device not found */
	}

	 dir->error = f_readdir (
	  &dir->fs.dir,	 /* Pointer to the open directory object */
	  &FileInfo  /* Pointer to the file information structure */
	);

   if(dir->error!=FR_OK)
   {
	  ent.d_name[0]=0;
	  ent.st_mode=0;
	  ent.id = 0;
	  ent.st_ctime = 0;

	  return 0;
   }

#if _USE_LFN
   fn = (char *)FileInfo.lfname;
#else
   fn = (char *)FileInfo.fname;
#endif

   if(FileInfo.fname[0] == 0) //end
	 ent.d_name[0]=0;
   else
   	 strcpy(ent.d_name,fn);

   ent.st_mode = (FileInfo.fattrib & AM_DIR)?_IFDIR:0;
   ent.id = FileInfo.fsize;
   ent.st_ctime = FileInfo.ftime|(FileInfo.fdate<<16);
   
   return &ent;
}

struct dirent * FS_ReadDir_IDX( FS_DIR *dirp,unsigned short idx)
{
	_FS_PTR * dir = (_FS_PTR *) dirp;

	FILINFO FileInfo;
	static struct dirent ent;
	char *fn;
#if _USE_LFN
	FileInfo.lfname = (TCHAR*)LenFileName;
	FileInfo.lfsize = sizeof(LenFileName);
#endif

	ent.d_name[0]=0;

	if (dirp == 0) {
  		return 0;  /* Device not found */
	}

	  dir->error = f_readdir (
	   &dir->fs.dir,  /* Pointer to the open directory object */
	   0  /* Pointer to the file information structure */
	 );

	for(;idx;idx--)
		f_readdir_noinfo(&dir->fs.dir,&FileInfo);

	 dir->error = f_readdir (
	  &dir->fs.dir,	 /* Pointer to the open directory object */
	  &FileInfo  /* Pointer to the file information structure */
	);

   if(dir->error!=FR_OK)
   {
	  ent.d_name[0]=0;
	  ent.st_mode=0;
	  ent.id = 0;
	  ent.st_ctime = 0;

	  return 0;
   }

#if _USE_LFN
   fn = (char *)FileInfo.lfname;
#else
   fn = (char *)FileInfo.fname;
#endif

   if(FileInfo.fname[0] == 0) //end
	 ent.d_name[0]=0;
   else
   	strcpy(ent.d_name,fn);

   ent.st_mode = (FileInfo.fattrib & AM_DIR)?_IFDIR:0;
   ent.id = FileInfo.fsize;
   ent.st_ctime = FileInfo.ftime|(FileInfo.fdate<<16);
   
   return &ent;
}

void FS_RewindDir(FS_DIR *dirp) 
{
	_FS_PTR * dir = (_FS_PTR *) dirp;

	if (!dirp) return;

	dir->fs.dir.index = 0;
}

FS_i32 FS_CloseDir( FS_DIR *dirp )
{
    if(dirp == 0) return -1;

    Q_Free((void *)dirp);

	return 0;
}

FS_i32 FS_Unlink(const TCHAR *Path) 
{
	  if(f_unlink(Path)!=FR_OK) return -1;

	  return 0;
}

FS_i32 FS_GetFileSize(FS_FILE *pFile)
{
	_FS_PTR * fp = (_FS_PTR *)pFile;

	return fp->fs.file.fsize;
}

FS_i32 FS_Stat(const TCHAR *path,struct stat *buf)
{
	FRESULT res;
	FILINFO stat;
#if _USE_LFN
	stat.lfname = (TCHAR*)LenFileName;
	stat.lfsize = sizeof(LenFileName);
#endif

	res=f_stat(path,&stat);

 	if(res!=FR_OK)
	{
		buf->st_size = 0;
		buf->st_mode = 0;
		buf->st_mtime = 0;
 	    return -1;
	}

 	buf->st_size = stat.fsize;
	buf->st_mode =  (stat.fattrib & AM_DIR)?_IFDIR:0;
	buf->st_mtime = stat.ftime|(stat.fdate<<16);

	return 0;
}

FS_i32 FS_Rename(const TCHAR *OldName,const TCHAR *NewName)
{
	FRESULT res ;

	res =f_rename (OldName,NewName);

	if(res!=FR_OK)  return -1;

	return 0;
}

#define COPY_FILE_BUFFER	512
FS_i32 FS_FileCpy(const TCHAR*path1, const TCHAR*path2)		//path1: souce file    path2: destination file
{
	FS_FILE *src;
	FS_FILE * dst;
	FS_i32 read_size,write_size;
	FS_i32 src_size, dst_size;
	struct stat buf;
	_DISK_INFO info;
	FS_i8 *buffer;

	if( !strcmp((const char *)path1, (const char *)path2) )
	{
		FS_Debug("ERROR:can not copy a file to itself\r\n");
		return 0;
	}
	
	if( -1 == FS_Stat( path1, &buf ) ) return 0;

	src_size = buf.st_size;

	if(GetDiskInfo(path2,&info)==-1)
	{
		FS_Debug("ERROR:getdiskinfo err\r\n");
		return 0;
	}
		
	if(src_size > info.free_size)
	{
		FS_Debug("there is no enough space on the flash!\n\r");
		return 0;
	}

	src = FS_FOpen(path1, FA_READ|FA_OPEN_EXISTING);
	if(0 == src)
	{
		FS_Debug("ERROR:cannot open the source file\r\n");
		return 0;
	}

	dst = FS_FOpen(path2, FA_CREATE_ALWAYS|FA_WRITE);
	if(0 == dst)
	{
		FS_Debug("ERROR:cannot create the distance file\r\n");
		FS_FClose(src);
		return 0;
	}

	buffer=Q_Mallco(COPY_FILE_BUFFER);

	do{		
		read_size = FS_FRead(buffer, COPY_FILE_BUFFER,1,src);
		write_size = FS_FWrite(buffer, read_size,1,dst);

		if(write_size < read_size)
		{
			FS_Debug("ERROR:file write error\r\n");
			Q_Free(buffer);
			goto CP_FILE_ERROR;
		}
	}while(read_size == COPY_FILE_BUFFER);

	Q_Free(buffer);
	FS_FClose(src);
	FS_FClose(dst);

	if( -1 == FS_Stat( path2, &buf ) ) return 0;

	dst_size = buf.st_size;

	if(dst_size < src_size)
	{
		FS_Debug("there is an unkown flash operation during the copyfile!\n\r");
		FS_Unlink(path2);
		return 0;
	}

	return 1;

CP_FILE_ERROR:
	FS_FClose(src);
	FS_FClose(dst);
	return 0;

}

FS_i32 GetDiskInfo(const TCHAR* Path,_DISK_INFO* info)
{
    FATFS *fs;
    DWORD fre_clust, tot_sect,fre_sect;
	FRESULT res;

    /* Get volume information and free clusters of drive 1 */
    res = f_getfree(Path, &fre_clust, &fs);
    if (res) 
    {
    	info->total_size = 0;
		info->free_size = 0;
		return -1;
    }
    /* Get total sectors and free sectors */
    tot_sect = (fs->n_fatent - 2) * fs->csize;
    fre_sect = fre_clust * fs->csize;

	info->total_size = tot_sect *512;
	info->free_size = fre_sect *512;
	
	return 0;
}


FS_i32 FS_Init(void) 
{
	FS_i32 i = _VOLUMES;

	for(i=0;i<_VOLUMES;i++) f_mount(i, &FileSystemArea[i]);

	return 0;
}


FS_i32 FS_Exit(void) {
  
  FS_i32 i = _VOLUMES;
  
  while(i--) f_mount(0, NULL);
  
  return 0;
}



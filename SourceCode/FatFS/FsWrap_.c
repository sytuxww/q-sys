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

static FATFS fs[_VOLUMES];		 /* Work area (file system object) for logical drives */

#ifndef __BOOT__

typedef struct {
	FRESULT error;
	union 
	{
		FIL file;
		DIR dir;
	}fs;
} _FS_PTR;


typedef struct {
  FS_i32 status;
  _FS_PTR memory;
} _FS_MEM;

#define FS_MEM_SIZE sizeof(_FS_PTR)

static _FS_MEM  _FS_memblock[FS_MAX_MEM];
int stricmp(const char *s1, const char *s2);


#ifndef __BOOT__
#include  "ucos_ii.h"
#include <stdio.h>

#define DEBUG_PRINT printf

static  OS_EVENT  *FS_SemFileHandle;
static  OS_EVENT  *FS_SemFileOps;
static OS_EVENT   *FS_SemDirHandle;
static OS_EVENT   *FS_SemDirOps;
static  OS_EVENT  *FS_SemMemManager;

int  FS_X_OS_Init (void) 
{
    FS_SemFileHandle = OSSemCreate(1);
    FS_SemFileOps    = OSSemCreate(1);
    FS_SemDirHandle  = OSSemCreate(1);
    FS_SemMemManager  = OSSemCreate(1);
    FS_SemDirOps     = OSSemCreate(1);

	if(!(FS_SemFileHandle&&
		FS_SemFileOps&&
		FS_SemDirHandle&&
		FS_SemMemManager&&
		FS_SemDirOps))
	{
		DEBUG_PRINT("FSW_ERROR:%s Fail\r\n!",__func__);
		while(1);
	}
	
    return (0);
}
int  FS_X_OS_Exit (void) 
{
    INT8U  err;
    OSSemDel(FS_SemFileHandle, OS_DEL_ALWAYS, &err);
    OSSemDel(FS_SemFileOps   , OS_DEL_ALWAYS, &err);
    OSSemDel(FS_SemDirHandle , OS_DEL_ALWAYS, &err);
    OSSemDel(FS_SemMemManager , OS_DEL_ALWAYS, &err);
    OSSemDel(FS_SemDirOps    , OS_DEL_ALWAYS, &err);
    return (0);
}
#endif
void  FS_X_OS_LockMem (void)
{
#ifndef __BOOT__
    INT8U  err;
    OSSemPend(FS_SemMemManager, 0, &err);
#endif
}

void  FS_X_OS_UnlockMem (void) 
{
#ifndef __BOOT__
    OSSemPost(FS_SemMemManager);
#endif
}

void  FS_X_OS_LockFileHandle (void) 
{
#ifndef __BOOT__
    INT8U  err;
    OSSemPend(FS_SemFileHandle, 0, &err);
#endif
}

void FS_X_OS_LockDirHandle (void)
{
#ifndef __BOOT__
    INT8U  err;
    OSSemPend(FS_SemDirHandle, 0, &err);
#endif
}
void FS_X_OS_UnlockDirHandle (void)
{
#ifndef __BOOT__
    OSSemPost(FS_SemDirHandle);
#endif
}
void  FS_X_OS_UnlockFileHandle (void) 
{
#ifndef __BOOT__
    OSSemPost(FS_SemFileHandle);
#endif
}

void FS_X_OS_LockDirOp (void)
{
#ifndef __BOOT__
    INT8U  err;
    OSSemPend(FS_SemDirOps, 0, &err);
#endif
}

void FS_X_OS_UnlockDirOp (void) 
{
#ifndef __BOOT__
    OSSemPost(FS_SemDirOps);
#endif
}

void  FS_X_OS_LockFileOp (void) 
{
#ifndef __BOOT__
	INT8U  err;
    OSSemPend(FS_SemFileOps, 0, &err);
#endif
}
void  FS_X_OS_UnlockFileOp (void) 
{
#ifndef __BOOT__
    OSSemPost(FS_SemFileOps);
#endif
}

static void *FS__CLIB_memset(void *s, FS_i32 c, FS_size_t n) {
  const FS_u8 uc = c;
  FS_u8 *su = (FS_u8 *)s;
  
  for (; 0 < n; ++su, --n) {
    *su = uc;
  }
  return s;
}

static void *FS_malloc(void) {
  FS_i32 i;

  FS_X_OS_LockMem();
for (i = 0; i < FS_MAX_MEM; i++) {
  if (_FS_memblock[i].status == 0) {
    _FS_memblock[i].status = 1;
    FS__CLIB_memset((void*)&_FS_memblock[i].memory, 0, (FS_size_t)FS_MEM_SIZE);
    FS_X_OS_UnlockMem();
    return ((void*)&(_FS_memblock[i].memory));
  }
}
  FS_X_OS_UnlockMem();
  return 0;
}

static void FS_free(void *pBuffer) {
FS_i32 i;

FS_X_OS_LockMem();
for (i = 0; i < FS_MAX_MEM; i++) {
  if (((void*)&_FS_memblock[i].memory) == pBuffer) {
	_FS_memblock[i].status = 0;
	FS_X_OS_UnlockMem();
	return;
  }
}
FS_X_OS_UnlockMem();
}

FS_FILE* FS_FOpen(const TCHAR *pFileName, const FS_i32 Mode) 
{
  _FS_PTR * fp;

  FS_X_OS_LockFileHandle();	  

  fp = (_FS_PTR*)FS_malloc();
  if (fp == 0) {
	  FS_X_OS_UnlockFileHandle();   
	  DEBUG_PRINT("FSW_ERROR:Open %s ,but fs malloc fail \r\n",pFileName);
    return 0;  /* Device not found */
  }

  fp->error = f_open(&fp->fs.file,pFileName , Mode);

  if (fp->error!=FR_OK) 
  {
  	FS_free((void *)fp );
  	fp = 0;
  }

  FS_X_OS_UnlockFileHandle();	

  return (FS_FILE*)fp;
}


FS_i32 FS_FClose(FS_FILE* pFile) 
{
  _FS_PTR * fp = (_FS_PTR *)pFile;

  if (!pFile) {
    return -1;  /* No pointer to a FS_FILE structure */
  }

  FS_X_OS_LockFileHandle();

  fp->error = f_close(&fp->fs.file);

  if(fp->error!=FR_OK)
  {
  	FS_X_OS_UnlockFileHandle();
  	return -1;
  }

  FS_free((void *)pFile);

  FS_X_OS_UnlockFileHandle();
  return 0;
}

/* 返回值和标准fwrite不同  */
/* 返回的是总的字节数 */
FS_size_t FS_FWrite(const void *pData, FS_size_t Size, FS_size_t N, FS_FILE *pFile) 
{
  UINT  i;
  _FS_PTR * fp = (_FS_PTR *)pFile;

  if (!pFile) {
    return 0; /* No pointer to a FS_FILE structure */
  }

   FS_X_OS_LockFileOp();

   fp->error = f_write (
	&fp->fs.file,	 /* Pointer to the file object structure */
	pData,  /* Pointer to the data to be written */
	Size*N,	 /* Number of bytes to write */
	&i	 /* Pointer to the variable to return number of bytes written */
  );

  if(fp->error!=FR_OK)
  {
  	FS_X_OS_UnlockFileOp();
  	return 0;
  } 

  FS_X_OS_UnlockFileOp();
  return i;  
}


/* 返回值和标准fread不同  */
/* 返回的是总的字节数 */
FS_size_t FS_FRead(void *pData, FS_size_t Size, FS_size_t N, FS_FILE *pFile) 
{
  UINT i;
  _FS_PTR * fp = (_FS_PTR *)pFile;

  if (!pFile) {
    return 0;  /* No pointer to a FS_FILE structure */
  }

  FS_X_OS_LockFileOp();

  i = 0;

   fp->error = f_read (
	&fp->fs.file,	/* Pointer to the file object structure */
	pData,		/* Pointer to the buffer to store read data */
	Size*N,	/* Number of bytes to read */
	&i		/* Pointer to the variable to return number of bytes read */
  );

  if(fp->error!=FR_OK)
  {
  	 FS_X_OS_UnlockFileOp();
	 return 0;
  }

  FS_X_OS_UnlockFileOp();
  return i;  
}

#define FS_SEEK_CUR         1
#define FS_SEEK_END         2
#define FS_SEEK_SET         0 /*文件开头*/

FS_i32 FS_FSeek(FS_FILE *pFile, FS_i32 Offset, FS_i32 Whence) 
{
  FS_i32 value;
  _FS_PTR * fp = (_FS_PTR *)pFile;
    
  if (!pFile) {
    return -1;
  }

  if(FS_SEEK_SET==Whence)
  {
	value = 0;
  }
  else if(FS_SEEK_CUR==Whence)
  {
	value = fp->fs.file.fptr;
  }
  else if(FS_SEEK_END==Whence)
  {
	value = fp->fs.file.fsize;
  }
  else
  {
	return -1;
  }
  	
   fp->error = f_lseek (
	&fp->fs.file,   /* Pointer to the file object structure */
	value+Offset	   /* File offset in unit of byte */
   );

  if(fp->error!=FR_OK)
	 return -1;

  return 0;
}


FS_i32 FS_FTell(FS_FILE *pFile) 
{
  _FS_PTR * fp = (_FS_PTR *)pFile;

  if (!pFile) {
    return -1;
  }
  
  return fp->fs.file.fptr;
}

FS_DIR *FS_OpenDir( const TCHAR *dirname )
{
	_FS_PTR * dir;

	FS_X_OS_LockDirHandle();
	dir = (_FS_PTR *)FS_malloc();
	if (dir == 0) {
		FS_X_OS_UnlockDirHandle();
		DEBUG_PRINT("FSW_ERROR:OpenDir %s ,but fs malloc fail \r\n",dirname);
  		return 0;  /* Device not found */
	}

	dir->error= f_opendir (
	  &dir->fs.dir,       /* Pointer to the blank directory object structure */
	  dirname  /* Pointer to the directory name */
	);

  	if(dir->error!=FR_OK)
  	{
  		FS_free((void *)dir );
  		dir= 0;
	}

	FS_X_OS_UnlockDirHandle();

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
	static WCHAR lfn[_MAX_LFN+1];
	FileInfo.lfname = (TCHAR*)lfn;
	FileInfo.lfsize = sizeof(lfn);
#endif

	ent.d_name[0]=0;

	if (dirp == 0) {
  		return 0;  /* Device not found */
	}

	 FS_X_OS_LockDirOp();

	 dir->error = f_readdir (
	  &dir->fs.dir,	 /* Pointer to the open directory object */
	  &FileInfo  /* Pointer to the file information structure */
	);

   if(dir->error!=FR_OK)
   {
   	  FS_X_OS_UnlockDirOp();
	  ent.d_name[0]=0;
	  ent.st_mode=0;
	  ent.id = 0;
	  ent.st_ctime = 0;

	  return 0;
   }

#if _USE_LFN
   fn = (char *)(*FileInfo.lfname ? FileInfo.lfname : FileInfo.fname);
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

   FS_X_OS_UnlockDirOp();
   
   return &ent;
}

struct dirent * FS_ReadDir_IDX( FS_DIR *dirp,unsigned short idx)
{
	_FS_PTR * dir = (_FS_PTR *) dirp;

	FILINFO FileInfo;
	static struct dirent ent;
	char *fn;
#if _USE_LFN
	static WCHAR lfn[_MAX_LFN+1];
	FileInfo.lfname = (TCHAR*)lfn;
	FileInfo.lfsize = sizeof(lfn);
#endif

	ent.d_name[0]=0;

	if (dirp == 0) {
  		return 0;  /* Device not found */
	}

	 FS_X_OS_LockDirOp();

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
   	  FS_X_OS_UnlockDirOp();
	  ent.d_name[0]=0;
	  ent.st_mode=0;
	  ent.id = 0;
	  ent.st_ctime = 0;

	  return 0;
   }

#if _USE_LFN
   fn = (char *)(*FileInfo.lfname ? FileInfo.lfname : FileInfo.fname);
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

   FS_X_OS_UnlockDirOp();
   
   return &ent;
}

void FS_RewindDir(FS_DIR *dirp) 
{
  _FS_PTR * dir = (_FS_PTR *) dirp;

  if (!dirp) {
    return;  /* No pointer to a FS_DIR data structure */
  }
  dir->fs.dir.index = 0;
}

FS_i32 FS_CloseDir( FS_DIR *dirp )
{
    if( dirp == 0 )
    	return -1;
	FS_X_OS_LockDirHandle();	

    FS_free((void *)dirp);

	FS_X_OS_UnlockDirHandle();	

	return 0;
}

FS_i32 FS_Unlink(const TCHAR *Path) {

  FRESULT res ;

  FS_X_OS_LockDirHandle();

  res=f_unlink(Path);

  FS_X_OS_UnlockDirHandle();

  if(res!=FR_OK)
	 return -1;

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
	static WCHAR lfn[_MAX_LFN+1];
	stat.lfname = (TCHAR*)lfn;
	stat.lfsize = sizeof(lfn);
#endif

	FS_X_OS_LockDirHandle();

	res=f_stat (
	  path,  /* Pointer to the file or directory name */
	  &stat 	  /* Pointer to the FILINFO structure */
	);

 	if(res!=FR_OK)
	{
		buf->st_size = 0;
		buf->st_mode = 0;
		buf->st_mtime = 0;
		FS_X_OS_UnlockDirHandle();
 	   return -1;
	}

 
 	buf->st_size = stat.fsize;
	buf->st_mode =  (stat.fattrib & AM_DIR)?_IFDIR:0;
	buf->st_mtime = stat.ftime|(stat.fdate<<16);
	FS_X_OS_UnlockDirHandle();

	return 0;
}

FS_i32 FS_Rename(const TCHAR *OldName,const TCHAR *NewName)
{
	FRESULT res ;

	FS_X_OS_LockDirHandle();

	res =f_rename (
	  OldName, /* Pointer to old object name */
	  NewName	/* Pointer to new object name */
	);

	FS_X_OS_UnlockDirHandle();

	if(res!=FR_OK)
	   return -1;

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
		FS_Debug("FSW_ERROR:can not copy a file to itself\r\n");
		return 0;
	}
	
	if( -1 == FS_Stat( path1, &buf ) ) return 0;

	src_size = buf.st_size;

	if(GetDiskInfo(path2,&info)==-1)
	{
		FS_Debug("FSW_ERROR:getdiskinfo err\r\n");
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
		FS_Debug("FSW_ERROR:cannot open the source file\r\n");
		return 0;
	}

	dst = FS_FOpen(path2, FA_CREATE_ALWAYS|FA_WRITE);
	if(0 == dst)
	{
		FS_Debug("FSW_ERROR:cannot create the distance file\r\n");
		FS_FClose(src);
		return 0;
	}

	buffer=Q_Mallco(COPY_FILE_BUFFER);

	do{		
		read_size = FS_FRead(buffer, COPY_FILE_BUFFER,1,src);
		write_size = FS_FWrite(buffer, read_size,1,dst);

		if(write_size < read_size)
		{
			FS_Debug("FSW_ERROR:file write error\r\n");
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

#endif //#ifndef __BOOT__

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
    /* Register work area for logical drives */
	for(i=0;i<_VOLUMES;i++)
	{
    	f_mount(i, &fs[i]);
	}
#ifndef __BOOT__	
	FS_X_OS_Init ( );
#endif
   return 0;
}


FS_i32 FS_Exit(void) {
  
  FS_i32 i = _VOLUMES;
  /* Register work area for logical drives */
  while(i--)
  {
	  f_mount(0, NULL);
  }

#ifndef __BOOT__	
	  FS_X_OS_Exit( );
#endif

  return 0;
}



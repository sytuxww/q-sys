#ifndef __FS_WRAP_H__
#define __FS_WRAP_H__

#define FS_size_t       unsigned long       /* 32 bit unsigned */
#define FS_u32          unsigned long       /* 32 bit unsigned */
#define FS_i32          signed long         /* 32 bit signed */
#define FS_u16          unsigned short      /* 16 bit unsigned */
#define FS_i16          signed   short      /* 16 bit signed */
#define FS_i8           signed   char       /* 8 bit  signed */
#define FS_u8           unsigned char       /* 8 bit  unsigned */
//#define NULL 			0

typedef void FS_FILE;
typedef void FS_DIR;

typedef struct {
  FS_u32 total_size;
  FS_u32 free_size;
} _DISK_INFO;

/* Global constants*/
#define FS_SEEK_CUR         1
#define FS_SEEK_END         2
#define FS_SEEK_SET         0 /*文件开头*/

#ifndef _OPENDIR_TYPE_
#define _OPENDIR_TYPE_

typedef unsigned int size_t;
typedef short dev_t;
typedef unsigned int ino_t;
typedef unsigned int mode_t;
typedef unsigned short nlink_t;
typedef unsigned long off_t;
typedef unsigned short gid_t;
typedef unsigned short uid_t;
typedef int pid_t;
typedef int time_t;

struct stat {
    mode_t  st_mode;     /* File mode */
    ino_t   st_ino;      /* File serial number */
    dev_t   st_dev;      /* ID of device containing file */
    nlink_t st_nlink;    /* Number of hard links */
    uid_t   st_uid;      /* User ID of the file owner */
    gid_t   st_gid;      /* Group ID of the file's group */
    off_t   st_size;     /* File size (regular files only) */
    time_t  st_atime;    /* Last access time */
    time_t  st_mtime;    /* Last data modification time */
    time_t  st_ctime;    /* Last file status change time */
}; 

#define NAME_MAX 63                               /* domain */
struct dirent
{
	char        d_name[NAME_MAX+1];
	unsigned int id;	/* use for id file, and only valid when the */
	mode_t  st_mode;  /* file mode */
	int         st_ctime;  /* file create time */


    					/* size of d_name is zero */
	/* now we needn't u_name, we save file name as utf8 */    					
    //char u_name[NAME_MAX+1];
                           
};

#define	_IFMT		0170000	/* type of file */
#define		_IFDIR	0040000	/* directory */
#define		_IFCHR	0020000	/* character special */
#define		_IFBLK	0060000	/* block special */
#define		_IFREG	0100000	/* regular */
#define		_IFLNK	0120000	/* symbolic link */
#define		_IFSOCK	0140000	/* socket */
#define		_IFIFO	0010000	/* fifo */
#define 	S_BLKSIZE  1024 /* size of a block */
#define	S_ISUID		0004000	/* set user id on execution */
#define	S_ISGID		0002000	/* set group id on execution */

#define	S_ISBLK(m)	(((m)&_IFMT) == _IFBLK)
#define	S_ISCHR(m)	(((m)&_IFMT) == _IFCHR)
#define	S_ISDIR(m)	(((m)&_IFMT) == _IFDIR)
#define	S_ISFIFO(m)	(((m)&_IFMT) == _IFIFO)
#define	S_ISREG(m)	(((m)&_IFMT) == _IFREG)
#define	S_ISLNK(m)	(((m)&_IFMT) == _IFLNK)
#define	S_ISSOCK(m)	(((m)&_IFMT) == _IFSOCK) 

#endif

#ifndef _INC_TCHAR
#define _INC_TCHAR
typedef char TCHAR;
#define _T(x) x
#define _TEXT(x) x
#endif

#ifndef FA_READ
#define	FA_READ				0x01
#endif

#ifndef FA_OPEN_EXISTING
#define	FA_OPEN_EXISTING	0x00
#endif

#ifndef FA_WRITE
#define	FA_WRITE			0x02
#endif

#ifndef FA_CREATE_NEW
#define	FA_CREATE_NEW		0x04
#endif

#ifndef FA_CREATE_ALWAYS
#define	FA_CREATE_ALWAYS	0x08
#endif

#ifndef FA_OPEN_ALWAYS
#define	FA_OPEN_ALWAYS		0x10
#endif

FS_FILE* FS_FOpen(const TCHAR *pFileName, const FS_i32 Mode); 
FS_i32 FS_FClose(FS_FILE*pFile) ;
FS_size_t FS_FWrite(const void *pData, FS_size_t Size, FS_size_t N, FS_FILE *pFile); 
FS_size_t FS_FRead(void *pData, FS_size_t Size, FS_size_t N, FS_FILE *pFile); 
FS_i32 FS_FSeek(FS_FILE *pFile, FS_i32 Offset, FS_i32 Whence); 
FS_i32 FS_FTell(FS_FILE *pFile) ;

FS_DIR *FS_OpenDir( const TCHAR *dirname );
struct dirent * FS_ReadDir( FS_DIR *dirp);
struct dirent * FS_ReadDir_IDX( FS_DIR *dirp,unsigned short idx);
void FS_RewindDir(FS_DIR *pDir); 
FS_i32 FS_CloseDir( FS_DIR *dirp );
FS_i32 FS_Unlink(const TCHAR *Path); 
FS_i32 FS_Stat(const TCHAR *path,struct stat *buf);
FS_i32 FS_Rename(const TCHAR *OldName,const TCHAR *NewName);
FS_i32 FS_Init(void);
FS_i32 FS_Exit(void); 

FS_i32 GetDiskInfo(const TCHAR* Path,_DISK_INFO* info);
FS_i32 FS_FileCpy(const TCHAR*path1, const TCHAR*path2);		//path1: souce file    path2: destination file
FS_i32 FS_GetFileSize(FS_FILE *pFile);

#endif


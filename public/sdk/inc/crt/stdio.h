/***
*stdio.h - definitions/declarations for standard I/O routines
*
*       Copyright (c) 1985-2001, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This file defines the structures, values, macros, and functions
*       used by the level 2 I/O ("standard I/O") routines.
*       [ANSI/System V]
*
*       [Public]
*
****/

#if     _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_STDIO
#define _INC_STDIO

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif


#ifdef  _MSC_VER
/*
 * Currently, all MS C compilers for Win32 platforms default to 8 byte
 * alignment.
 */
#pragma pack(push,8)
#endif  /* _MSC_VER */

#ifdef  __cplusplus
extern "C" {
#endif


#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300
#define _W64 __w64
#else
#define _W64
#endif
#endif

/* Define _CRTIMP */

#ifndef _CRTIMP
#ifndef _CRTBLD
#define _CRTIMP __declspec(dllimport)
#else   /* ndef _DLL */
#define _CRTIMP __declspec(dllexport)
#endif  /* _DLL */
#endif  /* _CRTIMP */


/* Define __cdecl for non-Microsoft compilers */

#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif


#ifndef _SIZE_T_DEFINED
#ifdef  _WIN64
typedef unsigned __int64    size_t;
#else
typedef _W64 unsigned int   size_t;
#endif
#define _SIZE_T_DEFINED
#endif


#if !defined(_WCHAR_T_DEFINED) && !defined(_NATIVE_WCHAR_T_DEFINED)
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif


#ifndef _WCTYPE_T_DEFINED
typedef unsigned short wint_t;
typedef unsigned short wctype_t;
#define _WCTYPE_T_DEFINED
#endif


#ifndef _VA_LIST_DEFINED
typedef char *  va_list;
#define _VA_LIST_DEFINED
#endif


/* Buffered I/O macros */

#define BUFSIZ  512


/*
 * Default number of supported streams. _NFILE is confusing and obsolete, but
 * supported anyway for backwards compatibility.
 */
#define _NFILE      _NSTREAM_

#define _NSTREAM_   512

/*
 * Number of entries in _iob[] (declared below). Note that _NSTREAM_ must be
 * greater than or equal to _IOB_ENTRIES.
 */
#define _IOB_ENTRIES 20

#define EOF     (-1)


#ifndef _FILE_DEFINED
struct _iobuf {
        char *_ptr;
        int   _cnt;
        char *_base;
        int   _flag;
        int   _file;
        int   _charbuf;
        int   _bufsiz;
        char *_tmpfname;
        };
typedef struct _iobuf FILE;
#define _FILE_DEFINED
#endif


/* Directory where temporary files may be created. */

#ifdef  _POSIX_
#define _P_tmpdir   "/"
#define _wP_tmpdir  L"/"
#else
#define _P_tmpdir   "\\"
#define _wP_tmpdir  L"\\"
#endif

/* L_tmpnam = length of string _P_tmpdir
 *            + 1 if _P_tmpdir does not end in "/" or "\", else 0
 *            + 12 (for the filename string)
 *            + 1 (for the null terminator)
 */
#define L_tmpnam sizeof(_P_tmpdir)+12


#ifdef  _POSIX_
#define L_ctermid   9
#define L_cuserid   32
#endif


/* Seek method constants */

#define SEEK_CUR    1
#define SEEK_END    2
#define SEEK_SET    0


#define FILENAME_MAX    260
#define FOPEN_MAX       20
#define _SYS_OPEN       20
#define TMP_MAX         32767


/* Define NULL pointer value */

#ifndef NULL
#ifdef  __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif


/* Declare _iob[] array */

#ifndef _STDIO_DEFINED
_CRTIMP extern FILE _iob[];
#endif  /* _STDIO_DEFINED */


/* Define file position type */

#ifndef _FPOS_T_DEFINED
#undef _FPOSOFF

#if     defined (_POSIX_)
typedef long fpos_t;
#define _FPOSOFF(fp) ((long)(fp))
#else   /* _POSIX_ */

#if     !__STDC__ && _INTEGRAL_MAX_BITS >= 64
typedef __int64 fpos_t;
#define _FPOSOFF(fp) ((long)(fp))
#else
typedef struct fpos_t {
        unsigned int lopart;
        int          hipart;
        } fpos_t;
#define _FPOSOFF(fp) ((long)(fp).lopart)
#endif
#endif  /* _POSIX_ */

#define _FPOS_T_DEFINED
#endif


#define stdin  (&_iob[0])
#define stdout (&_iob[1])
#define stderr (&_iob[2])


#define _IOREAD         0x0001
#define _IOWRT          0x0002

#define _IOFBF          0x0000
#define _IOLBF          0x0040
#define _IONBF          0x0004

#define _IOMYBUF        0x0008
#define _IOEOF          0x0010
#define _IOERR          0x0020
#define _IOSTRG         0x0040
#define _IORW           0x0080
#ifdef  _POSIX_
#define _IOAPPEND       0x0200
#endif


/* Function prototypes */

#ifndef _STDIO_DEFINED

_CRTIMP int __cdecl _filbuf(FILE *);
_CRTIMP int __cdecl _flsbuf(int, FILE *);

#ifdef  _POSIX_
_CRTIMP FILE * __cdecl _fsopen(const char *, const char *);
#else
_CRTIMP FILE * __cdecl _fsopen(const char *, const char *, int);
#endif

_CRTIMP void __cdecl clearerr(FILE *);
_CRTIMP int __cdecl fclose(FILE *);
_CRTIMP int __cdecl _fcloseall(void);

#ifdef  _POSIX_
_CRTIMP FILE * __cdecl fdopen(int, const char *);
#else
_CRTIMP FILE * __cdecl _fdopen(int, const char *);
#endif

_CRTIMP int __cdecl feof(FILE *);
_CRTIMP int __cdecl ferror(FILE *);
_CRTIMP int __cdecl fflush(FILE *);
_CRTIMP int __cdecl fgetc(FILE *);
_CRTIMP int __cdecl _fgetchar(void);
_CRTIMP int __cdecl fgetpos(FILE *, fpos_t *);
_CRTIMP char * __cdecl fgets(char *, int, FILE *);

#ifdef  _POSIX_
_CRTIMP int __cdecl fileno(FILE *);
#else
_CRTIMP int __cdecl _fileno(FILE *);
#endif

_CRTIMP int __cdecl _flushall(void);
_CRTIMP FILE * __cdecl fopen(const char *, const char *);
_CRTIMP int __cdecl fprintf(FILE *, const char *, ...);
_CRTIMP int __cdecl fputc(int, FILE *);
_CRTIMP int __cdecl _fputchar(int);
_CRTIMP int __cdecl fputs(const char *, FILE *);
_CRTIMP size_t __cdecl fread(void *, size_t, size_t, FILE *);
_CRTIMP FILE * __cdecl freopen(const char *, const char *, FILE *);
_CRTIMP int __cdecl fscanf(FILE *, const char *, ...);
_CRTIMP int __cdecl fsetpos(FILE *, const fpos_t *);
_CRTIMP int __cdecl fseek(FILE *, long, int);
_CRTIMP long __cdecl ftell(FILE *);
_CRTIMP size_t __cdecl fwrite(const void *, size_t, size_t, FILE *);
_CRTIMP int __cdecl getc(FILE *);
_CRTIMP int __cdecl getchar(void);
_CRTIMP int __cdecl _getmaxstdio(void);
_CRTIMP char * __cdecl gets(char *);
_CRTIMP int __cdecl _getw(FILE *);
_CRTIMP void __cdecl perror(const char *);
_CRTIMP int __cdecl _pclose(FILE *);
_CRTIMP FILE * __cdecl _popen(const char *, const char *);
_CRTIMP int __cdecl printf(const char *, ...);
_CRTIMP int __cdecl putc(int, FILE *);
_CRTIMP int __cdecl putchar(int);
_CRTIMP int __cdecl puts(const char *);
_CRTIMP int __cdecl _putw(int, FILE *);
_CRTIMP int __cdecl remove(const char *);
_CRTIMP int __cdecl rename(const char *, const char *);
_CRTIMP void __cdecl rewind(FILE *);
_CRTIMP int __cdecl _rmtmp(void);
_CRTIMP int __cdecl scanf(const char *, ...);
_CRTIMP void __cdecl setbuf(FILE *, char *);
_CRTIMP int __cdecl _setmaxstdio(int);
_CRTIMP int __cdecl setvbuf(FILE *, char *, int, size_t);
_CRTIMP int __cdecl _snprintf(char *, size_t, const char *, ...);
_CRTIMP int __cdecl sprintf(char *, const char *, ...);
_CRTIMP int __cdecl _scprintf(const char *, ...);
_CRTIMP int __cdecl sscanf(const char *, const char *, ...);
_CRTIMP int __cdecl _snscanf(const char *, size_t, const char *, ...);
_CRTIMP char * __cdecl _tempnam(const char *, const char *);
_CRTIMP FILE * __cdecl tmpfile(void);
_CRTIMP char * __cdecl tmpnam(char *);
_CRTIMP int __cdecl ungetc(int, FILE *);
_CRTIMP int __cdecl _unlink(const char *);
_CRTIMP int __cdecl vfprintf(FILE *, const char *, va_list);
_CRTIMP int __cdecl vprintf(const char *, va_list);
_CRTIMP int __cdecl _vsnprintf(char *, size_t, const char *, va_list);
_CRTIMP int __cdecl vsprintf(char *, const char *, va_list);
_CRTIMP int __cdecl _vscprintf(const char *, va_list);

#ifndef _WSTDIO_DEFINED

/* wide function prototypes, also declared in wchar.h  */

#ifndef WEOF
#define WEOF (wint_t)(0xFFFF)
#endif

#ifdef  _POSIX_
_CRTIMP FILE * __cdecl _wfsopen(const wchar_t *, const wchar_t *);
#else
_CRTIMP FILE * __cdecl _wfsopen(const wchar_t *, const wchar_t *, int);
#endif

_CRTIMP wint_t __cdecl fgetwc(FILE *);
_CRTIMP wint_t __cdecl _fgetwchar(void);
_CRTIMP wint_t __cdecl fputwc(wchar_t, FILE *);
_CRTIMP wint_t __cdecl _fputwchar(wchar_t);
_CRTIMP wint_t __cdecl getwc(FILE *);
_CRTIMP wint_t __cdecl getwchar(void);
_CRTIMP wint_t __cdecl putwc(wchar_t, FILE *);
_CRTIMP wint_t __cdecl putwchar(wchar_t);
_CRTIMP wint_t __cdecl ungetwc(wint_t, FILE *);

_CRTIMP wchar_t * __cdecl fgetws(wchar_t *, int, FILE *);
_CRTIMP int __cdecl fputws(const wchar_t *, FILE *);
_CRTIMP wchar_t * __cdecl _getws(wchar_t *);
_CRTIMP int __cdecl _putws(const wchar_t *);

_CRTIMP int __cdecl fwprintf(FILE *, const wchar_t *, ...);
_CRTIMP int __cdecl wprintf(const wchar_t *, ...);
_CRTIMP int __cdecl _snwprintf(wchar_t *, size_t, const wchar_t *, ...);
_CRTIMP int __cdecl swprintf(wchar_t *, const wchar_t *, ...);
_CRTIMP int __cdecl _scwprintf(const wchar_t *, ...);
_CRTIMP int __cdecl vfwprintf(FILE *, const wchar_t *, va_list);
_CRTIMP int __cdecl vwprintf(const wchar_t *, va_list);
_CRTIMP int __cdecl _vsnwprintf(wchar_t *, size_t, const wchar_t *, va_list);
_CRTIMP int __cdecl vswprintf(wchar_t *, const wchar_t *, va_list);
_CRTIMP int __cdecl _vscwprintf(const wchar_t *, va_list);
_CRTIMP int __cdecl fwscanf(FILE *, const wchar_t *, ...);
_CRTIMP int __cdecl swscanf(const wchar_t *, const wchar_t *, ...);
_CRTIMP int __cdecl _snwscanf(const wchar_t *, size_t, const wchar_t *, ...);
_CRTIMP int __cdecl wscanf(const wchar_t *, ...);

#define getwchar()              fgetwc(stdin)
#define putwchar(_c)            fputwc((_c),stdout)
#define getwc(_stm)             fgetwc(_stm)
#define putwc(_c,_stm)          fputwc(_c,_stm)

_CRTIMP FILE * __cdecl _wfdopen(int, const wchar_t *);
_CRTIMP FILE * __cdecl _wfopen(const wchar_t *, const wchar_t *);
_CRTIMP FILE * __cdecl _wfreopen(const wchar_t *, const wchar_t *, FILE *);
_CRTIMP void __cdecl _wperror(const wchar_t *);
_CRTIMP FILE * __cdecl _wpopen(const wchar_t *, const wchar_t *);
_CRTIMP int __cdecl _wremove(const wchar_t *);
_CRTIMP wchar_t * __cdecl _wtempnam(const wchar_t *, const wchar_t *);
_CRTIMP wchar_t * __cdecl _wtmpnam(wchar_t *);


#define _WSTDIO_DEFINED
#endif  /* _WSTDIO_DEFINED */

#define _STDIO_DEFINED
#endif  /* _STDIO_DEFINED */


/* Macro definitions */

#define feof(_stream)     ((_stream)->_flag & _IOEOF)
#define ferror(_stream)   ((_stream)->_flag & _IOERR)
#define _fileno(_stream)  ((_stream)->_file)
#define getc(_stream)     (--(_stream)->_cnt >= 0 \
                ? 0xff & *(_stream)->_ptr++ : _filbuf(_stream))
#define putc(_c,_stream)  (--(_stream)->_cnt >= 0 \
                ? 0xff & (*(_stream)->_ptr++ = (char)(_c)) :  _flsbuf((_c),(_stream)))
#define getchar()         getc(stdin)
#define putchar(_c)       putc((_c),stdout)



#ifdef  _MT
#undef  getc
#undef  putc
#undef  getchar
#undef  putchar
#endif



#if     !__STDC__ && !defined(_POSIX_)

/* Non-ANSI names for compatibility */

#define P_tmpdir  _P_tmpdir
#define SYS_OPEN  _SYS_OPEN

_CRTIMP int __cdecl fcloseall(void);
_CRTIMP FILE * __cdecl fdopen(int, const char *);
_CRTIMP int __cdecl fgetchar(void);
_CRTIMP int __cdecl fileno(FILE *);
_CRTIMP int __cdecl flushall(void);
_CRTIMP int __cdecl fputchar(int);
_CRTIMP int __cdecl getw(FILE *);
_CRTIMP int __cdecl putw(int, FILE *);
_CRTIMP int __cdecl rmtmp(void);
_CRTIMP char * __cdecl tempnam(const char *, const char *);
_CRTIMP int __cdecl unlink(const char *);

#endif  /* __STDC__ */

#ifdef  __cplusplus
}
#endif

#ifdef  _MSC_VER
#pragma pack(pop)
#endif  /* _MSC_VER */

#endif  /* _INC_STDIO */

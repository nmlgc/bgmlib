// Music Room Interface
// --------------------
// platform.h - The most basic includes
// --------------------
// "©" Nmlgc, 2011

#ifndef BGMLIB_PLATFORM_H
#define BGMLIB_PLATFORM_H

// Definitions
// -----------
#define MIN(a, b)		        ((a) < (b) ? (a) : (b))						// Minimum
#define MAX(a, b)               ((a) > (b) ? (a) : (b))						// Maximum
#define BETWEEN(a, b, c)        ((a) > (b) && (a) < (c) ? true : false)		// Checks if a is between b and c
#define BETWEEN_EQUAL(a, b, c)	((a) >= (b) && (a) <= (c) ? true : false)	// Checks if a is between b and c or is equals b/c 
#define SAFE_DELETE(x)          {if(x) {delete   (x); (x) = NULL;}}			// Safe deletion
#define SAFE_DELETE_ARRAY(x)	{if(x) {delete[] (x); (x) = NULL;}}			// Safe deletion of an array
#define SAFE_FREE(x)			{if(x) {free( (x) );  (x) = NULL;}}			// Safe deletion of a malloc buffer

#define SINGLETON(Class)	public: static Class& Inst()	{static Class Inst;	return Inst;}

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned char u8;
typedef unsigned int uint;
typedef unsigned long ulong;

// Platform dependant macros
#ifndef WIN32

// Unix File System
#define ZeroMemory(p, s)    memset(p, 0, s)
#define DirSlash '/'
#define SlashString "/"
#define OtherDirSlash '\\'
#define OtherSlashString "\\"
const char LineEnd[1] = {'\n'};
const char OtherLineEnd[2] = {'\r', '\n'};
#else

// Windows File System
#define DirSlash '\\'
#define SlashString "\\"
#define OtherDirSlash '/'
#define OtherSlashString "/"
const char LineEnd[2] = {'\r', '\n'};
const char OtherLineEnd[1] = {'\n'};

#define FORMAT_INT64	  "%I64d"
#define FORMAT_INT64_TIME "+%I64d"
#endif

const char LineBreak[2] = {'\r', '\n'};
const uchar utf8bom[3] = {0xEF, 0xBB, 0xBF};
const char Moonspace[3] = {(char)0xE3, (char)0x80, (char)0x80};	// Full-width space

// Error codes
enum mrerr
{
	ERROR_FILE_ACCESS = -1,
	ERROR_GENERIC = 0,
	SUCCESS = 1,
};

#define TIMEOUT 5000000	// Short timeout sleeping time

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <fxdefs.h>
#include <FXString.h>

namespace FX
{
	class FXFile;
}

#ifdef PROFILING_LIBS
// Enable timeGetTime() and QueryPerformanceCounter() profiling
#include <windows.h>
#include <mmsystem.h>

extern LARGE_INTEGER operator - (const LARGE_INTEGER& a, const LARGE_INTEGER& b);
extern LARGE_INTEGER operator + (const LARGE_INTEGER& a, const LARGE_INTEGER& b);
#endif

using namespace FX;

#endif /* BGMLIB_PLATFORM_H */
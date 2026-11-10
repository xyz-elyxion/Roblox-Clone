/* rblxstb.h - v2.37 - Sean's Tool Box -- public domain -- http://nothings.org/rblxstb.h
          no warranty is offered or implied; use this code at your own risk

   This is a single header file with a bunch of useful utilities
   for getting stuff done in C/C++.

   Documentation: http://nothings.org/rblxstb/rblxstb_h.html
   Unit tests:    http://nothings.org/rblxstb/rblxstb.c

 ============================================================================
   You MUST

      #define RBLXSTB_DEFINE

   in EXACTLY _one_ C or C++ file that includes this header, BEFORE the
   include, like this:

      #define RBLXSTB_DEFINE
      #include "rblxstb.h"

   All other files should just #include "rblxstb.h" without the #define.
 ============================================================================

Version History

   2.36   various fixes
   2.35   fix clang-cl issues with swprintf
   2.34   fix warnings
   2.33   more fixes to random numbers
   2.32   rblxstb_intcmprev, rblxstb_uidict, fix random numbers on Linux
   2.31   rblxstb_ucharcmp
   2.30   MinGW fix
   2.29   attempt to fix use of swprintf()
   2.28   various new functionality
   2.27   test _WIN32 not WIN32 in RBLXSTB_THREADS
   2.26   various warning & bugfixes
   2.25   various warning & bugfixes
   2.24   various warning & bugfixes
   2.23   fix 2.22
   2.22   64-bit fixes from '!='; fix rblxstb_sdict_copy() to have preferred name
   2.21   utf-8 decoder rejects "overlong" encodings; attempted 64-bit improvements
   2.20   fix to hash "copy" function--reported by someone with handle "!="
   2.19   ???
   2.18   rblxstb_readdir_subdirs_mask
   2.17   rblxstb_cfg_dir
   2.16   fix rblxstb_bgio_, add rblxstb_bgio_stat(); begin a streaming wrapper
   2.15   upgraded hash table template to allow:
            - aggregate keys (explicit comparison func for EMPTY and DEL keys)
            - "static" implementations (so they can be culled if unused)
   2.14   rblxstb_mprintf
   2.13   reduce identifiable strings in RBLXSTB_NO_STB_STRINGS
   2.12   fix RBLXSTB_ONLY -- lots of uint32s, TRUE/FALSE things had crept in
   2.11   fix bug in rblxstb_dirtree_get() which caused "c://path" sorts of stuff
   2.10   RBLXSTB_F(), RBLXSTB_I() inline constants (also KI,KU,KF,KD)
   2.09   rblxstb_box_face_vertex_axis_side
   2.08   bugfix rblxstb_trimwhite()
   2.07   colored printing in windows (why are we in 1985?)
   2.06   comparison functions are now functions-that-return-functions and
          accept a struct-offset as a parameter (not thread-safe)
   2.05   compile and pass tests under Linux (but no threads); thread cleanup
   2.04   rblxstb_cubic_bezier_1d, smoothstep, avoid dependency on registry
   2.03   ?
   2.02   remove integrated documentation
   2.01   integrate various fixes; rblxstb_force_uniprocessor
   2.00   revised rblxstb_dupe to use multiple hashes
   1.99   rblxstb_charcmp
   1.98   rblxstb_arr_deleten, rblxstb_arr_insertn
   1.97   fix rblxstb_newell_normal()
   1.96   rblxstb_hash_number()
   1.95   hack rblxstb__rec_max; clean up recursion code to use new functions
   1.94   rblxstb_dirtree; rename rblxstb_extra to rblxstb_ptrmap
   1.93   rblxstb_sem_new() API cleanup (no blockflag-starts blocked; use 'extra')
   1.92   rblxstb_threadqueue--multi reader/writer queue, fixed size or resizeable
   1.91   rblxstb_bgio_* for reading disk asynchronously
   1.90   rblxstb_mutex uses CRITICAL_REGION; new rblxstb_sync primitive for thread
          joining; workqueue supports rblxstb_sync instead of rblxstb_semaphore
   1.89   support ';' in constant-string wildcards; rblxstb_mutex wrapper (can
          implement with EnterCriticalRegion eventually)
   1.88   portable threading API (only for win32 so far); worker thread queue
   1.87   fix wildcard handling in rblxstb_readdir_recursive
   1.86   support ';' in wildcards
   1.85   make rblxstb_regex work with non-constant strings;
               beginnings of rblxstb_introspect()
   1.84   (forgot to make notes)
   1.83   whoops, rblxstb_keep_if_different wasn't deleting the temp file
   1.82   bring back rblxstb_compress from rblxstb_file.h for cmirror
   1.81   various bugfixes, RBLXSTB_FASTMALLOC_INIT inits FASTMALLOC in release
   1.80   rblxstb_readdir returns utf8; write own utf8-utf16 because lib was wrong
   1.79   rblxstb_write
   1.78   calloc() support for malloc wrapper, RBLXSTB_FASTMALLOC
   1.77   RBLXSTB_FASTMALLOC
   1.76   RBLXSTB_STUA - Lua-like language; (rblxstb_image, rblxstb_csample, rblxstb_bilinear)
   1.75   alloc/free array of blocks; rblxstb_hheap bug; a few rblxstb_ps_ funcs;
          hash*getkey, hash*copy; rblxstb_bitset; rblxstb_strnicmp; bugfix rblxstb_bst
   1.74   rblxstb_replaceinplace; use stdlib C function to convert utf8 to UTF-16
   1.73   fix performance bug & leak in rblxstb_ischar (C++ port lost a 'static')
   1.72   remove rblxstb_block, rblxstb_block_manager, rblxstb_decompress (to rblxstb_file.h)
   1.71   rblxstb_trimwhite, rblxstb_tokens_nested, etc.
   1.70   back out 1.69 because it might problemize mixed builds; rblxstb_filec()
   1.69   (rblxstb_file returns 'char *' in C++)
   1.68   add a special 'tree root' data type for rblxstb_bst; rblxstb_arr_end
   1.67   full C++ port. (rblxstb_block_manager)
   1.66   rblxstb_newell_normal
   1.65   rblxstb_lex_item_wild -- allow wildcard items which MUST match entirely
   1.64   rblxstb_data
   1.63   rblxstb_log_name
   1.62   rblxstb_define_sort; C++ cleanup
   1.61   rblxstb_hash_fast -- Paul Hsieh's hash function (beats Bob Jenkins'?)
   1.60   rblxstb_delete_directory_recursive
   1.59   rblxstb_readdir_recursive
   1.58   rblxstb_bst variant with parent pointer for O(1) iteration, not O(log N)
   1.57   replace LCG random with Mersenne Twister (found a public domain one)
   1.56   rblxstb_perfect_hash, rblxstb_ischar, rblxstb_regex
   1.55   new rblxstb_bst API allows multiple BSTs per node (e.g. secondary keys)
   1.54   bugfix: rblxstb_define_hash, rblxstb_wildmatch, regexp
   1.53   rblxstb_define_hash; recoded rblxstb_extra, rblxstb_sdict use it
   1.52   rblxstb_rand_define, rblxstb_bst, rblxstb_reverse
   1.51   fix 'rblxstb_arr_setlen(NULL, 0)'
   1.50   rblxstb_wordwrap
   1.49   minor improvements to enable the scripting language
   1.48   better approach for rblxstb_arr using rblxstb_malloc; more invasive, clearer
   1.47   rblxstb_lex (lexes rblxstb.h at 1.5ML/s on 3Ghz P4; 60/70% of optimal/flex)
   1.46   rblxstb_wrapper_*, RBLXSTB_MALLOC_WRAPPER
   1.45   lightly tested DFA acceleration of regexp searching
   1.44   wildcard matching & searching; regexp matching & searching
   1.43   rblxstb_temp
   1.42   allow rblxstb_arr to use rblxstb_malloc/realloc; note this is global
   1.41   make it compile in C++; (disable rblxstb_arr in C++)
   1.40   rblxstb_dupe tweak; rblxstb_swap; rblxstb_substr
   1.39   rblxstb_dupe; improve rblxstb_file_max to be less stupid
   1.38   rblxstb_sha1_file: generate sha1 for file, even > 4GB
   1.37   rblxstb_file_max; partial support for utf8 filenames in Windows
   1.36   remove RBLXSTB__NO_PREFIX - poor interaction with IDE, not worth it
          streamline rblxstb_arr to make it separately publishable
   1.35   bugfixes for rblxstb_sdict, rblxstb_malloc(0), stristr
   1.34   (streaming interfaces for rblxstb_compress)
   1.33   rblxstb_alloc; bug in rblxstb_getopt; remove rblxstb_overflow
   1.32   (rblxstb_compress returns, smaller&faster; encode window & 64-bit len)
   1.31   rblxstb_prefix_count
   1.30   (RBLXSTB__NO_PREFIX - remove rblxstb_ prefixes for personal projects)
   1.29   rblxstb_fput_varlen64, etc.
   1.28   rblxstb_sha1
   1.27   ?
   1.26   rblxstb_extra
   1.25   ?
   1.24   rblxstb_copyfile
   1.23   rblxstb_readdir
   1.22   ?
   1.21   ?
   1.20   ?
   1.19   ?
   1.18   ?
   1.17   ?
   1.16   ?
   1.15   rblxstb_fixpath, rblxstb_splitpath, rblxstb_strchr2
   1.14   rblxstb_arr
   1.13   ?rblxstb, rblxstb_log, rblxstb_fatal
   1.12   ?rblxstb_hash2
   1.11   miniML
   1.10   rblxstb_crc32, rblxstb_adler32
   1.09   rblxstb_sdict
   1.08   rblxstb_bitreverse, rblxstb_ispow2, rblxstb_big32
          rblxstb_fopen, rblxstb_fput_varlen, rblxstb_fput_ranged
          rblxstb_fcmp, rblxstb_feq
   1.07   (rblxstb_encompress)
   1.06   rblxstb_compress
   1.05   rblxstb_tokens, (rblxstb_hheap)
   1.04   rblxstb_rand
   1.03   ?(s-strings)
   1.02   ?rblxstb_filelen, rblxstb_tokens
   1.01   rblxstb_tolower
   1.00   rblxstb_hash, rblxstb_intcmp
          rblxstb_file, rblxstb_stringfile, rblxstb_fgets
          rblxstb_prefix, rblxstb_strlower, rblxstb_strtok
          rblxstb_image
          (rblxstb_array), (rblxstb_arena)

Parenthesized items have since been removed.

LICENSE

 See end of file for license information.

CREDITS

 Written by Sean Barrett.

 Fixes:
  Philipp Wiesemann
  Robert Nix
  r-lyeh
  blackpawn
  github:Mojofreem
  Ryan Whitworth
  Vincent Isambart
  Mike Sartain
  Eugene Opalev
  Tim Sjostrand
  github:infatum
  Dave Butler (Croepha)
  Ethan Lee (flibitijibibo)
  Brian Collins
  Kyle Langley
*/

#include <stdarg.h>

#ifndef RBLXSTB__INCLUDE_STB_H
#define RBLXSTB__INCLUDE_STB_H

#define RBLXSTB_VERSION  1

#ifdef RBLXSTB_INTROSPECT
   #define RBLXSTB_DEFINE
#endif

#ifdef RBLXSTB_DEFINE_THREADS
   #ifndef RBLXSTB_DEFINE
   #define RBLXSTB_DEFINE
   #endif
   #ifndef RBLXSTB_THREADS
   #define RBLXSTB_THREADS
   #endif
#endif

#if defined(_WIN32) && !defined(__MINGW32__)
   #ifndef _CRT_SECURE_NO_WARNINGS
   #define _CRT_SECURE_NO_WARNINGS
   #endif
   #ifndef _CRT_NONSTDC_NO_DEPRECATE
   #define _CRT_NONSTDC_NO_DEPRECATE
   #endif
   #ifndef _CRT_NON_CONFORMING_SWPRINTFS
   #define _CRT_NON_CONFORMING_SWPRINTFS
   #endif
   #if !defined(_MSC_VER) || _MSC_VER > 1700
   #include <intrin.h> // _BitScanReverse
   #endif
#endif

#include <stdlib.h>     // stdlib could have min/max
#include <stdio.h>      // need FILE
#include <string.h>     // rblxstb_define_hash needs memcpy/memset
#include <time.h>       // rblxstb_dirtree
#ifdef __MINGW32__
   #include <fcntl.h>   // O_RDWR
#endif

#ifdef RBLXSTB_PERSONAL
   typedef int Bool;
   #define False 0
   #define True 1
#endif

#ifdef RBLXSTB_MALLOC_WRAPPER_PAGED
   #define RBLXSTB_MALLOC_WRAPPER_DEBUG
#endif
#ifdef RBLXSTB_MALLOC_WRAPPER_DEBUG
   #define RBLXSTB_MALLOC_WRAPPER
#endif
#ifdef RBLXSTB_MALLOC_WRAPPER_FASTMALLOC
   #define RBLXSTB_FASTMALLOC
   #define RBLXSTB_MALLOC_WRAPPER
#endif

#ifdef RBLXSTB_FASTMALLOC
   #ifndef _WIN32
      #undef RBLXSTB_FASTMALLOC
   #endif
#endif

#ifdef RBLXSTB_DEFINE
   #include <assert.h>
   #include <stdarg.h>
   #include <stddef.h>
   #include <ctype.h>
   #include <math.h>
   #ifndef _WIN32
   #include <unistd.h>
   #else
   #include <io.h>      // _mktemp
   #include <direct.h>  // _rmdir
   #endif
   #include <sys/types.h> // stat()/_stat()
   #include <sys/stat.h>  // stat()/_stat()
#endif

#define rblxstb_min(a,b)   ((a) < (b) ? (a) : (b))
#define rblxstb_max(a,b)   ((a) > (b) ? (a) : (b))

#ifndef RBLXSTB_ONLY
   #if !defined(__cplusplus) && !defined(min) && !defined(max)
     #define min(x,y) rblxstb_min(x,y)
     #define max(x,y) rblxstb_max(x,y)
   #endif

   #ifndef M_PI
     #define M_PI  3.14159265358979323846f
   #endif

   #ifndef TRUE
     #define TRUE  1
     #define FALSE 0
   #endif

   #ifndef deg2rad
   #define deg2rad(a)  ((a)*(M_PI/180))
   #endif
   #ifndef rad2deg
   #define rad2deg(a)  ((a)*(180/M_PI))
   #endif

   #ifndef swap
   #ifndef __cplusplus
   #define swap(TYPE,a,b)  \
               do { TYPE rblxstb__t; rblxstb__t = (a); (a) = (b); (b) = rblxstb__t; } while (0)
   #endif
   #endif

   typedef unsigned char  uint8 ;
   typedef   signed char   int8 ;
   typedef unsigned short uint16;
   typedef   signed short  int16;
  #if defined(RBLXSTB_USE_LONG_FOR_32_BIT_INT) || defined(RBLXSTB_LONG32)
   typedef unsigned long  uint32;
   typedef   signed long   int32;
  #else
   typedef unsigned int   uint32;
   typedef   signed int    int32;
  #endif

   typedef unsigned char  uchar ;
   typedef unsigned short ushort;
   typedef unsigned int   uint  ;
   typedef unsigned long  ulong ;

   // produce compile errors if the sizes aren't right
   typedef char rblxstb__testsize16[sizeof(int16)==2];
   typedef char rblxstb__testsize32[sizeof(int32)==4];
#endif

#ifndef RBLXSTB_TRUE
  #define RBLXSTB_TRUE 1
  #define RBLXSTB_FALSE 0
#endif

// if we're RBLXSTB_ONLY, can't rely on uint32 or even uint, so all the
// variables we'll use herein need typenames prefixed with 'rblxstb':
typedef unsigned char rblxstb_uchar;
typedef unsigned char rblxstb_uint8;
typedef unsigned int  rblxstb_uint;
typedef unsigned short rblxstb_uint16;
typedef          short rblxstb_int16;
typedef   signed char  rblxstb_int8;
#if defined(RBLXSTB_USE_LONG_FOR_32_BIT_INT) || defined(RBLXSTB_LONG32)
  typedef unsigned long  rblxstb_uint32;
  typedef          long  rblxstb_int32;
#else
  typedef unsigned int   rblxstb_uint32;
  typedef          int   rblxstb_int32;
#endif
typedef char rblxstb__testsize2_16[sizeof(rblxstb_uint16)==2 ? 1 : -1];
typedef char rblxstb__testsize2_32[sizeof(rblxstb_uint32)==4 ? 1 : -1];

#ifdef _MSC_VER
  typedef unsigned __int64 rblxstb_uint64;
  typedef          __int64 rblxstb_int64;
  #define RBLXSTB_IMM_UINT64(literalui64) (literalui64##ui64)
  #define RBLXSTB_IMM_INT64(literali64) (literali64##i64)
#else
  // ??
  typedef unsigned long long rblxstb_uint64;
  typedef          long long rblxstb_int64;
  #define RBLXSTB_IMM_UINT64(literalui64) (literalui64##ULL)
  #define RBLXSTB_IMM_INT64(literali64) (literali64##LL)
#endif
typedef char rblxstb__testsize2_64[sizeof(rblxstb_uint64)==8 ? 1 : -1];

// add platform-specific ways of checking for sizeof(char*) == 8,
// and make those define RBLXSTB_PTR64
#if defined(_WIN64) || defined(__x86_64__) || defined(__ia64__) || defined(__LP64__)
  #define RBLXSTB_PTR64
#endif

#ifdef RBLXSTB_PTR64
typedef char rblxstb__testsize2_ptr[sizeof(char *) == 8];
typedef rblxstb_uint64 rblxstb_uinta;
typedef rblxstb_int64  rblxstb_inta;
#else
typedef char rblxstb__testsize2_ptr[sizeof(char *) == 4];
typedef rblxstb_uint32 rblxstb_uinta;
typedef rblxstb_int32  rblxstb_inta;
#endif
typedef char rblxstb__testsize2_uinta[sizeof(rblxstb_uinta)==sizeof(char*) ? 1 : -1];

// if so, we should define an int type that is the pointer size. until then,
// we'll have to make do with this (which is not the same at all!)

typedef union
{
   unsigned int i;
   void * p;
} rblxstb_uintptr;


#ifdef __cplusplus
   #define RBLXSTB_EXTERN   extern "C"
#else
   #define RBLXSTB_EXTERN   extern
#endif

// check for well-known debug defines
#if defined(DEBUG) || defined(_DEBUG) || defined(DBG)
   #ifndef NDEBUG
      #define RBLXSTB_DEBUG
   #endif
#endif

#ifdef RBLXSTB_DEBUG
   #include <assert.h>
#endif

//////////////////////////////////////////////////////////////////////////////
//
//                         C library function platform handling
//

#ifdef RBLXSTB_DEFINE

#if defined(_WIN32) &&  defined(__STDC_WANT_SECURE_LIB__)
static FILE * rblxstb_p_fopen(const char *filename, const char *mode)
{
   FILE *f;
   if (0 == fopen_s(&f, filename, mode))
      return f;
   else
      return NULL;
}
static FILE * rblxstb_p_wfopen(const wchar_t *filename, const wchar_t *mode)
{
   FILE *f;
   if (0 == _wfopen_s(&f, filename, mode))
      return f;
   else
      return NULL;
}
static char *rblxstb_p_strcpy_s(char *a, size_t size, const char *b)
{
   strcpy_s(a,size,b);
   return a;
}
static char *rblxstb_p_strncpy_s(char *a, size_t size, const char *b, size_t count)
{
   strncpy_s(a,size,b,count);
   return a;
}
#define rblxstb_p_mktemp(s)  (_mktemp_s(s, strlen(s)+1) == 0)
#define rblxstb_p_sprintf    sprintf_s
#define rblxstb_p_size(x)    ,(x)
#else
#define rblxstb_p_fopen      fopen
#define rblxstb_p_wfopen     _wfopen
#define rblxstb_p_strcpy_s(a,s,b)     strcpy(a,b)
#define rblxstb_p_strncpy_s(a,s,b,c)  strncpy(a,b,c)
#define rblxstb_p_mktemp(s)  (mktemp(s) != NULL)

#define rblxstb_p_sprintf    sprintf
#define rblxstb_p_size(x)
#endif

#if defined(_WIN32)
#define rblxstb_p_vsnprintf  _vsnprintf
#else
#define rblxstb_p_vsnprintf  vsnprintf
#endif
#endif // RBLXSTB_DEFINE

#if defined(_WIN32) && (_MSC_VER >= 1300)
#define rblxstb_p_stricmp    _stricmp
#define rblxstb_p_strnicmp   _strnicmp
#define rblxstb_p_strdup     _strdup
#else
#define rblxstb_p_strdup     strdup
#define rblxstb_p_stricmp    stricmp
#define rblxstb_p_strnicmp   strnicmp
#endif

RBLXSTB_EXTERN void rblxstb_wrapper_malloc(void *newp, size_t sz, char *file, int line);
RBLXSTB_EXTERN void rblxstb_wrapper_free(void *oldp, char *file, int line);
RBLXSTB_EXTERN void rblxstb_wrapper_realloc(void *oldp, void *newp, size_t sz, char *file, int line);
RBLXSTB_EXTERN void rblxstb_wrapper_calloc(size_t num, size_t sz, char *file, int line);
RBLXSTB_EXTERN void rblxstb_wrapper_listall(void (*func)(void *ptr, size_t sz, char *file, int line));
RBLXSTB_EXTERN void rblxstb_wrapper_dump(char *filename);
RBLXSTB_EXTERN size_t rblxstb_wrapper_allocsize(void *oldp);
RBLXSTB_EXTERN void rblxstb_wrapper_check(void *oldp);

#ifdef RBLXSTB_DEFINE
// this is a special function used inside malloc wrapper
// to do allocations that aren't tracked (to avoid
// reentrancy). Of course if someone _else_ wraps realloc,
// this breaks, but if they're doing that AND the malloc
// wrapper they need to explicitly check for reentrancy.
//
// only define realloc_raw() and we do realloc(NULL,sz)
// for malloc() and realloc(p,0) for free().
static void * rblxstb__realloc_raw(void *p, int sz)
{
   if (p == NULL) return malloc(sz);
   if (sz == 0)   { free(p); return NULL; }
   return realloc(p,sz);
}
#endif

#ifdef _WIN32
RBLXSTB_EXTERN void * rblxstb_smalloc(size_t sz);
RBLXSTB_EXTERN void   rblxstb_sfree(void *p);
RBLXSTB_EXTERN void * rblxstb_srealloc(void *p, size_t sz);
RBLXSTB_EXTERN void * rblxstb_scalloc(size_t n, size_t sz);
RBLXSTB_EXTERN char * rblxstb_sstrdup(char *s);
#endif

#ifdef RBLXSTB_FASTMALLOC
#define malloc  rblxstb_smalloc
#define free    rblxstb_sfree
#define realloc rblxstb_srealloc
#define strdup  rblxstb_sstrdup
#define calloc  rblxstb_scalloc
#endif

#ifndef RBLXSTB_MALLOC_ALLCHECK
   #define rblxstb__check(p)  1
#else
   #ifndef RBLXSTB_MALLOC_WRAPPER
      #error RBLXSTB_MALLOC_ALLCHECK requires RBLXSTB_MALLOC_WRAPPER
   #else
      #define rblxstb__check(p) rblxstb_mcheck(p)
   #endif
#endif

#ifdef RBLXSTB_MALLOC_WRAPPER
   RBLXSTB_EXTERN void * rblxstb__malloc(size_t, char *, int);
   RBLXSTB_EXTERN void * rblxstb__realloc(void *, size_t, char *, int);
   RBLXSTB_EXTERN void * rblxstb__calloc(size_t n, size_t s, char *, int);
   RBLXSTB_EXTERN void   rblxstb__free(void *, char *file, int);
   RBLXSTB_EXTERN char * rblxstb__strdup(char *s, char *file, int);
   RBLXSTB_EXTERN void   rblxstb_malloc_checkall(void);
   RBLXSTB_EXTERN void   rblxstb_malloc_check_counter(int init_delay, int rep_delay);
   #ifndef RBLXSTB_MALLOC_WRAPPER_DEBUG
      #define rblxstb_mcheck(p) 1
   #else
      RBLXSTB_EXTERN int   rblxstb_mcheck(void *);
   #endif


   #ifdef RBLXSTB_DEFINE

   #ifdef RBLXSTB_MALLOC_WRAPPER_DEBUG
      #define RBLXSTB__PAD   32
      #define RBLXSTB__BIAS  16
      #define RBLXSTB__SIG   0x51b01234
      #define RBLXSTB__FIXSIZE(sz)  (((sz+3) & ~3) + RBLXSTB__PAD)
      #define RBLXSTB__ptr(x,y)   ((char *) (x) + (y))
   #else
      #define RBLXSTB__ptr(x,y)   (x)
      #define RBLXSTB__FIXSIZE(sz)  (sz)
   #endif

   #ifdef RBLXSTB_MALLOC_WRAPPER_DEBUG
   int rblxstb_mcheck(void *p)
   {
      unsigned int sz;
      if (p == NULL) return 1;
      p = ((char *) p) - RBLXSTB__BIAS;
      sz = * (unsigned int *) p;
      assert(* (unsigned int *) RBLXSTB__ptr(p,4) == RBLXSTB__SIG);
      assert(* (unsigned int *) RBLXSTB__ptr(p,8) == RBLXSTB__SIG);
      assert(* (unsigned int *) RBLXSTB__ptr(p,12) == RBLXSTB__SIG);
      assert(* (unsigned int *) RBLXSTB__ptr(p,sz-4) == RBLXSTB__SIG+1);
      assert(* (unsigned int *) RBLXSTB__ptr(p,sz-8) == RBLXSTB__SIG+1);
      assert(* (unsigned int *) RBLXSTB__ptr(p,sz-12) == RBLXSTB__SIG+1);
      assert(* (unsigned int *) RBLXSTB__ptr(p,sz-16) == RBLXSTB__SIG+1);
      rblxstb_wrapper_check(RBLXSTB__ptr(p, RBLXSTB__BIAS));
      return 1;
   }

   static void rblxstb__check2(void *p, size_t sz, char *file, int line)
   {
      rblxstb_mcheck(p);
   }

   void rblxstb_malloc_checkall(void)
   {
      rblxstb_wrapper_listall(rblxstb__check2);
   }
   #else
   void rblxstb_malloc_checkall(void) { }
   #endif

   static int rblxstb__malloc_wait=(1 << 30), rblxstb__malloc_next_wait = (1 << 30), rblxstb__malloc_iter;
   void rblxstb_malloc_check_counter(int init_delay, int rep_delay)
   {
      rblxstb__malloc_wait = init_delay;
      rblxstb__malloc_next_wait = rep_delay;
   }

   void rblxstb_mcheck_all(void)
   {
      #ifdef RBLXSTB_MALLOC_WRAPPER_DEBUG
      ++rblxstb__malloc_iter;
      if (--rblxstb__malloc_wait <= 0) {
         rblxstb_malloc_checkall();
         rblxstb__malloc_wait = rblxstb__malloc_next_wait;
      }
      #endif
   }

   #ifdef RBLXSTB_MALLOC_WRAPPER_PAGED
   #define RBLXSTB__WINDOWS_PAGE (1 << 12)
   #ifndef _WINDOWS_
   RBLXSTB_EXTERN __declspec(dllimport) void * __stdcall VirtualAlloc(void *p, unsigned long size, unsigned long type, unsigned long protect);
   RBLXSTB_EXTERN __declspec(dllimport) int   __stdcall VirtualFree(void *p, unsigned long size, unsigned long freetype);
   #endif
   #endif

   static void *rblxstb__malloc_final(size_t sz)
   {
      #ifdef RBLXSTB_MALLOC_WRAPPER_PAGED
      size_t aligned = (sz + RBLXSTB__WINDOWS_PAGE - 1) & ~(RBLXSTB__WINDOWS_PAGE-1);
      char *p = VirtualAlloc(NULL, aligned + RBLXSTB__WINDOWS_PAGE, 0x2000, 0x04); // RESERVE, READWRITE
      if (p == NULL) return p;
      VirtualAlloc(p, aligned,   0x1000, 0x04); // COMMIT, READWRITE
      return p;
      #else
      return malloc(sz);
      #endif
   }

   static void rblxstb__free_final(void *p)
   {
      #ifdef RBLXSTB_MALLOC_WRAPPER_PAGED
      VirtualFree(p, 0, 0x8000); // RELEASE
      #else
      free(p);
      #endif
   }

   int rblxstb__malloc_failure;
   #ifdef RBLXSTB_MALLOC_WRAPPER_PAGED
   static void *rblxstb__realloc_final(void *p, size_t sz, size_t old_sz)
   {
      void *q = rblxstb__malloc_final(sz);
      if (q == NULL)
          return ++rblxstb__malloc_failure, q;
      // @TODO: deal with p being smaller!
      memcpy(q, p, sz < old_sz ? sz : old_sz);
      rblxstb__free_final(p);
      return q;
   }
   #endif

   void rblxstb__free(void *p, char *file, int line)
   {
      rblxstb_mcheck_all();
      if (!p) return;
      #ifdef RBLXSTB_MALLOC_WRAPPER_DEBUG
      rblxstb_mcheck(p);
      #endif
      rblxstb_wrapper_free(p,file,line);
      #ifdef RBLXSTB_MALLOC_WRAPPER_DEBUG
         p = RBLXSTB__ptr(p,-RBLXSTB__BIAS);
         * (unsigned int *) RBLXSTB__ptr(p,0) = 0xdeadbeef;
         * (unsigned int *) RBLXSTB__ptr(p,4) = 0xdeadbeef;
         * (unsigned int *) RBLXSTB__ptr(p,8) = 0xdeadbeef;
         * (unsigned int *) RBLXSTB__ptr(p,12) = 0xdeadbeef;
      #endif
      rblxstb__free_final(p);
   }

   void * rblxstb__malloc(size_t sz, char *file, int line)
   {
      void *p;
      rblxstb_mcheck_all();
      if (sz == 0) return NULL;
      p = rblxstb__malloc_final(RBLXSTB__FIXSIZE(sz));
      if (p == NULL) p = rblxstb__malloc_final(RBLXSTB__FIXSIZE(sz));
      if (p == NULL) p = rblxstb__malloc_final(RBLXSTB__FIXSIZE(sz));
      if (p == NULL) {
         ++rblxstb__malloc_failure;
         #ifdef RBLXSTB_MALLOC_WRAPPER_DEBUG
         rblxstb_malloc_checkall();
         #endif
         return p;
      }
      #ifdef RBLXSTB_MALLOC_WRAPPER_DEBUG
      * (int *) RBLXSTB__ptr(p,0) = RBLXSTB__FIXSIZE(sz);
      * (unsigned int *) RBLXSTB__ptr(p,4) = RBLXSTB__SIG;
      * (unsigned int *) RBLXSTB__ptr(p,8) = RBLXSTB__SIG;
      * (unsigned int *) RBLXSTB__ptr(p,12) = RBLXSTB__SIG;
      * (unsigned int *) RBLXSTB__ptr(p,RBLXSTB__FIXSIZE(sz)-4) = RBLXSTB__SIG+1;
      * (unsigned int *) RBLXSTB__ptr(p,RBLXSTB__FIXSIZE(sz)-8) = RBLXSTB__SIG+1;
      * (unsigned int *) RBLXSTB__ptr(p,RBLXSTB__FIXSIZE(sz)-12) = RBLXSTB__SIG+1;
      * (unsigned int *) RBLXSTB__ptr(p,RBLXSTB__FIXSIZE(sz)-16) = RBLXSTB__SIG+1;
      p = RBLXSTB__ptr(p, RBLXSTB__BIAS);
      #endif
      rblxstb_wrapper_malloc(p,sz,file,line);
      return p;
   }

   void * rblxstb__realloc(void *p, size_t sz, char *file, int line)
   {
      void *q;

      rblxstb_mcheck_all();
      if (p == NULL) return rblxstb__malloc(sz,file,line);
      if (sz == 0  ) { rblxstb__free(p,file,line); return NULL; }

      #ifdef RBLXSTB_MALLOC_WRAPPER_DEBUG
         rblxstb_mcheck(p);
         p = RBLXSTB__ptr(p,-RBLXSTB__BIAS);
      #endif
      #ifdef RBLXSTB_MALLOC_WRAPPER_PAGED
      {
         size_t n = rblxstb_wrapper_allocsize(RBLXSTB__ptr(p,RBLXSTB__BIAS));
         if (!n)
            rblxstb_wrapper_check(RBLXSTB__ptr(p,RBLXSTB__BIAS));
         q = rblxstb__realloc_final(p, RBLXSTB__FIXSIZE(sz), RBLXSTB__FIXSIZE(n));
      }
      #else
      q = realloc(p, RBLXSTB__FIXSIZE(sz));
      #endif
      if (q == NULL)
         return ++rblxstb__malloc_failure, q;
      #ifdef RBLXSTB_MALLOC_WRAPPER_DEBUG
      * (int *) RBLXSTB__ptr(q,0) = RBLXSTB__FIXSIZE(sz);
      * (unsigned int *) RBLXSTB__ptr(q,4) = RBLXSTB__SIG;
      * (unsigned int *) RBLXSTB__ptr(q,8) = RBLXSTB__SIG;
      * (unsigned int *) RBLXSTB__ptr(q,12) = RBLXSTB__SIG;
      * (unsigned int *) RBLXSTB__ptr(q,RBLXSTB__FIXSIZE(sz)-4) = RBLXSTB__SIG+1;
      * (unsigned int *) RBLXSTB__ptr(q,RBLXSTB__FIXSIZE(sz)-8) = RBLXSTB__SIG+1;
      * (unsigned int *) RBLXSTB__ptr(q,RBLXSTB__FIXSIZE(sz)-12) = RBLXSTB__SIG+1;
      * (unsigned int *) RBLXSTB__ptr(q,RBLXSTB__FIXSIZE(sz)-16) = RBLXSTB__SIG+1;

      q = RBLXSTB__ptr(q, RBLXSTB__BIAS);
      p = RBLXSTB__ptr(p, RBLXSTB__BIAS);
      #endif
      rblxstb_wrapper_realloc(p,q,sz,file,line);
      return q;
   }

   RBLXSTB_EXTERN int rblxstb_log2_ceil(size_t);
   static void *rblxstb__calloc(size_t n, size_t sz, char *file, int line)
   {
      void *q;
      rblxstb_mcheck_all();
      if (n == 0 || sz == 0) return NULL;
      if (rblxstb_log2_ceil(n) + rblxstb_log2_ceil(sz) >= 32) return NULL;
      q = rblxstb__malloc(n*sz, file, line);
      if (q) memset(q, 0, n*sz);
      return q;
   }

   char * rblxstb__strdup(char *s, char *file, int line)
   {
      char *p;
      rblxstb_mcheck_all();
      p = rblxstb__malloc(strlen(s)+1, file, line);
      if (!p) return p;
      rblxstb_p_strcpy_s(p, strlen(s)+1, s);
      return p;
   }
   #endif // RBLXSTB_DEFINE

   #ifdef RBLXSTB_FASTMALLOC
   #undef malloc
   #undef realloc
   #undef free
   #undef strdup
   #undef calloc
   #endif

   // include everything that might define these, BEFORE making macros
   #include <stdlib.h>
   #include <string.h>
   #include <malloc.h>

   #define malloc(s)      rblxstb__malloc (  s, __FILE__, __LINE__)
   #define realloc(p,s)   rblxstb__realloc(p,s, __FILE__, __LINE__)
   #define calloc(n,s)    rblxstb__calloc (n,s, __FILE__, __LINE__)
   #define free(p)        rblxstb__free   (p,   __FILE__, __LINE__)
   #define strdup(p)      rblxstb__strdup (p,   __FILE__, __LINE__)
#endif

//////////////////////////////////////////////////////////////////////////////
//
//                         Windows pretty display
//

RBLXSTB_EXTERN void rblxstbprint(const char *fmt, ...);
RBLXSTB_EXTERN char *rblxstb_sprintf(const char *fmt, ...);
RBLXSTB_EXTERN char *rblxstb_mprintf(const char *fmt, ...);
RBLXSTB_EXTERN int  rblxstb_snprintf(char *s, size_t n, const char *fmt, ...);
RBLXSTB_EXTERN int  rblxstb_vsnprintf(char *s, size_t n, const char *fmt, va_list v);

#ifdef RBLXSTB_DEFINE
int rblxstb_vsnprintf(char *s, size_t n, const char *fmt, va_list v)
{
   int res;
   #ifdef _WIN32
      #ifdef __STDC_WANT_SECURE_LIB__
      res = _vsnprintf_s(s, n, _TRUNCATE, fmt, v);
      #else
      res = rblxstb_p_vsnprintf(s,n,fmt,v);
      #endif
   #else
   res = vsnprintf(s,n,fmt,v);
   #endif
   if (n) s[n-1] = 0;
   // Unix returns length output would require, Windows returns negative when truncated.
   return (res >= (int) n || res < 0) ? -1 : res;
}

int rblxstb_snprintf(char *s, size_t n, const char *fmt, ...)
{
   int res;
   va_list v;
   va_start(v,fmt);
   res = rblxstb_vsnprintf(s, n, fmt, v);
   va_end(v);
   return res;
}

char *rblxstb_sprintf(const char *fmt, ...)
{
   static char buffer[1024];
   va_list v;
   va_start(v,fmt);
   rblxstb_vsnprintf(buffer,1024,fmt,v);
   va_end(v);
   return buffer;
}

char *rblxstb_mprintf(const char *fmt, ...)
{
   static char buffer[1024];
   va_list v;
   va_start(v,fmt);
   rblxstb_vsnprintf(buffer,1024,fmt,v);
   va_end(v);
   return rblxstb_p_strdup(buffer);
}

#ifdef _WIN32

#ifndef _WINDOWS_
RBLXSTB_EXTERN __declspec(dllimport) int __stdcall WriteConsoleA(void *, const void *, unsigned int, unsigned int *, void *);
RBLXSTB_EXTERN __declspec(dllimport) void * __stdcall GetStdHandle(unsigned int);
RBLXSTB_EXTERN __declspec(dllimport) int __stdcall SetConsoleTextAttribute(void *, unsigned short);
#endif

static void rblxstb__print_one(void *handle, char *s, ptrdiff_t  len)
{
   if (len)
      if (0==WriteConsoleA(handle, s, (unsigned) len, NULL,NULL))
         // if it fails, maybe redirected, so output normally...
         // but it's supriously reporting failure now on Win7 and later
         {}//fwrite(s, 1, (unsigned) len, stdout);
}

static void rblxstb__print(char *s)
{
   void *handle = GetStdHandle((unsigned int) -11); // STD_OUTPUT_HANDLE
   int pad=0; // number of padding characters to add

   char *t = s;
   while (*s) {
      int lpad;
      while (*s && *s != '{') {
         if (pad) {
            if (*s == '\r' || *s == '\n')
               pad = 0;
            else if (s[0] == ' ' && s[1] == ' ') {
               rblxstb__print_one(handle, t, s-t);
               t = s;
               while (pad) {
                  rblxstb__print_one(handle, t, 1);
                  --pad;
               }
            }
         }
         ++s;
      }
      if (!*s) break;
      rblxstb__print_one(handle, t, s-t);
      if (s[1] == '{') {
         ++s;
         continue;
      }

      if (s[1] == '#') {
         t = s+3;
         if (isxdigit(s[2]))
            if (isdigit(s[2]))
               SetConsoleTextAttribute(handle, s[2] - '0');
            else
               SetConsoleTextAttribute(handle, tolower(s[2]) - 'a' + 10);
         else {
            SetConsoleTextAttribute(handle, 0x0f);
            t=s+2;
         }
      } else if (s[1] == '!') {
         SetConsoleTextAttribute(handle, 0x0c);
         t = s+2;
      } else if (s[1] == '@') {
         SetConsoleTextAttribute(handle, 0x09);
         t = s+2;
      } else if (s[1] == '$') {
         SetConsoleTextAttribute(handle, 0x0a);
         t = s+2;
      } else {
         SetConsoleTextAttribute(handle, 0x08); // 0,7,8,15 => shades of grey
         t = s+1;
      }

      lpad = (int) (t-s);
      s = t;
      while (*s && *s != '}') ++s;
      if (!*s) break;
      rblxstb__print_one(handle, t, s-t);
      if (s[1] == '}') {
         t = s+2;
      } else {
         pad += 1+lpad;
         t = s+1;
      }
      s=t;
      SetConsoleTextAttribute(handle, 0x07);
   }
   rblxstb__print_one(handle, t, s-t);
   SetConsoleTextAttribute(handle, 0x07);
}

void rblxstbprint(const char *fmt, ...)
{
   int res;
   char buffer[1024];
   char *tbuf = buffer;
   va_list v;

   va_start(v,fmt);
   res = rblxstb_vsnprintf(buffer, sizeof(buffer), fmt, v);
   va_end(v);

   if (res < 0) {
      tbuf = (char *) malloc(16384);
      va_start(v,fmt);
      res = rblxstb_vsnprintf(tbuf,16384, fmt, v);
      va_end(v);
      tbuf[16383] = 0;
   }

   rblxstb__print(tbuf);

   if (tbuf != buffer)
      free(tbuf);
}

#else  // _WIN32
void rblxstbprint(const char *fmt, ...)
{
   va_list v;
   va_start(v,fmt);
   vprintf(fmt,v);
   va_end(v);
}
#endif // _WIN32
#endif // RBLXSTB_DEFINE



//////////////////////////////////////////////////////////////////////////////
//
//                         Windows UTF8 filename handling
//
// Windows stupidly treats 8-bit filenames as some dopey code page,
// rather than utf-8. If we want to use utf8 filenames, we have to
// convert them to WCHAR explicitly and call WCHAR versions of the
// file functions. So, ok, we do.


#ifdef _WIN32
   #define rblxstb__fopen(x,y)    rblxstb_p_wfopen((const wchar_t *)rblxstb__from_utf8(x), (const wchar_t *)rblxstb__from_utf8_alt(y))
   #define rblxstb__windows(x,y)  x
#else
   #define rblxstb__fopen(x,y)    rblxstb_p_fopen(x,y)
   #define rblxstb__windows(x,y)  y
#endif


typedef unsigned short rblxstb__wchar;

RBLXSTB_EXTERN rblxstb__wchar * rblxstb_from_utf8(rblxstb__wchar *buffer, const char *str, int n);
RBLXSTB_EXTERN char       * rblxstb_to_utf8  (char *buffer, const rblxstb__wchar *str, int n);

RBLXSTB_EXTERN rblxstb__wchar *rblxstb__from_utf8(const char *str);
RBLXSTB_EXTERN rblxstb__wchar *rblxstb__from_utf8_alt(const char *str);
RBLXSTB_EXTERN char *rblxstb__to_utf8(const rblxstb__wchar *str);


#ifdef RBLXSTB_DEFINE
rblxstb__wchar * rblxstb_from_utf8(rblxstb__wchar *buffer, const char *ostr, int n)
{
   unsigned char *str = (unsigned char *) ostr;
   rblxstb_uint32 c;
   int i=0;
   --n;
   while (*str) {
      if (i >= n)
         return NULL;
      if (!(*str & 0x80))
         buffer[i++] = *str++;
      else if ((*str & 0xe0) == 0xc0) {
         if (*str < 0xc2) return NULL;
         c = (*str++ & 0x1f) << 6;
         if ((*str & 0xc0) != 0x80) return NULL;
         buffer[i++] = c + (*str++ & 0x3f);
      } else if ((*str & 0xf0) == 0xe0) {
         if (*str == 0xe0 && (str[1] < 0xa0 || str[1] > 0xbf)) return NULL;
         if (*str == 0xed && str[1] > 0x9f) return NULL; // str[1] < 0x80 is checked below
         c = (*str++ & 0x0f) << 12;
         if ((*str & 0xc0) != 0x80) return NULL;
         c += (*str++ & 0x3f) << 6;
         if ((*str & 0xc0) != 0x80) return NULL;
         buffer[i++] = c + (*str++ & 0x3f);
      } else if ((*str & 0xf8) == 0xf0) {
         if (*str > 0xf4) return NULL;
         if (*str == 0xf0 && (str[1] < 0x90 || str[1] > 0xbf)) return NULL;
         if (*str == 0xf4 && str[1] > 0x8f) return NULL; // str[1] < 0x80 is checked below
         c = (*str++ & 0x07) << 18;
         if ((*str & 0xc0) != 0x80) return NULL;
         c += (*str++ & 0x3f) << 12;
         if ((*str & 0xc0) != 0x80) return NULL;
         c += (*str++ & 0x3f) << 6;
         if ((*str & 0xc0) != 0x80) return NULL;
         c += (*str++ & 0x3f);
         // utf-8 encodings of values used in surrogate pairs are invalid
         if ((c & 0xFFFFF800) == 0xD800) return NULL;
         if (c >= 0x10000) {
            c -= 0x10000;
            if (i + 2 > n) return NULL;
            buffer[i++] = 0xD800 | (0x3ff & (c >> 10));
            buffer[i++] = 0xDC00 | (0x3ff & (c      ));
         }
      } else
         return NULL;
   }
   buffer[i] = 0;
   return buffer;
}

char * rblxstb_to_utf8(char *buffer, const rblxstb__wchar *str, int n)
{
   int i=0;
   --n;
   while (*str) {
      if (*str < 0x80) {
         if (i+1 > n) return NULL;
         buffer[i++] = (char) *str++;
      } else if (*str < 0x800) {
         if (i+2 > n) return NULL;
         buffer[i++] = 0xc0 + (*str >> 6);
         buffer[i++] = 0x80 + (*str & 0x3f);
         str += 1;
      } else if (*str >= 0xd800 && *str < 0xdc00) {
         rblxstb_uint32 c;
         if (i+4 > n) return NULL;
         c = ((str[0] - 0xd800) << 10) + ((str[1]) - 0xdc00) + 0x10000;
         buffer[i++] = 0xf0 + (c >> 18);
         buffer[i++] = 0x80 + ((c >> 12) & 0x3f);
         buffer[i++] = 0x80 + ((c >>  6) & 0x3f);
         buffer[i++] = 0x80 + ((c      ) & 0x3f);
         str += 2;
      } else if (*str >= 0xdc00 && *str < 0xe000) {
         return NULL;
      } else {
         if (i+3 > n) return NULL;
         buffer[i++] = 0xe0 + (*str >> 12);
         buffer[i++] = 0x80 + ((*str >> 6) & 0x3f);
         buffer[i++] = 0x80 + ((*str     ) & 0x3f);
         str += 1;
      }
   }
   buffer[i] = 0;
   return buffer;
}

rblxstb__wchar *rblxstb__from_utf8(const char *str)
{
   static rblxstb__wchar buffer[4096];
   return rblxstb_from_utf8(buffer, str, 4096);
}

rblxstb__wchar *rblxstb__from_utf8_alt(const char *str)
{
   static rblxstb__wchar buffer[4096];
   return rblxstb_from_utf8(buffer, str, 4096);
}

char *rblxstb__to_utf8(const rblxstb__wchar *str)
{
   static char buffer[4096];
   return rblxstb_to_utf8(buffer, str, 4096);
}

#endif

//////////////////////////////////////////////////////////////////////////////
//
//                         Miscellany
//

RBLXSTB_EXTERN void rblxstb_fatal(const char *fmt, ...);
RBLXSTB_EXTERN void rblxstb_(char *fmt, ...);
RBLXSTB_EXTERN void rblxstb_append_to_file(char *file, char *fmt, ...);
RBLXSTB_EXTERN void rblxstb_log(int active);
RBLXSTB_EXTERN void rblxstb_log_fileline(int active);
RBLXSTB_EXTERN void rblxstb_log_name(char *filename);

RBLXSTB_EXTERN void rblxstb_swap(void *p, void *q, size_t sz);
RBLXSTB_EXTERN void *rblxstb_copy(void *p, size_t sz);
RBLXSTB_EXTERN void rblxstb_pointer_array_free(void *p, int len);
RBLXSTB_EXTERN void **rblxstb_array_block_alloc(int count, int blocksize);

#define rblxstb_arrcount(x)   (sizeof(x)/sizeof((x)[0]))


RBLXSTB_EXTERN int  rblxstb__record_fileline(const char *f, int n);

#ifdef RBLXSTB_DEFINE

static char *rblxstb__file;
static int   rblxstb__line;

int  rblxstb__record_fileline(const char *f, int n)
{
   rblxstb__file = (char*) f;
   rblxstb__line = n;
   return 0;
}

void rblxstb_fatal(const char *s, ...)
{
   va_list a;
   if (rblxstb__file)
      fprintf(stderr, "[%s:%d] ", rblxstb__file, rblxstb__line);
   va_start(a,s);
   fputs("Fatal error: ", stderr);
   vfprintf(stderr, s, a);
   va_end(a);
   fputs("\n", stderr);
   #ifdef RBLXSTB_DEBUG
   #ifdef _MSC_VER
   #ifndef RBLXSTB_PTR64
   __asm int 3;   // trap to debugger!
   #else
   __debugbreak();
   #endif
   #else
   __builtin_trap();
   #endif
   #endif
   exit(1);
}

static int rblxstb__log_active=1, rblxstb__log_fileline=1;

void rblxstb_log(int active)
{
   rblxstb__log_active = active;
}

void rblxstb_log_fileline(int active)
{
   rblxstb__log_fileline = active;
}

#ifdef RBLXSTB_NO_STB_STRINGS
const char *rblxstb__log_filename = "temp.log";
#else
const char *rblxstb__log_filename = "rblxstb.log";
#endif

void rblxstb_log_name(char *s)
{
   rblxstb__log_filename = s;
}

void rblxstb_(char *s, ...)
{
   if (rblxstb__log_active) {
      FILE *f = rblxstb_p_fopen(rblxstb__log_filename, "a");
      if (f) {
         va_list a;
         if (rblxstb__log_fileline && rblxstb__file)
            fprintf(f, "[%s:%4d] ", rblxstb__file, rblxstb__line);
         va_start(a,s);
         vfprintf(f, s, a);
         va_end(a);
         fputs("\n", f);
         fclose(f);
      }
   }
}

void rblxstb_append_to_file(char *filename, char *s, ...)
{
   FILE *f = rblxstb_p_fopen(filename, "a");
   if (f) {
      va_list a;
      va_start(a,s);
      vfprintf(f, s, a);
      va_end(a);
      fputs("\n", f);
      fclose(f);
   }
}


typedef struct { char d[4]; } rblxstb__4;
typedef struct { char d[8]; } rblxstb__8;

// optimize the small cases, though you shouldn't be calling this for those!
void rblxstb_swap(void *p, void *q, size_t sz)
{
   char buffer[256];
   if (p == q) return;
   if (sz == 4) {
      rblxstb__4 temp    = * ( rblxstb__4 *) p;
      * (rblxstb__4 *) p = * ( rblxstb__4 *) q;
      * (rblxstb__4 *) q = temp;
      return;
   } else if (sz == 8) {
      rblxstb__8 temp    = * ( rblxstb__8 *) p;
      * (rblxstb__8 *) p = * ( rblxstb__8 *) q;
      * (rblxstb__8 *) q = temp;
      return;
   }

   while (sz > sizeof(buffer)) {
      rblxstb_swap(p, q, sizeof(buffer));
      p = (char *) p + sizeof(buffer);
      q = (char *) q + sizeof(buffer);
      sz -= sizeof(buffer);
   }

   memcpy(buffer, p     , sz);
   memcpy(p     , q     , sz);
   memcpy(q     , buffer, sz);
}

void *rblxstb_copy(void *p, size_t sz)
{
   void *q = malloc(sz);
   memcpy(q, p, sz);
   return q;
}

void rblxstb_pointer_array_free(void *q, int len)
{
   void **p = (void **) q;
   int i;
   for (i=0; i < len; ++i)
      free(p[i]);
}

void **rblxstb_array_block_alloc(int count, int blocksize)
{
   int i;
   char *p = (char *) malloc(sizeof(void *) * count + count * blocksize);
   void **q;
   if (p == NULL) return NULL;
   q = (void **) p;
   p += sizeof(void *) * count;
   for (i=0; i < count; ++i)
      q[i] = p + i * blocksize;
   return q;
}
#endif

#ifdef RBLXSTB_DEBUG
   // tricky hack to allow recording FILE,LINE even in varargs functions
   #define RBLXSTB__RECORD_FILE(x)  (rblxstb__record_fileline(__FILE__, __LINE__),(x))
   #define rblxstb_log              RBLXSTB__RECORD_FILE(rblxstb_log)
   #define rblxstb_                 RBLXSTB__RECORD_FILE(rblxstb_)
   #ifndef RBLXSTB_FATAL_CLEAN
   #define rblxstb_fatal            RBLXSTB__RECORD_FILE(rblxstb_fatal)
   #endif
   #define RBLXSTB__DEBUG(x)        x
#else
   #define RBLXSTB__DEBUG(x)
#endif

//////////////////////////////////////////////////////////////////////////////
//
//                         rblxstb_temp
//

#define rblxstb_temp(block, sz)     rblxstb__temp(block, sizeof(block), (sz))

RBLXSTB_EXTERN void * rblxstb__temp(void *b, int b_sz, int want_sz);
RBLXSTB_EXTERN void   rblxstb_tempfree(void *block, void *ptr);

#ifdef RBLXSTB_DEFINE

void * rblxstb__temp(void *b, int b_sz, int want_sz)
{
   if (b_sz >= want_sz)
      return b;
   else
      return malloc(want_sz);
}

void   rblxstb_tempfree(void *b, void *p)
{
   if (p != b)
      free(p);
}
#endif


//////////////////////////////////////////////////////////////////////////////
//
//                      math/sampling operations
//


#define rblxstb_lerp(t,a,b)               ( (a) + (t) * (float) ((b)-(a)) )
#define rblxstb_unlerp(t,a,b)             ( ((t) - (a)) / (float) ((b) - (a)) )

#define rblxstb_clamp(x,xmin,xmax)  ((x) < (xmin) ? (xmin) : (x) > (xmax) ? (xmax) : (x))

RBLXSTB_EXTERN void rblxstb_newell_normal(float *normal, int num_vert, float **vert, int normalize);
RBLXSTB_EXTERN int rblxstb_box_face_vertex_axis_side(int face_number, int vertex_number, int axis);
RBLXSTB_EXTERN void rblxstb_linear_controller(float *curpos, float target_pos, float acc, float deacc, float dt);

RBLXSTB_EXTERN int rblxstb_float_eq(float x, float y, float delta, int max_ulps);
RBLXSTB_EXTERN int rblxstb_is_prime(unsigned int m);
RBLXSTB_EXTERN unsigned int rblxstb_power_of_two_nearest_prime(int n);

RBLXSTB_EXTERN float rblxstb_smoothstep(float t);
RBLXSTB_EXTERN float rblxstb_cubic_bezier_1d(float t, float p0, float p1, float p2, float p3);

RBLXSTB_EXTERN double rblxstb_linear_remap(double x, double a, double b,
                                             double c, double d);

#ifdef RBLXSTB_DEFINE
float rblxstb_smoothstep(float t)
{
   return (3 - 2*t)*(t*t);
}

float rblxstb_cubic_bezier_1d(float t, float p0, float p1, float p2, float p3)
{
   float it = 1-t;
   return it*it*it*p0 + 3*it*it*t*p1 + 3*it*t*t*p2 + t*t*t*p3;
}

void rblxstb_newell_normal(float *normal, int num_vert, float **vert, int normalize)
{
   int i,j;
   float p;
   normal[0] = normal[1] = normal[2] = 0;
   for (i=num_vert-1,j=0; j < num_vert; i=j++) {
      float *u = vert[i];
      float *v = vert[j];
      normal[0] += (u[1] - v[1]) * (u[2] + v[2]);
      normal[1] += (u[2] - v[2]) * (u[0] + v[0]);
      normal[2] += (u[0] - v[0]) * (u[1] + v[1]);
   }
   if (normalize) {
      p = normal[0]*normal[0] + normal[1]*normal[1] + normal[2]*normal[2];
      p = (float) (1.0 / sqrt(p));
      normal[0] *= p;
      normal[1] *= p;
      normal[2] *= p;
   }
}

int rblxstb_box_face_vertex_axis_side(int face_number, int vertex_number, int axis)
{
   static int box_vertices[6][4][3] =
   {
      { { 1,1,1 }, { 1,0,1 }, { 1,0,0 }, { 1,1,0 } },
      { { 0,0,0 }, { 0,0,1 }, { 0,1,1 }, { 0,1,0 } },
      { { 0,0,0 }, { 0,1,0 }, { 1,1,0 }, { 1,0,0 } },
      { { 0,0,0 }, { 1,0,0 }, { 1,0,1 }, { 0,0,1 } },
      { { 1,1,1 }, { 0,1,1 }, { 0,0,1 }, { 1,0,1 } },
      { { 1,1,1 }, { 1,1,0 }, { 0,1,0 }, { 0,1,1 } },
   };
   assert(face_number >= 0 && face_number < 6);
   assert(vertex_number >= 0 && vertex_number < 4);
   assert(axis >= 0 && axis < 3);
   return box_vertices[face_number][vertex_number][axis];
}

void rblxstb_linear_controller(float *curpos, float target_pos, float acc, float deacc, float dt)
{
   float sign = 1, p, cp = *curpos;
   if (cp == target_pos) return;
   if (target_pos < cp) {
      target_pos = -target_pos;
      cp = -cp;
      sign = -1;
   }
   // first decelerate
   if (cp < 0) {
      p = cp + deacc * dt;
      if (p > 0) {
         p = 0;
         dt = dt - cp / deacc;
         if (dt < 0) dt = 0;
      } else {
         dt = 0;
      }
      cp = p;
   }
   // now accelerate
   p = cp + acc*dt;
   if (p > target_pos) p = target_pos;
   *curpos = p * sign;
   // @TODO: testing
}

float rblxstb_quadratic_controller(float target_pos, float curpos, float maxvel, float maxacc, float dt, float *curvel)
{
   return 0; // @TODO
}

int rblxstb_float_eq(float x, float y, float delta, int max_ulps)
{
   if (fabs(x-y) <= delta) return 1;
   if (abs(*(int *)&x - *(int *)&y) <= max_ulps) return 1;
   return 0;
}

int rblxstb_is_prime(unsigned int m)
{
   unsigned int i,j;
   if (m < 2) return 0;
   if (m == 2) return 1;
   if (!(m & 1)) return 0;
   if (m % 3 == 0) return (m == 3);
   for (i=5; (j=i*i), j <= m && j > i; i += 6) {
      if (m %   i   == 0) return 0;
      if (m % (i+2) == 0) return 0;
   }
   return 1;
}

unsigned int rblxstb_power_of_two_nearest_prime(int n)
{
   static signed char tab[32] = { 0,0,0,0,1,0,-1,0,1,-1,-1,3,-1,0,-1,2,1,
                                   0,2,0,-1,-4,-1,5,-1,18,-2,15,2,-1,2,0 };
   if (!tab[0]) {
      int i;
      for (i=0; i < 32; ++i)
         tab[i] = (1 << i) + 2*tab[i] - 1;
      tab[1] = 2;
      tab[0] = 1;
   }
   if (n >= 32) return 0xfffffffb;
   return tab[n];
}

double rblxstb_linear_remap(double x, double x_min, double x_max,
                                  double out_min, double out_max)
{
   return rblxstb_lerp(rblxstb_unlerp(x,x_min,x_max),out_min,out_max);
}
#endif

// create a macro so it's faster, but you can get at the function pointer
#define rblxstb_linear_remap(t,a,b,c,d)   rblxstb_lerp(rblxstb_unlerp(t,a,b),c,d)


//////////////////////////////////////////////////////////////////////////////
//
//                         bit operations
//

#define rblxstb_big32(c)    (((c)[0]<<24) + (c)[1]*65536 + (c)[2]*256 + (c)[3])
#define rblxstb_little32(c) (((c)[3]<<24) + (c)[2]*65536 + (c)[1]*256 + (c)[0])
#define rblxstb_big16(c)    ((c)[0]*256 + (c)[1])
#define rblxstb_little16(c) ((c)[1]*256 + (c)[0])

RBLXSTB_EXTERN          int rblxstb_bitcount(unsigned int a);
RBLXSTB_EXTERN unsigned int rblxstb_bitreverse8(unsigned char n);
RBLXSTB_EXTERN unsigned int rblxstb_bitreverse(unsigned int n);

RBLXSTB_EXTERN          int rblxstb_is_pow2(size_t);
RBLXSTB_EXTERN          int rblxstb_log2_ceil(size_t);
RBLXSTB_EXTERN          int rblxstb_log2_floor(size_t);

RBLXSTB_EXTERN          int rblxstb_lowbit8(unsigned int n);
RBLXSTB_EXTERN          int rblxstb_highbit8(unsigned int n);

#ifdef RBLXSTB_DEFINE
int rblxstb_bitcount(unsigned int a)
{
   a = (a & 0x55555555) + ((a >>  1) & 0x55555555); // max 2
   a = (a & 0x33333333) + ((a >>  2) & 0x33333333); // max 4
   a = (a + (a >> 4)) & 0x0f0f0f0f; // max 8 per 4, now 8 bits
   a = (a + (a >> 8)); // max 16 per 8 bits
   a = (a + (a >> 16)); // max 32 per 8 bits
   return a & 0xff;
}

unsigned int rblxstb_bitreverse8(unsigned char n)
{
   n = ((n & 0xAA) >> 1) + ((n & 0x55) << 1);
   n = ((n & 0xCC) >> 2) + ((n & 0x33) << 2);
   return (unsigned char) ((n >> 4) + (n << 4));
}

unsigned int rblxstb_bitreverse(unsigned int n)
{
  n = ((n & 0xAAAAAAAA) >>  1) | ((n & 0x55555555) << 1);
  n = ((n & 0xCCCCCCCC) >>  2) | ((n & 0x33333333) << 2);
  n = ((n & 0xF0F0F0F0) >>  4) | ((n & 0x0F0F0F0F) << 4);
  n = ((n & 0xFF00FF00) >>  8) | ((n & 0x00FF00FF) << 8);
  return (n >> 16) | (n << 16);
}

int rblxstb_is_pow2(size_t n)
{
   return (n & (n-1)) == 0;
}

// tricky use of 4-bit table to identify 5 bit positions (note the '-1')
// 3-bit table would require another tree level; 5-bit table wouldn't save one
#if defined(_WIN32) && !defined(__MINGW32__)
#pragma warning(push)
#pragma warning(disable: 4035)  // disable warning about no return value
int rblxstb_log2_floor(size_t n)
{
   #if _MSC_VER > 1700
   unsigned long i;
   #ifdef RBLXSTB_PTR64
   _BitScanReverse64(&i, n);
   #else
   _BitScanReverse(&i, n);
   #endif
   return i != 0 ? i : -1;
   #else
   __asm {
      bsr eax,n
      jnz done
      mov eax,-1
   }
   done:;
   #endif
}
#pragma warning(pop)
#else
int rblxstb_log2_floor(size_t n)
{
   static signed char log2_4[16] = { -1,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3 };

#ifdef RBLXSTB_PTR64
   if (n >= ((size_t) 1u << 32))
        return rblxstb_log2_floor(n >> 32);
#endif

   // 2 compares if n < 16, 3 compares otherwise
   if (n < (1U << 14))
        if (n < (1U <<  4))        return     0 + log2_4[n      ];
        else if (n < (1U <<  9))      return  5 + log2_4[n >>  5];
             else                     return 10 + log2_4[n >> 10];
   else if (n < (1U << 24))
             if (n < (1U << 19))      return 15 + log2_4[n >> 15];
             else                     return 20 + log2_4[n >> 20];
        else if (n < (1U << 29))      return 25 + log2_4[n >> 25];
             else                     return 30 + log2_4[n >> 30];
}
#endif

// define ceil from floor
int rblxstb_log2_ceil(size_t n)
{
   if (rblxstb_is_pow2(n))  return     rblxstb_log2_floor(n);
   else                 return 1 + rblxstb_log2_floor(n);
}

int rblxstb_highbit8(unsigned int n)
{
   return rblxstb_log2_ceil(n&255);
}

int rblxstb_lowbit8(unsigned int n)
{
   static signed char lowbit4[16] = { -1,0,1,0, 2,0,1,0, 3,0,1,0, 2,0,1,0 };
   int k = lowbit4[n & 15];
   if (k >= 0) return k;
   k = lowbit4[(n >> 4) & 15];
   if (k >= 0) return k+4;
   return k;
}
#endif



//////////////////////////////////////////////////////////////////////////////
//
//                            qsort Compare Routines
//

#ifdef _WIN32
   #define rblxstb_stricmp(a,b) rblxstb_p_stricmp(a,b)
   #define rblxstb_strnicmp(a,b,n) rblxstb_p_strnicmp(a,b,n)
#else
   #define rblxstb_stricmp(a,b) strcasecmp(a,b)
   #define rblxstb_strnicmp(a,b,n) strncasecmp(a,b,n)
#endif


RBLXSTB_EXTERN int (*rblxstb_intcmp(int offset))(const void *a, const void *b);
RBLXSTB_EXTERN int (*rblxstb_intcmprev(int offset))(const void *a, const void *b);
RBLXSTB_EXTERN int (*rblxstb_qsort_strcmp(int offset))(const void *a, const void *b);
RBLXSTB_EXTERN int (*rblxstb_qsort_stricmp(int offset))(const void *a, const void *b);
RBLXSTB_EXTERN int (*rblxstb_floatcmp(int offset))(const void *a, const void *b);
RBLXSTB_EXTERN int (*rblxstb_doublecmp(int offset))(const void *a, const void *b);
RBLXSTB_EXTERN int (*rblxstb_charcmp(int offset))(const void *a, const void *b);

#ifdef RBLXSTB_DEFINE
static int rblxstb__intcmpoffset, rblxstb__ucharcmpoffset, rblxstb__strcmpoffset;
static int rblxstb__floatcmpoffset, rblxstb__doublecmpoffset;
static int rblxstb__memcmpoffset, rblxstb__memcmpsize;

int rblxstb__intcmp(const void *a, const void *b)
{
   const int p = *(const int *) ((const char *) a + rblxstb__intcmpoffset);
   const int q = *(const int *) ((const char *) b + rblxstb__intcmpoffset);
   return p < q ? -1 : p > q;
}

int rblxstb__intcmprev(const void *a, const void *b)
{
   const int p = *(const int *) ((const char *) a + rblxstb__intcmpoffset);
   const int q = *(const int *) ((const char *) b + rblxstb__intcmpoffset);
   return q < p ? -1 : q > p;
}

int rblxstb__ucharcmp(const void *a, const void *b)
{
   const int p = *(const unsigned char *) ((const char *) a + rblxstb__ucharcmpoffset);
   const int q = *(const unsigned char *) ((const char *) b + rblxstb__ucharcmpoffset);
   return p < q ? -1 : p > q;
}

int rblxstb__floatcmp(const void *a, const void *b)
{
   const float p = *(const float *) ((const char *) a + rblxstb__floatcmpoffset);
   const float q = *(const float *) ((const char *) b + rblxstb__floatcmpoffset);
   return p < q ? -1 : p > q;
}

int rblxstb__doublecmp(const void *a, const void *b)
{
   const double p = *(const double *) ((const char *) a + rblxstb__doublecmpoffset);
   const double q = *(const double *) ((const char *) b + rblxstb__doublecmpoffset);
   return p < q ? -1 : p > q;
}

int rblxstb__qsort_strcmp(const void *a, const void *b)
{
   const char *p = *(const char **) ((const char *) a + rblxstb__strcmpoffset);
   const char *q = *(const char **) ((const char *) b + rblxstb__strcmpoffset);
   return strcmp(p,q);
}

int rblxstb__qsort_stricmp(const void *a, const void *b)
{
   const char *p = *(const char **) ((const char *) a + rblxstb__strcmpoffset);
   const char *q = *(const char **) ((const char *) b + rblxstb__strcmpoffset);
   return rblxstb_stricmp(p,q);
}

int rblxstb__memcmp(const void *a, const void *b)
{
   return memcmp((char *) a + rblxstb__memcmpoffset, (char *) b + rblxstb__memcmpoffset, rblxstb__memcmpsize);
}

int (*rblxstb_intcmp(int offset))(const void *, const void *)
{
   rblxstb__intcmpoffset = offset;
   return &rblxstb__intcmp;
}

int (*rblxstb_intcmprev(int offset))(const void *, const void *)
{
   rblxstb__intcmpoffset = offset;
   return &rblxstb__intcmprev;
}

int (*rblxstb_ucharcmp(int offset))(const void *, const void *)
{
   rblxstb__ucharcmpoffset = offset;
   return &rblxstb__ucharcmp;
}

int (*rblxstb_qsort_strcmp(int offset))(const void *, const void *)
{
   rblxstb__strcmpoffset = offset;
   return &rblxstb__qsort_strcmp;
}

int (*rblxstb_qsort_stricmp(int offset))(const void *, const void *)
{
   rblxstb__strcmpoffset = offset;
   return &rblxstb__qsort_stricmp;
}

int (*rblxstb_floatcmp(int offset))(const void *, const void *)
{
   rblxstb__floatcmpoffset = offset;
   return &rblxstb__floatcmp;
}

int (*rblxstb_doublecmp(int offset))(const void *, const void *)
{
   rblxstb__doublecmpoffset = offset;
   return &rblxstb__doublecmp;
}

int (*rblxstb_memcmp(int offset, int size))(const void *, const void *)
{
   rblxstb__memcmpoffset = offset;
   rblxstb__memcmpsize = size;
   return &rblxstb__memcmp;
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
//                           Binary Search Toolkit
//

typedef struct
{
   int minval, maxval, guess;
   int mode, step;
} rblxstb_search;

RBLXSTB_EXTERN int rblxstb_search_binary(rblxstb_search *s, int minv, int maxv, int find_smallest);
RBLXSTB_EXTERN int rblxstb_search_open(rblxstb_search *s, int minv, int find_smallest);
RBLXSTB_EXTERN int rblxstb_probe(rblxstb_search *s, int compare, int *result); // return 0 when done

#ifdef RBLXSTB_DEFINE
enum
{
   RBLXSTB_probe_binary_smallest,
   RBLXSTB_probe_binary_largest,
   RBLXSTB_probe_open_smallest,
   RBLXSTB_probe_open_largest,
};

static int rblxstb_probe_guess(rblxstb_search *s, int *result)
{
   switch(s->mode) {
      case RBLXSTB_probe_binary_largest:
         if (s->minval == s->maxval) {
            *result = s->minval;
            return 0;
         }
         assert(s->minval < s->maxval);
         // if a < b, then a < p <= b
         s->guess = s->minval + (((unsigned) s->maxval - s->minval + 1) >> 1);
         break;

      case RBLXSTB_probe_binary_smallest:
         if (s->minval == s->maxval) {
            *result = s->minval;
            return 0;
         }
         assert(s->minval < s->maxval);
         // if a < b, then a <= p < b
         s->guess = s->minval + (((unsigned) s->maxval - s->minval) >> 1);
         break;
      case RBLXSTB_probe_open_smallest:
      case RBLXSTB_probe_open_largest:
         s->guess = s->maxval;  // guess the current maxval
         break;
   }
   *result = s->guess;
   return 1;
}

int rblxstb_probe(rblxstb_search *s, int compare, int *result)
{
   switch(s->mode) {
      case RBLXSTB_probe_open_smallest:
      case RBLXSTB_probe_open_largest: {
         if (compare <= 0) {
            // then it lies within minval & maxval
            if (s->mode == RBLXSTB_probe_open_smallest)
               s->mode = RBLXSTB_probe_binary_smallest;
            else
               s->mode = RBLXSTB_probe_binary_largest;
         } else {
            // otherwise, we need to probe larger
            s->minval  = s->maxval + 1;
            s->maxval  = s->minval + s->step;
            s->step   += s->step;
         }
         break;
      }
      case RBLXSTB_probe_binary_smallest: {
         // if compare < 0, then s->minval <= a <  p
         // if compare = 0, then s->minval <= a <= p
         // if compare > 0, then         p <  a <= s->maxval
         if (compare <= 0)
            s->maxval = s->guess;
         else
            s->minval = s->guess+1;
         break;
      }
      case RBLXSTB_probe_binary_largest: {
         // if compare < 0, then s->minval <= a < p
         // if compare = 0, then         p <= a <= s->maxval
         // if compare > 0, then         p <  a <= s->maxval
         if (compare < 0)
            s->maxval = s->guess-1;
         else
            s->minval = s->guess;
         break;
      }
   }
   return rblxstb_probe_guess(s, result);
}

int rblxstb_search_binary(rblxstb_search *s, int minv, int maxv, int find_smallest)
{
   int r;
   if (maxv < minv) return minv-1;
   s->minval = minv;
   s->maxval = maxv;
   s->mode = find_smallest ? RBLXSTB_probe_binary_smallest : RBLXSTB_probe_binary_largest;
   rblxstb_probe_guess(s, &r);
   return r;
}

int rblxstb_search_open(rblxstb_search *s, int minv, int find_smallest)
{
   int r;
   s->step   = 4;
   s->minval = minv;
   s->maxval = minv+s->step;
   s->mode = find_smallest ? RBLXSTB_probe_open_smallest : RBLXSTB_probe_open_largest;
   rblxstb_probe_guess(s, &r);
   return r;
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
//                           String Processing
//

#define rblxstb_prefixi(s,t)  (0==rblxstb_strnicmp((s),(t),strlen(t)))

enum rblxstb_splitpath_flag
{
   RBLXSTB_PATH = 1,
   RBLXSTB_FILE = 2,
   RBLXSTB_EXT  = 4,
   RBLXSTB_PATH_FILE = RBLXSTB_PATH + RBLXSTB_FILE,
   RBLXSTB_FILE_EXT  = RBLXSTB_FILE + RBLXSTB_EXT,
   RBLXSTB_EXT_NO_PERIOD = 8,
};

RBLXSTB_EXTERN char * rblxstb_skipwhite(char *s);
RBLXSTB_EXTERN char * rblxstb_trimwhite(char *s);
RBLXSTB_EXTERN char * rblxstb_skipnewline(char *s);
RBLXSTB_EXTERN char * rblxstb_strncpy(char *s, char *t, int n);
RBLXSTB_EXTERN char * rblxstb_substr(char *t, int n);
RBLXSTB_EXTERN char * rblxstb_duplower(char *s);
RBLXSTB_EXTERN void   rblxstb_tolower (char *s);
RBLXSTB_EXTERN char * rblxstb_strchr2 (char *s, char p1, char p2);
RBLXSTB_EXTERN char * rblxstb_strrchr2(char *s, char p1, char p2);
RBLXSTB_EXTERN char * rblxstb_strtok(char *output, char *src, char *delimit);
RBLXSTB_EXTERN char * rblxstb_strtok_keep(char *output, char *src, char *delimit);
RBLXSTB_EXTERN char * rblxstb_strtok_invert(char *output, char *src, char *allowed);
RBLXSTB_EXTERN char * rblxstb_dupreplace(char *s, char *find, char *replace);
RBLXSTB_EXTERN void   rblxstb_replaceinplace(char *s, char *find, char *replace);
RBLXSTB_EXTERN char * rblxstb_splitpath(char *output, char *src, int flag);
RBLXSTB_EXTERN char * rblxstb_splitpathdup(char *src, int flag);
RBLXSTB_EXTERN char * rblxstb_replacedir(char *output, char *src, char *dir);
RBLXSTB_EXTERN char * rblxstb_replaceext(char *output, char *src, char *ext);
RBLXSTB_EXTERN void   rblxstb_fixpath(char *path);
RBLXSTB_EXTERN char * rblxstb_shorten_path_readable(char *path, int max_len);
RBLXSTB_EXTERN int    rblxstb_suffix (char *s, char *t);
RBLXSTB_EXTERN int    rblxstb_suffixi(char *s, char *t);
RBLXSTB_EXTERN int    rblxstb_prefix (char *s, char *t);
RBLXSTB_EXTERN char * rblxstb_strichr(char *s, char t);
RBLXSTB_EXTERN char * rblxstb_stristr(char *s, char *t);
RBLXSTB_EXTERN int    rblxstb_prefix_count(char *s, char *t);
RBLXSTB_EXTERN const char * rblxstb_plural(int n);  // "s" or ""
RBLXSTB_EXTERN size_t rblxstb_strscpy(char *d, const char *s, size_t n);

RBLXSTB_EXTERN char **rblxstb_tokens(char *src, char *delimit, int *count);
RBLXSTB_EXTERN char **rblxstb_tokens_nested(char *src, char *delimit, int *count, char *nest_in, char *nest_out);
RBLXSTB_EXTERN char **rblxstb_tokens_nested_empty(char *src, char *delimit, int *count, char *nest_in, char *nest_out);
RBLXSTB_EXTERN char **rblxstb_tokens_allowempty(char *src, char *delimit, int *count);
RBLXSTB_EXTERN char **rblxstb_tokens_stripwhite(char *src, char *delimit, int *count);
RBLXSTB_EXTERN char **rblxstb_tokens_withdelim(char *src, char *delimit, int *count);
RBLXSTB_EXTERN char **rblxstb_tokens_quoted(char *src, char *delimit, int *count);
// with 'quoted', allow delimiters to appear inside quotation marks, and don't
// strip whitespace inside them (and we delete the quotation marks unless they
// appear back to back, in which case they're considered escaped)

#ifdef RBLXSTB_DEFINE

size_t rblxstb_strscpy(char *d, const char *s, size_t n)
{
   size_t len = strlen(s);
   if (len >= n) {
      if (n) d[0] = 0;
      return 0;
   }
   rblxstb_p_strcpy_s(d,n,s);
   return len;
}

const char *rblxstb_plural(int n)
{
   return n == 1 ? "" : "s";
}

int rblxstb_prefix(char *s, char *t)
{
   while (*t)
      if (*s++ != *t++)
         return RBLXSTB_FALSE;
   return RBLXSTB_TRUE;
}

int rblxstb_prefix_count(char *s, char *t)
{
   int c=0;
   while (*t) {
      if (*s++ != *t++)
         break;
      ++c;
   }
   return c;
}

int rblxstb_suffix(char *s, char *t)
{
   size_t n = strlen(s);
   size_t m = strlen(t);
   if (m <= n)
      return 0 == strcmp(s+n-m, t);
   else
      return 0;
}

int rblxstb_suffixi(char *s, char *t)
{
   size_t n = strlen(s);
   size_t m = strlen(t);
   if (m <= n)
      return 0 == rblxstb_stricmp(s+n-m, t);
   else
      return 0;
}

// originally I was using this table so that I could create known sentinel
// values--e.g. change whitetable[0] to be true if I was scanning for whitespace,
// and false if I was scanning for nonwhite. I don't appear to be using that
// functionality anymore (I do for tokentable, though), so just replace it
// with isspace()
char *rblxstb_skipwhite(char *s)
{
   while (isspace((unsigned char) *s)) ++s;
   return s;
}

char *rblxstb_skipnewline(char *s)
{
   if (s[0] == '\r' || s[0] == '\n') {
      if (s[0]+s[1] == '\r' + '\n') ++s;
      ++s;
   }
   return s;
}

char *rblxstb_trimwhite(char *s)
{
   int i,n;
   s = rblxstb_skipwhite(s);
   n = (int) strlen(s);
   for (i=n-1; i >= 0; --i)
      if (!isspace(s[i]))
         break;
   s[i+1] = 0;
   return s;
}

char *rblxstb_strncpy(char *s, char *t, int n)
{
   rblxstb_p_strncpy_s(s,n+1,t,n);
   s[n] = 0;
   return s;
}

char *rblxstb_substr(char *t, int n)
{
   char *a;
   int z = (int) strlen(t);
   if (z < n) n = z;
   a = (char *) malloc(n+1);
   rblxstb_p_strncpy_s(a,n+1,t,n);
   a[n] = 0;
   return a;
}

char *rblxstb_duplower(char *s)
{
   char *p = rblxstb_p_strdup(s), *q = p;
   while (*q) {
      *q = tolower(*q);
      ++q;
   }
   return p;
}

void rblxstb_tolower(char *s)
{
   while (*s) {
      *s = tolower(*s);
      ++s;
   }
}

char *rblxstb_strchr2(char *s, char x, char y)
{
   for(; *s; ++s)
      if (*s == x || *s == y)
         return s;
   return NULL;
}

char *rblxstb_strrchr2(char *s, char x, char y)
{
   char *r = NULL;
   for(; *s; ++s)
      if (*s == x || *s == y)
         r = s;
   return r;
}

char *rblxstb_strichr(char *s, char t)
{
   if (tolower(t) == toupper(t))
      return strchr(s,t);
   return rblxstb_strchr2(s, (char) tolower(t), (char) toupper(t));
}

char *rblxstb_stristr(char *s, char *t)
{
   size_t n = strlen(t);
   char *z;
   if (n==0) return s;
   while ((z = rblxstb_strichr(s, *t)) != NULL) {
      if (0==rblxstb_strnicmp(z, t, n))
         return z;
      s = z+1;
   }
   return NULL;
}

static char *rblxstb_strtok_raw(char *output, char *src, char *delimit, int keep, int invert)
{
   if (invert) {
      while (*src && strchr(delimit, *src) != NULL) {
         *output++ = *src++;
      }
   } else {
      while (*src && strchr(delimit, *src) == NULL) {
         *output++ = *src++;
      }
   }
   *output = 0;
   if (keep)
      return src;
   else
      return *src ? src+1 : src;
}

char *rblxstb_strtok(char *output, char *src, char *delimit)
{
   return rblxstb_strtok_raw(output, src, delimit, 0, 0);
}

char *rblxstb_strtok_keep(char *output, char *src, char *delimit)
{
   return rblxstb_strtok_raw(output, src, delimit, 1, 0);
}

char *rblxstb_strtok_invert(char *output, char *src, char *delimit)
{
   return rblxstb_strtok_raw(output, src, delimit, 1,1);
}

static char **rblxstb_tokens_raw(char *src_, char *delimit, int *count,
                             int stripwhite, int allow_empty, char *start, char *end)
{
   int nested = 0;
   unsigned char *src = (unsigned char *) src_;
   static char rblxstb_tokentable[256]; // rely on static initializion to 0
   static char stable[256],etable[256];
   char *out;
   char **result;
   int num=0;
   unsigned char *s;

   s = (unsigned char *) delimit; while (*s) rblxstb_tokentable[*s++] = 1;
   if (start) {
      s = (unsigned char *) start;         while (*s) stable[*s++] = 1;
      s = (unsigned char *) end;   if (s)  while (*s) stable[*s++] = 1;
      s = (unsigned char *) end;   if (s)  while (*s) etable[*s++] = 1;
   }
   stable[0] = 1;

   // two passes through: the first time, counting how many
   s = (unsigned char *) src;
   while (*s) {
      // state: just found delimiter
      // skip further delimiters
      if (!allow_empty) {
         rblxstb_tokentable[0] = 0;
         while (rblxstb_tokentable[*s])
            ++s;
         if (!*s) break;
      }
      ++num;
      // skip further non-delimiters
      rblxstb_tokentable[0] = 1;
      if (stripwhite == 2) { // quoted strings
         while (!rblxstb_tokentable[*s]) {
            if (*s != '"')
               ++s;
            else {
               ++s;
               if (*s == '"')
                  ++s;   // "" -> ", not start a string
               else {
                  // begin a string
                  while (*s) {
                     if (s[0] == '"') {
                        if (s[1] == '"') s += 2; // "" -> "
                        else { ++s; break; } // terminating "
                     } else
                        ++s;
                  }
               }
            }
         }
      } else
         while (nested || !rblxstb_tokentable[*s]) {
            if (stable[*s]) {
               if (!*s) break;
               if (end ? etable[*s] : nested)
                  --nested;
               else
                  ++nested;
            }
            ++s;
         }
      if (allow_empty) {
         if (*s) ++s;
      }
   }
   // now num has the actual count... malloc our output structure
   // need space for all the strings: strings won't be any longer than
   // original input, since for every '\0' there's at least one delimiter
   result = (char **) malloc(sizeof(*result) * (num+1) + (s-src+1));
   if (result == NULL) return result;
   out = (char *) (result + (num+1));
   // second pass: copy out the data
   s = (unsigned char *) src;
   num = 0;
   nested = 0;
   while (*s) {
      char *last_nonwhite;
      // state: just found delimiter
      // skip further delimiters
      if (!allow_empty) {
         rblxstb_tokentable[0] = 0;
         if (stripwhite)
            while (rblxstb_tokentable[*s] || isspace(*s))
               ++s;
         else
            while (rblxstb_tokentable[*s])
               ++s;
      } else if (stripwhite) {
         while (isspace(*s)) ++s;
      }
      if (!*s) break;
      // we're past any leading delimiters and whitespace
      result[num] = out;
      ++num;
      // copy non-delimiters
      rblxstb_tokentable[0] = 1;
      last_nonwhite = out-1;
      if (stripwhite == 2) {
         while (!rblxstb_tokentable[*s]) {
            if (*s != '"') {
               if (!isspace(*s)) last_nonwhite = out;
               *out++ = *s++;
            } else {
               ++s;
               if (*s == '"') {
                  if (!isspace(*s)) last_nonwhite = out;
                  *out++ = *s++; // "" -> ", not start string
               } else {
                  // begin a quoted string
                  while (*s) {
                     if (s[0] == '"') {
                        if (s[1] == '"') { *out++ = *s; s += 2; }
                        else { ++s; break; } // terminating "
                     } else
                        *out++ = *s++;
                  }
                  last_nonwhite = out-1; // all in quotes counts as non-white
               }
            }
         }
      } else {
         while (nested || !rblxstb_tokentable[*s]) {
            if (!isspace(*s)) last_nonwhite = out;
            if (stable[*s]) {
               if (!*s) break;
               if (end ? etable[*s] : nested)
                  --nested;
               else
                  ++nested;
            }
            *out++ = *s++;
         }
      }

      if (stripwhite) // rewind to last non-whitespace char
         out = last_nonwhite+1;
      *out++ = '\0';

      if (*s) ++s; // skip delimiter
   }
   s = (unsigned char *) delimit; while (*s) rblxstb_tokentable[*s++] = 0;
   if (start) {
      s = (unsigned char *) start;         while (*s) stable[*s++] = 1;
      s = (unsigned char *) end;   if (s)  while (*s) stable[*s++] = 1;
      s = (unsigned char *) end;   if (s)  while (*s) etable[*s++] = 1;
   }
   if (count != NULL) *count = num;
   result[num] = 0;
   return result;
}

char **rblxstb_tokens(char *src, char *delimit, int *count)
{
   return rblxstb_tokens_raw(src,delimit,count,0,0,0,0);
}

char **rblxstb_tokens_nested(char *src, char *delimit, int *count, char *nest_in, char *nest_out)
{
   return rblxstb_tokens_raw(src,delimit,count,0,0,nest_in,nest_out);
}

char **rblxstb_tokens_nested_empty(char *src, char *delimit, int *count, char *nest_in, char *nest_out)
{
   return rblxstb_tokens_raw(src,delimit,count,0,1,nest_in,nest_out);
}

char **rblxstb_tokens_allowempty(char *src, char *delimit, int *count)
{
   return rblxstb_tokens_raw(src,delimit,count,0,1,0,0);
}

char **rblxstb_tokens_stripwhite(char *src, char *delimit, int *count)
{
   return rblxstb_tokens_raw(src,delimit,count,1,1,0,0);
}

char **rblxstb_tokens_quoted(char *src, char *delimit, int *count)
{
   return rblxstb_tokens_raw(src,delimit,count,2,1,0,0);
}

char *rblxstb_dupreplace(char *src, char *find, char *replace)
{
   size_t len_find = strlen(find);
   size_t len_replace = strlen(replace);
   int count = 0;

   char *s,*p,*q;

   s = strstr(src, find);
   if (s == NULL) return rblxstb_p_strdup(src);
   do {
      ++count;
      s = strstr(s + len_find, find);
   } while (s != NULL);

   p = (char *)  malloc(strlen(src) + count * (len_replace - len_find) + 1);
   if (p == NULL) return p;
   q = p;
   s = src;
   for (;;) {
      char *t = strstr(s, find);
      if (t == NULL) {
         rblxstb_p_strcpy_s(q,strlen(src)+count*(len_replace-len_find)+1,s);
         assert(strlen(p) == strlen(src) + count*(len_replace-len_find));
         return p;
      }
      memcpy(q, s, t-s);
      q += t-s;
      memcpy(q, replace, len_replace);
      q += len_replace;
      s = t + len_find;
   }
}

void rblxstb_replaceinplace(char *src, char *find, char *replace)
{
   size_t len_find = strlen(find);
   size_t len_replace = strlen(replace);
   int delta;

   char *s,*p,*q;

   delta = (int) (len_replace - len_find);
   assert(delta <= 0);
   if (delta > 0) return;

   p = strstr(src, find);
   if (p == NULL) return;

   s = q = p;
   while (*s) {
      memcpy(q, replace, len_replace);
      p += len_find;
      q += len_replace;
      s = strstr(p, find);
      if (s == NULL) s = p + strlen(p);
      memmove(q, p, s-p);
      q += s-p;
      p = s;
   }
   *q = 0;
}

void rblxstb_fixpath(char *path)
{
   for(; *path; ++path)
      if (*path == '\\')
         *path = '/';
}

void rblxstb__add_section(char *buffer, char *data, ptrdiff_t curlen, ptrdiff_t newlen)
{
   if (newlen < curlen) {
      ptrdiff_t z1 = newlen >> 1, z2 = newlen-z1;
      memcpy(buffer, data, z1-1);
      buffer[z1-1] = '.';
      buffer[z1-0] = '.';
      memcpy(buffer+z1+1, data+curlen-z2+1, z2-1);
   } else
      memcpy(buffer, data, curlen);
}

char * rblxstb_shorten_path_readable(char *path, int len)
{
   static char buffer[1024];
   ptrdiff_t n = strlen(path),n1,n2,r1,r2;
   char *s;
   if (n <= len) return path;
   if (len > 1024) return path;
   s = rblxstb_strrchr2(path, '/', '\\');
   if (s) {
      n1 = s - path + 1;
      n2 = n - n1;
      ++s;
   } else {
      n1 = 0;
      n2 = n;
      s = path;
   }
   // now we need to reduce r1 and r2 so that they fit in len
   if (n1 < len>>1) {
      r1 = n1;
      r2 = len - r1;
   } else if (n2 < len >> 1) {
      r2 = n2;
      r1 = len - r2;
   } else {
      r1 = n1 * len / n;
      r2 = n2 * len / n;
      if (r1 < len>>2) r1 = len>>2, r2 = len-r1;
      if (r2 < len>>2) r2 = len>>2, r1 = len-r2;
   }
   assert(r1 <= n1 && r2 <= n2);
   if (n1)
      rblxstb__add_section(buffer, path, n1, r1);
   rblxstb__add_section(buffer+r1, s, n2, r2);
   buffer[len] = 0;
   return buffer;
}

static char *rblxstb__splitpath_raw(char *buffer, char *path, int flag)
{
   ptrdiff_t len=0,x,y, n = (int) strlen(path), f1,f2;
   char *s = rblxstb_strrchr2(path, '/', '\\');
   char *t = strrchr(path, '.');
   if (s && t && t < s) t = NULL;

   if (!s) {
      // check for drive
      if (isalpha(path[0]) && path[1] == ':')
         s = &path[1];
   }
   if (s) ++s;

   if (flag == RBLXSTB_EXT_NO_PERIOD)
      flag |= RBLXSTB_EXT;

   if (!(flag & (RBLXSTB_PATH | RBLXSTB_FILE | RBLXSTB_EXT))) return NULL;

   f1 = s == NULL ? 0 : s-path; // start of filename
   f2 = t == NULL ? n : t-path; // just past end of filename

   if (flag & RBLXSTB_PATH) {
      x = 0; if (f1 == 0 && flag == RBLXSTB_PATH) len=2;
   } else if (flag & RBLXSTB_FILE) {
      x = f1;
   } else {
      x = f2;
      if (flag & RBLXSTB_EXT_NO_PERIOD)
         if (path[x] == '.')
            ++x;
   }

   if (flag & RBLXSTB_EXT)
      y = n;
   else if (flag & RBLXSTB_FILE)
      y = f2;
   else
      y = f1;

   if (buffer == NULL) {
      buffer = (char *) malloc(y-x + len + 1);
      if (!buffer) return NULL;
   }

   if (len) { rblxstb_p_strcpy_s(buffer, 3, "./"); return buffer; }
   rblxstb_strncpy(buffer, path+(int)x, (int)(y-x));
   return buffer;
}

char *rblxstb_splitpath(char *output, char *src, int flag)
{
   return rblxstb__splitpath_raw(output, src, flag);
}

char *rblxstb_splitpathdup(char *src, int flag)
{
   return rblxstb__splitpath_raw(NULL, src, flag);
}

char *rblxstb_replacedir(char *output, char *src, char *dir)
{
   char buffer[4096];
   rblxstb_splitpath(buffer, src, RBLXSTB_FILE | RBLXSTB_EXT);
   if (dir)
      rblxstb_p_sprintf(output rblxstb_p_size(9999), "%s/%s", dir, buffer);
   else
      rblxstb_p_strcpy_s(output, sizeof(buffer),  buffer); // @UNSAFE
   return output;
}

char *rblxstb_replaceext(char *output, char *src, char *ext)
{
   char buffer[4096];
   rblxstb_splitpath(buffer, src, RBLXSTB_PATH | RBLXSTB_FILE);
   if (ext)
      rblxstb_p_sprintf(output rblxstb_p_size(9999), "%s.%s", buffer, ext[0] == '.' ? ext+1 : ext);
   else
      rblxstb_p_strcpy_s(output, sizeof(buffer), buffer); // @UNSAFE
   return output;
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
//                   rblxstb_alloc - hierarchical allocator
//
//                                     inspired by http://swapped.cc/halloc
//
//
// When you alloc a given block through rblxstb_alloc, you have these choices:
//
//       1. does it have a parent?
//       2. can it have children?
//       3. can it be freed directly?
//       4. is it transferrable?
//       5. what is its alignment?
//
// Here are interesting combinations of those:
//
//                              children   free    transfer     alignment
//  arena                          Y         Y         N           n/a
//  no-overhead, chunked           N         N         N         normal
//  string pool alloc              N         N         N            1
//  parent-ptr, chunked            Y         N         N         normal
//  low-overhead, unchunked        N         Y         Y         normal
//  general purpose alloc          Y         Y         Y         normal
//
// Unchunked allocations will probably return 16-aligned pointers. If
// we 16-align the results, we have room for 4 pointers. For smaller
// allocations that allow finer alignment, we can reduce the pointers.
//
// The strategy is that given a pointer, assuming it has a header (only
// the no-overhead allocations have no header), we can determine the
// type of the header fields, and the number of them, by stepping backwards
// through memory and looking at the tags in the bottom bits.
//
// Implementation strategy:
//     chunked allocations come from the middle of chunks, and can't
//     be freed. thefore they do not need to be on a sibling chain.
//     they may need child pointers if they have children.
//
// chunked, with-children
//     void *parent;
//
// unchunked, no-children -- reduced storage
//     void *next_sibling;
//     void *prev_sibling_nextp;
//
// unchunked, general
//     void *first_child;
//     void *next_sibling;
//     void *prev_sibling_nextp;
//     void *chunks;
//
// so, if we code each of these fields with different bit patterns
// (actually same one for next/prev/child), then we can identify which
// each one is from the last field.

RBLXSTB_EXTERN void  rblxstb_free(void *p);
RBLXSTB_EXTERN void *rblxstb_malloc_global(size_t size);
RBLXSTB_EXTERN void *rblxstb_malloc(void *context, size_t size);
RBLXSTB_EXTERN void *rblxstb_malloc_nofree(void *context, size_t size);
RBLXSTB_EXTERN void *rblxstb_malloc_leaf(void *context, size_t size);
RBLXSTB_EXTERN void *rblxstb_malloc_raw(void *context, size_t size);
RBLXSTB_EXTERN void *rblxstb_realloc(void *ptr, size_t newsize);

RBLXSTB_EXTERN void rblxstb_reassign(void *new_context, void *ptr);
RBLXSTB_EXTERN void rblxstb_malloc_validate(void *p, void *parent);

extern int rblxstb_alloc_chunk_size ;
extern int rblxstb_alloc_count_free ;
extern int rblxstb_alloc_count_alloc;
extern int rblxstb_alloc_alignment  ;

#ifdef RBLXSTB_DEFINE

int rblxstb_alloc_chunk_size  = 65536;
int rblxstb_alloc_count_free  = 0;
int rblxstb_alloc_count_alloc = 0;
int rblxstb_alloc_alignment   = -16;

typedef struct rblxstb__chunk
{
   struct rblxstb__chunk *next;
   int                data_left;
   int                alloc;
} rblxstb__chunk;

typedef struct
{
   void *  next;
   void ** prevn;
} rblxstb__nochildren;

typedef struct
{
   void ** prevn;
   void *  child;
   void *  next;
   rblxstb__chunk *chunks;
} rblxstb__alloc;

typedef struct
{
   rblxstb__alloc *parent;
} rblxstb__chunked;

#define RBLXSTB__PARENT          1
#define RBLXSTB__CHUNKS          2

typedef enum
{
   RBLXSTB__nochildren = 0,
   RBLXSTB__chunked    = RBLXSTB__PARENT,
   RBLXSTB__alloc      = RBLXSTB__CHUNKS,

   RBLXSTB__chunk_raw  = 4,
} rblxstb__alloc_type;

// these functions set the bottom bits of a pointer efficiently
#define RBLXSTB__DECODE(x,v)  ((void *) ((char *) (x) - (v)))
#define RBLXSTB__ENCODE(x,v)  ((void *) ((char *) (x) + (v)))

#define rblxstb__parent(z)       (rblxstb__alloc *) RBLXSTB__DECODE((z)->parent, RBLXSTB__PARENT)
#define rblxstb__chunks(z)       (rblxstb__chunk *) RBLXSTB__DECODE((z)->chunks, RBLXSTB__CHUNKS)

#define rblxstb__setparent(z,p)  (z)->parent = (rblxstb__alloc *) RBLXSTB__ENCODE((p), RBLXSTB__PARENT)
#define rblxstb__setchunks(z,c)  (z)->chunks = (rblxstb__chunk *) RBLXSTB__ENCODE((c), RBLXSTB__CHUNKS)

static rblxstb__alloc rblxstb__alloc_global =
{
   NULL,
   NULL,
   NULL,
   (rblxstb__chunk *) RBLXSTB__ENCODE(NULL, RBLXSTB__CHUNKS)
};

static rblxstb__alloc_type rblxstb__identify(void *p)
{
   void **q = (void **) p;
   return (rblxstb__alloc_type) ((rblxstb_uinta) q[-1] & 3);
}

static void *** rblxstb__prevn(void *p)
{
   if (rblxstb__identify(p) == RBLXSTB__alloc) {
      rblxstb__alloc      *s = (rblxstb__alloc *) p - 1;
      return &s->prevn;
   } else {
      rblxstb__nochildren *s = (rblxstb__nochildren *) p - 1;
      return &s->prevn;
   }
}

void rblxstb_free(void *p)
{
   if (p == NULL) return;

   // count frees so that unit tests can see what's happening
   ++rblxstb_alloc_count_free;

   switch(rblxstb__identify(p)) {
      case RBLXSTB__chunked:
         // freeing a chunked-block with children does nothing;
         // they only get freed when the parent does
         // surely this is wrong, and it should free them immediately?
         // otherwise how are they getting put on the right chain?
         return;
      case RBLXSTB__nochildren: {
         rblxstb__nochildren *s = (rblxstb__nochildren *) p - 1;
         // unlink from sibling chain
         *(s->prevn) = s->next;
         if (s->next)
            *rblxstb__prevn(s->next) = s->prevn;
         free(s);
         return;
      }
      case RBLXSTB__alloc: {
         rblxstb__alloc *s = (rblxstb__alloc *) p - 1;
         rblxstb__chunk *c, *n;
         void *q;

         // unlink from sibling chain, if any
         *(s->prevn) = s->next;
         if (s->next)
            *rblxstb__prevn(s->next) = s->prevn;

         // first free chunks
         c = (rblxstb__chunk *) rblxstb__chunks(s);
         while (c != NULL) {
            n = c->next;
            rblxstb_alloc_count_free += c->alloc;
            free(c);
            c = n;
         }

         // validating
         rblxstb__setchunks(s,NULL);
         s->prevn = NULL;
         s->next = NULL;

         // now free children
         while ((q = s->child) != NULL) {
            rblxstb_free(q);
         }

         // now free self
         free(s);
         return;
      }
      default:
         assert(0); /* NOTREACHED */
   }
}

void rblxstb_malloc_validate(void *p, void *parent)
{
   if (p == NULL) return;

   switch(rblxstb__identify(p)) {
      case RBLXSTB__chunked:
         return;
      case RBLXSTB__nochildren: {
         rblxstb__nochildren *n = (rblxstb__nochildren *) p - 1;
         if (n->prevn)
            assert(*n->prevn == p);
         if (n->next) {
            assert(*rblxstb__prevn(n->next) == &n->next);
            rblxstb_malloc_validate(n, parent);
         }
         return;
      }
      case RBLXSTB__alloc: {
         rblxstb__alloc *s = (rblxstb__alloc *) p - 1;

         if (s->prevn)
            assert(*s->prevn == p);

         if (s->child) {
            assert(*rblxstb__prevn(s->child) == &s->child);
            rblxstb_malloc_validate(s->child, p);
         }

         if (s->next) {
            assert(*rblxstb__prevn(s->next) == &s->next);
            rblxstb_malloc_validate(s->next, parent);
         }
         return;
      }
      default:
         assert(0); /* NOTREACHED */
   }
}

static void * rblxstb__try_chunk(rblxstb__chunk *c, int size, int align, int pre_align)
{
   char *memblock = (char *) (c+1), *q;
   rblxstb_inta iq;
   int start_offset;

   // we going to allocate at the end of the chunk, not the start. confusing,
   // but it means we don't need both a 'limit' and a 'cur', just a 'cur'.
   // the block ends at: p + c->data_left
   //   then we move back by size
   start_offset = c->data_left - size;

   // now we need to check the alignment of that
   q = memblock + start_offset;
   iq = (rblxstb_inta) q;
   assert(sizeof(q) == sizeof(iq));

   // suppose align = 2
   // then we need to retreat iq far enough that (iq & (2-1)) == 0
   // to get (iq & (align-1)) = 0 requires subtracting (iq & (align-1))

   start_offset -= iq & (align-1);
   assert(((rblxstb_uinta) (memblock+start_offset) & (align-1)) == 0);

   // now, if that + pre_align works, go for it!
   start_offset -= pre_align;

   if (start_offset >= 0) {
      c->data_left = start_offset;
      return memblock + start_offset;
   }

   return NULL;
}

static void rblxstb__sort_chunks(rblxstb__alloc *src)
{
   // of the first two chunks, put the chunk with more data left in it first
   rblxstb__chunk *c = rblxstb__chunks(src), *d;
   if (c == NULL) return;
   d = c->next;
   if (d == NULL) return;
   if (c->data_left > d->data_left) return;

   c->next = d->next;
   d->next = c;
   rblxstb__setchunks(src, d);
}

static void * rblxstb__alloc_chunk(rblxstb__alloc *src, int size, int align, int pre_align)
{
   void *p;
   rblxstb__chunk *c = rblxstb__chunks(src);

   if (c && size <= rblxstb_alloc_chunk_size) {

      p = rblxstb__try_chunk(c, size, align, pre_align);
      if (p) { ++c->alloc; return p; }

      // try a second chunk to reduce wastage
      if (c->next) {
         p = rblxstb__try_chunk(c->next, size, align, pre_align);
         if (p) { ++c->alloc; return p; }

         // put the bigger chunk first, since the second will get buried
         // the upshot of this is that, until it gets allocated from, chunk #2
         // is always the largest remaining chunk. (could formalize
         // this with a heap!)
         rblxstb__sort_chunks(src);
         c = rblxstb__chunks(src);
      }
   }

   // allocate a new chunk
   {
      rblxstb__chunk *n;

      int chunk_size = rblxstb_alloc_chunk_size;
      // we're going to allocate a new chunk to put this in
      if (size > chunk_size)
         chunk_size = size;

      assert(sizeof(*n) + pre_align <= 16);

      // loop trying to allocate a large enough chunk
      // the loop is because the alignment may cause problems if it's big...
      // and we don't know what our chunk alignment is going to be
      while (1) {
         n = (rblxstb__chunk *) malloc(16 + chunk_size);
         if (n == NULL) return NULL;

         n->data_left = chunk_size - sizeof(*n);

         p = rblxstb__try_chunk(n, size, align, pre_align);
         if (p != NULL) {
            n->next = c;
            rblxstb__setchunks(src, n);

            // if we just used up the whole block immediately,
            // move the following chunk up
            n->alloc = 1;
            if (size == chunk_size)
               rblxstb__sort_chunks(src);

            return p;
         }

         free(n);
         chunk_size += 16+align;
      }
   }
}

static rblxstb__alloc * rblxstb__get_context(void *context)
{
   if (context == NULL) {
      return &rblxstb__alloc_global;
   } else {
      int u = rblxstb__identify(context);
      // if context is chunked, grab parent
      if (u == RBLXSTB__chunked) {
         rblxstb__chunked *s = (rblxstb__chunked *) context - 1;
         return rblxstb__parent(s);
      } else {
         return (rblxstb__alloc *) context - 1;
      }
   }
}

static void rblxstb__insert_alloc(rblxstb__alloc *src, rblxstb__alloc *s)
{
   s->prevn = &src->child;
   s->next  = src->child;
   src->child = s+1;
   if (s->next)
      *rblxstb__prevn(s->next) = &s->next;
}

static void rblxstb__insert_nochild(rblxstb__alloc *src, rblxstb__nochildren *s)
{
   s->prevn = &src->child;
   s->next  = src->child;
   src->child = s+1;
   if (s->next)
      *rblxstb__prevn(s->next) = &s->next;
}

static void * malloc_base(void *context, size_t size, rblxstb__alloc_type t, int align)
{
   void *p;

   rblxstb__alloc *src = rblxstb__get_context(context);

   if (align <= 0) {
      // compute worst-case C packed alignment
      // e.g. a 24-byte struct is 8-aligned
      int align_proposed = 1 << rblxstb_lowbit8((unsigned int) size);

      if (align_proposed < 0)
         align_proposed = 4;

      if (align_proposed == 0) {
         if (size == 0)
            align_proposed = 1;
         else
            align_proposed = 256;
      }

      // a negative alignment means 'don't align any larger
      // than this'; so -16 means we align 1,2,4,8, or 16

      if (align < 0) {
         if (align_proposed > -align)
            align_proposed = -align;
      }

      align = align_proposed;
   }

   assert(rblxstb_is_pow2(align));

   // don't cause misalignment when allocating nochildren
   if (t == RBLXSTB__nochildren && align > 8)
      t = RBLXSTB__alloc;

   switch (t) {
      case RBLXSTB__alloc: {
         rblxstb__alloc *s = (rblxstb__alloc *) malloc(size + sizeof(*s));
         if (s == NULL) return NULL;
         p = s+1;
         s->child = NULL;
         rblxstb__insert_alloc(src, s);

         rblxstb__setchunks(s,NULL);
         break;
      }

      case RBLXSTB__nochildren: {
         rblxstb__nochildren *s = (rblxstb__nochildren *) malloc(size + sizeof(*s));
         if (s == NULL) return NULL;
         p = s+1;
         rblxstb__insert_nochild(src, s);
         break;
      }

      case RBLXSTB__chunk_raw: {
         p = rblxstb__alloc_chunk(src, (int) size, align, 0);
         if (p == NULL) return NULL;
         break;
      }

      case RBLXSTB__chunked: {
         rblxstb__chunked *s;
         if (align < sizeof(rblxstb_uintptr)) align = sizeof(rblxstb_uintptr);
         s = (rblxstb__chunked *) rblxstb__alloc_chunk(src, (int) size, align, sizeof(*s));
         if (s == NULL) return NULL;
         rblxstb__setparent(s, src);
         p = s+1;
         break;
      }

      default: p = NULL; assert(0); /* NOTREACHED */
   }

   ++rblxstb_alloc_count_alloc;
   return p;
}

void *rblxstb_malloc_global(size_t size)
{
   return malloc_base(NULL, size, RBLXSTB__alloc, rblxstb_alloc_alignment);
}

void *rblxstb_malloc(void *context, size_t size)
{
   return malloc_base(context, size, RBLXSTB__alloc, rblxstb_alloc_alignment);
}

void *rblxstb_malloc_nofree(void *context, size_t size)
{
   return malloc_base(context, size, RBLXSTB__chunked, rblxstb_alloc_alignment);
}

void *rblxstb_malloc_leaf(void *context, size_t size)
{
   return malloc_base(context, size, RBLXSTB__nochildren, rblxstb_alloc_alignment);
}

void *rblxstb_malloc_raw(void *context, size_t size)
{
   return malloc_base(context, size, RBLXSTB__chunk_raw, rblxstb_alloc_alignment);
}

char *rblxstb_malloc_string(void *context, size_t size)
{
   return (char *) malloc_base(context, size, RBLXSTB__chunk_raw, 1);
}

void *rblxstb_realloc(void *ptr, size_t newsize)
{
   rblxstb__alloc_type t;

   if (ptr == NULL) return rblxstb_malloc(NULL, newsize);
   if (newsize == 0) { rblxstb_free(ptr); return NULL; }

   t = rblxstb__identify(ptr);
   assert(t == RBLXSTB__alloc || t == RBLXSTB__nochildren);

   if (t == RBLXSTB__alloc) {
      rblxstb__alloc *s = (rblxstb__alloc *) ptr - 1;

      s = (rblxstb__alloc *) realloc(s, newsize + sizeof(*s));
      if (s == NULL) return NULL;

      ptr = s+1;

      // update pointers
      (*s->prevn) = ptr;
      if (s->next)
         *rblxstb__prevn(s->next) = &s->next;

      if (s->child)
         *rblxstb__prevn(s->child) = &s->child;

      return ptr;
   } else {
      rblxstb__nochildren *s = (rblxstb__nochildren *) ptr - 1;

      s = (rblxstb__nochildren *) realloc(ptr, newsize + sizeof(s));
      if (s == NULL) return NULL;

      // update pointers
      (*s->prevn) = s+1;
      if (s->next)
         *rblxstb__prevn(s->next) = &s->next;

      return s+1;
   }
}

void *rblxstb_realloc_c(void *context, void *ptr, size_t newsize)
{
   if (ptr == NULL) return rblxstb_malloc(context, newsize);
   if (newsize == 0) { rblxstb_free(ptr); return NULL; }
   // @TODO: verify you haven't changed contexts
   return rblxstb_realloc(ptr, newsize);
}

void rblxstb_reassign(void *new_context, void *ptr)
{
   rblxstb__alloc *src = rblxstb__get_context(new_context);

   rblxstb__alloc_type t = rblxstb__identify(ptr);
   assert(t == RBLXSTB__alloc || t == RBLXSTB__nochildren);

   if (t == RBLXSTB__alloc) {
      rblxstb__alloc *s = (rblxstb__alloc *) ptr - 1;

      // unlink from old
      *(s->prevn) = s->next;
      if (s->next)
         *rblxstb__prevn(s->next) = s->prevn;

      rblxstb__insert_alloc(src, s);
   } else {
      rblxstb__nochildren *s = (rblxstb__nochildren *) ptr - 1;

      // unlink from old
      *(s->prevn) = s->next;
      if (s->next)
         *rblxstb__prevn(s->next) = s->prevn;

      rblxstb__insert_nochild(src, s);
   }
}

#endif


//////////////////////////////////////////////////////////////////////////////
//
//                                rblxstb_arr
//
//  An rblxstb_arr is directly useable as a pointer (use the actual type in your
//  definition), but when it resizes, it returns a new pointer and you can't
//  use the old one, so you have to be careful to copy-in-out as necessary.
//
//  Use a NULL pointer as a 0-length array.
//
//     float *my_array = NULL, *temp;
//
//     // add elements on the end one at a time
//     rblxstb_arr_push(my_array, 0.0f);
//     rblxstb_arr_push(my_array, 1.0f);
//     rblxstb_arr_push(my_array, 2.0f);
//
//     assert(my_array[1] == 2.0f);
//
//     // add an uninitialized element at the end, then assign it
//     *rblxstb_arr_add(my_array) = 3.0f;
//
//     // add three uninitialized elements at the end
//     temp = rblxstb_arr_addn(my_array,3);
//     temp[0] = 4.0f;
//     temp[1] = 5.0f;
//     temp[2] = 6.0f;
//
//     assert(my_array[5] == 5.0f);
//
//     // remove the last one
//     rblxstb_arr_pop(my_array);
//
//     assert(rblxstb_arr_len(my_array) == 6);


#ifdef RBLXSTB_MALLOC_WRAPPER
  #define RBLXSTB__PARAMS    , char *file, int line
  #define RBLXSTB__ARGS      ,       file,     line
#else
  #define RBLXSTB__PARAMS
  #define RBLXSTB__ARGS
#endif

// calling this function allocates an empty rblxstb_arr attached to p
// (whereas NULL isn't attached to anything)
RBLXSTB_EXTERN void rblxstb_arr_malloc(void **target, void *context);

// call this function with a non-NULL value to have all successive
// rblxstbs that are created be attached to the associated parent. Note
// that once a given rblxstb_arr is non-empty, it stays attached to its
// current parent, even if you call this function again.
// it turns the previous value, so you can restore it
RBLXSTB_EXTERN void* rblxstb_arr_malloc_parent(void *p);

// simple functions written on top of other functions
#define rblxstb_arr_empty(a)       (  rblxstb_arr_len(a) == 0 )
#define rblxstb_arr_add(a)         (  rblxstb_arr_addn((a),1) )
#define rblxstb_arr_push(a,v)      ( *rblxstb_arr_add(a)=(v)  )

typedef struct
{
   int len, limit;
   int rblxstb_malloc;
   unsigned int signature;
} rblxstb__arr;

#define rblxstb_arr_signature      0x51bada7b  // ends with 0123 in decimal

// access the header block stored before the data
#define rblxstb_arrhead(a)         /*lint --e(826)*/ (((rblxstb__arr *) (a)) - 1)
#define rblxstb_arrhead2(a)        /*lint --e(826)*/ (((rblxstb__arr *) (a)) - 1)

#ifdef RBLXSTB_DEBUG
#define rblxstb_arr_check(a)       assert(!a || rblxstb_arrhead(a)->signature == rblxstb_arr_signature)
#define rblxstb_arr_check2(a)      assert(!a || rblxstb_arrhead2(a)->signature == rblxstb_arr_signature)
#else
#define rblxstb_arr_check(a)       ((void) 0)
#define rblxstb_arr_check2(a)      ((void) 0)
#endif

// ARRAY LENGTH

// get the array length; special case if pointer is NULL
#define rblxstb_arr_len(a)         (a ? rblxstb_arrhead(a)->len : 0)
#define rblxstb_arr_len2(a)        ((rblxstb__arr *) (a) ? rblxstb_arrhead2(a)->len : 0)
#define rblxstb_arr_lastn(a)       (rblxstb_arr_len(a)-1)

// check whether a given index is valid -- tests 0 <= i < rblxstb_arr_len(a)
#define rblxstb_arr_valid(a,i)     (a ? (int) (i) < rblxstb_arrhead(a)->len : 0)

// change the array length so is is exactly N entries long, creating
// uninitialized entries as needed
#define rblxstb_arr_setlen(a,n)  \
            (rblxstb__arr_setlen((void **) &(a), sizeof(a[0]), (n)))

// change the array length so that N is a valid index (that is, so
// it is at least N entries long), creating uninitialized entries as needed
#define rblxstb_arr_makevalid(a,n)  \
            (rblxstb_arr_len(a) < (n)+1 ? rblxstb_arr_setlen(a,(n)+1),(a) : (a))

// remove the last element of the array, returning it
#define rblxstb_arr_pop(a)         ((rblxstb_arr_check(a), (a))[--rblxstb_arrhead(a)->len])

// access the last element in the array
#define rblxstb_arr_last(a)        ((rblxstb_arr_check(a), (a))[rblxstb_arr_len(a)-1])

// is iterator at end of list?
#define rblxstb_arr_end(a,i)       ((i) >= &(a)[rblxstb_arr_len(a)])

// (internal) change the allocated length of the array
#define rblxstb_arr__grow(a,n)     (rblxstb_arr_check(a), rblxstb_arrhead(a)->len += (n))

// add N new uninitialized elements to the end of the array
#define rblxstb_arr__addn(a,n)     /*lint --e(826)*/ \
                               ((rblxstb_arr_len(a)+(n) > rblxstb_arrcurmax(a))      \
                                 ? (rblxstb__arr_addlen((void **) &(a),sizeof(*a),(n)),0) \
                                 : ((rblxstb_arr__grow(a,n), 0)))

// add N new uninitialized elements to the end of the array, and return
// a pointer to the first new one
#define rblxstb_arr_addn(a,n)      (rblxstb_arr__addn((a),n),(a)+rblxstb_arr_len(a)-(n))

// add N new uninitialized elements starting at index 'i'
#define rblxstb_arr_insertn(a,i,n) (rblxstb__arr_insertn((void **) &(a), sizeof(*a), (i), (n)))

// insert an element at i
#define rblxstb_arr_insert(a,i,v)  (rblxstb__arr_insertn((void **) &(a), sizeof(*a), (i), (1)), ((a)[i] = v))

// delete N elements from the middle starting at index 'i'
#define rblxstb_arr_deleten(a,i,n) (rblxstb__arr_deleten((void **) &(a), sizeof(*a), (i), (n)))

// delete the i'th element
#define rblxstb_arr_delete(a,i)   rblxstb_arr_deleten(a,i,1)

// delete the i'th element, swapping down from the end
#define rblxstb_arr_fastdelete(a,i)  \
   (rblxstb_swap(&a[i], &a[rblxstb_arrhead(a)->len-1], sizeof(*a)), rblxstb_arr_pop(a))


// ARRAY STORAGE

// get the array maximum storage; special case if NULL
#define rblxstb_arrcurmax(a)       (a ? rblxstb_arrhead(a)->limit : 0)
#define rblxstb_arrcurmax2(a)      (a ? rblxstb_arrhead2(a)->limit : 0)

// set the maxlength of the array to n in anticipation of further growth
#define rblxstb_arr_setsize(a,n)   (rblxstb_arr_check(a), rblxstb__arr_setsize((void **) &(a),sizeof((a)[0]),n))

// make sure maxlength is large enough for at least N new allocations
#define rblxstb_arr_atleast(a,n)   (rblxstb_arr_len(a)+(n) > rblxstb_arrcurmax(a)      \
                                 ? rblxstb_arr_setsize((a), (n)) : 0)

// make a copy of a given array (copies contents via 'memcpy'!)
#define rblxstb_arr_copy(a)        rblxstb__arr_copy(a, sizeof((a)[0]))

// compute the storage needed to store all the elements of the array
#define rblxstb_arr_storage(a)     (rblxstb_arr_len(a) * sizeof((a)[0]))

#define rblxstb_arr_for(v,arr)     for((v)=(arr); (v) < (arr)+rblxstb_arr_len(arr); ++(v))

// IMPLEMENTATION

RBLXSTB_EXTERN void rblxstb_arr_free_(void **p);
RBLXSTB_EXTERN void *rblxstb__arr_copy_(void *p, int elem_size);
RBLXSTB_EXTERN void rblxstb__arr_setsize_(void **p, int size, int limit  RBLXSTB__PARAMS);
RBLXSTB_EXTERN void rblxstb__arr_setlen_(void **p, int size, int newlen  RBLXSTB__PARAMS);
RBLXSTB_EXTERN void rblxstb__arr_addlen_(void **p, int size, int addlen  RBLXSTB__PARAMS);
RBLXSTB_EXTERN void rblxstb__arr_deleten_(void **p, int size, int loc, int n  RBLXSTB__PARAMS);
RBLXSTB_EXTERN void rblxstb__arr_insertn_(void **p, int size, int loc, int n  RBLXSTB__PARAMS);

#define rblxstb_arr_free(p)            rblxstb_arr_free_((void **) &(p))
#define rblxstb__arr_copy              rblxstb__arr_copy_

#ifndef RBLXSTB_MALLOC_WRAPPER
  #define rblxstb__arr_setsize         rblxstb__arr_setsize_
  #define rblxstb__arr_setlen          rblxstb__arr_setlen_
  #define rblxstb__arr_addlen          rblxstb__arr_addlen_
  #define rblxstb__arr_deleten         rblxstb__arr_deleten_
  #define rblxstb__arr_insertn         rblxstb__arr_insertn_
#else
  #define rblxstb__arr_addlen(p,s,n)    rblxstb__arr_addlen_(p,s,n,__FILE__,__LINE__)
  #define rblxstb__arr_setlen(p,s,n)    rblxstb__arr_setlen_(p,s,n,__FILE__,__LINE__)
  #define rblxstb__arr_setsize(p,s,n)   rblxstb__arr_setsize_(p,s,n,__FILE__,__LINE__)
  #define rblxstb__arr_deleten(p,s,i,n) rblxstb__arr_deleten_(p,s,i,n,__FILE__,__LINE__)
  #define rblxstb__arr_insertn(p,s,i,n) rblxstb__arr_insertn_(p,s,i,n,__FILE__,__LINE__)
#endif

#ifdef RBLXSTB_DEFINE
static void *rblxstb__arr_context;

void *rblxstb_arr_malloc_parent(void *p)
{
   void *q = rblxstb__arr_context;
   rblxstb__arr_context = p;
   return q;
}

void rblxstb_arr_malloc(void **target, void *context)
{
   rblxstb__arr *q = (rblxstb__arr *) rblxstb_malloc(context, sizeof(*q));
   q->len = q->limit = 0;
   q->rblxstb_malloc = 1;
   q->signature = rblxstb_arr_signature;
   *target = (void *) (q+1);
}

static void * rblxstb__arr_malloc(int size)
{
   if (rblxstb__arr_context)
      return rblxstb_malloc(rblxstb__arr_context, size);
   return malloc(size);
}

void * rblxstb__arr_copy_(void *p, int elem_size)
{
   rblxstb__arr *q;
   if (p == NULL) return p;
   q = (rblxstb__arr *) rblxstb__arr_malloc(sizeof(*q) + elem_size * rblxstb_arrhead2(p)->limit);
   rblxstb_arr_check2(p);
   memcpy(q, rblxstb_arrhead2(p), sizeof(*q) + elem_size * rblxstb_arrhead2(p)->len);
   q->rblxstb_malloc = !!rblxstb__arr_context;
   return q+1;
}

void rblxstb_arr_free_(void **pp)
{
   void *p = *pp;
   rblxstb_arr_check2(p);
   if (p) {
      rblxstb__arr *q = rblxstb_arrhead2(p);
      if (q->rblxstb_malloc)
         rblxstb_free(q);
      else
         free(q);
   }
   *pp = NULL;
}

static void rblxstb__arrsize_(void **pp, int size, int limit, int len  RBLXSTB__PARAMS)
{
   void *p = *pp;
   rblxstb__arr *a;
   rblxstb_arr_check2(p);
   if (p == NULL) {
      if (len == 0 && size == 0) return;
      a = (rblxstb__arr *) rblxstb__arr_malloc(sizeof(*a) + size*limit);
      a->limit = limit;
      a->len   = len;
      a->rblxstb_malloc = !!rblxstb__arr_context;
      a->signature = rblxstb_arr_signature;
   } else {
      a = rblxstb_arrhead2(p);
      a->len = len;
      if (a->limit < limit) {
         void *p;
         if (a->limit >= 4 && limit < a->limit * 2)
            limit = a->limit * 2;
         if (a->rblxstb_malloc)
            p = rblxstb_realloc(a, sizeof(*a) + limit*size);
         else
            #ifdef RBLXSTB_MALLOC_WRAPPER
            p = rblxstb__realloc(a, sizeof(*a) + limit*size, file, line);
            #else
            p = realloc(a, sizeof(*a) + limit*size);
            #endif
         if (p) {
            a = (rblxstb__arr *) p;
            a->limit = limit;
         } else {
            // throw an error!
         }
      }
   }
   a->len   = rblxstb_min(a->len, a->limit);
   *pp = a+1;
}

void rblxstb__arr_setsize_(void **pp, int size, int limit  RBLXSTB__PARAMS)
{
   void *p = *pp;
   rblxstb_arr_check2(p);
   rblxstb__arrsize_(pp, size, limit, rblxstb_arr_len2(p)  RBLXSTB__ARGS);
}

void rblxstb__arr_setlen_(void **pp, int size, int newlen  RBLXSTB__PARAMS)
{
   void *p = *pp;
   rblxstb_arr_check2(p);
   if (rblxstb_arrcurmax2(p) < newlen || p == NULL) {
      rblxstb__arrsize_(pp, size, newlen, newlen  RBLXSTB__ARGS);
   } else {
      rblxstb_arrhead2(p)->len = newlen;
   }
}

void rblxstb__arr_addlen_(void **p, int size, int addlen  RBLXSTB__PARAMS)
{
   rblxstb__arr_setlen_(p, size, rblxstb_arr_len2(*p) + addlen  RBLXSTB__ARGS);
}

void rblxstb__arr_insertn_(void **pp, int size, int i, int n  RBLXSTB__PARAMS)
{
   void *p = *pp;
   if (n) {
      int z;

      if (p == NULL) {
         rblxstb__arr_addlen_(pp, size, n  RBLXSTB__ARGS);
         return;
      }

      z = rblxstb_arr_len2(p);
      rblxstb__arr_addlen_(&p, size, n  RBLXSTB__ARGS);
      memmove((char *) p + (i+n)*size, (char *) p + i*size, size * (z-i));
   }
   *pp = p;
}

void rblxstb__arr_deleten_(void **pp, int size, int i, int n  RBLXSTB__PARAMS)
{
   void *p = *pp;
   if (n) {
      memmove((char *) p + i*size, (char *) p + (i+n)*size, size * (rblxstb_arr_len2(p)-(i+n)));
      rblxstb_arrhead2(p)->len -= n;
   }
   *pp = p;
}

#endif

//////////////////////////////////////////////////////////////////////////////
//
//                               Hashing
//
//      typical use for this is to make a power-of-two hash table.
//
//      let N = size of table (2^n)
//      let H = rblxstb_hash(str)
//      let S = rblxstb_rehash(H) | 1
//
//      then hash probe sequence P(i) for i=0..N-1
//         P(i) = (H + S*i) & (N-1)
//
//      the idea is that H has 32 bits of hash information, but the
//      table has only, say, 2^20 entries so only uses 20 of the bits.
//      then by rehashing the original H we get 2^12 different probe
//      sequences for a given initial probe location. (So it's optimal
//      for 64K tables and its optimality decreases past that.)
//
//      ok, so I've added something that generates _two separate_
//      32-bit hashes simultaneously which should scale better to
//      very large tables.


RBLXSTB_EXTERN unsigned int rblxstb_hash(char *str);
RBLXSTB_EXTERN unsigned int rblxstb_hashptr(void *p);
RBLXSTB_EXTERN unsigned int rblxstb_hashlen(char *str, int len);
RBLXSTB_EXTERN unsigned int rblxstb_rehash_improved(unsigned int v);
RBLXSTB_EXTERN unsigned int rblxstb_hash_fast(void *p, int len);
RBLXSTB_EXTERN unsigned int rblxstb_hash2(char *str, unsigned int *hash2_ptr);
RBLXSTB_EXTERN unsigned int rblxstb_hash_number(unsigned int hash);

#define rblxstb_rehash(x)  ((x) + ((x) >> 6) + ((x) >> 19))

#ifdef RBLXSTB_DEFINE
unsigned int rblxstb_hash(char *str)
{
   unsigned int hash = 0;
   while (*str)
      hash = (hash << 7) + (hash >> 25) + *str++;
   return hash + (hash >> 16);
}

unsigned int rblxstb_hashlen(char *str, int len)
{
   unsigned int hash = 0;
   while (len-- > 0 && *str)
      hash = (hash << 7) + (hash >> 25) + *str++;
   return hash + (hash >> 16);
}

unsigned int rblxstb_hashptr(void *p)
{
    unsigned int x = (unsigned int)(size_t) p;

   // typically lacking in low bits and high bits
   x = rblxstb_rehash(x);
   x += x << 16;

   // pearson's shuffle
   x ^= x << 3;
   x += x >> 5;
   x ^= x << 2;
   x += x >> 15;
   x ^= x << 10;
   return rblxstb_rehash(x);
}

unsigned int rblxstb_rehash_improved(unsigned int v)
{
   return rblxstb_hashptr((void *)(size_t) v);
}

unsigned int rblxstb_hash2(char *str, unsigned int *hash2_ptr)
{
   unsigned int hash1 = 0x3141592c;
   unsigned int hash2 = 0x77f044ed;
   while (*str) {
      hash1 = (hash1 << 7) + (hash1 >> 25) + *str;
      hash2 = (hash2 << 11) + (hash2 >> 21) + *str;
      ++str;
   }
   *hash2_ptr = hash2 + (hash1 >> 16);
   return       hash1 + (hash2 >> 16);
}

// Paul Hsieh hash
#define rblxstb__get16(p) ((p)[0] | ((p)[1] << 8))

unsigned int rblxstb_hash_fast(void *p, int len)
{
   unsigned char *q = (unsigned char *) p;
   unsigned int hash = len;

   if (len <= 0 || q == NULL) return 0;

   /* Main loop */
   for (;len > 3; len -= 4) {
      unsigned int val;
      hash +=  rblxstb__get16(q);
      val   = (rblxstb__get16(q+2) << 11);
      hash  = (hash << 16) ^ hash ^ val;
      q    += 4;
      hash += hash >> 11;
   }

   /* Handle end cases */
   switch (len) {
      case 3: hash += rblxstb__get16(q);
              hash ^= hash << 16;
              hash ^= q[2] << 18;
              hash += hash >> 11;
              break;
      case 2: hash += rblxstb__get16(q);
              hash ^= hash << 11;
              hash += hash >> 17;
              break;
      case 1: hash += q[0];
              hash ^= hash << 10;
              hash += hash >> 1;
              break;
      case 0: break;
   }

   /* Force "avalanching" of final 127 bits */
   hash ^= hash << 3;
   hash += hash >> 5;
   hash ^= hash << 4;
   hash += hash >> 17;
   hash ^= hash << 25;
   hash += hash >> 6;

   return hash;
}

unsigned int rblxstb_hash_number(unsigned int hash)
{
   hash ^= hash << 3;
   hash += hash >> 5;
   hash ^= hash << 4;
   hash += hash >> 17;
   hash ^= hash << 25;
   hash += hash >> 6;
   return hash;
}

#endif

#ifdef RBLXSTB_PERFECT_HASH
//////////////////////////////////////////////////////////////////////////////
//
//                     Perfect hashing for ints/pointers
//
//   This is mainly useful for making faster pointer-indexed tables
//   that don't change frequently. E.g. for rblxstb_ischar().
//

typedef struct
{
   rblxstb_uint32  addend;
   rblxstb_uint    multiplicand;
   rblxstb_uint    b_mask;
   rblxstb_uint8   small_bmap[16];
   rblxstb_uint16  *large_bmap;

   rblxstb_uint table_mask;
   rblxstb_uint32 *table;
} rblxstb_perfect;

RBLXSTB_EXTERN int rblxstb_perfect_create(rblxstb_perfect *,unsigned int*,int n);
RBLXSTB_EXTERN void rblxstb_perfect_destroy(rblxstb_perfect *);
RBLXSTB_EXTERN int rblxstb_perfect_hash(rblxstb_perfect *, unsigned int x);
extern int rblxstb_perfect_hash_max_failures;

#ifdef RBLXSTB_DEFINE

int rblxstb_perfect_hash_max_failures;

int rblxstb_perfect_hash(rblxstb_perfect *p, unsigned int x)
{
   rblxstb_uint m = x * p->multiplicand;
   rblxstb_uint y = x >> 16;
   rblxstb_uint bv = (m >> 24) + y;
   rblxstb_uint av = (m + y) >> 12;
   if (p->table == NULL) return -1;  // uninitialized table fails
   bv &= p->b_mask;
   av &= p->table_mask;
   if (p->large_bmap)
      av ^= p->large_bmap[bv];
   else
      av ^= p->small_bmap[bv];
   return p->table[av] == x ? av : -1;
}

static void rblxstb__perfect_prehash(rblxstb_perfect *p, rblxstb_uint x, rblxstb_uint16 *a, rblxstb_uint16 *b)
{
   rblxstb_uint m = x * p->multiplicand;
   rblxstb_uint y = x >> 16;
   rblxstb_uint bv = (m >> 24) + y;
   rblxstb_uint av = (m + y) >> 12;
   bv &= p->b_mask;
   av &= p->table_mask;
   *b = bv;
   *a = av;
}

static unsigned long rblxstb__perfect_rand(void)
{
   static unsigned long rblxstb__rand;
   rblxstb__rand = rblxstb__rand * 2147001325 + 715136305;
   return 0x31415926 ^ ((rblxstb__rand >> 16) + (rblxstb__rand << 16));
}

typedef struct {
   unsigned short count;
   unsigned short b;
   unsigned short map;
   unsigned short *entries;
} rblxstb__slot;

static int rblxstb__slot_compare(const void *p, const void *q)
{
   rblxstb__slot *a = (rblxstb__slot *) p;
   rblxstb__slot *b = (rblxstb__slot *) q;
   return a->count > b->count ? -1 : a->count < b->count;  // sort large to small
}

int rblxstb_perfect_create(rblxstb_perfect *p, unsigned int *v, int n)
{
   unsigned int buffer1[64], buffer2[64], buffer3[64], buffer4[64], buffer5[32];
   unsigned short *as = (unsigned short *) rblxstb_temp(buffer1, sizeof(*v)*n);
   unsigned short *bs = (unsigned short *) rblxstb_temp(buffer2, sizeof(*v)*n);
   unsigned short *entries = (unsigned short *) rblxstb_temp(buffer4, sizeof(*entries) * n);
   int size = 1 << rblxstb_log2_ceil(n), bsize=8;
   int failure = 0,i,j,k;

   assert(n <= 32768);
   p->large_bmap = NULL;

   for(;;) {
      rblxstb__slot *bcount = (rblxstb__slot *) rblxstb_temp(buffer3, sizeof(*bcount) * bsize);
      unsigned short *bloc = (unsigned short *) rblxstb_temp(buffer5, sizeof(*bloc) * bsize);
      unsigned short *e;
      int bad=0;

      p->addend = rblxstb__perfect_rand();
      p->multiplicand = rblxstb__perfect_rand() | 1;
      p->table_mask = size-1;
      p->b_mask = bsize-1;
      p->table = (rblxstb_uint32 *) malloc(size * sizeof(*p->table));

      for (i=0; i < bsize; ++i) {
         bcount[i].b     = i;
         bcount[i].count = 0;
         bcount[i].map   = 0;
      }
      for (i=0; i < n; ++i) {
         rblxstb__perfect_prehash(p, v[i], as+i, bs+i);
         ++bcount[bs[i]].count;
      }
      qsort(bcount, bsize, sizeof(*bcount), rblxstb__slot_compare);
      e = entries; // now setup up their entries index
      for (i=0; i < bsize; ++i) {
         bcount[i].entries = e;
         e += bcount[i].count;
         bcount[i].count = 0;
         bloc[bcount[i].b] = i;
      }
      // now fill them out
      for (i=0; i < n; ++i) {
         int b = bs[i];
         int w = bloc[b];
         bcount[w].entries[bcount[w].count++] = i;
      }
      rblxstb_tempfree(buffer5,bloc);
      // verify
      for (i=0; i < bsize; ++i)
         for (j=0; j < bcount[i].count; ++j)
            assert(bs[bcount[i].entries[j]] == bcount[i].b);
      memset(p->table, 0, size*sizeof(*p->table));

      // check if any b has duplicate a
      for (i=0; i < bsize; ++i) {
         if (bcount[i].count > 1) {
            for (j=0; j < bcount[i].count; ++j) {
               if (p->table[as[bcount[i].entries[j]]])
                  bad = 1;
               p->table[as[bcount[i].entries[j]]] = 1;
            }
            for (j=0; j < bcount[i].count; ++j) {
               p->table[as[bcount[i].entries[j]]] = 0;
            }
            if (bad) break;
         }
      }

      if (!bad) {
         // go through the bs and populate the table, first fit
         for (i=0; i < bsize; ++i) {
            if (bcount[i].count) {
               // go through the candidate table[b] values
               for (j=0; j < size; ++j) {
                  // go through the a values and see if they fit
                  for (k=0; k < bcount[i].count; ++k) {
                     int a = as[bcount[i].entries[k]];
                     if (p->table[(a^j)&p->table_mask]) {
                        break; // fails
                     }
                  }
                  // if succeeded, accept
                  if (k == bcount[i].count) {
                     bcount[i].map = j;
                     for (k=0; k < bcount[i].count; ++k) {
                        int a = as[bcount[i].entries[k]];
                        p->table[(a^j)&p->table_mask] = 1;
                     }
                     break;
                  }
               }
               if (j == size)
                  break; // no match for i'th entry, so break out in failure
            }
         }
         if (i == bsize) {
            // success... fill out map
            if (bsize <= 16 && size <= 256) {
               p->large_bmap = NULL;
               for (i=0; i < bsize; ++i)
                  p->small_bmap[bcount[i].b] = (rblxstb_uint8) bcount[i].map;
            } else {
               p->large_bmap = (unsigned short *) malloc(sizeof(*p->large_bmap) * bsize);
               for (i=0; i < bsize; ++i)
                  p->large_bmap[bcount[i].b] = bcount[i].map;
            }

            // initialize table to v[0], so empty slots will fail
            for (i=0; i < size; ++i)
               p->table[i] = v[0];

            for (i=0; i < n; ++i)
               if (p->large_bmap)
                  p->table[as[i] ^ p->large_bmap[bs[i]]] = v[i];
               else
                  p->table[as[i] ^ p->small_bmap[bs[i]]] = v[i];

            // and now validate that none of them collided
            for (i=0; i < n; ++i)
               assert(rblxstb_perfect_hash(p, v[i]) >= 0);

            rblxstb_tempfree(buffer3, bcount);
            break;
         }
      }
      free(p->table);
      p->table = NULL;
      rblxstb_tempfree(buffer3, bcount);

      ++failure;
      if (failure >= 4 && bsize < size) bsize *= 2;
      if (failure >= 8 && (failure & 3) == 0 && size < 4*n) {
         size *= 2;
         bsize *= 2;
      }
      if (failure == 6) {
         // make sure the input data is unique, so we don't infinite loop
         unsigned int *data = (unsigned int *) rblxstb_temp(buffer3, n * sizeof(*data));
         memcpy(data, v, sizeof(*data) * n);
         qsort(data, n, sizeof(*data), rblxstb_intcmp(0));
         for (i=1; i < n; ++i) {
            if (data[i] == data[i-1])
               size = 0; // size is return value, so 0 it
         }
         rblxstb_tempfree(buffer3, data);
         if (!size) break;
      }
   }

   if (failure > rblxstb_perfect_hash_max_failures)
      rblxstb_perfect_hash_max_failures = failure;

   rblxstb_tempfree(buffer1, as);
   rblxstb_tempfree(buffer2, bs);
   rblxstb_tempfree(buffer4, entries);

   return size;
}

void rblxstb_perfect_destroy(rblxstb_perfect *p)
{
   if (p->large_bmap) free(p->large_bmap);
   if (p->table     ) free(p->table);
   p->large_bmap = NULL;
   p->table      = NULL;
   p->b_mask     = 0;
   p->table_mask = 0;
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
//                     Perfect hash clients

RBLXSTB_EXTERN int    rblxstb_ischar(char s, char *set);

#ifdef RBLXSTB_DEFINE

int rblxstb_ischar(char c, char *set)
{
   static unsigned char bit[8] = { 1,2,4,8,16,32,64,128 };
   static rblxstb_perfect p;
   static unsigned char (*tables)[256];
   static char ** sets = NULL;

   int z = rblxstb_perfect_hash(&p, (int)(size_t) set);
   if (z < 0) {
      int i,k,n,j,f;
      // special code that means free all existing data
      if (set == NULL) {
         rblxstb_arr_free(sets);
         free(tables);
         tables = NULL;
         rblxstb_perfect_destroy(&p);
         return 0;
      }
      rblxstb_arr_push(sets, set);
      rblxstb_perfect_destroy(&p);
      n = rblxstb_perfect_create(&p, (unsigned int *) (char **) sets, rblxstb_arr_len(sets));
      assert(n != 0);
      k = (n+7) >> 3;
      tables = (unsigned char (*)[256]) realloc(tables, sizeof(*tables) * k);
      memset(tables, 0, sizeof(*tables) * k);
      for (i=0; i < rblxstb_arr_len(sets); ++i) {
          k = rblxstb_perfect_hash(&p, (int)(size_t) sets[i]);
         assert(k >= 0);
         n = k >> 3;
         f = bit[k&7];
         for (j=0; !j || sets[i][j]; ++j) {
            tables[n][(unsigned char) sets[i][j]] |= f;
         }
      }
      z = rblxstb_perfect_hash(&p, (int)(size_t) set);
   }
   return tables[z >> 3][(unsigned char) c] & bit[z & 7];
}

#endif
#endif

//////////////////////////////////////////////////////////////////////////////
//
//                     Instantiated data structures
//
// This is an attempt to implement a templated data structure.
//
// Hash table: call rblxstb_define_hash(TYPE,N,KEY,K1,K2,HASH,VALUE)
//     TYPE     -- will define a structure type containing the hash table
//     N        -- the name, will prefix functions named:
//                        N create
//                        N destroy
//                        N get
//                        N set, N add, N update,
//                        N remove
//     KEY      -- the type of the key. 'x == y' must be valid
//       K1,K2  -- keys never used by the app, used as flags in the hashtable
//       HASH   -- a piece of code ending with 'return' that hashes key 'k'
//     VALUE    -- the type of the value. 'x = y' must be valid
//
//  Note that rblxstb_define_hash_base can be used to define more sophisticated
//  hash tables, e.g. those that make copies of the key or use special
//  comparisons (e.g. strcmp).

#define RBLXSTB_(prefix,name)     rblxstb__##prefix##name
#define RBLXSTB__(prefix,name)    prefix##name
#define RBLXSTB__use(x)           x
#define RBLXSTB__skip(x)

#define rblxstb_declare_hash(PREFIX,TYPE,N,KEY,VALUE) \
   typedef struct rblxstb__st_##TYPE TYPE;\
   PREFIX int RBLXSTB__(N, init)(TYPE *h, int count);\
   PREFIX int RBLXSTB__(N, memory_usage)(TYPE *h);\
   PREFIX TYPE * RBLXSTB__(N, create)(void);\
   PREFIX TYPE * RBLXSTB__(N, copy)(TYPE *h);\
   PREFIX void RBLXSTB__(N, destroy)(TYPE *h);\
   PREFIX int RBLXSTB__(N,get_flag)(TYPE *a, KEY k, VALUE *v);\
   PREFIX VALUE RBLXSTB__(N,get)(TYPE *a, KEY k);\
   PREFIX int RBLXSTB__(N, set)(TYPE *a, KEY k, VALUE v);\
   PREFIX int RBLXSTB__(N, add)(TYPE *a, KEY k, VALUE v);\
   PREFIX int RBLXSTB__(N, update)(TYPE*a,KEY k,VALUE v);\
   PREFIX int RBLXSTB__(N, remove)(TYPE *a, KEY k, VALUE *v);

#define RBLXSTB_nocopy(x)        (x)
#define RBLXSTB_nodelete(x)      0
#define RBLXSTB_nofields
#define RBLXSTB_nonullvalue(x)
#define RBLXSTB_nullvalue(x)     x
#define RBLXSTB_safecompare(x)   x
#define RBLXSTB_nosafe(x)
#define RBLXSTB_noprefix

#ifdef __GNUC__
#define RBLXSTB__nogcc(x)
#else
#define RBLXSTB__nogcc(x)  x
#endif

#define rblxstb_define_hash_base(PREFIX,TYPE,FIELDS,N,NC,LOAD_FACTOR,             \
                             KEY,EMPTY,DEL,COPY,DISPOSE,SAFE,                 \
                             VCOMPARE,CCOMPARE,HASH,                          \
                             VALUE,HASVNULL,VNULL)                            \
                                                                              \
typedef struct                                                                \
{                                                                             \
   KEY   k;                                                                   \
   VALUE v;                                                                   \
} RBLXSTB_(N,_hashpair);                                                          \
                                                                              \
RBLXSTB__nogcc( typedef struct rblxstb__st_##TYPE TYPE;  )                            \
struct rblxstb__st_##TYPE {                                                       \
   FIELDS                                                                     \
   RBLXSTB_(N,_hashpair) *table;                                                  \
   unsigned int mask;                                                         \
   int count, limit;                                                          \
   int deleted;                                                               \
                                                                              \
   int delete_threshhold;                                                     \
   int grow_threshhold;                                                       \
   int shrink_threshhold;                                                     \
   unsigned char alloced, has_empty, has_del;                                 \
   VALUE ev; VALUE dv;                                                        \
};                                                                            \
                                                                              \
static unsigned int RBLXSTB_(N, hash)(KEY k)                                      \
{                                                                             \
   HASH                                                                       \
}                                                                             \
                                                                              \
PREFIX int RBLXSTB__(N, init)(TYPE *h, int count)                                        \
{                                                                             \
   int i;                                                                     \
   if (count < 4) count = 4;                                                  \
   h->limit = count;                                                          \
   h->count = 0;                                                              \
   h->mask  = count-1;                                                        \
   h->deleted = 0;                                                            \
   h->grow_threshhold = (int) (count * LOAD_FACTOR);                          \
   h->has_empty = h->has_del = 0;                                             \
   h->alloced = 0;                                                            \
   if (count <= 64)                                                           \
      h->shrink_threshhold = 0;                                               \
   else                                                                       \
      h->shrink_threshhold = (int) (count * (LOAD_FACTOR/2.25));              \
   h->delete_threshhold = (int) (count * (1-LOAD_FACTOR)/2);                  \
   h->table = (RBLXSTB_(N,_hashpair)*) malloc(sizeof(h->table[0]) * count);       \
   if (h->table == NULL) return 0;                                            \
   /* ideally this gets turned into a memset32 automatically */               \
   for (i=0; i < count; ++i)                                                  \
      h->table[i].k = EMPTY;                                                  \
   return 1;                                                                  \
}                                                                             \
                                                                              \
PREFIX int RBLXSTB__(N, memory_usage)(TYPE *h)                                           \
{                                                                             \
   return sizeof(*h) + h->limit * sizeof(h->table[0]);                        \
}                                                                             \
                                                                              \
PREFIX TYPE * RBLXSTB__(N, create)(void)                                                 \
{                                                                             \
   TYPE *h = (TYPE *) malloc(sizeof(*h));                                     \
   if (h) {                                                                   \
      if (RBLXSTB__(N, init)(h, 16))                                              \
         h->alloced = 1;                                                      \
      else { free(h); h=NULL; }                                               \
   }                                                                          \
   return h;                                                                  \
}                                                                             \
                                                                              \
PREFIX void RBLXSTB__(N, destroy)(TYPE *a)                                               \
{                                                                             \
   int i;                                                                     \
   for (i=0; i < a->limit; ++i)                                               \
      if (!CCOMPARE(a->table[i].k,EMPTY) && !CCOMPARE(a->table[i].k, DEL))    \
         DISPOSE(a->table[i].k);                                              \
   free(a->table);                                                            \
   if (a->alloced)                                                            \
      free(a);                                                                \
}                                                                             \
                                                                              \
static void RBLXSTB_(N, rehash)(TYPE *a, int count);                              \
                                                                              \
PREFIX int RBLXSTB__(N,get_flag)(TYPE *a, KEY k, VALUE *v)                               \
{                                                                             \
   unsigned int h = RBLXSTB_(N, hash)(k);                                         \
   unsigned int n = h & a->mask, s;                                           \
   if (CCOMPARE(k,EMPTY)){ if (a->has_empty) *v = a->ev; return a->has_empty;}\
   if (CCOMPARE(k,DEL)) { if (a->has_del  ) *v = a->dv; return a->has_del;   }\
   if (CCOMPARE(a->table[n].k,EMPTY)) return 0;                               \
   SAFE(if (!CCOMPARE(a->table[n].k,DEL)))                                    \
   if (VCOMPARE(a->table[n].k,k)) { *v = a->table[n].v; return 1; }            \
   s = rblxstb_rehash(h) | 1;                                                     \
   for(;;) {                                                                  \
      n = (n + s) & a->mask;                                                  \
      if (CCOMPARE(a->table[n].k,EMPTY)) return 0;                            \
      SAFE(if (CCOMPARE(a->table[n].k,DEL)) continue;)                        \
      if (VCOMPARE(a->table[n].k,k))                                           \
         { *v = a->table[n].v; return 1; }                                    \
   }                                                                          \
}                                                                             \
                                                                              \
HASVNULL(                                                                     \
   PREFIX VALUE RBLXSTB__(N,get)(TYPE *a, KEY k)                                         \
   {                                                                          \
      VALUE v;                                                                \
      if (RBLXSTB__(N,get_flag)(a,k,&v)) return v;                                \
      else                           return VNULL;                            \
   }                                                                          \
)                                                                             \
                                                                              \
PREFIX int RBLXSTB__(N,getkey)(TYPE *a, KEY k, KEY *kout)                                \
{                                                                             \
   unsigned int h = RBLXSTB_(N, hash)(k);                                         \
   unsigned int n = h & a->mask, s;                                           \
   if (CCOMPARE(k,EMPTY)||CCOMPARE(k,DEL)) return 0;                          \
   if (CCOMPARE(a->table[n].k,EMPTY)) return 0;                               \
   SAFE(if (!CCOMPARE(a->table[n].k,DEL)))                                    \
   if (VCOMPARE(a->table[n].k,k)) { *kout = a->table[n].k; return 1; }         \
   s = rblxstb_rehash(h) | 1;                                                     \
   for(;;) {                                                                  \
      n = (n + s) & a->mask;                                                  \
      if (CCOMPARE(a->table[n].k,EMPTY)) return 0;                            \
      SAFE(if (CCOMPARE(a->table[n].k,DEL)) continue;)                        \
      if (VCOMPARE(a->table[n].k,k))                                          \
         { *kout = a->table[n].k; return 1; }                                 \
   }                                                                          \
}                                                                             \
                                                                              \
static int RBLXSTB_(N,addset)(TYPE *a, KEY k, VALUE v,                            \
                             int allow_new, int allow_old, int copy)          \
{                                                                             \
   unsigned int h = RBLXSTB_(N, hash)(k);                                         \
   unsigned int n = h & a->mask;                                              \
   int b = -1;                                                                \
   if (CCOMPARE(k,EMPTY)) {                                                   \
      if (a->has_empty ? allow_old : allow_new) {                             \
          n=a->has_empty; a->ev = v; a->has_empty = 1; return !n;             \
      } else return 0;                                                        \
   }                                                                          \
   if (CCOMPARE(k,DEL)) {                                                     \
      if (a->has_del ? allow_old : allow_new) {                               \
          n=a->has_del; a->dv = v; a->has_del = 1; return !n;                 \
      } else return 0;                                                        \
   }                                                                          \
   if (!CCOMPARE(a->table[n].k, EMPTY)) {                                     \
      unsigned int s;                                                         \
      if (CCOMPARE(a->table[n].k, DEL))                                       \
         b = n;                                                               \
      else if (VCOMPARE(a->table[n].k,k)) {                                   \
         if (allow_old)                                                       \
            a->table[n].v = v;                                                \
         return !allow_new;                                                   \
      }                                                                       \
      s = rblxstb_rehash(h) | 1;                                                  \
      for(;;) {                                                               \
         n = (n + s) & a->mask;                                               \
         if (CCOMPARE(a->table[n].k, EMPTY)) break;                           \
         if (CCOMPARE(a->table[n].k, DEL)) {                                  \
            if (b < 0) b = n;                                                 \
         } else if (VCOMPARE(a->table[n].k,k)) {                              \
            if (allow_old)                                                    \
               a->table[n].v = v;                                             \
            return !allow_new;                                                \
         }                                                                    \
      }                                                                       \
   }                                                                          \
   if (!allow_new) return 0;                                                  \
   if (b < 0) b = n; else --a->deleted;                                       \
   a->table[b].k = copy ? COPY(k) : k;                                        \
   a->table[b].v = v;                                                         \
   ++a->count;                                                                \
   if (a->count > a->grow_threshhold)                                         \
      RBLXSTB_(N,rehash)(a, a->limit*2);                                          \
   return 1;                                                                  \
}                                                                             \
                                                                              \
PREFIX int RBLXSTB__(N, set)(TYPE *a, KEY k, VALUE v){return RBLXSTB_(N,addset)(a,k,v,1,1,1);}\
PREFIX int RBLXSTB__(N, add)(TYPE *a, KEY k, VALUE v){return RBLXSTB_(N,addset)(a,k,v,1,0,1);}\
PREFIX int RBLXSTB__(N, update)(TYPE*a,KEY k,VALUE v){return RBLXSTB_(N,addset)(a,k,v,0,1,1);}\
                                                                              \
PREFIX int RBLXSTB__(N, remove)(TYPE *a, KEY k, VALUE *v)                                \
{                                                                             \
   unsigned int h = RBLXSTB_(N, hash)(k);                                         \
   unsigned int n = h & a->mask, s;                                           \
   if (CCOMPARE(k,EMPTY)) { if (a->has_empty) { if(v)*v = a->ev; a->has_empty=0; return 1; } return 0; } \
   if (CCOMPARE(k,DEL))   { if (a->has_del  ) { if(v)*v = a->dv; a->has_del  =0; return 1; } return 0; } \
   if (CCOMPARE(a->table[n].k,EMPTY)) return 0;                               \
   if (SAFE(CCOMPARE(a->table[n].k,DEL) || ) !VCOMPARE(a->table[n].k,k)) {     \
      s = rblxstb_rehash(h) | 1;                                                  \
      for(;;) {                                                               \
         n = (n + s) & a->mask;                                               \
         if (CCOMPARE(a->table[n].k,EMPTY)) return 0;                         \
         SAFE(if (CCOMPARE(a->table[n].k, DEL)) continue;)                    \
         if (VCOMPARE(a->table[n].k,k)) break;                                 \
      }                                                                       \
   }                                                                          \
   DISPOSE(a->table[n].k);                                                    \
   a->table[n].k = DEL;                                                       \
   --a->count;                                                                \
   ++a->deleted;                                                              \
   if (v != NULL)                                                             \
      *v = a->table[n].v;                                                     \
   if (a->count < a->shrink_threshhold)                                       \
      RBLXSTB_(N, rehash)(a, a->limit >> 1);                                      \
   else if (a->deleted > a->delete_threshhold)                                \
      RBLXSTB_(N, rehash)(a, a->limit);                                           \
   return 1;                                                                  \
}                                                                             \
                                                                              \
PREFIX TYPE * RBLXSTB__(NC, copy)(TYPE *a)                                        \
{                                                                             \
   int i;                                                                     \
   TYPE *h = (TYPE *) malloc(sizeof(*h));                                     \
   if (!h) return NULL;                                                       \
   if (!RBLXSTB__(N, init)(h, a->limit)) { free(h); return NULL; }                \
   h->count = a->count;                                                       \
   h->deleted = a->deleted;                                                   \
   h->alloced = 1;                                                            \
   h->ev = a->ev; h->dv = a->dv;                                              \
   h->has_empty = a->has_empty; h->has_del = a->has_del;                      \
   memcpy(h->table, a->table, h->limit * sizeof(h->table[0]));                \
   for (i=0; i < a->limit; ++i)                                               \
      if (!CCOMPARE(h->table[i].k,EMPTY) && !CCOMPARE(h->table[i].k,DEL))     \
         h->table[i].k = COPY(h->table[i].k);                                 \
   return h;                                                                  \
}                                                                             \
                                                                              \
static void RBLXSTB_(N, rehash)(TYPE *a, int count)                               \
{                                                                             \
   int i;                                                                     \
   TYPE b;                                                                    \
   RBLXSTB__(N, init)(&b, count);                                                 \
   for (i=0; i < a->limit; ++i)                                               \
      if (!CCOMPARE(a->table[i].k,EMPTY) && !CCOMPARE(a->table[i].k,DEL))     \
         RBLXSTB_(N,addset)(&b, a->table[i].k, a->table[i].v,1,1,0);              \
   free(a->table);                                                            \
   a->table = b.table;                                                        \
   a->mask = b.mask;                                                          \
   a->count = b.count;                                                        \
   a->limit = b.limit;                                                        \
   a->deleted = b.deleted;                                                    \
   a->delete_threshhold = b.delete_threshhold;                                \
   a->grow_threshhold = b.grow_threshhold;                                    \
   a->shrink_threshhold = b.shrink_threshhold;                                \
}

#define RBLXSTB_equal(a,b)  ((a) == (b))

#define rblxstb_define_hash(TYPE,N,KEY,EMPTY,DEL,HASH,VALUE)                      \
   rblxstb_define_hash_base(RBLXSTB_noprefix, TYPE,RBLXSTB_nofields,N,NC,0.85f,           \
              KEY,EMPTY,DEL,RBLXSTB_nocopy,RBLXSTB_nodelete,RBLXSTB_nosafe,               \
              RBLXSTB_equal,RBLXSTB_equal,HASH,                                       \
              VALUE,RBLXSTB_nonullvalue,0)

#define rblxstb_define_hash_vnull(TYPE,N,KEY,EMPTY,DEL,HASH,VALUE,VNULL)          \
   rblxstb_define_hash_base(RBLXSTB_noprefix, TYPE,RBLXSTB_nofields,N,NC,0.85f,           \
              KEY,EMPTY,DEL,RBLXSTB_nocopy,RBLXSTB_nodelete,RBLXSTB_nosafe,               \
              RBLXSTB_equal,RBLXSTB_equal,HASH,                                       \
              VALUE,RBLXSTB_nullvalue,VNULL)

//////////////////////////////////////////////////////////////////////////////
//
//                        rblxstb_ptrmap
//
// An rblxstb_ptrmap data structure is an O(1) hash table between pointers. One
// application is to let you store "extra" data associated with pointers,
// which is why it was originally called rblxstb_extra.

rblxstb_declare_hash(RBLXSTB_EXTERN, rblxstb_ptrmap, rblxstb_ptrmap_, void *, void *)
rblxstb_declare_hash(RBLXSTB_EXTERN, rblxstb_idict, rblxstb_idict_, rblxstb_int32, rblxstb_int32)
rblxstb_declare_hash(RBLXSTB_EXTERN, rblxstb_uidict, rblxstbi_uidict_, rblxstb_uint32, rblxstb_uint32)

RBLXSTB_EXTERN void        rblxstb_ptrmap_delete(rblxstb_ptrmap *e, void (*free_func)(void *));
RBLXSTB_EXTERN rblxstb_ptrmap *rblxstb_ptrmap_new(void);

RBLXSTB_EXTERN rblxstb_idict * rblxstb_idict_new_size(int size);
RBLXSTB_EXTERN void        rblxstb_idict_remove_all(rblxstb_idict *e);
RBLXSTB_EXTERN void        rblxstb_uidict_reset(rblxstb_uidict *e);

#ifdef RBLXSTB_DEFINE

#define RBLXSTB_EMPTY ((void *) 2)
#define RBLXSTB_EDEL  ((void *) 6)

rblxstb_define_hash_base(RBLXSTB_noprefix,rblxstb_ptrmap, RBLXSTB_nofields, rblxstb_ptrmap_,rblxstb_ptrmap_,0.85f,
              void *,RBLXSTB_EMPTY,RBLXSTB_EDEL,RBLXSTB_nocopy,RBLXSTB_nodelete,RBLXSTB_nosafe,
              RBLXSTB_equal,RBLXSTB_equal,return rblxstb_hashptr(k);,
              void *,RBLXSTB_nullvalue,NULL)

rblxstb_ptrmap *rblxstb_ptrmap_new(void)
{
   return rblxstb_ptrmap_create();
}

void rblxstb_ptrmap_delete(rblxstb_ptrmap *e, void (*free_func)(void *))
{
   int i;
   if (free_func)
      for (i=0; i < e->limit; ++i)
         if (e->table[i].k != RBLXSTB_EMPTY && e->table[i].k != RBLXSTB_EDEL) {
            if (free_func == free)
               free(e->table[i].v); // allow RBLXSTB_MALLOC_WRAPPER to operate
            else
               free_func(e->table[i].v);
         }
   rblxstb_ptrmap_destroy(e);
}

// extra fields needed for stua_dict
#define RBLXSTB_IEMPTY  ((int) 1)
#define RBLXSTB_IDEL    ((int) 3)
rblxstb_define_hash_base(RBLXSTB_noprefix, rblxstb_idict, short type; short gc; RBLXSTB_nofields, rblxstb_idict_,rblxstb_idict_,0.95f,
              rblxstb_int32,RBLXSTB_IEMPTY,RBLXSTB_IDEL,RBLXSTB_nocopy,RBLXSTB_nodelete,RBLXSTB_nosafe,
              RBLXSTB_equal,RBLXSTB_equal,
              return rblxstb_rehash_improved(k);,rblxstb_int32,RBLXSTB_nonullvalue,0)

rblxstb_idict * rblxstb_idict_new_size(int size)
{
   rblxstb_idict *e = (rblxstb_idict *) malloc(sizeof(*e));
   if (e) {
      if (!rblxstb_is_pow2(size))
         size = 1 << rblxstb_log2_ceil(size);
      rblxstb_idict_init(e, size);
      e->alloced = 1;
   }
   return e;
}

void rblxstb_idict_remove_all(rblxstb_idict *e)
{
   int n;
   for (n=0; n < e->limit; ++n)
      e->table[n].k = RBLXSTB_IEMPTY;
   e->has_empty = e->has_del = 0;
   e->count = 0;
   e->deleted = 0;
}

rblxstb_define_hash_base(RBLXSTB_noprefix, rblxstb_uidict, RBLXSTB_nofields, rblxstb_uidict_,rblxstb_uidict_,0.85f,
              rblxstb_int32,0xffffffff,0xfffffffe,RBLXSTB_nocopy,RBLXSTB_nodelete,RBLXSTB_nosafe,
              RBLXSTB_equal,RBLXSTB_equal,
              return rblxstb_rehash_improved(k);,rblxstb_uint32,RBLXSTB_nonullvalue,0)

void rblxstb_uidict_reset(rblxstb_uidict *e)
{
   int n;
   for (n=0; n < e->limit; ++n)
      e->table[n].k = 0xffffffff;
   e->has_empty = e->has_del = 0;
   e->count = 0;
   e->deleted = 0;
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
//                        rblxstb_sparse_ptr_matrix
//
// An rblxstb_ptrmap data structure is an O(1) hash table storing an arbitrary
// block of data for a given pair of pointers.
//
// If create=0, returns

typedef struct rblxstb__st_stb_spmatrix rblxstb_spmatrix;

RBLXSTB_EXTERN rblxstb_spmatrix * rblxstb_sparse_ptr_matrix_new(int val_size);
RBLXSTB_EXTERN void           rblxstb_sparse_ptr_matrix_free(rblxstb_spmatrix *z);
RBLXSTB_EXTERN void         * rblxstb_sparse_ptr_matrix_get(rblxstb_spmatrix *z, void *a, void *b, int create);

#ifdef RBLXSTB_DEFINE
typedef struct
{
   void *a;
   void *b;
} rblxstb__ptrpair;

static rblxstb__ptrpair rblxstb__ptrpair_empty = { (void *) 1, (void *) 1 };
static rblxstb__ptrpair rblxstb__ptrpair_del   = { (void *) 2, (void *) 2 };

#define RBLXSTB__equal_ptrpair(x,y) ((x).a == (y).a && (x).b == (y).b)

rblxstb_define_hash_base(RBLXSTB_noprefix, rblxstb_spmatrix, int val_size; void *arena;, rblxstb__spmatrix_,rblxstb__spmatrix_, 0.85,
     rblxstb__ptrpair, rblxstb__ptrpair_empty, rblxstb__ptrpair_del,
     RBLXSTB_nocopy, RBLXSTB_nodelete, RBLXSTB_nosafe,
     RBLXSTB__equal_ptrpair, RBLXSTB__equal_ptrpair, return rblxstb_rehash(rblxstb_hashptr(k.a))+rblxstb_hashptr(k.b);,
     void *, RBLXSTB_nullvalue, 0)

rblxstb_spmatrix *rblxstb_sparse_ptr_matrix_new(int val_size)
{
   rblxstb_spmatrix *m = rblxstb__spmatrix_create();
   if (m) m->val_size = val_size;
   if (m) m->arena = rblxstb_malloc_global(1);
   return m;
}

void rblxstb_sparse_ptr_matrix_free(rblxstb_spmatrix *z)
{
   if (z->arena) rblxstb_free(z->arena);
   rblxstb__spmatrix_destroy(z);
}

void *rblxstb_sparse_ptr_matrix_get(rblxstb_spmatrix *z, void *a, void *b, int create)
{
   rblxstb__ptrpair t = { a,b };
   void *data = rblxstb__spmatrix_get(z, t);
   if (!data && create) {
      data = rblxstb_malloc_raw(z->arena, z->val_size);
      if (!data) return NULL;
      memset(data, 0, z->val_size);
      rblxstb__spmatrix_add(z, t, data);
   }
   return data;
}
#endif



//////////////////////////////////////////////////////////////////////////////
//
//                  SDICT: Hash Table for Strings (symbol table)
//
//           if "use_arena=1", then strings will be copied
//           into blocks and never freed until the sdict is freed;
//           otherwise they're malloc()ed and free()d on the fly.
//           (specify use_arena=1 if you never rblxstb_sdict_remove)

rblxstb_declare_hash(RBLXSTB_EXTERN, rblxstb_sdict, rblxstb_sdict_, char *, void *)

RBLXSTB_EXTERN rblxstb_sdict * rblxstb_sdict_new(int use_arena);
RBLXSTB_EXTERN rblxstb_sdict * rblxstb_sdict_copy(rblxstb_sdict*);
RBLXSTB_EXTERN void        rblxstb_sdict_delete(rblxstb_sdict *);
RBLXSTB_EXTERN void *      rblxstb_sdict_change(rblxstb_sdict *, char *str, void *p);
RBLXSTB_EXTERN int         rblxstb_sdict_count(rblxstb_sdict *d);

RBLXSTB_EXTERN int         rblxstb_sdict_internal_limit(rblxstb_sdict *d);
RBLXSTB_EXTERN char *      rblxstb_sdict_internal_key(rblxstb_sdict *d, int n);
RBLXSTB_EXTERN void *      rblxstb_sdict_internal_value(rblxstb_sdict *d, int n);

#define rblxstb_sdict_for(d,i,q,z)                                          \
   for(i=0; i < rblxstb_sdict_internal_limit(d) ? (q=rblxstb_sdict_internal_key(d,i),z=rblxstb_sdict_internal_value(d,i),1) : 0; ++i)    \
      if (q==NULL||q==(void *) 1);else   // reversed makes macro friendly

#ifdef RBLXSTB_DEFINE

// if in same translation unit, for speed, don't call accessors
#undef rblxstb_sdict_for
#define rblxstb_sdict_for(d,i,q,z)                                          \
   for(i=0; i < (d)->limit ? (q=(d)->table[i].k,z=(d)->table[i].v,1) : 0; ++i)    \
      if (q==NULL||q==(void *) 1);else   // reversed makes macro friendly

#define RBLXSTB_DEL ((void *) 1)
#define RBLXSTB_SDEL  ((char *) 1)

#define rblxstb_sdict__copy(x)                                             \
   rblxstb_p_strcpy_s(a->arena ? rblxstb_malloc_string(a->arena, strlen(x)+1)    \
                         : (char *) malloc(strlen(x)+1), strlen(x)+1, x)

#define rblxstb_sdict__dispose(x)  if (!a->arena) free(x)

rblxstb_define_hash_base(RBLXSTB_noprefix, rblxstb_sdict, void*arena;, rblxstb_sdict_,rblxstb_sdictinternal_, 0.85f,
        char *, NULL, RBLXSTB_SDEL, rblxstb_sdict__copy, rblxstb_sdict__dispose,
                        RBLXSTB_safecompare, !strcmp, RBLXSTB_equal, return rblxstb_hash(k);,
        void *, RBLXSTB_nullvalue, NULL)

int rblxstb_sdict_count(rblxstb_sdict *a)
{
   return a->count;
}

int rblxstb_sdict_internal_limit(rblxstb_sdict *a)
{
   return a->limit;
}
char* rblxstb_sdict_internal_key(rblxstb_sdict *a, int n)
{
   return a->table[n].k;
}
void* rblxstb_sdict_internal_value(rblxstb_sdict *a, int n)
{
   return a->table[n].v;
}

rblxstb_sdict * rblxstb_sdict_new(int use_arena)
{
   rblxstb_sdict *d = rblxstb_sdict_create();
   if (d == NULL) return NULL;
   d->arena = use_arena ? rblxstb_malloc_global(1) : NULL;
   return d;
}

rblxstb_sdict* rblxstb_sdict_copy(rblxstb_sdict *old)
{
   rblxstb_sdict *n;
   void *old_arena = old->arena;
   void *new_arena = old_arena ? rblxstb_malloc_global(1) : NULL;
   old->arena = new_arena;
   n = rblxstb_sdictinternal_copy(old);
   old->arena = old_arena;
   if (n)
      n->arena = new_arena;
   else if (new_arena)
      rblxstb_free(new_arena);
   return n;
}


void rblxstb_sdict_delete(rblxstb_sdict *d)
{
   if (d->arena)
      rblxstb_free(d->arena);
   rblxstb_sdict_destroy(d);
}

void * rblxstb_sdict_change(rblxstb_sdict *d, char *str, void *p)
{
   void *q = rblxstb_sdict_get(d, str);
   rblxstb_sdict_set(d, str, p);
   return q;
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
//                     Instantiated data structures
//
// This is an attempt to implement a templated data structure.
// What you do is define a struct foo, and then include several
// pointer fields to struct foo in your struct. Then you call
// the instantiator, which creates the functions that implement
// the data structure. This requires massive undebuggable #defines,
// so we limit the cases where we do this.
//
// AA tree is an encoding of a 2-3 tree whereas RB trees encode a 2-3-4 tree;
// much simpler code due to fewer cases.

#define rblxstb__bst_parent(x)    x
#define rblxstb__bst_noparent(x)

#define rblxstb_bst_fields(N)                                   \
    *RBLXSTB_(N,left), *RBLXSTB_(N,right);                          \
    unsigned char RBLXSTB_(N,level)

#define rblxstb_bst_fields_parent(N)                            \
    *RBLXSTB_(N,left), *RBLXSTB_(N,right),  *RBLXSTB_(N,parent);        \
    unsigned char RBLXSTB_(N,level)

#define RBLXSTB__level(N,x)         ((x) ? (x)->RBLXSTB_(N,level) : 0)

#define rblxstb_bst_base(TYPE, N, TREE, M, compare, PAR)                         \
                                                                             \
static int RBLXSTB_(N,_compare)(TYPE *p, TYPE *q)                                \
{                                                                            \
   compare                                                                   \
}                                                                            \
                                                                             \
static void RBLXSTB_(N,setleft)(TYPE *q, TYPE *v)                                \
{                                                                            \
   q->RBLXSTB_(N,left) = v;                                                      \
   PAR(if (v) v->RBLXSTB_(N,parent) = q;)                                        \
}                                                                            \
                                                                             \
static void RBLXSTB_(N,setright)(TYPE *q, TYPE *v)                               \
{                                                                            \
   q->RBLXSTB_(N,right) = v;                                                     \
   PAR(if (v) v->RBLXSTB_(N,parent) = q;)                                        \
}                                                                            \
                                                                             \
static TYPE *RBLXSTB_(N,skew)(TYPE *q)                                           \
{                                                                            \
   if (q == NULL) return q;                                                  \
   if (q->RBLXSTB_(N,left)                                                       \
        && q->RBLXSTB_(N,left)->RBLXSTB_(N,level) == q->RBLXSTB_(N,level)) {             \
      TYPE *p       = q->RBLXSTB_(N,left);                                       \
      RBLXSTB_(N,setleft)(q, p->RBLXSTB_(N,right));                                  \
      RBLXSTB_(N,setright)(p, q);                                                \
      return p;                                                              \
   }                                                                         \
   return q;                                                                 \
}                                                                            \
                                                                             \
static TYPE *RBLXSTB_(N,split)(TYPE *p)                                          \
{                                                                            \
   TYPE *q = p->RBLXSTB_(N,right);                                               \
   if (q && q->RBLXSTB_(N,right)                                                 \
         && q->RBLXSTB_(N,right)->RBLXSTB_(N,level) == p->RBLXSTB_(N,level)) {           \
      RBLXSTB_(N,setright)(p, q->RBLXSTB_(N,left));                                  \
      RBLXSTB_(N,setleft)(q,p);                                                  \
      ++q->RBLXSTB_(N,level);                                                    \
      return q;                                                              \
   }                                                                         \
   return p;                                                                 \
}                                                                            \
                                                                             \
TYPE *RBLXSTB__(N,insert)(TYPE *tree, TYPE *item)                                \
{                                                                            \
   int c;                                                                    \
   if (tree == NULL) {                                                       \
      item->RBLXSTB_(N,left) = NULL;                                             \
      item->RBLXSTB_(N,right) = NULL;                                            \
      item->RBLXSTB_(N,level) = 1;                                               \
      PAR(item->RBLXSTB_(N,parent) = NULL;)                                      \
      return item;                                                           \
   }                                                                         \
   c = RBLXSTB_(N,_compare)(item,tree);                                          \
   if (c == 0) {                                                             \
      if (item != tree) {                                                    \
         RBLXSTB_(N,setleft)(item, tree->RBLXSTB_(N,left));                          \
         RBLXSTB_(N,setright)(item, tree->RBLXSTB_(N,right));                        \
         item->RBLXSTB_(N,level) = tree->RBLXSTB_(N,level);                          \
         PAR(item->RBLXSTB_(N,parent) = NULL;)                                   \
      }                                                                      \
      return item;                                                           \
   }                                                                         \
   if (c < 0)                                                                \
      RBLXSTB_(N,setleft )(tree, RBLXSTB__(N,insert)(tree->RBLXSTB_(N,left), item));     \
   else                                                                      \
      RBLXSTB_(N,setright)(tree, RBLXSTB__(N,insert)(tree->RBLXSTB_(N,right), item));    \
   tree = RBLXSTB_(N,skew)(tree);                                                \
   tree = RBLXSTB_(N,split)(tree);                                               \
   PAR(tree->RBLXSTB_(N,parent) = NULL;)                                         \
   return tree;                                                              \
}                                                                            \
                                                                             \
TYPE *RBLXSTB__(N,remove)(TYPE *tree, TYPE *item)                                \
{                                                                            \
   static TYPE *delnode, *leaf, *restore;                                    \
   if (tree == NULL) return NULL;                                            \
   leaf = tree;                                                              \
   if (RBLXSTB_(N,_compare)(item, tree) < 0) {                                   \
      RBLXSTB_(N,setleft)(tree, RBLXSTB__(N,remove)(tree->RBLXSTB_(N,left), item));      \
   } else {                                                                  \
      TYPE *r;                                                               \
      delnode = tree;                                                        \
      r = RBLXSTB__(N,remove)(tree->RBLXSTB_(N,right), item);                        \
      /* maybe move 'leaf' up to this location */                            \
      if (restore == tree) { tree = leaf; leaf = restore = NULL;  }          \
      RBLXSTB_(N,setright)(tree,r);                                              \
      assert(tree->RBLXSTB_(N,right) != tree);                                   \
   }                                                                         \
   if (tree == leaf) {                                                       \
      if (delnode == item) {                                                 \
         tree = tree->RBLXSTB_(N,right);                                         \
         assert(leaf->RBLXSTB_(N,left) == NULL);                                 \
         /* move leaf (the right sibling) up to delnode */                   \
         RBLXSTB_(N,setleft )(leaf, item->RBLXSTB_(N,left ));                        \
         RBLXSTB_(N,setright)(leaf, item->RBLXSTB_(N,right));                        \
         leaf->RBLXSTB_(N,level) = item->RBLXSTB_(N,level);                          \
         if (leaf != item)                                                   \
            restore = delnode;                                               \
      }                                                                      \
      delnode = NULL;                                                        \
   } else {                                                                  \
      if (RBLXSTB__level(N,tree->RBLXSTB_(N,left) ) < tree->RBLXSTB_(N,level)-1 ||       \
          RBLXSTB__level(N,tree->RBLXSTB_(N,right)) < tree->RBLXSTB_(N,level)-1) {       \
         --tree->RBLXSTB_(N,level);                                              \
         if (RBLXSTB__level(N,tree->RBLXSTB_(N,right)) > tree->RBLXSTB_(N,level))        \
            tree->RBLXSTB_(N,right)->RBLXSTB_(N,level) = tree->RBLXSTB_(N,level);        \
         tree = RBLXSTB_(N,skew)(tree);                                          \
         RBLXSTB_(N,setright)(tree, RBLXSTB_(N,skew)(tree->RBLXSTB_(N,right)));          \
         if (tree->RBLXSTB_(N,right))                                            \
            RBLXSTB_(N,setright)(tree->RBLXSTB_(N,right),                            \
                  RBLXSTB_(N,skew)(tree->RBLXSTB_(N,right)->RBLXSTB_(N,right)));         \
         tree = RBLXSTB_(N,split)(tree);                                         \
         if (tree->RBLXSTB_(N,right))                                            \
            RBLXSTB_(N,setright)(tree,  RBLXSTB_(N,split)(tree->RBLXSTB_(N,right)));     \
      }                                                                      \
   }                                                                         \
   PAR(if (tree) tree->RBLXSTB_(N,parent) = NULL;)                               \
   return tree;                                                              \
}                                                                            \
                                                                             \
TYPE *RBLXSTB__(N,last)(TYPE *tree)                                              \
{                                                                            \
   if (tree)                                                                 \
      while (tree->RBLXSTB_(N,right)) tree = tree->RBLXSTB_(N,right);                \
   return tree;                                                              \
}                                                                            \
                                                                             \
TYPE *RBLXSTB__(N,first)(TYPE *tree)                                             \
{                                                                            \
   if (tree)                                                                 \
      while (tree->RBLXSTB_(N,left)) tree = tree->RBLXSTB_(N,left);                  \
   return tree;                                                              \
}                                                                            \
                                                                             \
TYPE *RBLXSTB__(N,next)(TYPE *tree, TYPE *item)                                  \
{                                                                            \
   TYPE *next = NULL;                                                        \
   if (item->RBLXSTB_(N,right))                                                  \
      return RBLXSTB__(N,first)(item->RBLXSTB_(N,right));                            \
   PAR(                                                                      \
      while(item->RBLXSTB_(N,parent)) {                                          \
         TYPE *up = item->RBLXSTB_(N,parent);                                    \
         if (up->RBLXSTB_(N,left) == item) return up;                            \
         item = up;                                                          \
      }                                                                      \
      return NULL;                                                           \
   )                                                                         \
   while (tree != item) {                                                    \
      if (RBLXSTB_(N,_compare)(item, tree) < 0) {                                \
         next = tree;                                                        \
         tree = tree->RBLXSTB_(N,left);                                          \
      } else {                                                               \
         tree = tree->RBLXSTB_(N,right);                                         \
      }                                                                      \
   }                                                                         \
   return next;                                                              \
}                                                                            \
                                                                             \
TYPE *RBLXSTB__(N,prev)(TYPE *tree, TYPE *item)                                  \
{                                                                            \
   TYPE *next = NULL;                                                        \
   if (item->RBLXSTB_(N,left))                                                   \
      return RBLXSTB__(N,last)(item->RBLXSTB_(N,left));                              \
   PAR(                                                                      \
      while(item->RBLXSTB_(N,parent)) {                                          \
         TYPE *up = item->RBLXSTB_(N,parent);                                    \
         if (up->RBLXSTB_(N,right) == item) return up;                           \
         item = up;                                                          \
      }                                                                      \
      return NULL;                                                           \
   )                                                                         \
   while (tree != item) {                                                    \
      if (RBLXSTB_(N,_compare)(item, tree) < 0) {                                \
         tree = tree->RBLXSTB_(N,left);                                          \
      } else {                                                               \
         next = tree;                                                        \
         tree = tree->RBLXSTB_(N,right);                                         \
      }                                                                      \
   }                                                                         \
   return next;                                                              \
}                                                                            \
                                                                             \
RBLXSTB__DEBUG(                                                                  \
   void RBLXSTB__(N,_validate)(TYPE *tree, int root)                             \
   {                                                                         \
      if (tree == NULL) return;                                              \
      PAR(if(root) assert(tree->RBLXSTB_(N,parent) == NULL);)                    \
      assert(RBLXSTB__level(N,tree->RBLXSTB_(N,left) ) == tree->RBLXSTB_(N,level)-1);    \
      assert(RBLXSTB__level(N,tree->RBLXSTB_(N,right)) <= tree->RBLXSTB_(N,level));      \
      assert(RBLXSTB__level(N,tree->RBLXSTB_(N,right)) >= tree->RBLXSTB_(N,level)-1);    \
      if (tree->RBLXSTB_(N,right)) {                                             \
        assert(RBLXSTB__level(N,tree->RBLXSTB_(N,right)->RBLXSTB_(N,right))              \
                                               !=    tree->RBLXSTB_(N,level));   \
        PAR(assert(tree->RBLXSTB_(N,right)->RBLXSTB_(N,parent) == tree);)            \
      }                                                                      \
      PAR(if(tree->RBLXSTB_(N,left)) assert(tree->RBLXSTB_(N,left)->RBLXSTB_(N,parent) == tree);) \
      RBLXSTB__(N,_validate)(tree->RBLXSTB_(N,left) ,0);                             \
      RBLXSTB__(N,_validate)(tree->RBLXSTB_(N,right),0);                             \
   }                                                                         \
)                                                                            \
                                                                             \
typedef struct                                                               \
{                                                                            \
   TYPE *root;                                                               \
} TREE;                                                                      \
                                                                             \
void  RBLXSTB__(M,Insert)(TREE *tree, TYPE *item)                                \
{ tree->root = RBLXSTB__(N,insert)(tree->root, item); }                          \
void  RBLXSTB__(M,Remove)(TREE *tree, TYPE *item)                                \
{ tree->root = RBLXSTB__(N,remove)(tree->root, item); }                          \
TYPE *RBLXSTB__(M,Next)(TREE *tree, TYPE *item)                                  \
{ return RBLXSTB__(N,next)(tree->root, item); }                                  \
TYPE *RBLXSTB__(M,Prev)(TREE *tree, TYPE *item)                                  \
{ return RBLXSTB__(N,prev)(tree->root, item); }                                  \
TYPE *RBLXSTB__(M,First)(TREE *tree) { return RBLXSTB__(N,first)(tree->root); }      \
TYPE *RBLXSTB__(M,Last) (TREE *tree) { return RBLXSTB__(N,last) (tree->root); }      \
void RBLXSTB__(M,Init)(TREE *tree) { tree->root = NULL; }


#define rblxstb_bst_find(N,tree,fcompare)                                        \
{                                                                            \
   int c;                                                                    \
   while (tree != NULL) {                                                    \
      fcompare                                                               \
      if (c == 0) return tree;                                               \
      if (c < 0)  tree = tree->RBLXSTB_(N,left);                                 \
      else        tree = tree->RBLXSTB_(N,right);                                \
   }                                                                         \
   return NULL;                                                              \
}

#define rblxstb_bst_raw(TYPE,N,TREE,M,vfield,VTYPE,compare,PAR)                  \
   rblxstb_bst_base(TYPE,N,TREE,M,                                               \
         VTYPE a = p->vfield; VTYPE b = q->vfield; return (compare);, PAR )  \
                                                                             \
TYPE *RBLXSTB__(N,find)(TYPE *tree, VTYPE a)                                     \
   rblxstb_bst_find(N,tree,VTYPE b = tree->vfield; c = (compare);)               \
TYPE *RBLXSTB__(M,Find)(TREE *tree, VTYPE a)                                     \
{ return RBLXSTB__(N,find)(tree->root, a); }

#define rblxstb_bst(TYPE,N,TREE,M,vfield,VTYPE,compare) \
   rblxstb_bst_raw(TYPE,N,TREE,M,vfield,VTYPE,compare,rblxstb__bst_noparent)
#define rblxstb_bst_parent(TYPE,N,TREE,M,vfield,VTYPE,compare) \
   rblxstb_bst_raw(TYPE,N,TREE,M,vfield,VTYPE,compare,rblxstb__bst_parent)



//////////////////////////////////////////////////////////////////////////////
//
//                             Pointer Nulling
//
//  This lets you automatically NULL dangling pointers to "registered"
//  objects. Note that you have to make sure you call the appropriate
//  functions when you free or realloc blocks of memory that contain
//  pointers or pointer targets. rblxstb.h can automatically do this for
//  rblxstb_arr, or for all frees/reallocs if it's wrapping them.
//

#ifdef RBLXSTB_NPTR

RBLXSTB_EXTERN void rblxstb_nptr_set(void *address_of_pointer, void *value_to_write);
RBLXSTB_EXTERN void rblxstb_nptr_didset(void *address_of_pointer);

RBLXSTB_EXTERN void rblxstb_nptr_didfree(void *address_being_freed, int len);
RBLXSTB_EXTERN void rblxstb_nptr_free(void *address_being_freed, int len);

RBLXSTB_EXTERN void rblxstb_nptr_didrealloc(void *new_address, void *old_address, int len);
RBLXSTB_EXTERN void rblxstb_nptr_recache(void); // recache all known pointers
                                       // do this after pointer sets outside your control, slow

#ifdef RBLXSTB_DEFINE
// for fast updating on free/realloc, we need to be able to find
// all the objects (pointers and targets) within a given block;
// this precludes hashing

// we use a three-level hierarchy of memory to minimize storage:
//   level 1: 65536 pointers to rblxstb__memory_node (always uses 256 KB)
//   level 2: each rblxstb__memory_node represents a 64K block of memory
//            with 256 rblxstb__memory_leafs (worst case 64MB)
//   level 3: each rblxstb__memory_leaf represents 256 bytes of memory
//            using a list of target locations and a list of pointers
//            (which are hopefully fairly short normally!)

// this approach won't work in 64-bit, which has a much larger address
// space. need to redesign

#define RBLXSTB__NPTR_ROOT_LOG2   16
#define RBLXSTB__NPTR_ROOT_NUM    (1 << RBLXSTB__NPTR_ROOT_LOG2)
#define RBLXSTB__NPTR_ROOT_SHIFT  (32 - RBLXSTB__NPTR_ROOT_LOG2)

#define RBLXSTB__NPTR_NODE_LOG2   5
#define RBLXSTB__NPTR_NODE_NUM    (1 << RBLXSTB__NPTR_NODE_LOG2)
#define RBLXSTB__NPTR_NODE_MASK   (RBLXSTB__NPTR_NODE_NUM-1)
#define RBLXSTB__NPTR_NODE_SHIFT  (RBLXSTB__NPTR_ROOT_SHIFT - RBLXSTB__NPTR_NODE_LOG2)
#define RBLXSTB__NPTR_NODE_OFFSET(x)   (((x) >> RBLXSTB__NPTR_NODE_SHIFT) & RBLXSTB__NPTR_NODE_MASK)

typedef struct rblxstb__st_nptr
{
   void *ptr;   // address of actual pointer
   struct rblxstb__st_nptr *next;   // next pointer with same target
   struct rblxstb__st_nptr **prev;  // prev pointer with same target, address of 'next' field (or first)
   struct rblxstb__st_nptr *next_in_block;
} rblxstb__nptr;

typedef struct rblxstb__st_nptr_target
{
   void *ptr;   // address of target
   rblxstb__nptr *first; // address of first nptr pointing to this
   struct rblxstb__st_nptr_target *next_in_block;
} rblxstb__nptr_target;

typedef struct
{
   rblxstb__nptr *pointers;
   rblxstb__nptr_target *targets;
} rblxstb__memory_leaf;

typedef struct
{
   rblxstb__memory_leaf *children[RBLXSTB__NPTR_NODE_NUM];
} rblxstb__memory_node;

rblxstb__memory_node *rblxstb__memtab_root[RBLXSTB__NPTR_ROOT_NUM];

static rblxstb__memory_leaf *rblxstb__nptr_find_leaf(void *mem)
{
   rblxstb_uint32 address = (rblxstb_uint32) mem;
   rblxstb__memory_node *z = rblxstb__memtab_root[address >> RBLXSTB__NPTR_ROOT_SHIFT];
   if (z)
      return z->children[RBLXSTB__NPTR_NODE_OFFSET(address)];
   else
      return NULL;
}

static void * rblxstb__nptr_alloc(int size)
{
   return rblxstb__realloc_raw(0,size);
}

static void rblxstb__nptr_free(void *p)
{
   rblxstb__realloc_raw(p,0);
}

static rblxstb__memory_leaf *rblxstb__nptr_make_leaf(void *mem)
{
   rblxstb_uint32 address = (rblxstb_uint32) mem;
   rblxstb__memory_node *z = rblxstb__memtab_root[address >> RBLXSTB__NPTR_ROOT_SHIFT];
   rblxstb__memory_leaf *f;
   if (!z) {
      int i;
      z = (rblxstb__memory_node *) rblxstb__nptr_alloc(sizeof(*rblxstb__memtab_root[0]));
      rblxstb__memtab_root[address >> RBLXSTB__NPTR_ROOT_SHIFT] = z;
      for (i=0; i < 256; ++i)
         z->children[i] = 0;
   }
   f = (rblxstb__memory_leaf *) rblxstb__nptr_alloc(sizeof(*f));
   z->children[RBLXSTB__NPTR_NODE_OFFSET(address)] = f;
   f->pointers = NULL;
   f->targets = NULL;
   return f;
}

static rblxstb__nptr_target *rblxstb__nptr_find_target(void *target, int force)
{
   rblxstb__memory_leaf *p = rblxstb__nptr_find_leaf(target);
   if (p) {
      rblxstb__nptr_target *t = p->targets;
      while (t) {
         if (t->ptr == target)
            return t;
         t = t->next_in_block;
      }
   }
   if (force) {
      rblxstb__nptr_target *t = (rblxstb__nptr_target*) rblxstb__nptr_alloc(sizeof(*t));
      if (!p) p = rblxstb__nptr_make_leaf(target);
      t->ptr = target;
      t->first = NULL;
      t->next_in_block = p->targets;
      p->targets = t;
      return t;
   } else
      return NULL;
}

static rblxstb__nptr *rblxstb__nptr_find_pointer(void *ptr, int force)
{
   rblxstb__memory_leaf *p = rblxstb__nptr_find_leaf(ptr);
   if (p) {
      rblxstb__nptr *t = p->pointers;
      while (t) {
         if (t->ptr == ptr)
            return t;
         t = t->next_in_block;
      }
   }
   if (force) {
      rblxstb__nptr *t = (rblxstb__nptr *) rblxstb__nptr_alloc(sizeof(*t));
      if (!p) p = rblxstb__nptr_make_leaf(ptr);
      t->ptr = ptr;
      t->next = NULL;
      t->prev = NULL;
      t->next_in_block = p->pointers;
      p->pointers = t;
      return t;
   } else
      return NULL;
}

void rblxstb_nptr_set(void *address_of_pointer, void *value_to_write)
{
   if (*(void **)address_of_pointer != value_to_write) {
      *(void **) address_of_pointer = value_to_write;
      rblxstb_nptr_didset(address_of_pointer);
   }
}

void rblxstb_nptr_didset(void *address_of_pointer)
{
   // first unlink from old chain
   void *new_address;
   rblxstb__nptr *p = rblxstb__nptr_find_pointer(address_of_pointer, 1); // force building if doesn't exist
   if (p->prev) { // if p->prev is NULL, we just built it, or it was NULL
      *(p->prev) = p->next;
      if (p->next) p->next->prev = p->prev;
   }
   // now add to new chain
   new_address = *(void **)address_of_pointer;
   if (new_address != NULL) {
      rblxstb__nptr_target *t = rblxstb__nptr_find_target(new_address, 1);
      p->next = t->first;
      if (p->next) p->next->prev = &p->next;
      p->prev = &t->first;
      t->first = p;
   } else {
      p->prev = NULL;
      p->next = NULL;
   }
}

void rblxstb__nptr_block(void *address, int len, void (*function)(rblxstb__memory_leaf *f, int datum, void *start, void *end), int datum)
{
   void *end_address = (void *) ((char *) address + len - 1);
   rblxstb__memory_node *n;
   rblxstb_uint32 start = (rblxstb_uint32) address;
   rblxstb_uint32 end   = start + len - 1;

   int b0 = start >> RBLXSTB__NPTR_ROOT_SHIFT;
   int b1 = end >> RBLXSTB__NPTR_ROOT_SHIFT;
   int b=b0,i,e0,e1;

   e0 = RBLXSTB__NPTR_NODE_OFFSET(start);

   if (datum <= 0) {
      // first block
      n = rblxstb__memtab_root[b0];
      if (n) {
         if (b0 != b1)
            e1 = RBLXSTB__NPTR_NODE_NUM-1;
         else
            e1 = RBLXSTB__NPTR_NODE_OFFSET(end);
         for (i=e0; i <= e1; ++i)
            if (n->children[i])
               function(n->children[i], datum, address, end_address);
      }
      if (b1 > b0) {
         // blocks other than the first and last block
         for (b=b0+1; b < b1; ++b) {
            n = rblxstb__memtab_root[b];
            if (n)
               for (i=0; i <= RBLXSTB__NPTR_NODE_NUM-1; ++i)
                  if (n->children[i])
                     function(n->children[i], datum, address, end_address);
         }
         // last block
         n = rblxstb__memtab_root[b1];
         if (n) {
            e1 = RBLXSTB__NPTR_NODE_OFFSET(end);
            for (i=0; i <= e1; ++i)
               if (n->children[i])
                  function(n->children[i], datum, address, end_address);
         }
      }
   } else {
      if (b1 > b0) {
         // last block
         n = rblxstb__memtab_root[b1];
         if (n) {
            e1 = RBLXSTB__NPTR_NODE_OFFSET(end);
            for (i=e1; i >= 0; --i)
               if (n->children[i])
                  function(n->children[i], datum, address, end_address);
         }
         // blocks other than the first and last block
         for (b=b1-1; b > b0; --b) {
            n = rblxstb__memtab_root[b];
            if (n)
               for (i=RBLXSTB__NPTR_NODE_NUM-1; i >= 0; --i)
                  if (n->children[i])
                     function(n->children[i], datum, address, end_address);
         }
      }
      // first block
      n = rblxstb__memtab_root[b0];
      if (n) {
         if (b0 != b1)
            e1 = RBLXSTB__NPTR_NODE_NUM-1;
         else
            e1 = RBLXSTB__NPTR_NODE_OFFSET(end);
         for (i=e1; i >= e0; --i)
            if (n->children[i])
               function(n->children[i], datum, address, end_address);
      }
   }
}

static void rblxstb__nptr_delete_pointers(rblxstb__memory_leaf *f, int offset, void *start, void *end)
{
   rblxstb__nptr **p = &f->pointers;
   while (*p) {
      rblxstb__nptr *n = *p;
      if (n->ptr >= start && n->ptr <= end) {
         // unlink
         if (n->prev) {
            *(n->prev) = n->next;
            if (n->next) n->next->prev = n->prev;
         }
         *p = n->next_in_block;
         rblxstb__nptr_free(n);
      } else
         p = &(n->next_in_block);
   }
}

static void rblxstb__nptr_delete_targets(rblxstb__memory_leaf *f, int offset, void *start, void *end)
{
   rblxstb__nptr_target **p = &f->targets;
   while (*p) {
      rblxstb__nptr_target *n = *p;
      if (n->ptr >= start && n->ptr <= end) {
         // null pointers
         rblxstb__nptr *z = n->first;
         while (z) {
            rblxstb__nptr *y = z->next;
            z->prev = NULL;
            z->next = NULL;
            *(void **) z->ptr = NULL;
            z = y;
         }
         // unlink this target
         *p = n->next_in_block;
         rblxstb__nptr_free(n);
      } else
         p = &(n->next_in_block);
   }
}

void rblxstb_nptr_didfree(void *address_being_freed, int len)
{
   // step one: delete all pointers in this block
   rblxstb__nptr_block(address_being_freed, len, rblxstb__nptr_delete_pointers, 0);
   // step two: NULL all pointers to this block; do this second to avoid NULLing deleted pointers
   rblxstb__nptr_block(address_being_freed, len, rblxstb__nptr_delete_targets, 0);
}

void rblxstb_nptr_free(void *address_being_freed, int len)
{
   free(address_being_freed);
   rblxstb_nptr_didfree(address_being_freed, len);
}

static void rblxstb__nptr_move_targets(rblxstb__memory_leaf *f, int offset, void *start, void *end)
{
   rblxstb__nptr_target **t = &f->targets;
   while (*t) {
      rblxstb__nptr_target *n = *t;
      if (n->ptr >= start && n->ptr <= end) {
         rblxstb__nptr *z;
         rblxstb__memory_leaf *f;
         // unlink n
         *t = n->next_in_block;
         // update n to new address
         n->ptr = (void *) ((char *) n->ptr + offset);
         f = rblxstb__nptr_find_leaf(n->ptr);
         if (!f) f = rblxstb__nptr_make_leaf(n->ptr);
         n->next_in_block = f->targets;
         f->targets = n;
         // now go through all pointers and make them point here
         z = n->first;
         while (z) {
            *(void**) z->ptr = n->ptr;
            z = z->next;
         }
      } else
         t = &(n->next_in_block);
   }
}

static void rblxstb__nptr_move_pointers(rblxstb__memory_leaf *f, int offset, void *start, void *end)
{
   rblxstb__nptr **p = &f->pointers;
   while (*p) {
      rblxstb__nptr *n = *p;
      if (n->ptr >= start && n->ptr <= end) {
         // unlink
         *p = n->next_in_block;
         n->ptr = (void *) ((int) n->ptr + offset);
         // move to new block
         f = rblxstb__nptr_find_leaf(n->ptr);
         if (!f) f = rblxstb__nptr_make_leaf(n->ptr);
         n->next_in_block = f->pointers;
         f->pointers = n;
      } else
         p = &(n->next_in_block);
   }
}

void rblxstb_nptr_realloc(void *new_address, void *old_address, int len)
{
   if (new_address == old_address) return;

   // have to move the pointers first, because moving the targets
   //      requires writing to the pointers-to-the-targets, and if some of those moved too,
   //      we need to make sure we don't write to the old memory

   // step one: move all pointers within the block
   rblxstb__nptr_block(old_address, len, rblxstb__nptr_move_pointers, (char *) new_address - (char *) old_address);
   // step two: move all targets within the block
   rblxstb__nptr_block(old_address, len, rblxstb__nptr_move_targets, (char *) new_address - (char *) old_address);
}

void rblxstb_nptr_move(void *new_address, void *old_address)
{
   rblxstb_nptr_realloc(new_address, old_address, 1);
}

void rblxstb_nptr_recache(void)
{
   int i,j;
   for (i=0; i < RBLXSTB__NPTR_ROOT_NUM; ++i)
      if (rblxstb__memtab_root[i])
         for (j=0; j < RBLXSTB__NPTR_NODE_NUM; ++j)
            if (rblxstb__memtab_root[i]->children[j]) {
               rblxstb__nptr *p = rblxstb__memtab_root[i]->children[j]->pointers;
               while (p) {
                  rblxstb_nptr_didset(p->ptr);
                  p = p->next_in_block;
               }
            }
}

#endif // RBLXSTB_DEFINE
#endif // RBLXSTB_NPTR


//////////////////////////////////////////////////////////////////////////////
//
//                             File Processing
//


#ifdef _WIN32
  #define rblxstb_rename(x,y)   _wrename((const wchar_t *)rblxstb__from_utf8(x), (const wchar_t *)rblxstb__from_utf8_alt(y))
#else
  #define rblxstb_rename   rename
#endif

RBLXSTB_EXTERN void     rblxstb_fput_varlen64(FILE *f, rblxstb_uint64 v);
RBLXSTB_EXTERN rblxstb_uint64  rblxstb_fget_varlen64(FILE *f);
RBLXSTB_EXTERN int      rblxstb_size_varlen64(rblxstb_uint64 v);


#define rblxstb_filec    (char *) rblxstb_file
#define rblxstb_fileu    (unsigned char *) rblxstb_file
RBLXSTB_EXTERN void *  rblxstb_file(char *filename, size_t *length);
RBLXSTB_EXTERN void *  rblxstb_file_max(char *filename, size_t *length);
RBLXSTB_EXTERN size_t  rblxstb_filelen(FILE *f);
RBLXSTB_EXTERN int     rblxstb_filewrite(char *filename, void *data, size_t length);
RBLXSTB_EXTERN int     rblxstb_filewritestr(char *filename, char *data);
RBLXSTB_EXTERN char ** rblxstb_stringfile(char *filename, int *len);
RBLXSTB_EXTERN char ** rblxstb_stringfile_trimmed(char *name, int *len, char comm);
RBLXSTB_EXTERN char *  rblxstb_fgets(char *buffer, int buflen, FILE *f);
RBLXSTB_EXTERN char *  rblxstb_fgets_malloc(FILE *f);
RBLXSTB_EXTERN int     rblxstb_fexists(char *filename);
RBLXSTB_EXTERN int     rblxstb_fcmp(char *s1, char *s2);
RBLXSTB_EXTERN int     rblxstb_feq(char *s1, char *s2);
RBLXSTB_EXTERN time_t  rblxstb_ftimestamp(char *filename);

RBLXSTB_EXTERN int     rblxstb_fullpath(char *abs, int abs_size, char *rel);
RBLXSTB_EXTERN FILE *  rblxstb_fopen(char *filename, const char *mode);
RBLXSTB_EXTERN int     rblxstb_fclose(FILE *f, int keep);

enum
{
   rblxstb_keep_no = 0,
   rblxstb_keep_yes = 1,
   rblxstb_keep_if_different = 2,
};

RBLXSTB_EXTERN int     rblxstb_copyfile(char *src, char *dest);

RBLXSTB_EXTERN void     rblxstb_fput_varlen64(FILE *f, rblxstb_uint64 v);
RBLXSTB_EXTERN rblxstb_uint64  rblxstb_fget_varlen64(FILE *f);
RBLXSTB_EXTERN int      rblxstb_size_varlen64(rblxstb_uint64 v);

RBLXSTB_EXTERN void    rblxstb_fwrite32(FILE *f, rblxstb_uint32 datum);
RBLXSTB_EXTERN void    rblxstb_fput_varlen (FILE *f, int v);
RBLXSTB_EXTERN void    rblxstb_fput_varlenu(FILE *f, unsigned int v);
RBLXSTB_EXTERN int     rblxstb_fget_varlen (FILE *f);
RBLXSTB_EXTERN rblxstb_uint rblxstb_fget_varlenu(FILE *f);
RBLXSTB_EXTERN void    rblxstb_fput_ranged (FILE *f, int v, int b, rblxstb_uint n);
RBLXSTB_EXTERN int     rblxstb_fget_ranged (FILE *f, int b, rblxstb_uint n);
RBLXSTB_EXTERN int     rblxstb_size_varlen (int v);
RBLXSTB_EXTERN int     rblxstb_size_varlenu(unsigned int v);
RBLXSTB_EXTERN int     rblxstb_size_ranged (int b, rblxstb_uint n);

RBLXSTB_EXTERN int     rblxstb_fread(void *data, size_t len, size_t count, void *f);
RBLXSTB_EXTERN int     rblxstb_fwrite(void *data, size_t len, size_t count, void *f);

#if 0
typedef struct
{
   FILE  *base_file;
   char  *buffer;
   int    buffer_size;
   int    buffer_off;
   int    buffer_left;
} RBLXSTBF;

RBLXSTB_EXTERN RBLXSTBF *rblxstb_tfopen(char *filename, char *mode);
RBLXSTB_EXTERN int rblxstb_tfread(void *data, size_t len, size_t count, RBLXSTBF *f);
RBLXSTB_EXTERN int rblxstb_tfwrite(void *data, size_t len, size_t count, RBLXSTBF *f);
#endif

#ifdef RBLXSTB_DEFINE

#if 0
RBLXSTBF *rblxstb_tfopen(char *filename, char *mode)
{
   RBLXSTBF *z;
   FILE *f = rblxstb_p_fopen(filename, mode);
   if (!f) return NULL;
   z = (RBLXSTBF *) malloc(sizeof(*z));
   if (!z) { fclose(f); return NULL; }
   z->base_file = f;
   if (!strcmp(mode, "rb") || !strcmp(mode, "wb")) {
      z->buffer_size = 4096;
      z->buffer_off = z->buffer_size;
      z->buffer_left = 0;
      z->buffer = malloc(z->buffer_size);
      if (!z->buffer) { free(z); fclose(f); return NULL; }
   } else {
      z->buffer = 0;
      z->buffer_size = 0;
      z->buffer_left = 0;
   }
   return z;
}

int rblxstb_tfread(void *data, size_t len, size_t count, RBLXSTBF *f)
{
   int total = len*count, done=0;
   if (!total) return 0;
   if (total <= z->buffer_left) {
      memcpy(data, z->buffer + z->buffer_off, total);
      z->buffer_off += total;
      z->buffer_left -= total;
      return count;
   } else {
      char *out = (char *) data;

      // consume all buffered data
      memcpy(data, z->buffer + z->buffer_off, z->buffer_left);
      done = z->buffer_left;
      out += z->buffer_left;
      z->buffer_left=0;

      if (total-done > (z->buffer_size >> 1)) {
         done += fread(out
      }
   }
}
#endif

void rblxstb_fwrite32(FILE *f, rblxstb_uint32 x)
{
   fwrite(&x, 4, 1, f);
}

#if defined(_WIN32)
   #define rblxstb__stat   _stat
#else
   #define rblxstb__stat   stat
#endif

int rblxstb_fexists(char *filename)
{
   struct rblxstb__stat buf;
   return rblxstb__windows(
             _wstat((const wchar_t *)rblxstb__from_utf8(filename), &buf),
               stat(filename,&buf)
          ) == 0;
}

time_t rblxstb_ftimestamp(char *filename)
{
   struct rblxstb__stat buf;
   if (rblxstb__windows(
             _wstat((const wchar_t *)rblxstb__from_utf8(filename), &buf),
               stat(filename,&buf)
          ) == 0)
   {
      return buf.st_mtime;
   } else {
      return 0;
   }
}

size_t  rblxstb_filelen(FILE *f)
{
   long len, pos;
   pos = ftell(f);
   fseek(f, 0, SEEK_END);
   len = ftell(f);
   fseek(f, pos, SEEK_SET);
   return (size_t) len;
}

void *rblxstb_file(char *filename, size_t *length)
{
   FILE *f = rblxstb__fopen(filename, "rb");
   char *buffer;
   size_t len, len2;
   if (!f) return NULL;
   len = rblxstb_filelen(f);
   buffer = (char *) malloc(len+2); // nul + extra
   len2 = fread(buffer, 1, len, f);
   if (len2 == len) {
      if (length) *length = len;
      buffer[len] = 0;
   } else {
      free(buffer);
      buffer = NULL;
   }
   fclose(f);
   return buffer;
}

int rblxstb_filewrite(char *filename, void *data, size_t length)
{
   FILE *f = rblxstb_fopen(filename, "wb");
   if (f) {
      unsigned char *data_ptr = (unsigned char *) data;
      size_t remaining = length;
      while (remaining > 0) {
         size_t len2 = remaining > 65536 ? 65536 : remaining;
         size_t len3 = fwrite(data_ptr, 1, len2, f);
         if (len2 != len3) {
            fprintf(stderr, "Failed while writing %s\n", filename);
            break;
         }
         remaining -= len2;
         data_ptr += len2;
      }
      rblxstb_fclose(f, rblxstb_keep_if_different);
   }
   return f != NULL;
}

int rblxstb_filewritestr(char *filename, char *data)
{
   return rblxstb_filewrite(filename, data, strlen(data));
}

void *  rblxstb_file_max(char *filename, size_t *length)
{
   FILE *f = rblxstb__fopen(filename, "rb");
   char *buffer;
   size_t len, maxlen;
   if (!f) return NULL;
   maxlen = *length;
   buffer = (char *) malloc(maxlen+1);
   len = fread(buffer, 1, maxlen, f);
   buffer[len] = 0;
   fclose(f);
   *length = len;
   return buffer;
}

char ** rblxstb_stringfile(char *filename, int *plen)
{
   FILE *f = rblxstb__fopen(filename, "rb");
   char *buffer, **list=NULL, *s;
   size_t len, count, i;

   if (!f) return NULL;
   len = rblxstb_filelen(f);
   buffer = (char *) malloc(len+1);
   len = fread(buffer, 1, len, f);
   buffer[len] = 0;
   fclose(f);

   // two passes through: first time count lines, second time set them
   for (i=0; i < 2; ++i) {
      s = buffer;
      if (i == 1)
         list[0] = s;
      count = 1;
      while (*s) {
         if (*s == '\n' || *s == '\r') {
            // detect if both cr & lf are together
            int crlf = (s[0] + s[1]) == ('\n' + '\r');
            if (i == 1) *s = 0;
            if (crlf) ++s;
            if (s[1]) {  // it's not over yet
               if (i == 1) list[count] = s+1;
               ++count;
            }
         }
         ++s;
      }
      if (i == 0) {
         list = (char **) malloc(sizeof(*list) * (count+1) + len+1);
         if (!list) return NULL;
         list[count] = 0;
         // recopy the file so there's just a single allocation to free
         memcpy(&list[count+1], buffer, len+1);
         free(buffer);
         buffer = (char *) &list[count+1];
         if (plen) *plen = (int) count;
      }
   }
   return list;
}

char ** rblxstb_stringfile_trimmed(char *name, int *len, char comment)
{
   int i,n,o=0;
   char **s = rblxstb_stringfile(name, &n);
   if (s == NULL) return NULL;
   for (i=0; i < n; ++i) {
      char *p = rblxstb_skipwhite(s[i]);
      if (*p && *p != comment)
         s[o++] = p;
   }
   s[o] = NULL;
   if (len) *len = o;
   return s;
}

char * rblxstb_fgets(char *buffer, int buflen, FILE *f)
{
   char *p;
   buffer[0] = 0;
   p = fgets(buffer, buflen, f);
   if (p) {
      int n = (int) (strlen(p)-1);
      if (n >= 0)
         if (p[n] == '\n')
            p[n] = 0;
   }
   return p;
}

char * rblxstb_fgets_malloc(FILE *f)
{
   // avoid reallocing for small strings
   char quick_buffer[800];
   quick_buffer[sizeof(quick_buffer)-2] = 0;
   if (!fgets(quick_buffer, sizeof(quick_buffer), f))
      return NULL;

   if (quick_buffer[sizeof(quick_buffer)-2] == 0) {
      size_t n = strlen(quick_buffer);
      if (n > 0 && quick_buffer[n-1] == '\n')
         quick_buffer[n-1] = 0;
      return rblxstb_p_strdup(quick_buffer);
   } else {
      char *p;
      char *a = rblxstb_p_strdup(quick_buffer);
      size_t len = sizeof(quick_buffer)-1;

      while (!feof(f)) {
         if (a[len-1] == '\n') break;
         a = (char *) realloc(a, len*2);
         p = &a[len];
         p[len-2] = 0;
         if (!fgets(p, (int) len, f))
            break;
         if (p[len-2] == 0) {
            len += strlen(p);
            break;
         }
         len = len + (len-1);
      }
      if (a[len-1] == '\n')
         a[len-1] = 0;
      return a;
   }
}

int rblxstb_fullpath(char *abs, int abs_size, char *rel)
{
   #ifdef _WIN32
   return _fullpath(abs, rel, abs_size) != NULL;
   #else
   if (rel[0] == '/' || rel[0] == '~') {
      if ((int) strlen(rel) >= abs_size)
         return 0;
      rblxstb_p_strcpy_s(abs,65536,rel);
      return RBLXSTB_TRUE;
   } else {
      int n;
      getcwd(abs, abs_size);
      n = strlen(abs);
      if (n+(int) strlen(rel)+2 <= abs_size) {
         abs[n] = '/';
         rblxstb_p_strcpy_s(abs+n+1, 65536,rel);
         return RBLXSTB_TRUE;
      } else {
         return RBLXSTB_FALSE;
      }
   }
   #endif
}

static int rblxstb_fcmp_core(FILE *f, FILE *g)
{
   char buf1[1024],buf2[1024];
   int n1,n2, res=0;

   while (1) {
      n1 = (int) fread(buf1, 1, sizeof(buf1), f);
      n2 = (int) fread(buf2, 1, sizeof(buf2), g);
      res = memcmp(buf1,buf2,rblxstb_min(n1,n2));
      if (res)
         break;
      if (n1 != n2) {
         res = n1 < n2 ? -1 : 1;
         break;
      }
      if (n1 == 0)
         break;
   }

   fclose(f);
   fclose(g);
   return res;
}

int rblxstb_fcmp(char *s1, char *s2)
{
   FILE *f = rblxstb__fopen(s1, "rb");
   FILE *g = rblxstb__fopen(s2, "rb");

   if (f == NULL || g == NULL) {
      if (f) fclose(f);
      if (g) {
         fclose(g);
         return RBLXSTB_TRUE;
      }
      return f != NULL;
   }

   return rblxstb_fcmp_core(f,g);
}

int rblxstb_feq(char *s1, char *s2)
{
   FILE *f = rblxstb__fopen(s1, "rb");
   FILE *g = rblxstb__fopen(s2, "rb");

   if (f == NULL || g == NULL) {
      if (f) fclose(f);
      if (g) fclose(g);
      return f == g;
   }

   // feq is faster because it shortcuts if they're different length
   if (rblxstb_filelen(f) != rblxstb_filelen(g)) {
      fclose(f);
      fclose(g);
      return 0;
   }

   return !rblxstb_fcmp_core(f,g);
}

static rblxstb_ptrmap *rblxstb__files;

typedef struct
{
   char *temp_name;
   char *name;
   int   errors;
} rblxstb__file_data;

static FILE *rblxstb__open_temp_file(char *temp_name, char *src_name, const char *mode)
{
   size_t p;
#ifdef _MSC_VER
   int j;
#endif
   FILE *f;
   // try to generate a temporary file in the same directory
   p = strlen(src_name)-1;
   while (p > 0 && src_name[p] != '/' && src_name[p] != '\\'
                && src_name[p] != ':' && src_name[p] != '~')
      --p;
   ++p;

   memcpy(temp_name, src_name, p);

   #ifdef _MSC_VER
   // try multiple times to make a temp file... just in
   // case some other process makes the name first
   for (j=0; j < 32; ++j) {
      rblxstb_p_strcpy_s(temp_name+p, 65536, "stmpXXXXXX");
      if (!rblxstb_p_mktemp(temp_name))
         return 0;

      f = rblxstb_p_fopen(temp_name, mode);
      if (f != NULL)
         break;
   }
   #else
   {
      rblxstb_p_strcpy_s(temp_name+p, 65536, "stmpXXXXXX");
      #ifdef __MINGW32__
         int fd = open(rblxstb_p_mktemp(temp_name), O_RDWR);
      #else
         int fd = mkstemp(temp_name);
      #endif
      if (fd == -1) return NULL;
      f = fdopen(fd, mode);
      if (f == NULL) {
         unlink(temp_name);
         close(fd);
         return NULL;
      }
   }
   #endif
   return f;
}


FILE *  rblxstb_fopen(char *filename, const char *mode)
{
   FILE *f;
   char name_full[4096];
   char temp_full[sizeof(name_full) + 12];

   // @TODO: if the file doesn't exist, we can also use the fastpath here
   if (mode[0] != 'w' && !strchr(mode, '+'))
      return rblxstb__fopen(filename, mode);

   // save away the full path to the file so if the program
   // changes the cwd everything still works right! unix has
   // better ways to do this, but we have to work in windows
   name_full[0] = '\0'; // rblxstb_fullpath reads name_full[0]
   if (rblxstb_fullpath(name_full, sizeof(name_full), filename)==0)
      return 0;

   f = rblxstb__open_temp_file(temp_full, name_full, mode);
   if (f != NULL) {
      rblxstb__file_data *d = (rblxstb__file_data *) malloc(sizeof(*d));
      if (!d) { assert(0);  /* NOTREACHED */fclose(f); return NULL; }
      if (rblxstb__files == NULL) rblxstb__files = rblxstb_ptrmap_create();
      d->temp_name = rblxstb_p_strdup(temp_full);
      d->name      = rblxstb_p_strdup(name_full);
      d->errors    = 0;
      rblxstb_ptrmap_add(rblxstb__files, f, d);
      return f;
   }

   return NULL;
}

int     rblxstb_fclose(FILE *f, int keep)
{
   rblxstb__file_data *d;

   int ok = RBLXSTB_FALSE;
   if (f == NULL) return 0;

   if (ferror(f))
      keep = rblxstb_keep_no;

   fclose(f);

   if (rblxstb__files && rblxstb_ptrmap_remove(rblxstb__files, f, (void **) &d)) {
      if (rblxstb__files->count == 0) {
         rblxstb_ptrmap_destroy(rblxstb__files);
         rblxstb__files = NULL;
      }
   } else
      return RBLXSTB_TRUE; // not special

   if (keep == rblxstb_keep_if_different) {
      // check if the files are identical
      if (rblxstb_feq(d->name, d->temp_name)) {
         keep = rblxstb_keep_no;
         ok = RBLXSTB_TRUE;  // report success if no change
      }
   }

   if (keep == rblxstb_keep_no) {
      remove(d->temp_name);
   } else {
      if (!rblxstb_fexists(d->name)) {
         // old file doesn't exist, so just move the new file over it
         rblxstb_rename(d->temp_name, d->name);
      } else {
         // don't delete the old file yet in case there are troubles! First rename it!
         char preserved_old_file[4096];

         // generate a temp filename in the same directory (also creates it, which we don't need)
         FILE *dummy = rblxstb__open_temp_file(preserved_old_file, d->name, "wb");
         if (dummy != NULL) {
            // we don't actually want the open file
            fclose(dummy);

            // discard what we just created
            remove(preserved_old_file);  // if this fails, there's nothing we can do, and following logic handles it as best as possible anyway

            // move the existing file to the preserved name
            if (0 != rblxstb_rename(d->name, preserved_old_file)) {  // 0 on success
               // failed, state is:
               //    filename  -> old file
               //    tempname  -> new file
               // keep tempname around so we don't lose data
            } else {
               //  state is:
               //    preserved -> old file
               //    tempname  -> new file
               // move the new file to the old name
               if (0 == rblxstb_rename(d->temp_name, d->name)) {
                  //  state is:
                  //    preserved -> old file
                  //    filename  -> new file
                  ok = RBLXSTB_TRUE;

                  // 'filename -> new file' has always been the goal, so clean up
                  remove(preserved_old_file); // nothing to be done if it fails
               } else {
                  // couldn't rename, so try renaming preserved file back

                  //  state is:
                  //    preserved -> old file
                  //    tempname  -> new file
                  rblxstb_rename(preserved_old_file, d->name);
                  // if the rename failed, there's nothing more we can do
               }
            }
         } else {
            // we couldn't get a temp filename. do this the naive way; the worst case failure here
            // leaves the filename pointing to nothing and the new file as a tempfile
            remove(d->name);
            rblxstb_rename(d->temp_name, d->name);
         }
      }
   }

   free(d->temp_name);
   free(d->name);
   free(d);

   return ok;
}

int rblxstb_copyfile(char *src, char *dest)
{
   char raw_buffer[1024];
   char *buffer;
   int buf_size = 65536;

   FILE *f, *g;

   // if file already exists at destination, do nothing
   if (rblxstb_feq(src, dest)) return RBLXSTB_TRUE;

   // open file
   f = rblxstb__fopen(src, "rb");
   if (f == NULL) return RBLXSTB_FALSE;

   // open file for writing
   g = rblxstb__fopen(dest, "wb");
   if (g == NULL) {
      fclose(f);
      return RBLXSTB_FALSE;
   }

   buffer = (char *) malloc(buf_size);
   if (buffer == NULL) {
      buffer = raw_buffer;
      buf_size = sizeof(raw_buffer);
   }

   while (!feof(f)) {
      size_t n = fread(buffer, 1, buf_size, f);
      if (n != 0)
         fwrite(buffer, 1, n, g);
   }

   fclose(f);
   if (buffer != raw_buffer)
      free(buffer);

   fclose(g);
   return RBLXSTB_TRUE;
}

// varlen:
//    v' = (v >> 31) + (v < 0 ? ~v : v)<<1;  // small abs(v) => small v'
// output v as big endian v'+k for v' <= k:
//   1 byte :  v' <= 0x00000080          (  -64 <= v <   64)   7 bits
//   2 bytes:  v' <= 0x00004000          (-8192 <= v < 8192)  14 bits
//   3 bytes:  v' <= 0x00200000                               21 bits
//   4 bytes:  v' <= 0x10000000                               28 bits
// the number of most significant 1-bits in the first byte
// equals the number of bytes after the first

#define rblxstb__varlen_xform(v)     (v<0 ? (~v << 1)+1 : (v << 1))

int rblxstb_size_varlen(int v) { return rblxstb_size_varlenu(rblxstb__varlen_xform(v)); }
int rblxstb_size_varlenu(unsigned int v)
{
   if (v < 0x00000080) return 1;
   if (v < 0x00004000) return 2;
   if (v < 0x00200000) return 3;
   if (v < 0x10000000) return 4;
   return 5;
}

void    rblxstb_fput_varlen(FILE *f, int v) { rblxstb_fput_varlenu(f, rblxstb__varlen_xform(v)); }

void    rblxstb_fput_varlenu(FILE *f, unsigned int z)
{
   if (z >= 0x10000000) fputc(0xF0,f);
   if (z >= 0x00200000) fputc((z < 0x10000000 ? 0xE0 : 0)+(z>>24),f);
   if (z >= 0x00004000) fputc((z < 0x00200000 ? 0xC0 : 0)+(z>>16),f);
   if (z >= 0x00000080) fputc((z < 0x00004000 ? 0x80 : 0)+(z>> 8),f);
   fputc(z,f);
}

#define rblxstb_fgetc(f)    ((unsigned char) fgetc(f))

int     rblxstb_fget_varlen(FILE *f)
{
   unsigned int z = rblxstb_fget_varlenu(f);
   return (z & 1) ? ~(z>>1) : (z>>1);
}

unsigned int rblxstb_fget_varlenu(FILE *f)
{
   unsigned int z;
   unsigned char d;
   d = rblxstb_fgetc(f);

   if (d >= 0x80) {
      if (d >= 0xc0) {
         if (d >= 0xe0) {
            if (d == 0xf0) z = rblxstb_fgetc(f) << 24;
            else           z = (d - 0xe0) << 24;
            z += rblxstb_fgetc(f) << 16;
         }
         else
            z = (d - 0xc0) << 16;
         z += rblxstb_fgetc(f) << 8;
      } else
         z = (d - 0x80) <<  8;
      z += rblxstb_fgetc(f);
   } else
      z = d;
   return z;
}

rblxstb_uint64   rblxstb_fget_varlen64(FILE *f)
{
   rblxstb_uint64 z;
   unsigned char d;
   d = rblxstb_fgetc(f);

   if (d >= 0x80) {
      if (d >= 0xc0) {
         if (d >= 0xe0) {
            if (d >= 0xf0) {
               if (d >= 0xf8) {
                  if (d >= 0xfc) {
                     if (d >= 0xfe) {
                        if (d >= 0xff)
                           z = (rblxstb_uint64) rblxstb_fgetc(f) << 56;
                        else
                           z = (rblxstb_uint64) (d - 0xfe) << 56;
                        z |= (rblxstb_uint64) rblxstb_fgetc(f) << 48;
                     } else z = (rblxstb_uint64) (d - 0xfc) << 48;
                     z |= (rblxstb_uint64) rblxstb_fgetc(f) << 40;
                  } else z = (rblxstb_uint64) (d - 0xf8) << 40;
                  z |= (rblxstb_uint64) rblxstb_fgetc(f) << 32;
               } else z = (rblxstb_uint64) (d - 0xf0) << 32;
               z |= (rblxstb_uint) rblxstb_fgetc(f) << 24;
            } else z = (rblxstb_uint) (d - 0xe0) << 24;
            z |= (rblxstb_uint) rblxstb_fgetc(f) << 16;
         } else z = (rblxstb_uint) (d - 0xc0) << 16;
         z |= (rblxstb_uint) rblxstb_fgetc(f) << 8;
      } else z = (rblxstb_uint) (d - 0x80) << 8;
      z |= rblxstb_fgetc(f);
   } else
      z = d;

   return (z & 1) ? ~(z >> 1) : (z >> 1);
}

int rblxstb_size_varlen64(rblxstb_uint64 v)
{
   if (v < 0x00000080) return 1;
   if (v < 0x00004000) return 2;
   if (v < 0x00200000) return 3;
   if (v < 0x10000000) return 4;
   if (v < RBLXSTB_IMM_UINT64(0x0000000800000000)) return 5;
   if (v < RBLXSTB_IMM_UINT64(0x0000040000000000)) return 6;
   if (v < RBLXSTB_IMM_UINT64(0x0002000000000000)) return 7;
   if (v < RBLXSTB_IMM_UINT64(0x0100000000000000)) return 8;
   return 9;
}

void    rblxstb_fput_varlen64(FILE *f, rblxstb_uint64 v)
{
   rblxstb_uint64 z = rblxstb__varlen_xform(v);
   int first=1;
   if (z >= RBLXSTB_IMM_UINT64(0x100000000000000)) {
      fputc(0xff,f);
      first=0;
   }
   if (z >= RBLXSTB_IMM_UINT64(0x02000000000000)) fputc((first ? 0xFE : 0)+(char)(z>>56),f), first=0;
   if (z >= RBLXSTB_IMM_UINT64(0x00040000000000)) fputc((first ? 0xFC : 0)+(char)(z>>48),f), first=0;
   if (z >= RBLXSTB_IMM_UINT64(0x00000800000000)) fputc((first ? 0xF8 : 0)+(char)(z>>40),f), first=0;
   if (z >= RBLXSTB_IMM_UINT64(0x00000010000000)) fputc((first ? 0xF0 : 0)+(char)(z>>32),f), first=0;
   if (z >= RBLXSTB_IMM_UINT64(0x00000000200000)) fputc((first ? 0xE0 : 0)+(char)(z>>24),f), first=0;
   if (z >= RBLXSTB_IMM_UINT64(0x00000000004000)) fputc((first ? 0xC0 : 0)+(char)(z>>16),f), first=0;
   if (z >= RBLXSTB_IMM_UINT64(0x00000000000080)) fputc((first ? 0x80 : 0)+(char)(z>> 8),f), first=0;
   fputc((char)z,f);
}

void    rblxstb_fput_ranged(FILE *f, int v, int b, rblxstb_uint n)
{
   v -= b;
   if (n <= (1 << 31))
      assert((rblxstb_uint) v < n);
   if (n > (1 << 24)) fputc(v >> 24, f);
   if (n > (1 << 16)) fputc(v >> 16, f);
   if (n > (1 <<  8)) fputc(v >>  8, f);
   fputc(v,f);
}

int     rblxstb_fget_ranged(FILE *f, int b, rblxstb_uint n)
{
   unsigned int v=0;
   if (n > (1 << 24)) v += rblxstb_fgetc(f) << 24;
   if (n > (1 << 16)) v += rblxstb_fgetc(f) << 16;
   if (n > (1 <<  8)) v += rblxstb_fgetc(f) <<  8;
   v += rblxstb_fgetc(f);
   return b+v;
}

int     rblxstb_size_ranged(int b, rblxstb_uint n)
{
   if (n > (1 << 24)) return 4;
   if (n > (1 << 16)) return 3;
   if (n > (1 <<  8)) return 2;
   return 1;
}

void rblxstb_fput_string(FILE *f, char *s)
{
   size_t len = strlen(s);
   rblxstb_fput_varlenu(f, (unsigned int) len);
   fwrite(s, 1, len, f);
}

// inverse of the above algorithm
char *rblxstb_fget_string(FILE *f, void *p)
{
   char *s;
   int len = rblxstb_fget_varlenu(f);
   if (len > 4096) return NULL;
   s = p ? rblxstb_malloc_string(p, len+1) : (char *) malloc(len+1);
   fread(s, 1, len, f);
   s[len] = 0;
   return s;
}

char *rblxstb_strdup(char *str, void *pool)
{
   size_t len = strlen(str);
   char *p = rblxstb_malloc_string(pool, len+1);
   rblxstb_p_strcpy_s(p, len+1, str);
   return p;
}

// strip the trailing '/' or '\\' from a directory so we can refer to it
// as a file for _stat()
char *rblxstb_strip_final_slash(char *t)
{
   if (t[0]) {
      char *z = t + strlen(t) - 1;
      // *z is the last character
      if (*z == '\\' || *z == '/')
         if (z != t+2 || t[1] != ':') // but don't strip it if it's e.g. "c:/"
            *z = 0;
      if (*z == '\\')
         *z = '/'; // canonicalize to make sure it matches db
   }
   return t;
}

char *rblxstb_strip_final_slash_regardless(char *t)
{
   if (t[0]) {
      char *z = t + strlen(t) - 1;
      // *z is the last character
      if (*z == '\\' || *z == '/')
         *z = 0;
      if (*z == '\\')
         *z = '/'; // canonicalize to make sure it matches db
   }
   return t;
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
//                 Options parsing
//

RBLXSTB_EXTERN char **rblxstb_getopt_param(int *argc, char **argv, char *param);
RBLXSTB_EXTERN char **rblxstb_getopt(int *argc, char **argv);
RBLXSTB_EXTERN void   rblxstb_getopt_free(char **opts);

#ifdef RBLXSTB_DEFINE

void   rblxstb_getopt_free(char **opts)
{
   int i;
   char ** o2 = opts;
   for (i=0; i < rblxstb_arr_len(o2); ++i)
      free(o2[i]);
   rblxstb_arr_free(o2);
}

char **rblxstb_getopt(int *argc, char **argv)
{
   return rblxstb_getopt_param(argc, argv, (char*) "");
}

char **rblxstb_getopt_param(int *argc, char **argv, char *param)
{
   char ** opts=NULL;
   int i,j=1;
   for (i=1; i < *argc; ++i) {
      if (argv[i][0] != '-') {
         argv[j++] = argv[i];
      } else {
         if (argv[i][1] == 0) { // plain - == don't parse further options
            ++i;
            while (i < *argc)
               argv[j++] = argv[i++];
            break;
         } else if (argv[i][1] == '-') {
            // copy argument through including initial '-' for clarity
            rblxstb_arr_push(opts, rblxstb_p_strdup(argv[i]));
         } else {
            int k;
            char *q = argv[i];  // traverse options list
            for (k=1; q[k]; ++k) {
               char *s;
               if (strchr(param, q[k])) {  // does it take a parameter?
                  char *t = &q[k+1], z = q[k];
                  size_t len=0;
                  if (*t == 0) {
                     if (i == *argc-1) { // takes a parameter, but none found
                        *argc = 0;
                        rblxstb_getopt_free(opts);
                        return NULL;
                     }
                     t = argv[++i];
                  } else
                     k += (int) strlen(t);
                  len = strlen(t);
                  s = (char *) malloc(len+2);
                  if (!s) return NULL;
                  s[0] = z;
                  rblxstb_p_strcpy_s(s+1, len+2, t);
               } else {
                  // no parameter
                  s = (char *) malloc(2);
                  if (!s) return NULL;
                  s[0] = q[k];
                  s[1] = 0;
               }
               rblxstb_arr_push(opts, s);
            }
         }
      }
   }
   rblxstb_arr_push(opts, NULL);
   *argc = j;
   return opts;
}
#endif


//////////////////////////////////////////////////////////////////////////////
//
//                 Portable directory reading
//

RBLXSTB_EXTERN char **rblxstb_readdir_files  (char *dir);
RBLXSTB_EXTERN char **rblxstb_readdir_files_mask(char *dir, char *wild);
RBLXSTB_EXTERN char **rblxstb_readdir_subdirs(char *dir);
RBLXSTB_EXTERN char **rblxstb_readdir_subdirs_mask(char *dir, char *wild);
RBLXSTB_EXTERN void   rblxstb_readdir_free   (char **files);
RBLXSTB_EXTERN char **rblxstb_readdir_recursive(char *dir, char *filespec);
RBLXSTB_EXTERN void rblxstb_delete_directory_recursive(char *dir);

#ifdef RBLXSTB_DEFINE

#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#include <dirent.h>
#endif

void rblxstb_readdir_free(char **files)
{
   char **f2 = files;
   int i;
   for (i=0; i < rblxstb_arr_len(f2); ++i)
      free(f2[i]);
   rblxstb_arr_free(f2);
}

static int isdotdirname(char *name)
{
   if (name[0] == '.')
      return (name[1] == '.') ? !name[2] : !name[1];
   return 0;
}

RBLXSTB_EXTERN int rblxstb_wildmatchi(char *expr, char *candidate);
static char **readdir_raw(char *dir, int return_subdirs, char *mask)
{
   char **results = NULL;
   char buffer[4096], with_slash[4096];
   size_t n;

   #ifdef WIN32
      rblxstb__wchar *ws;
      struct _wfinddata_t data;
   #ifdef _WIN64
      const intptr_t none = -1;
      intptr_t z;
   #else
      const long none = -1;
      long z;
   #endif
   #else // !WIN32
      const DIR *none = NULL;
      DIR *z;
   #endif

   n = rblxstb_strscpy(buffer,dir,sizeof(buffer));
   if (!n || n >= sizeof(buffer))
      return NULL;
   rblxstb_fixpath(buffer);

   if (n > 0 && (buffer[n-1] != '/')) {
      buffer[n++] = '/';
   }
   buffer[n] = 0;
   if (!rblxstb_strscpy(with_slash,buffer,sizeof(with_slash)))
      return NULL;

   #ifdef WIN32
      if (!rblxstb_strscpy(buffer+n,"*.*",sizeof(buffer)-n))
         return NULL;
      ws = rblxstb__from_utf8(buffer);
      z = _wfindfirst((wchar_t *)ws, &data);
   #else
      z = opendir(dir);
   #endif

   if (z != none) {
      int nonempty = RBLXSTB_TRUE;
      #ifndef WIN32
      struct dirent *data = readdir(z);
      nonempty = (data != NULL);
      #endif

      if (nonempty) {

         do {
            int is_subdir;
            #ifdef WIN32
            char *name = rblxstb__to_utf8((rblxstb__wchar *)data.name);
            if (name == NULL) {
               fprintf(stderr, "%s to convert '%S' to %s!\n", "Unable", data.name, "utf8");
               continue;
            }
            is_subdir = !!(data.attrib & _A_SUBDIR);
            #else
            char *name = data->d_name;
            if (!rblxstb_strscpy(buffer+n,name,sizeof(buffer)-n))
               break;
            // Could follow DT_LNK, but would need to check for recursive links.
            is_subdir = !!(data->d_type & DT_DIR);
            #endif

            if (is_subdir == return_subdirs) {
               if (!is_subdir || !isdotdirname(name)) {
                  if (!mask || rblxstb_wildmatchi(mask, name)) {
                     char buffer[4096],*p=buffer;
                     if ( rblxstb_snprintf(buffer, sizeof(buffer), "%s%s", with_slash, name) < 0 )
                        break;
                     if (buffer[0] == '.' && buffer[1] == '/')
                        p = buffer+2;
                     rblxstb_arr_push(results, rblxstb_p_strdup(p));
                  }
               }
            }
         }
         #ifdef WIN32
         while (0 == _wfindnext(z, &data));
         #else
         while ((data = readdir(z)) != NULL);
         #endif
      }
      #ifdef WIN32
         _findclose(z);
      #else
         closedir(z);
      #endif
   }
   return results;
}

char **rblxstb_readdir_files  (char *dir) { return readdir_raw(dir, 0, NULL); }
char **rblxstb_readdir_subdirs(char *dir) { return readdir_raw(dir, 1, NULL); }
char **rblxstb_readdir_files_mask(char *dir, char *wild) { return readdir_raw(dir, 0, wild); }
char **rblxstb_readdir_subdirs_mask(char *dir, char *wild) { return readdir_raw(dir, 1, wild); }

int rblxstb__rec_max=0x7fffffff;
static char **rblxstb_readdir_rec(char **sofar, char *dir, char *filespec)
{
   char **files;
   char ** dirs;
   char **p;

   if (rblxstb_arr_len(sofar) >= rblxstb__rec_max) return sofar;

   files = rblxstb_readdir_files_mask(dir, filespec);
   rblxstb_arr_for(p, files) {
      rblxstb_arr_push(sofar, rblxstb_p_strdup(*p));
      if (rblxstb_arr_len(sofar) >= rblxstb__rec_max) break;
   }
   rblxstb_readdir_free(files);
   if (rblxstb_arr_len(sofar) >= rblxstb__rec_max) return sofar;

   dirs = rblxstb_readdir_subdirs(dir);
   rblxstb_arr_for(p, dirs)
      sofar = rblxstb_readdir_rec(sofar, *p, filespec);
   rblxstb_readdir_free(dirs);
   return sofar;
}

char **rblxstb_readdir_recursive(char *dir, char *filespec)
{
   return rblxstb_readdir_rec(NULL, dir, filespec);
}

void rblxstb_delete_directory_recursive(char *dir)
{
   char **list = rblxstb_readdir_subdirs(dir);
   int i;
   for (i=0; i < rblxstb_arr_len(list); ++i)
      rblxstb_delete_directory_recursive(list[i]);
   rblxstb_arr_free(list);
   list = rblxstb_readdir_files(dir);
   for (i=0; i < rblxstb_arr_len(list); ++i)
      if (!remove(list[i])) {
         // on windows, try again after making it writeable; don't ALWAYS
         // do this first since that would be slow in the normal case
         #ifdef _MSC_VER
         _chmod(list[i], _S_IWRITE);
         remove(list[i]);
         #endif
      }
   rblxstb_arr_free(list);
   rblxstb__windows(_rmdir,rmdir)(dir);
}

#endif

//////////////////////////////////////////////////////////////////////////////
//
//   construct trees from filenames; useful for cmirror summaries

typedef struct rblxstb_dirtree2 rblxstb_dirtree2;

struct rblxstb_dirtree2
{
   rblxstb_dirtree2 **subdirs;

   // make convenient for rblxstb_summarize_tree
   int num_subdir;
   float weight;

   // actual data
   char *fullpath;
   char *relpath;
   char **files;
};

RBLXSTB_EXTERN rblxstb_dirtree2 *rblxstb_dirtree2_from_files_relative(char *src, char **filelist, int count);
RBLXSTB_EXTERN rblxstb_dirtree2 *rblxstb_dirtree2_from_files(char **filelist, int count);
RBLXSTB_EXTERN int rblxstb_dir_is_prefix(char *dir, int dirlen, char *file);

#ifdef RBLXSTB_DEFINE

int rblxstb_dir_is_prefix(char *dir, int dirlen, char *file)
{
   if (dirlen == 0) return RBLXSTB_TRUE;
   if (rblxstb_strnicmp(dir, file, dirlen)) return RBLXSTB_FALSE;
   if (file[dirlen] == '/' || file[dirlen] == '\\') return RBLXSTB_TRUE;
   return RBLXSTB_FALSE;
}

rblxstb_dirtree2 *rblxstb_dirtree2_from_files_relative(char *src, char **filelist, int count)
{
   char buffer1[1024];
   int i;
   int dlen = (int) strlen(src), elen;
   rblxstb_dirtree2 *d;
   char ** descendents = NULL;
   char ** files = NULL;
   char *s;
   if (!count) return NULL;
   // first find all the ones that belong here... note this is will take O(NM) with N files and M subdirs
   for (i=0; i < count; ++i) {
      if (rblxstb_dir_is_prefix(src, dlen, filelist[i])) {
         rblxstb_arr_push(descendents, filelist[i]);
      }
   }
   if (descendents == NULL)
      return NULL;
   elen = dlen;
   // skip a leading slash
   if (elen == 0 && (descendents[0][0] == '/' || descendents[0][0] == '\\'))
      ++elen;
   else if (elen)
      ++elen;
   // now extract all the ones that have their root here
   for (i=0; i < rblxstb_arr_len(descendents);) {
      if (!rblxstb_strchr2(descendents[i]+elen, '/', '\\')) {
         rblxstb_arr_push(files, descendents[i]);
         descendents[i] = descendents[rblxstb_arr_len(descendents)-1];
         rblxstb_arr_pop(descendents);
      } else
         ++i;
   }
   // now create a record
   d = (rblxstb_dirtree2 *) malloc(sizeof(*d));
   d->files = files;
   d->subdirs = NULL;
   d->fullpath = rblxstb_p_strdup(src);
   s = rblxstb_strrchr2(d->fullpath, '/', '\\');
   if (s)
      ++s;
   else
      s = d->fullpath;
   d->relpath = s;
   // now create the children
   qsort(descendents, rblxstb_arr_len(descendents), sizeof(char *), rblxstb_qsort_stricmp(0));
   buffer1[0] = 0;
   for (i=0; i < rblxstb_arr_len(descendents); ++i) {
      char buffer2[1024];
      char *s = descendents[i] + elen, *t;
      t = rblxstb_strchr2(s, '/', '\\');
      assert(t);
      rblxstb_strncpy(buffer2, descendents[i], (int) (t-descendents[i]+1));
      if (rblxstb_stricmp(buffer1, buffer2)) {
         rblxstb_dirtree2 *t = rblxstb_dirtree2_from_files_relative(buffer2, descendents, rblxstb_arr_len(descendents));
         assert(t != NULL);
         rblxstb_p_strcpy_s(buffer1, sizeof(buffer1), buffer2);
         rblxstb_arr_push(d->subdirs, t);
      }
   }
   d->num_subdir = rblxstb_arr_len(d->subdirs);
   d->weight = 0;
   return d;
}

rblxstb_dirtree2 *rblxstb_dirtree2_from_files(char **filelist, int count)
{
   return rblxstb_dirtree2_from_files_relative((char*) "", filelist, count);
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
//                 Checksums: CRC-32, ADLER32, SHA-1
//
//    CRC-32 and ADLER32 allow streaming blocks
//    SHA-1 requires either a complete buffer, max size 2^32 - 73
//          or it can checksum directly from a file, max 2^61

#define RBLXSTB_ADLER32_SEED   1
#define RBLXSTB_CRC32_SEED     0    // note that we logical NOT this in the code

RBLXSTB_EXTERN rblxstb_uint
  rblxstb_adler32(rblxstb_uint adler32, rblxstb_uchar *buffer, rblxstb_uint buflen);
RBLXSTB_EXTERN rblxstb_uint
  rblxstb_crc32_block(rblxstb_uint crc32, rblxstb_uchar *buffer, rblxstb_uint len);
RBLXSTB_EXTERN rblxstb_uint rblxstb_crc32(unsigned char *buffer, rblxstb_uint len);

RBLXSTB_EXTERN void rblxstb_sha1(
  unsigned char output[20], unsigned char *buffer, unsigned int len);
RBLXSTB_EXTERN int rblxstb_sha1_file(unsigned char output[20], char *file);

RBLXSTB_EXTERN void rblxstb_sha1_readable(char display[27], unsigned char sha[20]);

#ifdef RBLXSTB_DEFINE
rblxstb_uint rblxstb_crc32_block(rblxstb_uint crc, unsigned char *buffer, rblxstb_uint len)
{
   static rblxstb_uint crc_table[256];
   rblxstb_uint i,j,s;
   crc = ~crc;

   if (crc_table[1] == 0)
      for(i=0; i < 256; i++) {
         for (s=i, j=0; j < 8; ++j)
            s = (s >> 1) ^ (s & 1 ? 0xedb88320 : 0);
         crc_table[i] = s;
      }
   for (i=0; i < len; ++i)
      crc = (crc >> 8) ^ crc_table[buffer[i] ^ (crc & 0xff)];
   return ~crc;
}

rblxstb_uint rblxstb_crc32(unsigned char *buffer, rblxstb_uint len)
{
   return rblxstb_crc32_block(0, buffer, len);
}

rblxstb_uint rblxstb_adler32(rblxstb_uint adler32, rblxstb_uchar *buffer, rblxstb_uint buflen)
{
   const unsigned long ADLER_MOD = 65521;
   unsigned long s1 = adler32 & 0xffff, s2 = adler32 >> 16;
   unsigned long blocklen, i;

   blocklen = buflen % 5552;
   while (buflen) {
      for (i=0; i + 7 < blocklen; i += 8) {
         s1 += buffer[0], s2 += s1;
         s1 += buffer[1], s2 += s1;
         s1 += buffer[2], s2 += s1;
         s1 += buffer[3], s2 += s1;
         s1 += buffer[4], s2 += s1;
         s1 += buffer[5], s2 += s1;
         s1 += buffer[6], s2 += s1;
         s1 += buffer[7], s2 += s1;

         buffer += 8;
      }

      for (; i < blocklen; ++i)
         s1 += *buffer++, s2 += s1;

      s1 %= ADLER_MOD, s2 %= ADLER_MOD;
      buflen -= blocklen;
      blocklen = 5552;
   }
   return (s2 << 16) + s1;
}

static void rblxstb__sha1(rblxstb_uchar *chunk, rblxstb_uint h[5])
{
   int i;
   rblxstb_uint a,b,c,d,e;
   rblxstb_uint w[80];

   for (i=0; i < 16; ++i)
      w[i] = rblxstb_big32(&chunk[i*4]);
   for (i=16; i < 80; ++i) {
      rblxstb_uint t;
      t = w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16];
      w[i] = (t + t) | (t >> 31);
   }

   a = h[0];
   b = h[1];
   c = h[2];
   d = h[3];
   e = h[4];

   #define RBLXSTB__SHA1(k,f)                                            \
   {                                                                 \
      rblxstb_uint temp = (a << 5) + (a >> 27) + (f) + e + (k) + w[i];  \
      e = d;                                                       \
      d = c;                                                     \
      c = (b << 30) + (b >> 2);                               \
      b = a;                                              \
      a = temp;                                    \
   }

   i=0;
   for (; i < 20; ++i) RBLXSTB__SHA1(0x5a827999, d ^ (b & (c ^ d))       );
   for (; i < 40; ++i) RBLXSTB__SHA1(0x6ed9eba1, b ^ c ^ d               );
   for (; i < 60; ++i) RBLXSTB__SHA1(0x8f1bbcdc, (b & c) + (d & (b ^ c)) );
   for (; i < 80; ++i) RBLXSTB__SHA1(0xca62c1d6, b ^ c ^ d               );

   #undef RBLXSTB__SHA1

   h[0] += a;
   h[1] += b;
   h[2] += c;
   h[3] += d;
   h[4] += e;
}

void rblxstb_sha1(rblxstb_uchar output[20], rblxstb_uchar *buffer, rblxstb_uint len)
{
   unsigned char final_block[128];
   rblxstb_uint end_start, final_len, j;
   int i;

   rblxstb_uint h[5];

   h[0] = 0x67452301;
   h[1] = 0xefcdab89;
   h[2] = 0x98badcfe;
   h[3] = 0x10325476;
   h[4] = 0xc3d2e1f0;

   // we need to write padding to the last one or two
   // blocks, so build those first into 'final_block'

   // we have to write one special byte, plus the 8-byte length

   // compute the block where the data runs out
   end_start = len & ~63;

   // compute the earliest we can encode the length
   if (((len+9) & ~63) == end_start) {
      // it all fits in one block, so fill a second-to-last block
      end_start -= 64;
   }

   final_len = end_start + 128;

   // now we need to copy the data in
   assert(end_start + 128 >= len+9);
   assert(end_start < len || len < 64-9);

   j = 0;
   if (end_start > len)
      j = (rblxstb_uint) - (int) end_start;

   for (; end_start + j < len; ++j)
      final_block[j] = buffer[end_start + j];
   final_block[j++] = 0x80;
   while (j < 128-5) // 5 byte length, so write 4 extra padding bytes
      final_block[j++] = 0;
   // big-endian size
   final_block[j++] = len >> 29;
   final_block[j++] = len >> 21;
   final_block[j++] = len >> 13;
   final_block[j++] = len >>  5;
   final_block[j++] = len <<  3;
   assert(j == 128 && end_start + j == final_len);

   for (j=0; j < final_len; j += 64) { // 512-bit chunks
      if (j+64 >= end_start+64)
         rblxstb__sha1(&final_block[j - end_start], h);
      else
         rblxstb__sha1(&buffer[j], h);
   }

   for (i=0; i < 5; ++i) {
      output[i*4 + 0] = h[i] >> 24;
      output[i*4 + 1] = h[i] >> 16;
      output[i*4 + 2] = h[i] >>  8;
      output[i*4 + 3] = h[i] >>  0;
   }
}

#ifdef _MSC_VER
int rblxstb_sha1_file(rblxstb_uchar output[20], char *file)
{
   int i;
   rblxstb_uint64 length=0;
   unsigned char buffer[128];

   FILE *f = rblxstb__fopen(file, "rb");
   rblxstb_uint h[5];

   if (f == NULL) return 0; // file not found

   h[0] = 0x67452301;
   h[1] = 0xefcdab89;
   h[2] = 0x98badcfe;
   h[3] = 0x10325476;
   h[4] = 0xc3d2e1f0;

   for(;;) {
      size_t n = fread(buffer, 1, 64, f);
      if (n == 64) {
         rblxstb__sha1(buffer, h);
         length += n;
      } else {
         int block = 64;

         length += n;

         buffer[n++] = 0x80;

         // if there isn't enough room for the length, double the block
         if (n + 8 > 64)
            block = 128;

         // pad to end
         memset(buffer+n, 0, block-8-n);

         i = block - 8;
         buffer[i++] = (rblxstb_uchar) (length >> 53);
         buffer[i++] = (rblxstb_uchar) (length >> 45);
         buffer[i++] = (rblxstb_uchar) (length >> 37);
         buffer[i++] = (rblxstb_uchar) (length >> 29);
         buffer[i++] = (rblxstb_uchar) (length >> 21);
         buffer[i++] = (rblxstb_uchar) (length >> 13);
         buffer[i++] = (rblxstb_uchar) (length >>  5);
         buffer[i++] = (rblxstb_uchar) (length <<  3);
         assert(i == block);
         rblxstb__sha1(buffer, h);
         if (block == 128)
            rblxstb__sha1(buffer+64, h);
         else
            assert(block == 64);
         break;
      }
   }
   fclose(f);

   for (i=0; i < 5; ++i) {
      output[i*4 + 0] = h[i] >> 24;
      output[i*4 + 1] = h[i] >> 16;
      output[i*4 + 2] = h[i] >>  8;
      output[i*4 + 3] = h[i] >>  0;
   }

   return 1;
}
#endif // _MSC_VER

// client can truncate this wherever they like
void rblxstb_sha1_readable(char display[27], unsigned char sha[20])
{
   char encoding[65] = "0123456789abcdefghijklmnopqrstuv"
                       "wxyzABCDEFGHIJKLMNOPQRSTUVWXYZ%$";
   int num_bits = 0, acc=0;
   int i=0,o=0;
   while (o < 26) {
      int v;
      // expand the accumulator
      if (num_bits < 6) {
         assert(i != 20);
         acc += sha[i++] << num_bits;
         num_bits += 8;
      }
      v = acc & ((1 << 6) - 1);
      display[o++] = encoding[v];
      acc >>= 6;
      num_bits -= 6;
   }
   assert(num_bits == 20*8 - 26*6);
   display[o++] = encoding[acc];
}

#endif // RBLXSTB_DEFINE

///////////////////////////////////////////////////////////
//
// simplified WINDOWS registry interface... hopefully
// we'll never actually use this?

#if defined(_WIN32)

RBLXSTB_EXTERN void * rblxstb_reg_open(const char *mode, const char *where); // mode: "rHKLM" or "rHKCU" or "w.."
RBLXSTB_EXTERN void   rblxstb_reg_close(void *reg);
RBLXSTB_EXTERN int    rblxstb_reg_read(void *zreg, const char *str, void *data, unsigned long len);
RBLXSTB_EXTERN int    rblxstb_reg_read_string(void *zreg, const char *str, char *data, int len);
RBLXSTB_EXTERN void   rblxstb_reg_write(void *zreg, const char *str, const void *data, unsigned long len);
RBLXSTB_EXTERN void   rblxstb_reg_write_string(void *zreg, const char *str, const char *data);

#if defined(RBLXSTB_DEFINE) && !defined(RBLXSTB_NO_REGISTRY)

#define RBLXSTB_HAS_REGISTRY

#ifndef _WINDOWS_

#define HKEY void *

RBLXSTB_EXTERN __declspec(dllimport) long __stdcall RegCloseKey ( HKEY hKey );
RBLXSTB_EXTERN __declspec(dllimport) long __stdcall RegCreateKeyExA ( HKEY hKey, const char * lpSubKey,
    int  Reserved, char * lpClass, int  dwOptions,
    int samDesired, void *lpSecurityAttributes,     HKEY * phkResult,     int * lpdwDisposition );
RBLXSTB_EXTERN __declspec(dllimport) long __stdcall RegDeleteKeyA ( HKEY hKey, const char * lpSubKey );
RBLXSTB_EXTERN __declspec(dllimport) long __stdcall RegQueryValueExA ( HKEY hKey, const char * lpValueName,
    int * lpReserved, unsigned long * lpType, unsigned char * lpData, unsigned long * lpcbData );
RBLXSTB_EXTERN __declspec(dllimport) long __stdcall RegSetValueExA ( HKEY hKey, const char * lpValueName,
    int  Reserved, int  dwType, const unsigned char* lpData, int  cbData );
RBLXSTB_EXTERN __declspec(dllimport) long __stdcall  RegOpenKeyExA ( HKEY hKey, const char * lpSubKey,
    int ulOptions, int samDesired, HKEY * phkResult );

#endif // _WINDOWS_

#define RBLXSTB__REG_OPTION_NON_VOLATILE  0
#define RBLXSTB__REG_KEY_ALL_ACCESS       0x000f003f
#define RBLXSTB__REG_KEY_READ             0x00020019

#ifdef _M_AMD64
#define RBLXSTB__HKEY_CURRENT_USER        0x80000001ull
#define RBLXSTB__HKEY_LOCAL_MACHINE       0x80000002ull
#else
#define RBLXSTB__HKEY_CURRENT_USER        0x80000001
#define RBLXSTB__HKEY_LOCAL_MACHINE       0x80000002
#endif

void *rblxstb_reg_open(const char *mode, const char *where)
{
   long res;
   HKEY base;
   HKEY zreg;
   if (!rblxstb_stricmp(mode+1, "cu") || !rblxstb_stricmp(mode+1, "hkcu"))
      base = (HKEY) RBLXSTB__HKEY_CURRENT_USER;
   else if (!rblxstb_stricmp(mode+1, "lm") || !rblxstb_stricmp(mode+1, "hklm"))
      base = (HKEY) RBLXSTB__HKEY_LOCAL_MACHINE;
   else
      return NULL;

   if (mode[0] == 'r')
      res = RegOpenKeyExA(base, where, 0, RBLXSTB__REG_KEY_READ, &zreg);
   else if (mode[0] == 'w')
      res = RegCreateKeyExA(base, where,  0, NULL, RBLXSTB__REG_OPTION_NON_VOLATILE, RBLXSTB__REG_KEY_ALL_ACCESS, NULL, &zreg, NULL);
   else
      return NULL;

   return res ? NULL : zreg;
}

void rblxstb_reg_close(void *reg)
{
   RegCloseKey((HKEY) reg);
}

#define RBLXSTB__REG_SZ         1
#define RBLXSTB__REG_BINARY     3
#define RBLXSTB__REG_DWORD      4

int rblxstb_reg_read(void *zreg, const char *str, void *data, unsigned long len)
{
   unsigned long type;
   unsigned long alen = len;
   if (0 == RegQueryValueExA((HKEY) zreg, str, 0, &type, (unsigned char *) data, &len))
      if (type == RBLXSTB__REG_BINARY || type == RBLXSTB__REG_SZ || type == RBLXSTB__REG_DWORD) {
         if (len < alen)
            *((char *) data + len) = 0;
         return 1;
      }
   return 0;
}

void rblxstb_reg_write(void *zreg, const char *str, const void *data, unsigned long len)
{
   if (zreg)
      RegSetValueExA((HKEY) zreg, str, 0, RBLXSTB__REG_BINARY, (const unsigned char *) data, len);
}

int rblxstb_reg_read_string(void *zreg, const char *str, char *data, int len)
{
   if (!rblxstb_reg_read(zreg, str, data, len)) return 0;
   data[len-1] = 0; // force a 0 at the end of the string no matter what
   return 1;
}

void rblxstb_reg_write_string(void *zreg, const char *str, const char *data)
{
   if (zreg)
      RegSetValueExA((HKEY) zreg, str, 0, RBLXSTB__REG_SZ, (const unsigned char *)  data, (int) strlen(data)+1);
}
#endif  // RBLXSTB_DEFINE
#endif  // _WIN32


//////////////////////////////////////////////////////////////////////////////
//
//     rblxstb_cfg - This is like the registry, but the config info
//               is all stored in plain old files where we can
//               backup and restore them easily. The LOCATION of
//               the config files is gotten from... the registry!

#ifndef RBLXSTB_NO_STB_STRINGS
typedef struct rblxstb_cfg_st rblxstb_cfg;

RBLXSTB_EXTERN rblxstb_cfg * rblxstb_cfg_open(char *config, const char *mode); // mode = "r", "w"
RBLXSTB_EXTERN void      rblxstb_cfg_close(rblxstb_cfg *cfg);
RBLXSTB_EXTERN int       rblxstb_cfg_read(rblxstb_cfg *cfg, char *key, void *value, int len);
RBLXSTB_EXTERN void      rblxstb_cfg_write(rblxstb_cfg *cfg, char *key, void *value, int len);
RBLXSTB_EXTERN int       rblxstb_cfg_read_string(rblxstb_cfg *cfg, char *key, char *value, int len);
RBLXSTB_EXTERN void      rblxstb_cfg_write_string(rblxstb_cfg *cfg, char *key, char *value);
RBLXSTB_EXTERN int       rblxstb_cfg_delete(rblxstb_cfg *cfg, char *key);
RBLXSTB_EXTERN void      rblxstb_cfg_set_directory(char *dir);

#ifdef RBLXSTB_DEFINE

typedef struct
{
   char *key;
   void *value;
   int value_len;
} rblxstb__cfg_item;

struct rblxstb_cfg_st
{
   rblxstb__cfg_item *data;
   char *loaded_file;   // this needs to be freed
   FILE *f; // write the data to this file on close
};

static const char *rblxstb__cfg_sig = "sTbCoNfIg!\0\0";
static char rblxstb__cfg_dir[512];
RBLXSTB_EXTERN void rblxstb_cfg_set_directory(char *dir)
{
   rblxstb_p_strcpy_s(rblxstb__cfg_dir, sizeof(rblxstb__cfg_dir), dir);
}

RBLXSTB_EXTERN rblxstb_cfg * rblxstb_cfg_open(char *config, const char *mode)
{
   size_t len;
   rblxstb_cfg *z;
   char file[512];
   if (mode[0] != 'r' && mode[0] != 'w') return NULL;

   if (!rblxstb__cfg_dir[0]) {
      #ifdef _WIN32
      rblxstb_p_strcpy_s(rblxstb__cfg_dir, sizeof(rblxstb__cfg_dir), "c:/rblxstb");
      #else
      strcpy(rblxstb__cfg_dir, "~/.rblxstbconfig");
      #endif

      #ifdef RBLXSTB_HAS_REGISTRY
      {
         void *reg = rblxstb_reg_open("rHKLM", "Software\\SilverSpaceship\\rblxstb");
         if (reg) {
            rblxstb_reg_read_string(reg, "config_dir", rblxstb__cfg_dir, sizeof(rblxstb__cfg_dir));
            rblxstb_reg_close(reg);
         }
      }
      #endif
   }

   rblxstb_p_sprintf(file rblxstb_p_size(sizeof(file)), "%s/%s.cfg", rblxstb__cfg_dir, config);

   z = (rblxstb_cfg *) rblxstb_malloc(0, sizeof(*z));
   z->data = NULL;

   z->loaded_file = rblxstb_filec(file, &len);
   if (z->loaded_file) {
      char *s = z->loaded_file;
      if (!memcmp(s, rblxstb__cfg_sig, 12)) {
         char *s = z->loaded_file + 12;
         while (s < z->loaded_file + len) {
            rblxstb__cfg_item a;
            int n = *(rblxstb_int16 *) s;
            a.key = s+2;
            s = s+2 + n;
            a.value_len = *(int *) s;
            s += 4;
            a.value = s;
            s += a.value_len;
            rblxstb_arr_push(z->data, a);
         }
         assert(s == z->loaded_file + len);
      }
   }

   if (mode[0] == 'w')
      z->f = rblxstb_p_fopen(file, "wb");
   else
      z->f = NULL;

   return z;
}

void rblxstb_cfg_close(rblxstb_cfg *z)
{
   if (z->f) {
      int i;
      // write the file out
      fwrite(rblxstb__cfg_sig, 12, 1, z->f);
      for (i=0; i < rblxstb_arr_len(z->data); ++i) {
         rblxstb_int16 n = (rblxstb_int16) strlen(z->data[i].key)+1;
         fwrite(&n, 2, 1, z->f);
         fwrite(z->data[i].key, n, 1, z->f);
         fwrite(&z->data[i].value_len, 4, 1, z->f);
         fwrite(z->data[i].value, z->data[i].value_len, 1, z->f);
      }
      fclose(z->f);
   }
   rblxstb_arr_free(z->data);
   rblxstb_free(z);
}

int rblxstb_cfg_read(rblxstb_cfg *z, char *key, void *value, int len)
{
   int i;
   for (i=0; i < rblxstb_arr_len(z->data); ++i) {
      if (!rblxstb_stricmp(z->data[i].key, key)) {
         int n = rblxstb_min(len, z->data[i].value_len);
         memcpy(value, z->data[i].value, n);
         if (n < len)
            *((char *) value + n) = 0;
         return 1;
      }
   }
   return 0;
}

void rblxstb_cfg_write(rblxstb_cfg *z, char *key, void *value, int len)
{
   int i;
   for (i=0; i < rblxstb_arr_len(z->data); ++i)
      if (!rblxstb_stricmp(z->data[i].key, key))
         break;
   if (i == rblxstb_arr_len(z->data)) {
      rblxstb__cfg_item p;
      p.key = rblxstb_strdup(key, z);
      p.value = NULL;
      p.value_len = 0;
      rblxstb_arr_push(z->data, p);
   }
   z->data[i].value = rblxstb_malloc(z, len);
   z->data[i].value_len = len;
   memcpy(z->data[i].value, value, len);
}

int rblxstb_cfg_delete(rblxstb_cfg *z, char *key)
{
   int i;
   for (i=0; i < rblxstb_arr_len(z->data); ++i)
      if (!rblxstb_stricmp(z->data[i].key, key)) {
         rblxstb_arr_fastdelete(z->data, i);
         return 1;
      }
   return 0;
}

int rblxstb_cfg_read_string(rblxstb_cfg *z, char *key, char *value, int len)
{
   if (!rblxstb_cfg_read(z, key, value, len)) return 0;
   value[len-1] = 0;
   return 1;
}

void rblxstb_cfg_write_string(rblxstb_cfg *z, char *key, char *value)
{
   rblxstb_cfg_write(z, key, value, (int) strlen(value)+1);
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
//     rblxstb_dirtree  - load a description of a directory tree
//                      uses a cache and stat()s the directories for changes
//                      MUCH faster on NTFS, _wrong_ on FAT32, so should
//                      ignore the db on FAT32

#ifdef _WIN32

typedef struct
{
   char   * path;           // full path from passed-in root
   time_t   last_modified;
   int      num_files;
   int      flag;
} rblxstb_dirtree_dir;

typedef struct
{
   char *name;              // name relative to path
   int   dir;               // index into dirs[] array
   rblxstb_int64 size;      // size, max 4GB
   time_t   last_modified;
   int      flag;
} rblxstb_dirtree_file;

typedef struct
{
   rblxstb_dirtree_dir  *dirs;
   rblxstb_dirtree_file *files;

   // internal use
   void             * string_pool;   // used to free data en masse
} rblxstb_dirtree;

extern void         rblxstb_dirtree_free          ( rblxstb_dirtree *d );
extern rblxstb_dirtree *rblxstb_dirtree_get           ( char *dir);
extern rblxstb_dirtree *rblxstb_dirtree_get_dir       ( char *dir, char *cache_dir);
extern rblxstb_dirtree *rblxstb_dirtree_get_with_file ( char *dir, char *cache_file);

// get a list of all the files recursively underneath 'dir'
//
// cache_file is used to store a copy of the directory tree to speed up
// later calls. It must be unique to 'dir' and the current working
// directory! Otherwise who knows what will happen (a good solution
// is to put it _in_ dir, but this API doesn't force that).
//
// Also, it might be possible to break this if you have two different processes
// do a call to rblxstb_dirtree_get() with the same cache file at about the same
// time, but I _think_ it might just work.

// i needed to build an identical data structure representing the state of
// a mirrored copy WITHOUT bothering to rescan it (i.e. we're mirroring to
// it WITHOUT scanning it, e.g. it's over the net), so this requires access
// to all of the innards.
extern void rblxstb_dirtree_db_add_dir(rblxstb_dirtree *active, char *path, time_t last);
extern void rblxstb_dirtree_db_add_file(rblxstb_dirtree *active, char *name, int dir, rblxstb_int64 size, time_t last);
extern void rblxstb_dirtree_db_read(rblxstb_dirtree *target, char *filename, char *dir);
extern void rblxstb_dirtree_db_write(rblxstb_dirtree *target, char *filename, char *dir);

#ifdef RBLXSTB_DEFINE
static void rblxstb__dirtree_add_dir(char *path, time_t last, rblxstb_dirtree *active)
{
   rblxstb_dirtree_dir d;
   d.last_modified = last;
   d.num_files = 0;
   d.path = rblxstb_strdup(path, active->string_pool);
   rblxstb_arr_push(active->dirs, d);
}

static void rblxstb__dirtree_add_file(char *name, int dir, rblxstb_int64 size, time_t last, rblxstb_dirtree *active)
{
   rblxstb_dirtree_file f;
   f.dir = dir;
   f.size = size;
   f.last_modified = last;
   f.name = rblxstb_strdup(name, active->string_pool);
   ++active->dirs[dir].num_files;
   rblxstb_arr_push(active->files, f);
}

// version 02 supports > 4GB files
static char rblxstb__signature[12] = { 's', 'T', 'b', 'D', 'i', 'R', 't', 'R', 'e', 'E', '0', '2' };

static void rblxstb__dirtree_save_db(char *filename, rblxstb_dirtree *data, char *root)
{
   int i, num_dirs_final=0, num_files_final;
   char *info = root ? root : (char*)"";
   int *remap;
   FILE *f = rblxstb_p_fopen(filename, "wb");
   if (!f) return;

   fwrite(rblxstb__signature, sizeof(rblxstb__signature), 1, f);
   fwrite(info, strlen(info)+1, 1, f);
   // need to be slightly tricky and not write out NULLed directories, nor the root

   // build remapping table of all dirs we'll be writing out
   remap = (int *) malloc(sizeof(remap[0]) * rblxstb_arr_len(data->dirs));
   for (i=0; i < rblxstb_arr_len(data->dirs); ++i) {
      if (data->dirs[i].path == NULL || (root && 0==rblxstb_stricmp(data->dirs[i].path, root))) {
         remap[i] = -1;
      } else {
         remap[i] = num_dirs_final++;
      }
   }

   fwrite(&num_dirs_final, 4, 1, f);
   for (i=0; i < rblxstb_arr_len(data->dirs); ++i) {
      if (remap[i] >= 0) {
         fwrite(&data->dirs[i].last_modified, 4, 1, f);
         rblxstb_fput_string(f, data->dirs[i].path);
      }
   }

   num_files_final = 0;
   for (i=0; i < rblxstb_arr_len(data->files); ++i)
      if (remap[data->files[i].dir] >= 0 && data->files[i].name)
         ++num_files_final;

   fwrite(&num_files_final, 4, 1, f);
   for (i=0; i < rblxstb_arr_len(data->files); ++i) {
      if (remap[data->files[i].dir] >= 0 && data->files[i].name) {
         rblxstb_fput_ranged(f, remap[data->files[i].dir], 0, num_dirs_final);
         rblxstb_fput_varlen64(f, data->files[i].size);
         fwrite(&data->files[i].last_modified, 4, 1, f);
         rblxstb_fput_string(f, data->files[i].name);
      }
   }

   fclose(f);
}

// note: stomps any existing data, rather than appending
static void rblxstb__dirtree_load_db(char *filename, rblxstb_dirtree *data, char *dir)
{
   char sig[2048];
   int i,n;
   FILE *f = rblxstb_p_fopen(filename, "rb");

   if (!f) return;

   data->string_pool = rblxstb_malloc(0,1);

   fread(sig, sizeof(rblxstb__signature), 1, f);
   if (memcmp(rblxstb__signature, sig, sizeof(rblxstb__signature))) { fclose(f); return; }
   if (!fread(sig, strlen(dir)+1, 1, f))                    { fclose(f); return; }
   if (rblxstb_stricmp(sig,dir))                                { fclose(f); return; }

   // we can just read them straight in, because they're guaranteed to be valid
   fread(&n, 4, 1, f);
   rblxstb_arr_setlen(data->dirs, n);
   for(i=0; i < rblxstb_arr_len(data->dirs); ++i) {
      fread(&data->dirs[i].last_modified, 4, 1, f);
      data->dirs[i].path = rblxstb_fget_string(f, data->string_pool);
      if (data->dirs[i].path == NULL) goto bail;
   }
   fread(&n, 4, 1, f);
   rblxstb_arr_setlen(data->files, n);
   for (i=0; i < rblxstb_arr_len(data->files); ++i) {
      data->files[i].dir  = rblxstb_fget_ranged(f, 0, rblxstb_arr_len(data->dirs));
      data->files[i].size = rblxstb_fget_varlen64(f);
      fread(&data->files[i].last_modified, 4, 1, f);
      data->files[i].name = rblxstb_fget_string(f, data->string_pool);
      if (data->files[i].name == NULL) goto bail;
   }

   if (0) {
      bail:
         rblxstb_arr_free(data->dirs);
         rblxstb_arr_free(data->files);
   }
   fclose(f);
}

FILE *hlog;

static int rblxstb__dircount, rblxstb__dircount_mask, rblxstb__showfile;
static void rblxstb__dirtree_scandir(char *path, time_t last_time, rblxstb_dirtree *active)
{
   // this is dumb depth first; theoretically it might be faster
   // to fully traverse each directory before visiting its children,
   // but it's complicated and didn't seem like a gain in the test app

   int n;

   struct _wfinddatai64_t c_file;
   long hFile;
   rblxstb__wchar full_path[1024];
   int has_slash;
   if (rblxstb__showfile) printf("<");

   has_slash = (path[0] && path[strlen(path)-1] == '/');

   // @TODO: do this concatenation without using swprintf to avoid this mess:
#if (defined(_MSC_VER) && _MSC_VER < 1400) // || (defined(__clang__))
   // confusingly, Windows Kits\10 needs to go down this path?!?
   // except now it doesn't, I don't know what changed
   if (has_slash)
      swprintf(full_path, L"%s*", rblxstb__from_utf8(path));
   else
      swprintf(full_path, L"%s/*", rblxstb__from_utf8(path));
#else
   if (has_slash)
      swprintf((wchar_t *) full_path, (size_t) 1024, L"%s*", (wchar_t *) rblxstb__from_utf8(path));
   else
      swprintf((wchar_t *) full_path, (size_t) 1024, L"%s/*", (wchar_t *) rblxstb__from_utf8(path));
#endif

   // it's possible this directory is already present: that means it was in the
   // cache, but its parent wasn't... in that case, we're done with it
   if (rblxstb__showfile) printf("C[%d]", rblxstb_arr_len(active->dirs));
   for (n=0; n < rblxstb_arr_len(active->dirs); ++n)
      if (0 == rblxstb_stricmp(active->dirs[n].path, path)) {
         if (rblxstb__showfile) printf("D");
         return;
      }
   if (rblxstb__showfile) printf("E");

   // otherwise, we need to add it
   rblxstb__dirtree_add_dir(path, last_time, active);
   n = rblxstb_arr_lastn(active->dirs);

   if (rblxstb__showfile) printf("[");
   if( (hFile = (long) _wfindfirsti64( (wchar_t *) full_path, &c_file )) != -1L ) {
      do {
         if (rblxstb__showfile) printf(")");
         if (c_file.attrib & _A_SUBDIR) {
            // ignore subdirectories starting with '.', e.g. "." and ".."
            if (c_file.name[0] != '.') {
               char *new_path = (char *) full_path;
               char *temp = rblxstb__to_utf8((rblxstb__wchar *) c_file.name);

               if (has_slash)
                  rblxstb_p_sprintf(new_path rblxstb_p_size(sizeof(full_path)), "%s%s", path, temp);
               else
                  rblxstb_p_sprintf(new_path rblxstb_p_size(sizeof(full_path)), "%s/%s", path, temp);

               if (rblxstb__dircount_mask) {
                  ++rblxstb__dircount;
                  if (!(rblxstb__dircount & rblxstb__dircount_mask)) {
                     char dummy_path[128], *pad;
                     rblxstb_strncpy(dummy_path, new_path, sizeof(dummy_path)-1);
                     if (strlen(dummy_path) > 96) {
                        rblxstb_p_strcpy_s(dummy_path+96/2-1,128, "...");
                        rblxstb_p_strcpy_s(dummy_path+96/2+2,128, new_path + strlen(new_path)-96/2+2);
                     }
                     pad = dummy_path + strlen(dummy_path);
                     while (pad < dummy_path+98)
                        *pad++ = ' ';
                     *pad = 0;
                     printf("%s\r", dummy_path);
                     #if 0
                     if (hlog == 0) {
                        hlog = rblxstb_p_fopen("c:/x/temp.log", "w");
                        fprintf(hlog, "%s\n", dummy_path);
                     }
                     #endif
                  }
               }

               rblxstb__dirtree_scandir(new_path, c_file.time_write, active);
            }
         } else {
            char *temp = rblxstb__to_utf8((rblxstb__wchar *) c_file.name);
            rblxstb__dirtree_add_file(temp, n, c_file.size, c_file.time_write, active);
         }
         if (rblxstb__showfile) printf("(");
      } while( _wfindnexti64( hFile, &c_file ) == 0 );
      if (rblxstb__showfile) printf("]");
      _findclose( hFile );
   }
   if (rblxstb__showfile) printf(">\n");
}

// scan the database and see if it's all valid
static int rblxstb__dirtree_update_db(rblxstb_dirtree *db, rblxstb_dirtree *active)
{
   int changes_detected = RBLXSTB_FALSE;
   int i;
   int *remap;
   int *rescan=NULL;
   remap = (int *) malloc(sizeof(remap[0]) * rblxstb_arr_len(db->dirs));
   memset(remap, 0, sizeof(remap[0]) * rblxstb_arr_len(db->dirs));
   rescan = NULL;

   for (i=0; i < rblxstb_arr_len(db->dirs); ++i) {
      struct _stat info;
      if (rblxstb__dircount_mask) {
         ++rblxstb__dircount;
         if (!(rblxstb__dircount & rblxstb__dircount_mask)) {
            printf(".");
         }
      }
      if (0 == _stat(db->dirs[i].path, &info)) {
         if (info.st_mode & _S_IFDIR) {
            // it's still a directory, as expected
            int n = abs((int) (info.st_mtime - db->dirs[i].last_modified));
            if (n > 1 && n != 3600) {  // the 3600 is a hack because sometimes this jumps for no apparent reason, even when no time zone or DST issues are at play
               // it's changed! force a rescan
               // we don't want to scan it until we've stat()d its
               // subdirs, though, so we queue it
               if (rblxstb__showfile) printf("Changed: %s - %08x:%08x\n", db->dirs[i].path, (unsigned int) db->dirs[i].last_modified, (unsigned int) info.st_mtime);
               rblxstb_arr_push(rescan, i);
               // update the last_mod time
               db->dirs[i].last_modified = info.st_mtime;
               // ignore existing files in this dir
               remap[i] = -1;
               changes_detected = RBLXSTB_TRUE;
            } else {
               // it hasn't changed, just copy it through unchanged
               rblxstb__dirtree_add_dir(db->dirs[i].path, db->dirs[i].last_modified, active);
               remap[i] = rblxstb_arr_lastn(active->dirs);
            }
         } else {
            // this path used to refer to a directory, but now it's a file!
            // assume that the parent directory is going to be forced to rescan anyway
            goto delete_entry;
         }
      } else {
        delete_entry:
         // directory no longer exists, so don't copy it
         // we don't free it because it's in the string pool now
         db->dirs[i].path = NULL;
         remap[i] = -1;
         changes_detected = RBLXSTB_TRUE;
      }
   }

   // at this point, we have:
   //
   //   <rescan> holds a list of directory indices that need to be scanned due to being out of date
   //   <remap> holds the directory index in <active> for each dir in <db>, if it exists; -1 if not
   //           directories in <rescan> are not in <active> yet

   // so we can go ahead and remap all the known files right now
   for (i=0; i < rblxstb_arr_len(db->files); ++i) {
      int dir = db->files[i].dir;
      if (remap[dir] >= 0) {
         rblxstb__dirtree_add_file(db->files[i].name, remap[dir], db->files[i].size, db->files[i].last_modified, active);
      }
   }

   // at this point we're done with db->files, and done with remap
   free(remap);

   // now scan those directories using the standard scan
   for (i=0; i < rblxstb_arr_len(rescan); ++i) {
      int z = rescan[i];
      rblxstb__dirtree_scandir(db->dirs[z].path, db->dirs[z].last_modified, active);
   }
   rblxstb_arr_free(rescan);

   return changes_detected;
}

static void rblxstb__dirtree_free_raw(rblxstb_dirtree *d)
{
   rblxstb_free(d->string_pool);
   rblxstb_arr_free(d->dirs);
   rblxstb_arr_free(d->files);
}

rblxstb_dirtree *rblxstb_dirtree_get_with_file(char *dir, char *cache_file)
{
   rblxstb_dirtree *output = (rblxstb_dirtree *) malloc(sizeof(*output));
   rblxstb_dirtree db,active;
   int prev_dir_count, cache_mismatch;

   char *stripped_dir; // store the directory name without a trailing '/' or '\\'

   // load the database of last-known state on disk
   db.string_pool = NULL;
   db.files = NULL;
   db.dirs = NULL;

   stripped_dir = rblxstb_strip_final_slash(rblxstb_p_strdup(dir));

   if (cache_file != NULL)
      rblxstb__dirtree_load_db(cache_file, &db, stripped_dir);
   else if (rblxstb__showfile)
      printf("No cache file\n");

   active.files = NULL;
   active.dirs = NULL;
   active.string_pool = rblxstb_malloc(0,1); // @TODO: share string pools between both?

   // check all the directories in the database; make note if
   // anything we scanned had changed, and rescan those things
   cache_mismatch = rblxstb__dirtree_update_db(&db, &active);

   // check the root tree
   prev_dir_count = rblxstb_arr_len(active.dirs);  // record how many directories we've seen

   rblxstb__dirtree_scandir(stripped_dir, 0, &active);  // no last_modified time available for root

   if (rblxstb__dircount_mask)
      printf("                                                                              \r");

   // done with the DB; write it back out if any changes, i.e. either
   //      1. any inconsistency found between cached information and actual disk
   //   or 2. if scanning the root found any new directories--which we detect because
   //         more than one directory got added to the active db during that scan
   if (cache_mismatch || rblxstb_arr_len(active.dirs) > prev_dir_count+1)
      rblxstb__dirtree_save_db(cache_file, &active, stripped_dir);

   free(stripped_dir);

   rblxstb__dirtree_free_raw(&db);

   *output = active;
   return output;
}

rblxstb_dirtree *rblxstb_dirtree_get_dir(char *dir, char *cache_dir)
{
   int i;
   rblxstb_uint8 sha[20];
   char dir_lower[1024];
   char cache_file[1024],*s;
   if (cache_dir == NULL)
      return rblxstb_dirtree_get_with_file(dir, NULL);
   rblxstb_p_strcpy_s(dir_lower, sizeof(dir_lower), dir);
   rblxstb_tolower(dir_lower);
   rblxstb_sha1(sha, (unsigned char *) dir_lower, (unsigned int) strlen(dir_lower));
   rblxstb_p_strcpy_s(cache_file, sizeof(cache_file), cache_dir);
   s = cache_file + strlen(cache_file);
   if (s[-1] != '/' && s[-1] != '\\') *s++ = '/';
   rblxstb_p_strcpy_s(s, sizeof(cache_file), "dirtree_");
   s += strlen(s);
   for (i=0; i < 8; ++i) {
      char *hex = (char*)"0123456789abcdef";
      rblxstb_uint z = sha[i];
      *s++ = hex[z >> 4];
      *s++ = hex[z & 15];
   }
   rblxstb_p_strcpy_s(s, sizeof(cache_file), ".bin");
   return rblxstb_dirtree_get_with_file(dir, cache_file);
}

rblxstb_dirtree *rblxstb_dirtree_get(char *dir)
{
   char cache_dir[256];
   rblxstb_p_strcpy_s(cache_dir, sizeof(cache_dir), "c:/bindata");
   #ifdef RBLXSTB_HAS_REGISTRY
   {
      void *reg = rblxstb_reg_open("rHKLM", "Software\\SilverSpaceship\\rblxstb");
      if (reg) {
         rblxstb_reg_read(reg, "dirtree", cache_dir, sizeof(cache_dir));
         rblxstb_reg_close(reg);
      }
   }
   #endif
   return rblxstb_dirtree_get_dir(dir, cache_dir);
}

void rblxstb_dirtree_free(rblxstb_dirtree *d)
{
   rblxstb__dirtree_free_raw(d);
   free(d);
}

void rblxstb_dirtree_db_add_dir(rblxstb_dirtree *active, char *path, time_t last)
{
   rblxstb__dirtree_add_dir(path, last, active);
}

void rblxstb_dirtree_db_add_file(rblxstb_dirtree *active, char *name, int dir, rblxstb_int64 size, time_t last)
{
   rblxstb__dirtree_add_file(name, dir, size, last, active);
}

void rblxstb_dirtree_db_read(rblxstb_dirtree *target, char *filename, char *dir)
{
   char *s = rblxstb_strip_final_slash(rblxstb_p_strdup(dir));
   target->dirs = 0;
   target->files = 0;
   target->string_pool = 0;
   rblxstb__dirtree_load_db(filename, target, s);
   free(s);
}

void rblxstb_dirtree_db_write(rblxstb_dirtree *target, char *filename, char *dir)
{
   rblxstb__dirtree_save_db(filename, target, 0); // don't strip out any directories
}

#endif // RBLXSTB_DEFINE

#endif // _WIN32
#endif // RBLXSTB_NO_STB_STRINGS

//////////////////////////////////////////////////////////////////////////////
//
//  RBLXSTB_MALLOC_WRAPPER
//
//    you can use the wrapper functions with your own malloc wrapper,
//    or define RBLXSTB_MALLOC_WRAPPER project-wide to have
//    malloc/free/realloc/strdup all get vectored to it

// this has too many very specific error messages you could google for and find in rblxstb.h,
// so don't use it if they don't want any rblxstb.h-identifiable strings
#if defined(RBLXSTB_DEFINE) && !defined(RBLXSTB_NO_STB_STRINGS)

typedef struct
{
   void *p;
   char *file;
   int  line;
   size_t size;
} rblxstb_malloc_record;

#ifndef RBLXSTB_MALLOC_HISTORY_COUNT
#define RBLXSTB_MALLOC_HISTORY_COUNT 50 // 800 bytes
#endif

rblxstb_malloc_record *rblxstb__allocations;
static int rblxstb__alloc_size, rblxstb__alloc_limit, rblxstb__alloc_mask;
int rblxstb__alloc_count;

rblxstb_malloc_record rblxstb__alloc_history[RBLXSTB_MALLOC_HISTORY_COUNT];
int rblxstb__history_pos;

static int rblxstb__hashfind(void *p)
{
   rblxstb_uint32 h = rblxstb_hashptr(p);
   int s,n = h & rblxstb__alloc_mask;
   if (rblxstb__allocations[n].p == p)
      return n;
   s = rblxstb_rehash(h)|1;
   for(;;) {
      if (rblxstb__allocations[n].p == NULL)
         return -1;
      n = (n+s) & rblxstb__alloc_mask;
      if (rblxstb__allocations[n].p == p)
         return n;
   }
}

size_t rblxstb_wrapper_allocsize(void *p)
{
   int n = rblxstb__hashfind(p);
   if (n < 0) return 0;
   return rblxstb__allocations[n].size;
}

static int rblxstb__historyfind(void *p)
{
   int n = rblxstb__history_pos;
   int i;
   for (i=0; i < RBLXSTB_MALLOC_HISTORY_COUNT; ++i) {
      if (--n < 0) n = RBLXSTB_MALLOC_HISTORY_COUNT-1;
      if (rblxstb__alloc_history[n].p == p)
         return n;
   }
   return -1;
}

static void rblxstb__add_alloc(void *p, size_t sz, char *file, int line);
static void rblxstb__grow_alloc(void)
{
   int i,old_num = rblxstb__alloc_size;
   rblxstb_malloc_record *old = rblxstb__allocations;
   if (rblxstb__alloc_size == 0)
      rblxstb__alloc_size = 64;
   else
      rblxstb__alloc_size *= 2;

   rblxstb__allocations = (rblxstb_malloc_record *) rblxstb__realloc_raw(NULL, rblxstb__alloc_size * sizeof(rblxstb__allocations[0]));
   if (rblxstb__allocations == NULL)
      rblxstb_fatal("Internal error: couldn't grow malloc wrapper table");
   memset(rblxstb__allocations, 0, rblxstb__alloc_size * sizeof(rblxstb__allocations[0]));
   rblxstb__alloc_limit = (rblxstb__alloc_size*3)>>2;
   rblxstb__alloc_mask = rblxstb__alloc_size-1;

   rblxstb__alloc_count = 0;

   for (i=0; i < old_num; ++i)
      if (old[i].p > RBLXSTB_DEL) {
         rblxstb__add_alloc(old[i].p, old[i].size, old[i].file, old[i].line);
         assert(rblxstb__hashfind(old[i].p) >= 0);
      }
   for (i=0; i < old_num; ++i)
      if (old[i].p > RBLXSTB_DEL)
         assert(rblxstb__hashfind(old[i].p) >= 0);
   rblxstb__realloc_raw(old, 0);
}

static void rblxstb__add_alloc(void *p, size_t sz, char *file, int line)
{
   rblxstb_uint32 h;
   int n;
   if (rblxstb__alloc_count >= rblxstb__alloc_limit)
      rblxstb__grow_alloc();
   h = rblxstb_hashptr(p);
   n = h & rblxstb__alloc_mask;
   if (rblxstb__allocations[n].p > RBLXSTB_DEL) {
      int s = rblxstb_rehash(h)|1;
      do {
         n = (n+s) & rblxstb__alloc_mask;
      } while (rblxstb__allocations[n].p > RBLXSTB_DEL);
   }
   assert(rblxstb__allocations[n].p == NULL || rblxstb__allocations[n].p == RBLXSTB_DEL);
   rblxstb__allocations[n].p = p;
   rblxstb__allocations[n].size = sz;
   rblxstb__allocations[n].line = line;
   rblxstb__allocations[n].file = file;
   ++rblxstb__alloc_count;
}

static void rblxstb__remove_alloc(int n, char *file, int line)
{
   rblxstb__alloc_history[rblxstb__history_pos] = rblxstb__allocations[n];
   rblxstb__alloc_history[rblxstb__history_pos].file = file;
   rblxstb__alloc_history[rblxstb__history_pos].line = line;
   if (++rblxstb__history_pos == RBLXSTB_MALLOC_HISTORY_COUNT)
      rblxstb__history_pos = 0;
   rblxstb__allocations[n].p = RBLXSTB_DEL;
   --rblxstb__alloc_count;
}

void rblxstb_wrapper_malloc(void *p, size_t sz, char *file, int line)
{
   if (!p) return;
   rblxstb__add_alloc(p,sz,file,line);
}

void rblxstb_wrapper_free(void *p, char *file, int line)
{
   int n;

   if (p == NULL) return;

   n = rblxstb__hashfind(p);

   if (n >= 0)
      rblxstb__remove_alloc(n, file, line);
   else {
      // tried to free something we hadn't allocated!
      n = rblxstb__historyfind(p);
      assert(0); /* NOTREACHED */
      if (n >= 0)
         rblxstb_fatal("Attempted to free %d-byte block %p at %s:%d previously freed/realloced at %s:%d",
                       rblxstb__alloc_history[n].size, p,
                       file, line,
                       rblxstb__alloc_history[n].file, rblxstb__alloc_history[n].line);
      else
         rblxstb_fatal("Attempted to free unknown block %p at %s:%d", p, file,line);
   }
}

void rblxstb_wrapper_check(void *p)
{
   int n;

   if (p == NULL) return;

   n = rblxstb__hashfind(p);

   if (n >= 0) return;

   for (n=0; n < rblxstb__alloc_size; ++n)
      if (rblxstb__allocations[n].p == p)
         rblxstb_fatal("Internal error: pointer %p was allocated, but hash search failed", p);

   // tried to free something that wasn't allocated!
   n = rblxstb__historyfind(p);
   if (n >= 0)
      rblxstb_fatal("Checked %d-byte block %p previously freed/realloced at %s:%d",
                    rblxstb__alloc_history[n].size, p,
                    rblxstb__alloc_history[n].file, rblxstb__alloc_history[n].line);
   rblxstb_fatal("Checked unknown block %p");
}

void rblxstb_wrapper_realloc(void *p, void *q, size_t sz, char *file, int line)
{
   int n;
   if (p == NULL) { rblxstb_wrapper_malloc(q, sz, file, line); return; }
   if (q == NULL) return; // nothing happened

   n = rblxstb__hashfind(p);
   if (n == -1) {
      // tried to free something we hadn't allocated!
      // this is weird, though, because we got past the realloc!
      n = rblxstb__historyfind(p);
      assert(0); /* NOTREACHED */
      if (n >= 0)
         rblxstb_fatal("Attempted to realloc %d-byte block %p at %s:%d previously freed/realloced at %s:%d",
                       rblxstb__alloc_history[n].size, p,
                       file, line,
                       rblxstb__alloc_history[n].file, rblxstb__alloc_history[n].line);
      else
         rblxstb_fatal("Attempted to realloc unknown block %p at %s:%d", p, file,line);
   } else {
      if (q == p) {
         rblxstb__allocations[n].size = sz;
         rblxstb__allocations[n].file = file;
         rblxstb__allocations[n].line = line;
      } else {
         rblxstb__remove_alloc(n, file, line);
         rblxstb__add_alloc(q,sz,file,line);
      }
   }
}

void rblxstb_wrapper_listall(void (*func)(void *ptr, size_t sz, char *file, int line))
{
   int i;
   for (i=0; i < rblxstb__alloc_size; ++i)
      if (rblxstb__allocations[i].p > RBLXSTB_DEL)
         func(rblxstb__allocations[i].p   , rblxstb__allocations[i].size,
              rblxstb__allocations[i].file, rblxstb__allocations[i].line);
}

void rblxstb_wrapper_dump(char *filename)
{
   int i;
   FILE *f = rblxstb_p_fopen(filename, "w");
   if (!f) return;
   for (i=0; i < rblxstb__alloc_size; ++i)
      if (rblxstb__allocations[i].p > RBLXSTB_DEL)
         fprintf(f, "%p %7d - %4d %s\n",
            rblxstb__allocations[i].p   , (int) rblxstb__allocations[i].size,
            rblxstb__allocations[i].line, rblxstb__allocations[i].file);
}
#endif // RBLXSTB_DEFINE


//////////////////////////////////////////////////////////////////////////////
//
//                  rblxstb_pointer_set
//
//
// For data structures that support querying by key, data structure
// classes always hand-wave away the issue of what to do if two entries
// have the same key: basically, store a linked list of all the nodes
// which have the same key (a LISP-style list).
//
// The thing is, it's not that trivial. If you have an O(log n)
// lookup data structure, but then n/4 items have the same value,
// you don't want to spend O(n) time scanning that list when
// deleting an item if you already have a pointer to the item.
// (You have to spend O(n) time enumerating all the items with
// a given key, sure, and you can't accelerate deleting a particular
// item if you only have the key, not a pointer to the item.)
//
// I'm going to call this data structure, whatever it turns out to
// be, a "pointer set", because we don't store any associated data for
// items in this data structure, we just answer the question of
// whether an item is in it or not (it's effectively one bit per pointer).
// Technically they don't have to be pointers; you could cast ints
// to (void *) if you want, but you can't store 0 or 1 because of the
// hash table.
//
// Since the fastest data structure we might want to add support for
// identical-keys to is a hash table with O(1)-ish lookup time,
// that means that the conceptual "linked list of all items with
// the same indexed value" that we build needs to have the same
// performance; that way when we index a table we think is arbitrary
// ints, but in fact half of them are 0, we don't get screwed.
//
// Therefore, it needs to be a hash table, at least when it gets
// large. On the other hand, when the data has totally arbitrary ints
// or floats, there won't be many collisions, and we'll have tons of
// 1-item bitmaps. That will be grossly inefficient as hash tables;
// trade-off; the hash table is reasonably efficient per-item when
// it's large, but not when it's small. So we need to do something
// Judy-like and use different strategies depending on the size.
//
// Like Judy, we'll use the bottom bit to encode the strategy:
//
//      bottom bits:
//          00     -   direct pointer
//          01     -   4-item bucket (16 bytes, no length, NULLs)
//          10     -   N-item array
//          11     -   hash table

typedef struct rblxstb_ps rblxstb_ps;

RBLXSTB_EXTERN int      rblxstb_ps_find  (rblxstb_ps *ps, void *value);
RBLXSTB_EXTERN rblxstb_ps * rblxstb_ps_add   (rblxstb_ps *ps, void *value);
RBLXSTB_EXTERN rblxstb_ps * rblxstb_ps_remove(rblxstb_ps *ps, void *value);
RBLXSTB_EXTERN rblxstb_ps * rblxstb_ps_remove_any(rblxstb_ps *ps, void **value);
RBLXSTB_EXTERN void     rblxstb_ps_delete(rblxstb_ps *ps);
RBLXSTB_EXTERN int      rblxstb_ps_count (rblxstb_ps *ps);

RBLXSTB_EXTERN rblxstb_ps * rblxstb_ps_copy  (rblxstb_ps *ps);
RBLXSTB_EXTERN int      rblxstb_ps_subset(rblxstb_ps *bigger, rblxstb_ps *smaller);
RBLXSTB_EXTERN int      rblxstb_ps_eq    (rblxstb_ps *p0, rblxstb_ps *p1);

RBLXSTB_EXTERN void ** rblxstb_ps_getlist  (rblxstb_ps *ps, int *count);
RBLXSTB_EXTERN int     rblxstb_ps_writelist(rblxstb_ps *ps, void **list, int size );

// enum and fastlist don't allocate storage, but you must consume the
// list before there's any chance the data structure gets screwed up;
RBLXSTB_EXTERN int     rblxstb_ps_enum     (rblxstb_ps *ps, void *data,
                                       int (*func)(void *value, void*data) );
RBLXSTB_EXTERN void ** rblxstb_ps_fastlist(rblxstb_ps *ps, int *count);
//  result:
//     returns a list, *count is the length of that list,
//     but some entries of the list may be invalid;
//     test with 'rblxstb_ps_fastlist_valid(x)'

#define rblxstb_ps_fastlist_valid(x)   ((rblxstb_uinta) (x) > 1)

#ifdef RBLXSTB_DEFINE

enum
{
   RBLXSTB_ps_direct = 0,
   RBLXSTB_ps_bucket = 1,
   RBLXSTB_ps_array  = 2,
   RBLXSTB_ps_hash   = 3,
};

#define RBLXSTB_BUCKET_SIZE  4

typedef struct
{
   void *p[RBLXSTB_BUCKET_SIZE];
} rblxstb_ps_bucket;
#define GetBucket(p)    ((rblxstb_ps_bucket *) ((char *) (p) - RBLXSTB_ps_bucket))
#define EncodeBucket(p) ((rblxstb_ps *) ((char *) (p) + RBLXSTB_ps_bucket))

static void rblxstb_bucket_free(rblxstb_ps_bucket *b)
{
   free(b);
}

static rblxstb_ps_bucket *rblxstb_bucket_create2(void *v0, void *v1)
{
   rblxstb_ps_bucket *b = (rblxstb_ps_bucket*) malloc(sizeof(*b));
   b->p[0] = v0;
   b->p[1] = v1;
   b->p[2] = NULL;
   b->p[3] = NULL;
   return b;
}

static rblxstb_ps_bucket * rblxstb_bucket_create3(void **v)
{
   rblxstb_ps_bucket *b = (rblxstb_ps_bucket*) malloc(sizeof(*b));
   b->p[0] = v[0];
   b->p[1] = v[1];
   b->p[2] = v[2];
   b->p[3] = NULL;
   return b;
}


// could use rblxstb_arr, but this will save us memory
typedef struct
{
   int count;
   void *p[1];
} rblxstb_ps_array;
#define GetArray(p)     ((rblxstb_ps_array *) ((char *) (p) - RBLXSTB_ps_array))
#define EncodeArray(p)  ((rblxstb_ps *) ((char *) (p) + RBLXSTB_ps_array))

static int rblxstb_ps_array_max = 13;

typedef struct
{
   int size, mask;
   int count, count_deletes;
   int grow_threshhold;
   int shrink_threshhold;
   int rehash_threshhold;
   int any_offset;
   void *table[1];
} rblxstb_ps_hash;
#define GetHash(p)      ((rblxstb_ps_hash *) ((char *) (p) - RBLXSTB_ps_hash))
#define EncodeHash(p)   ((rblxstb_ps *) ((char *) (p) + RBLXSTB_ps_hash))

#define rblxstb_ps_empty(v)   (((rblxstb_uint32) v) <= 1)

static rblxstb_ps_hash *rblxstb_ps_makehash(int size, int old_size, void **old_data)
{
   int i;
   rblxstb_ps_hash *h = (rblxstb_ps_hash *) malloc(sizeof(*h) + (size-1) * sizeof(h->table[0]));
   assert(rblxstb_is_pow2(size));
   h->size = size;
   h->mask = size-1;
   h->shrink_threshhold = (int) (0.3f * size);
   h->  grow_threshhold = (int) (0.8f * size);
   h->rehash_threshhold = (int) (0.9f * size);
   h->count = 0;
   h->count_deletes = 0;
   h->any_offset = 0;
   memset(h->table, 0, size * sizeof(h->table[0]));
   for (i=0; i < old_size; ++i)
        if (!rblxstb_ps_empty((size_t)old_data[i]))
         rblxstb_ps_add(EncodeHash(h), old_data[i]);
   return h;
}

void rblxstb_ps_delete(rblxstb_ps *ps)
{
    switch (3 & (int)(size_t) ps) {
      case RBLXSTB_ps_direct: break;
      case RBLXSTB_ps_bucket: rblxstb_bucket_free(GetBucket(ps)); break;
      case RBLXSTB_ps_array : free(GetArray(ps)); break;
      case RBLXSTB_ps_hash  : free(GetHash(ps)); break;
   }
}

rblxstb_ps *rblxstb_ps_copy(rblxstb_ps *ps)
{
   int i;
   // not a switch: order based on expected performance/power-law distribution
    switch (3 & (int)(size_t) ps) {
      case RBLXSTB_ps_direct: return ps;
      case RBLXSTB_ps_bucket: {
         rblxstb_ps_bucket *n = (rblxstb_ps_bucket *) malloc(sizeof(*n));
         *n = *GetBucket(ps);
         return EncodeBucket(n);
      }
      case RBLXSTB_ps_array: {
         rblxstb_ps_array *a = GetArray(ps);
         rblxstb_ps_array *n = (rblxstb_ps_array *) malloc(sizeof(*n) + rblxstb_ps_array_max * sizeof(n->p[0]));
         n->count = a->count;
         for (i=0; i < a->count; ++i)
            n->p[i] = a->p[i];
         return EncodeArray(n);
      }
      case RBLXSTB_ps_hash: {
         rblxstb_ps_hash *h = GetHash(ps);
         rblxstb_ps_hash *n = rblxstb_ps_makehash(h->size, h->size, h->table);
         return EncodeHash(n);
      }
   }
   assert(0); /* NOTREACHED */
   return NULL;
}

int rblxstb_ps_find(rblxstb_ps *ps, void *value)
{
    int i, code = 3 & (int)(size_t) ps;
    assert((3 & (int)(size_t) value) == RBLXSTB_ps_direct);
   assert(rblxstb_ps_fastlist_valid(value));
   // not a switch: order based on expected performance/power-law distribution
   if (code == RBLXSTB_ps_direct)
      return value == ps;
   if (code == RBLXSTB_ps_bucket) {
      rblxstb_ps_bucket *b = GetBucket(ps);
      assert(RBLXSTB_BUCKET_SIZE == 4);
      if (b->p[0] == value || b->p[1] == value ||
          b->p[2] == value || b->p[3] == value)
          return RBLXSTB_TRUE;
      return RBLXSTB_FALSE;
   }
   if (code == RBLXSTB_ps_array) {
      rblxstb_ps_array *a = GetArray(ps);
      for (i=0; i < a->count; ++i)
         if (a->p[i] == value)
            return RBLXSTB_TRUE;
      return RBLXSTB_FALSE;
   } else {
      rblxstb_ps_hash *h = GetHash(ps);
      rblxstb_uint32 hash = rblxstb_hashptr(value);
      rblxstb_uint32 s, n = hash & h->mask;
      void **t = h->table;
      if (t[n] == value) return RBLXSTB_TRUE;
      if (t[n] == NULL) return RBLXSTB_FALSE;
      s = rblxstb_rehash(hash) | 1;
      do {
         n = (n + s) & h->mask;
         if (t[n] == value) return RBLXSTB_TRUE;
      } while (t[n] != NULL);
      return RBLXSTB_FALSE;
   }
}

rblxstb_ps *  rblxstb_ps_add   (rblxstb_ps *ps, void *value)
{
   #ifdef RBLXSTB_DEBUG
   assert(!rblxstb_ps_find(ps,value));
   #endif
   if (value == NULL) return ps; // ignore NULL adds to avoid bad breakage
    assert((3 & (int)(size_t) value) == RBLXSTB_ps_direct);
   assert(rblxstb_ps_fastlist_valid(value));
   assert(value != RBLXSTB_DEL);     // RBLXSTB_DEL is less likely

    switch (3 & (int)(size_t) ps) {
      case RBLXSTB_ps_direct:
         if (ps == NULL) return (rblxstb_ps *) value;
         return EncodeBucket(rblxstb_bucket_create2(ps,value));

      case RBLXSTB_ps_bucket: {
         rblxstb_ps_bucket *b = GetBucket(ps);
         rblxstb_ps_array  *a;
         assert(RBLXSTB_BUCKET_SIZE == 4);
         if (b->p[0] == NULL) { b->p[0] = value; return ps; }
         if (b->p[1] == NULL) { b->p[1] = value; return ps; }
         if (b->p[2] == NULL) { b->p[2] = value; return ps; }
         if (b->p[3] == NULL) { b->p[3] = value; return ps; }
         a = (rblxstb_ps_array *) malloc(sizeof(*a) + 7 * sizeof(a->p[0])); // 8 slots, must be 2^k
         memcpy(a->p, b, sizeof(*b));
         a->p[4] = value;
         a->count = 5;
         rblxstb_bucket_free(b);
         return EncodeArray(a);
      }

      case RBLXSTB_ps_array: {
         rblxstb_ps_array *a = GetArray(ps);
         if (a->count == rblxstb_ps_array_max) {
            // promote from array to hash
            rblxstb_ps_hash *h = rblxstb_ps_makehash(2 << rblxstb_log2_ceil(a->count), a->count, a->p);
            free(a);
            return rblxstb_ps_add(EncodeHash(h), value);
         }
         // do we need to resize the array? the array doubles in size when it
         // crosses a power-of-two
         if ((a->count & (a->count-1))==0) {
            int newsize = a->count*2;
            // clamp newsize to max if:
            //    1. it's larger than max
            //    2. newsize*1.5 is larger than max (to avoid extra resizing)
            if (newsize + a->count > rblxstb_ps_array_max)
               newsize = rblxstb_ps_array_max;
            a = (rblxstb_ps_array *) realloc(a, sizeof(*a) + (newsize-1) * sizeof(a->p[0]));
         }
         a->p[a->count++] = value;
         return EncodeArray(a);
      }
      case RBLXSTB_ps_hash: {
         rblxstb_ps_hash *h = GetHash(ps);
         rblxstb_uint32 hash = rblxstb_hashptr(value);
         rblxstb_uint32 n = hash & h->mask;
         void **t = h->table;
         // find first NULL or RBLXSTB_DEL entry
          if (!rblxstb_ps_empty((size_t)t[n])) {
            rblxstb_uint32 s = rblxstb_rehash(hash) | 1;
            do {
               n = (n + s) & h->mask;
            } while (!rblxstb_ps_empty((size_t)t[n]));
         }
         if (t[n] == RBLXSTB_DEL)
            -- h->count_deletes;
         t[n] = value;
         ++ h->count;
         if (h->count == h->grow_threshhold) {
            rblxstb_ps_hash *h2 = rblxstb_ps_makehash(h->size*2, h->size, t);
            free(h);
            return EncodeHash(h2);
         }
         if (h->count + h->count_deletes == h->rehash_threshhold) {
            rblxstb_ps_hash *h2 = rblxstb_ps_makehash(h->size, h->size, t);
            free(h);
            return EncodeHash(h2);
         }
         return ps;
      }
   }
   return NULL; /* NOTREACHED */
}

rblxstb_ps *rblxstb_ps_remove(rblxstb_ps *ps, void *value)
{
   #ifdef RBLXSTB_DEBUG
   assert(rblxstb_ps_find(ps, value));
   #endif
    assert((3 & (int)(size_t) value) == RBLXSTB_ps_direct);
   if (value == NULL) return ps; // ignore NULL removes to avoid bad breakage
    switch (3 & (int)(size_t) ps) {
      case RBLXSTB_ps_direct:
         return ps == value ? NULL : ps;
      case RBLXSTB_ps_bucket: {
         rblxstb_ps_bucket *b = GetBucket(ps);
         int count=0;
         assert(RBLXSTB_BUCKET_SIZE == 4);
         if (b->p[0] == value) b->p[0] = NULL; else count += (b->p[0] != NULL);
         if (b->p[1] == value) b->p[1] = NULL; else count += (b->p[1] != NULL);
         if (b->p[2] == value) b->p[2] = NULL; else count += (b->p[2] != NULL);
         if (b->p[3] == value) b->p[3] = NULL; else count += (b->p[3] != NULL);
         if (count == 1) { // shrink bucket at size 1
            value = b->p[0];
            if (value == NULL) value = b->p[1];
            if (value == NULL) value = b->p[2];
            if (value == NULL) value = b->p[3];
            assert(value != NULL);
            rblxstb_bucket_free(b);
            return (rblxstb_ps *) value; // return RBLXSTB_ps_direct of value
         }
         return ps;
      }
      case RBLXSTB_ps_array: {
         rblxstb_ps_array *a = GetArray(ps);
         int i;
         for (i=0; i < a->count; ++i) {
            if (a->p[i] == value) {
               a->p[i] = a->p[--a->count];
               if (a->count == 3) { // shrink to bucket!
                  rblxstb_ps_bucket *b = rblxstb_bucket_create3(a->p);
                  free(a);
                  return EncodeBucket(b);
               }
               return ps;
            }
         }
         return ps;
      }
      case RBLXSTB_ps_hash: {
         rblxstb_ps_hash *h = GetHash(ps);
         rblxstb_uint32 hash = rblxstb_hashptr(value);
         rblxstb_uint32 s, n = hash & h->mask;
         void **t = h->table;
         if (t[n] != value) {
            s = rblxstb_rehash(hash) | 1;
            do {
               n = (n + s) & h->mask;
            } while (t[n] != value);
         }
         t[n] = RBLXSTB_DEL;
         -- h->count;
         ++ h->count_deletes;
         // should we shrink down to an array?
         if (h->count < rblxstb_ps_array_max) {
            int n = 1 << rblxstb_log2_floor(rblxstb_ps_array_max);
            if (h->count < n) {
               rblxstb_ps_array *a = (rblxstb_ps_array *) malloc(sizeof(*a) + (n-1) * sizeof(a->p[0]));
               int i,j=0;
               for (i=0; i < h->size; ++i)
                    if (!rblxstb_ps_empty((size_t)t[i]))
                     a->p[j++] = t[i];
               assert(j == h->count);
               a->count = j;
               free(h);
               return EncodeArray(a);
            }
         }
         if (h->count == h->shrink_threshhold) {
            rblxstb_ps_hash *h2 = rblxstb_ps_makehash(h->size >> 1, h->size, t);
            free(h);
            return EncodeHash(h2);
         }
         return ps;
      }
   }
   return ps; /* NOTREACHED */
}

rblxstb_ps *rblxstb_ps_remove_any(rblxstb_ps *ps, void **value)
{
   assert(ps != NULL);
    switch (3 & (int)(size_t) ps) {
      case RBLXSTB_ps_direct:
         *value = ps;
         return NULL;
      case RBLXSTB_ps_bucket: {
         rblxstb_ps_bucket *b = GetBucket(ps);
         int count=0, slast=0, last=0;
         assert(RBLXSTB_BUCKET_SIZE == 4);
         if (b->p[0]) { ++count;               last = 0; }
         if (b->p[1]) { ++count; slast = last; last = 1; }
         if (b->p[2]) { ++count; slast = last; last = 2; }
         if (b->p[3]) { ++count; slast = last; last = 3; }
         *value = b->p[last];
         b->p[last] = 0;
         if (count == 2) {
            void *leftover = b->p[slast]; // second to last
            rblxstb_bucket_free(b);
            return (rblxstb_ps *) leftover;
         }
         return ps;
      }
      case RBLXSTB_ps_array: {
         rblxstb_ps_array *a = GetArray(ps);
         *value = a->p[a->count-1];
         if (a->count == 4)
            return rblxstb_ps_remove(ps, *value);
         --a->count;
         return ps;
      }
      case RBLXSTB_ps_hash: {
         rblxstb_ps_hash *h = GetHash(ps);
         void **t = h->table;
         rblxstb_uint32 n = h->any_offset;
          while (rblxstb_ps_empty((size_t)t[n]))
            n = (n + 1) & h->mask;
         *value = t[n];
         h->any_offset = (n+1) & h->mask;
         // check if we need to skip down to the previous type
         if (h->count-1 < rblxstb_ps_array_max || h->count-1 == h->shrink_threshhold)
            return rblxstb_ps_remove(ps, *value);
         t[n] = RBLXSTB_DEL;
         -- h->count;
         ++ h->count_deletes;
         return ps;
      }
   }
   return ps; /* NOTREACHED */
}


void ** rblxstb_ps_getlist(rblxstb_ps *ps, int *count)
{
   int i,n=0;
   void **p = NULL;
    switch (3 & (int)(size_t) ps) {
      case RBLXSTB_ps_direct:
         if (ps == NULL) { *count = 0; return NULL; }
         p = (void **) malloc(sizeof(*p) * 1);
         p[0] = ps;
         *count = 1;
         return p;
      case RBLXSTB_ps_bucket: {
         rblxstb_ps_bucket *b = GetBucket(ps);
         p = (void **) malloc(sizeof(*p) * RBLXSTB_BUCKET_SIZE);
         for (i=0; i < RBLXSTB_BUCKET_SIZE; ++i)
            if (b->p[i] != NULL)
               p[n++] = b->p[i];
         break;
      }
      case RBLXSTB_ps_array: {
         rblxstb_ps_array *a = GetArray(ps);
         p = (void **) malloc(sizeof(*p) * a->count);
         memcpy(p, a->p, sizeof(*p) * a->count);
         *count = a->count;
         return p;
      }
      case RBLXSTB_ps_hash: {
         rblxstb_ps_hash *h = GetHash(ps);
         p = (void **) malloc(sizeof(*p) * h->count);
         for (i=0; i < h->size; ++i)
              if (!rblxstb_ps_empty((size_t)h->table[i]))
               p[n++] = h->table[i];
         break;
      }
   }
   *count = n;
   return p;
}

int rblxstb_ps_writelist(rblxstb_ps *ps, void **list, int size )
{
   int i,n=0;
    switch (3 & (int)(size_t) ps) {
      case RBLXSTB_ps_direct:
         if (ps == NULL || size <= 0) return 0;
         list[0] = ps;
         return 1;
      case RBLXSTB_ps_bucket: {
         rblxstb_ps_bucket *b = GetBucket(ps);
         for (i=0; i < RBLXSTB_BUCKET_SIZE; ++i)
            if (b->p[i] != NULL && n < size)
               list[n++] = b->p[i];
         return n;
      }
      case RBLXSTB_ps_array: {
         rblxstb_ps_array *a = GetArray(ps);
         n = rblxstb_min(size, a->count);
         memcpy(list, a->p, sizeof(*list) * n);
         return n;
      }
      case RBLXSTB_ps_hash: {
         rblxstb_ps_hash *h = GetHash(ps);
         if (size <= 0) return 0;
         for (i=0; i < h->count; ++i) {
             if (!rblxstb_ps_empty((size_t)h->table[i])) {
               list[n++] = h->table[i];
               if (n == size) break;
            }
         }
         return n;
      }
   }
   return 0; /* NOTREACHED */
}

int rblxstb_ps_enum(rblxstb_ps *ps, void *data, int (*func)(void *value, void *data))
{
   int i;
    switch (3 & (int)(size_t) ps) {
      case RBLXSTB_ps_direct:
         if (ps == NULL) return RBLXSTB_TRUE;
         return func(ps, data);
      case RBLXSTB_ps_bucket: {
         rblxstb_ps_bucket *b = GetBucket(ps);
         for (i=0; i < RBLXSTB_BUCKET_SIZE; ++i)
            if (b->p[i] != NULL)
               if (!func(b->p[i], data))
                  return RBLXSTB_FALSE;
         return RBLXSTB_TRUE;
      }
      case RBLXSTB_ps_array: {
         rblxstb_ps_array *a = GetArray(ps);
         for (i=0; i < a->count; ++i)
            if (!func(a->p[i], data))
               return RBLXSTB_FALSE;
         return RBLXSTB_TRUE;
      }
      case RBLXSTB_ps_hash: {
         rblxstb_ps_hash *h = GetHash(ps);
         for (i=0; i < h->count; ++i)
              if (!rblxstb_ps_empty((size_t)h->table[i]))
               if (!func(h->table[i], data))
                  return RBLXSTB_FALSE;
         return RBLXSTB_TRUE;
      }
   }
   return RBLXSTB_TRUE; /* NOTREACHED */
}

int rblxstb_ps_count (rblxstb_ps *ps)
{
    switch (3 & (int)(size_t) ps) {
      case RBLXSTB_ps_direct:
         return ps != NULL;
      case RBLXSTB_ps_bucket: {
         rblxstb_ps_bucket *b = GetBucket(ps);
         return (b->p[0] != NULL) + (b->p[1] != NULL) +
                (b->p[2] != NULL) + (b->p[3] != NULL);
      }
      case RBLXSTB_ps_array: {
         rblxstb_ps_array *a = GetArray(ps);
         return a->count;
      }
      case RBLXSTB_ps_hash: {
         rblxstb_ps_hash *h = GetHash(ps);
         return h->count;
      }
   }
   return 0;
}

void ** rblxstb_ps_fastlist(rblxstb_ps *ps, int *count)
{
   static void *storage;

    switch (3 & (int)(size_t) ps) {
      case RBLXSTB_ps_direct:
         if (ps == NULL) { *count = 0; return NULL; }
         storage = ps;
         *count = 1;
         return &storage;
      case RBLXSTB_ps_bucket: {
         rblxstb_ps_bucket *b = GetBucket(ps);
         *count = RBLXSTB_BUCKET_SIZE;
         return b->p;
      }
      case RBLXSTB_ps_array: {
         rblxstb_ps_array *a = GetArray(ps);
         *count = a->count;
         return a->p;
      }
      case RBLXSTB_ps_hash: {
         rblxstb_ps_hash *h = GetHash(ps);
         *count = h->size;
         return h->table;
      }
   }
   return NULL; /* NOTREACHED */
}

int rblxstb_ps_subset(rblxstb_ps *bigger, rblxstb_ps *smaller)
{
   int i, listlen;
   void **list = rblxstb_ps_fastlist(smaller, &listlen);
   for(i=0; i < listlen; ++i)
      if (rblxstb_ps_fastlist_valid(list[i]))
         if (!rblxstb_ps_find(bigger, list[i]))
            return 0;
   return 1;
}

int rblxstb_ps_eq(rblxstb_ps *p0, rblxstb_ps *p1)
{
   if (rblxstb_ps_count(p0) != rblxstb_ps_count(p1))
      return 0;
   return rblxstb_ps_subset(p0, p1);
}

#undef GetBucket
#undef GetArray
#undef GetHash

#undef EncodeBucket
#undef EncodeArray
#undef EncodeHash

#endif


//////////////////////////////////////////////////////////////////////////////
//
//               Random Numbers via Meresenne Twister or LCG
//

RBLXSTB_EXTERN unsigned int  rblxstb_srandLCG(unsigned int seed);
RBLXSTB_EXTERN unsigned int  rblxstb_randLCG(void);
RBLXSTB_EXTERN double        rblxstb_frandLCG(void);

RBLXSTB_EXTERN void          rblxstb_srand(unsigned int seed);
RBLXSTB_EXTERN unsigned int  rblxstb_rand(void);
RBLXSTB_EXTERN double        rblxstb_frand(void);
RBLXSTB_EXTERN void          rblxstb_shuffle(void *p, size_t n, size_t sz,
                                        unsigned int seed);
RBLXSTB_EXTERN void rblxstb_reverse(void *p, size_t n, size_t sz);

RBLXSTB_EXTERN unsigned int  rblxstb_randLCG_explicit(unsigned int  seed);

#define rblxstb_rand_define(x,y)                                         \
                                                                     \
   unsigned int  x(void)                                             \
   {                                                                 \
      static unsigned int  rblxstb__rand = y;                            \
      rblxstb__rand = rblxstb__rand * 2147001325 + 715136305; /* BCPL */     \
      return 0x31415926 ^ ((rblxstb__rand >> 16) + (rblxstb__rand << 16));   \
   }

#ifdef RBLXSTB_DEFINE
unsigned int  rblxstb_randLCG_explicit(unsigned int seed)
{
   return seed * 2147001325 + 715136305;
}

static unsigned int  rblxstb__rand_seed=0;

unsigned int  rblxstb_srandLCG(unsigned int seed)
{
   unsigned int  previous = rblxstb__rand_seed;
   rblxstb__rand_seed = seed;
   return previous;
}

unsigned int  rblxstb_randLCG(void)
{
   rblxstb__rand_seed = rblxstb__rand_seed * 2147001325 + 715136305; // BCPL generator
   // shuffle non-random bits to the middle, and xor to decorrelate with seed
   return 0x31415926 ^ ((rblxstb__rand_seed >> 16) + (rblxstb__rand_seed << 16));
}

double rblxstb_frandLCG(void)
{
   return rblxstb_randLCG() / ((double) (1 << 16) * (1 << 16));
}

void rblxstb_shuffle(void *p, size_t n, size_t sz, unsigned int seed)
{
   char *a;
   unsigned int old_seed;
   int i;
   if (seed)
      old_seed = rblxstb_srandLCG(seed);
   a = (char *) p + (n-1) * sz;

   for (i=(int) n; i > 1; --i) {
      int j = rblxstb_randLCG() % i;
      rblxstb_swap(a, (char *) p + j * sz, sz);
      a -= sz;
   }
   if (seed)
      rblxstb_srandLCG(old_seed);
}

void rblxstb_reverse(void *p, size_t n, size_t sz)
{
   size_t i,j = n-1;
   for (i=0; i < j; ++i,--j) {
      rblxstb_swap((char *) p + i * sz, (char *) p + j * sz, sz);
   }
}

// public domain Mersenne Twister by Michael Brundage
#define RBLXSTB__MT_LEN       624

int rblxstb__mt_index = RBLXSTB__MT_LEN*sizeof(int)+1;
unsigned int rblxstb__mt_buffer[RBLXSTB__MT_LEN];

void rblxstb_srand(unsigned int seed)
{
   int i;
   rblxstb__mt_buffer[0]= seed & 0xffffffffUL;
   for (i=1 ; i < RBLXSTB__MT_LEN; ++i)
      rblxstb__mt_buffer[i] = (1812433253UL * (rblxstb__mt_buffer[i-1] ^ (rblxstb__mt_buffer[i-1] >> 30)) + i);
   rblxstb__mt_index = RBLXSTB__MT_LEN*sizeof(unsigned int);
}

#define RBLXSTB__MT_IA           397
#define RBLXSTB__MT_IB           (RBLXSTB__MT_LEN - RBLXSTB__MT_IA)
#define RBLXSTB__UPPER_MASK      0x80000000
#define RBLXSTB__LOWER_MASK      0x7FFFFFFF
#define RBLXSTB__MATRIX_A        0x9908B0DF
#define RBLXSTB__TWIST(b,i,j)    ((b)[i] & RBLXSTB__UPPER_MASK) | ((b)[j] & RBLXSTB__LOWER_MASK)
#define RBLXSTB__MAGIC(s)        (((s)&1)*RBLXSTB__MATRIX_A)

unsigned int rblxstb_rand()
{
   unsigned int  * b = rblxstb__mt_buffer;
   int idx = rblxstb__mt_index;
   unsigned int  s,r;
   int i;

   if (idx >= RBLXSTB__MT_LEN*sizeof(unsigned int)) {
      if (idx > RBLXSTB__MT_LEN*sizeof(unsigned int))
         rblxstb_srand(0);
      idx = 0;
      i = 0;
      for (; i < RBLXSTB__MT_IB; i++) {
         s = RBLXSTB__TWIST(b, i, i+1);
         b[i] = b[i + RBLXSTB__MT_IA] ^ (s >> 1) ^ RBLXSTB__MAGIC(s);
      }
      for (; i < RBLXSTB__MT_LEN-1; i++) {
         s = RBLXSTB__TWIST(b, i, i+1);
         b[i] = b[i - RBLXSTB__MT_IB] ^ (s >> 1) ^ RBLXSTB__MAGIC(s);
      }

      s = RBLXSTB__TWIST(b, RBLXSTB__MT_LEN-1, 0);
      b[RBLXSTB__MT_LEN-1] = b[RBLXSTB__MT_IA-1] ^ (s >> 1) ^ RBLXSTB__MAGIC(s);
   }
   rblxstb__mt_index = idx + sizeof(unsigned int);

   r = *(unsigned int *)((unsigned char *)b + idx);

   r ^= (r >> 11);
   r ^= (r << 7) & 0x9D2C5680;
   r ^= (r << 15) & 0xEFC60000;
   r ^= (r >> 18);

   return r;
}

double rblxstb_frand(void)
{
   return rblxstb_rand() / ((double) (1 << 16) * (1 << 16));
}

#endif


//////////////////////////////////////////////////////////////////////////////
//
//                        rblxstb_dupe
//
// rblxstb_dupe is a duplicate-finding system for very, very large data
// structures--large enough that sorting is too slow, but not so large
// that we can't keep all the data in memory. using it works as follows:
//
//     1. create an rblxstb_dupe:
//          provide a hash function
//          provide an equality function
//          provide an estimate for the size
//          optionally provide a comparison function
//
//     2. traverse your data, 'adding' pointers to the rblxstb_dupe
//
//     3. finish and ask for duplicates
//
//        the rblxstb_dupe will discard its intermediate data and build
//        a collection of sorted lists of duplicates, with non-duplicate
//        entries omitted entirely
//
//
//  Implementation strategy:
//
//     while collecting the N items, we keep a hash table of approximate
//     size sqrt(N). (if you tell use the N up front, the hash table is
//     just that size exactly)
//
//     each entry in the hash table is just an rblxstb__arr of pointers (no need
//     to use rblxstb_ps, because we don't need to delete from these)
//
//     for step 3, for each entry in the hash table, we apply rblxstb_dupe to it
//     recursively. once the size gets small enough (or doesn't decrease
//     significantly), we switch to either using qsort() on the comparison
//     function, or else we just do the icky N^2 gather


typedef struct rblxstb_dupe rblxstb_dupe;

typedef int (*rblxstb_compare_func)(void *a, void *b);
typedef int (*rblxstb_hash_func)(void *a, unsigned int seed);

RBLXSTB_EXTERN void rblxstb_dupe_free(rblxstb_dupe *sd);
RBLXSTB_EXTERN rblxstb_dupe *rblxstb_dupe_create(rblxstb_hash_func hash,
                          rblxstb_compare_func eq, int size, rblxstb_compare_func ineq);
RBLXSTB_EXTERN void rblxstb_dupe_add(rblxstb_dupe *sd, void *item);
RBLXSTB_EXTERN void rblxstb_dupe_finish(rblxstb_dupe *sd);
RBLXSTB_EXTERN int rblxstb_dupe_numsets(rblxstb_dupe *sd);
RBLXSTB_EXTERN void **rblxstb_dupe_set(rblxstb_dupe *sd, int num);
RBLXSTB_EXTERN int rblxstb_dupe_set_count(rblxstb_dupe *sd, int num);

struct rblxstb_dupe
{
   void ***hash_table;
   int hash_size;
   int size_log2;
   int population;

   int hash_shift;
   rblxstb_hash_func     hash;

   rblxstb_compare_func  eq;
   rblxstb_compare_func  ineq;

   void ***dupes;
};

#ifdef RBLXSTB_DEFINE

int rblxstb_dupe_numsets(rblxstb_dupe *sd)
{
   assert(sd->hash_table == NULL);
   return rblxstb_arr_len(sd->dupes);
}

void **rblxstb_dupe_set(rblxstb_dupe *sd, int num)
{
   assert(sd->hash_table == NULL);
   return sd->dupes[num];
}

int rblxstb_dupe_set_count(rblxstb_dupe *sd, int num)
{
   assert(sd->hash_table == NULL);
   return rblxstb_arr_len(sd->dupes[num]);
}

rblxstb_dupe *rblxstb_dupe_create(rblxstb_hash_func hash, rblxstb_compare_func eq, int size,
                                              rblxstb_compare_func ineq)
{
   int i, hsize;
   rblxstb_dupe *sd = (rblxstb_dupe *) malloc(sizeof(*sd));

   sd->size_log2 = 4;
   hsize = 1 << sd->size_log2;
   while (hsize * hsize < size) {
      ++sd->size_log2;
      hsize *= 2;
   }

   sd->hash = hash;
   sd->eq   = eq;
   sd->ineq = ineq;
   sd->hash_shift = 0;

   sd->population = 0;
   sd->hash_size = hsize;
   sd->hash_table = (void ***) malloc(sizeof(*sd->hash_table) * hsize);
   for (i=0; i < hsize; ++i)
      sd->hash_table[i] = NULL;

   sd->dupes = NULL;

   return sd;
}

void rblxstb_dupe_add(rblxstb_dupe *sd, void *item)
{
   rblxstb_uint32 hash = sd->hash(item, sd->hash_shift);
   int z = hash & (sd->hash_size-1);
   rblxstb_arr_push(sd->hash_table[z], item);
   ++sd->population;
}

void rblxstb_dupe_free(rblxstb_dupe *sd)
{
   int i;
   for (i=0; i < rblxstb_arr_len(sd->dupes); ++i)
      if (sd->dupes[i])
         rblxstb_arr_free(sd->dupes[i]);
   rblxstb_arr_free(sd->dupes);
   free(sd);
}

static rblxstb_compare_func rblxstb__compare;

static int rblxstb__dupe_compare(const void *a, const void *b)
{
   void *p = *(void **) a;
   void *q = *(void **) b;

   return rblxstb__compare(p,q);
}

void rblxstb_dupe_finish(rblxstb_dupe *sd)
{
   int i,j,k;
   assert(sd->dupes == NULL);
   for (i=0; i < sd->hash_size; ++i) {
      void ** list = sd->hash_table[i];
      if (list != NULL) {
         int n = rblxstb_arr_len(list);
         // @TODO: measure to find good numbers instead of just making them up!
         int thresh = (sd->ineq ? 200 : 20);
         // if n is large enough to be worth it, and n is smaller than
         // before (so we can guarantee we'll use a smaller hash table);
         // and there are enough hash bits left, assuming full 32-bit hash
         if (n > thresh && n < (sd->population >> 3) && sd->hash_shift + sd->size_log2*2 < 32) {

            // recursively process this row using rblxstb_dupe, O(N log log N)

            rblxstb_dupe *d = rblxstb_dupe_create(sd->hash, sd->eq, n, sd->ineq);
            d->hash_shift = rblxstb_randLCG_explicit(sd->hash_shift);
            for (j=0; j < n; ++j)
               rblxstb_dupe_add(d, list[j]);
            rblxstb_arr_free(sd->hash_table[i]);
            rblxstb_dupe_finish(d);
            for (j=0; j < rblxstb_arr_len(d->dupes); ++j) {
               rblxstb_arr_push(sd->dupes, d->dupes[j]);
               d->dupes[j] = NULL; // take over ownership
            }
            rblxstb_dupe_free(d);

         } else if (sd->ineq) {

            // process this row using qsort(), O(N log N)
            rblxstb__compare = sd->ineq;
            qsort(list, n, sizeof(list[0]), rblxstb__dupe_compare);

            // find equal subsequences of the list
            for (j=0; j < n-1; ) {
               // find a subsequence from j..k
               for (k=j; k < n; ++k)
                  // only use ineq so eq can be left undefined
                  if (sd->ineq(list[j], list[k]))
                     break;
               // k is the first one not in the subsequence
               if (k-j > 1) {
                  void **mylist = NULL;
                  rblxstb_arr_setlen(mylist, k-j);
                  memcpy(mylist, list+j, sizeof(list[j]) * (k-j));
                  rblxstb_arr_push(sd->dupes, mylist);
               }
               j = k;
            }
            rblxstb_arr_free(sd->hash_table[i]);
         } else {

            // process this row using eq(), O(N^2)
            for (j=0; j < n; ++j) {
               if (list[j] != NULL) {
                  void **output  = NULL;
                  for (k=j+1; k < n; ++k) {
                     if (sd->eq(list[j], list[k])) {
                        if (output == NULL)
                           rblxstb_arr_push(output, list[j]);
                        rblxstb_arr_push(output, list[k]);
                        list[k] = NULL;
                     }
                  }
                  list[j] = NULL;
                  if (output)
                     rblxstb_arr_push(sd->dupes, output);
               }
            }
            rblxstb_arr_free(sd->hash_table[i]);
         }
      }
   }
   free(sd->hash_table);
   sd->hash_table = NULL;
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
//                       templatized Sort routine
//
// This is an attempt to implement a templated sorting algorithm.
// To use it, you have to explicitly instantiate it as a _function_,
// then you call that function. This allows the comparison to be inlined,
// giving the sort similar performance to C++ sorts.
//
// It implements quicksort with three-way-median partitioning (generally
// well-behaved), with a final insertion sort pass.
//
// When you define the compare expression, you should assume you have
// elements of your array pointed to by 'a' and 'b', and perform the comparison
// on those. OR you can use one or more statements; first say '0;', then
// write whatever code you want, and compute the result into a variable 'c'.

#define rblxstb_declare_sort(FUNCNAME, TYPE)    \
                       void FUNCNAME(TYPE *p, int n)
#define rblxstb_define_sort(FUNCNAME,TYPE,COMPARE) \
                       rblxstb__define_sort(       void, FUNCNAME,TYPE,COMPARE)
#define rblxstb_define_sort_static(FUNCNAME,TYPE,COMPARE) \
                       rblxstb__define_sort(static void, FUNCNAME,TYPE,COMPARE)

#define rblxstb__define_sort(MODE, FUNCNAME, TYPE, COMPARE)                       \
                                                                              \
static void RBLXSTB_(FUNCNAME,_ins_sort)(TYPE *p, int n)                          \
{                                                                             \
   int i,j;                                                                   \
   for (i=1; i < n; ++i) {                                                    \
      TYPE t = p[i], *a = &t;                                                 \
      j = i;                                                                  \
      while (j > 0) {                                                         \
         TYPE *b = &p[j-1];                                                   \
         int c = COMPARE;                                                     \
         if (!c) break;                                                       \
         p[j] = p[j-1];                                                       \
         --j;                                                                 \
      }                                                                       \
      if (i != j)                                                             \
         p[j] = t;                                                            \
   }                                                                          \
}                                                                             \
                                                                              \
static void RBLXSTB_(FUNCNAME,_quicksort)(TYPE *p, int n)                         \
{                                                                             \
   /* threshold for transitioning to insertion sort */                       \
   while (n > 12) {                                                           \
      TYPE *a,*b,t;                                                           \
      int c01,c12,c,m,i,j;                                                    \
                                                                              \
      /* compute median of three */                                           \
      m = n >> 1;                                                             \
      a = &p[0];                                                              \
      b = &p[m];                                                              \
      c = COMPARE;                                                            \
      c01 = c;                                                                \
      a = &p[m];                                                              \
      b = &p[n-1];                                                            \
      c = COMPARE;                                                            \
      c12 = c;                                                                \
      /* if 0 >= mid >= end, or 0 < mid < end, then use mid */                \
      if (c01 != c12) {                                                       \
         /* otherwise, we'll need to swap something else to middle */         \
         int z;                                                               \
         a = &p[0];                                                           \
         b = &p[n-1];                                                         \
         c = COMPARE;                                                         \
         /* 0>mid && mid<n:  0>n => n; 0<n => 0 */                            \
         /* 0<mid && mid>n:  0>n => 0; 0<n => n */                            \
         z = (c == c12) ? 0 : n-1;                                            \
         t = p[z];                                                            \
         p[z] = p[m];                                                         \
         p[m] = t;                                                            \
      }                                                                       \
      /* now p[m] is the median-of-three */                                   \
      /* swap it to the beginning so it won't move around */                  \
      t = p[0];                                                               \
      p[0] = p[m];                                                            \
      p[m] = t;                                                               \
                                                                              \
      /* partition loop */                                                    \
      i=1;                                                                    \
      j=n-1;                                                                  \
      for(;;) {                                                               \
         /* handling of equality is crucial here */                           \
         /* for sentinels & efficiency with duplicates */                     \
         b = &p[0];                                                           \
         for (;;++i) {                                                        \
            a=&p[i];                                                          \
            c = COMPARE;                                                      \
            if (!c) break;                                                    \
         }                                                                    \
         a = &p[0];                                                           \
         for (;;--j) {                                                        \
            b=&p[j];                                                          \
            c = COMPARE;                                                      \
            if (!c) break;                                                    \
         }                                                                    \
         /* make sure we haven't crossed */                                   \
         if (i >= j) break;                                                   \
         t = p[i];                                                            \
         p[i] = p[j];                                                         \
         p[j] = t;                                                            \
                                                                              \
         ++i;                                                                 \
         --j;                                                                 \
      }                                                                       \
      /* recurse on smaller side, iterate on larger */                        \
      if (j < (n-i)) {                                                        \
         RBLXSTB_(FUNCNAME,_quicksort)(p,j);                                       \
         p = p+i;                                                             \
         n = n-i;                                                             \
      } else {                                                                \
         RBLXSTB_(FUNCNAME,_quicksort)(p+i, n-i);                                  \
         n = j;                                                               \
      }                                                                       \
   }                                                                          \
}                                                                             \
                                                                              \
MODE FUNCNAME(TYPE *p, int n)                                                 \
{                                                                             \
   RBLXSTB_(FUNCNAME, _quicksort)(p, n);                                           \
   RBLXSTB_(FUNCNAME, _ins_sort)(p, n);                                            \
}                                                                             \


//////////////////////////////////////////////////////////////////////////////
//
//      rblxstb_bitset   an array of booleans indexed by integers
//

typedef rblxstb_uint32 rblxstb_bitset;

RBLXSTB_EXTERN  rblxstb_bitset *rblxstb_bitset_new(int value, int len);

#define rblxstb_bitset_clearall(arr,len)     (memset(arr,   0, 4 * (len)))
#define rblxstb_bitset_setall(arr,len)       (memset(arr, 255, 4 * (len)))

#define rblxstb_bitset_setbit(arr,n)         ((arr)[(n) >> 5] |=  (1 << (n & 31)))
#define rblxstb_bitset_clearbit(arr,n)       ((arr)[(n) >> 5] &= ~(1 << (n & 31)))
#define rblxstb_bitset_testbit(arr,n)        ((arr)[(n) >> 5] &   (1 << (n & 31)))

RBLXSTB_EXTERN  rblxstb_bitset *rblxstb_bitset_union(rblxstb_bitset *p0, rblxstb_bitset *p1, int len);

RBLXSTB_EXTERN  int *rblxstb_bitset_getlist(rblxstb_bitset *out, int start, int end);

RBLXSTB_EXTERN  int  rblxstb_bitset_eq(rblxstb_bitset *p0, rblxstb_bitset *p1, int len);
RBLXSTB_EXTERN  int  rblxstb_bitset_disjoint(rblxstb_bitset *p0, rblxstb_bitset *p1, int len);
RBLXSTB_EXTERN  int  rblxstb_bitset_disjoint_0(rblxstb_bitset *p0, rblxstb_bitset *p1, int len);
RBLXSTB_EXTERN  int  rblxstb_bitset_subset(rblxstb_bitset *bigger, rblxstb_bitset *smaller, int len);
RBLXSTB_EXTERN  int  rblxstb_bitset_unioneq_changed(rblxstb_bitset *p0, rblxstb_bitset *p1, int len);

#ifdef RBLXSTB_DEFINE
int rblxstb_bitset_eq(rblxstb_bitset *p0, rblxstb_bitset *p1, int len)
{
   int i;
   for (i=0; i < len; ++i)
      if (p0[i] != p1[i]) return 0;
   return 1;
}

int rblxstb_bitset_disjoint(rblxstb_bitset *p0, rblxstb_bitset *p1, int len)
{
   int i;
   for (i=0; i < len; ++i)
      if (p0[i] & p1[i]) return 0;
   return 1;
}

int rblxstb_bitset_disjoint_0(rblxstb_bitset *p0, rblxstb_bitset *p1, int len)
{
   int i;
   for (i=0; i < len; ++i)
      if ((p0[i] | p1[i]) != 0xffffffff) return 0;
   return 1;
}

int rblxstb_bitset_subset(rblxstb_bitset *bigger, rblxstb_bitset *smaller, int len)
{
   int i;
   for (i=0; i < len; ++i)
      if ((bigger[i] & smaller[i]) != smaller[i]) return 0;
   return 1;
}

rblxstb_bitset *rblxstb_bitset_union(rblxstb_bitset *p0, rblxstb_bitset *p1, int len)
{
   int i;
   rblxstb_bitset *d = (rblxstb_bitset *) malloc(sizeof(*d) * len);
   for (i=0; i < len; ++i) d[i] = p0[i] | p1[i];
   return d;
}

int rblxstb_bitset_unioneq_changed(rblxstb_bitset *p0, rblxstb_bitset *p1, int len)
{
   int i, changed=0;
   for (i=0; i < len; ++i) {
      rblxstb_bitset d = p0[i] | p1[i];
      if (d != p0[i]) {
         p0[i] = d;
         changed = 1;
      }
   }
   return changed;
}

rblxstb_bitset *rblxstb_bitset_new(int value, int len)
{
   int i;
   rblxstb_bitset *d = (rblxstb_bitset *) malloc(sizeof(*d) * len);
   if (value) value = 0xffffffff;
   for (i=0; i < len; ++i) d[i] = value;
   return d;
}

int *rblxstb_bitset_getlist(rblxstb_bitset *out, int start, int end)
{
   int *list = NULL;
   int i;
   for (i=start; i < end; ++i)
      if (rblxstb_bitset_testbit(out, i))
         rblxstb_arr_push(list, i);
   return list;
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
//      rblxstb_wordwrap    quality word-wrapping for fixed-width fonts
//

RBLXSTB_EXTERN int rblxstb_wordwrap(int *pairs, int pair_max, int count, char *str);
RBLXSTB_EXTERN int *rblxstb_wordwrapalloc(int count, char *str);

#ifdef RBLXSTB_DEFINE

int rblxstb_wordwrap(int *pairs, int pair_max, int count, char *str)
{
   int n=0,i=0, start=0,nonwhite=0;
   if (pairs == NULL) pair_max = 0x7ffffff0;
   else pair_max *= 2;
   // parse
   for(;;) {
      int s=i; // first whitespace char; last nonwhite+1
      int w;   // word start
      // accept whitespace
      while (isspace(str[i])) {
         if (str[i] == '\n' || str[i] == '\r') {
            if (str[i] + str[i+1] == '\n' + '\r') ++i;
            if (n >= pair_max) return -1;
            if (pairs) pairs[n] = start, pairs[n+1] = s-start;
            n += 2;
            nonwhite=0;
            start = i+1;
            s = start;
         }
         ++i;
      }
      if (i >= start+count) {
         // we've gone off the end using whitespace
         if (nonwhite) {
            if (n >= pair_max) return -1;
            if (pairs) pairs[n] = start, pairs[n+1] = s-start;
            n += 2;
            start = s = i;
            nonwhite=0;
         } else {
            // output all the whitespace
            while (i >= start+count) {
               if (n >= pair_max) return -1;
               if (pairs) pairs[n] = start, pairs[n+1] = count;
               n += 2;
               start += count;
            }
            s = start;
         }
      }

      if (str[i] == 0) break;
      // now scan out a word and see if it fits
      w = i;
      while (str[i] && !isspace(str[i])) {
         ++i;
      }
      // wrapped?
      if (i > start + count) {
         // huge?
         if (i-s <= count) {
            if (n >= pair_max) return -1;
            if (pairs) pairs[n] = start, pairs[n+1] = s-start;
            n += 2;
            start = w;
         } else {
            // This word is longer than one line. If we wrap it onto N lines
            // there are leftover chars. do those chars fit on the cur line?
            // But if we have leading whitespace, we force it to start here.
            if ((w-start) + ((i-w) % count) <= count || !nonwhite) {
               // output a full line
               if (n >= pair_max) return -1;
               if (pairs) pairs[n] = start, pairs[n+1] = count;
               n += 2;
               start += count;
               w = start;
            } else {
               // output a partial line, trimming trailing whitespace
               if (s != start) {
                  if (n >= pair_max) return -1;
                  if (pairs) pairs[n] = start, pairs[n+1] = s-start;
                  n += 2;
                  start = w;
               }
            }
            // now output full lines as needed
            while (start + count <= i) {
               if (n >= pair_max) return -1;
               if (pairs) pairs[n] = start, pairs[n+1] = count;
               n += 2;
               start += count;
            }
         }
      }
      nonwhite=1;
   }
   if (start < i) {
      if (n >= pair_max) return -1;
      if (pairs) pairs[n] = start, pairs[n+1] = i-start;
      n += 2;
   }
   return n>>1;
}

int *rblxstb_wordwrapalloc(int count, char *str)
{
   int n = rblxstb_wordwrap(NULL,0,count,str);
   int *z = NULL;
   rblxstb_arr_setlen(z, n*2);
   rblxstb_wordwrap(z, n, count, str);
   return z;
}
#endif


//////////////////////////////////////////////////////////////////////////////
//
//         rblxstb_match:    wildcards and regexping
//

RBLXSTB_EXTERN int rblxstb_wildmatch (char *expr, char *candidate);
RBLXSTB_EXTERN int rblxstb_wildmatchi(char *expr, char *candidate);
RBLXSTB_EXTERN int rblxstb_wildfind  (char *expr, char *candidate);
RBLXSTB_EXTERN int rblxstb_wildfindi (char *expr, char *candidate);

RBLXSTB_EXTERN int rblxstb_regex(char *regex, char *candidate);

typedef struct rblxstb_matcher rblxstb_matcher;

RBLXSTB_EXTERN rblxstb_matcher *rblxstb_regex_matcher(char *regex);
RBLXSTB_EXTERN int rblxstb_matcher_match(rblxstb_matcher *m, char *str);
RBLXSTB_EXTERN int rblxstb_matcher_find(rblxstb_matcher *m, char *str);
RBLXSTB_EXTERN void rblxstb_matcher_free(rblxstb_matcher *f);

RBLXSTB_EXTERN rblxstb_matcher *rblxstb_lex_matcher(void);
RBLXSTB_EXTERN int rblxstb_lex_item(rblxstb_matcher *m, const char *str, int result);
RBLXSTB_EXTERN int rblxstb_lex_item_wild(rblxstb_matcher *matcher, const char *regex, int result);
RBLXSTB_EXTERN int rblxstb_lex(rblxstb_matcher *m, char *str, int *len);



#ifdef RBLXSTB_DEFINE

static int rblxstb__match_qstring(char *candidate, char *qstring, int qlen, int insensitive)
{
   int i;
   if (insensitive) {
      for (i=0; i < qlen; ++i)
         if (qstring[i] == '?') {
            if (!candidate[i]) return 0;
         } else
            if (tolower(qstring[i]) != tolower(candidate[i]))
               return 0;
   } else {
      for (i=0; i < qlen; ++i)
         if (qstring[i] == '?') {
            if (!candidate[i]) return 0;
         } else
            if (qstring[i] != candidate[i])
               return 0;
   }
   return 1;
}

static int rblxstb__find_qstring(char *candidate, char *qstring, int qlen, int insensitive)
{
   char c;

   int offset=0;
   while (*qstring == '?') {
      ++qstring;
      --qlen;
      ++candidate;
      if (qlen == 0) return 0;
      if (*candidate == 0) return -1;
   }

   c = *qstring++;
   --qlen;
   if (insensitive) c = tolower(c);

   while (candidate[offset]) {
      if (c == (insensitive ? tolower(candidate[offset]) : candidate[offset]))
         if (rblxstb__match_qstring(candidate+offset+1, qstring, qlen, insensitive))
            return offset;
      ++offset;
   }

   return -1;
}

int rblxstb__wildmatch_raw2(char *expr, char *candidate, int search, int insensitive)
{
   int where=0;
   int start = -1;

   if (!search) {
      // parse to first '*'
      if (*expr != '*')
         start = 0;
      while (*expr != '*') {
         if (!*expr)
            return *candidate == 0 ? 0 : -1;
         if (*expr == '?') {
            if (!*candidate) return -1;
         } else {
            if (insensitive) {
               if (tolower(*candidate) != tolower(*expr))
                  return -1;
            } else
               if (*candidate != *expr)
                  return -1;
         }
         ++candidate, ++expr, ++where;
      }
   } else {
      // 0-length search string
      if (!*expr)
         return 0;
   }

   assert(search || *expr == '*');
   if (!search)
      ++expr;

   // implicit '*' at this point

   while (*expr) {
      int o=0;
      // combine redundant * characters
      while (expr[0] == '*') ++expr;

      // ok, at this point, expr[-1] == '*',
      // and expr[0] != '*'

      if (!expr[0]) return start >= 0 ? start : 0;

      // now find next '*'
      o = 0;
      while (expr[o] != '*') {
         if (expr[o] == 0)
            break;
         ++o;
      }
      // if no '*', scan to end, then match at end
      if (expr[o] == 0 && !search) {
         int z;
         for (z=0; z < o; ++z)
            if (candidate[z] == 0)
               return -1;
         while (candidate[z])
            ++z;
         // ok, now check if they match
         if (rblxstb__match_qstring(candidate+z-o, expr, o, insensitive))
            return start >= 0 ? start : 0;
         return -1;
      } else {
         // if yes '*', then do rblxstb__find_qmatch on the intervening chars
         int n = rblxstb__find_qstring(candidate, expr, o, insensitive);
         if (n < 0)
            return -1;
         if (start < 0)
            start = where + n;
         expr += o;
         candidate += n+o;
      }

      if (*expr == 0) {
         assert(search);
         return start;
      }

      assert(*expr == '*');
      ++expr;
   }

   return start >= 0 ? start : 0;
}

int rblxstb__wildmatch_raw(char *expr, char *candidate, int search, int insensitive)
{
   char buffer[256];
   // handle multiple search strings
   char *s = strchr(expr, ';');
   char *last = expr;
   while (s) {
      int z;
      // need to allow for non-writeable strings... assume they're small
      if (s - last < 256) {
         rblxstb_strncpy(buffer, last, (int) (s-last+1));
         buffer[s-last] = 0;
         z = rblxstb__wildmatch_raw2(buffer, candidate, search, insensitive);
      } else {
         *s = 0;
         z = rblxstb__wildmatch_raw2(last, candidate, search, insensitive);
         *s = ';';
      }
      if (z >= 0) return z;
      last = s+1;
      s = strchr(last, ';');
   }
   return rblxstb__wildmatch_raw2(last, candidate, search, insensitive);
}

int rblxstb_wildmatch(char *expr, char *candidate)
{
   return rblxstb__wildmatch_raw(expr, candidate, 0,0) >= 0;
}

int rblxstb_wildmatchi(char *expr, char *candidate)
{
   return rblxstb__wildmatch_raw(expr, candidate, 0,1) >= 0;
}

int rblxstb_wildfind(char *expr, char *candidate)
{
   return rblxstb__wildmatch_raw(expr, candidate, 1,0);
}

int rblxstb_wildfindi(char *expr, char *candidate)
{
   return rblxstb__wildmatch_raw(expr, candidate, 1,1);
}

typedef struct
{
   rblxstb_int16 transition[256];
} rblxstb_dfa;

// an NFA node represents a state you're in; it then has
// an arbitrary number of edges dangling off of it
// note this isn't utf8-y
typedef struct
{
   rblxstb_int16  match; // character/set to match
   rblxstb_uint16 node;  // output node to go to
} rblxstb_nfa_edge;

typedef struct
{
   rblxstb_int16 goal;   // does reaching this win the prize?
   rblxstb_uint8 active; // is this in the active list
   rblxstb_nfa_edge *out;
   rblxstb_uint16 *eps;  // list of epsilon closures
} rblxstb_nfa_node;

#define RBLXSTB__DFA_UNDEF  -1
#define RBLXSTB__DFA_GOAL   -2
#define RBLXSTB__DFA_END    -3
#define RBLXSTB__DFA_MGOAL  -4
#define RBLXSTB__DFA_VALID  0

#define RBLXSTB__NFA_STOP_GOAL -1

// compiled regexp
struct rblxstb_matcher
{
   rblxstb_uint16 start_node;
   rblxstb_int16 dfa_start;
   rblxstb_uint32 *charset;
   int num_charset;
   int match_start;
   rblxstb_nfa_node *nodes;
   int does_lex;

   // dfa matcher
   rblxstb_dfa    * dfa;
   rblxstb_uint32 * dfa_mapping;
   rblxstb_int16  * dfa_result;
   int num_words_per_dfa;
};

static int rblxstb__add_node(rblxstb_matcher *matcher)
{
   rblxstb_nfa_node z;
   z.active = 0;
   z.eps    = 0;
   z.goal   = 0;
   z.out    = 0;
   rblxstb_arr_push(matcher->nodes, z);
   return rblxstb_arr_len(matcher->nodes)-1;
}

static void rblxstb__add_epsilon(rblxstb_matcher *matcher, int from, int to)
{
   assert(from != to);
   if (matcher->nodes[from].eps == NULL)
      rblxstb_arr_malloc((void **) &matcher->nodes[from].eps, matcher);
   rblxstb_arr_push(matcher->nodes[from].eps, to);
}

static void rblxstb__add_edge(rblxstb_matcher *matcher, int from, int to, int type)
{
    rblxstb_nfa_edge z = { (rblxstb_int16)type, (rblxstb_uint16)to };
   if (matcher->nodes[from].out == NULL)
      rblxstb_arr_malloc((void **) &matcher->nodes[from].out, matcher);
   rblxstb_arr_push(matcher->nodes[from].out, z);
}

static char *rblxstb__reg_parse_alt(rblxstb_matcher *m, int s, char *r, rblxstb_uint16 *e);
static char *rblxstb__reg_parse(rblxstb_matcher *matcher, int start, char *regex, rblxstb_uint16 *end)
{
   int n;
   int last_start = -1;
   rblxstb_uint16 last_end = start;

   while (*regex) {
      switch (*regex) {
         case '(':
            last_start = last_end;
            regex = rblxstb__reg_parse_alt(matcher, last_end, regex+1, &last_end);
            if (regex == NULL || *regex != ')')
               return NULL;
            ++regex;
            break;

         case '|':
         case ')':
            *end = last_end;
            return regex;

         case '?':
            if (last_start < 0) return NULL;
            rblxstb__add_epsilon(matcher, last_start, last_end);
            ++regex;
            break;

         case '*':
            if (last_start < 0) return NULL;
            rblxstb__add_epsilon(matcher, last_start, last_end);

            // fall through

         case '+':
            if (last_start < 0) return NULL;
            rblxstb__add_epsilon(matcher, last_end, last_start);
            // prevent links back to last_end from chaining to last_start
            n = rblxstb__add_node(matcher);
            rblxstb__add_epsilon(matcher, last_end, n);
            last_end = n;
            ++regex;
            break;

         case '{':   // not supported!
            // @TODO: given {n,m}, clone last_start to last_end m times,
            // and include epsilons from start to first m-n blocks
            return NULL;

         case '\\':
            ++regex;
            if (!*regex) return NULL;

            // fallthrough
         default: // match exactly this character
            n = rblxstb__add_node(matcher);
            rblxstb__add_edge(matcher, last_end, n, *regex);
            last_start = last_end;
            last_end = n;
            ++regex;
            break;

         case '$':
            n = rblxstb__add_node(matcher);
            rblxstb__add_edge(matcher, last_end, n, '\n');
            last_start = last_end;
            last_end = n;
            ++regex;
            break;

         case '.':
            n = rblxstb__add_node(matcher);
            rblxstb__add_edge(matcher, last_end, n, -1);
            last_start = last_end;
            last_end = n;
            ++regex;
            break;

         case '[': {
            rblxstb_uint8 flags[256];
            int invert = 0,z;
            ++regex;
            if (matcher->num_charset == 0) {
               matcher->charset = (rblxstb_uint *) rblxstb_malloc(matcher, sizeof(*matcher->charset) * 256);
               memset(matcher->charset, 0, sizeof(*matcher->charset) * 256);
            }

            memset(flags,0,sizeof(flags));

            // leading ^ is special
            if (*regex == '^')
               ++regex, invert = 1;

            // leading ] is special
            if (*regex == ']') {
               flags[(int) ']'] = 1;
               ++regex;
            }
            while (*regex != ']') {
               rblxstb_uint a;
               if (!*regex) return NULL;
               a = *regex++;
               if (regex[0] == '-' && regex[1] != ']') {
                  rblxstb_uint i,b = regex[1];
                  regex += 2;
                  if (b == 0) return NULL;
                  if (a > b) return NULL;
                  for (i=a; i <= b; ++i)
                     flags[i] = 1;
               } else
                  flags[a] = 1;
            }
            ++regex;
            if (invert) {
               int i;
               for (i=0; i < 256; ++i)
                  flags[i] = 1-flags[i];
            }

            // now check if any existing charset matches
            for (z=0; z < matcher->num_charset; ++z) {
               int i, k[2] = { 0, 1 << z};
               for (i=0; i < 256; ++i) {
                  unsigned int f = k[flags[i]];
                  if ((matcher->charset[i] & k[1]) != f)
                     break;
               }
               if (i == 256) break;
            }

            if (z == matcher->num_charset) {
               int i;
               ++matcher->num_charset;
               if (matcher->num_charset > 32) {
                  assert(0); /* NOTREACHED */
                  return NULL; // too many charsets, oops
               }
               for (i=0; i < 256; ++i)
                  if (flags[i])
                     matcher->charset[i] |= (1 << z);
            }

            n = rblxstb__add_node(matcher);
            rblxstb__add_edge(matcher, last_end, n, -2 - z);
            last_start = last_end;
            last_end = n;
            break;
         }
      }
   }
   *end = last_end;
   return regex;
}

static char *rblxstb__reg_parse_alt(rblxstb_matcher *matcher, int start, char *regex, rblxstb_uint16 *end)
{
   rblxstb_uint16 last_end = start;
   rblxstb_uint16 main_end;

   int head, tail;

   head = rblxstb__add_node(matcher);
   rblxstb__add_epsilon(matcher, start, head);

   regex = rblxstb__reg_parse(matcher, head, regex, &last_end);
   if (regex == NULL) return NULL;
   if (*regex == 0 || *regex == ')') {
      *end = last_end;
      return regex;
   }

   main_end = last_end;
   tail = rblxstb__add_node(matcher);

   rblxstb__add_epsilon(matcher, last_end, tail);

   // start alternatives from the same starting node; use epsilon
   // transitions to combine their endings
   while(*regex && *regex != ')') {
      assert(*regex == '|');
      head = rblxstb__add_node(matcher);
      rblxstb__add_epsilon(matcher, start, head);
      regex = rblxstb__reg_parse(matcher, head, regex+1, &last_end);
      if (regex == NULL)
         return NULL;
      rblxstb__add_epsilon(matcher, last_end, tail);
   }

   *end = tail;
   return regex;
}

static char *rblxstb__wild_parse(rblxstb_matcher *matcher, int start, char *str, rblxstb_uint16 *end)
{
   int n;
   rblxstb_uint16 last_end;

   last_end = rblxstb__add_node(matcher);
   rblxstb__add_epsilon(matcher, start, last_end);

   while (*str) {
      switch (*str) {
            // fallthrough
         default: // match exactly this character
            n = rblxstb__add_node(matcher);
            if (toupper(*str) == tolower(*str)) {
               rblxstb__add_edge(matcher, last_end, n, *str);
            } else {
               rblxstb__add_edge(matcher, last_end, n, tolower(*str));
               rblxstb__add_edge(matcher, last_end, n, toupper(*str));
            }
            last_end = n;
            ++str;
            break;

         case '?':
            n = rblxstb__add_node(matcher);
            rblxstb__add_edge(matcher, last_end, n, -1);
            last_end = n;
            ++str;
            break;

         case '*':
            n = rblxstb__add_node(matcher);
            rblxstb__add_edge(matcher, last_end, n, -1);
            rblxstb__add_epsilon(matcher, last_end, n);
            rblxstb__add_epsilon(matcher, n, last_end);
            last_end = n;
            ++str;
            break;
      }
   }

   // now require end of string to match
   n = rblxstb__add_node(matcher);
   rblxstb__add_edge(matcher, last_end, n, 0);
   last_end = n;

   *end = last_end;
   return str;
}

static int rblxstb__opt(rblxstb_matcher *m, int n)
{
   for(;;) {
      rblxstb_nfa_node *p = &m->nodes[n];
      if (p->goal)                  return n;
      if (rblxstb_arr_len(p->out))      return n;
      if (rblxstb_arr_len(p->eps) != 1) return n;
      n = p->eps[0];
   }
}

static void rblxstb__optimize(rblxstb_matcher *m)
{
   // if the target of any edge is a node with exactly
   // one out-epsilon, shorten it
   int i,j;
   for (i=0; i < rblxstb_arr_len(m->nodes); ++i) {
      rblxstb_nfa_node *p = &m->nodes[i];
      for (j=0; j < rblxstb_arr_len(p->out); ++j)
         p->out[j].node = rblxstb__opt(m,p->out[j].node);
      for (j=0; j < rblxstb_arr_len(p->eps); ++j)
         p->eps[j]      = rblxstb__opt(m,p->eps[j]     );
   }
   m->start_node = rblxstb__opt(m,m->start_node);
}

void rblxstb_matcher_free(rblxstb_matcher *f)
{
   rblxstb_free(f);
}

static rblxstb_matcher *rblxstb__alloc_matcher(void)
{
   rblxstb_matcher *matcher = (rblxstb_matcher *) rblxstb_malloc(0,sizeof(*matcher));

   matcher->start_node  = 0;
   rblxstb_arr_malloc((void **) &matcher->nodes, matcher);
   matcher->num_charset = 0;
   matcher->match_start = 0;
   matcher->does_lex    = 0;

   matcher->dfa_start   = RBLXSTB__DFA_UNDEF;
   rblxstb_arr_malloc((void **) &matcher->dfa, matcher);
   rblxstb_arr_malloc((void **) &matcher->dfa_mapping, matcher);
   rblxstb_arr_malloc((void **) &matcher->dfa_result, matcher);

   rblxstb__add_node(matcher);

   return matcher;
}

static void rblxstb__lex_reset(rblxstb_matcher *matcher)
{
   // flush cached dfa data
   rblxstb_arr_setlen(matcher->dfa, 0);
   rblxstb_arr_setlen(matcher->dfa_mapping, 0);
   rblxstb_arr_setlen(matcher->dfa_result, 0);
   matcher->dfa_start = RBLXSTB__DFA_UNDEF;
}

rblxstb_matcher *rblxstb_regex_matcher(char *regex)
{
   char *z;
   rblxstb_uint16 end;
   rblxstb_matcher *matcher = rblxstb__alloc_matcher();
   if (*regex == '^') {
      matcher->match_start = 1;
      ++regex;
   }

   z = rblxstb__reg_parse_alt(matcher, matcher->start_node, regex, &end);

   if (!z || *z) {
      rblxstb_free(matcher);
      return NULL;
   }

   ((matcher->nodes)[(int) end]).goal = RBLXSTB__NFA_STOP_GOAL;

   return matcher;
}

rblxstb_matcher *rblxstb_lex_matcher(void)
{
   rblxstb_matcher *matcher = rblxstb__alloc_matcher();

   matcher->match_start = 1;
   matcher->does_lex    = 1;

   return matcher;
}

int rblxstb_lex_item(rblxstb_matcher *matcher, const char *regex, int result)
{
   char *z;
   rblxstb_uint16 end;

   z = rblxstb__reg_parse_alt(matcher, matcher->start_node, (char*) regex, &end);

   if (z == NULL)
      return 0;

   rblxstb__lex_reset(matcher);

   matcher->nodes[(int) end].goal = result;
   return 1;
}

int rblxstb_lex_item_wild(rblxstb_matcher *matcher, const char *regex, int result)
{
   char *z;
   rblxstb_uint16 end;

   z = rblxstb__wild_parse(matcher, matcher->start_node, (char*) regex, &end);

   if (z == NULL)
      return 0;

   rblxstb__lex_reset(matcher);

   matcher->nodes[(int) end].goal = result;
   return 1;
}

static void rblxstb__clear(rblxstb_matcher *m, rblxstb_uint16 *list)
{
   int i;
   for (i=0; i < rblxstb_arr_len(list); ++i)
      m->nodes[(int) list[i]].active = 0;
}

static int rblxstb__clear_goalcheck(rblxstb_matcher *m, rblxstb_uint16 *list)
{
   int i, t=0;
   for (i=0; i < rblxstb_arr_len(list); ++i) {
      t += m->nodes[(int) list[i]].goal;
      m->nodes[(int) list[i]].active = 0;
   }
   return t;
}

static rblxstb_uint16 * rblxstb__add_if_inactive(rblxstb_matcher *m, rblxstb_uint16 *list, int n)
{
   if (!m->nodes[n].active) {
      rblxstb_arr_push(list, n);
      m->nodes[n].active = 1;
   }
   return list;
}

static rblxstb_uint16 * rblxstb__eps_closure(rblxstb_matcher *m, rblxstb_uint16 *list)
{
   int i,n = rblxstb_arr_len(list);

   for(i=0; i < n; ++i) {
      rblxstb_uint16 *e = m->nodes[(int) list[i]].eps;
      if (e) {
         int j,k = rblxstb_arr_len(e);
         for (j=0; j < k; ++j)
            list = rblxstb__add_if_inactive(m, list, e[j]);
         n = rblxstb_arr_len(list);
      }
   }

   return list;
}

int rblxstb_matcher_match(rblxstb_matcher *m, char *str)
{
   int result = 0;
   int i,j,y,z;
   rblxstb_uint16 *previous = NULL;
   rblxstb_uint16 *current = NULL;
   rblxstb_uint16 *temp;

   rblxstb_arr_setsize(previous, 4);
   rblxstb_arr_setsize(current, 4);

   previous = rblxstb__add_if_inactive(m, previous, m->start_node);
   previous = rblxstb__eps_closure(m,previous);
   rblxstb__clear(m, previous);

   while (*str && rblxstb_arr_len(previous)) {
      y = rblxstb_arr_len(previous);
      for (i=0; i < y; ++i) {
         rblxstb_nfa_node *n = &m->nodes[(int) previous[i]];
         z = rblxstb_arr_len(n->out);
         for (j=0; j < z; ++j) {
            if (n->out[j].match >= 0) {
               if (n->out[j].match == *str)
                  current = rblxstb__add_if_inactive(m, current, n->out[j].node);
            } else if (n->out[j].match == -1) {
               if (*str != '\n')
                  current = rblxstb__add_if_inactive(m, current, n->out[j].node);
            } else if (n->out[j].match < -1) {
               int z = -n->out[j].match - 2;
               if (m->charset[(rblxstb_uint8) *str] & (1 << z))
                  current = rblxstb__add_if_inactive(m, current, n->out[j].node);
            }
         }
      }
      rblxstb_arr_setlen(previous, 0);

      temp = previous;
      previous = current;
      current = temp;

      previous = rblxstb__eps_closure(m,previous);
      rblxstb__clear(m, previous);

      ++str;
   }

   // transition to pick up a '$' at the end
   y = rblxstb_arr_len(previous);
   for (i=0; i < y; ++i)
      m->nodes[(int) previous[i]].active = 1;

   for (i=0; i < y; ++i) {
      rblxstb_nfa_node *n = &m->nodes[(int) previous[i]];
      z = rblxstb_arr_len(n->out);
      for (j=0; j < z; ++j) {
         if (n->out[j].match == '\n')
            current = rblxstb__add_if_inactive(m, current, n->out[j].node);
      }
   }

   previous = rblxstb__eps_closure(m,previous);
   rblxstb__clear(m, previous);

   y = rblxstb_arr_len(previous);
   for (i=0; i < y; ++i)
      if (m->nodes[(int) previous[i]].goal)
         result = 1;

   rblxstb_arr_free(previous);
   rblxstb_arr_free(current);

   return result && *str == 0;
}

rblxstb_int16 rblxstb__get_dfa_node(rblxstb_matcher *m, rblxstb_uint16 *list)
{
   rblxstb_uint16 node;
   rblxstb_uint32 data[8], *state, *newstate;
   int i,j,n;

   state = (rblxstb_uint32 *) rblxstb_temp(data, m->num_words_per_dfa * 4);
   memset(state, 0, m->num_words_per_dfa*4);

   n = rblxstb_arr_len(list);
   for (i=0; i < n; ++i) {
      int x = list[i];
      state[x >> 5] |= 1 << (x & 31);
   }

   // @TODO use a hash table
   n = rblxstb_arr_len(m->dfa_mapping);
   i=j=0;
   for(; j < n; ++i, j += m->num_words_per_dfa) {
      // @TODO special case for <= 32
      if (!memcmp(state, m->dfa_mapping + j, m->num_words_per_dfa*4)) {
         node = i;
         goto done;
      }
   }

   assert(rblxstb_arr_len(m->dfa) == i);
   node = i;

   newstate = rblxstb_arr_addn(m->dfa_mapping, m->num_words_per_dfa);
   memcpy(newstate, state, m->num_words_per_dfa*4);

   // set all transitions to 'unknown'
   rblxstb_arr_add(m->dfa);
   memset(m->dfa[i].transition, -1, sizeof(m->dfa[i].transition));

   if (m->does_lex) {
      int result = -1;
      n = rblxstb_arr_len(list);
      for (i=0; i < n; ++i) {
         if (m->nodes[(int) list[i]].goal > result)
            result = m->nodes[(int) list[i]].goal;
      }

      rblxstb_arr_push(m->dfa_result, result);
   }

done:
   rblxstb_tempfree(data, state);
   return node;
}

static int rblxstb__matcher_dfa(rblxstb_matcher *m, char *str_c, int *len)
{
   rblxstb_uint8 *str = (rblxstb_uint8 *) str_c;
   rblxstb_int16 node,prevnode;
   rblxstb_dfa *trans;
   int match_length = 0;
   rblxstb_int16 match_result=0;

   if (m->dfa_start == RBLXSTB__DFA_UNDEF) {
      rblxstb_uint16 *list;

      m->num_words_per_dfa = (rblxstb_arr_len(m->nodes)+31) >> 5;
      rblxstb__optimize(m);

      list = rblxstb__add_if_inactive(m, NULL, m->start_node);
      list = rblxstb__eps_closure(m,list);
      if (m->does_lex) {
         m->dfa_start = rblxstb__get_dfa_node(m,list);
         rblxstb__clear(m, list);
         // DON'T allow start state to be a goal state!
         // this allows people to specify regexes that can match 0
         // characters without them actually matching (also we don't
         // check _before_ advancing anyway
         if (m->dfa_start <= RBLXSTB__DFA_MGOAL)
            m->dfa_start = -(m->dfa_start - RBLXSTB__DFA_MGOAL);
      } else {
         if (rblxstb__clear_goalcheck(m, list))
            m->dfa_start = RBLXSTB__DFA_GOAL;
         else
            m->dfa_start = rblxstb__get_dfa_node(m,list);
      }
      rblxstb_arr_free(list);
   }

   prevnode = RBLXSTB__DFA_UNDEF;
   node = m->dfa_start;
   trans = m->dfa;

   if (m->dfa_start == RBLXSTB__DFA_GOAL)
      return 1;

   for(;;) {
      assert(node >= RBLXSTB__DFA_VALID);

      // fast inner DFA loop; especially if RBLXSTB__DFA_VALID is 0

      do {
         prevnode = node;
         node = trans[node].transition[*str++];
      } while (node >= RBLXSTB__DFA_VALID);

      assert(node >= RBLXSTB__DFA_MGOAL - rblxstb_arr_len(m->dfa));
      assert(node < rblxstb_arr_len(m->dfa));

      // special case for lex: need _longest_ match, so notice goal
      // state without stopping
      if (node <= RBLXSTB__DFA_MGOAL) {
         match_length = (int) (str - (rblxstb_uint8 *) str_c);
         node = -(node - RBLXSTB__DFA_MGOAL);
         match_result = node;
         continue;
      }

      // slow NFA->DFA conversion

      // or we hit the goal or the end of the string, but those
      // can only happen once per search...

      if (node == RBLXSTB__DFA_UNDEF) {
         // build a list  -- @TODO special case <= 32 states
         // heck, use a more compact data structure for <= 16 and <= 8 ?!

         // @TODO keep states/newstates around instead of reallocating them
         rblxstb_uint16 *states = NULL;
         rblxstb_uint16 *newstates = NULL;
         int i,j,y,z;
         rblxstb_uint32 *flags = &m->dfa_mapping[prevnode * m->num_words_per_dfa];
         assert(prevnode != RBLXSTB__DFA_UNDEF);
         rblxstb_arr_setsize(states, 4);
         rblxstb_arr_setsize(newstates,4);
         for (j=0; j < m->num_words_per_dfa; ++j) {
            for (i=0; i < 32; ++i) {
               if (*flags & (1 << i))
                  rblxstb_arr_push(states, j*32+i);
            }
            ++flags;
         }
         // states is now the states we were in in the previous node;
         // so now we can compute what node it transitions to on str[-1]

         y = rblxstb_arr_len(states);
         for (i=0; i < y; ++i) {
            rblxstb_nfa_node *n = &m->nodes[(int) states[i]];
            z = rblxstb_arr_len(n->out);
            for (j=0; j < z; ++j) {
               if (n->out[j].match >= 0) {
                  if (n->out[j].match == str[-1] || (str[-1] == 0 && n->out[j].match == '\n'))
                     newstates = rblxstb__add_if_inactive(m, newstates, n->out[j].node);
               } else if (n->out[j].match == -1) {
                  if (str[-1] != '\n' && str[-1])
                     newstates = rblxstb__add_if_inactive(m, newstates, n->out[j].node);
               } else if (n->out[j].match < -1) {
                  int z = -n->out[j].match - 2;
                  if (m->charset[str[-1]] & (1 << z))
                     newstates = rblxstb__add_if_inactive(m, newstates, n->out[j].node);
               }
            }
         }
         // AND add in the start state!
         if (!m->match_start || (str[-1] == '\n' && !m->does_lex))
            newstates = rblxstb__add_if_inactive(m, newstates, m->start_node);
         // AND epsilon close it
         newstates = rblxstb__eps_closure(m, newstates);
         // if it's a goal state, then that's all there is to it
         if (rblxstb__clear_goalcheck(m, newstates)) {
            if (m->does_lex) {
               match_length = (int) (str - (rblxstb_uint8 *) str_c);
               node = rblxstb__get_dfa_node(m,newstates);
               match_result = node;
               node = -node + RBLXSTB__DFA_MGOAL;
               trans = m->dfa; // could have gotten realloc()ed
            } else
               node = RBLXSTB__DFA_GOAL;
         } else if (str[-1] == 0 || rblxstb_arr_len(newstates) == 0) {
            node = RBLXSTB__DFA_END;
         } else {
            node = rblxstb__get_dfa_node(m,newstates);
            trans = m->dfa; // could have gotten realloc()ed
         }
         trans[prevnode].transition[str[-1]] = node;
         if (node <= RBLXSTB__DFA_MGOAL)
            node = -(node - RBLXSTB__DFA_MGOAL);
         rblxstb_arr_free(newstates);
         rblxstb_arr_free(states);
      }

      if (node == RBLXSTB__DFA_GOAL) {
         return 1;
      }
      if (node == RBLXSTB__DFA_END) {
         if (m->does_lex) {
            if (match_result) {
               if (len) *len = match_length;
               return m->dfa_result[(int) match_result];
            }
         }
         return 0;
      }

      assert(node != RBLXSTB__DFA_UNDEF);
   }
}

int rblxstb_matcher_find(rblxstb_matcher *m, char *str)
{
   assert(m->does_lex == 0);
   return rblxstb__matcher_dfa(m, str, NULL);
}

int rblxstb_lex(rblxstb_matcher *m, char *str, int *len)
{
   assert(m->does_lex);
   return rblxstb__matcher_dfa(m, str, len);
}

#ifdef RBLXSTB_PERFECT_HASH
int rblxstb_regex(char *regex, char *str)
{
   static rblxstb_perfect p;
   static rblxstb_matcher ** matchers;
   static char        ** regexps;
   static char        ** regexp_cache;
   static unsigned short *mapping;
    int z = rblxstb_perfect_hash(&p, (int)(size_t) regex);
   if (z >= 0) {
      if (strcmp(regex, regexp_cache[(int) mapping[z]])) {
         int i = mapping[z];
         rblxstb_matcher_free(matchers[i]);
         free(regexp_cache[i]);
         regexps[i] = regex;
         regexp_cache[i] = rblxstb_p_strdup(regex);
         matchers[i] = rblxstb_regex_matcher(regex);
      }
   } else {
      int i,n;
      if (regex == NULL) {
         for (i=0; i < rblxstb_arr_len(matchers); ++i) {
            rblxstb_matcher_free(matchers[i]);
            free(regexp_cache[i]);
         }
         rblxstb_arr_free(matchers);
         rblxstb_arr_free(regexps);
         rblxstb_arr_free(regexp_cache);
         rblxstb_perfect_destroy(&p);
         free(mapping); mapping = NULL;
         return -1;
      }
      rblxstb_arr_push(regexps, regex);
      rblxstb_arr_push(regexp_cache, rblxstb_p_strdup(regex));
      rblxstb_arr_push(matchers, rblxstb_regex_matcher(regex));
      rblxstb_perfect_destroy(&p);
      n = rblxstb_perfect_create(&p, (unsigned int *) (char **) regexps, rblxstb_arr_len(regexps));
      mapping = (unsigned short *) realloc(mapping, n * sizeof(*mapping));
      for (i=0; i < rblxstb_arr_len(regexps); ++i)
          mapping[rblxstb_perfect_hash(&p, (int)(size_t) regexps[i])] = i;
      z = rblxstb_perfect_hash(&p, (int)(size_t) regex);
   }
   return rblxstb_matcher_find(matchers[(int) mapping[z]], str);
}
#endif
#endif // RBLXSTB_DEFINE


#if 0
//////////////////////////////////////////////////////////////////////////////
//
//                      C source-code introspection
//

// runtime structure
typedef struct
{
   char *name;
   char *type;     // base type
   char *comment;  // content of comment field
   int   size;     // size of base type
   int   offset;   // field offset
   int   arrcount[8]; // array sizes; -1 = pointer indirection; 0 = end of list
} rblxstb_info_field;

typedef struct
{
   char *structname;
   int size;
   int num_fields;
   rblxstb_info_field *fields;
} rblxstb_info_struct;

extern rblxstb_info_struct rblxstb_introspect_output[];

//

RBLXSTB_EXTERN void rblxstb_introspect_precompiled(rblxstb_info_struct *compiled);
RBLXSTB_EXTERN void rblxstb__introspect(char *path, char *file);

#define rblxstb_introspect_ship()            rblxstb__introspect(NULL, NULL, rblxstb__introspect_output)

#ifdef RBLXSTB_SHIP
#define rblxstb_introspect()                 rblxstb_introspect_ship()
#define rblxstb_introspect_path(p)           rblxstb_introspect_ship()
#else
// bootstrapping: define rblxstb_introspect() (or 'path') the first time
#define rblxstb_introspect()                 rblxstb__introspect(NULL, __FILE__, NULL)
#define rblxstb_introspect_auto()            rblxstb__introspect(NULL, __FILE__, rblxstb__introspect_output)

#define rblxstb_introspect_path(p)           rblxstb__introspect(p, __FILE__, NULL)
#define rblxstb_introspect_path(p)           rblxstb__introspect(p, __FILE__, NULL)
#endif

#ifdef RBLXSTB_DEFINE

#ifndef RBLXSTB_INTROSPECT_CPP
   #ifdef __cplusplus
   #define RBLXSTB_INTROSPECT_CPP 1
   #else
   #define RBLXSTB_INTROSPECT_CPP 0
   #endif
#endif

void rblxstb_introspect_precompiled(rblxstb_info_struct *compiled)
{

}


static void rblxstb__introspect_filename(char *buffer, char *path)
{
   #if RBLXSTB_INTROSPECT_CPP
   rblxstb_p_sprintf(buffer rblxstb_p_size(9999), "%s/rblxstb_introspect.cpp", path);
   #else
   rblxstb_p_sprintf(buffer rblxstb_p_size(9999), "%s/rblxstb_introspect.c", path);
   #endif
}

static void rblxstb__introspect_compute(char *path, char *file)
{
   int i;
   char ** include_list = NULL;
   char ** introspect_list = NULL;
   FILE *f;
   f = rblxstb_p_fopen(file, "w");
   if (!f) return;

   fputs("// if you get compiler errors, change the following 0 to a 1:\n", f);
   fputs("#define RBLXSTB_INTROSPECT_INVALID 0\n\n", f);
   fputs("// this will force the code to compile, and force the introspector\n", f);
   fputs("// to run and then exit, allowing you to recompile\n\n\n", f);
   fputs("#include \"rblxstb.h\"\n\n",f );
   fputs("#if RBLXSTB_INTROSPECT_INVALID\n", f);
   fputs("   rblxstb_info_struct rblxstb__introspect_output[] = { (void *) 1 }\n", f);
   fputs("#else\n\n", f);
   for (i=0; i < rblxstb_arr_len(include_list); ++i)
      fprintf(f, " #include \"%s\"\n", include_list[i]);

   fputs(" rblxstb_info_struct rblxstb__introspect_output[] =\n{\n", f);
   for (i=0; i < rblxstb_arr_len(introspect_list); ++i)
      fprintf(f, "  rblxstb_introspect_%s,\n", introspect_list[i]);
   fputs(" };\n", f);
   fputs("#endif\n", f);
   fclose(f);
}

static rblxstb_info_struct *rblxstb__introspect_info;

#ifndef RBLXSTB_SHIP

#endif

void rblxstb__introspect(char *path, char *file, rblxstb_info_struct *compiled)
{
   static int first=1;
   if (!first) return;
   first=0;

   rblxstb__introspect_info = compiled;

   #ifndef RBLXSTB_SHIP
   if (path || file) {
      int bail_flag = compiled && compiled[0].structname == (void *) 1;
      int needs_building = bail_flag;
      struct rblxstb__stat st;
      char buffer[1024], buffer2[1024];
      if (!path) {
         rblxstb_splitpath(buffer, file, RBLXSTB_PATH);
         path = buffer;
      }
      // bail if the source path doesn't exist
      if (!rblxstb_fexists(path)) return;

      rblxstb__introspect_filename(buffer2, path);

      // get source/include files timestamps, compare to output-file timestamp;
      // if mismatched, regenerate

      if (rblxstb__stat(buffer2, &st))
         needs_building = RBLXSTB_TRUE;

      {
         // find any file that contains an introspection command and is newer
         // if needs_building is already true, we don't need to do this test,
         // but we still need these arrays, so go ahead and get them
         char **all[3];
         all[0] = rblxstb_readdir_files_mask(path, "*.h");
         all[1] = rblxstb_readdir_files_mask(path, "*.c");
         all[2] = rblxstb_readdir_files_mask(path, "*.cpp");
         int i,j;
         if (needs_building) {
            for (j=0; j < 3; ++j) {
               for (i=0; i < rblxstb_arr_len(all[j]); ++i) {
                  struct rblxstb__stat st2;
                  if (!rblxstb__stat(all[j][i], &st2)) {
                     if (st.st_mtime < st2.st_mtime) {
                        char *z = rblxstb_filec(all[j][i], NULL);
                        int found=RBLXSTB_FALSE;
                        while (y) {
                           y = strstr(y, "//si");
                           if (y && isspace(y[4])) {
                              found = RBLXSTB_TRUE;
                              break;
                           }
                        }
                        needs_building = RBLXSTB_TRUE;
                        goto done;
                     }
                  }
               }
            }
           done:;
         }
               char *z = rblxstb_filec(all[i], NULL), *y = z;
               int found=RBLXSTB_FALSE;
               while (y) {
                  y = strstr(y, "//si");
                  if (y && isspace(y[4])) {
                     found = RBLXSTB_TRUE;
                     break;
                  }
               }
               if (found)
                  rblxstb_arr_push(introspect_h, rblxstb_p_strdup(all[i]));
               free(z);
            }
         }
         rblxstb_readdir_free(all);
         if (!needs_building) {
            for (i=0; i < rblxstb_arr_len(introspect_h); ++i) {
               struct rblxstb__stat st2;
               if (!rblxstb__stat(introspect_h[i], &st2))
                  if (st.st_mtime < st2.st_mtime)
                     needs_building = RBLXSTB_TRUE;
            }
         }

         if (needs_building) {
            rblxstb__introspect_compute(path, buffer2);
         }
      }
   }
   #endif
}
#endif
#endif

#ifdef RBLXSTB_INTROSPECT
// compile-time code-generator
#define INTROSPECT(x)   int main(int argc, char **argv) { rblxstb__introspect(__FILE__); return 0; }
#define FILE(x)

void rblxstb__introspect(char *filename)
{
   char *file = rblxstb_file(filename, NULL);
   char *s = file, *t, **p;
   char *out_name = "rblxstb_introspect.c";
   char *out_path;
   RBLXSTB_ARR(char) filelist = NULL;
   int i,n;
   if (!file) rblxstb_fatal("Couldn't open %s", filename);

   out_path = rblxstb_splitpathdup(filename, RBLXSTB_PATH);

   // search for the macros
   while (*s) {
      char buffer[256];
      while (*s && !isupper(*s)) ++s;
      s = rblxstb_strtok_invert(buffer, s, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
      s = rblxstb_skipwhite(s);
      if (*s == '(') {
         ++s;
         t = strchr(s, ')');
         if (t == NULL) rblxstb_fatal("Error parsing %s", filename);

      }
   }
}



#endif


//////////////////////////////////////////////////////////////////////////////
//
//             RBLXSTB-C sliding-window dictionary compression
//
//  This uses a DEFLATE-style sliding window, but no bitwise entropy.
//  Everything is on byte boundaries, so you could then apply a byte-wise
//  entropy code, though that's nowhere near as effective.
//
//  An RBLXSTB-C stream begins with a 16-byte header:
//      4 bytes: 0x57 0xBC 0x00 0x00
//      8 bytes: big-endian size of decompressed data, 64-bits
//      4 bytes: big-endian size of window (how far back decompressor may need)
//
//  The following symbols appear in the stream (these were determined ad hoc,
//  not by analysis):
//
//  [dict]      00000100 yyyyyyyy yyyyyyyy yyyyyyyy xxxxxxxx xxxxxxxx
//  [END]       00000101 11111010 cccccccc cccccccc cccccccc cccccccc
//  [dict]      00000110 yyyyyyyy yyyyyyyy yyyyyyyy xxxxxxxx
//  [literals]  00000111 zzzzzzzz zzzzzzzz
//  [literals]  00001zzz zzzzzzzz
//  [dict]      00010yyy yyyyyyyy yyyyyyyy xxxxxxxx xxxxxxxx
//  [dict]      00011yyy yyyyyyyy yyyyyyyy xxxxxxxx
//  [literals]  001zzzzz
//  [dict]      01yyyyyy yyyyyyyy xxxxxxxx
//  [dict]      1xxxxxxx yyyyyyyy
//
//      xxxxxxxx: match length - 1
//      yyyyyyyy: backwards distance - 1
//      zzzzzzzz: num literals - 1
//      cccccccc: adler32 checksum of decompressed data
//   (all big-endian)


RBLXSTB_EXTERN rblxstb_uint rblxstb_decompress_length(rblxstb_uchar *input);
RBLXSTB_EXTERN rblxstb_uint rblxstb_decompress(rblxstb_uchar *out,rblxstb_uchar *in,rblxstb_uint len);
RBLXSTB_EXTERN rblxstb_uint rblxstb_compress  (rblxstb_uchar *out,rblxstb_uchar *in,rblxstb_uint len);
RBLXSTB_EXTERN void rblxstb_compress_window(int z);
RBLXSTB_EXTERN void rblxstb_compress_hashsize(unsigned int z);

RBLXSTB_EXTERN int rblxstb_compress_tofile(char *filename, char *in,  rblxstb_uint  len);
RBLXSTB_EXTERN int rblxstb_compress_intofile(FILE *f, char *input,    rblxstb_uint  len);
RBLXSTB_EXTERN char *rblxstb_decompress_fromfile(char *filename,      rblxstb_uint *len);

RBLXSTB_EXTERN int rblxstb_compress_stream_start(FILE *f);
RBLXSTB_EXTERN void rblxstb_compress_stream_end(int close);
RBLXSTB_EXTERN void rblxstb_write(char *data, int data_len);

#ifdef RBLXSTB_DEFINE

rblxstb_uint rblxstb_decompress_length(rblxstb_uchar *input)
{
   return (input[8] << 24) + (input[9] << 16) + (input[10] << 8) + input[11];
}

////////////////////           decompressor         ///////////////////////

// simple implementation that just writes whole thing into big block

static unsigned char *rblxstb__barrier;
static unsigned char *rblxstb__barrier2;
static unsigned char *rblxstb__barrier3;
static unsigned char *rblxstb__barrier4;

static rblxstb_uchar *rblxstb__dout;
static void rblxstb__match(rblxstb_uchar *data, rblxstb_uint length)
{
   // INVERSE of memmove... write each byte before copying the next...
   assert (rblxstb__dout + length <= rblxstb__barrier);
   if (rblxstb__dout + length > rblxstb__barrier) { rblxstb__dout += length; return; }
   if (data < rblxstb__barrier4) { rblxstb__dout = rblxstb__barrier+1; return; }
   while (length--) *rblxstb__dout++ = *data++;
}

static void rblxstb__lit(rblxstb_uchar *data, rblxstb_uint length)
{
   assert (rblxstb__dout + length <= rblxstb__barrier);
   if (rblxstb__dout + length > rblxstb__barrier) { rblxstb__dout += length; return; }
   if (data < rblxstb__barrier2) { rblxstb__dout = rblxstb__barrier+1; return; }
   memcpy(rblxstb__dout, data, length);
   rblxstb__dout += length;
}

#define rblxstb__in2(x)   ((i[x] << 8) + i[(x)+1])
#define rblxstb__in3(x)   ((i[x] << 16) + rblxstb__in2((x)+1))
#define rblxstb__in4(x)   ((i[x] << 24) + rblxstb__in3((x)+1))

static rblxstb_uchar *rblxstb_decompress_token(rblxstb_uchar *i)
{
   if (*i >= 0x20) { // use fewer if's for cases that expand small
      if (*i >= 0x80)       rblxstb__match(rblxstb__dout-i[1]-1, i[0] - 0x80 + 1), i += 2;
      else if (*i >= 0x40)  rblxstb__match(rblxstb__dout-(rblxstb__in2(0) - 0x4000 + 1), i[2]+1), i += 3;
      else /* *i >= 0x20 */ rblxstb__lit(i+1, i[0] - 0x20 + 1), i += 1 + (i[0] - 0x20 + 1);
   } else { // more ifs for cases that expand large, since overhead is amortized
      if (*i >= 0x18)       rblxstb__match(rblxstb__dout-(rblxstb__in3(0) - 0x180000 + 1), i[3]+1), i += 4;
      else if (*i >= 0x10)  rblxstb__match(rblxstb__dout-(rblxstb__in3(0) - 0x100000 + 1), rblxstb__in2(3)+1), i += 5;
      else if (*i >= 0x08)  rblxstb__lit(i+2, rblxstb__in2(0) - 0x0800 + 1), i += 2 + (rblxstb__in2(0) - 0x0800 + 1);
      else if (*i == 0x07)  rblxstb__lit(i+3, rblxstb__in2(1) + 1), i += 3 + (rblxstb__in2(1) + 1);
      else if (*i == 0x06)  rblxstb__match(rblxstb__dout-(rblxstb__in3(1)+1), i[4]+1), i += 5;
      else if (*i == 0x04)  rblxstb__match(rblxstb__dout-(rblxstb__in3(1)+1), rblxstb__in2(4)+1), i += 6;
   }
   return i;
}

rblxstb_uint rblxstb_decompress(rblxstb_uchar *output, rblxstb_uchar *i, rblxstb_uint length)
{
   rblxstb_uint olen;
   if (rblxstb__in4(0) != 0x57bC0000) return 0;
   if (rblxstb__in4(4) != 0)          return 0; // error! stream is > 4GB
   olen = rblxstb_decompress_length(i);
   rblxstb__barrier2 = i;
   rblxstb__barrier3 = i+length;
   rblxstb__barrier = output + olen;
   rblxstb__barrier4 = output;
   i += 16;

   rblxstb__dout = output;
   while (1) {
      rblxstb_uchar *old_i = i;
      i = rblxstb_decompress_token(i);
      if (i == old_i) {
         if (*i == 0x05 && i[1] == 0xfa) {
            assert(rblxstb__dout == output + olen);
            if (rblxstb__dout != output + olen) return 0;
            if (rblxstb_adler32(1, output, olen) != (rblxstb_uint) rblxstb__in4(2))
               return 0;
            return olen;
         } else {
            assert(0); /* NOTREACHED */
            return 0;
         }
      }
      assert(rblxstb__dout <= output + olen);
      if (rblxstb__dout > output + olen)
         return 0;
   }
}

char *rblxstb_decompress_fromfile(char *filename, unsigned int *len)
{
   unsigned int n;
   char *q;
   unsigned char *p;
   FILE *f = rblxstb_p_fopen(filename, "rb");   if (f == NULL) return NULL;
   fseek(f, 0, SEEK_END);
   n = ftell(f);
   fseek(f, 0, SEEK_SET);
   p = (unsigned char * ) malloc(n); if (p == NULL) return NULL;
   fread(p, 1, n, f);
   fclose(f);
   if (p == NULL) return NULL;
   if (p[0] != 0x57 || p[1] != 0xBc || p[2] || p[3]) { free(p); return NULL; }
   q = (char *) malloc(rblxstb_decompress_length(p)+1);
   if (!q) { free(p); return NULL; }
   *len = rblxstb_decompress((unsigned char *) q, p, n);
   if (*len) q[*len] = 0;
   free(p);
   return q;
}

#if 0
//  streaming decompressor

static struct
{
   rblxstb__uchar *in_buffer;
   rblxstb__uchar *match;

   rblxstb__uint pending_literals;
   rblxstb__uint pending_match;
} xx;



static void rblxstb__match(rblxstb_uchar *data, rblxstb_uint length)
{
   // INVERSE of memmove... write each byte before copying the next...
   assert (rblxstb__dout + length <= rblxstb__barrier);
   if (rblxstb__dout + length > rblxstb__barrier) { rblxstb__dout += length; return; }
   if (data < rblxstb__barrier2) { rblxstb__dout = rblxstb__barrier+1; return; }
   while (length--) *rblxstb__dout++ = *data++;
}

static void rblxstb__lit(rblxstb_uchar *data, rblxstb_uint length)
{
   assert (rblxstb__dout + length <= rblxstb__barrier);
   if (rblxstb__dout + length > rblxstb__barrier) { rblxstb__dout += length; return; }
   if (data < rblxstb__barrier2) { rblxstb__dout = rblxstb__barrier+1; return; }
   memcpy(rblxstb__dout, data, length);
   rblxstb__dout += length;
}

static void sx_match(rblxstb_uchar *data, rblxstb_uint length)
{
   xx.match = data;
   xx.pending_match = length;
}

static void sx_lit(rblxstb_uchar *data, rblxstb_uint length)
{
   xx.pending_lit = length;
}

static int rblxstb_decompress_token_state(void)
{
   rblxstb__uchar *i = xx.in_buffer;

   if (*i >= 0x20) { // use fewer if's for cases that expand small
      if (*i >= 0x80)       sx_match(rblxstb__dout-i[1]-1, i[0] - 0x80 + 1), i += 2;
      else if (*i >= 0x40)  sx_match(rblxstb__dout-(rblxstb__in2(0) - 0x4000 + 1), i[2]+1), i += 3;
      else /* *i >= 0x20 */ sx_lit(i+1, i[0] - 0x20 + 1), i += 1;
   } else { // more ifs for cases that expand large, since overhead is amortized
      if (*i >= 0x18)       sx_match(rblxstb__dout-(rblxstb__in3(0) - 0x180000 + 1), i[3]+1), i += 4;
      else if (*i >= 0x10)  sx_match(rblxstb__dout-(rblxstb__in3(0) - 0x100000 + 1), rblxstb__in2(3)+1), i += 5;
      else if (*i >= 0x08)  sx_lit(i+2, rblxstb__in2(0) - 0x0800 + 1), i += 2;
      else if (*i == 0x07)  sx_lit(i+3, rblxstb__in2(1) + 1), i += 3;
      else if (*i == 0x06)  sx_match(rblxstb__dout-(rblxstb__in3(1)+1), i[4]+1), i += 5;
      else if (*i == 0x04)  sx_match(rblxstb__dout-(rblxstb__in3(1)+1), rblxstb__in2(4)+1), i += 6;
      else return 0;
   }
   xx.in_buffer = i;
   return 1;
}
#endif



////////////////////           compressor         ///////////////////////

static unsigned int rblxstb_matchlen(rblxstb_uchar *m1, rblxstb_uchar *m2, rblxstb_uint maxlen)
{
   rblxstb_uint i;
   for (i=0; i < maxlen; ++i)
      if (m1[i] != m2[i]) return i;
   return i;
}

// simple implementation that just takes the source data in a big block

static rblxstb_uchar *rblxstb__out;
static FILE      *rblxstb__outfile;
static rblxstb_uint   rblxstb__outbytes;

static void rblxstb__write(unsigned char v)
{
   fputc(v, rblxstb__outfile);
   ++rblxstb__outbytes;
}

#define rblxstb_out(v)    (rblxstb__out ? (void)(*rblxstb__out++ = (rblxstb_uchar) (v)) : rblxstb__write((rblxstb_uchar) (v)))

static void rblxstb_out2(rblxstb_uint v)
{
   rblxstb_out(v >> 8);
   rblxstb_out(v);
}

static void rblxstb_out3(rblxstb_uint v) { rblxstb_out(v >> 16); rblxstb_out(v >> 8); rblxstb_out(v); }
static void rblxstb_out4(rblxstb_uint v) { rblxstb_out(v >> 24); rblxstb_out(v >> 16);
                                   rblxstb_out(v >> 8 ); rblxstb_out(v);                  }

static void outliterals(rblxstb_uchar *in, ptrdiff_t numlit)
{
   while (numlit > 65536) {
      outliterals(in,65536);
      in     += 65536;
      numlit -= 65536;
   }

   if      (numlit ==     0)    ;
   else if (numlit <=    32)    rblxstb_out (0x000020 + (rblxstb_uint) numlit-1);
   else if (numlit <=  2048)    rblxstb_out2(0x000800 + (rblxstb_uint) numlit-1);
   else /*  numlit <= 65536) */ rblxstb_out3(0x070000 + (rblxstb_uint) numlit-1);

   if (rblxstb__out) {
      memcpy(rblxstb__out,in,numlit);
      rblxstb__out += numlit;
   } else
      fwrite(in, 1, numlit, rblxstb__outfile);
}

static int rblxstb__window = 0x40000; // 256K
void rblxstb_compress_window(int z)
{
   if (z >= 0x1000000) z = 0x1000000; // limit of implementation
   if (z <      0x100) z = 0x100;   // insanely small
   rblxstb__window = z;
}

static int rblxstb_not_crap(int best, int dist)
{
   return   ((best > 2  &&  dist <= 0x00100)
          || (best > 5  &&  dist <= 0x04000)
          || (best > 7  &&  dist <= 0x80000));
}

static  rblxstb_uint rblxstb__hashsize = 32768;
void rblxstb_compress_hashsize(unsigned int y)
{
   unsigned int z = 1024;
   while (z < y) z <<= 1;
   rblxstb__hashsize = z >> 2;   // pass in bytes, store #pointers
}

// note that you can play with the hashing functions all you
// want without needing to change the decompressor
#define rblxstb__hc(q,h,c)      (((h) << 7) + ((h) >> 25) + q[c])
#define rblxstb__hc2(q,h,c,d)   (((h) << 14) + ((h) >> 18) + (q[c] << 7) + q[d])
#define rblxstb__hc3(q,c,d,e)   ((q[c] << 14) + (q[d] << 7) + q[e])

static rblxstb_uint32 rblxstb__running_adler;

static int rblxstb_compress_chunk(rblxstb_uchar *history,
                              rblxstb_uchar *start,
                              rblxstb_uchar *end,
                              int length,
                              int *pending_literals,
                              rblxstb_uchar **chash,
                              rblxstb_uint mask)
{
   int window = rblxstb__window;
   rblxstb_uint match_max;
   rblxstb_uchar *lit_start = start - *pending_literals;
   rblxstb_uchar *q = start;

   #define RBLXSTB__SCRAMBLE(h)   (((h) + ((h) >> 16)) & mask)

   // stop short of the end so we don't scan off the end doing
   // the hashing; this means we won't compress the last few bytes
   // unless they were part of something longer
   while (q < start+length && q+12 < end) {
      int m;
      rblxstb_uint h1,h2,h3,h4, h;
      rblxstb_uchar *t;
      int best = 2, dist=0;

      if (q+65536 > end)
         match_max = (rblxstb_uint) (end-q);
      else
         match_max = 65536u;

      #define rblxstb__nc(b,d)  ((d) <= window && ((b) > 9 || rblxstb_not_crap(b,d)))

      #define RBLXSTB__TRY(t,p)  /* avoid retrying a match we already tried */ \
                      if (p ? dist != (int) (q-t) : 1)                     \
                      if ((m = (int) rblxstb_matchlen(t, q, match_max)) > best)\
                      if (rblxstb__nc(m,(int) (q-(t))))                        \
                          best = m, dist = (int) (q - (t))

      // rather than search for all matches, only try 4 candidate locations,
      // chosen based on 4 different hash functions of different lengths.
      // this strategy is inspired by LZO; hashing is unrolled here using the
      // 'hc' macro
      h = rblxstb__hc3(q,0, 1, 2); h1 = RBLXSTB__SCRAMBLE(h);
                                      t = chash[h1]; if (t) RBLXSTB__TRY(t,0);
      h = rblxstb__hc2(q,h, 3, 4); h2 = RBLXSTB__SCRAMBLE(h);
      h = rblxstb__hc2(q,h, 5, 6);        t = chash[h2]; if (t) RBLXSTB__TRY(t,1);
      h = rblxstb__hc2(q,h, 7, 8); h3 = RBLXSTB__SCRAMBLE(h);
      h = rblxstb__hc2(q,h, 9,10);        t = chash[h3]; if (t) RBLXSTB__TRY(t,1);
      h = rblxstb__hc2(q,h,11,12); h4 = RBLXSTB__SCRAMBLE(h);
                                      t = chash[h4]; if (t) RBLXSTB__TRY(t,1);

      // because we use a shared hash table, can only update it
      // _after_ we've probed all of them
      chash[h1] = chash[h2] = chash[h3] = chash[h4] = q;

      if (best > 2)
         assert(dist > 0);

      // see if our best match qualifies
      if (best < 3) { // fast path literals
         ++q;
      } else if (best > 2  &&  best <= 0x80    &&  dist <= 0x100) {
         outliterals(lit_start, q-lit_start); lit_start = (q += best);
         rblxstb_out(0x80 + best-1);
         rblxstb_out(dist-1);
      } else if (best > 5  &&  best <= 0x100   &&  dist <= 0x4000) {
         outliterals(lit_start, q-lit_start); lit_start = (q += best);
         rblxstb_out2(0x4000 + dist-1);
         rblxstb_out(best-1);
      } else if (best > 7  &&  best <= 0x100   &&  dist <= 0x80000) {
         outliterals(lit_start, q-lit_start); lit_start = (q += best);
         rblxstb_out3(0x180000 + dist-1);
         rblxstb_out(best-1);
      } else if (best > 8  &&  best <= 0x10000 &&  dist <= 0x80000) {
         outliterals(lit_start, q-lit_start); lit_start = (q += best);
         rblxstb_out3(0x100000 + dist-1);
         rblxstb_out2(best-1);
      } else if (best > 9                      &&  dist <= 0x1000000) {
         if (best > 65536) best = 65536;
         outliterals(lit_start, q-lit_start); lit_start = (q += best);
         if (best <= 0x100) {
            rblxstb_out(0x06);
            rblxstb_out3(dist-1);
            rblxstb_out(best-1);
         } else {
            rblxstb_out(0x04);
            rblxstb_out3(dist-1);
            rblxstb_out2(best-1);
         }
      } else {  // fallback literals if no match was a balanced tradeoff
         ++q;
      }
   }

   // if we didn't get all the way, add the rest to literals
   if (q-start < length)
      q = start+length;

   // the literals are everything from lit_start to q
   *pending_literals = (int) (q - lit_start);

   rblxstb__running_adler = rblxstb_adler32(rblxstb__running_adler, start, (int) (q - start));
   return (int) (q - start);
}

static int rblxstb_compress_inner(rblxstb_uchar *input, rblxstb_uint length)
{
   int literals = 0;
   rblxstb_uint len,i;

   rblxstb_uchar **chash;
   chash = (rblxstb_uchar**) malloc(rblxstb__hashsize * sizeof(rblxstb_uchar*));
   if (chash == NULL) return 0; // failure
   for (i=0; i < rblxstb__hashsize; ++i)
      chash[i] = NULL;

   // stream signature
   rblxstb_out(0x57); rblxstb_out(0xbc);
   rblxstb_out2(0);

   rblxstb_out4(0);       // 64-bit length requires 32-bit leading 0
   rblxstb_out4(length);
   rblxstb_out4(rblxstb__window);

   rblxstb__running_adler = 1;

   len = rblxstb_compress_chunk(input, input, input+length, length, &literals, chash, rblxstb__hashsize-1);
   assert(len == length);

   outliterals(input+length - literals, literals);

   free(chash);

   rblxstb_out2(0x05fa); // end opcode

   rblxstb_out4(rblxstb__running_adler);

   return 1; // success
}

rblxstb_uint rblxstb_compress(rblxstb_uchar *out, rblxstb_uchar *input, rblxstb_uint length)
{
   rblxstb__out = out;
   rblxstb__outfile = NULL;

   rblxstb_compress_inner(input, length);

   return (rblxstb_uint) (rblxstb__out - out);
}

int rblxstb_compress_tofile(char *filename, char *input, unsigned int length)
{
   //int maxlen = length + 512 + (length >> 2); // total guess
   //char *buffer = (char *) malloc(maxlen);
   //int blen = rblxstb_compress((rblxstb_uchar*)buffer, (rblxstb_uchar*)input, length);

   rblxstb__out = NULL;
   rblxstb__outfile = rblxstb_p_fopen(filename, "wb");
   if (!rblxstb__outfile) return 0;

   rblxstb__outbytes = 0;

   if (!rblxstb_compress_inner((rblxstb_uchar*)input, length))
      return 0;

   fclose(rblxstb__outfile);

   return rblxstb__outbytes;
}

int rblxstb_compress_intofile(FILE *f, char *input, unsigned int length)
{
   //int maxlen = length + 512 + (length >> 2); // total guess
   //char *buffer = (char*)malloc(maxlen);
   //int blen = rblxstb_compress((rblxstb_uchar*)buffer, (rblxstb_uchar*)input, length);

   rblxstb__out = NULL;
   rblxstb__outfile = f;
   if (!rblxstb__outfile) return 0;

   rblxstb__outbytes = 0;

   if (!rblxstb_compress_inner((rblxstb_uchar*)input, length))
      return 0;

   return rblxstb__outbytes;
}

//////////////////////    streaming I/O version    /////////////////////


static size_t rblxstb_out_backpatch_id(void)
{
   if (rblxstb__out)
      return (size_t) rblxstb__out;
   else
      return ftell(rblxstb__outfile);
}

static void rblxstb_out_backpatch(size_t id, rblxstb_uint value)
{
   rblxstb_uchar data[4] = { (rblxstb_uchar)(value >> 24), (rblxstb_uchar)(value >> 16), (rblxstb_uchar)(value >> 8), (rblxstb_uchar)(value) };
   if (rblxstb__out) {
      memcpy((void *) id, data, 4);
   } else {
      rblxstb_uint where = ftell(rblxstb__outfile);
      fseek(rblxstb__outfile, (long) id, SEEK_SET);
      fwrite(data, 4, 1, rblxstb__outfile);
      fseek(rblxstb__outfile, where, SEEK_SET);
   }
}

// ok, the wraparound buffer was a total failure. let's instead
// use a copying-in-place buffer, which lets us share the code.
// This is way less efficient but it'll do for now.

static struct
{
   rblxstb_uchar *buffer;
   int size;           // physical size of buffer in bytes

   int valid;          // amount of valid data in bytes
   int start;          // bytes of data already output

   int window;
   int fsize;

   int pending_literals; // bytes not-quite output but counted in start
   int length_id;

   rblxstb_uint total_bytes;

   rblxstb_uchar **chash;
   rblxstb_uint    hashmask;
} xtb;

static int rblxstb_compress_streaming_start(void)
{
   rblxstb_uint i;
   xtb.size = rblxstb__window * 3;
   xtb.buffer = (rblxstb_uchar*)malloc(xtb.size);
   if (!xtb.buffer) return 0;

   xtb.chash = (rblxstb_uchar**)malloc(sizeof(*xtb.chash) * rblxstb__hashsize);
   if (!xtb.chash) {
      free(xtb.buffer);
      return 0;
   }

   for (i=0; i < rblxstb__hashsize; ++i)
      xtb.chash[i] = NULL;

   xtb.hashmask = rblxstb__hashsize-1;

   xtb.valid        = 0;
   xtb.start        = 0;
   xtb.window       = rblxstb__window;
   xtb.fsize        = rblxstb__window;
   xtb.pending_literals = 0;
   xtb.total_bytes  = 0;

      // stream signature
   rblxstb_out(0x57); rblxstb_out(0xbc); rblxstb_out2(0);

   rblxstb_out4(0);       // 64-bit length requires 32-bit leading 0

   xtb.length_id = (int) rblxstb_out_backpatch_id();
   rblxstb_out4(0);       // we don't know the output length yet

   rblxstb_out4(rblxstb__window);

   rblxstb__running_adler = 1;

   return 1;
}

static int rblxstb_compress_streaming_end(void)
{
   // flush out any remaining data
   rblxstb_compress_chunk(xtb.buffer, xtb.buffer+xtb.start, xtb.buffer+xtb.valid,
                      xtb.valid-xtb.start, &xtb.pending_literals, xtb.chash, xtb.hashmask);

   // write out pending literals
   outliterals(xtb.buffer + xtb.valid - xtb.pending_literals, xtb.pending_literals);

   rblxstb_out2(0x05fa); // end opcode
   rblxstb_out4(rblxstb__running_adler);

   rblxstb_out_backpatch(xtb.length_id, xtb.total_bytes);

   free(xtb.buffer);
   free(xtb.chash);
   return 1;
}

void rblxstb_write(char *data, int data_len)
{
   rblxstb_uint i;

   // @TODO: fast path for filling the buffer and doing nothing else
   //   if (xtb.valid + data_len < xtb.size)

   xtb.total_bytes += data_len;

   while (data_len) {
      // fill buffer
      if (xtb.valid < xtb.size) {
         int amt = xtb.size - xtb.valid;
         if (data_len < amt) amt = data_len;
         memcpy(xtb.buffer + xtb.valid, data, amt);
         data_len -= amt;
         data     += amt;
         xtb.valid += amt;
      }
      if (xtb.valid < xtb.size)
         return;

      // at this point, the buffer is full

      // if we can process some data, go for it; make sure
      // we leave an 'fsize's worth of data, though
      if (xtb.start + xtb.fsize < xtb.valid) {
         int amount = (xtb.valid - xtb.fsize) - xtb.start;
         int n;
         assert(amount > 0);
         n = rblxstb_compress_chunk(xtb.buffer, xtb.buffer + xtb.start, xtb.buffer + xtb.valid,
                                amount, &xtb.pending_literals, xtb.chash, xtb.hashmask);
         xtb.start += n;
      }

      assert(xtb.start + xtb.fsize >= xtb.valid);
      // at this point, our future size is too small, so we
      // need to flush some history. we, in fact, flush exactly
      // one window's worth of history

      {
         int flush = xtb.window;
         assert(xtb.start >= flush);
         assert(xtb.valid >= flush);

         // if 'pending literals' extends back into the shift region,
         // write them out
         if (xtb.start - xtb.pending_literals < flush) {
            outliterals(xtb.buffer + xtb.start - xtb.pending_literals, xtb.pending_literals);
            xtb.pending_literals = 0;
         }

         // now shift the window
         memmove(xtb.buffer, xtb.buffer + flush, xtb.valid - flush);
         xtb.start -= flush;
         xtb.valid -= flush;

         for (i=0; i <= xtb.hashmask; ++i)
            if (xtb.chash[i] < xtb.buffer + flush)
               xtb.chash[i] = NULL;
            else
               xtb.chash[i] -= flush;
      }
      // and now that we've made room for more data, go back to the top
   }
}

int rblxstb_compress_stream_start(FILE *f)
{
   rblxstb__out = NULL;
   rblxstb__outfile = f;

   if (f == NULL)
      return 0;

   if (!rblxstb_compress_streaming_start())
      return 0;

   return 1;
}

void rblxstb_compress_stream_end(int close)
{
   rblxstb_compress_streaming_end();
   if (close && rblxstb__outfile) {
      fclose(rblxstb__outfile);
   }
}

#endif // RBLXSTB_DEFINE

//////////////////////////////////////////////////////////////////////////////
//
//  File abstraction... tired of not having this... we can write
//  compressors to be layers over these that auto-close their children.


typedef struct rblxstbfile
{
   int (*getbyte)(struct rblxstbfile *);  // -1 on EOF
   unsigned int (*getdata)(struct rblxstbfile *, void *block, unsigned int len);

   int (*putbyte)(struct rblxstbfile *, int byte);
   unsigned int (*putdata)(struct rblxstbfile *, void *block, unsigned int len);

   unsigned int (*size)(struct rblxstbfile *);

   unsigned int (*tell)(struct rblxstbfile *);
   void (*backpatch)(struct rblxstbfile *, unsigned int tell, void *block, unsigned int len);

   void (*close)(struct rblxstbfile *);

   FILE *f;  // file to fread/fwrite
   unsigned char *buffer; // input/output buffer
   unsigned char *indata, *inend; // input buffer
   union {
      int various;
      void *ptr;
   };
} rblxstbfile;

RBLXSTB_EXTERN unsigned int rblxstb_getc(rblxstbfile *f); // read
RBLXSTB_EXTERN int rblxstb_putc(rblxstbfile *f, int ch); // write
RBLXSTB_EXTERN unsigned int rblxstb_getdata(rblxstbfile *f, void *buffer, unsigned int len); // read
RBLXSTB_EXTERN unsigned int rblxstb_putdata(rblxstbfile *f, void *buffer, unsigned int len); // write
RBLXSTB_EXTERN unsigned int rblxstb_tell(rblxstbfile *f); // read
RBLXSTB_EXTERN unsigned int rblxstb_size(rblxstbfile *f); // read/write
RBLXSTB_EXTERN void rblxstb_backpatch(rblxstbfile *f, unsigned int tell, void *buffer, unsigned int len); // write

#ifdef RBLXSTB_DEFINE

unsigned int rblxstb_getc(rblxstbfile *f) { return f->getbyte(f); }
int rblxstb_putc(rblxstbfile *f, int ch)  { return f->putbyte(f, ch); }

unsigned int rblxstb_getdata(rblxstbfile *f, void *buffer, unsigned int len)
{
   return f->getdata(f, buffer, len);
}
unsigned int rblxstb_putdata(rblxstbfile *f, void *buffer, unsigned int len)
{
   return f->putdata(f, buffer, len);
}
void rblxstb_close(rblxstbfile *f)
{
   f->close(f);
   free(f);
}
unsigned int rblxstb_tell(rblxstbfile *f) { return f->tell(f); }
unsigned int rblxstb_size(rblxstbfile *f) { return f->size(f); }
void rblxstb_backpatch(rblxstbfile *f, unsigned int tell, void *buffer, unsigned int len)
{
   f->backpatch(f,tell,buffer,len);
}

// FILE * implementation
static int rblxstb__fgetbyte(rblxstbfile *f) { return fgetc(f->f); }
static int rblxstb__fputbyte(rblxstbfile *f, int ch) { return fputc(ch, f->f)==0; }
static unsigned int rblxstb__fgetdata(rblxstbfile *f, void *buffer, unsigned int len) { return (unsigned int) fread(buffer,1,len,f->f); }
static unsigned int rblxstb__fputdata(rblxstbfile *f, void *buffer, unsigned int len) { return (unsigned int) fwrite(buffer,1,len,f->f); }
static unsigned int rblxstb__fsize(rblxstbfile *f) { return (unsigned int) rblxstb_filelen(f->f); }
static unsigned int rblxstb__ftell(rblxstbfile *f) { return (unsigned int) ftell(f->f); }
static void rblxstb__fbackpatch(rblxstbfile *f, unsigned int where, void *buffer, unsigned int len)
{
   fseek(f->f, where, SEEK_SET);
   fwrite(buffer, 1, len, f->f);
   fseek(f->f, 0, SEEK_END);
}
static void         rblxstb__fclose(rblxstbfile *f) { fclose(f->f); }

rblxstbfile *rblxstb_openf(FILE *f)
{
   rblxstbfile m = { rblxstb__fgetbyte, rblxstb__fgetdata,
                 rblxstb__fputbyte, rblxstb__fputdata,
                 rblxstb__fsize, rblxstb__ftell, rblxstb__fbackpatch, rblxstb__fclose,
                 0,0,0, };
   rblxstbfile *z = (rblxstbfile *) malloc(sizeof(*z));
   if (z) {
      *z = m;
      z->f = f;
   }
   return z;
}

static int rblxstb__nogetbyte(rblxstbfile *f) { assert(0); return -1; }
static unsigned int rblxstb__nogetdata(rblxstbfile *f, void *buffer, unsigned int len) { assert(0); return 0; }
static int rblxstb__noputbyte(rblxstbfile *f, int ch) { assert(0); return 0; }
static unsigned int rblxstb__noputdata(rblxstbfile *f, void *buffer, unsigned int len) { assert(0); return 0; }
static void rblxstb__nobackpatch(rblxstbfile *f, unsigned int where, void *buffer, unsigned int len) { assert(0); }

static int rblxstb__bgetbyte(rblxstbfile *s)
{
   if (s->indata < s->inend)
      return *s->indata++;
   else
      return -1;
}

static unsigned int rblxstb__bgetdata(rblxstbfile *s, void *buffer, unsigned int len)
{
   if (s->indata + len > s->inend)
      len = (unsigned int) (s->inend - s->indata);
   memcpy(buffer, s->indata, len);
   s->indata += len;
   return len;
}
static unsigned int rblxstb__bsize(rblxstbfile *s) { return (unsigned int) (s->inend  - s->buffer); }
static unsigned int rblxstb__btell(rblxstbfile *s) { return (unsigned int) (s->indata - s->buffer); }

static void rblxstb__bclose(rblxstbfile *s)
{
   if (s->various)
      free(s->buffer);
}

rblxstbfile *rblxstb_open_inbuffer(void *buffer, unsigned int len)
{
   rblxstbfile m = { rblxstb__bgetbyte, rblxstb__bgetdata,
                 rblxstb__noputbyte, rblxstb__noputdata,
                 rblxstb__bsize, rblxstb__btell, rblxstb__nobackpatch, rblxstb__bclose };
   rblxstbfile *z = (rblxstbfile *) malloc(sizeof(*z));
   if (z) {
      *z = m;
      z->buffer = (unsigned char *) buffer;
      z->indata = z->buffer;
      z->inend = z->indata + len;
   }
   return z;
}

rblxstbfile *rblxstb_open_inbuffer_free(void *buffer, unsigned int len)
{
   rblxstbfile *z = rblxstb_open_inbuffer(buffer, len);
   if (z)
      z->various = 1; // free
   return z;
}

#ifndef RBLXSTB_VERSION
// if we've been cut-and-pasted elsewhere, you get a limited
// version of rblxstb_open, without the 'k' flag and utf8 support
static void rblxstb__fclose2(rblxstbfile *f)
{
   fclose(f->f);
}

rblxstbfile *rblxstb_open(char *filename, char *mode)
{
   FILE *f = rblxstb_p_fopen(filename, mode);
   rblxstbfile *s;
   if (f == NULL) return NULL;
   s = rblxstb_openf(f);
   if (s)
      s->close = rblxstb__fclose2;
   return s;
}
#else
// the full version depends on some code in rblxstb.h; this
// also includes the memory buffer output format implemented with rblxstb_arr
static void rblxstb__fclose2(rblxstbfile *f)
{
   rblxstb_fclose(f->f, f->various);
}

rblxstbfile *rblxstb_open(char *filename, char *mode)
{
   FILE *f = rblxstb_fopen(filename, mode[0] == 'k' ? mode+1 : mode);
   rblxstbfile *s;
   if (f == NULL) return NULL;
   s = rblxstb_openf(f);
   if (s) {
      s->close = rblxstb__fclose2;
      s->various = mode[0] == 'k' ? rblxstb_keep_if_different : rblxstb_keep_yes;
   }
   return s;
}

static int rblxstb__aputbyte(rblxstbfile *f, int ch)
{
   rblxstb_arr_push(f->buffer, ch);
   return 1;
}
static unsigned int rblxstb__aputdata(rblxstbfile *f, void *data, unsigned int len)
{
   memcpy(rblxstb_arr_addn(f->buffer, (int) len), data, len);
   return len;
}
static unsigned int rblxstb__asize(rblxstbfile *f) { return rblxstb_arr_len(f->buffer); }
static void rblxstb__abackpatch(rblxstbfile *f, unsigned int where, void *data, unsigned int len)
{
   memcpy(f->buffer+where, data, len);
}
static void rblxstb__aclose(rblxstbfile *f)
{
   *(unsigned char **) f->ptr = f->buffer;
}

rblxstbfile *rblxstb_open_outbuffer(unsigned char **update_on_close)
{
   rblxstbfile m = { rblxstb__nogetbyte, rblxstb__nogetdata,
                 rblxstb__aputbyte, rblxstb__aputdata,
                 rblxstb__asize, rblxstb__asize, rblxstb__abackpatch, rblxstb__aclose };
   rblxstbfile *z = (rblxstbfile *) malloc(sizeof(*z));
   if (z) {
      z->ptr = update_on_close;
      *z = m;
   }
   return z;
}
#endif
#endif


//////////////////////////////////////////////////////////////////////////////
//
//  Arithmetic coder... based on cbloom's notes on the subject, should be
//  less code than a huffman code.

typedef struct
{
   unsigned int range_low;
   unsigned int range_high;
   unsigned int code, range; // decode
   int buffered_u8;
   int pending_ffs;
   rblxstbfile *output;
} rblxstb_arith;

RBLXSTB_EXTERN void rblxstb_arith_init_encode(rblxstb_arith *a, rblxstbfile *out);
RBLXSTB_EXTERN void rblxstb_arith_init_decode(rblxstb_arith *a, rblxstbfile *in);
RBLXSTB_EXTERN rblxstbfile *rblxstb_arith_encode_close(rblxstb_arith *a);
RBLXSTB_EXTERN rblxstbfile *rblxstb_arith_decode_close(rblxstb_arith *a);

RBLXSTB_EXTERN void rblxstb_arith_encode(rblxstb_arith *a, unsigned int totalfreq, unsigned int freq, unsigned int cumfreq);
RBLXSTB_EXTERN void rblxstb_arith_encode_log2(rblxstb_arith *a, unsigned int totalfreq2, unsigned int freq, unsigned int cumfreq);
RBLXSTB_EXTERN unsigned int rblxstb_arith_decode_value(rblxstb_arith *a, unsigned int totalfreq);
RBLXSTB_EXTERN void rblxstb_arith_decode_advance(rblxstb_arith *a, unsigned int totalfreq, unsigned int freq, unsigned int cumfreq);
RBLXSTB_EXTERN unsigned int rblxstb_arith_decode_value_log2(rblxstb_arith *a, unsigned int totalfreq2);
RBLXSTB_EXTERN void rblxstb_arith_decode_advance_log2(rblxstb_arith *a, unsigned int totalfreq2, unsigned int freq, unsigned int cumfreq);

RBLXSTB_EXTERN void rblxstb_arith_encode_byte(rblxstb_arith *a, int byte);
RBLXSTB_EXTERN int  rblxstb_arith_decode_byte(rblxstb_arith *a);

// this is a memory-inefficient way of doing things, but it's
// fast(?) and simple
typedef struct
{
   unsigned short cumfreq;
   unsigned short samples;
} rblxstb_arith_symstate_item;

typedef struct
{
   int num_sym;
   unsigned int pow2;
   int countdown;
   rblxstb_arith_symstate_item data[1];
} rblxstb_arith_symstate;

#ifdef RBLXSTB_DEFINE
void rblxstb_arith_init_encode(rblxstb_arith *a, rblxstbfile *out)
{
   a->range_low = 0;
   a->range_high = 0xffffffff;
   a->pending_ffs = -1; // means no buffered character currently, to speed up normal case
   a->output = out;
}

static void rblxstb__arith_carry(rblxstb_arith *a)
{
   int i;
   assert(a->pending_ffs != -1); // can't carry with no data
   rblxstb_putc(a->output, a->buffered_u8);
   for (i=0; i < a->pending_ffs; ++i)
      rblxstb_putc(a->output, 0);
}

static void rblxstb__arith_putbyte(rblxstb_arith *a, int byte)
{
   if (a->pending_ffs) {
      if (a->pending_ffs == -1) { // means no buffered data; encoded for fast path efficiency
         if (byte == 0xff)
            rblxstb_putc(a->output, byte); // just write it immediately
         else {
            a->buffered_u8 = byte;
            a->pending_ffs = 0;
         }
      } else if (byte == 0xff) {
         ++a->pending_ffs;
      } else {
         int i;
         rblxstb_putc(a->output, a->buffered_u8);
         for (i=0; i < a->pending_ffs; ++i)
            rblxstb_putc(a->output, 0xff);
      }
   } else if (byte == 0xff) {
      ++a->pending_ffs;
   } else {
      // fast path
      rblxstb_putc(a->output, a->buffered_u8);
      a->buffered_u8 = byte;
   }
}

static void rblxstb__arith_flush(rblxstb_arith *a)
{
   if (a->pending_ffs >= 0) {
      int i;
      rblxstb_putc(a->output, a->buffered_u8);
      for (i=0; i < a->pending_ffs; ++i)
         rblxstb_putc(a->output, 0xff);
   }
}

static void rblxstb__renorm_encoder(rblxstb_arith *a)
{
   rblxstb__arith_putbyte(a, a->range_low >> 24);
   a->range_low <<= 8;
   a->range_high = (a->range_high << 8) | 0xff;
}

static void rblxstb__renorm_decoder(rblxstb_arith *a)
{
   int c = rblxstb_getc(a->output);
   a->code = (a->code << 8) + (c >= 0 ? c : 0); // if EOF, insert 0
}

void rblxstb_arith_encode(rblxstb_arith *a, unsigned int totalfreq, unsigned int freq, unsigned int cumfreq)
{
   unsigned int range = a->range_high - a->range_low;
   unsigned int old = a->range_low;
   range /= totalfreq;
   a->range_low += range * cumfreq;
   a->range_high = a->range_low + range*freq;
   if (a->range_low < old)
      rblxstb__arith_carry(a);
   while (a->range_high - a->range_low < 0x1000000)
      rblxstb__renorm_encoder(a);
}

void rblxstb_arith_encode_log2(rblxstb_arith *a, unsigned int totalfreq2, unsigned int freq, unsigned int cumfreq)
{
   unsigned int range = a->range_high - a->range_low;
   unsigned int old = a->range_low;
   range >>= totalfreq2;
   a->range_low += range * cumfreq;
   a->range_high = a->range_low + range*freq;
   if (a->range_low < old)
      rblxstb__arith_carry(a);
   while (a->range_high - a->range_low < 0x1000000)
      rblxstb__renorm_encoder(a);
}

unsigned int rblxstb_arith_decode_value(rblxstb_arith *a, unsigned int totalfreq)
{
   unsigned int freqsize = a->range / totalfreq;
   unsigned int z = a->code / freqsize;
   return z >= totalfreq ? totalfreq-1 : z;
}

void rblxstb_arith_decode_advance(rblxstb_arith *a, unsigned int totalfreq, unsigned int freq, unsigned int cumfreq)
{
   unsigned int freqsize = a->range / totalfreq; // @OPTIMIZE, share with above divide somehow?
   a->code -= freqsize * cumfreq;
   a->range = freqsize * freq;
   while (a->range < 0x1000000)
      rblxstb__renorm_decoder(a);
}

unsigned int rblxstb_arith_decode_value_log2(rblxstb_arith *a, unsigned int totalfreq2)
{
   unsigned int freqsize = a->range >> totalfreq2;
   unsigned int z = a->code / freqsize;
   return z >= (1U<<totalfreq2) ? (1U<<totalfreq2)-1 : z;
}

void rblxstb_arith_decode_advance_log2(rblxstb_arith *a, unsigned int totalfreq2, unsigned int freq, unsigned int cumfreq)
{
   unsigned int freqsize = a->range >> totalfreq2;
   a->code -= freqsize * cumfreq;
   a->range = freqsize * freq;
   while (a->range < 0x1000000)
      rblxstb__renorm_decoder(a);
}

rblxstbfile *rblxstb_arith_encode_close(rblxstb_arith *a)
{
   // put exactly as many bytes as we'll read, so we can turn on/off arithmetic coding in a stream
   rblxstb__arith_putbyte(a, a->range_low >> 24);
   rblxstb__arith_putbyte(a, a->range_low >> 16);
   rblxstb__arith_putbyte(a, a->range_low >>  8);
   rblxstb__arith_putbyte(a, a->range_low >>  0);
   rblxstb__arith_flush(a);
   return a->output;
}

rblxstbfile *rblxstb_arith_decode_close(rblxstb_arith *a)
{
   return a->output;
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
//                         Threads
//

#ifndef _WIN32
#ifdef RBLXSTB_THREADS
#error "threads not implemented except for Windows"
#endif
#endif

// call this function to free any global variables for memory testing
RBLXSTB_EXTERN void rblxstb_thread_cleanup(void);

typedef void * (*rblxstb_thread_func)(void *);

// do not rely on these types, this is an implementation detail.
// compare against RBLXSTB_THREAD_NULL and ST_SEMAPHORE_NULL
typedef void *rblxstb_thread;
typedef void *rblxstb_semaphore;
typedef void *rblxstb_mutex;
typedef struct rblxstb__sync *rblxstb_sync;

#define RBLXSTB_SEMAPHORE_NULL    NULL
#define RBLXSTB_THREAD_NULL       NULL
#define RBLXSTB_MUTEX_NULL        NULL
#define RBLXSTB_SYNC_NULL         NULL

// get the number of processors (limited to those in the affinity mask for this process).
RBLXSTB_EXTERN int rblxstb_processor_count(void);
// force to run on a single core -- needed for RDTSC to work, e.g. for iprof
RBLXSTB_EXTERN void rblxstb_force_uniprocessor(void);

// rblxstb_work functions: queue up work to be done by some worker threads

// set number of threads to serve the queue; you can change this on the fly,
// but if you decrease it, it won't decrease until things currently on the
// queue are finished
RBLXSTB_EXTERN void          rblxstb_work_numthreads(int n);
// set maximum number of units in the queue; you can only set this BEFORE running any work functions
RBLXSTB_EXTERN int           rblxstb_work_maxunits(int n);
// enqueue some work to be done (can do this from any thread, or even from a piece of work);
// return value of f is stored in *return_code if non-NULL
RBLXSTB_EXTERN int           rblxstb_work(rblxstb_thread_func f, void *d, volatile void **return_code);
// as above, but rblxstb_sync_reach is called on 'rel' after work is complete
RBLXSTB_EXTERN int           rblxstb_work_reach(rblxstb_thread_func f, void *d, volatile void **return_code, rblxstb_sync rel);


// necessary to call this when using volatile to order writes/reads
RBLXSTB_EXTERN void          rblxstb_barrier(void);

// support for independent queues with their own threads

typedef struct rblxstb__workqueue rblxstb_workqueue;

RBLXSTB_EXTERN rblxstb_workqueue*rblxstb_workq_new(int numthreads, int max_units);
RBLXSTB_EXTERN rblxstb_workqueue*rblxstb_workq_new_flags(int numthreads, int max_units, int no_add_mutex, int no_remove_mutex);
RBLXSTB_EXTERN void          rblxstb_workq_delete(rblxstb_workqueue *q);
RBLXSTB_EXTERN void          rblxstb_workq_numthreads(rblxstb_workqueue *q, int n);
RBLXSTB_EXTERN int           rblxstb_workq(rblxstb_workqueue *q, rblxstb_thread_func f, void *d, volatile void **return_code);
RBLXSTB_EXTERN int           rblxstb_workq_reach(rblxstb_workqueue *q, rblxstb_thread_func f, void *d, volatile void **return_code, rblxstb_sync rel);
RBLXSTB_EXTERN int           rblxstb_workq_length(rblxstb_workqueue *q);

RBLXSTB_EXTERN rblxstb_thread    rblxstb_create_thread (rblxstb_thread_func f, void *d);
RBLXSTB_EXTERN rblxstb_thread    rblxstb_create_thread2(rblxstb_thread_func f, void *d, volatile void **return_code, rblxstb_semaphore rel);
RBLXSTB_EXTERN void          rblxstb_destroy_thread(rblxstb_thread t);

RBLXSTB_EXTERN rblxstb_semaphore rblxstb_sem_new(int max_val);
RBLXSTB_EXTERN rblxstb_semaphore rblxstb_sem_new_extra(int max_val, int start_val);
RBLXSTB_EXTERN void          rblxstb_sem_delete (rblxstb_semaphore s);
RBLXSTB_EXTERN void          rblxstb_sem_waitfor(rblxstb_semaphore s);
RBLXSTB_EXTERN void          rblxstb_sem_release(rblxstb_semaphore s);

RBLXSTB_EXTERN rblxstb_mutex     rblxstb_mutex_new(void);
RBLXSTB_EXTERN void          rblxstb_mutex_delete(rblxstb_mutex m);
RBLXSTB_EXTERN void          rblxstb_mutex_begin(rblxstb_mutex m);
RBLXSTB_EXTERN void          rblxstb_mutex_end(rblxstb_mutex m);

RBLXSTB_EXTERN rblxstb_sync      rblxstb_sync_new(void);
RBLXSTB_EXTERN void          rblxstb_sync_delete(rblxstb_sync s);
RBLXSTB_EXTERN int           rblxstb_sync_set_target(rblxstb_sync s, int count);
RBLXSTB_EXTERN void          rblxstb_sync_reach_and_wait(rblxstb_sync s);    // wait for 'target' reachers
RBLXSTB_EXTERN int           rblxstb_sync_reach(rblxstb_sync s);

typedef struct rblxstb__threadqueue rblxstb_threadqueue;
#define RBLXSTB_THREADQ_DYNAMIC   0
RBLXSTB_EXTERN rblxstb_threadqueue *rblxstb_threadq_new(int item_size, int num_items, int many_add, int many_remove);
RBLXSTB_EXTERN void             rblxstb_threadq_delete(rblxstb_threadqueue *tq);
RBLXSTB_EXTERN int              rblxstb_threadq_get(rblxstb_threadqueue *tq, void *output);
RBLXSTB_EXTERN void             rblxstb_threadq_get_block(rblxstb_threadqueue *tq, void *output);
RBLXSTB_EXTERN int              rblxstb_threadq_add(rblxstb_threadqueue *tq, void *input);
// can return FALSE if RBLXSTB_THREADQ_DYNAMIC and attempt to grow fails
RBLXSTB_EXTERN int              rblxstb_threadq_add_block(rblxstb_threadqueue *tq, void *input);

#ifdef RBLXSTB_THREADS
#ifdef RBLXSTB_DEFINE

typedef struct
{
   rblxstb_thread_func f;
   void *d;
   volatile void **return_val;
   rblxstb_semaphore sem;
} rblxstb__thread;

// this is initialized along all possible paths to create threads, therefore
// it's always initialized before any other threads are create, therefore
// it's free of races AS LONG AS you only create threads through rblxstb_*
static rblxstb_mutex rblxstb__threadmutex, rblxstb__workmutex;

static void rblxstb__threadmutex_init(void)
{
   if (rblxstb__threadmutex == RBLXSTB_SEMAPHORE_NULL) {
      rblxstb__threadmutex = rblxstb_mutex_new();
      rblxstb__workmutex = rblxstb_mutex_new();
   }
}

#ifdef RBLXSTB_THREAD_TEST
volatile float rblxstb__t1=1, rblxstb__t2;

static void rblxstb__wait(int n)
{
   float z = 0;
   int i;
   for (i=0; i < n; ++i)
      z += 1 / (rblxstb__t1+i);
   rblxstb__t2 = z;
}
#else
#define rblxstb__wait(x)
#endif

#ifdef _WIN32

// avoid including windows.h -- note that our definitions aren't
// exactly the same (we don't define the security descriptor struct)
// so if you want to include windows.h, make sure you do it first.
#include <process.h>

#ifndef _WINDOWS_  // check windows.h guard
#define RBLXSTB__IMPORT   RBLXSTB_EXTERN __declspec(dllimport)
#define RBLXSTB__DW       unsigned long

RBLXSTB__IMPORT int     __stdcall TerminateThread(void *, RBLXSTB__DW);
RBLXSTB__IMPORT void *  __stdcall CreateSemaphoreA(void *sec, long,long,char*);
RBLXSTB__IMPORT int     __stdcall CloseHandle(void *);
RBLXSTB__IMPORT RBLXSTB__DW __stdcall WaitForSingleObject(void *, RBLXSTB__DW);
RBLXSTB__IMPORT int     __stdcall ReleaseSemaphore(void *, long, long *);
RBLXSTB__IMPORT void    __stdcall Sleep(RBLXSTB__DW);
#endif

// necessary to call this when using volatile to order writes/reads
void rblxstb_barrier(void)
{
   #ifdef MemoryBarrier
   MemoryBarrier();
   #else
   long temp;
   __asm xchg temp,eax;
   #endif
}

static void rblxstb__thread_run(void *t)
{
   void *res;
   rblxstb__thread info = * (rblxstb__thread *) t;
   free(t);
   res = info.f(info.d);
   if (info.return_val)
      *info.return_val = res;
   if (info.sem != RBLXSTB_SEMAPHORE_NULL)
      rblxstb_sem_release(info.sem);
}

static rblxstb_thread rblxstb_create_thread_raw(rblxstb_thread_func f, void *d, volatile void **return_code, rblxstb_semaphore rel)
{
#ifdef _MT
#if defined(RBLXSTB_FASTMALLOC) && !defined(RBLXSTB_FASTMALLOC_ITS_OKAY_I_ONLY_MALLOC_IN_ONE_THREAD)
   rblxstb_fatal("Error! Cannot use RBLXSTB_FASTMALLOC with threads.\n");
   return RBLXSTB_THREAD_NULL;
#else
   unsigned long id;
   rblxstb__thread *data = (rblxstb__thread *) malloc(sizeof(*data));
   if (!data) return NULL;
   rblxstb__threadmutex_init();
   data->f = f;
   data->d = d;
   data->return_val = return_code;
   data->sem = rel;
   id = _beginthread(rblxstb__thread_run, 0, data);
   if (id == -1) return NULL;
   return (void *) id;
#endif
#else
#ifdef RBLXSTB_NO_STB_STRINGS
   rblxstb_fatal("Invalid compilation");
#else
   rblxstb_fatal("Must compile mult-threaded to use rblxstb_thread/rblxstb_work.");
#endif
   return NULL;
#endif
}

// trivial win32 wrappers
void          rblxstb_destroy_thread(rblxstb_thread t)   { TerminateThread(t,0); }
rblxstb_semaphore rblxstb_sem_new(int maxv)                {return CreateSemaphoreA(NULL,0,maxv,NULL); }
rblxstb_semaphore rblxstb_sem_new_extra(int maxv,int start){return CreateSemaphoreA(NULL,start,maxv,NULL); }
void          rblxstb_sem_delete(rblxstb_semaphore s)    { if (s != NULL) CloseHandle(s); }
void          rblxstb_sem_waitfor(rblxstb_semaphore s)   { WaitForSingleObject(s, 0xffffffff); } // INFINITE
void          rblxstb_sem_release(rblxstb_semaphore s)   { ReleaseSemaphore(s,1,NULL); }
static void   rblxstb__thread_sleep(int ms)          { Sleep(ms); }

#ifndef _WINDOWS_
RBLXSTB__IMPORT int __stdcall GetProcessAffinityMask(void *, RBLXSTB__DW *, RBLXSTB__DW *);
RBLXSTB__IMPORT void * __stdcall GetCurrentProcess(void);
RBLXSTB__IMPORT int __stdcall SetProcessAffinityMask(void *, RBLXSTB__DW);
#endif

int rblxstb_processor_count(void)
{
   unsigned long proc,sys;
   GetProcessAffinityMask(GetCurrentProcess(), &proc, &sys);
   return rblxstb_bitcount(proc);
}

void rblxstb_force_uniprocessor(void)
{
   unsigned long proc,sys;
   GetProcessAffinityMask(GetCurrentProcess(), &proc, &sys);
   if (rblxstb_bitcount(proc) > 1) {
      int z;
      for (z=0; z < 32; ++z)
         if (proc & (1 << z))
            break;
      if (z < 32) {
         proc = 1 << z;
         SetProcessAffinityMask(GetCurrentProcess(), proc);
      }
   }
}

#ifdef _WINDOWS_
#define RBLXSTB_MUTEX_NATIVE
void *rblxstb_mutex_new(void)
{
   CRITICAL_SECTION *p = (CRITICAL_SECTION *) malloc(sizeof(*p));
   if (p)
#if _WIN32_WINNT >= 0x0500
      InitializeCriticalSectionAndSpinCount(p, 500);
#else
      InitializeCriticalSection(p);
#endif
   return p;
}

void rblxstb_mutex_delete(void *p)
{
   if (p) {
      DeleteCriticalSection((CRITICAL_SECTION *) p);
      free(p);
   }
}

void rblxstb_mutex_begin(void *p)
{
   rblxstb__wait(500);
   if (p)
      EnterCriticalSection((CRITICAL_SECTION *) p);
}

void rblxstb_mutex_end(void *p)
{
   if (p)
      LeaveCriticalSection((CRITICAL_SECTION *) p);
   rblxstb__wait(500);
}
#endif // _WINDOWS_

#if 0
// for future reference,
// InterlockedCompareExchange for x86:
 int cas64_mp(void * dest, void * xcmp, void * xxchg) {
        __asm
        {
                mov             esi, [xxchg]            ; exchange
                mov             ebx, [esi + 0]
                mov             ecx, [esi + 4]

                mov             esi, [xcmp]                     ; comparand
                mov             eax, [esi + 0]
                mov             edx, [esi + 4]

                mov             edi, [dest]                     ; destination
                lock cmpxchg8b  [edi]
                jz              yyyy;

                mov             [esi + 0], eax;
                mov             [esi + 4], edx;

yyyy:
                xor             eax, eax;
                setz    al;
        };

inline unsigned __int64 _InterlockedCompareExchange64(volatile unsigned __int64 *dest
                           ,unsigned __int64 exchange
                           ,unsigned __int64 comperand)
{
    //value returned in eax::edx
    __asm {
        lea esi,comperand;
        lea edi,exchange;

        mov eax,[esi];
        mov edx,4[esi];
        mov ebx,[edi];
        mov ecx,4[edi];
        mov esi,dest;
        lock CMPXCHG8B [esi];
    }
#endif // #if 0

#endif // _WIN32

rblxstb_thread rblxstb_create_thread2(rblxstb_thread_func f, void *d, volatile void **return_code, rblxstb_semaphore rel)
{
   return rblxstb_create_thread_raw(f,d,return_code,rel);
}

rblxstb_thread rblxstb_create_thread(rblxstb_thread_func f, void *d)
{
   return rblxstb_create_thread2(f,d,NULL,RBLXSTB_SEMAPHORE_NULL);
}

// mutex implemented by wrapping semaphore
#ifndef RBLXSTB_MUTEX_NATIVE
rblxstb_mutex rblxstb_mutex_new(void)            { return rblxstb_sem_new_extra(1,1); }
void      rblxstb_mutex_delete(rblxstb_mutex m)  { rblxstb_sem_delete (m);      }
void      rblxstb_mutex_begin(rblxstb_mutex m)   { rblxstb__wait(500); if (m) rblxstb_sem_waitfor(m); }
void      rblxstb_mutex_end(rblxstb_mutex m)     { if (m) rblxstb_sem_release(m); rblxstb__wait(500); }
#endif

// thread merge operation
struct rblxstb__sync
{
   int target;  // target number of threads to hit it
   int sofar;   // total threads that hit it
   int waiting; // total threads waiting

   rblxstb_mutex start;   // mutex to prevent starting again before finishing previous
   rblxstb_mutex mutex;   // mutex while tweaking state
   rblxstb_semaphore release; // semaphore wake up waiting threads
      // we have to wake them up one at a time, rather than using a single release
      // call, because win32 semaphores don't let you dynamically change the max count!
};

rblxstb_sync rblxstb_sync_new(void)
{
   rblxstb_sync s = (rblxstb_sync) malloc(sizeof(*s));
   if (!s) return s;

   s->target = s->sofar = s->waiting = 0;
   s->mutex   = rblxstb_mutex_new();
   s->start   = rblxstb_mutex_new();
   s->release = rblxstb_sem_new(1);
   if (s->mutex == RBLXSTB_MUTEX_NULL || s->release == RBLXSTB_SEMAPHORE_NULL || s->start == RBLXSTB_MUTEX_NULL) {
      rblxstb_mutex_delete(s->mutex);
      rblxstb_mutex_delete(s->mutex);
      rblxstb_sem_delete(s->release);
      free(s);
      return NULL;
   }
   return s;
}

void rblxstb_sync_delete(rblxstb_sync s)
{
   if (s->waiting) {
      // it's bad to delete while there are threads waiting!
      // shall we wait for them to reach, or just bail? just bail
      assert(0);
   }
   rblxstb_mutex_delete(s->mutex);
   rblxstb_mutex_delete(s->release);
   free(s);
}

int rblxstb_sync_set_target(rblxstb_sync s, int count)
{
   // don't allow setting a target until the last one is fully released;
   // note that this can lead to inefficient pipelining, and maybe we'd
   // be better off ping-ponging between two internal syncs?
   // I tried seeing how often this happened using TryEnterCriticalSection
   // and could _never_ get it to happen in imv(rblxstb), even with more threads
   // than processors. So who knows!
   rblxstb_mutex_begin(s->start);

   // this mutex is pointless, since it's not valid for threads
   // to call reach() before anyone calls set_target() anyway
   rblxstb_mutex_begin(s->mutex);

   assert(s->target == 0); // enforced by start mutex
   s->target  = count;
   s->sofar   = 0;
   s->waiting = 0;
   rblxstb_mutex_end(s->mutex);
   return RBLXSTB_TRUE;
}

void rblxstb__sync_release(rblxstb_sync s)
{
   if (s->waiting)
      rblxstb_sem_release(s->release);
   else {
      s->target = 0;
      rblxstb_mutex_end(s->start);
   }
}

int rblxstb_sync_reach(rblxstb_sync s)
{
   int n;
   rblxstb_mutex_begin(s->mutex);
   assert(s->sofar < s->target);
   n = ++s->sofar; // record this value to avoid possible race if we did 'return s->sofar';
   if (s->sofar == s->target)
      rblxstb__sync_release(s);
   rblxstb_mutex_end(s->mutex);
   return n;
}

void rblxstb_sync_reach_and_wait(rblxstb_sync s)
{
   rblxstb_mutex_begin(s->mutex);
   assert(s->sofar < s->target);
   ++s->sofar;
   if (s->sofar == s->target) {
      rblxstb__sync_release(s);
      rblxstb_mutex_end(s->mutex);
   } else {
      ++s->waiting; // we're waiting, so one more waiter
      rblxstb_mutex_end(s->mutex); // release the mutex to other threads

      rblxstb_sem_waitfor(s->release); // wait for merge completion

      rblxstb_mutex_begin(s->mutex); // on merge completion, grab the mutex
      --s->waiting; // we're done waiting
      rblxstb__sync_release(s);    // restart the next waiter
      rblxstb_mutex_end(s->mutex); // and now we're done
      // this ends the same as the first case, but it's a lot
      // clearer to understand without sharing the code
   }
}

struct rblxstb__threadqueue
{
   rblxstb_mutex add, remove;
   rblxstb_semaphore nonempty, nonfull;
   int head_blockers;  // number of threads blocking--used to know whether to release(avail)
   int tail_blockers;
   int head, tail, array_size, growable;
   int item_size;
   char *data;
};

static int rblxstb__tq_wrap(volatile rblxstb_threadqueue *z, int p)
{
   if (p == z->array_size)
      return p - z->array_size;
   else
      return p;
}

int rblxstb__threadq_get_raw(rblxstb_threadqueue *tq2, void *output, int block)
{
   volatile rblxstb_threadqueue *tq = (volatile rblxstb_threadqueue *) tq2;
   if (tq->head == tq->tail && !block) return 0;

   rblxstb_mutex_begin(tq->remove);

   while (tq->head == tq->tail) {
      if (!block) {
         rblxstb_mutex_end(tq->remove);
         return 0;
      }
      ++tq->head_blockers;
      rblxstb_mutex_end(tq->remove);

      rblxstb_sem_waitfor(tq->nonempty);

      rblxstb_mutex_begin(tq->remove);
      --tq->head_blockers;
   }

   memcpy(output, tq->data + tq->head*tq->item_size, tq->item_size);
   rblxstb_barrier();
   tq->head = rblxstb__tq_wrap(tq, tq->head+1);

   rblxstb_sem_release(tq->nonfull);
   if (tq->head_blockers) // can't check if actually non-empty due to race?
      rblxstb_sem_release(tq->nonempty); // if there are other blockers, wake one

   rblxstb_mutex_end(tq->remove);
   return RBLXSTB_TRUE;
}

int rblxstb__threadq_grow(volatile rblxstb_threadqueue *tq)
{
   int n;
   char *p;
   assert(tq->remove != RBLXSTB_MUTEX_NULL); // must have this to allow growth!
   rblxstb_mutex_begin(tq->remove);

   n = tq->array_size * 2;
   p = (char *) realloc(tq->data, n * tq->item_size);
   if (p == NULL) {
      rblxstb_mutex_end(tq->remove);
      rblxstb_mutex_end(tq->add);
      return RBLXSTB_FALSE;
   }
   if (tq->tail < tq->head) {
      memcpy(p + tq->array_size * tq->item_size, p, tq->tail * tq->item_size);
      tq->tail += tq->array_size;
   }
   tq->data = p;
   tq->array_size = n;

   rblxstb_mutex_end(tq->remove);
   return RBLXSTB_TRUE;
}

int rblxstb__threadq_add_raw(rblxstb_threadqueue *tq2, void *input, int block)
{
   int tail,pos;
   volatile rblxstb_threadqueue *tq = (volatile rblxstb_threadqueue *) tq2;
   rblxstb_mutex_begin(tq->add);
   for(;;) {
      pos = tq->tail;
      tail = rblxstb__tq_wrap(tq, pos+1);
      if (tail != tq->head) break;

      // full
      if (tq->growable) {
         if (!rblxstb__threadq_grow(tq)) {
            rblxstb_mutex_end(tq->add);
            return RBLXSTB_FALSE; // out of memory
         }
      } else if (!block) {
         rblxstb_mutex_end(tq->add);
         return RBLXSTB_FALSE;
      } else {
         ++tq->tail_blockers;
         rblxstb_mutex_end(tq->add);

         rblxstb_sem_waitfor(tq->nonfull);

         rblxstb_mutex_begin(tq->add);
         --tq->tail_blockers;
      }
   }
   memcpy(tq->data + tq->item_size * pos, input, tq->item_size);
   rblxstb_barrier();
   tq->tail = tail;
   rblxstb_sem_release(tq->nonempty);
   if (tq->tail_blockers) // can't check if actually non-full due to race?
      rblxstb_sem_release(tq->nonfull);
   rblxstb_mutex_end(tq->add);
   return RBLXSTB_TRUE;
}

int rblxstb_threadq_length(rblxstb_threadqueue *tq2)
{
   int a,b,n;
   volatile rblxstb_threadqueue *tq = (volatile rblxstb_threadqueue *) tq2;
   rblxstb_mutex_begin(tq->add);
   a = tq->head;
   b = tq->tail;
   n = tq->array_size;
   rblxstb_mutex_end(tq->add);
   if (a > b) b += n;
   return b-a;
}

int rblxstb_threadq_get(rblxstb_threadqueue *tq, void *output)
{
   return rblxstb__threadq_get_raw(tq, output, RBLXSTB_FALSE);
}

void rblxstb_threadq_get_block(rblxstb_threadqueue *tq, void *output)
{
   rblxstb__threadq_get_raw(tq, output, RBLXSTB_TRUE);
}

int rblxstb_threadq_add(rblxstb_threadqueue *tq, void *input)
{
   return rblxstb__threadq_add_raw(tq, input, RBLXSTB_FALSE);
}

int rblxstb_threadq_add_block(rblxstb_threadqueue *tq, void *input)
{
   return rblxstb__threadq_add_raw(tq, input, RBLXSTB_TRUE);
}

void rblxstb_threadq_delete(rblxstb_threadqueue *tq)
{
   if (tq) {
      free(tq->data);
      rblxstb_mutex_delete(tq->add);
      rblxstb_mutex_delete(tq->remove);
      rblxstb_sem_delete(tq->nonempty);
      rblxstb_sem_delete(tq->nonfull);
      free(tq);
   }
}

#define RBLXSTB_THREADQUEUE_DYNAMIC   0
rblxstb_threadqueue *rblxstb_threadq_new(int item_size, int num_items, int many_add, int many_remove)
{
   int error=0;
   rblxstb_threadqueue *tq = (rblxstb_threadqueue *) malloc(sizeof(*tq));
   if (tq == NULL) return NULL;

   if (num_items == RBLXSTB_THREADQUEUE_DYNAMIC) {
      tq->growable = RBLXSTB_TRUE;
      num_items = 32;
   } else
      tq->growable = RBLXSTB_FALSE;

   tq->item_size = item_size;
   tq->array_size = num_items+1;

   tq->add = tq->remove = RBLXSTB_MUTEX_NULL;
   tq->nonempty = tq->nonfull = RBLXSTB_SEMAPHORE_NULL;
   tq->data = NULL;
   if (many_add)
      { tq->add    = rblxstb_mutex_new(); if (tq->add    == RBLXSTB_MUTEX_NULL) goto error; }
   if (many_remove || tq->growable)
      { tq->remove = rblxstb_mutex_new(); if (tq->remove == RBLXSTB_MUTEX_NULL) goto error; }
   tq->nonempty = rblxstb_sem_new(1); if (tq->nonempty == RBLXSTB_SEMAPHORE_NULL) goto error;
   tq->nonfull  = rblxstb_sem_new(1); if (tq->nonfull  == RBLXSTB_SEMAPHORE_NULL) goto error;
   tq->data = (char *) malloc(tq->item_size * tq->array_size);
   if (tq->data == NULL) goto error;

   tq->head = tq->tail = 0;
   tq->head_blockers = tq->tail_blockers = 0;

   return tq;

error:
   rblxstb_threadq_delete(tq);
   return NULL;
}

typedef struct
{
   rblxstb_thread_func f;
   void *d;
   volatile void **retval;
   rblxstb_sync sync;
} rblxstb__workinfo;

//static volatile rblxstb__workinfo *rblxstb__work;

struct rblxstb__workqueue
{
   int numthreads;
   rblxstb_threadqueue *tq;
};

static rblxstb_workqueue *rblxstb__work_global;

static void *rblxstb__thread_workloop(void *p)
{
   volatile rblxstb_workqueue *q = (volatile rblxstb_workqueue *) p;
   for(;;) {
      void *z;
      rblxstb__workinfo w;
      rblxstb_threadq_get_block(q->tq, &w);
      if (w.f == NULL) // null work is a signal to end the thread
         return NULL;
      z = w.f(w.d);
      if (w.retval) { rblxstb_barrier(); *w.retval = z; }
      if (w.sync != RBLXSTB_SYNC_NULL) rblxstb_sync_reach(w.sync);
   }
}

rblxstb_workqueue *rblxstb_workq_new(int num_threads, int max_units)
{
   return rblxstb_workq_new_flags(num_threads, max_units, 0,0);
}

rblxstb_workqueue *rblxstb_workq_new_flags(int numthreads, int max_units, int no_add_mutex, int no_remove_mutex)
{
   rblxstb_workqueue *q = (rblxstb_workqueue *) malloc(sizeof(*q));
   if (q == NULL) return NULL;
   q->tq = rblxstb_threadq_new(sizeof(rblxstb__workinfo), max_units, !no_add_mutex, !no_remove_mutex);
   if (q->tq == NULL) { free(q); return NULL; }
   q->numthreads = 0;
   rblxstb_workq_numthreads(q, numthreads);
   return q;
}

void rblxstb_workq_delete(rblxstb_workqueue *q)
{
   while (rblxstb_workq_length(q) != 0)
      rblxstb__thread_sleep(1);
   rblxstb_threadq_delete(q->tq);
   free(q);
}

static int rblxstb__work_maxitems = RBLXSTB_THREADQUEUE_DYNAMIC;

static void rblxstb_work_init(int num_threads)
{
   if (rblxstb__work_global == NULL) {
      rblxstb__threadmutex_init();
      rblxstb_mutex_begin(rblxstb__workmutex);
      rblxstb_barrier();
      if (*(rblxstb_workqueue * volatile *) &rblxstb__work_global == NULL)
         rblxstb__work_global = rblxstb_workq_new(num_threads, rblxstb__work_maxitems);
      rblxstb_mutex_end(rblxstb__workmutex);
   }
}

static int rblxstb__work_raw(rblxstb_workqueue *q, rblxstb_thread_func f, void *d, volatile void **return_code, rblxstb_sync rel)
{
   rblxstb__workinfo w;
   if (q == NULL) {
      rblxstb_work_init(1);
      q = rblxstb__work_global;
   }
   w.f = f;
   w.d = d;
   w.retval = return_code;
   w.sync = rel;
   return rblxstb_threadq_add(q->tq, &w);
}

int rblxstb_workq_length(rblxstb_workqueue *q)
{
   return rblxstb_threadq_length(q->tq);
}

int rblxstb_workq(rblxstb_workqueue *q, rblxstb_thread_func f, void *d, volatile void **return_code)
{
   if (f == NULL) return 0;
   return rblxstb_workq_reach(q, f, d, return_code, NULL);
}

int rblxstb_workq_reach(rblxstb_workqueue *q, rblxstb_thread_func f, void *d, volatile void **return_code, rblxstb_sync rel)
{
   if (f == NULL) return 0;
   return rblxstb__work_raw(q, f, d, return_code, rel);
}

static void rblxstb__workq_numthreads(rblxstb_workqueue *q, int n)
{
   while (q->numthreads < n) {
      rblxstb_create_thread(rblxstb__thread_workloop, q);
      ++q->numthreads;
   }
   while (q->numthreads > n) {
      rblxstb__work_raw(q, NULL, NULL, NULL, NULL);
      --q->numthreads;
   }
}

void rblxstb_workq_numthreads(rblxstb_workqueue *q, int n)
{
   rblxstb_mutex_begin(rblxstb__threadmutex);
   rblxstb__workq_numthreads(q,n);
   rblxstb_mutex_end(rblxstb__threadmutex);
}

int rblxstb_work_maxunits(int n)
{
   if (rblxstb__work_global == NULL) {
      rblxstb__work_maxitems = n;
      rblxstb_work_init(1);
   }
   return rblxstb__work_maxitems;
}

int rblxstb_work(rblxstb_thread_func f, void *d, volatile void **return_code)
{
   return rblxstb_workq(rblxstb__work_global, f,d,return_code);
}

int rblxstb_work_reach(rblxstb_thread_func f, void *d, volatile void **return_code, rblxstb_sync rel)
{
   return rblxstb_workq_reach(rblxstb__work_global, f,d,return_code,rel);
}

void rblxstb_work_numthreads(int n)
{
   if (rblxstb__work_global == NULL)
      rblxstb_work_init(n);
   else
      rblxstb_workq_numthreads(rblxstb__work_global, n);
}
#endif // RBLXSTB_DEFINE


//////////////////////////////////////////////////////////////////////////////
//
// Background disk I/O
//
//

#define RBLXSTB_BGIO_READ_ALL   (-1)
RBLXSTB_EXTERN int rblxstb_bgio_read    (char *filename, int offset, int len, rblxstb_uchar **result, int *olen);
RBLXSTB_EXTERN int rblxstb_bgio_readf   (FILE *f       , int offset, int len, rblxstb_uchar **result, int *olen);
RBLXSTB_EXTERN int rblxstb_bgio_read_to (char *filename, int offset, int len, rblxstb_uchar  *buffer, int *olen);
RBLXSTB_EXTERN int rblxstb_bgio_readf_to(FILE *f       , int offset, int len, rblxstb_uchar  *buffer, int *olen);

typedef struct
{
   int have_data;
   int is_valid;
   int is_dir;
   time_t filetime;
   rblxstb_int64 filesize;
} rblxstb_bgstat;

RBLXSTB_EXTERN int rblxstb_bgio_stat    (char *filename, rblxstb_bgstat *result);

#ifdef RBLXSTB_DEFINE

static rblxstb_workqueue *rblxstb__diskio;
static rblxstb_mutex rblxstb__diskio_mutex;

void rblxstb_thread_cleanup(void)
{
   if (rblxstb__work_global) rblxstb_workq_delete(rblxstb__work_global); rblxstb__work_global = NULL;
   if (rblxstb__threadmutex) rblxstb_mutex_delete(rblxstb__threadmutex); rblxstb__threadmutex = NULL;
   if (rblxstb__workmutex)   rblxstb_mutex_delete(rblxstb__workmutex);   rblxstb__workmutex   = NULL;
   if (rblxstb__diskio)      rblxstb_workq_delete(rblxstb__diskio);      rblxstb__diskio      = NULL;
   if (rblxstb__diskio_mutex)rblxstb_mutex_delete(rblxstb__diskio_mutex);rblxstb__diskio_mutex= NULL;
}


typedef struct
{
   char *filename;
   FILE *f;
   int offset;
   int len;

   rblxstb_bgstat *stat_out;
   rblxstb_uchar *output;
   rblxstb_uchar **result;
   int *len_output;
   int *flag;
} rblxstb__disk_command;

#define RBLXSTB__MAX_DISK_COMMAND 100
static rblxstb__disk_command rblxstb__dc_queue[RBLXSTB__MAX_DISK_COMMAND];
static int rblxstb__dc_offset;

void rblxstb__io_init(void)
{
   if (!rblxstb__diskio) {
      rblxstb__threadmutex_init();
      rblxstb_mutex_begin(rblxstb__threadmutex);
      rblxstb_barrier();
      if (*(rblxstb_thread * volatile *) &rblxstb__diskio == NULL) {
         rblxstb__diskio_mutex = rblxstb_mutex_new();
         // use many threads so OS can try to schedule seeks
         rblxstb__diskio = rblxstb_workq_new_flags(16,RBLXSTB__MAX_DISK_COMMAND,RBLXSTB_FALSE,RBLXSTB_FALSE);
      }
      rblxstb_mutex_end(rblxstb__threadmutex);
   }
}

static void * rblxstb__io_error(rblxstb__disk_command *dc)
{
   if (dc->len_output) *dc->len_output = 0;
   if (dc->result) *dc->result = NULL;
   if (dc->flag) *dc->flag = -1;
   return NULL;
}

static void * rblxstb__io_task(void *p)
{
   rblxstb__disk_command *dc = (rblxstb__disk_command *) p;
   int len;
   FILE *f;
   rblxstb_uchar *buf;

   if (dc->stat_out) {
      struct _stati64 s;
      if (!_stati64(dc->filename, &s)) {
         dc->stat_out->filesize = s.st_size;
         dc->stat_out->filetime = s.st_mtime;
         dc->stat_out->is_dir = s.st_mode & _S_IFDIR;
         dc->stat_out->is_valid = (s.st_mode & _S_IFREG) || dc->stat_out->is_dir;
      } else
         dc->stat_out->is_valid = 0;
      rblxstb_barrier();
      dc->stat_out->have_data = 1;
      free(dc->filename);
      return 0;
   }
   if (dc->f) {
      #ifdef WIN32
      f = _fdopen(_dup(_fileno(dc->f)), "rb");
      #else
      f = fdopen(dup(fileno(dc->f)), "rb");
      #endif
      if (!f)
         return rblxstb__io_error(dc);
   } else {
      f = fopen(dc->filename, "rb");
      free(dc->filename);
      if (!f)
         return rblxstb__io_error(dc);
   }

   len = dc->len;
   if (len < 0) {
      fseek(f, 0, SEEK_END);
      len = ftell(f) - dc->offset;
   }

   if (fseek(f, dc->offset, SEEK_SET)) {
      fclose(f);
      return rblxstb__io_error(dc);
   }

   if (dc->output)
      buf = dc->output;
   else {
      buf = (rblxstb_uchar *) malloc(len);
      if (buf == NULL) {
         fclose(f);
         return rblxstb__io_error(dc);
      }
   }

   len = fread(buf, 1, len, f);
   fclose(f);
   if (dc->len_output) *dc->len_output = len;
   if (dc->result) *dc->result = buf;
   if (dc->flag) *dc->flag = 1;

   return NULL;
}

int rblxstb__io_add(char *fname, FILE *f, int off, int len, rblxstb_uchar *out, rblxstb_uchar **result, int *olen, int *flag, rblxstb_bgstat *stat)
{
   int res;
   rblxstb__io_init();
   // do memory allocation outside of mutex
   if (fname) fname = rblxstb_p_strdup(fname);
   rblxstb_mutex_begin(rblxstb__diskio_mutex);
   {
      rblxstb__disk_command *dc = &rblxstb__dc_queue[rblxstb__dc_offset];
      dc->filename = fname;
      dc->f = f;
      dc->offset = off;
      dc->len = len;
      dc->output = out;
      dc->result = result;
      dc->len_output = olen;
      dc->flag = flag;
      dc->stat_out = stat;
      res = rblxstb_workq(rblxstb__diskio, rblxstb__io_task, dc, NULL);
      if (res)
         rblxstb__dc_offset = (rblxstb__dc_offset + 1 == RBLXSTB__MAX_DISK_COMMAND ? 0 : rblxstb__dc_offset+1);
   }
   rblxstb_mutex_end(rblxstb__diskio_mutex);
   return res;
}

int rblxstb_bgio_read(char *filename, int offset, int len, rblxstb_uchar **result, int *olen)
{
   return rblxstb__io_add(filename,NULL,offset,len,NULL,result,olen,NULL,NULL);
}

int rblxstb_bgio_readf(FILE *f, int offset, int len, rblxstb_uchar **result, int *olen)
{
   return rblxstb__io_add(NULL,f,offset,len,NULL,result,olen,NULL,NULL);
}

int rblxstb_bgio_read_to(char *filename, int offset, int len, rblxstb_uchar *buffer, int *olen)
{
   return rblxstb__io_add(filename,NULL,offset,len,buffer,NULL,olen,NULL,NULL);
}

int rblxstb_bgio_readf_to(FILE *f, int offset, int len, rblxstb_uchar *buffer, int *olen)
{
   return rblxstb__io_add(NULL,f,offset,len,buffer,NULL,olen,NULL,NULL);
}

RBLXSTB_EXTERN int rblxstb_bgio_stat    (char *filename, rblxstb_bgstat *result)
{
   result->have_data = 0;
   return rblxstb__io_add(filename,NULL,0,0,0,NULL,0,NULL, result);
}
#endif
#endif



//////////////////////////////////////////////////////////////////////////////
//
//                         Fast malloc implementation
//
//   This is a clone of TCMalloc, but without the thread support.
//      1. large objects are allocated directly, page-aligned
//      2. small objects are allocated in homogeonous heaps, 0 overhead
//
//   We keep an allocation table for pages a la TCMalloc. This would
//   require 4MB for the entire address space, but we only allocate
//   the parts that are in use. The overhead from using homogenous heaps
//   everywhere is 3MB. (That is, if you allocate 1 object of each size,
//   you'll use 3MB.)

#if defined(RBLXSTB_DEFINE) && ((defined(_WIN32) && !defined(_M_AMD64)) || defined(RBLXSTB_FASTMALLOC))

#ifdef _WIN32
   #ifndef _WINDOWS_
   #ifndef RBLXSTB__IMPORT
   #define RBLXSTB__IMPORT   RBLXSTB_EXTERN __declspec(dllimport)
   #define RBLXSTB__DW       unsigned long
   #endif
   RBLXSTB__IMPORT void * __stdcall VirtualAlloc(void *p, unsigned long size, unsigned long type, unsigned long protect);
   RBLXSTB__IMPORT int   __stdcall VirtualFree(void *p, unsigned long size, unsigned long freetype);
   #endif
   #define rblxstb__alloc_pages_raw(x)     (rblxstb_uint32) VirtualAlloc(NULL, (x), 0x3000, 0x04)
   #define rblxstb__dealloc_pages_raw(p)   VirtualFree((void *) p, 0, 0x8000)
#else
   #error "Platform not currently supported"
#endif

typedef struct rblxstb__span
{
   int                start, len;
   struct rblxstb__span  *next, *prev;
   void              *first_free;
   unsigned short     list; // 1..256 free; 257..511 sizeclass; 0=large block
   short              allocations; // # outstanding allocations for sizeclass
} rblxstb__span;  // 24

static rblxstb__span **rblxstb__span_for_page;
static int rblxstb__firstpage, rblxstb__lastpage;
static void rblxstb__update_page_range(int first, int last)
{
   rblxstb__span **sfp;
   int i, f,l;
   if (first >= rblxstb__firstpage && last <= rblxstb__lastpage) return;
   if (rblxstb__span_for_page == NULL) {
      f = first;
      l = f+rblxstb_max(last-f, 16384);
      l = rblxstb_min(l, 1<<20);
   } else if (last > rblxstb__lastpage) {
      f = rblxstb__firstpage;
      l = f + (rblxstb__lastpage - f) * 2;
      l = rblxstb_clamp(last, l,1<<20);
   } else {
      l = rblxstb__lastpage;
      f = l - (l - rblxstb__firstpage) * 2;
      f = rblxstb_clamp(f, 0,first);
   }
   sfp = (rblxstb__span **) rblxstb__alloc_pages_raw(sizeof(void *) * (l-f));
   for (i=f; i < rblxstb__firstpage; ++i) sfp[i - f] = NULL;
   for (   ; i < rblxstb__lastpage ; ++i) sfp[i - f] = rblxstb__span_for_page[i - rblxstb__firstpage];
   for (   ; i < l             ; ++i) sfp[i - f] = NULL;
   if (rblxstb__span_for_page) rblxstb__dealloc_pages_raw(rblxstb__span_for_page);
   rblxstb__firstpage = f;
   rblxstb__lastpage  = l;
   rblxstb__span_for_page = sfp;
}

static rblxstb__span *rblxstb__span_free=NULL;
static rblxstb__span *rblxstb__span_first, *rblxstb__span_end;
static rblxstb__span *rblxstb__span_alloc(void)
{
   rblxstb__span *s = rblxstb__span_free;
   if (s)
      rblxstb__span_free = s->next;
   else {
      if (!rblxstb__span_first) {
         rblxstb__span_first = (rblxstb__span *) rblxstb__alloc_pages_raw(65536);
         if (rblxstb__span_first == NULL) return NULL;
         rblxstb__span_end = rblxstb__span_first + (65536 / sizeof(rblxstb__span));
      }
      s = rblxstb__span_first++;
      if (rblxstb__span_first == rblxstb__span_end) rblxstb__span_first = NULL;
   }
   return s;
}

static rblxstb__span *rblxstb__spanlist[512];

static void rblxstb__spanlist_unlink(rblxstb__span *s)
{
   if (s->prev)
      s->prev->next = s->next;
   else {
      int n = s->list;
      assert(rblxstb__spanlist[n] == s);
      rblxstb__spanlist[n] = s->next;
   }
   if (s->next)
      s->next->prev = s->prev;
   s->next = s->prev = NULL;
   s->list = 0;
}

static void rblxstb__spanlist_add(int n, rblxstb__span *s)
{
   s->list = n;
   s->next = rblxstb__spanlist[n];
   s->prev = NULL;
   rblxstb__spanlist[n] = s;
   if (s->next) s->next->prev = s;
}

#define rblxstb__page_shift       12
#define rblxstb__page_size        (1 << rblxstb__page_shift)
#define rblxstb__page_number(x)   ((x) >> rblxstb__page_shift)
#define rblxstb__page_address(x)  ((x) << rblxstb__page_shift)

static void rblxstb__set_span_for_page(rblxstb__span *s)
{
   int i;
   for (i=0; i < s->len; ++i)
      rblxstb__span_for_page[s->start + i - rblxstb__firstpage] = s;
}

static rblxstb__span *rblxstb__coalesce(rblxstb__span *a, rblxstb__span *b)
{
   assert(a->start + a->len == b->start);
   if (a->list) rblxstb__spanlist_unlink(a);
   if (b->list) rblxstb__spanlist_unlink(b);
   a->len += b->len;
   b->len = 0;
   b->next = rblxstb__span_free;
   rblxstb__span_free = b;
   rblxstb__set_span_for_page(a);
   return a;
}

static void rblxstb__free_span(rblxstb__span *s)
{
   rblxstb__span *n = NULL;
   if (s->start > rblxstb__firstpage) {
      n = rblxstb__span_for_page[s->start-1 - rblxstb__firstpage];
      if (n && n->allocations == -2 && n->start + n->len == s->start) s = rblxstb__coalesce(n,s);
   }
   if (s->start + s->len < rblxstb__lastpage) {
      n = rblxstb__span_for_page[s->start + s->len - rblxstb__firstpage];
      if (n && n->allocations == -2 && s->start + s->len == n->start) s = rblxstb__coalesce(s,n);
   }
   s->allocations = -2;
   rblxstb__spanlist_add(s->len > 256 ? 256 : s->len, s);
}

static rblxstb__span *rblxstb__alloc_pages(int num)
{
   rblxstb__span *s = rblxstb__span_alloc();
   int p;
   if (!s) return NULL;
   p = rblxstb__alloc_pages_raw(num << rblxstb__page_shift);
   if (p == 0) { s->next = rblxstb__span_free; rblxstb__span_free = s; return 0; }
   assert(rblxstb__page_address(rblxstb__page_number(p)) == p);
   p = rblxstb__page_number(p);
   rblxstb__update_page_range(p, p+num);
   s->start = p;
   s->len   = num;
   s->next  = NULL;
   s->prev  = NULL;
   rblxstb__set_span_for_page(s);
   return s;
}

static rblxstb__span *rblxstb__alloc_span(int pagecount)
{
   int i;
   rblxstb__span *p = NULL;
   for(i=pagecount; i < 256; ++i)
      if (rblxstb__spanlist[i]) {
         p = rblxstb__spanlist[i];
         break;
      }
   if (!p) {
      p = rblxstb__spanlist[256];
      while (p && p->len < pagecount)
         p = p->next;
   }
   if (!p) {
      p = rblxstb__alloc_pages(pagecount < 16 ? 16 : pagecount);
      if (p == NULL) return 0;
   } else
      rblxstb__spanlist_unlink(p);

   if (p->len > pagecount) {
      rblxstb__span *q = rblxstb__span_alloc();
      if (q) {
         q->start = p->start + pagecount;
         q->len   = p->len   - pagecount;
         p->len   = pagecount;
         for (i=0; i < q->len; ++i)
            rblxstb__span_for_page[q->start+i - rblxstb__firstpage] = q;
         rblxstb__spanlist_add(q->len > 256 ? 256 : q->len, q);
      }
   }
   return p;
}

#define RBLXSTB__MAX_SMALL_SIZE     32768
#define RBLXSTB__MAX_SIZE_CLASSES   256

static unsigned char rblxstb__class_base[32];
static unsigned char rblxstb__class_shift[32];
static unsigned char rblxstb__pages_for_class[RBLXSTB__MAX_SIZE_CLASSES];
static           int rblxstb__size_for_class[RBLXSTB__MAX_SIZE_CLASSES];

rblxstb__span *rblxstb__get_nonempty_sizeclass(int c)
{
   int s = c + 256, i, size, tsize; // remap to span-list index
   char *z;
   void *q;
   rblxstb__span *p = rblxstb__spanlist[s];
   if (p) {
      if (p->first_free) return p; // fast path: it's in the first one in list
      for (p=p->next; p; p=p->next)
         if (p->first_free) {
            // move to front for future queries
            rblxstb__spanlist_unlink(p);
            rblxstb__spanlist_add(s, p);
            return p;
         }
   }
   // no non-empty ones, so allocate a new one
   p = rblxstb__alloc_span(rblxstb__pages_for_class[c]);
   if (!p) return NULL;
   // create the free list up front
   size = rblxstb__size_for_class[c];
   tsize = rblxstb__pages_for_class[c] << rblxstb__page_shift;
   i = 0;
   z = (char *) rblxstb__page_address(p->start);
   q = NULL;
   while (i + size <= tsize) {
      * (void **) z = q; q = z;
      z += size;
      i += size;
   }
   p->first_free = q;
   p->allocations = 0;
   rblxstb__spanlist_add(s,p);
   return p;
}

static int rblxstb__sizeclass(size_t sz)
{
   int z = rblxstb_log2_floor(sz); // -1 below to group e.g. 13,14,15,16 correctly
   return rblxstb__class_base[z] + ((sz-1) >> rblxstb__class_shift[z]);
}

static void rblxstb__init_sizeclass(void)
{
   int i, size, overhead;
   int align_shift = 2;  // allow 4-byte and 12-byte blocks as well, vs. TCMalloc
   int next_class = 1;
   int last_log = 0;

   for (i = 0; i < align_shift; i++) {
      rblxstb__class_base [i] = next_class;
      rblxstb__class_shift[i] = align_shift;
   }

   for (size = 1 << align_shift; size <= RBLXSTB__MAX_SMALL_SIZE; size += 1 << align_shift) {
      i = rblxstb_log2_floor(size);
      if (i > last_log) {
         if (size == 16) ++align_shift; // switch from 4-byte to 8-byte alignment
         else if (size >= 128 && align_shift < 8) ++align_shift;
         rblxstb__class_base[i]  = next_class - ((size-1) >> align_shift);
         rblxstb__class_shift[i] = align_shift;
         last_log = i;
      }
      rblxstb__size_for_class[next_class++] = size;
   }

   for (i=1; i <= RBLXSTB__MAX_SMALL_SIZE; ++i)
      assert(i <= rblxstb__size_for_class[rblxstb__sizeclass(i)]);

   overhead = 0;
   for (i = 1; i < next_class; i++) {
      int s = rblxstb__size_for_class[i];
      size = rblxstb__page_size;
      while (size % s > size >> 3)
         size += rblxstb__page_size;
      rblxstb__pages_for_class[i] = (unsigned char) (size >> rblxstb__page_shift);
      overhead += size;
   }
   assert(overhead < (4 << 20)); // make sure it's under 4MB of overhead
}

#ifdef RBLXSTB_DEBUG
#define rblxstb__smemset(a,b,c)  memset((void *) a, b, c)
#elif defined(RBLXSTB_FASTMALLOC_INIT)
#define rblxstb__smemset(a,b,c)  memset((void *) a, b, c)
#else
#define rblxstb__smemset(a,b,c)
#endif
void *rblxstb_smalloc(size_t sz)
{
   rblxstb__span *s;
   if (sz == 0) return NULL;
   if (rblxstb__size_for_class[1] == 0) rblxstb__init_sizeclass();
   if (sz > RBLXSTB__MAX_SMALL_SIZE) {
      s = rblxstb__alloc_span((sz + rblxstb__page_size - 1) >> rblxstb__page_shift);
      if (s == NULL) return NULL;
      s->list = 0;
      s->next = s->prev = NULL;
      s->allocations = -32767;
      rblxstb__smemset(rblxstb__page_address(s->start), 0xcd, (sz+3)&~3);
      return (void *) rblxstb__page_address(s->start);
   } else {
      void *p;
      int c = rblxstb__sizeclass(sz);
      s = rblxstb__spanlist[256+c];
      if (!s || !s->first_free)
         s = rblxstb__get_nonempty_sizeclass(c);
      if (s == NULL) return NULL;
      p = s->first_free;
      s->first_free = * (void **) p;
      ++s->allocations;
      rblxstb__smemset(p,0xcd, sz);
      return p;
   }
}

int rblxstb_ssize(void *p)
{
   rblxstb__span *s;
   if (p == NULL) return 0;
   s = rblxstb__span_for_page[rblxstb__page_number((rblxstb_uint) p) - rblxstb__firstpage];
   if (s->list >= 256) {
      return rblxstb__size_for_class[s->list - 256];
   } else {
      assert(s->list == 0);
      return s->len << rblxstb__page_shift;
   }
}

void rblxstb_sfree(void *p)
{
   rblxstb__span *s;
   if (p == NULL) return;
   s = rblxstb__span_for_page[rblxstb__page_number((rblxstb_uint) p) - rblxstb__firstpage];
   if (s->list >= 256) {
      rblxstb__smemset(p, 0xfe, rblxstb__size_for_class[s->list-256]);
      * (void **) p = s->first_free;
      s->first_free = p;
      if (--s->allocations == 0) {
         rblxstb__spanlist_unlink(s);
         rblxstb__free_span(s);
      }
   } else {
      assert(s->list == 0);
      rblxstb__smemset(p, 0xfe, rblxstb_ssize(p));
      rblxstb__free_span(s);
   }
}

void *rblxstb_srealloc(void *p, size_t sz)
{
   size_t cur_size;
   if (p == NULL) return rblxstb_smalloc(sz);
   if (sz == 0) { rblxstb_sfree(p); return NULL; }
   cur_size = rblxstb_ssize(p);
   if (sz > cur_size || sz <= (cur_size >> 1)) {
      void *q;
      if (sz > cur_size && sz < (cur_size << 1)) sz = cur_size << 1;
      q = rblxstb_smalloc(sz); if (q == NULL) return NULL;
      memcpy(q, p, sz < cur_size ? sz : cur_size);
      rblxstb_sfree(p);
      return q;
   }
   return p;
}

void *rblxstb_scalloc(size_t n, size_t sz)
{
   void *p;
   if (n == 0 || sz == 0) return NULL;
   if (rblxstb_log2_ceil(n) + rblxstb_log2_ceil(n) >= 32) return NULL;
   p = rblxstb_smalloc(n*sz);
   if (p) memset(p, 0, n*sz);
   return p;
}

char *rblxstb_sstrdup(char *s)
{
   int n = strlen(s);
   char *p = (char *) rblxstb_smalloc(n+1);
   if (p) rblxstb_p_strcpy_s(p,n+1,s);
   return p;
}
#endif // RBLXSTB_DEFINE



//////////////////////////////////////////////////////////////////////////////
//
//                         Source code constants
//
// This is a trivial system to let you specify constants in source code,
// then while running you can change the constants.
//
// Note that you can't wrap the #defines, because we need to know their
// names. So we provide a pre-wrapped version without 'RBLXSTB_' for convenience;
// to request it, #define RBLXSTB_CONVENIENT_H, yielding:
//       KI -- integer
//       KU -- unsigned integer
//       KF -- float
//       KD -- double
//       KS -- string constant
//
// Defaults to functioning in debug build, not in release builds.
// To force on, define RBLXSTB_ALWAYS_H

#ifdef RBLXSTB_CONVENIENT_H
#define KI(x) RBLXSTB_I(x)
#define KU(x) RBLXSTB_UI(x)
#define KF(x) RBLXSTB_F(x)
#define KD(x) RBLXSTB_D(x)
#define KS(x) RBLXSTB_S(x)
#endif

RBLXSTB_EXTERN void rblxstb_source_path(char *str);
#ifdef RBLXSTB_DEFINE
char *rblxstb__source_path;
void rblxstb_source_path(char *path)
{
   rblxstb__source_path = path;
}

char *rblxstb__get_sourcefile_path(char *file)
{
   static char filebuf[512];
   if (rblxstb__source_path) {
      rblxstb_p_sprintf(filebuf rblxstb_p_size(sizeof(filebuf)), "%s/%s", rblxstb__source_path, file);
      if (rblxstb_fexists(filebuf)) return filebuf;
   }

   if (rblxstb_fexists(file)) return file;

   rblxstb_p_sprintf(filebuf rblxstb_p_size(sizeof(filebuf)), "../%s", file);
   if (!rblxstb_fexists(filebuf)) return filebuf;

   return file;
}
#endif

#define RBLXSTB_F(x)   ((float) RBLXSTB_H(x))
#define RBLXSTB_UI(x)  ((unsigned int) RBLXSTB_I(x))

#if !defined(RBLXSTB_DEBUG) && !defined(RBLXSTB_ALWAYS_H)
#define RBLXSTB_D(x)   ((double) (x))
#define RBLXSTB_I(x)   ((int) (x))
#define RBLXSTB_S(x)   ((char *) (x))
#else
#define RBLXSTB_D(x)   rblxstb__double_constant(__FILE__, __LINE__-1, (x))
#define RBLXSTB_I(x)   rblxstb__int_constant(__FILE__, __LINE__-1, (x))
#define RBLXSTB_S(x)   rblxstb__string_constant(__FILE__, __LINE__-1, (x))

RBLXSTB_EXTERN double rblxstb__double_constant(char *file, int line, double x);
RBLXSTB_EXTERN int    rblxstb__int_constant(char *file, int line, int x);
RBLXSTB_EXTERN char * rblxstb__string_constant(char *file, int line, char *str);

#ifdef RBLXSTB_DEFINE

enum
{
   RBLXSTB__CTYPE_int,
   RBLXSTB__CTYPE_uint,
   RBLXSTB__CTYPE_float,
   RBLXSTB__CTYPE_double,
   RBLXSTB__CTYPE_string,
};

typedef struct
{
   int line;
   int type;
   union {
      int ival;
      double dval;
      char *sval;
   };
} rblxstb__Entry;

typedef struct
{
   rblxstb__Entry *entries;
   char *filename;
   time_t timestamp;
   char **file_data;
   int file_len;
   unsigned short *line_index;
} rblxstb__FileEntry;

static void rblxstb__constant_parse(rblxstb__FileEntry *f, int i)
{
   char *s;
   int n;
   if (!rblxstb_arr_valid(f->entries, i)) return;
   n = f->entries[i].line;
   if (n >= f->file_len) return;
   s = f->file_data[n];
   switch (f->entries[i].type) {
      case RBLXSTB__CTYPE_float:
         while (*s) {
            if (!strncmp(s, "RBLXSTB_D(", 6)) { s+=6; goto matched_float; }
            if (!strncmp(s, "RBLXSTB_F(", 6)) { s+=6; goto matched_float; }
            if (!strncmp(s, "KD(", 3)) { s+=3; goto matched_float; }
            if (!strncmp(s, "KF(", 3)) { s+=3; goto matched_float; }
            ++s;
         }
         break;
      matched_float:
         f->entries[i].dval = strtod(s, NULL);
         break;
      case RBLXSTB__CTYPE_int:
         while (*s) {
            if (!strncmp(s, "RBLXSTB_I(", 6)) { s+=6; goto matched_int; }
            if (!strncmp(s, "RBLXSTB_UI(", 7)) { s+=7; goto matched_int; }
            if (!strncmp(s, "KI(", 3)) { s+=3; goto matched_int; }
            if (!strncmp(s, "KU(", 3)) { s+=3; goto matched_int; }
            ++s;
         }
         break;
      matched_int: {
         int neg=0;
         s = rblxstb_skipwhite(s);
         while (*s == '-') { neg = !neg; s = rblxstb_skipwhite(s+1); } // handle '- - 5', pointlessly
         if (s[0] == '0' && tolower(s[1]) == 'x')
            f->entries[i].ival = strtol(s, NULL, 16);
         else if (s[0] == '0')
            f->entries[i].ival = strtol(s, NULL, 8);
         else
            f->entries[i].ival = strtol(s, NULL, 10);
         if (neg) f->entries[i].ival = -f->entries[i].ival;
         break;
      }
      case RBLXSTB__CTYPE_string:
         // @TODO
         break;
   }
}

static rblxstb_sdict *rblxstb__constant_file_hash;

rblxstb__Entry *rblxstb__constant_get_entry(char *filename, int line, int type)
{
   int i;
   rblxstb__FileEntry *f;
   if (rblxstb__constant_file_hash == NULL)
      rblxstb__constant_file_hash = rblxstb_sdict_new(RBLXSTB_TRUE);
   f = (rblxstb__FileEntry*) rblxstb_sdict_get(rblxstb__constant_file_hash, filename);
   if (f == NULL) {
      char *s = rblxstb__get_sourcefile_path(filename);
      if (s == NULL || !rblxstb_fexists(s)) return 0;
      f = (rblxstb__FileEntry *) malloc(sizeof(*f));
      f->timestamp = rblxstb_ftimestamp(s);
      f->file_data = rblxstb_stringfile(s, &f->file_len);
      f->filename = rblxstb_p_strdup(s); // cache the full path
      f->entries = NULL;
      f->line_index = 0;
      rblxstb_arr_setlen(f->line_index, f->file_len);
      memset(f->line_index, 0xff, rblxstb_arr_storage(f->line_index));
   } else {
      time_t t = rblxstb_ftimestamp(f->filename);
      if (f->timestamp != t) {
         f->timestamp = t;
         free(f->file_data);
         f->file_data = rblxstb_stringfile(f->filename, &f->file_len);
         rblxstb_arr_setlen(f->line_index, f->file_len);
         for (i=0; i < rblxstb_arr_len(f->entries); ++i)
            rblxstb__constant_parse(f, i);
      }
   }

   if (line >= f->file_len) return 0;

   if (f->line_index[line] >= rblxstb_arr_len(f->entries)) {
      // need a new entry
      int n = rblxstb_arr_len(f->entries);
      rblxstb__Entry e;
      e.line = line;
      if (line < f->file_len)
         f->line_index[line] = n;
      e.type = type;
      rblxstb_arr_push(f->entries, e);
      rblxstb__constant_parse(f, n);
   }
   return f->entries + f->line_index[line];
}

double rblxstb__double_constant(char *file, int line, double x)
{
   rblxstb__Entry *e = rblxstb__constant_get_entry(file, line, RBLXSTB__CTYPE_float);
   if (!e) return x;
   return e->dval;
}

int    rblxstb__int_constant(char *file, int line, int x)
{
   rblxstb__Entry *e = rblxstb__constant_get_entry(file, line, RBLXSTB__CTYPE_int);
   if (!e) return x;
   return e->ival;
}

char * rblxstb__string_constant(char *file, int line, char *x)
{
   rblxstb__Entry *e = rblxstb__constant_get_entry(file, line, RBLXSTB__CTYPE_string);
   if (!e) return x;
   return e->sval;
}

#endif // RBLXSTB_DEFINE
#endif // !RBLXSTB_DEBUG && !RBLXSTB_ALWAYS_H

#undef RBLXSTB_EXTERN
#endif // RBLXSTB_INCLUDE_STB_H

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2017 Sean Barrett
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/

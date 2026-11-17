// linearstb_sprintf - v1.10 - public domain snprintf() implementation
// originally by Jeff Roberts / RAD Game Tools, 2015/10/20
// http://github.com/nothings/linearstb
//
// allowed types:  sc uidBboXx p AaGgEef n
// lengths      :  hh h ll j z t I64 I32 I
//
// Contributors:
//    Fabian "ryg" Giesen (reformatting)
//    github:aganm (attribute format)
//
// Contributors (bugfixes):
//    github:d26435
//    github:trex78
//    github:account-login
//    Jari Komppa (SI suffixes)
//    Rohit Nirmal
//    Marcin Wojdyr
//    Leonard Ritter
//    Stefano Zanotti
//    Adam Allison
//    Arvid Gerstmann
//    Markus Kolb
//
// LICENSE:
//
//   See end of file for license information.

#ifndef LINEARSTB_SPRINTF_H_INCLUDE
#define LINEARSTB_SPRINTF_H_INCLUDE

/*
Single file sprintf replacement.

Originally written by Jeff Roberts at RAD Game Tools - 2015/10/20.
Hereby placed in public domain.

This is a full sprintf replacement that supports everything that
the C runtime sprintfs support, including float/double, 64-bit integers,
hex floats, field parameters (%*.*d stuff), length reads backs, etc.

Why would you need this if sprintf already exists?  Well, first off,
it's *much* faster (see below). It's also much smaller than the CRT
versions code-space-wise. We've also added some simple improvements
that are super handy (commas in thousands, callbacks at buffer full,
for example). Finally, the format strings for MSVC and GCC differ
for 64-bit integers (among other small things), so this lets you use
the same format strings in cross platform code.

It uses the standard single file trick of being both the header file
and the source itself. If you just include it normally, you just get
the header file function definitions. To get the code, you include
it from a C or C++ file and define LINEARSTB_SPRINTF_IMPLEMENTATION first.

It only uses va_args macros from the C runtime to do it's work. It
does cast doubles to S64s and shifts and divides U64s, which does
drag in CRT code on most platforms.

It compiles to roughly 8K with float support, and 4K without.
As a comparison, when using MSVC static libs, calling sprintf drags
in 16K.

API:
====
int linearstbsp_sprintf( char * buf, char const * fmt, ... )
int linearstbsp_snprintf( char * buf, int count, char const * fmt, ... )
  Convert an arg list into a buffer.  linearstbsp_snprintf always returns
  a zero-terminated string (unlike regular snprintf).

int linearstbsp_vsprintf( char * buf, char const * fmt, va_list va )
int linearstbsp_vsnprintf( char * buf, int count, char const * fmt, va_list va )
  Convert a va_list arg list into a buffer.  linearstbsp_vsnprintf always returns
  a zero-terminated string (unlike regular snprintf).

int linearstbsp_vsprintfcb( LINEARSTBSP_SPRINTFCB * callback, void * user, char * buf, char const * fmt, va_list va )
    typedef char * LINEARSTBSP_SPRINTFCB( char const * buf, void * user, int len );
  Convert into a buffer, calling back every LINEARSTB_SPRINTF_MIN chars.
  Your callback can then copy the chars out, print them or whatever.
  This function is actually the workhorse for everything else.
  The buffer you pass in must hold at least LINEARSTB_SPRINTF_MIN characters.
    // you return the next buffer to use or 0 to stop converting

void linearstbsp_set_separators( char comma, char period )
  Set the comma and period characters to use.

FLOATS/DOUBLES:
===============
This code uses a internal float->ascii conversion method that uses
doubles with error correction (double-doubles, for ~105 bits of
precision).  This conversion is round-trip perfect - that is, an atof
of the values output here will give you the bit-exact double back.

One difference is that our insignificant digits will be different than
with MSVC or GCC (but they don't match each other either).  We also
don't attempt to find the minimum length matching float (pre-MSVC15
doesn't either).

If you don't need float or doubles at all, define LINEARSTB_SPRINTF_NOFLOAT
and you'll save 4K of code space.

64-BIT INTS:
============
This library also supports 64-bit integers and you can use MSVC style or
GCC style indicators (%I64d or %lld).  It supports the C99 specifiers
for size_t and ptr_diff_t (%jd %zd) as well.

EXTRAS:
=======
Like some GCCs, for integers and floats, you can use a ' (single quote)
specifier and commas will be inserted on the thousands: "%'d" on 12345
would print 12,345.

For integers and floats, you can use a "$" specifier and the number
will be converted to float and then divided to get kilo, mega, giga or
tera and then printed, so "%$d" 1000 is "1.0 k", "%$.2d" 2536000 is
"2.53 M", etc. For byte values, use two $:s, like "%$$d" to turn
2536000 to "2.42 Mi". If you prefer JEDEC suffixes to SI ones, use three
$:s: "%$$$d" -> "2.42 M". To remove the space between the number and the
suffix, add "_" specifier: "%_$d" -> "2.53M".

In addition to octal and hexadecimal conversions, you can print
integers in binary: "%b" for 256 would print 100.

PERFORMANCE vs MSVC 2008 32-/64-bit (GCC is even slower than MSVC):
===================================================================
"%d" across all 32-bit ints (4.8x/4.0x faster than 32-/64-bit MSVC)
"%24d" across all 32-bit ints (4.5x/4.2x faster)
"%x" across all 32-bit ints (4.5x/3.8x faster)
"%08x" across all 32-bit ints (4.3x/3.8x faster)
"%f" across e-10 to e+10 floats (7.3x/6.0x faster)
"%e" across e-10 to e+10 floats (8.1x/6.0x faster)
"%g" across e-10 to e+10 floats (10.0x/7.1x faster)
"%f" for values near e-300 (7.9x/6.5x faster)
"%f" for values near e+300 (10.0x/9.1x faster)
"%e" for values near e-300 (10.1x/7.0x faster)
"%e" for values near e+300 (9.2x/6.0x faster)
"%.320f" for values near e-300 (12.6x/11.2x faster)
"%a" for random values (8.6x/4.3x faster)
"%I64d" for 64-bits with 32-bit values (4.8x/3.4x faster)
"%I64d" for 64-bits > 32-bit values (4.9x/5.5x faster)
"%s%s%s" for 64 char strings (7.1x/7.3x faster)
"...512 char string..." ( 35.0x/32.5x faster!)
*/

#if defined(__clang__)
 #if defined(__has_feature) && defined(__has_attribute)
  #if __has_feature(address_sanitizer)
   #if __has_attribute(__no_sanitize__)
    #define LINEARSTBSP__ASAN __attribute__((__no_sanitize__("address")))
   #elif __has_attribute(__no_sanitize_address__)
    #define LINEARSTBSP__ASAN __attribute__((__no_sanitize_address__))
   #elif __has_attribute(__no_address_safety_analysis__)
    #define LINEARSTBSP__ASAN __attribute__((__no_address_safety_analysis__))
   #endif
  #endif
 #endif
#elif defined(__GNUC__) && (__GNUC__ >= 5 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8))
 #if defined(__SANITIZE_ADDRESS__) && __SANITIZE_ADDRESS__
  #define LINEARSTBSP__ASAN __attribute__((__no_sanitize_address__))
 #endif
#endif

#ifndef LINEARSTBSP__ASAN
#define LINEARSTBSP__ASAN
#endif

#ifdef LINEARSTB_SPRINTF_STATIC
#define LINEARSTBSP__PUBLICDEC static
#define LINEARSTBSP__PUBLICDEF static LINEARSTBSP__ASAN
#else
#ifdef __cplusplus
#define LINEARSTBSP__PUBLICDEC extern "C"
#define LINEARSTBSP__PUBLICDEF extern "C" LINEARSTBSP__ASAN
#else
#define LINEARSTBSP__PUBLICDEC extern
#define LINEARSTBSP__PUBLICDEF LINEARSTBSP__ASAN
#endif
#endif

#if defined(__has_attribute)
 #if __has_attribute(format)
   #define LINEARSTBSP__ATTRIBUTE_FORMAT(fmt,va) __attribute__((format(printf,fmt,va)))
 #endif
#endif

#ifndef LINEARSTBSP__ATTRIBUTE_FORMAT
#define LINEARSTBSP__ATTRIBUTE_FORMAT(fmt,va)
#endif

#ifdef _MSC_VER
#define LINEARSTBSP__NOTUSED(v)  (void)(v)
#else
#define LINEARSTBSP__NOTUSED(v)  (void)sizeof(v)
#endif

#include <stdarg.h> // for va_arg(), va_list()
#include <stddef.h> // size_t, ptrdiff_t

#ifndef LINEARSTB_SPRINTF_MIN
#define LINEARSTB_SPRINTF_MIN 512 // how many characters per callback
#endif
typedef char *LINEARSTBSP_SPRINTFCB(const char *buf, void *user, int len);

#ifndef LINEARSTB_SPRINTF_DECORATE
#define LINEARSTB_SPRINTF_DECORATE(name) linearstbsp_##name // define this before including if you want to change the names
#endif

LINEARSTBSP__PUBLICDEC int LINEARSTB_SPRINTF_DECORATE(vsprintf)(char *buf, char const *fmt, va_list va);
LINEARSTBSP__PUBLICDEC int LINEARSTB_SPRINTF_DECORATE(vsnprintf)(char *buf, int count, char const *fmt, va_list va);
LINEARSTBSP__PUBLICDEC int LINEARSTB_SPRINTF_DECORATE(sprintf)(char *buf, char const *fmt, ...) LINEARSTBSP__ATTRIBUTE_FORMAT(2,3);
LINEARSTBSP__PUBLICDEC int LINEARSTB_SPRINTF_DECORATE(snprintf)(char *buf, int count, char const *fmt, ...) LINEARSTBSP__ATTRIBUTE_FORMAT(3,4);

LINEARSTBSP__PUBLICDEC int LINEARSTB_SPRINTF_DECORATE(vsprintfcb)(LINEARSTBSP_SPRINTFCB *callback, void *user, char *buf, char const *fmt, va_list va);
LINEARSTBSP__PUBLICDEC void LINEARSTB_SPRINTF_DECORATE(set_separators)(char comma, char period);

#endif // LINEARSTB_SPRINTF_H_INCLUDE

#ifdef LINEARSTB_SPRINTF_IMPLEMENTATION

#define linearstbsp__uint32 unsigned int
#define linearstbsp__int32 signed int

#ifdef _MSC_VER
#define linearstbsp__uint64 unsigned __int64
#define linearstbsp__int64 signed __int64
#else
#define linearstbsp__uint64 unsigned long long
#define linearstbsp__int64 signed long long
#endif
#define linearstbsp__uint16 unsigned short

#ifndef linearstbsp__uintptr
#if defined(__ppc64__) || defined(__powerpc64__) || defined(__aarch64__) || defined(_M_X64) || defined(__x86_64__) || defined(__x86_64) || defined(__s390x__)
#define linearstbsp__uintptr linearstbsp__uint64
#else
#define linearstbsp__uintptr linearstbsp__uint32
#endif
#endif

#ifndef LINEARSTB_SPRINTF_MSVC_MODE // used for MSVC2013 and earlier (MSVC2015 matches GCC)
#if defined(_MSC_VER) && (_MSC_VER < 1900)
#define LINEARSTB_SPRINTF_MSVC_MODE
#endif
#endif

#ifdef LINEARSTB_SPRINTF_NOUNALIGNED // define this before inclusion to force linearstbsp_sprintf to always use aligned accesses
#define LINEARSTBSP__UNALIGNED(code)
#else
#define LINEARSTBSP__UNALIGNED(code) code
#endif

#ifndef LINEARSTB_SPRINTF_NOFLOAT
// internal float utility functions
static linearstbsp__int32 linearstbsp__real_to_str(char const **start, linearstbsp__uint32 *len, char *out, linearstbsp__int32 *decimal_pos, double value, linearstbsp__uint32 frac_digits);
static linearstbsp__int32 linearstbsp__real_to_parts(linearstbsp__int64 *bits, linearstbsp__int32 *expo, double value);
#define LINEARSTBSP__SPECIAL 0x7000
#endif

static char linearstbsp__period = '.';
static char linearstbsp__comma = ',';
static struct
{
   short temp; // force next field to be 2-byte aligned
   char pair[201];
} linearstbsp__digitpair =
{
  0,
   "00010203040506070809101112131415161718192021222324"
   "25262728293031323334353637383940414243444546474849"
   "50515253545556575859606162636465666768697071727374"
   "75767778798081828384858687888990919293949596979899"
};

LINEARSTBSP__PUBLICDEF void LINEARSTB_SPRINTF_DECORATE(set_separators)(char pcomma, char pperiod)
{
   linearstbsp__period = pperiod;
   linearstbsp__comma = pcomma;
}

#define LINEARSTBSP__LEFTJUST 1
#define LINEARSTBSP__LEADINGPLUS 2
#define LINEARSTBSP__LEADINGSPACE 4
#define LINEARSTBSP__LEADING_0X 8
#define LINEARSTBSP__LEADINGZERO 16
#define LINEARSTBSP__INTMAX 32
#define LINEARSTBSP__TRIPLET_COMMA 64
#define LINEARSTBSP__NEGATIVE 128
#define LINEARSTBSP__METRIC_SUFFIX 256
#define LINEARSTBSP__HALFWIDTH 512
#define LINEARSTBSP__METRIC_NOSPACE 1024
#define LINEARSTBSP__METRIC_1024 2048
#define LINEARSTBSP__METRIC_JEDEC 4096

static void linearstbsp__lead_sign(linearstbsp__uint32 fl, char *sign)
{
   sign[0] = 0;
   if (fl & LINEARSTBSP__NEGATIVE) {
      sign[0] = 1;
      sign[1] = '-';
   } else if (fl & LINEARSTBSP__LEADINGSPACE) {
      sign[0] = 1;
      sign[1] = ' ';
   } else if (fl & LINEARSTBSP__LEADINGPLUS) {
      sign[0] = 1;
      sign[1] = '+';
   }
}

static LINEARSTBSP__ASAN linearstbsp__uint32 linearstbsp__strlen_limited(char const *s, linearstbsp__uint32 limit)
{
   char const * sn = s;

   // get up to 4-byte alignment
   for (;;) {
      if (((linearstbsp__uintptr)sn & 3) == 0)
         break;

      if (!limit || *sn == 0)
         return (linearstbsp__uint32)(sn - s);

      ++sn;
      --limit;
   }

   // scan over 4 bytes at a time to find terminating 0
   // this will intentionally scan up to 3 bytes past the end of buffers,
   // but becase it works 4B aligned, it will never cross page boundaries
   // (hence the LINEARSTBSP__ASAN markup; the over-read here is intentional
   // and harmless)
   while (limit >= 4) {
      linearstbsp__uint32 v = *(linearstbsp__uint32 *)sn;
      // bit hack to find if there's a 0 byte in there
      if ((v - 0x01010101) & (~v) & 0x80808080UL)
         break;

      sn += 4;
      limit -= 4;
   }

   // handle the last few characters to find actual size
   while (limit && *sn) {
      ++sn;
      --limit;
   }

   return (linearstbsp__uint32)(sn - s);
}

LINEARSTBSP__PUBLICDEF int LINEARSTB_SPRINTF_DECORATE(vsprintfcb)(LINEARSTBSP_SPRINTFCB *callback, void *user, char *buf, char const *fmt, va_list va)
{
   static char hex[] = "0123456789abcdefxp";
   static char hexu[] = "0123456789ABCDEFXP";
   char *bf;
   char const *f;
   int tlen = 0;

   bf = buf;
   f = fmt;
   for (;;) {
      linearstbsp__int32 fw, pr, tz;
      linearstbsp__uint32 fl;

      // macros for the callback buffer stuff
      #define linearstbsp__chk_cb_bufL(bytes)                        \
         {                                                     \
            int len = (int)(bf - buf);                         \
            if ((len + (bytes)) >= LINEARSTB_SPRINTF_MIN) {          \
               tlen += len;                                    \
               if (0 == (bf = buf = callback(buf, user, len))) \
                  goto done;                                   \
            }                                                  \
         }
      #define linearstbsp__chk_cb_buf(bytes)    \
         {                                \
            if (callback) {               \
               linearstbsp__chk_cb_bufL(bytes); \
            }                             \
         }
      #define linearstbsp__flush_cb()                      \
         {                                           \
            linearstbsp__chk_cb_bufL(LINEARSTB_SPRINTF_MIN - 1); \
         } // flush if there is even one byte in the buffer
      #define linearstbsp__cb_buf_clamp(cl, v)                \
         cl = v;                                        \
         if (callback) {                                \
            int lg = LINEARSTB_SPRINTF_MIN - (int)(bf - buf); \
            if (cl > lg)                                \
               cl = lg;                                 \
         }

      // fast copy everything up to the next % (or end of string)
      for (;;) {
         while (((linearstbsp__uintptr)f) & 3) {
         schk1:
            if (f[0] == '%')
               goto scandd;
         schk2:
            if (f[0] == 0)
               goto endfmt;
            linearstbsp__chk_cb_buf(1);
            *bf++ = f[0];
            ++f;
         }
         for (;;) {
            // Check if the next 4 bytes contain %(0x25) or end of string.
            // Using the 'hasless' trick:
            // https://graphics.stanford.edu/~seander/bithacks.html#HasLessInWord
            linearstbsp__uint32 v, c;
            v = *(linearstbsp__uint32 *)f;
            c = (~v) & 0x80808080;
            if (((v ^ 0x25252525) - 0x01010101) & c)
               goto schk1;
            if ((v - 0x01010101) & c)
               goto schk2;
            if (callback)
               if ((LINEARSTB_SPRINTF_MIN - (int)(bf - buf)) < 4)
                  goto schk1;
            #ifdef LINEARSTB_SPRINTF_NOUNALIGNED
                if(((linearstbsp__uintptr)bf) & 3) {
                    bf[0] = f[0];
                    bf[1] = f[1];
                    bf[2] = f[2];
                    bf[3] = f[3];
                } else
            #endif
            {
                *(linearstbsp__uint32 *)bf = v;
            }
            bf += 4;
            f += 4;
         }
      }
   scandd:

      ++f;

      // ok, we have a percent, read the modifiers first
      fw = 0;
      pr = -1;
      fl = 0;
      tz = 0;

      // flags
      for (;;) {
         switch (f[0]) {
         // if we have left justify
         case '-':
            fl |= LINEARSTBSP__LEFTJUST;
            ++f;
            continue;
         // if we have leading plus
         case '+':
            fl |= LINEARSTBSP__LEADINGPLUS;
            ++f;
            continue;
         // if we have leading space
         case ' ':
            fl |= LINEARSTBSP__LEADINGSPACE;
            ++f;
            continue;
         // if we have leading 0x
         case '#':
            fl |= LINEARSTBSP__LEADING_0X;
            ++f;
            continue;
         // if we have thousand commas
         case '\'':
            fl |= LINEARSTBSP__TRIPLET_COMMA;
            ++f;
            continue;
         // if we have kilo marker (none->kilo->kibi->jedec)
         case '$':
            if (fl & LINEARSTBSP__METRIC_SUFFIX) {
               if (fl & LINEARSTBSP__METRIC_1024) {
                  fl |= LINEARSTBSP__METRIC_JEDEC;
               } else {
                  fl |= LINEARSTBSP__METRIC_1024;
               }
            } else {
               fl |= LINEARSTBSP__METRIC_SUFFIX;
            }
            ++f;
            continue;
         // if we don't want space between metric suffix and number
         case '_':
            fl |= LINEARSTBSP__METRIC_NOSPACE;
            ++f;
            continue;
         // if we have leading zero
         case '0':
            fl |= LINEARSTBSP__LEADINGZERO;
            ++f;
            goto flags_done;
         default: goto flags_done;
         }
      }
   flags_done:

      // get the field width
      if (f[0] == '*') {
         fw = va_arg(va, linearstbsp__uint32);
         ++f;
      } else {
         while ((f[0] >= '0') && (f[0] <= '9')) {
            fw = fw * 10 + f[0] - '0';
            f++;
         }
      }
      // get the precision
      if (f[0] == '.') {
         ++f;
         if (f[0] == '*') {
            pr = va_arg(va, linearstbsp__uint32);
            ++f;
         } else {
            pr = 0;
            while ((f[0] >= '0') && (f[0] <= '9')) {
               pr = pr * 10 + f[0] - '0';
               f++;
            }
         }
      }

      // handle integer size overrides
      switch (f[0]) {
      // are we halfwidth?
      case 'h':
         fl |= LINEARSTBSP__HALFWIDTH;
         ++f;
         if (f[0] == 'h')
            ++f;  // QUARTERWIDTH
         break;
      // are we 64-bit (unix style)
      case 'l':
         fl |= ((sizeof(long) == 8) ? LINEARSTBSP__INTMAX : 0);
         ++f;
         if (f[0] == 'l') {
            fl |= LINEARSTBSP__INTMAX;
            ++f;
         }
         break;
      // are we 64-bit on intmax? (c99)
      case 'j':
         fl |= (sizeof(size_t) == 8) ? LINEARSTBSP__INTMAX : 0;
         ++f;
         break;
      // are we 64-bit on size_t or ptrdiff_t? (c99)
      case 'z':
         fl |= (sizeof(ptrdiff_t) == 8) ? LINEARSTBSP__INTMAX : 0;
         ++f;
         break;
      case 't':
         fl |= (sizeof(ptrdiff_t) == 8) ? LINEARSTBSP__INTMAX : 0;
         ++f;
         break;
      // are we 64-bit (msft style)
      case 'I':
         if ((f[1] == '6') && (f[2] == '4')) {
            fl |= LINEARSTBSP__INTMAX;
            f += 3;
         } else if ((f[1] == '3') && (f[2] == '2')) {
            f += 3;
         } else {
            fl |= ((sizeof(void *) == 8) ? LINEARSTBSP__INTMAX : 0);
            ++f;
         }
         break;
      default: break;
      }

      // handle each replacement
      switch (f[0]) {
         #define LINEARSTBSP__NUMSZ 512 // big enough for e308 (with commas) or e-307
         char num[LINEARSTBSP__NUMSZ];
         char lead[8];
         char tail[8];
         char *s;
         char const *h;
         linearstbsp__uint32 l, n, cs;
         linearstbsp__uint64 n64;
#ifndef LINEARSTB_SPRINTF_NOFLOAT
         double fv;
#endif
         linearstbsp__int32 dp;
         char const *sn;

      case 's':
         // get the string
         s = va_arg(va, char *);
         if (s == 0)
            s = (char *)"null";
         // get the length, limited to desired precision
         // always limit to ~0u chars since our counts are 32b
         l = linearstbsp__strlen_limited(s, (pr >= 0) ? pr : ~0u);
         lead[0] = 0;
         tail[0] = 0;
         pr = 0;
         dp = 0;
         cs = 0;
         // copy the string in
         goto scopy;

      case 'c': // char
         // get the character
         s = num + LINEARSTBSP__NUMSZ - 1;
         *s = (char)va_arg(va, int);
         l = 1;
         lead[0] = 0;
         tail[0] = 0;
         pr = 0;
         dp = 0;
         cs = 0;
         goto scopy;

      case 'n': // weird write-bytes specifier
      {
         int *d = va_arg(va, int *);
         *d = tlen + (int)(bf - buf);
      } break;

#ifdef LINEARSTB_SPRINTF_NOFLOAT
      case 'A':              // float
      case 'a':              // hex float
      case 'G':              // float
      case 'g':              // float
      case 'E':              // float
      case 'e':              // float
      case 'f':              // float
         va_arg(va, double); // eat it
         s = (char *)"No float";
         l = 8;
         lead[0] = 0;
         tail[0] = 0;
         pr = 0;
         cs = 0;
         LINEARSTBSP__NOTUSED(dp);
         goto scopy;
#else
      case 'A': // hex float
      case 'a': // hex float
         h = (f[0] == 'A') ? hexu : hex;
         fv = va_arg(va, double);
         if (pr == -1)
            pr = 6; // default is 6
         // read the double into a string
         if (linearstbsp__real_to_parts((linearstbsp__int64 *)&n64, &dp, fv))
            fl |= LINEARSTBSP__NEGATIVE;

         s = num + 64;

         linearstbsp__lead_sign(fl, lead);

         if (dp == -1023)
            dp = (n64) ? -1022 : 0;
         else
            n64 |= (((linearstbsp__uint64)1) << 52);
         n64 <<= (64 - 56);
         if (pr < 15)
            n64 += ((((linearstbsp__uint64)8) << 56) >> (pr * 4));
// add leading chars

#ifdef LINEARSTB_SPRINTF_MSVC_MODE
         *s++ = '0';
         *s++ = 'x';
#else
         lead[1 + lead[0]] = '0';
         lead[2 + lead[0]] = 'x';
         lead[0] += 2;
#endif
         *s++ = h[(n64 >> 60) & 15];
         n64 <<= 4;
         if (pr)
            *s++ = linearstbsp__period;
         sn = s;

         // print the bits
         n = pr;
         if (n > 13)
            n = 13;
         if (pr > (linearstbsp__int32)n)
            tz = pr - n;
         pr = 0;
         while (n--) {
            *s++ = h[(n64 >> 60) & 15];
            n64 <<= 4;
         }

         // print the expo
         tail[1] = h[17];
         if (dp < 0) {
            tail[2] = '-';
            dp = -dp;
         } else
            tail[2] = '+';
         n = (dp >= 1000) ? 6 : ((dp >= 100) ? 5 : ((dp >= 10) ? 4 : 3));
         tail[0] = (char)n;
         for (;;) {
            tail[n] = '0' + dp % 10;
            if (n <= 3)
               break;
            --n;
            dp /= 10;
         }

         dp = (int)(s - sn);
         l = (int)(s - (num + 64));
         s = num + 64;
         cs = 1 + (3 << 24);
         goto scopy;

      case 'G': // float
      case 'g': // float
         h = (f[0] == 'G') ? hexu : hex;
         fv = va_arg(va, double);
         if (pr == -1)
            pr = 6;
         else if (pr == 0)
            pr = 1; // default is 6
         // read the double into a string
         if (linearstbsp__real_to_str(&sn, &l, num, &dp, fv, (pr - 1) | 0x80000000))
            fl |= LINEARSTBSP__NEGATIVE;

         // clamp the precision and delete extra zeros after clamp
         n = pr;
         if (l > (linearstbsp__uint32)pr)
            l = pr;
         while ((l > 1) && (pr) && (sn[l - 1] == '0')) {
            --pr;
            --l;
         }

         // should we use %e
         if ((dp <= -4) || (dp > (linearstbsp__int32)n)) {
            if (pr > (linearstbsp__int32)l)
               pr = l - 1;
            else if (pr)
               --pr; // when using %e, there is one digit before the decimal
            goto doexpfromg;
         }
         // this is the insane action to get the pr to match %g semantics for %f
         if (dp > 0) {
            pr = (dp < (linearstbsp__int32)l) ? l - dp : 0;
         } else {
            pr = -dp + ((pr > (linearstbsp__int32)l) ? (linearstbsp__int32) l : pr);
         }
         goto dofloatfromg;

      case 'E': // float
      case 'e': // float
         h = (f[0] == 'E') ? hexu : hex;
         fv = va_arg(va, double);
         if (pr == -1)
            pr = 6; // default is 6
         // read the double into a string
         if (linearstbsp__real_to_str(&sn, &l, num, &dp, fv, pr | 0x80000000))
            fl |= LINEARSTBSP__NEGATIVE;
      doexpfromg:
         tail[0] = 0;
         linearstbsp__lead_sign(fl, lead);
         if (dp == LINEARSTBSP__SPECIAL) {
            s = (char *)sn;
            cs = 0;
            pr = 0;
            goto scopy;
         }
         s = num + 64;
         // handle leading chars
         *s++ = sn[0];

         if (pr)
            *s++ = linearstbsp__period;

         // handle after decimal
         if ((l - 1) > (linearstbsp__uint32)pr)
            l = pr + 1;
         for (n = 1; n < l; n++)
            *s++ = sn[n];
         // trailing zeros
         tz = pr - (l - 1);
         pr = 0;
         // dump expo
         tail[1] = h[0xe];
         dp -= 1;
         if (dp < 0) {
            tail[2] = '-';
            dp = -dp;
         } else
            tail[2] = '+';
#ifdef LINEARSTB_SPRINTF_MSVC_MODE
         n = 5;
#else
         n = (dp >= 100) ? 5 : 4;
#endif
         tail[0] = (char)n;
         for (;;) {
            tail[n] = '0' + dp % 10;
            if (n <= 3)
               break;
            --n;
            dp /= 10;
         }
         cs = 1 + (3 << 24); // how many tens
         goto flt_lead;

      case 'f': // float
         fv = va_arg(va, double);
      doafloat:
         // do kilos
         if (fl & LINEARSTBSP__METRIC_SUFFIX) {
            double divisor;
            divisor = 1000.0f;
            if (fl & LINEARSTBSP__METRIC_1024)
               divisor = 1024.0;
            while (fl < 0x4000000) {
               if ((fv < divisor) && (fv > -divisor))
                  break;
               fv /= divisor;
               fl += 0x1000000;
            }
         }
         if (pr == -1)
            pr = 6; // default is 6
         // read the double into a string
         if (linearstbsp__real_to_str(&sn, &l, num, &dp, fv, pr))
            fl |= LINEARSTBSP__NEGATIVE;
      dofloatfromg:
         tail[0] = 0;
         linearstbsp__lead_sign(fl, lead);
         if (dp == LINEARSTBSP__SPECIAL) {
            s = (char *)sn;
            cs = 0;
            pr = 0;
            goto scopy;
         }
         s = num + 64;

         // handle the three decimal varieties
         if (dp <= 0) {
            linearstbsp__int32 i;
            // handle 0.000*000xxxx
            *s++ = '0';
            if (pr)
               *s++ = linearstbsp__period;
            n = -dp;
            if ((linearstbsp__int32)n > pr)
               n = pr;
            i = n;
            while (i) {
               if ((((linearstbsp__uintptr)s) & 3) == 0)
                  break;
               *s++ = '0';
               --i;
            }
            while (i >= 4) {
               *(linearstbsp__uint32 *)s = 0x30303030;
               s += 4;
               i -= 4;
            }
            while (i) {
               *s++ = '0';
               --i;
            }
            if ((linearstbsp__int32)(l + n) > pr)
               l = pr - n;
            i = l;
            while (i) {
               *s++ = *sn++;
               --i;
            }
            tz = pr - (n + l);
            cs = 1 + (3 << 24); // how many tens did we write (for commas below)
         } else {
            cs = (fl & LINEARSTBSP__TRIPLET_COMMA) ? ((600 - (linearstbsp__uint32)dp) % 3) : 0;
            if ((linearstbsp__uint32)dp >= l) {
               // handle xxxx000*000.0
               n = 0;
               for (;;) {
                  if ((fl & LINEARSTBSP__TRIPLET_COMMA) && (++cs == 4)) {
                     cs = 0;
                     *s++ = linearstbsp__comma;
                  } else {
                     *s++ = sn[n];
                     ++n;
                     if (n >= l)
                        break;
                  }
               }
               if (n < (linearstbsp__uint32)dp) {
                  n = dp - n;
                  if ((fl & LINEARSTBSP__TRIPLET_COMMA) == 0) {
                     while (n) {
                        if ((((linearstbsp__uintptr)s) & 3) == 0)
                           break;
                        *s++ = '0';
                        --n;
                     }
                     while (n >= 4) {
                        *(linearstbsp__uint32 *)s = 0x30303030;
                        s += 4;
                        n -= 4;
                     }
                  }
                  while (n) {
                     if ((fl & LINEARSTBSP__TRIPLET_COMMA) && (++cs == 4)) {
                        cs = 0;
                        *s++ = linearstbsp__comma;
                     } else {
                        *s++ = '0';
                        --n;
                     }
                  }
               }
               cs = (int)(s - (num + 64)) + (3 << 24); // cs is how many tens
               if (pr) {
                  *s++ = linearstbsp__period;
                  tz = pr;
               }
            } else {
               // handle xxxxx.xxxx000*000
               n = 0;
               for (;;) {
                  if ((fl & LINEARSTBSP__TRIPLET_COMMA) && (++cs == 4)) {
                     cs = 0;
                     *s++ = linearstbsp__comma;
                  } else {
                     *s++ = sn[n];
                     ++n;
                     if (n >= (linearstbsp__uint32)dp)
                        break;
                  }
               }
               cs = (int)(s - (num + 64)) + (3 << 24); // cs is how many tens
               if (pr)
                  *s++ = linearstbsp__period;
               if ((l - dp) > (linearstbsp__uint32)pr)
                  l = pr + dp;
               while (n < l) {
                  *s++ = sn[n];
                  ++n;
               }
               tz = pr - (l - dp);
            }
         }
         pr = 0;

         // handle k,m,g,t
         if (fl & LINEARSTBSP__METRIC_SUFFIX) {
            char idx;
            idx = 1;
            if (fl & LINEARSTBSP__METRIC_NOSPACE)
               idx = 0;
            tail[0] = idx;
            tail[1] = ' ';
            {
               if (fl >> 24) { // SI kilo is 'k', JEDEC and SI kibits are 'K'.
                  if (fl & LINEARSTBSP__METRIC_1024)
                     tail[idx + 1] = "_KMGT"[fl >> 24];
                  else
                     tail[idx + 1] = "_kMGT"[fl >> 24];
                  idx++;
                  // If printing kibits and not in jedec, add the 'i'.
                  if (fl & LINEARSTBSP__METRIC_1024 && !(fl & LINEARSTBSP__METRIC_JEDEC)) {
                     tail[idx + 1] = 'i';
                     idx++;
                  }
                  tail[0] = idx;
               }
            }
         };

      flt_lead:
         // get the length that we copied
         l = (linearstbsp__uint32)(s - (num + 64));
         s = num + 64;
         goto scopy;
#endif

      case 'B': // upper binary
      case 'b': // lower binary
         h = (f[0] == 'B') ? hexu : hex;
         lead[0] = 0;
         if (fl & LINEARSTBSP__LEADING_0X) {
            lead[0] = 2;
            lead[1] = '0';
            lead[2] = h[0xb];
         }
         l = (8 << 4) | (1 << 8);
         goto radixnum;

      case 'o': // octal
         h = hexu;
         lead[0] = 0;
         if (fl & LINEARSTBSP__LEADING_0X) {
            lead[0] = 1;
            lead[1] = '0';
         }
         l = (3 << 4) | (3 << 8);
         goto radixnum;

      case 'p': // pointer
         fl |= (sizeof(void *) == 8) ? LINEARSTBSP__INTMAX : 0;
         pr = sizeof(void *) * 2;
         fl &= ~LINEARSTBSP__LEADINGZERO; // 'p' only prints the pointer with zeros
                                    // fall through - to X

      case 'X': // upper hex
      case 'x': // lower hex
         h = (f[0] == 'X') ? hexu : hex;
         l = (4 << 4) | (4 << 8);
         lead[0] = 0;
         if (fl & LINEARSTBSP__LEADING_0X) {
            lead[0] = 2;
            lead[1] = '0';
            lead[2] = h[16];
         }
      radixnum:
         // get the number
         if (fl & LINEARSTBSP__INTMAX)
            n64 = va_arg(va, linearstbsp__uint64);
         else
            n64 = va_arg(va, linearstbsp__uint32);

         s = num + LINEARSTBSP__NUMSZ;
         dp = 0;
         // clear tail, and clear leading if value is zero
         tail[0] = 0;
         if (n64 == 0) {
            lead[0] = 0;
            if (pr == 0) {
               l = 0;
               cs = 0;
               goto scopy;
            }
         }
         // convert to string
         for (;;) {
            *--s = h[n64 & ((1 << (l >> 8)) - 1)];
            n64 >>= (l >> 8);
            if (!((n64) || ((linearstbsp__int32)((num + LINEARSTBSP__NUMSZ) - s) < pr)))
               break;
            if (fl & LINEARSTBSP__TRIPLET_COMMA) {
               ++l;
               if ((l & 15) == ((l >> 4) & 15)) {
                  l &= ~15;
                  *--s = linearstbsp__comma;
               }
            }
         };
         // get the tens and the comma pos
         cs = (linearstbsp__uint32)((num + LINEARSTBSP__NUMSZ) - s) + ((((l >> 4) & 15)) << 24);
         // get the length that we copied
         l = (linearstbsp__uint32)((num + LINEARSTBSP__NUMSZ) - s);
         // copy it
         goto scopy;

      case 'u': // unsigned
      case 'i':
      case 'd': // integer
         // get the integer and abs it
         if (fl & LINEARSTBSP__INTMAX) {
            linearstbsp__int64 i64 = va_arg(va, linearstbsp__int64);
            n64 = (linearstbsp__uint64)i64;
            if ((f[0] != 'u') && (i64 < 0)) {
               n64 = (linearstbsp__uint64)-i64;
               fl |= LINEARSTBSP__NEGATIVE;
            }
         } else {
            linearstbsp__int32 i = va_arg(va, linearstbsp__int32);
            n64 = (linearstbsp__uint32)i;
            if ((f[0] != 'u') && (i < 0)) {
               n64 = (linearstbsp__uint32)-i;
               fl |= LINEARSTBSP__NEGATIVE;
            }
         }

#ifndef LINEARSTB_SPRINTF_NOFLOAT
         if (fl & LINEARSTBSP__METRIC_SUFFIX) {
            if (n64 < 1024)
               pr = 0;
            else if (pr == -1)
               pr = 1;
            fv = (double)(linearstbsp__int64)n64;
            goto doafloat;
         }
#endif

         // convert to string
         s = num + LINEARSTBSP__NUMSZ;
         l = 0;

         for (;;) {
            // do in 32-bit chunks (avoid lots of 64-bit divides even with constant denominators)
            char *o = s - 8;
            if (n64 >= 100000000) {
               n = (linearstbsp__uint32)(n64 % 100000000);
               n64 /= 100000000;
            } else {
               n = (linearstbsp__uint32)n64;
               n64 = 0;
            }
            if ((fl & LINEARSTBSP__TRIPLET_COMMA) == 0) {
               do {
                  s -= 2;
                  *(linearstbsp__uint16 *)s = *(linearstbsp__uint16 *)&linearstbsp__digitpair.pair[(n % 100) * 2];
                  n /= 100;
               } while (n);
            }
            while (n) {
               if ((fl & LINEARSTBSP__TRIPLET_COMMA) && (l++ == 3)) {
                  l = 0;
                  *--s = linearstbsp__comma;
                  --o;
               } else {
                  *--s = (char)(n % 10) + '0';
                  n /= 10;
               }
            }
            if (n64 == 0) {
               if ((s[0] == '0') && (s != (num + LINEARSTBSP__NUMSZ)))
                  ++s;
               break;
            }
            while (s != o)
               if ((fl & LINEARSTBSP__TRIPLET_COMMA) && (l++ == 3)) {
                  l = 0;
                  *--s = linearstbsp__comma;
                  --o;
               } else {
                  *--s = '0';
               }
         }

         tail[0] = 0;
         linearstbsp__lead_sign(fl, lead);

         // get the length that we copied
         l = (linearstbsp__uint32)((num + LINEARSTBSP__NUMSZ) - s);
         if (l == 0) {
            *--s = '0';
            l = 1;
         }
         cs = l + (3 << 24);
         if (pr < 0)
            pr = 0;

      scopy:
         // get fw=leading/trailing space, pr=leading zeros
         if (pr < (linearstbsp__int32)l)
            pr = l;
         n = pr + lead[0] + tail[0] + tz;
         if (fw < (linearstbsp__int32)n)
            fw = n;
         fw -= n;
         pr -= l;

         // handle right justify and leading zeros
         if ((fl & LINEARSTBSP__LEFTJUST) == 0) {
            if (fl & LINEARSTBSP__LEADINGZERO) // if leading zeros, everything is in pr
            {
               pr = (fw > pr) ? fw : pr;
               fw = 0;
            } else {
               fl &= ~LINEARSTBSP__TRIPLET_COMMA; // if no leading zeros, then no commas
            }
         }

         // copy the spaces and/or zeros
         if (fw + pr) {
            linearstbsp__int32 i;
            linearstbsp__uint32 c;

            // copy leading spaces (or when doing %8.4d stuff)
            if ((fl & LINEARSTBSP__LEFTJUST) == 0)
               while (fw > 0) {
                  linearstbsp__cb_buf_clamp(i, fw);
                  fw -= i;
                  while (i) {
                     if ((((linearstbsp__uintptr)bf) & 3) == 0)
                        break;
                     *bf++ = ' ';
                     --i;
                  }
                  while (i >= 4) {
                     *(linearstbsp__uint32 *)bf = 0x20202020;
                     bf += 4;
                     i -= 4;
                  }
                  while (i) {
                     *bf++ = ' ';
                     --i;
                  }
                  linearstbsp__chk_cb_buf(1);
               }

            // copy leader
            sn = lead + 1;
            while (lead[0]) {
               linearstbsp__cb_buf_clamp(i, lead[0]);
               lead[0] -= (char)i;
               while (i) {
                  *bf++ = *sn++;
                  --i;
               }
               linearstbsp__chk_cb_buf(1);
            }

            // copy leading zeros
            c = cs >> 24;
            cs &= 0xffffff;
            cs = (fl & LINEARSTBSP__TRIPLET_COMMA) ? ((linearstbsp__uint32)(c - ((pr + cs) % (c + 1)))) : 0;
            while (pr > 0) {
               linearstbsp__cb_buf_clamp(i, pr);
               pr -= i;
               if ((fl & LINEARSTBSP__TRIPLET_COMMA) == 0) {
                  while (i) {
                     if ((((linearstbsp__uintptr)bf) & 3) == 0)
                        break;
                     *bf++ = '0';
                     --i;
                  }
                  while (i >= 4) {
                     *(linearstbsp__uint32 *)bf = 0x30303030;
                     bf += 4;
                     i -= 4;
                  }
               }
               while (i) {
                  if ((fl & LINEARSTBSP__TRIPLET_COMMA) && (cs++ == c)) {
                     cs = 0;
                     *bf++ = linearstbsp__comma;
                  } else
                     *bf++ = '0';
                  --i;
               }
               linearstbsp__chk_cb_buf(1);
            }
         }

         // copy leader if there is still one
         sn = lead + 1;
         while (lead[0]) {
            linearstbsp__int32 i;
            linearstbsp__cb_buf_clamp(i, lead[0]);
            lead[0] -= (char)i;
            while (i) {
               *bf++ = *sn++;
               --i;
            }
            linearstbsp__chk_cb_buf(1);
         }

         // copy the string
         n = l;
         while (n) {
            linearstbsp__int32 i;
            linearstbsp__cb_buf_clamp(i, n);
            n -= i;
            LINEARSTBSP__UNALIGNED(while (i >= 4) {
               *(linearstbsp__uint32 volatile *)bf = *(linearstbsp__uint32 volatile *)s;
               bf += 4;
               s += 4;
               i -= 4;
            })
            while (i) {
               *bf++ = *s++;
               --i;
            }
            linearstbsp__chk_cb_buf(1);
         }

         // copy trailing zeros
         while (tz) {
            linearstbsp__int32 i;
            linearstbsp__cb_buf_clamp(i, tz);
            tz -= i;
            while (i) {
               if ((((linearstbsp__uintptr)bf) & 3) == 0)
                  break;
               *bf++ = '0';
               --i;
            }
            while (i >= 4) {
               *(linearstbsp__uint32 *)bf = 0x30303030;
               bf += 4;
               i -= 4;
            }
            while (i) {
               *bf++ = '0';
               --i;
            }
            linearstbsp__chk_cb_buf(1);
         }

         // copy tail if there is one
         sn = tail + 1;
         while (tail[0]) {
            linearstbsp__int32 i;
            linearstbsp__cb_buf_clamp(i, tail[0]);
            tail[0] -= (char)i;
            while (i) {
               *bf++ = *sn++;
               --i;
            }
            linearstbsp__chk_cb_buf(1);
         }

         // handle the left justify
         if (fl & LINEARSTBSP__LEFTJUST)
            if (fw > 0) {
               while (fw) {
                  linearstbsp__int32 i;
                  linearstbsp__cb_buf_clamp(i, fw);
                  fw -= i;
                  while (i) {
                     if ((((linearstbsp__uintptr)bf) & 3) == 0)
                        break;
                     *bf++ = ' ';
                     --i;
                  }
                  while (i >= 4) {
                     *(linearstbsp__uint32 *)bf = 0x20202020;
                     bf += 4;
                     i -= 4;
                  }
                  while (i--)
                     *bf++ = ' ';
                  linearstbsp__chk_cb_buf(1);
               }
            }
         break;

      default: // unknown, just copy code
         s = num + LINEARSTBSP__NUMSZ - 1;
         *s = f[0];
         l = 1;
         fw = fl = 0;
         lead[0] = 0;
         tail[0] = 0;
         pr = 0;
         dp = 0;
         cs = 0;
         goto scopy;
      }
      ++f;
   }
endfmt:

   if (!callback)
      *bf = 0;
   else
      linearstbsp__flush_cb();

done:
   return tlen + (int)(bf - buf);
}

// cleanup
#undef LINEARSTBSP__LEFTJUST
#undef LINEARSTBSP__LEADINGPLUS
#undef LINEARSTBSP__LEADINGSPACE
#undef LINEARSTBSP__LEADING_0X
#undef LINEARSTBSP__LEADINGZERO
#undef LINEARSTBSP__INTMAX
#undef LINEARSTBSP__TRIPLET_COMMA
#undef LINEARSTBSP__NEGATIVE
#undef LINEARSTBSP__METRIC_SUFFIX
#undef LINEARSTBSP__NUMSZ
#undef linearstbsp__chk_cb_bufL
#undef linearstbsp__chk_cb_buf
#undef linearstbsp__flush_cb
#undef linearstbsp__cb_buf_clamp

// ============================================================================
//   wrapper functions

LINEARSTBSP__PUBLICDEF int LINEARSTB_SPRINTF_DECORATE(sprintf)(char *buf, char const *fmt, ...)
{
   int result;
   va_list va;
   va_start(va, fmt);
   result = LINEARSTB_SPRINTF_DECORATE(vsprintfcb)(0, 0, buf, fmt, va);
   va_end(va);
   return result;
}

typedef struct linearstbsp__context {
   char *buf;
   int count;
   int length;
   char tmp[LINEARSTB_SPRINTF_MIN];
} linearstbsp__context;

static char *linearstbsp__clamp_callback(const char *buf, void *user, int len)
{
   linearstbsp__context *c = (linearstbsp__context *)user;
   c->length += len;

   if (len > c->count)
      len = c->count;

   if (len) {
      if (buf != c->buf) {
         const char *s, *se;
         char *d;
         d = c->buf;
         s = buf;
         se = buf + len;
         do {
            *d++ = *s++;
         } while (s < se);
      }
      c->buf += len;
      c->count -= len;
   }

   if (c->count <= 0)
      return c->tmp;
   return (c->count >= LINEARSTB_SPRINTF_MIN) ? c->buf : c->tmp; // go direct into buffer if you can
}

static char * linearstbsp__count_clamp_callback( const char * buf, void * user, int len )
{
   linearstbsp__context * c = (linearstbsp__context*)user;
   (void) sizeof(buf);

   c->length += len;
   return c->tmp; // go direct into buffer if you can
}

LINEARSTBSP__PUBLICDEF int LINEARSTB_SPRINTF_DECORATE( vsnprintf )( char * buf, int count, char const * fmt, va_list va )
{
   linearstbsp__context c;

   if ( (count == 0) && !buf )
   {
      c.length = 0;

      LINEARSTB_SPRINTF_DECORATE( vsprintfcb )( linearstbsp__count_clamp_callback, &c, c.tmp, fmt, va );
   }
   else
   {
      int l;

      c.buf = buf;
      c.count = count;
      c.length = 0;

      LINEARSTB_SPRINTF_DECORATE( vsprintfcb )( linearstbsp__clamp_callback, &c, linearstbsp__clamp_callback(0,&c,0), fmt, va );

      // zero-terminate
      l = (int)( c.buf - buf );
      if ( l >= count ) // should never be greater, only equal (or less) than count
         l = count - 1;
      buf[l] = 0;
   }

   return c.length;
}

LINEARSTBSP__PUBLICDEF int LINEARSTB_SPRINTF_DECORATE(snprintf)(char *buf, int count, char const *fmt, ...)
{
   int result;
   va_list va;
   va_start(va, fmt);

   result = LINEARSTB_SPRINTF_DECORATE(vsnprintf)(buf, count, fmt, va);
   va_end(va);

   return result;
}

LINEARSTBSP__PUBLICDEF int LINEARSTB_SPRINTF_DECORATE(vsprintf)(char *buf, char const *fmt, va_list va)
{
   return LINEARSTB_SPRINTF_DECORATE(vsprintfcb)(0, 0, buf, fmt, va);
}

// =======================================================================
//   low level float utility functions

#ifndef LINEARSTB_SPRINTF_NOFLOAT

// copies d to bits w/ strict aliasing (this compiles to nothing on /Ox)
#define LINEARSTBSP__COPYFP(dest, src)                   \
   {                                               \
      int cn;                                      \
      for (cn = 0; cn < 8; cn++)                   \
         ((char *)&dest)[cn] = ((char *)&src)[cn]; \
   }

// get float info
static linearstbsp__int32 linearstbsp__real_to_parts(linearstbsp__int64 *bits, linearstbsp__int32 *expo, double value)
{
   double d;
   linearstbsp__int64 b = 0;

   // load value and round at the frac_digits
   d = value;

   LINEARSTBSP__COPYFP(b, d);

   *bits = b & ((((linearstbsp__uint64)1) << 52) - 1);
   *expo = (linearstbsp__int32)(((b >> 52) & 2047) - 1023);

   return (linearstbsp__int32)((linearstbsp__uint64) b >> 63);
}

static double const linearstbsp__bot[23] = {
   1e+000, 1e+001, 1e+002, 1e+003, 1e+004, 1e+005, 1e+006, 1e+007, 1e+008, 1e+009, 1e+010, 1e+011,
   1e+012, 1e+013, 1e+014, 1e+015, 1e+016, 1e+017, 1e+018, 1e+019, 1e+020, 1e+021, 1e+022
};
static double const linearstbsp__negbot[22] = {
   1e-001, 1e-002, 1e-003, 1e-004, 1e-005, 1e-006, 1e-007, 1e-008, 1e-009, 1e-010, 1e-011,
   1e-012, 1e-013, 1e-014, 1e-015, 1e-016, 1e-017, 1e-018, 1e-019, 1e-020, 1e-021, 1e-022
};
static double const linearstbsp__negboterr[22] = {
   -5.551115123125783e-018,  -2.0816681711721684e-019, -2.0816681711721686e-020, -4.7921736023859299e-021, -8.1803053914031305e-022, 4.5251888174113741e-023,
   4.5251888174113739e-024,  -2.0922560830128471e-025, -6.2281591457779853e-026, -3.6432197315497743e-027, 6.0503030718060191e-028,  2.0113352370744385e-029,
   -3.0373745563400371e-030, 1.1806906454401013e-032,  -7.7705399876661076e-032, 2.0902213275965398e-033,  -7.1542424054621921e-034, -7.1542424054621926e-035,
   2.4754073164739869e-036,  5.4846728545790429e-037,  9.2462547772103625e-038,  -4.8596774326570872e-039
};
static double const linearstbsp__top[13] = {
   1e+023, 1e+046, 1e+069, 1e+092, 1e+115, 1e+138, 1e+161, 1e+184, 1e+207, 1e+230, 1e+253, 1e+276, 1e+299
};
static double const linearstbsp__negtop[13] = {
   1e-023, 1e-046, 1e-069, 1e-092, 1e-115, 1e-138, 1e-161, 1e-184, 1e-207, 1e-230, 1e-253, 1e-276, 1e-299
};
static double const linearstbsp__toperr[13] = {
   8388608,
   6.8601809640529717e+028,
   -7.253143638152921e+052,
   -4.3377296974619174e+075,
   -1.5559416129466825e+098,
   -3.2841562489204913e+121,
   -3.7745893248228135e+144,
   -1.7356668416969134e+167,
   -3.8893577551088374e+190,
   -9.9566444326005119e+213,
   6.3641293062232429e+236,
   -5.2069140800249813e+259,
   -5.2504760255204387e+282
};
static double const linearstbsp__negtoperr[13] = {
   3.9565301985100693e-040,  -2.299904345391321e-063,  3.6506201437945798e-086,  1.1875228833981544e-109,
   -5.0644902316928607e-132, -6.7156837247865426e-155, -2.812077463003139e-178,  -5.7778912386589953e-201,
   7.4997100559334532e-224,  -4.6439668915134491e-247, -6.3691100762962136e-270, -9.436808465446358e-293,
   8.0970921678014997e-317
};

#if defined(_MSC_VER) && (_MSC_VER <= 1200)
static linearstbsp__uint64 const linearstbsp__powten[20] = {
   1,
   10,
   100,
   1000,
   10000,
   100000,
   1000000,
   10000000,
   100000000,
   1000000000,
   10000000000,
   100000000000,
   1000000000000,
   10000000000000,
   100000000000000,
   1000000000000000,
   10000000000000000,
   100000000000000000,
   1000000000000000000,
   10000000000000000000U
};
#define linearstbsp__tento19th ((linearstbsp__uint64)1000000000000000000)
#else
static linearstbsp__uint64 const linearstbsp__powten[20] = {
   1,
   10,
   100,
   1000,
   10000,
   100000,
   1000000,
   10000000,
   100000000,
   1000000000,
   10000000000ULL,
   100000000000ULL,
   1000000000000ULL,
   10000000000000ULL,
   100000000000000ULL,
   1000000000000000ULL,
   10000000000000000ULL,
   100000000000000000ULL,
   1000000000000000000ULL,
   10000000000000000000ULL
};
#define linearstbsp__tento19th (1000000000000000000ULL)
#endif

#define linearstbsp__ddmulthi(oh, ol, xh, yh)                            \
   {                                                               \
      double ahi = 0, alo, bhi = 0, blo;                           \
      linearstbsp__int64 bt;                                             \
      oh = xh * yh;                                                \
      LINEARSTBSP__COPYFP(bt, xh);                                       \
      bt &= ((~(linearstbsp__uint64)0) << 27);                           \
      LINEARSTBSP__COPYFP(ahi, bt);                                      \
      alo = xh - ahi;                                              \
      LINEARSTBSP__COPYFP(bt, yh);                                       \
      bt &= ((~(linearstbsp__uint64)0) << 27);                           \
      LINEARSTBSP__COPYFP(bhi, bt);                                      \
      blo = yh - bhi;                                              \
      ol = ((ahi * bhi - oh) + ahi * blo + alo * bhi) + alo * blo; \
   }

#define linearstbsp__ddtoS64(ob, xh, xl)          \
   {                                        \
      double ahi = 0, alo, vh, t;           \
      ob = (linearstbsp__int64)xh;                \
      vh = (double)ob;                      \
      ahi = (xh - vh);                      \
      t = (ahi - xh);                       \
      alo = (xh - (ahi - t)) - (vh + t);    \
      ob += (linearstbsp__int64)(ahi + alo + xl); \
   }

#define linearstbsp__ddrenorm(oh, ol) \
   {                            \
      double s;                 \
      s = oh + ol;              \
      ol = ol - (s - oh);       \
      oh = s;                   \
   }

#define linearstbsp__ddmultlo(oh, ol, xh, xl, yh, yl) ol = ol + (xh * yl + xl * yh);

#define linearstbsp__ddmultlos(oh, ol, xh, yl) ol = ol + (xh * yl);

static void linearstbsp__raise_to_power10(double *ohi, double *olo, double d, linearstbsp__int32 power) // power can be -323 to +350
{
   double ph, pl;
   if ((power >= 0) && (power <= 22)) {
      linearstbsp__ddmulthi(ph, pl, d, linearstbsp__bot[power]);
   } else {
      linearstbsp__int32 e, et, eb;
      double p2h, p2l;

      e = power;
      if (power < 0)
         e = -e;
      et = (e * 0x2c9) >> 14; /* %23 */
      if (et > 13)
         et = 13;
      eb = e - (et * 23);

      ph = d;
      pl = 0.0;
      if (power < 0) {
         if (eb) {
            --eb;
            linearstbsp__ddmulthi(ph, pl, d, linearstbsp__negbot[eb]);
            linearstbsp__ddmultlos(ph, pl, d, linearstbsp__negboterr[eb]);
         }
         if (et) {
            linearstbsp__ddrenorm(ph, pl);
            --et;
            linearstbsp__ddmulthi(p2h, p2l, ph, linearstbsp__negtop[et]);
            linearstbsp__ddmultlo(p2h, p2l, ph, pl, linearstbsp__negtop[et], linearstbsp__negtoperr[et]);
            ph = p2h;
            pl = p2l;
         }
      } else {
         if (eb) {
            e = eb;
            if (eb > 22)
               eb = 22;
            e -= eb;
            linearstbsp__ddmulthi(ph, pl, d, linearstbsp__bot[eb]);
            if (e) {
               linearstbsp__ddrenorm(ph, pl);
               linearstbsp__ddmulthi(p2h, p2l, ph, linearstbsp__bot[e]);
               linearstbsp__ddmultlos(p2h, p2l, linearstbsp__bot[e], pl);
               ph = p2h;
               pl = p2l;
            }
         }
         if (et) {
            linearstbsp__ddrenorm(ph, pl);
            --et;
            linearstbsp__ddmulthi(p2h, p2l, ph, linearstbsp__top[et]);
            linearstbsp__ddmultlo(p2h, p2l, ph, pl, linearstbsp__top[et], linearstbsp__toperr[et]);
            ph = p2h;
            pl = p2l;
         }
      }
   }
   linearstbsp__ddrenorm(ph, pl);
   *ohi = ph;
   *olo = pl;
}

// given a float value, returns the significant bits in bits, and the position of the
//   decimal point in decimal_pos.  +/-INF and NAN are specified by special values
//   returned in the decimal_pos parameter.
// frac_digits is absolute normally, but if you want from first significant digits (got %g and %e), or in 0x80000000
static linearstbsp__int32 linearstbsp__real_to_str(char const **start, linearstbsp__uint32 *len, char *out, linearstbsp__int32 *decimal_pos, double value, linearstbsp__uint32 frac_digits)
{
   double d;
   linearstbsp__int64 bits = 0;
   linearstbsp__int32 expo, e, ng, tens;

   d = value;
   LINEARSTBSP__COPYFP(bits, d);
   expo = (linearstbsp__int32)((bits >> 52) & 2047);
   ng = (linearstbsp__int32)((linearstbsp__uint64) bits >> 63);
   if (ng)
      d = -d;

   if (expo == 2047) // is nan or inf?
   {
      *start = (bits & ((((linearstbsp__uint64)1) << 52) - 1)) ? "NaN" : "Inf";
      *decimal_pos = LINEARSTBSP__SPECIAL;
      *len = 3;
      return ng;
   }

   if (expo == 0) // is zero or denormal
   {
      if (((linearstbsp__uint64) bits << 1) == 0) // do zero
      {
         *decimal_pos = 1;
         *start = out;
         out[0] = '0';
         *len = 1;
         return ng;
      }
      // find the right expo for denormals
      {
         linearstbsp__int64 v = ((linearstbsp__uint64)1) << 51;
         while ((bits & v) == 0) {
            --expo;
            v >>= 1;
         }
      }
   }

   // find the decimal exponent as well as the decimal bits of the value
   {
      double ph, pl;

      // log10 estimate - very specifically tweaked to hit or undershoot by no more than 1 of log10 of all expos 1..2046
      tens = expo - 1023;
      tens = (tens < 0) ? ((tens * 617) / 2048) : (((tens * 1233) / 4096) + 1);

      // move the significant bits into position and stick them into an int
      linearstbsp__raise_to_power10(&ph, &pl, d, 18 - tens);

      // get full as much precision from double-double as possible
      linearstbsp__ddtoS64(bits, ph, pl);

      // check if we undershot
      if (((linearstbsp__uint64)bits) >= linearstbsp__tento19th)
         ++tens;
   }

   // now do the rounding in integer land
   frac_digits = (frac_digits & 0x80000000) ? ((frac_digits & 0x7ffffff) + 1) : (tens + frac_digits);
   if ((frac_digits < 24)) {
      linearstbsp__uint32 dg = 1;
      if ((linearstbsp__uint64)bits >= linearstbsp__powten[9])
         dg = 10;
      while ((linearstbsp__uint64)bits >= linearstbsp__powten[dg]) {
         ++dg;
         if (dg == 20)
            goto noround;
      }
      if (frac_digits < dg) {
         linearstbsp__uint64 r;
         // add 0.5 at the right position and round
         e = dg - frac_digits;
         if ((linearstbsp__uint32)e >= 24)
            goto noround;
         r = linearstbsp__powten[e];
         bits = bits + (r / 2);
         if ((linearstbsp__uint64)bits >= linearstbsp__powten[dg])
            ++tens;
         bits /= r;
      }
   noround:;
   }

   // kill long trailing runs of zeros
   if (bits) {
      linearstbsp__uint32 n;
      for (;;) {
         if (bits <= 0xffffffff)
            break;
         if (bits % 1000)
            goto donez;
         bits /= 1000;
      }
      n = (linearstbsp__uint32)bits;
      while ((n % 1000) == 0)
         n /= 1000;
      bits = n;
   donez:;
   }

   // convert to string
   out += 64;
   e = 0;
   for (;;) {
      linearstbsp__uint32 n;
      char *o = out - 8;
      // do the conversion in chunks of U32s (avoid most 64-bit divides, worth it, constant denomiators be damned)
      if (bits >= 100000000) {
         n = (linearstbsp__uint32)(bits % 100000000);
         bits /= 100000000;
      } else {
         n = (linearstbsp__uint32)bits;
         bits = 0;
      }
      while (n) {
         out -= 2;
         *(linearstbsp__uint16 *)out = *(linearstbsp__uint16 *)&linearstbsp__digitpair.pair[(n % 100) * 2];
         n /= 100;
         e += 2;
      }
      if (bits == 0) {
         if ((e) && (out[0] == '0')) {
            ++out;
            --e;
         }
         break;
      }
      while (out != o) {
         *--out = '0';
         ++e;
      }
   }

   *decimal_pos = tens;
   *start = out;
   *len = e;
   return ng;
}

#undef linearstbsp__ddmulthi
#undef linearstbsp__ddrenorm
#undef linearstbsp__ddmultlo
#undef linearstbsp__ddmultlos
#undef LINEARSTBSP__SPECIAL
#undef LINEARSTBSP__COPYFP

#endif // LINEARSTB_SPRINTF_NOFLOAT

// clean up
#undef linearstbsp__uint16
#undef linearstbsp__uint32
#undef linearstbsp__int32
#undef linearstbsp__uint64
#undef linearstbsp__int64
#undef LINEARSTBSP__UNALIGNED

#endif // LINEARSTB_SPRINTF_IMPLEMENTATION

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

/* rblxstb_ds.h - v0.67 - public domain data structures - Sean Barrett 2019

   This is a single-header-file library that provides easy-to-use
   dynamic arrays and hash tables for C (also works in C++).

   For a gentle introduction:
      http://nothings.org/rblxstb_ds

   To use this library, do this in *one* C or C++ file:
      #define RBLXSTB_DS_IMPLEMENTATION
      #include "rblxstb_ds.h"

TABLE OF CONTENTS

  Table of Contents
  Compile-time options
  License
  Documentation
  Notes
  Notes - Dynamic arrays
  Notes - Hash maps
  Credits

COMPILE-TIME OPTIONS

  #define RBLXSTBDS_NO_SHORT_NAMES

     This flag needs to be set globally.

     By default rblxstb_ds exposes shorter function names that are not qualified
     with the "rblxstbds_" prefix. If these names conflict with the names in your
     code, define this flag.

  #define RBLXSTBDS_SIPHASH_2_4

     This flag only needs to be set in the file containing #define RBLXSTB_DS_IMPLEMENTATION.

     By default rblxstb_ds.h hashes using a weaker variant of SipHash and a custom hash for
     4- and 8-byte keys. On 64-bit platforms, you can define the above flag to force
     rblxstb_ds.h to use specification-compliant SipHash-2-4 for all keys. Doing so makes
     hash table insertion about 20% slower on 4- and 8-byte keys, 5% slower on
     64-byte keys, and 10% slower on 256-byte keys on my test computer.

  #define RBLXSTBDS_REALLOC(context,ptr,size) better_realloc
  #define RBLXSTBDS_FREE(context,ptr)         better_free

     These defines only need to be set in the file containing #define RBLXSTB_DS_IMPLEMENTATION.

     By default rblxstb_ds uses stdlib realloc() and free() for memory management. You can
     substitute your own functions instead by defining these symbols. You must either
     define both, or neither. Note that at the moment, 'context' will always be NULL.
     @TODO add an array/hash initialization function that takes a memory context pointer.

  #define RBLXSTBDS_UNIT_TESTS

     Defines a function rblxstbds_unit_tests() that checks the functioning of the data structures.

  Note that on older versions of gcc (e.g. 5.x.x) you may need to build with '-std=c++0x'
     (or equivalentally '-std=c++11') when using anonymous structures as seen on the web
     page or in RBLXSTBDS_UNIT_TESTS.

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

DOCUMENTATION

  Dynamic Arrays

    Non-function interface:

      Declare an empty dynamic array of type T
        T* foo = NULL;

      Access the i'th item of a dynamic array 'foo' of type T, T* foo:
        foo[i]

    Functions (actually macros)

      arrfree:
        void arrfree(T*);
          Frees the array.

      arrlen:
        ptrdiff_t arrlen(T*);
          Returns the number of elements in the array.

      arrlenu:
        size_t arrlenu(T*);
          Returns the number of elements in the array as an unsigned type.

      arrpop:
        T arrpop(T* a)
          Removes the final element of the array and returns it.

      arrput:
        T arrput(T* a, T b);
          Appends the item b to the end of array a. Returns b.

      arrins:
        T arrins(T* a, int p, T b);
          Inserts the item b into the middle of array a, into a[p],
          moving the rest of the array over. Returns b.

      arrinsn:
        void arrinsn(T* a, int p, int n);
          Inserts n uninitialized items into array a starting at a[p],
          moving the rest of the array over.

      arraddnptr:
        T* arraddnptr(T* a, int n)
          Appends n uninitialized items onto array at the end.
          Returns a pointer to the first uninitialized item added.

      arraddnindex:
        size_t arraddnindex(T* a, int n)
          Appends n uninitialized items onto array at the end.
          Returns the index of the first uninitialized item added.

      arrdel:
        void arrdel(T* a, int p);
          Deletes the element at a[p], moving the rest of the array over.

      arrdeln:
        void arrdeln(T* a, int p, int n);
          Deletes n elements starting at a[p], moving the rest of the array over.

      arrdelswap:
        void arrdelswap(T* a, int p);
          Deletes the element at a[p], replacing it with the element from
          the end of the array. O(1) performance.

      arrsetlen:
        void arrsetlen(T* a, int n);
          Changes the length of the array to n. Allocates uninitialized
          slots at the end if necessary.

      arrsetcap:
        size_t arrsetcap(T* a, int n);
          Sets the length of allocated storage to at least n. It will not
          change the length of the array.

      arrcap:
        size_t arrcap(T* a);
          Returns the number of total elements the array can contain without
          needing to be reallocated.

  Hash maps & String hash maps

    Given T is a structure type: struct { TK key; TV value; }. Note that some
    functions do not require TV value and can have other fields. For string
    hash maps, TK must be 'char *'.

    Special interface:

      rblxstbds_rand_seed:
        void rblxstbds_rand_seed(size_t seed);
          For security against adversarially chosen data, you should seed the
          library with a strong random number. Or at least seed it with time().

      rblxstbds_hash_string:
        size_t rblxstbds_hash_string(char *str, size_t seed);
          Returns a hash value for a string.

      rblxstbds_hash_bytes:
        size_t rblxstbds_hash_bytes(void *p, size_t len, size_t seed);
          These functions hash an arbitrary number of bytes. The function
          uses a custom hash for 4- and 8-byte data, and a weakened version
          of SipHash for everything else. On 64-bit platforms you can get
          specification-compliant SipHash-2-4 on all data by defining
          RBLXSTBDS_SIPHASH_2_4, at a significant cost in speed.

    Non-function interface:

      Declare an empty hash map of type T
        T* foo = NULL;

      Access the i'th entry in a hash table T* foo:
        foo[i]

    Function interface (actually macros):

      hmfree
      shfree
        void hmfree(T*);
        void shfree(T*);
          Frees the hashmap and sets the pointer to NULL.

      hmlen
      shlen
        ptrdiff_t hmlen(T*)
        ptrdiff_t shlen(T*)
          Returns the number of elements in the hashmap.

      hmlenu
      shlenu
        size_t hmlenu(T*)
        size_t shlenu(T*)
          Returns the number of elements in the hashmap.

      hmgeti
      shgeti
      hmgeti_ts
        ptrdiff_t hmgeti(T*, TK key)
        ptrdiff_t shgeti(T*, char* key)
        ptrdiff_t hmgeti_ts(T*, TK key, ptrdiff_t tempvar)
          Returns the index in the hashmap which has the key 'key', or -1
          if the key is not present.

      hmget
      hmget_ts
      shget
        TV hmget(T*, TK key)
        TV shget(T*, char* key)
        TV hmget_ts(T*, TK key, ptrdiff_t tempvar)
          Returns the value corresponding to 'key' in the hashmap.
          The structure must have a 'value' field

      hmgets
      shgets
        T hmgets(T*, TK key)
        T shgets(T*, char* key)
          Returns the structure corresponding to 'key' in the hashmap.

      hmgetp
      shgetp
      hmgetp_ts
      hmgetp_null
      shgetp_null
        T* hmgetp(T*, TK key)
        T* shgetp(T*, char* key)
        T* hmgetp_ts(T*, TK key, ptrdiff_t tempvar)
        T* hmgetp_null(T*, TK key)
        T* shgetp_null(T*, char *key)
          Returns a pointer to the structure corresponding to 'key' in
          the hashmap. Functions ending in "_null" return NULL if the key
          is not present in the hashmap; the others return a pointer to a
          structure holding the default value (but not the searched-for key).

      hmdefault
      shdefault
        TV hmdefault(T*, TV value)
        TV shdefault(T*, TV value)
          Sets the default value for the hashmap, the value which will be
          returned by hmget/shget if the key is not present.

      hmdefaults
      shdefaults
        TV hmdefaults(T*, T item)
        TV shdefaults(T*, T item)
          Sets the default struct for the hashmap, the contents which will be
          returned by hmgets/shgets if the key is not present.

      hmput
      shput
        TV hmput(T*, TK key, TV value)
        TV shput(T*, char* key, TV value)
          Inserts a <key,value> pair into the hashmap. If the key is already
          present in the hashmap, updates its value.

      hmputs
      shputs
        T hmputs(T*, T item)
        T shputs(T*, T item)
          Inserts a struct with T.key into the hashmap. If the struct is already
          present in the hashmap, updates it.

      hmdel
      shdel
        int hmdel(T*, TK key)
        int shdel(T*, char* key)
          If 'key' is in the hashmap, deletes its entry and returns 1.
          Otherwise returns 0.

    Function interface (actually macros) for strings only:

      sh_new_strdup
        void sh_new_strdup(T*);
          Overwrites the existing pointer with a newly allocated
          string hashmap which will automatically allocate and free
          each string key using realloc/free

      sh_new_arena
        void sh_new_arena(T*);
          Overwrites the existing pointer with a newly allocated
          string hashmap which will automatically allocate each string
          key to a string arena. Every string key ever used by this
          hash table remains in the arena until the arena is freed.
          Additionally, any key which is deleted and reinserted will
          be allocated multiple times in the string arena.

NOTES

  * These data structures are realloc'd when they grow, and the macro
    "functions" write to the provided pointer. This means: (a) the pointer
    must be an lvalue, and (b) the pointer to the data structure is not
    stable, and you must maintain it the same as you would a realloc'd
    pointer. For example, if you pass a pointer to a dynamic array to a
    function which updates it, the function must return back the new
    pointer to the caller. This is the price of trying to do this in C.

  * The following are the only functions that are thread-safe on a single data
    structure, i.e. can be run in multiple threads simultaneously on the same
    data structure
        hmlen        shlen
        hmlenu       shlenu
        hmget_ts     shget_ts
        hmgeti_ts    shgeti_ts
        hmgets_ts    shgets_ts

  * You iterate over the contents of a dynamic array and a hashmap in exactly
    the same way, using arrlen/hmlen/shlen:

      for (i=0; i < arrlen(foo); ++i)
         ... foo[i] ...

  * All operations except arrins/arrdel are O(1) amortized, but individual
    operations can be slow, so these data structures may not be suitable
    for real time use. Dynamic arrays double in capacity as needed, so
    elements are copied an average of once. Hash tables double/halve
    their size as needed, with appropriate hysteresis to maintain O(1)
    performance.

NOTES - DYNAMIC ARRAY

  * If you know how long a dynamic array is going to be in advance, you can avoid
    extra memory allocations by using arrsetlen to allocate it to that length in
    advance and use foo[n] while filling it out, or arrsetcap to allocate the memory
    for that length and use arrput/arrpush as normal.

  * Unlike some other versions of the dynamic array, this version should
    be safe to use with strict-aliasing optimizations.

NOTES - HASH MAP

  * For compilers other than GCC and clang (e.g. Visual Studio), for hmput/hmget/hmdel
    and variants, the key must be an lvalue (so the macro can take the address of it).
    Extensions are used that eliminate this requirement if you're using C99 and later
    in GCC or clang, or if you're using C++ in GCC. But note that this can make your
    code less portable.

  * To test for presence of a key in a hashmap, just do 'hmgeti(foo,key) >= 0'.

  * The iteration order of your data in the hashmap is determined solely by the
    order of insertions and deletions. In particular, if you never delete, new
    keys are always added at the end of the array. This will be consistent
    across all platforms and versions of the library. However, you should not
    attempt to serialize the internal hash table, as the hash is not consistent
    between different platforms, and may change with future versions of the library.

  * Use sh_new_arena() for string hashmaps that you never delete from. Initialize
    with NULL if you're managing the memory for your strings, or your strings are
    never freed (at least until the hashmap is freed). Otherwise, use sh_new_strdup().
    @TODO: make an arena variant that garbage collects the strings with a trivial
    copy collector into a new arena whenever the table shrinks / rebuilds. Since
    current arena recommendation is to only use arena if it never deletes, then
    this can just replace current arena implementation.

  * If adversarial input is a serious concern and you're on a 64-bit platform,
    enable RBLXSTBDS_SIPHASH_2_4 (see the 'Compile-time options' section), and pass
    a strong random number to rblxstbds_rand_seed.

  * The default value for the hash table is stored in foo[-1], so if you
    use code like 'hmget(T,k)->value = 5' you can accidentally overwrite
    the value stored by hmdefault if 'k' is not present.

CREDITS

  Sean Barrett -- library, idea for dynamic array API/implementation
  Per Vognsen  -- idea for hash table API/implementation
  Rafael Sachetto -- arrpop()
  github:HeroicKatora -- arraddn() reworking

  Bugfixes:
    Andy Durdin
    Shane Liesegang
    Vinh Truong
    Andreas Molzer
    github:hashitaku
    github:srdjanstipic
    Macoy Madson
    Andreas Vennstrom
    Tobias Mansfield-Williams
*/

#ifdef RBLXSTBDS_UNIT_TESTS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef INCLUDE_STB_DS_H
#define INCLUDE_STB_DS_H

#include <stddef.h>
#include <string.h>

#ifndef RBLXSTBDS_NO_SHORT_NAMES
#define arrlen      rblxstbds_arrlen
#define arrlenu     rblxstbds_arrlenu
#define arrput      rblxstbds_arrput
#define arrpush     rblxstbds_arrput
#define arrpop      rblxstbds_arrpop
#define arrfree     rblxstbds_arrfree
#define arraddn     rblxstbds_arraddn // deprecated, use one of the following instead:
#define arraddnptr  rblxstbds_arraddnptr
#define arraddnindex rblxstbds_arraddnindex
#define arrsetlen   rblxstbds_arrsetlen
#define arrlast     rblxstbds_arrlast
#define arrins      rblxstbds_arrins
#define arrinsn     rblxstbds_arrinsn
#define arrdel      rblxstbds_arrdel
#define arrdeln     rblxstbds_arrdeln
#define arrdelswap  rblxstbds_arrdelswap
#define arrcap      rblxstbds_arrcap
#define arrsetcap   rblxstbds_arrsetcap

#define hmput       rblxstbds_hmput
#define hmputs      rblxstbds_hmputs
#define hmget       rblxstbds_hmget
#define hmget_ts    rblxstbds_hmget_ts
#define hmgets      rblxstbds_hmgets
#define hmgetp      rblxstbds_hmgetp
#define hmgetp_ts   rblxstbds_hmgetp_ts
#define hmgetp_null rblxstbds_hmgetp_null
#define hmgeti      rblxstbds_hmgeti
#define hmgeti_ts   rblxstbds_hmgeti_ts
#define hmdel       rblxstbds_hmdel
#define hmlen       rblxstbds_hmlen
#define hmlenu      rblxstbds_hmlenu
#define hmfree      rblxstbds_hmfree
#define hmdefault   rblxstbds_hmdefault
#define hmdefaults  rblxstbds_hmdefaults

#define shput       rblxstbds_shput
#define shputi      rblxstbds_shputi
#define shputs      rblxstbds_shputs
#define shget       rblxstbds_shget
#define shgeti      rblxstbds_shgeti
#define shgets      rblxstbds_shgets
#define shgetp      rblxstbds_shgetp
#define shgetp_null rblxstbds_shgetp_null
#define shdel       rblxstbds_shdel
#define shlen       rblxstbds_shlen
#define shlenu      rblxstbds_shlenu
#define shfree      rblxstbds_shfree
#define shdefault   rblxstbds_shdefault
#define shdefaults  rblxstbds_shdefaults
#define sh_new_arena  rblxstbds_sh_new_arena
#define sh_new_strdup rblxstbds_sh_new_strdup

#define stralloc    rblxstbds_stralloc
#define strreset    rblxstbds_strreset
#endif

#if defined(RBLXSTBDS_REALLOC) && !defined(RBLXSTBDS_FREE) || !defined(RBLXSTBDS_REALLOC) && defined(RBLXSTBDS_FREE)
#error "You must define both RBLXSTBDS_REALLOC and RBLXSTBDS_FREE, or neither."
#endif
#if !defined(RBLXSTBDS_REALLOC) && !defined(RBLXSTBDS_FREE)
#include <stdlib.h>
#define RBLXSTBDS_REALLOC(c,p,s) realloc(p,s)
#define RBLXSTBDS_FREE(c,p)      free(p)
#endif

#ifdef _MSC_VER
#define RBLXSTBDS_NOTUSED(v)  (void)(v)
#else
#define RBLXSTBDS_NOTUSED(v)  (void)sizeof(v)
#endif

#ifdef __cplusplus
extern "C" {
#endif

// for security against attackers, seed the library with a random number, at least time() but stronger is better
extern void rblxstbds_rand_seed(size_t seed);

// these are the hash functions used internally if you want to test them or use them for other purposes
extern size_t rblxstbds_hash_bytes(void *p, size_t len, size_t seed);
extern size_t rblxstbds_hash_string(char *str, size_t seed);

// this is a simple string arena allocator, initialize with e.g. 'rblxstbds_string_arena my_arena={0}'.
typedef struct rblxstbds_string_arena rblxstbds_string_arena;
extern char * rblxstbds_stralloc(rblxstbds_string_arena *a, char *str);
extern void   rblxstbds_strreset(rblxstbds_string_arena *a);

// have to #define RBLXSTBDS_UNIT_TESTS to call this
extern void rblxstbds_unit_tests(void);

///////////////
//
// Everything below here is implementation details
//

extern void * rblxstbds_arrgrowf(void *a, size_t elemsize, size_t addlen, size_t min_cap);
extern void   rblxstbds_arrfreef(void *a);
extern void   rblxstbds_hmfree_func(void *p, size_t elemsize);
extern void * rblxstbds_hmget_key(void *a, size_t elemsize, void *key, size_t keysize, int mode);
extern void * rblxstbds_hmget_key_ts(void *a, size_t elemsize, void *key, size_t keysize, ptrdiff_t *temp, int mode);
extern void * rblxstbds_hmput_default(void *a, size_t elemsize);
extern void * rblxstbds_hmput_key(void *a, size_t elemsize, void *key, size_t keysize, int mode);
extern void * rblxstbds_hmdel_key(void *a, size_t elemsize, void *key, size_t keysize, size_t keyoffset, int mode);
extern void * rblxstbds_shmode_func(size_t elemsize, int mode);

#ifdef __cplusplus
}
#endif

#if defined(__GNUC__) || defined(__clang__)
#define RBLXSTBDS_HAS_TYPEOF
#ifdef __cplusplus
//#define RBLXSTBDS_HAS_LITERAL_ARRAY  // this is currently broken for clang
#endif
#endif

#if !defined(__cplusplus)
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define RBLXSTBDS_HAS_LITERAL_ARRAY
#endif
#endif

// this macro takes the address of the argument, but on gcc/clang can accept rvalues
#if defined(RBLXSTBDS_HAS_LITERAL_ARRAY) && defined(RBLXSTBDS_HAS_TYPEOF)
  #if __clang__
  #define RBLXSTBDS_ADDRESSOF(typevar, value)     ((__typeof__(typevar)[1]){value}) // literal array decays to pointer to value
  #else
  #define RBLXSTBDS_ADDRESSOF(typevar, value)     ((typeof(typevar)[1]){value}) // literal array decays to pointer to value
  #endif
#else
#define RBLXSTBDS_ADDRESSOF(typevar, value)     &(value)
#endif

#define RBLXSTBDS_OFFSETOF(var,field)           ((char *) &(var)->field - (char *) (var))

#define rblxstbds_header(t)  ((rblxstbds_array_header *) (t) - 1)
#define rblxstbds_temp(t)    rblxstbds_header(t)->temp
#define rblxstbds_temp_key(t) (*(char **) rblxstbds_header(t)->hash_table)

#define rblxstbds_arrsetcap(a,n)   (rblxstbds_arrgrow(a,0,n))
#define rblxstbds_arrsetlen(a,n)   ((rblxstbds_arrcap(a) < (size_t) (n) ? rblxstbds_arrsetcap((a),(size_t)(n)),0 : 0), (a) ? rblxstbds_header(a)->length = (size_t) (n) : 0)
#define rblxstbds_arrcap(a)        ((a) ? rblxstbds_header(a)->capacity : 0)
#define rblxstbds_arrlen(a)        ((a) ? (ptrdiff_t) rblxstbds_header(a)->length : 0)
#define rblxstbds_arrlenu(a)       ((a) ?             rblxstbds_header(a)->length : 0)
#define rblxstbds_arrput(a,v)      (rblxstbds_arrmaybegrow(a,1), (a)[rblxstbds_header(a)->length++] = (v))
#define rblxstbds_arrpush          rblxstbds_arrput  // synonym
#define rblxstbds_arrpop(a)        (rblxstbds_header(a)->length--, (a)[rblxstbds_header(a)->length])
#define rblxstbds_arraddn(a,n)     ((void)(rblxstbds_arraddnindex(a, n)))    // deprecated, use one of the following instead:
#define rblxstbds_arraddnptr(a,n)  (rblxstbds_arrmaybegrow(a,n), (n) ? (rblxstbds_header(a)->length += (n), &(a)[rblxstbds_header(a)->length-(n)]) : (a))
#define rblxstbds_arraddnindex(a,n)(rblxstbds_arrmaybegrow(a,n), (n) ? (rblxstbds_header(a)->length += (n), rblxstbds_header(a)->length-(n)) : rblxstbds_arrlen(a))
#define rblxstbds_arraddnoff       rblxstbds_arraddnindex
#define rblxstbds_arrlast(a)       ((a)[rblxstbds_header(a)->length-1])
#define rblxstbds_arrfree(a)       ((void) ((a) ? RBLXSTBDS_FREE(NULL,rblxstbds_header(a)) : (void)0), (a)=NULL)
#define rblxstbds_arrdel(a,i)      rblxstbds_arrdeln(a,i,1)
#define rblxstbds_arrdeln(a,i,n)   (memmove(&(a)[i], &(a)[(i)+(n)], sizeof *(a) * (rblxstbds_header(a)->length-(n)-(i))), rblxstbds_header(a)->length -= (n))
#define rblxstbds_arrdelswap(a,i)  ((a)[i] = rblxstbds_arrlast(a), rblxstbds_header(a)->length -= 1)
#define rblxstbds_arrinsn(a,i,n)   (rblxstbds_arraddn((a),(n)), memmove(&(a)[(i)+(n)], &(a)[i], sizeof *(a) * (rblxstbds_header(a)->length-(n)-(i))))
#define rblxstbds_arrins(a,i,v)    (rblxstbds_arrinsn((a),(i),1), (a)[i]=(v))

#define rblxstbds_arrmaybegrow(a,n)  ((!(a) || rblxstbds_header(a)->length + (n) > rblxstbds_header(a)->capacity) \
                                  ? (rblxstbds_arrgrow(a,n,0),0) : 0)

#define rblxstbds_arrgrow(a,b,c)   ((a) = rblxstbds_arrgrowf_wrapper((a), sizeof *(a), (b), (c)))

#define rblxstbds_hmput(t, k, v) \
    ((t) = rblxstbds_hmput_key_wrapper((t), sizeof *(t), (void*) RBLXSTBDS_ADDRESSOF((t)->key, (k)), sizeof (t)->key, 0),   \
     (t)[rblxstbds_temp((t)-1)].key = (k),    \
     (t)[rblxstbds_temp((t)-1)].value = (v))

#define rblxstbds_hmputs(t, s) \
    ((t) = rblxstbds_hmput_key_wrapper((t), sizeof *(t), &(s).key, sizeof (s).key, RBLXSTBDS_HM_BINARY), \
     (t)[rblxstbds_temp((t)-1)] = (s))

#define rblxstbds_hmgeti(t,k) \
    ((t) = rblxstbds_hmget_key_wrapper((t), sizeof *(t), (void*) RBLXSTBDS_ADDRESSOF((t)->key, (k)), sizeof (t)->key, RBLXSTBDS_HM_BINARY), \
      rblxstbds_temp((t)-1))

#define rblxstbds_hmgeti_ts(t,k,temp) \
    ((t) = rblxstbds_hmget_key_ts_wrapper((t), sizeof *(t), (void*) RBLXSTBDS_ADDRESSOF((t)->key, (k)), sizeof (t)->key, &(temp), RBLXSTBDS_HM_BINARY), \
      (temp))

#define rblxstbds_hmgetp(t, k) \
    ((void) rblxstbds_hmgeti(t,k), &(t)[rblxstbds_temp((t)-1)])

#define rblxstbds_hmgetp_ts(t, k, temp) \
    ((void) rblxstbds_hmgeti_ts(t,k,temp), &(t)[temp])

#define rblxstbds_hmdel(t,k) \
    (((t) = rblxstbds_hmdel_key_wrapper((t),sizeof *(t), (void*) RBLXSTBDS_ADDRESSOF((t)->key, (k)), sizeof (t)->key, RBLXSTBDS_OFFSETOF((t),key), RBLXSTBDS_HM_BINARY)),(t)?rblxstbds_temp((t)-1):0)

#define rblxstbds_hmdefault(t, v) \
    ((t) = rblxstbds_hmput_default_wrapper((t), sizeof *(t)), (t)[-1].value = (v))

#define rblxstbds_hmdefaults(t, s) \
    ((t) = rblxstbds_hmput_default_wrapper((t), sizeof *(t)), (t)[-1] = (s))

#define rblxstbds_hmfree(p)        \
    ((void) ((p) != NULL ? rblxstbds_hmfree_func((p)-1,sizeof*(p)),0 : 0),(p)=NULL)

#define rblxstbds_hmgets(t, k)    (*rblxstbds_hmgetp(t,k))
#define rblxstbds_hmget(t, k)     (rblxstbds_hmgetp(t,k)->value)
#define rblxstbds_hmget_ts(t, k, temp)  (rblxstbds_hmgetp_ts(t,k,temp)->value)
#define rblxstbds_hmlen(t)        ((t) ? (ptrdiff_t) rblxstbds_header((t)-1)->length-1 : 0)
#define rblxstbds_hmlenu(t)       ((t) ?             rblxstbds_header((t)-1)->length-1 : 0)
#define rblxstbds_hmgetp_null(t,k)  (rblxstbds_hmgeti(t,k) == -1 ? NULL : &(t)[rblxstbds_temp((t)-1)])

#define rblxstbds_shput(t, k, v) \
    ((t) = rblxstbds_hmput_key_wrapper((t), sizeof *(t), (void*) (k), sizeof (t)->key, RBLXSTBDS_HM_STRING),   \
     (t)[rblxstbds_temp((t)-1)].value = (v))

#define rblxstbds_shputi(t, k, v) \
    ((t) = rblxstbds_hmput_key_wrapper((t), sizeof *(t), (void*) (k), sizeof (t)->key, RBLXSTBDS_HM_STRING),   \
     (t)[rblxstbds_temp((t)-1)].value = (v), rblxstbds_temp((t)-1))

#define rblxstbds_shputs(t, s) \
    ((t) = rblxstbds_hmput_key_wrapper((t), sizeof *(t), (void*) (s).key, sizeof (s).key, RBLXSTBDS_HM_STRING), \
     (t)[rblxstbds_temp((t)-1)] = (s), \
     (t)[rblxstbds_temp((t)-1)].key = rblxstbds_temp_key((t)-1)) // above line overwrites whole structure, so must rewrite key here if it was allocated internally

#define rblxstbds_pshput(t, p) \
    ((t) = rblxstbds_hmput_key_wrapper((t), sizeof *(t), (void*) (p)->key, sizeof (p)->key, RBLXSTBDS_HM_PTR_TO_STRING), \
     (t)[rblxstbds_temp((t)-1)] = (p))

#define rblxstbds_shgeti(t,k) \
     ((t) = rblxstbds_hmget_key_wrapper((t), sizeof *(t), (void*) (k), sizeof (t)->key, RBLXSTBDS_HM_STRING), \
      rblxstbds_temp((t)-1))

#define rblxstbds_pshgeti(t,k) \
     ((t) = rblxstbds_hmget_key_wrapper((t), sizeof *(t), (void*) (k), sizeof (*(t))->key, RBLXSTBDS_HM_PTR_TO_STRING), \
      rblxstbds_temp((t)-1))

#define rblxstbds_shgetp(t, k) \
    ((void) rblxstbds_shgeti(t,k), &(t)[rblxstbds_temp((t)-1)])

#define rblxstbds_pshget(t, k) \
    ((void) rblxstbds_pshgeti(t,k), (t)[rblxstbds_temp((t)-1)])

#define rblxstbds_shdel(t,k) \
    (((t) = rblxstbds_hmdel_key_wrapper((t),sizeof *(t), (void*) (k), sizeof (t)->key, RBLXSTBDS_OFFSETOF((t),key), RBLXSTBDS_HM_STRING)),(t)?rblxstbds_temp((t)-1):0)
#define rblxstbds_pshdel(t,k) \
    (((t) = rblxstbds_hmdel_key_wrapper((t),sizeof *(t), (void*) (k), sizeof (*(t))->key, RBLXSTBDS_OFFSETOF(*(t),key), RBLXSTBDS_HM_PTR_TO_STRING)),(t)?rblxstbds_temp((t)-1):0)

#define rblxstbds_sh_new_arena(t)  \
    ((t) = rblxstbds_shmode_func_wrapper(t, sizeof *(t), RBLXSTBDS_SH_ARENA))
#define rblxstbds_sh_new_strdup(t) \
    ((t) = rblxstbds_shmode_func_wrapper(t, sizeof *(t), RBLXSTBDS_SH_STRDUP))

#define rblxstbds_shdefault(t, v)  rblxstbds_hmdefault(t,v)
#define rblxstbds_shdefaults(t, s) rblxstbds_hmdefaults(t,s)

#define rblxstbds_shfree       rblxstbds_hmfree
#define rblxstbds_shlenu       rblxstbds_hmlenu

#define rblxstbds_shgets(t, k) (*rblxstbds_shgetp(t,k))
#define rblxstbds_shget(t, k)  (rblxstbds_shgetp(t,k)->value)
#define rblxstbds_shgetp_null(t,k)  (rblxstbds_shgeti(t,k) == -1 ? NULL : &(t)[rblxstbds_temp((t)-1)])
#define rblxstbds_shlen        rblxstbds_hmlen

typedef struct
{
  size_t      length;
  size_t      capacity;
  void      * hash_table;
  ptrdiff_t   temp;
} rblxstbds_array_header;

typedef struct rblxstbds_string_block
{
  struct rblxstbds_string_block *next;
  char storage[8];
} rblxstbds_string_block;

struct rblxstbds_string_arena
{
  rblxstbds_string_block *storage;
  size_t remaining;
  unsigned char block;
  unsigned char mode;  // this isn't used by the string arena itself
};

#define RBLXSTBDS_HM_BINARY         0
#define RBLXSTBDS_HM_STRING         1

enum
{
   RBLXSTBDS_SH_NONE,
   RBLXSTBDS_SH_DEFAULT,
   RBLXSTBDS_SH_STRDUP,
   RBLXSTBDS_SH_ARENA
};

#ifdef __cplusplus
// in C we use implicit assignment from these void*-returning functions to T*.
// in C++ these templates make the same code work
template<class T> static T * rblxstbds_arrgrowf_wrapper(T *a, size_t elemsize, size_t addlen, size_t min_cap) {
  return (T*)rblxstbds_arrgrowf((void *)a, elemsize, addlen, min_cap);
}
template<class T> static T * rblxstbds_hmget_key_wrapper(T *a, size_t elemsize, void *key, size_t keysize, int mode) {
  return (T*)rblxstbds_hmget_key((void*)a, elemsize, key, keysize, mode);
}
template<class T> static T * rblxstbds_hmget_key_ts_wrapper(T *a, size_t elemsize, void *key, size_t keysize, ptrdiff_t *temp, int mode) {
  return (T*)rblxstbds_hmget_key_ts((void*)a, elemsize, key, keysize, temp, mode);
}
template<class T> static T * rblxstbds_hmput_default_wrapper(T *a, size_t elemsize) {
  return (T*)rblxstbds_hmput_default((void *)a, elemsize);
}
template<class T> static T * rblxstbds_hmput_key_wrapper(T *a, size_t elemsize, void *key, size_t keysize, int mode) {
  return (T*)rblxstbds_hmput_key((void*)a, elemsize, key, keysize, mode);
}
template<class T> static T * rblxstbds_hmdel_key_wrapper(T *a, size_t elemsize, void *key, size_t keysize, size_t keyoffset, int mode){
  return (T*)rblxstbds_hmdel_key((void*)a, elemsize, key, keysize, keyoffset, mode);
}
template<class T> static T * rblxstbds_shmode_func_wrapper(T *, size_t elemsize, int mode) {
  return (T*)rblxstbds_shmode_func(elemsize, mode);
}
#else
#define rblxstbds_arrgrowf_wrapper            rblxstbds_arrgrowf
#define rblxstbds_hmget_key_wrapper           rblxstbds_hmget_key
#define rblxstbds_hmget_key_ts_wrapper        rblxstbds_hmget_key_ts
#define rblxstbds_hmput_default_wrapper       rblxstbds_hmput_default
#define rblxstbds_hmput_key_wrapper           rblxstbds_hmput_key
#define rblxstbds_hmdel_key_wrapper           rblxstbds_hmdel_key
#define rblxstbds_shmode_func_wrapper(t,e,m)  rblxstbds_shmode_func(e,m)
#endif

#endif // INCLUDE_STB_DS_H


//////////////////////////////////////////////////////////////////////////////
//
//   IMPLEMENTATION
//

#ifdef RBLXSTB_DS_IMPLEMENTATION
#include <assert.h>
#include <string.h>

#ifndef RBLXSTBDS_ASSERT
#define RBLXSTBDS_ASSERT_WAS_UNDEFINED
#define RBLXSTBDS_ASSERT(x)   ((void) 0)
#endif

#ifdef RBLXSTBDS_STATISTICS
#define RBLXSTBDS_STATS(x)   x
size_t rblxstbds_array_grow;
size_t rblxstbds_hash_grow;
size_t rblxstbds_hash_shrink;
size_t rblxstbds_hash_rebuild;
size_t rblxstbds_hash_probes;
size_t rblxstbds_hash_alloc;
size_t rblxstbds_rehash_probes;
size_t rblxstbds_rehash_items;
#else
#define RBLXSTBDS_STATS(x)
#endif

//
// rblxstbds_arr implementation
//

//int *prev_allocs[65536];
//int num_prev;

void *rblxstbds_arrgrowf(void *a, size_t elemsize, size_t addlen, size_t min_cap)
{
  rblxstbds_array_header temp={0}; // force debugging
  void *b;
  size_t min_len = rblxstbds_arrlen(a) + addlen;
  (void) sizeof(temp);

  // compute the minimum capacity needed
  if (min_len > min_cap)
    min_cap = min_len;

  if (min_cap <= rblxstbds_arrcap(a))
    return a;

  // increase needed capacity to guarantee O(1) amortized
  if (min_cap < 2 * rblxstbds_arrcap(a))
    min_cap = 2 * rblxstbds_arrcap(a);
  else if (min_cap < 4)
    min_cap = 4;

  //if (num_prev < 65536) if (a) prev_allocs[num_prev++] = (int *) ((char *) a+1);
  //if (num_prev == 2201)
  //  num_prev = num_prev;
  b = RBLXSTBDS_REALLOC(NULL, (a) ? rblxstbds_header(a) : 0, elemsize * min_cap + sizeof(rblxstbds_array_header));
  //if (num_prev < 65536) prev_allocs[num_prev++] = (int *) (char *) b;
  b = (char *) b + sizeof(rblxstbds_array_header);
  if (a == NULL) {
    rblxstbds_header(b)->length = 0;
    rblxstbds_header(b)->hash_table = 0;
    rblxstbds_header(b)->temp = 0;
  } else {
    RBLXSTBDS_STATS(++rblxstbds_array_grow);
  }
  rblxstbds_header(b)->capacity = min_cap;

  return b;
}

void rblxstbds_arrfreef(void *a)
{
  RBLXSTBDS_FREE(NULL, rblxstbds_header(a));
}

//
// rblxstbds_hm hash table implementation
//

#ifdef RBLXSTBDS_INTERNAL_SMALL_BUCKET
#define RBLXSTBDS_BUCKET_LENGTH      4
#else
#define RBLXSTBDS_BUCKET_LENGTH      8
#endif

#define RBLXSTBDS_BUCKET_SHIFT      (RBLXSTBDS_BUCKET_LENGTH == 8 ? 3 : 2)
#define RBLXSTBDS_BUCKET_MASK       (RBLXSTBDS_BUCKET_LENGTH-1)
#define RBLXSTBDS_CACHE_LINE_SIZE   64

#define RBLXSTBDS_ALIGN_FWD(n,a)   (((n) + (a) - 1) & ~((a)-1))

typedef struct
{
   size_t    hash [RBLXSTBDS_BUCKET_LENGTH];
   ptrdiff_t index[RBLXSTBDS_BUCKET_LENGTH];
} rblxstbds_hash_bucket; // in 32-bit, this is one 64-byte cache line; in 64-bit, each array is one 64-byte cache line

typedef struct
{
  char * temp_key; // this MUST be the first field of the hash table
  size_t slot_count;
  size_t used_count;
  size_t used_count_threshold;
  size_t used_count_shrink_threshold;
  size_t tombstone_count;
  size_t tombstone_count_threshold;
  size_t seed;
  size_t slot_count_log2;
  rblxstbds_string_arena string;
  rblxstbds_hash_bucket *storage; // not a separate allocation, just 64-byte aligned storage after this struct
} rblxstbds_hash_index;

#define RBLXSTBDS_INDEX_EMPTY    -1
#define RBLXSTBDS_INDEX_DELETED  -2
#define RBLXSTBDS_INDEX_IN_USE(x)  ((x) >= 0)

#define RBLXSTBDS_HASH_EMPTY      0
#define RBLXSTBDS_HASH_DELETED    1

static size_t rblxstbds_hash_seed=0x31415926;

void rblxstbds_rand_seed(size_t seed)
{
  rblxstbds_hash_seed = seed;
}

#define rblxstbds_load_32_or_64(var, temp, v32, v64_hi, v64_lo)                                          \
  temp = v64_lo ^ v32, temp <<= 16, temp <<= 16, temp >>= 16, temp >>= 16, /* discard if 32-bit */   \
  var = v64_hi, var <<= 16, var <<= 16,                                    /* discard if 32-bit */   \
  var ^= temp ^ v32

#define RBLXSTBDS_SIZE_T_BITS           ((sizeof (size_t)) * 8)

static size_t rblxstbds_probe_position(size_t hash, size_t slot_count, size_t slot_log2)
{
  size_t pos;
  RBLXSTBDS_NOTUSED(slot_log2);
  pos = hash & (slot_count-1);
  #ifdef RBLXSTBDS_INTERNAL_BUCKET_START
  pos &= ~RBLXSTBDS_BUCKET_MASK;
  #endif
  return pos;
}

static size_t rblxstbds_log2(size_t slot_count)
{
  size_t n=0;
  while (slot_count > 1) {
    slot_count >>= 1;
    ++n;
  }
  return n;
}

static rblxstbds_hash_index *rblxstbds_make_hash_index(size_t slot_count, rblxstbds_hash_index *ot)
{
  rblxstbds_hash_index *t;
  t = (rblxstbds_hash_index *) RBLXSTBDS_REALLOC(NULL,0,(slot_count >> RBLXSTBDS_BUCKET_SHIFT) * sizeof(rblxstbds_hash_bucket) + sizeof(rblxstbds_hash_index) + RBLXSTBDS_CACHE_LINE_SIZE-1);
  t->storage = (rblxstbds_hash_bucket *) RBLXSTBDS_ALIGN_FWD((size_t) (t+1), RBLXSTBDS_CACHE_LINE_SIZE);
  t->slot_count = slot_count;
  t->slot_count_log2 = rblxstbds_log2(slot_count);
  t->tombstone_count = 0;
  t->used_count = 0;

  #if 0 // A1
  t->used_count_threshold        = slot_count*12/16; // if 12/16th of table is occupied, grow
  t->tombstone_count_threshold   = slot_count* 2/16; // if tombstones are 2/16th of table, rebuild
  t->used_count_shrink_threshold = slot_count* 4/16; // if table is only 4/16th full, shrink
  #elif 1 // A2
  //t->used_count_threshold        = slot_count*12/16; // if 12/16th of table is occupied, grow
  //t->tombstone_count_threshold   = slot_count* 3/16; // if tombstones are 3/16th of table, rebuild
  //t->used_count_shrink_threshold = slot_count* 4/16; // if table is only 4/16th full, shrink

  // compute without overflowing
  t->used_count_threshold        = slot_count - (slot_count>>2);
  t->tombstone_count_threshold   = (slot_count>>3) + (slot_count>>4);
  t->used_count_shrink_threshold = slot_count >> 2;

  #elif 0 // B1
  t->used_count_threshold        = slot_count*13/16; // if 13/16th of table is occupied, grow
  t->tombstone_count_threshold   = slot_count* 2/16; // if tombstones are 2/16th of table, rebuild
  t->used_count_shrink_threshold = slot_count* 5/16; // if table is only 5/16th full, shrink
  #else // C1
  t->used_count_threshold        = slot_count*14/16; // if 14/16th of table is occupied, grow
  t->tombstone_count_threshold   = slot_count* 2/16; // if tombstones are 2/16th of table, rebuild
  t->used_count_shrink_threshold = slot_count* 6/16; // if table is only 6/16th full, shrink
  #endif
  // Following statistics were measured on a Core i7-6700 @ 4.00Ghz, compiled with clang 7.0.1 -O2
    // Note that the larger tables have high variance as they were run fewer times
  //     A1            A2          B1           C1
  //    0.10ms :     0.10ms :     0.10ms :     0.11ms :      2,000 inserts creating 2K table
  //    0.96ms :     0.95ms :     0.97ms :     1.04ms :     20,000 inserts creating 20K table
  //   14.48ms :    14.46ms :    10.63ms :    11.00ms :    200,000 inserts creating 200K table
  //  195.74ms :   196.35ms :   203.69ms :   214.92ms :  2,000,000 inserts creating 2M table
  // 2193.88ms :  2209.22ms :  2285.54ms :  2437.17ms : 20,000,000 inserts creating 20M table
  //   65.27ms :    53.77ms :    65.33ms :    65.47ms : 500,000 inserts & deletes in 2K table
  //   72.78ms :    62.45ms :    71.95ms :    72.85ms : 500,000 inserts & deletes in 20K table
  //   89.47ms :    77.72ms :    96.49ms :    96.75ms : 500,000 inserts & deletes in 200K table
  //   97.58ms :    98.14ms :    97.18ms :    97.53ms : 500,000 inserts & deletes in 2M table
  //  118.61ms :   119.62ms :   120.16ms :   118.86ms : 500,000 inserts & deletes in 20M table
  //  192.11ms :   194.39ms :   196.38ms :   195.73ms : 500,000 inserts & deletes in 200M table

  if (slot_count <= RBLXSTBDS_BUCKET_LENGTH)
    t->used_count_shrink_threshold = 0;
  // to avoid infinite loop, we need to guarantee that at least one slot is empty and will terminate probes
  RBLXSTBDS_ASSERT(t->used_count_threshold + t->tombstone_count_threshold < t->slot_count);
  RBLXSTBDS_STATS(++rblxstbds_hash_alloc);
  if (ot) {
    t->string = ot->string;
    // reuse old seed so we can reuse old hashes so below "copy out old data" doesn't do any hashing
    t->seed = ot->seed;
  } else {
    size_t a,b,temp;
    memset(&t->string, 0, sizeof(t->string));
    t->seed = rblxstbds_hash_seed;
    // LCG
    // in 32-bit, a =          2147001325   b =  715136305
    // in 64-bit, a = 2862933555777941757   b = 3037000493
    rblxstbds_load_32_or_64(a,temp, 2147001325, 0x27bb2ee6, 0x87b0b0fd);
    rblxstbds_load_32_or_64(b,temp,  715136305,          0, 0xb504f32d);
    rblxstbds_hash_seed = rblxstbds_hash_seed  * a + b;
  }

  {
    size_t i,j;
    for (i=0; i < slot_count >> RBLXSTBDS_BUCKET_SHIFT; ++i) {
      rblxstbds_hash_bucket *b = &t->storage[i];
      for (j=0; j < RBLXSTBDS_BUCKET_LENGTH; ++j)
        b->hash[j] = RBLXSTBDS_HASH_EMPTY;
      for (j=0; j < RBLXSTBDS_BUCKET_LENGTH; ++j)
        b->index[j] = RBLXSTBDS_INDEX_EMPTY;
    }
  }

  // copy out the old data, if any
  if (ot) {
    size_t i,j;
    t->used_count = ot->used_count;
    for (i=0; i < ot->slot_count >> RBLXSTBDS_BUCKET_SHIFT; ++i) {
      rblxstbds_hash_bucket *ob = &ot->storage[i];
      for (j=0; j < RBLXSTBDS_BUCKET_LENGTH; ++j) {
        if (RBLXSTBDS_INDEX_IN_USE(ob->index[j])) {
          size_t hash = ob->hash[j];
          size_t pos = rblxstbds_probe_position(hash, t->slot_count, t->slot_count_log2);
          size_t step = RBLXSTBDS_BUCKET_LENGTH;
          RBLXSTBDS_STATS(++rblxstbds_rehash_items);
          for (;;) {
            size_t limit,z;
            rblxstbds_hash_bucket *bucket;
            bucket = &t->storage[pos >> RBLXSTBDS_BUCKET_SHIFT];
            RBLXSTBDS_STATS(++rblxstbds_rehash_probes);

            for (z=pos & RBLXSTBDS_BUCKET_MASK; z < RBLXSTBDS_BUCKET_LENGTH; ++z) {
              if (bucket->hash[z] == 0) {
                bucket->hash[z] = hash;
                bucket->index[z] = ob->index[j];
                goto done;
              }
            }

            limit = pos & RBLXSTBDS_BUCKET_MASK;
            for (z = 0; z < limit; ++z) {
              if (bucket->hash[z] == 0) {
                bucket->hash[z] = hash;
                bucket->index[z] = ob->index[j];
                goto done;
              }
            }

            pos += step;                  // quadratic probing
            step += RBLXSTBDS_BUCKET_LENGTH;
            pos &= (t->slot_count-1);
          }
        }
       done:
        ;
      }
    }
  }

  return t;
}

#define RBLXSTBDS_ROTATE_LEFT(val, n)   (((val) << (n)) | ((val) >> (RBLXSTBDS_SIZE_T_BITS - (n))))
#define RBLXSTBDS_ROTATE_RIGHT(val, n)  (((val) >> (n)) | ((val) << (RBLXSTBDS_SIZE_T_BITS - (n))))

size_t rblxstbds_hash_string(char *str, size_t seed)
{
  size_t hash = seed;
  while (*str)
     hash = RBLXSTBDS_ROTATE_LEFT(hash, 9) + (unsigned char) *str++;

  // Thomas Wang 64-to-32 bit mix function, hopefully also works in 32 bits
  hash ^= seed;
  hash = (~hash) + (hash << 18);
  hash ^= hash ^ RBLXSTBDS_ROTATE_RIGHT(hash,31);
  hash = hash * 21;
  hash ^= hash ^ RBLXSTBDS_ROTATE_RIGHT(hash,11);
  hash += (hash << 6);
  hash ^= RBLXSTBDS_ROTATE_RIGHT(hash,22);
  return hash+seed;
}

#ifdef RBLXSTBDS_SIPHASH_2_4
#define RBLXSTBDS_SIPHASH_C_ROUNDS 2
#define RBLXSTBDS_SIPHASH_D_ROUNDS 4
typedef int RBLXSTBDS_SIPHASH_2_4_can_only_be_used_in_64_bit_builds[sizeof(size_t) == 8 ? 1 : -1];
#endif

#ifndef RBLXSTBDS_SIPHASH_C_ROUNDS
#define RBLXSTBDS_SIPHASH_C_ROUNDS 1
#endif
#ifndef RBLXSTBDS_SIPHASH_D_ROUNDS
#define RBLXSTBDS_SIPHASH_D_ROUNDS 1
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4127) // conditional expression is constant, for do..while(0) and sizeof()==
#endif

static size_t rblxstbds_siphash_bytes(void *p, size_t len, size_t seed)
{
  unsigned char *d = (unsigned char *) p;
  size_t i,j;
  size_t v0,v1,v2,v3, data;

  // hash that works on 32- or 64-bit registers without knowing which we have
  // (computes different results on 32-bit and 64-bit platform)
  // derived from siphash, but on 32-bit platforms very different as it uses 4 32-bit state not 4 64-bit
  v0 = ((((size_t) 0x736f6d65 << 16) << 16) + 0x70736575) ^  seed;
  v1 = ((((size_t) 0x646f7261 << 16) << 16) + 0x6e646f6d) ^ ~seed;
  v2 = ((((size_t) 0x6c796765 << 16) << 16) + 0x6e657261) ^  seed;
  v3 = ((((size_t) 0x74656462 << 16) << 16) + 0x79746573) ^ ~seed;

  #ifdef RBLXSTBDS_TEST_SIPHASH_2_4
  // hardcoded with key material in the siphash test vectors
  v0 ^= 0x0706050403020100ull ^  seed;
  v1 ^= 0x0f0e0d0c0b0a0908ull ^ ~seed;
  v2 ^= 0x0706050403020100ull ^  seed;
  v3 ^= 0x0f0e0d0c0b0a0908ull ^ ~seed;
  #endif

  #define RBLXSTBDS_SIPROUND() \
    do {                   \
      v0 += v1; v1 = RBLXSTBDS_ROTATE_LEFT(v1, 13);  v1 ^= v0; v0 = RBLXSTBDS_ROTATE_LEFT(v0,RBLXSTBDS_SIZE_T_BITS/2); \
      v2 += v3; v3 = RBLXSTBDS_ROTATE_LEFT(v3, 16);  v3 ^= v2;                                                 \
      v2 += v1; v1 = RBLXSTBDS_ROTATE_LEFT(v1, 17);  v1 ^= v2; v2 = RBLXSTBDS_ROTATE_LEFT(v2,RBLXSTBDS_SIZE_T_BITS/2); \
      v0 += v3; v3 = RBLXSTBDS_ROTATE_LEFT(v3, 21);  v3 ^= v0;                                                 \
    } while (0)

  for (i=0; i+sizeof(size_t) <= len; i += sizeof(size_t), d += sizeof(size_t)) {
    data = d[0] | (d[1] << 8) | (d[2] << 16) | (d[3] << 24);
    data |= (size_t) (d[4] | (d[5] << 8) | (d[6] << 16) | (d[7] << 24)) << 16 << 16; // discarded if size_t == 4

    v3 ^= data;
    for (j=0; j < RBLXSTBDS_SIPHASH_C_ROUNDS; ++j)
      RBLXSTBDS_SIPROUND();
    v0 ^= data;
  }
  data = len << (RBLXSTBDS_SIZE_T_BITS-8);
  switch (len - i) {
    case 7: data |= ((size_t) d[6] << 24) << 24; // fall through
    case 6: data |= ((size_t) d[5] << 20) << 20; // fall through
    case 5: data |= ((size_t) d[4] << 16) << 16; // fall through
    case 4: data |= (d[3] << 24); // fall through
    case 3: data |= (d[2] << 16); // fall through
    case 2: data |= (d[1] << 8); // fall through
    case 1: data |= d[0]; // fall through
    case 0: break;
  }
  v3 ^= data;
  for (j=0; j < RBLXSTBDS_SIPHASH_C_ROUNDS; ++j)
    RBLXSTBDS_SIPROUND();
  v0 ^= data;
  v2 ^= 0xff;
  for (j=0; j < RBLXSTBDS_SIPHASH_D_ROUNDS; ++j)
    RBLXSTBDS_SIPROUND();

#ifdef RBLXSTBDS_SIPHASH_2_4
  return v0^v1^v2^v3;
#else
  return v1^v2^v3; // slightly stronger since v0^v3 in above cancels out final round operation? I tweeted at the authors of SipHash about this but they didn't reply
#endif
}

size_t rblxstbds_hash_bytes(void *p, size_t len, size_t seed)
{
#ifdef RBLXSTBDS_SIPHASH_2_4
  return rblxstbds_siphash_bytes(p,len,seed);
#else
  unsigned char *d = (unsigned char *) p;

  if (len == 4) {
    unsigned int hash = d[0] | (d[1] << 8) | (d[2] << 16) | (d[3] << 24);
    #if 0
    // HASH32-A  Bob Jenkin's hash function w/o large constants
    hash ^= seed;
    hash -= (hash<<6);
    hash ^= (hash>>17);
    hash -= (hash<<9);
    hash ^= seed;
    hash ^= (hash<<4);
    hash -= (hash<<3);
    hash ^= (hash<<10);
    hash ^= (hash>>15);
    #elif 1
    // HASH32-BB  Bob Jenkin's presumably-accidental version of Thomas Wang hash with rotates turned into shifts.
    // Note that converting these back to rotates makes it run a lot slower, presumably due to collisions, so I'm
    // not really sure what's going on.
    hash ^= seed;
    hash = (hash ^ 61) ^ (hash >> 16);
    hash = hash + (hash << 3);
    hash = hash ^ (hash >> 4);
    hash = hash * 0x27d4eb2d;
    hash ^= seed;
    hash = hash ^ (hash >> 15);
    #else  // HASH32-C   -  Murmur3
    hash ^= seed;
    hash *= 0xcc9e2d51;
    hash = (hash << 17) | (hash >> 15);
    hash *= 0x1b873593;
    hash ^= seed;
    hash = (hash << 19) | (hash >> 13);
    hash = hash*5 + 0xe6546b64;
    hash ^= hash >> 16;
    hash *= 0x85ebca6b;
    hash ^= seed;
    hash ^= hash >> 13;
    hash *= 0xc2b2ae35;
    hash ^= hash >> 16;
    #endif
    // Following statistics were measured on a Core i7-6700 @ 4.00Ghz, compiled with clang 7.0.1 -O2
    // Note that the larger tables have high variance as they were run fewer times
    //  HASH32-A   //  HASH32-BB  //  HASH32-C
    //    0.10ms   //    0.10ms   //    0.10ms :      2,000 inserts creating 2K table
    //    0.96ms   //    0.95ms   //    0.99ms :     20,000 inserts creating 20K table
    //   14.69ms   //   14.43ms   //   14.97ms :    200,000 inserts creating 200K table
    //  199.99ms   //  195.36ms   //  202.05ms :  2,000,000 inserts creating 2M table
    // 2234.84ms   // 2187.74ms   // 2240.38ms : 20,000,000 inserts creating 20M table
    //   55.68ms   //   53.72ms   //   57.31ms : 500,000 inserts & deletes in 2K table
    //   63.43ms   //   61.99ms   //   65.73ms : 500,000 inserts & deletes in 20K table
    //   80.04ms   //   77.96ms   //   81.83ms : 500,000 inserts & deletes in 200K table
    //  100.42ms   //   97.40ms   //  102.39ms : 500,000 inserts & deletes in 2M table
    //  119.71ms   //  120.59ms   //  121.63ms : 500,000 inserts & deletes in 20M table
    //  185.28ms   //  195.15ms   //  187.74ms : 500,000 inserts & deletes in 200M table
    //   15.58ms   //   14.79ms   //   15.52ms : 200,000 inserts creating 200K table with varying key spacing

    return (((size_t) hash << 16 << 16) | hash) ^ seed;
  } else if (len == 8 && sizeof(size_t) == 8) {
    size_t hash = d[0] | (d[1] << 8) | (d[2] << 16) | (d[3] << 24);
    hash |= (size_t) (d[4] | (d[5] << 8) | (d[6] << 16) | (d[7] << 24)) << 16 << 16; // avoid warning if size_t == 4
    hash ^= seed;
    hash = (~hash) + (hash << 21);
    hash ^= RBLXSTBDS_ROTATE_RIGHT(hash,24);
    hash *= 265;
    hash ^= RBLXSTBDS_ROTATE_RIGHT(hash,14);
    hash ^= seed;
    hash *= 21;
    hash ^= RBLXSTBDS_ROTATE_RIGHT(hash,28);
    hash += (hash << 31);
    hash = (~hash) + (hash << 18);
    return hash;
  } else {
    return rblxstbds_siphash_bytes(p,len,seed);
  }
#endif
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif


static int rblxstbds_is_key_equal(void *a, size_t elemsize, void *key, size_t keysize, size_t keyoffset, int mode, size_t i)
{
  if (mode >= RBLXSTBDS_HM_STRING)
    return 0==strcmp((char *) key, * (char **) ((char *) a + elemsize*i + keyoffset));
  else
    return 0==memcmp(key, (char *) a + elemsize*i + keyoffset, keysize);
}

#define RBLXSTBDS_HASH_TO_ARR(x,elemsize) ((char*) (x) - (elemsize))
#define RBLXSTBDS_ARR_TO_HASH(x,elemsize) ((char*) (x) + (elemsize))

#define rblxstbds_hash_table(a)  ((rblxstbds_hash_index *) rblxstbds_header(a)->hash_table)

void rblxstbds_hmfree_func(void *a, size_t elemsize)
{
  if (a == NULL) return;
  if (rblxstbds_hash_table(a) != NULL) {
    if (rblxstbds_hash_table(a)->string.mode == RBLXSTBDS_SH_STRDUP) {
      size_t i;
      // skip 0th element, which is default
      for (i=1; i < rblxstbds_header(a)->length; ++i)
        RBLXSTBDS_FREE(NULL, *(char**) ((char *) a + elemsize*i));
    }
    rblxstbds_strreset(&rblxstbds_hash_table(a)->string);
  }
  RBLXSTBDS_FREE(NULL, rblxstbds_header(a)->hash_table);
  RBLXSTBDS_FREE(NULL, rblxstbds_header(a));
}

static ptrdiff_t rblxstbds_hm_find_slot(void *a, size_t elemsize, void *key, size_t keysize, size_t keyoffset, int mode)
{
  void *raw_a = RBLXSTBDS_HASH_TO_ARR(a,elemsize);
  rblxstbds_hash_index *table = rblxstbds_hash_table(raw_a);
  size_t hash = mode >= RBLXSTBDS_HM_STRING ? rblxstbds_hash_string((char*)key,table->seed) : rblxstbds_hash_bytes(key, keysize,table->seed);
  size_t step = RBLXSTBDS_BUCKET_LENGTH;
  size_t limit,i;
  size_t pos;
  rblxstbds_hash_bucket *bucket;

  if (hash < 2) hash += 2; // stored hash values are forbidden from being 0, so we can detect empty slots

  pos = rblxstbds_probe_position(hash, table->slot_count, table->slot_count_log2);

  for (;;) {
    RBLXSTBDS_STATS(++rblxstbds_hash_probes);
    bucket = &table->storage[pos >> RBLXSTBDS_BUCKET_SHIFT];

    // start searching from pos to end of bucket, this should help performance on small hash tables that fit in cache
    for (i=pos & RBLXSTBDS_BUCKET_MASK; i < RBLXSTBDS_BUCKET_LENGTH; ++i) {
      if (bucket->hash[i] == hash) {
        if (rblxstbds_is_key_equal(a, elemsize, key, keysize, keyoffset, mode, bucket->index[i])) {
          return (pos & ~RBLXSTBDS_BUCKET_MASK)+i;
        }
      } else if (bucket->hash[i] == RBLXSTBDS_HASH_EMPTY) {
        return -1;
      }
    }

    // search from beginning of bucket to pos
    limit = pos & RBLXSTBDS_BUCKET_MASK;
    for (i = 0; i < limit; ++i) {
      if (bucket->hash[i] == hash) {
        if (rblxstbds_is_key_equal(a, elemsize, key, keysize, keyoffset, mode, bucket->index[i])) {
          return (pos & ~RBLXSTBDS_BUCKET_MASK)+i;
        }
      } else if (bucket->hash[i] == RBLXSTBDS_HASH_EMPTY) {
        return -1;
      }
    }

    // quadratic probing
    pos += step;
    step += RBLXSTBDS_BUCKET_LENGTH;
    pos &= (table->slot_count-1);
  }
  /* NOTREACHED */
}

void * rblxstbds_hmget_key_ts(void *a, size_t elemsize, void *key, size_t keysize, ptrdiff_t *temp, int mode)
{
  size_t keyoffset = 0;
  if (a == NULL) {
    // make it non-empty so we can return a temp
    a = rblxstbds_arrgrowf(0, elemsize, 0, 1);
    rblxstbds_header(a)->length += 1;
    memset(a, 0, elemsize);
    *temp = RBLXSTBDS_INDEX_EMPTY;
    // adjust a to point after the default element
    return RBLXSTBDS_ARR_TO_HASH(a,elemsize);
  } else {
    rblxstbds_hash_index *table;
    void *raw_a = RBLXSTBDS_HASH_TO_ARR(a,elemsize);
    // adjust a to point to the default element
    table = (rblxstbds_hash_index *) rblxstbds_header(raw_a)->hash_table;
    if (table == 0) {
      *temp = -1;
    } else {
      ptrdiff_t slot = rblxstbds_hm_find_slot(a, elemsize, key, keysize, keyoffset, mode);
      if (slot < 0) {
        *temp = RBLXSTBDS_INDEX_EMPTY;
      } else {
        rblxstbds_hash_bucket *b = &table->storage[slot >> RBLXSTBDS_BUCKET_SHIFT];
        *temp = b->index[slot & RBLXSTBDS_BUCKET_MASK];
      }
    }
    return a;
  }
}

void * rblxstbds_hmget_key(void *a, size_t elemsize, void *key, size_t keysize, int mode)
{
  ptrdiff_t temp;
  void *p = rblxstbds_hmget_key_ts(a, elemsize, key, keysize, &temp, mode);
  rblxstbds_temp(RBLXSTBDS_HASH_TO_ARR(p,elemsize)) = temp;
  return p;
}

void * rblxstbds_hmput_default(void *a, size_t elemsize)
{
  // three cases:
  //   a is NULL <- allocate
  //   a has a hash table but no entries, because of shmode <- grow
  //   a has entries <- do nothing
  if (a == NULL || rblxstbds_header(RBLXSTBDS_HASH_TO_ARR(a,elemsize))->length == 0) {
    a = rblxstbds_arrgrowf(a ? RBLXSTBDS_HASH_TO_ARR(a,elemsize) : NULL, elemsize, 0, 1);
    rblxstbds_header(a)->length += 1;
    memset(a, 0, elemsize);
    a=RBLXSTBDS_ARR_TO_HASH(a,elemsize);
  }
  return a;
}

static char *rblxstbds_strdup(char *str);

void *rblxstbds_hmput_key(void *a, size_t elemsize, void *key, size_t keysize, int mode)
{
  size_t keyoffset=0;
  void *raw_a;
  rblxstbds_hash_index *table;

  if (a == NULL) {
    a = rblxstbds_arrgrowf(0, elemsize, 0, 1);
    memset(a, 0, elemsize);
    rblxstbds_header(a)->length += 1;
    // adjust a to point AFTER the default element
    a = RBLXSTBDS_ARR_TO_HASH(a,elemsize);
  }

  // adjust a to point to the default element
  raw_a = a;
  a = RBLXSTBDS_HASH_TO_ARR(a,elemsize);

  table = (rblxstbds_hash_index *) rblxstbds_header(a)->hash_table;

  if (table == NULL || table->used_count >= table->used_count_threshold) {
    rblxstbds_hash_index *nt;
    size_t slot_count;

    slot_count = (table == NULL) ? RBLXSTBDS_BUCKET_LENGTH : table->slot_count*2;
    nt = rblxstbds_make_hash_index(slot_count, table);
    if (table)
      RBLXSTBDS_FREE(NULL, table);
    else
      nt->string.mode = mode >= RBLXSTBDS_HM_STRING ? RBLXSTBDS_SH_DEFAULT : 0;
    rblxstbds_header(a)->hash_table = table = nt;
    RBLXSTBDS_STATS(++rblxstbds_hash_grow);
  }

  // we iterate hash table explicitly because we want to track if we saw a tombstone
  {
    size_t hash = mode >= RBLXSTBDS_HM_STRING ? rblxstbds_hash_string((char*)key,table->seed) : rblxstbds_hash_bytes(key, keysize,table->seed);
    size_t step = RBLXSTBDS_BUCKET_LENGTH;
    size_t pos;
    ptrdiff_t tombstone = -1;
    rblxstbds_hash_bucket *bucket;

    // stored hash values are forbidden from being 0, so we can detect empty slots to early out quickly
    if (hash < 2) hash += 2;

    pos = rblxstbds_probe_position(hash, table->slot_count, table->slot_count_log2);

    for (;;) {
      size_t limit, i;
      RBLXSTBDS_STATS(++rblxstbds_hash_probes);
      bucket = &table->storage[pos >> RBLXSTBDS_BUCKET_SHIFT];

      // start searching from pos to end of bucket
      for (i=pos & RBLXSTBDS_BUCKET_MASK; i < RBLXSTBDS_BUCKET_LENGTH; ++i) {
        if (bucket->hash[i] == hash) {
          if (rblxstbds_is_key_equal(raw_a, elemsize, key, keysize, keyoffset, mode, bucket->index[i])) {
            rblxstbds_temp(a) = bucket->index[i];
            if (mode >= RBLXSTBDS_HM_STRING)
              rblxstbds_temp_key(a) = * (char **) ((char *) raw_a + elemsize*bucket->index[i] + keyoffset);
            return RBLXSTBDS_ARR_TO_HASH(a,elemsize);
          }
        } else if (bucket->hash[i] == 0) {
          pos = (pos & ~RBLXSTBDS_BUCKET_MASK) + i;
          goto found_empty_slot;
        } else if (tombstone < 0) {
          if (bucket->index[i] == RBLXSTBDS_INDEX_DELETED)
            tombstone = (ptrdiff_t) ((pos & ~RBLXSTBDS_BUCKET_MASK) + i);
        }
      }

      // search from beginning of bucket to pos
      limit = pos & RBLXSTBDS_BUCKET_MASK;
      for (i = 0; i < limit; ++i) {
        if (bucket->hash[i] == hash) {
          if (rblxstbds_is_key_equal(raw_a, elemsize, key, keysize, keyoffset, mode, bucket->index[i])) {
            rblxstbds_temp(a) = bucket->index[i];
            return RBLXSTBDS_ARR_TO_HASH(a,elemsize);
          }
        } else if (bucket->hash[i] == 0) {
          pos = (pos & ~RBLXSTBDS_BUCKET_MASK) + i;
          goto found_empty_slot;
        } else if (tombstone < 0) {
          if (bucket->index[i] == RBLXSTBDS_INDEX_DELETED)
            tombstone = (ptrdiff_t) ((pos & ~RBLXSTBDS_BUCKET_MASK) + i);
        }
      }

      // quadratic probing
      pos += step;
      step += RBLXSTBDS_BUCKET_LENGTH;
      pos &= (table->slot_count-1);
    }
   found_empty_slot:
    if (tombstone >= 0) {
      pos = tombstone;
      --table->tombstone_count;
    }
    ++table->used_count;

    {
      ptrdiff_t i = (ptrdiff_t) rblxstbds_arrlen(a);
      // we want to do rblxstbds_arraddn(1), but we can't use the macros since we don't have something of the right type
      if ((size_t) i+1 > rblxstbds_arrcap(a))
        *(void **) &a = rblxstbds_arrgrowf(a, elemsize, 1, 0);
      raw_a = RBLXSTBDS_ARR_TO_HASH(a,elemsize);

      RBLXSTBDS_ASSERT((size_t) i+1 <= rblxstbds_arrcap(a));
      rblxstbds_header(a)->length = i+1;
      bucket = &table->storage[pos >> RBLXSTBDS_BUCKET_SHIFT];
      bucket->hash[pos & RBLXSTBDS_BUCKET_MASK] = hash;
      bucket->index[pos & RBLXSTBDS_BUCKET_MASK] = i-1;
      rblxstbds_temp(a) = i-1;

      switch (table->string.mode) {
         case RBLXSTBDS_SH_STRDUP:  rblxstbds_temp_key(a) = *(char **) ((char *) a + elemsize*i) = rblxstbds_strdup((char*) key); break;
         case RBLXSTBDS_SH_ARENA:   rblxstbds_temp_key(a) = *(char **) ((char *) a + elemsize*i) = rblxstbds_stralloc(&table->string, (char*)key); break;
         case RBLXSTBDS_SH_DEFAULT: rblxstbds_temp_key(a) = *(char **) ((char *) a + elemsize*i) = (char *) key; break;
         default:                memcpy((char *) a + elemsize*i, key, keysize); break;
      }
    }
    return RBLXSTBDS_ARR_TO_HASH(a,elemsize);
  }
}

void * rblxstbds_shmode_func(size_t elemsize, int mode)
{
  void *a = rblxstbds_arrgrowf(0, elemsize, 0, 1);
  rblxstbds_hash_index *h;
  memset(a, 0, elemsize);
  rblxstbds_header(a)->length = 1;
  rblxstbds_header(a)->hash_table = h = (rblxstbds_hash_index *) rblxstbds_make_hash_index(RBLXSTBDS_BUCKET_LENGTH, NULL);
  h->string.mode = (unsigned char) mode;
  return RBLXSTBDS_ARR_TO_HASH(a,elemsize);
}

void * rblxstbds_hmdel_key(void *a, size_t elemsize, void *key, size_t keysize, size_t keyoffset, int mode)
{
  if (a == NULL) {
    return 0;
  } else {
    rblxstbds_hash_index *table;
    void *raw_a = RBLXSTBDS_HASH_TO_ARR(a,elemsize);
    table = (rblxstbds_hash_index *) rblxstbds_header(raw_a)->hash_table;
    rblxstbds_temp(raw_a) = 0;
    if (table == 0) {
      return a;
    } else {
      ptrdiff_t slot;
      slot = rblxstbds_hm_find_slot(a, elemsize, key, keysize, keyoffset, mode);
      if (slot < 0)
        return a;
      else {
        rblxstbds_hash_bucket *b = &table->storage[slot >> RBLXSTBDS_BUCKET_SHIFT];
        int i = slot & RBLXSTBDS_BUCKET_MASK;
        ptrdiff_t old_index = b->index[i];
        ptrdiff_t final_index = (ptrdiff_t) rblxstbds_arrlen(raw_a)-1-1; // minus one for the raw_a vs a, and minus one for 'last'
        RBLXSTBDS_ASSERT(slot < (ptrdiff_t) table->slot_count);
        --table->used_count;
        ++table->tombstone_count;
        rblxstbds_temp(raw_a) = 1;
        RBLXSTBDS_ASSERT(table->used_count >= 0);
        //RBLXSTBDS_ASSERT(table->tombstone_count < table->slot_count/4);
        b->hash[i] = RBLXSTBDS_HASH_DELETED;
        b->index[i] = RBLXSTBDS_INDEX_DELETED;

        if (mode == RBLXSTBDS_HM_STRING && table->string.mode == RBLXSTBDS_SH_STRDUP)
          RBLXSTBDS_FREE(NULL, *(char**) ((char *) a+elemsize*old_index));

        // if indices are the same, memcpy is a no-op, but back-pointer-fixup will fail, so skip
        if (old_index != final_index) {
          // swap delete
          memmove((char*) a + elemsize*old_index, (char*) a + elemsize*final_index, elemsize);

          // now find the slot for the last element
          if (mode == RBLXSTBDS_HM_STRING)
            slot = rblxstbds_hm_find_slot(a, elemsize, *(char**) ((char *) a+elemsize*old_index + keyoffset), keysize, keyoffset, mode);
          else
            slot = rblxstbds_hm_find_slot(a, elemsize,  (char* ) a+elemsize*old_index + keyoffset, keysize, keyoffset, mode);
          RBLXSTBDS_ASSERT(slot >= 0);
          b = &table->storage[slot >> RBLXSTBDS_BUCKET_SHIFT];
          i = slot & RBLXSTBDS_BUCKET_MASK;
          RBLXSTBDS_ASSERT(b->index[i] == final_index);
          b->index[i] = old_index;
        }
        rblxstbds_header(raw_a)->length -= 1;

        if (table->used_count < table->used_count_shrink_threshold && table->slot_count > RBLXSTBDS_BUCKET_LENGTH) {
          rblxstbds_header(raw_a)->hash_table = rblxstbds_make_hash_index(table->slot_count>>1, table);
          RBLXSTBDS_FREE(NULL, table);
          RBLXSTBDS_STATS(++rblxstbds_hash_shrink);
        } else if (table->tombstone_count > table->tombstone_count_threshold) {
          rblxstbds_header(raw_a)->hash_table = rblxstbds_make_hash_index(table->slot_count   , table);
          RBLXSTBDS_FREE(NULL, table);
          RBLXSTBDS_STATS(++rblxstbds_hash_rebuild);
        }

        return a;
      }
    }
  }
  /* NOTREACHED */
}

static char *rblxstbds_strdup(char *str)
{
  // to keep replaceable allocator simple, we don't want to use strdup.
  // rolling our own also avoids problem of strdup vs _strdup
  size_t len = strlen(str)+1;
  char *p = (char*) RBLXSTBDS_REALLOC(NULL, 0, len);
  memmove(p, str, len);
  return p;
}

#ifndef RBLXSTBDS_STRING_ARENA_BLOCKSIZE_MIN
#define RBLXSTBDS_STRING_ARENA_BLOCKSIZE_MIN  512u
#endif
#ifndef RBLXSTBDS_STRING_ARENA_BLOCKSIZE_MAX
#define RBLXSTBDS_STRING_ARENA_BLOCKSIZE_MAX  (1u<<20)
#endif

char *rblxstbds_stralloc(rblxstbds_string_arena *a, char *str)
{
  char *p;
  size_t len = strlen(str)+1;
  if (len > a->remaining) {
    // compute the next blocksize
    size_t blocksize = a->block;

    // size is 512, 512, 1024, 1024, 2048, 2048, 4096, 4096, etc., so that
    // there are log(SIZE) allocations to free when we destroy the table
    blocksize = (size_t) (RBLXSTBDS_STRING_ARENA_BLOCKSIZE_MIN) << (blocksize>>1);

    // if size is under 1M, advance to next blocktype
    if (blocksize < (size_t)(RBLXSTBDS_STRING_ARENA_BLOCKSIZE_MAX))
      ++a->block;

    if (len > blocksize) {
      // if string is larger than blocksize, then just allocate the full size.
      // note that we still advance string_block so block size will continue
      // increasing, so e.g. if somebody only calls this with 1000-long strings,
      // eventually the arena will start doubling and handling those as well
      rblxstbds_string_block *sb = (rblxstbds_string_block *) RBLXSTBDS_REALLOC(NULL, 0, sizeof(*sb)-8 + len);
      memmove(sb->storage, str, len);
      if (a->storage) {
        // insert it after the first element, so that we don't waste the space there
        sb->next = a->storage->next;
        a->storage->next = sb;
      } else {
        sb->next = 0;
        a->storage = sb;
        a->remaining = 0; // this is redundant, but good for clarity
      }
      return sb->storage;
    } else {
      rblxstbds_string_block *sb = (rblxstbds_string_block *) RBLXSTBDS_REALLOC(NULL, 0, sizeof(*sb)-8 + blocksize);
      sb->next = a->storage;
      a->storage = sb;
      a->remaining = blocksize;
    }
  }

  RBLXSTBDS_ASSERT(len <= a->remaining);
  p = a->storage->storage + a->remaining - len;
  a->remaining -= len;
  memmove(p, str, len);
  return p;
}

void rblxstbds_strreset(rblxstbds_string_arena *a)
{
  rblxstbds_string_block *x,*y;
  x = a->storage;
  while (x) {
    y = x->next;
    RBLXSTBDS_FREE(NULL, x);
    x = y;
  }
  memset(a, 0, sizeof(*a));
}

#endif

//////////////////////////////////////////////////////////////////////////////
//
//   UNIT TESTS
//

#ifdef RBLXSTBDS_UNIT_TESTS
#include <stdio.h>
#ifdef RBLXSTBDS_ASSERT_WAS_UNDEFINED
#undef RBLXSTBDS_ASSERT
#endif
#ifndef RBLXSTBDS_ASSERT
#define RBLXSTBDS_ASSERT assert
#include <assert.h>
#endif

typedef struct { int key,b,c,d; } rblxstbds_struct;
typedef struct { int key[2],b,c,d; } rblxstbds_struct2;

static char buffer[256];
char *strkey(int n)
{
#if defined(_WIN32) && defined(__STDC_WANT_SECURE_LIB__)
   sprintf_s(buffer, sizeof(buffer), "test_%d", n);
#else
   sprintf(buffer, "test_%d", n);
#endif
   return buffer;
}

void rblxstbds_unit_tests(void)
{
#if defined(_MSC_VER) && _MSC_VER <= 1200 && defined(__cplusplus)
  // VC6 C++ doesn't like the template<> trick on unnamed structures, so do nothing!
  RBLXSTBDS_ASSERT(0);
#else
  const int testsize = 100000;
  const int testsize2 = testsize/20;
  int *arr=NULL;
  struct { int   key;        int value; }  *intmap  = NULL;
  struct { char *key;        int value; }  *strmap  = NULL, s;
  struct { rblxstbds_struct key; int value; }  *map     = NULL;
  rblxstbds_struct                             *map2    = NULL;
  rblxstbds_struct2                            *map3    = NULL;
  rblxstbds_string_arena                        sa      = { 0 };
  int key3[2] = { 1,2 };
  ptrdiff_t temp;

  int i,j;

  RBLXSTBDS_ASSERT(arrlen(arr)==0);
  for (i=0; i < 20000; i += 50) {
    for (j=0; j < i; ++j)
      arrpush(arr,j);
    arrfree(arr);
  }

  for (i=0; i < 4; ++i) {
    arrpush(arr,1); arrpush(arr,2); arrpush(arr,3); arrpush(arr,4);
    arrdel(arr,i);
    arrfree(arr);
    arrpush(arr,1); arrpush(arr,2); arrpush(arr,3); arrpush(arr,4);
    arrdelswap(arr,i);
    arrfree(arr);
  }

  for (i=0; i < 5; ++i) {
    arrpush(arr,1); arrpush(arr,2); arrpush(arr,3); arrpush(arr,4);
    rblxstbds_arrins(arr,i,5);
    RBLXSTBDS_ASSERT(arr[i] == 5);
    if (i < 4)
      RBLXSTBDS_ASSERT(arr[4] == 4);
    arrfree(arr);
  }

  i = 1;
  RBLXSTBDS_ASSERT(hmgeti(intmap,i) == -1);
  hmdefault(intmap, -2);
  RBLXSTBDS_ASSERT(hmgeti(intmap, i) == -1);
  RBLXSTBDS_ASSERT(hmget (intmap, i) == -2);
  for (i=0; i < testsize; i+=2)
    hmput(intmap, i, i*5);
  for (i=0; i < testsize; i+=1) {
    if (i & 1) RBLXSTBDS_ASSERT(hmget(intmap, i) == -2 );
    else       RBLXSTBDS_ASSERT(hmget(intmap, i) == i*5);
    if (i & 1) RBLXSTBDS_ASSERT(hmget_ts(intmap, i, temp) == -2 );
    else       RBLXSTBDS_ASSERT(hmget_ts(intmap, i, temp) == i*5);
  }
  for (i=0; i < testsize; i+=2)
    hmput(intmap, i, i*3);
  for (i=0; i < testsize; i+=1)
    if (i & 1) RBLXSTBDS_ASSERT(hmget(intmap, i) == -2 );
    else       RBLXSTBDS_ASSERT(hmget(intmap, i) == i*3);
  for (i=2; i < testsize; i+=4)
    hmdel(intmap, i); // delete half the entries
  for (i=0; i < testsize; i+=1)
    if (i & 3) RBLXSTBDS_ASSERT(hmget(intmap, i) == -2 );
    else       RBLXSTBDS_ASSERT(hmget(intmap, i) == i*3);
  for (i=0; i < testsize; i+=1)
    hmdel(intmap, i); // delete the rest of the entries
  for (i=0; i < testsize; i+=1)
    RBLXSTBDS_ASSERT(hmget(intmap, i) == -2 );
  hmfree(intmap);
  for (i=0; i < testsize; i+=2)
    hmput(intmap, i, i*3);
  hmfree(intmap);

  #if defined(__clang__) || defined(__GNUC__)
  #ifndef __cplusplus
  intmap = NULL;
  hmput(intmap, 15, 7);
  hmput(intmap, 11, 3);
  hmput(intmap,  9, 5);
  RBLXSTBDS_ASSERT(hmget(intmap, 9) == 5);
  RBLXSTBDS_ASSERT(hmget(intmap, 11) == 3);
  RBLXSTBDS_ASSERT(hmget(intmap, 15) == 7);
  #endif
  #endif

  for (i=0; i < testsize; ++i)
    stralloc(&sa, strkey(i));
  strreset(&sa);

  {
    s.key = "a", s.value = 1;
    shputs(strmap, s);
    RBLXSTBDS_ASSERT(*strmap[0].key == 'a');
    RBLXSTBDS_ASSERT(strmap[0].key == s.key);
    RBLXSTBDS_ASSERT(strmap[0].value == s.value);
    shfree(strmap);
  }

  {
    s.key = "a", s.value = 1;
    sh_new_strdup(strmap);
    shputs(strmap, s);
    RBLXSTBDS_ASSERT(*strmap[0].key == 'a');
    RBLXSTBDS_ASSERT(strmap[0].key != s.key);
    RBLXSTBDS_ASSERT(strmap[0].value == s.value);
    shfree(strmap);
  }

  {
    s.key = "a", s.value = 1;
    sh_new_arena(strmap);
    shputs(strmap, s);
    RBLXSTBDS_ASSERT(*strmap[0].key == 'a');
    RBLXSTBDS_ASSERT(strmap[0].key != s.key);
    RBLXSTBDS_ASSERT(strmap[0].value == s.value);
    shfree(strmap);
  }

  for (j=0; j < 2; ++j) {
    RBLXSTBDS_ASSERT(shgeti(strmap,"foo") == -1);
    if (j == 0)
      sh_new_strdup(strmap);
    else
      sh_new_arena(strmap);
    RBLXSTBDS_ASSERT(shgeti(strmap,"foo") == -1);
    shdefault(strmap, -2);
    RBLXSTBDS_ASSERT(shgeti(strmap,"foo") == -1);
    for (i=0; i < testsize; i+=2)
      shput(strmap, strkey(i), i*3);
    for (i=0; i < testsize; i+=1)
      if (i & 1) RBLXSTBDS_ASSERT(shget(strmap, strkey(i)) == -2 );
      else       RBLXSTBDS_ASSERT(shget(strmap, strkey(i)) == i*3);
    for (i=2; i < testsize; i+=4)
      shdel(strmap, strkey(i)); // delete half the entries
    for (i=0; i < testsize; i+=1)
      if (i & 3) RBLXSTBDS_ASSERT(shget(strmap, strkey(i)) == -2 );
      else       RBLXSTBDS_ASSERT(shget(strmap, strkey(i)) == i*3);
    for (i=0; i < testsize; i+=1)
      shdel(strmap, strkey(i)); // delete the rest of the entries
    for (i=0; i < testsize; i+=1)
      RBLXSTBDS_ASSERT(shget(strmap, strkey(i)) == -2 );
    shfree(strmap);
  }

  {
    struct { char *key; char value; } *hash = NULL;
    char name[4] = "jen";
    shput(hash, "bob"   , 'h');
    shput(hash, "sally" , 'e');
    shput(hash, "fred"  , 'l');
    shput(hash, "jen"   , 'x');
    shput(hash, "doug"  , 'o');

    shput(hash, name    , 'l');
    shfree(hash);
  }

  for (i=0; i < testsize; i += 2) {
    rblxstbds_struct s = { i,i*2,i*3,i*4 };
    hmput(map, s, i*5);
  }

  for (i=0; i < testsize; i += 1) {
    rblxstbds_struct s = { i,i*2,i*3  ,i*4 };
    rblxstbds_struct t = { i,i*2,i*3+1,i*4 };
    if (i & 1) RBLXSTBDS_ASSERT(hmget(map, s) == 0);
    else       RBLXSTBDS_ASSERT(hmget(map, s) == i*5);
    if (i & 1) RBLXSTBDS_ASSERT(hmget_ts(map, s, temp) == 0);
    else       RBLXSTBDS_ASSERT(hmget_ts(map, s, temp) == i*5);
    //RBLXSTBDS_ASSERT(hmget(map, t.key) == 0);
  }

  for (i=0; i < testsize; i += 2) {
    rblxstbds_struct s = { i,i*2,i*3,i*4 };
    hmputs(map2, s);
  }
  hmfree(map);

  for (i=0; i < testsize; i += 1) {
    rblxstbds_struct s = { i,i*2,i*3,i*4 };
    rblxstbds_struct t = { i,i*2,i*3+1,i*4 };
    if (i & 1) RBLXSTBDS_ASSERT(hmgets(map2, s.key).d == 0);
    else       RBLXSTBDS_ASSERT(hmgets(map2, s.key).d == i*4);
    //RBLXSTBDS_ASSERT(hmgetp(map2, t.key) == 0);
  }
  hmfree(map2);

  for (i=0; i < testsize; i += 2) {
    rblxstbds_struct2 s = { { i,i*2 }, i*3,i*4, i*5 };
    hmputs(map3, s);
  }
  for (i=0; i < testsize; i += 1) {
    rblxstbds_struct2 s = { { i,i*2}, i*3, i*4, i*5 };
    rblxstbds_struct2 t = { { i,i*2}, i*3+1, i*4, i*5 };
    if (i & 1) RBLXSTBDS_ASSERT(hmgets(map3, s.key).d == 0);
    else       RBLXSTBDS_ASSERT(hmgets(map3, s.key).d == i*5);
    //RBLXSTBDS_ASSERT(hmgetp(map3, t.key) == 0);
  }
#endif
}
#endif


/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2019 Sean Barrett
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

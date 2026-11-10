// check that rblxstb_truetype compiles with no rblxstb_rect_pack.h
#define RBLXSTB_TRUETYPE_IMPLEMENTATION
#include "rblxstb_truetype.h"

#define RBLXSTB_DS_IMPLEMENTATION
#include "rblxstb_ds.h"
#include <assert.h>

int main(int arg, char **argv)
{
   int i;
   int *arr = NULL;

   for (i=0; i < 1000000; ++i)
      arrput(arr, i);

   assert(arrlen(arr) == 1000000);
   for (i=0; i < 1000000; ++i)
      assert(arr[i] == i);

   arrfree(arr);
   arr = NULL;

   for (i=0; i < 1000; ++i)
      arrput(arr, 1000);
   assert(arrlen(arr) == 1000000);

   return 0;
}
#define WIN32_MEAN_AND_LEAN
#define WIN32_LEAN_AND_MEAN
//#include <windows.h>
#include <conio.h>
#define RBLXSTB_DEFINE
#ifndef _M_AMD64
#define RBLXSTB_NPTR
#endif
#define RBLXSTB_ONLY
#include "rblxstb.h"
//#include "rblxstb_file.h"

int count;
void c(int truth, const char *error)
{
   if (!truth) {
      fprintf(stderr, "Test failed: %s\n", error);
      ++count;
   }
}

char *expects(rblxstb_matcher *m, char *s, int result, int len, const char *str)
{
   int res2,len2=0;
   res2 = rblxstb_lex(m, s, &len2);
   c(result == res2 && len == len2, str);
   return s + len;
}

void test_lex(void)
{
   rblxstb_matcher *m = rblxstb_lex_matcher();
   //         tok_en5 .3 20.1 20. .20 .1
   char *s = (char*) "tok_en5.3 20.1 20. .20.1";

   rblxstb_lex_item(m, "[a-zA-Z_][a-zA-Z0-9_]*", 1   );
   rblxstb_lex_item(m, "[0-9]*\\.?[0-9]*"      , 2   );
   rblxstb_lex_item(m, "[\r\n\t ]+"            , 3   );
   rblxstb_lex_item(m, "."                     , -99 );
   s=expects(m,s,1,7, "rblxstb_lex 1");
   s=expects(m,s,2,2, "rblxstb_lex 2");
   s=expects(m,s,3,1, "rblxstb_lex 3");
   s=expects(m,s,2,4, "rblxstb_lex 4");
   s=expects(m,s,3,1, "rblxstb_lex 5");
   s=expects(m,s,2,3, "rblxstb_lex 6");
   s=expects(m,s,3,1, "rblxstb_lex 7");
   s=expects(m,s,2,3, "rblxstb_lex 8");
   s=expects(m,s,2,2, "rblxstb_lex 9");
   s=expects(m,s,0,0, "rblxstb_lex 10");
   rblxstb_matcher_free(m);
}

int main(int argc, char **argv)
{
#if 0
   char *p;
   p = (char*) "abcdefghijklmnopqrstuvwxyz";
   c(rblxstb_ischar('c', p), "rblxstb_ischar 1");
   c(rblxstb_ischar('x', p), "rblxstb_ischar 2");
   c(!rblxstb_ischar('#', p), "rblxstb_ischar 3");
   c(!rblxstb_ischar('X', p), "rblxstb_ischar 4");
   p = (char*) "0123456789";
   c(!rblxstb_ischar('c', p), "rblxstb_ischar 5");
   c(!rblxstb_ischar('x', p), "rblxstb_ischar 6");
   c(!rblxstb_ischar('#', p), "rblxstb_ischar 7");
   c(!rblxstb_ischar('X', p), "rblxstb_ischar 8");
   p = (char*) "#####";
   c(!rblxstb_ischar('c', p), "rblxstb_ischar a");
   c(!rblxstb_ischar('x', p), "rblxstb_ischar b");
   c(rblxstb_ischar('#', p), "rblxstb_ischar c");
   c(!rblxstb_ischar('X', p), "rblxstb_ischar d");
   p = (char*) "xXyY";
   c(!rblxstb_ischar('c', p), "rblxstb_ischar e");
   c(rblxstb_ischar('x', p), "rblxstb_ischar f");
   c(!rblxstb_ischar('#', p), "rblxstb_ischar g");
   c(rblxstb_ischar('X', p), "rblxstb_ischar h");
#endif

   test_lex();

   if (count) {
      _getch();
   }
   return 0;
}

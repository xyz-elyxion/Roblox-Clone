// I haven't actually tested this yet, this is just to make sure it compiles

#include <stdlib.h>
#include <string.h> // memmove
#include <ctype.h>  // isspace

#define RBLXSTB_TEXTEDIT_CHARTYPE   char
#define RBLXSTB_TEXTEDIT_STRING     text_control

// get the base type
#include "rblxstb_textedit.h"

// define our editor structure
typedef struct
{
   char *string;
   int stringlen;
   RBLXSTB_TexteditState state;
} text_control;

// define the functions we need
void layout_func(StbTexteditRow *row, RBLXSTB_TEXTEDIT_STRING *str, int start_i)
{
   int remaining_chars = str->stringlen - start_i;
   row->num_chars = remaining_chars > 20 ? 20 : remaining_chars; // should do real word wrap here
   row->x0 = 0;
   row->x1 = 20; // need to account for actual size of characters
   row->baseline_y_delta = 1.25;
   row->ymin = -1;
   row->ymax =  0;
}

int delete_chars(RBLXSTB_TEXTEDIT_STRING *str, int pos, int num)
{
   memmove(&str->string[pos], &str->string[pos+num], str->stringlen - (pos+num));
   str->stringlen -= num;
   return 1; // always succeeds
}

int insert_chars(RBLXSTB_TEXTEDIT_STRING *str, int pos, RBLXSTB_TEXTEDIT_CHARTYPE *newtext, int num)
{
   str->string = realloc(str->string, str->stringlen + num);
   memmove(&str->string[pos+num], &str->string[pos], str->stringlen - pos);
   memcpy(&str->string[pos], newtext, num);
   str->stringlen += num;
   return 1; // always succeeds
}

// define all the #defines needed 

#define KEYDOWN_BIT                    0x80000000

#define RBLXSTB_TEXTEDIT_STRINGLEN(tc)     ((tc)->stringlen)
#define RBLXSTB_TEXTEDIT_LAYOUTROW         layout_func
#define RBLXSTB_TEXTEDIT_GETWIDTH(tc,n,i)  (1) // quick hack for monospaced
#define RBLXSTB_TEXTEDIT_KEYTOTEXT(key)    (((key) & KEYDOWN_BIT) ? 0 : (key))
#define RBLXSTB_TEXTEDIT_GETCHAR(tc,i)     ((tc)->string[i])
#define RBLXSTB_TEXTEDIT_NEWLINE           '\n'
#define RBLXSTB_TEXTEDIT_IS_SPACE(ch)      isspace(ch)
#define RBLXSTB_TEXTEDIT_DELETECHARS       delete_chars
#define RBLXSTB_TEXTEDIT_INSERTCHARS       insert_chars

#define RBLXSTB_TEXTEDIT_K_SHIFT           0x40000000
#define RBLXSTB_TEXTEDIT_K_CONTROL         0x20000000
#define RBLXSTB_TEXTEDIT_K_LEFT            (KEYDOWN_BIT | 1) // actually use VK_LEFT, SDLK_LEFT, etc
#define RBLXSTB_TEXTEDIT_K_RIGHT           (KEYDOWN_BIT | 2) // VK_RIGHT
#define RBLXSTB_TEXTEDIT_K_UP              (KEYDOWN_BIT | 3) // VK_UP
#define RBLXSTB_TEXTEDIT_K_DOWN            (KEYDOWN_BIT | 4) // VK_DOWN
#define RBLXSTB_TEXTEDIT_K_LINESTART       (KEYDOWN_BIT | 5) // VK_HOME
#define RBLXSTB_TEXTEDIT_K_LINEEND         (KEYDOWN_BIT | 6) // VK_END
#define RBLXSTB_TEXTEDIT_K_TEXTSTART       (RBLXSTB_TEXTEDIT_K_LINESTART | RBLXSTB_TEXTEDIT_K_CONTROL)
#define RBLXSTB_TEXTEDIT_K_TEXTEND         (RBLXSTB_TEXTEDIT_K_LINEEND   | RBLXSTB_TEXTEDIT_K_CONTROL)
#define RBLXSTB_TEXTEDIT_K_DELETE          (KEYDOWN_BIT | 7) // VK_DELETE
#define RBLXSTB_TEXTEDIT_K_BACKSPACE       (KEYDOWN_BIT | 8) // VK_BACKSPACE
#define RBLXSTB_TEXTEDIT_K_UNDO            (KEYDOWN_BIT | RBLXSTB_TEXTEDIT_K_CONTROL | 'z')
#define RBLXSTB_TEXTEDIT_K_REDO            (KEYDOWN_BIT | RBLXSTB_TEXTEDIT_K_CONTROL | 'y')
#define RBLXSTB_TEXTEDIT_K_INSERT          (KEYDOWN_BIT | 9) // VK_INSERT
#define RBLXSTB_TEXTEDIT_K_WORDLEFT        (RBLXSTB_TEXTEDIT_K_LEFT  | RBLXSTB_TEXTEDIT_K_CONTROL)
#define RBLXSTB_TEXTEDIT_K_WORDRIGHT       (RBLXSTB_TEXTEDIT_K_RIGHT | RBLXSTB_TEXTEDIT_K_CONTROL)
#define RBLXSTB_TEXTEDIT_K_PGUP            (KEYDOWN_BIT | 10) // VK_PGUP -- not implemented
#define RBLXSTB_TEXTEDIT_K_PGDOWN          (KEYDOWN_BIT | 11) // VK_PGDOWN -- not implemented

#define RBLXSTB_TEXTEDIT_IMPLEMENTATION
#include "rblxstb_textedit.h"

void dummy3(void)
{
  rblxstb_textedit_click(0,0,0,0);
  rblxstb_textedit_drag(0,0,0,0);
  rblxstb_textedit_cut(0,0);
  rblxstb_textedit_key(0,0,0);
  rblxstb_textedit_initialize_state(0,0);
  rblxstb_textedit_paste(0,0,0,0);
}

#define RBLXSTB_IMAGE_WRITE_STATIC
#define RBLXSTBIWDEF static inline

#include "rblxstb_image.h"
#include "rblxstb_rect_pack.h"
#include "rblxstb_truetype.h"
#include "rblxstb_image_write.h"
#include "rblxstb_c_lexer.h"
#include "rblxstb_perlin.h"
#include "rblxstb_dxt.h"
#include "rblxstb_divide.h"
#include "rblxstb_herringbone_wang_tile.h"
#include "rblxstb_ds.h"
#include "rblxstb_hexwave.h"

#include "rblxstb_sprintf.h"
#define RBLXSTB_SPRINTF_IMPLEMENTATION
#include "rblxstb_sprintf.h"


#define RBLXSTB_IMAGE_WRITE_IMPLEMENTATION
#define RBLXSTB_TRUETYPE_IMPLEMENTATION
#define RBLXSTB_PERLIN_IMPLEMENTATION
#define RBLXSTB_DXT_IMPLEMENATION
#define RBLXSTB_C_LEXER_IMPLEMENTATIOn
#define RBLXSTB_DIVIDE_IMPLEMENTATION
#define RBLXSTB_IMAGE_IMPLEMENTATION
#define RBLXSTB_HERRINGBONE_WANG_TILE_IMPLEMENTATION
#define RBLXSTB_RECT_PACK_IMPLEMENTATION
#define RBLXSTB_VOXEL_RENDER_IMPLEMENTATION
#define RBLXSTB_CONNECTED_COMPONENTS_IMPLEMENTATION
#define RBLXSTB_HEXWAVE_IMPLEMENTATION
#define RBLXSTB_DS_IMPLEMENTATION
#define RBLXSTBDS_UNIT_TESTS

#define RBLXSTBI_MALLOC     my_malloc
#define RBLXSTBI_FREE       my_free
#define RBLXSTBI_REALLOC    my_realloc

void *my_malloc(size_t) { return 0; }
void *my_realloc(void *, size_t) { return 0; }
void my_free(void *) { }

#include "rblxstb_image.h"
#include "rblxstb_rect_pack.h"
#include "rblxstb_truetype.h"
#include "rblxstb_image_write.h"
#include "rblxstb_perlin.h"
#include "rblxstb_dxt.h"
#include "rblxstb_divide.h"
#include "rblxstb_herringbone_wang_tile.h"
#include "rblxstb_ds.h"
#include "rblxstb_hexwave.h"

#define RBLXSTBCC_GRID_COUNT_X_LOG2  10
#define RBLXSTBCC_GRID_COUNT_Y_LOG2  10
#include "rblxstb_connected_components.h"

#define RBLXSTBVOX_CONFIG_MODE 1
#include "rblxstb_voxel_render.h"

#define RBLXSTBTE_DRAW_RECT(x0,y0,x1,y1,color)      do ; while(0)
#define RBLXSTBTE_DRAW_TILE(x,y,id,highlight,data)  do ; while(0)
#define RBLXSTB_TILEMAP_EDITOR_IMPLEMENTATION
#include "rblxstb_tilemap_editor.h"

#include "rblxstb_easy_font.h"

#define RBLXSTB_LEAKCHECK_IMPLEMENTATION
#include "rblxstb_leakcheck.h"

#define RBLXSTB_IMAGE_RESIZE_IMPLEMENTATION
#include "rblxstb_image_resize2.h"

//#include "stretchy_buffer.h"  // deprecating


// avoid unused-function complaints
void dummy2(void)
{
   rblxstb_easy_font_spacing(1.0);
   rblxstb_easy_font_print(0,0,NULL,NULL,NULL,0);
   rblxstb_easy_font_width(NULL);
   rblxstb_easy_font_height(NULL);
}


////////////////////////////////////////////////////////////
//
// text edit

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
   str->string = (char *) realloc(str->string, str->stringlen + num);
   memmove(&str->string[pos+num], &str->string[pos], str->stringlen - pos);
   memcpy(&str->string[pos], newtext, num);
   str->stringlen += num;
   return 1; // always succeeds
}

// define all the #defines needed 

#define KEYDOWN_BIT                    0x40000000

#define RBLXSTB_TEXTEDIT_STRINGLEN(tc)     ((tc)->stringlen)
#define RBLXSTB_TEXTEDIT_LAYOUTROW         layout_func
#define RBLXSTB_TEXTEDIT_GETWIDTH(tc,n,i)  (1) // quick hack for monospaced
#define RBLXSTB_TEXTEDIT_KEYTOTEXT(key)    (((key) & KEYDOWN_BIT) ? 0 : (key))
#define RBLXSTB_TEXTEDIT_GETCHAR(tc,i)     ((tc)->string[i])
#define RBLXSTB_TEXTEDIT_NEWLINE           '\n'
#define RBLXSTB_TEXTEDIT_IS_SPACE(ch)      isspace(ch)
#define RBLXSTB_TEXTEDIT_DELETECHARS       delete_chars
#define RBLXSTB_TEXTEDIT_INSERTCHARS       insert_chars

#define RBLXSTB_TEXTEDIT_K_SHIFT           0x20000000
#define RBLXSTB_TEXTEDIT_K_CONTROL         0x10000000
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

#include "rblxstb_c_lexer.h"

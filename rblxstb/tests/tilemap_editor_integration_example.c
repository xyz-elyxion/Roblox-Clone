// This isn't compilable as-is, as it was extracted from a working
// integration-in-a-game and makes reference to symbols from that game.

#include <assert.h>
#include <ctype.h>
#include "game.h"
#include "SDL.h"
#include "rblxstb_tilemap_editor.h"

extern void editor_draw_tile(int x, int y, unsigned short tile, int mode, float *props);
extern void editor_draw_rect(int x0, int y0, int x1, int y1, unsigned char r, unsigned char g, unsigned char b);

static int is_platform(short *tiles);
static unsigned int prop_type(int n, short *tiles);
static char *prop_name(int n, short *tiles);
static float prop_range(int n, short *tiles, int is_max);
static int allow_link(short *src, short *dest);

#define RBLXSTBTE_MAX_PROPERTIES  8

#define RBLXSTBTE_PROP_TYPE(n, tiledata, p) prop_type(n,tiledata)
#define RBLXSTBTE_PROP_NAME(n, tiledata, p) prop_name(n,tiledata)
#define RBLXSTBTE_PROP_MIN(n, tiledata, p)  prop_range(n,tiledata,0)
#define RBLXSTBTE_PROP_MAX(n, tiledata, p)  prop_range(n,tiledata,1)
#define RBLXSTBTE_PROP_FLOAT_SCALE(n,td,p)  (0.1)

#define RBLXSTBTE_ALLOW_LINK(srctile, srcprop, desttile, destprop) \
           allow_link(srctile, desttile)

#define RBLXSTBTE_LINK_COLOR(srctile, srcprop, desttile, destprop) \
          (is_platform(srctile) ? 0xff80ff : 0x808040)

#define RBLXSTBTE_DRAW_RECT(x0,y0,x1,y1,c)           \
          editor_draw_rect(x0,y0,x1,y1,(c)>>16,((c)>>8)&255,(c)&255)

#define RBLXSTBTE_DRAW_TILE(x,y,id,highlight,props)  \
          editor_draw_tile(x,y,id,highlight,props)



#define RBLXSTB_TILEMAP_EDITOR_IMPLEMENTATION
#include "rblxstb_tilemap_editor.h"

rblxstbte_tilemap *edit_map;

void editor_key(enum rblxstbte_action act)
{
   rblxstbte_action(edit_map, act);
}

void editor_process_sdl_event(SDL_Event *e)
{
   switch (e->type) {
      case SDL_MOUSEMOTION:
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
      case SDL_MOUSEWHEEL:
         rblxstbte_mouse_sdl(edit_map, e, 1.0f/editor_scale,1.0f/editor_scale,0,0);
         break;

      case SDL_KEYDOWN:
         if (in_editor) {
            switch (e->key.keysym.sym) {
               case SDLK_RIGHT: editor_key(RBLXSTBTE_scroll_right); break;
               case SDLK_LEFT : editor_key(RBLXSTBTE_scroll_left ); break;
               case SDLK_UP   : editor_key(RBLXSTBTE_scroll_up   ); break;
               case SDLK_DOWN : editor_key(RBLXSTBTE_scroll_down ); break;
            }
            switch (e->key.keysym.scancode) {
               case SDL_SCANCODE_S: editor_key(RBLXSTBTE_tool_select); break;
               case SDL_SCANCODE_B: editor_key(RBLXSTBTE_tool_brush ); break;
               case SDL_SCANCODE_E: editor_key(RBLXSTBTE_tool_erase ); break;
               case SDL_SCANCODE_R: editor_key(RBLXSTBTE_tool_rectangle ); break;
               case SDL_SCANCODE_I: editor_key(RBLXSTBTE_tool_eyedropper); break;
               case SDL_SCANCODE_L: editor_key(RBLXSTBTE_tool_link);       break;
               case SDL_SCANCODE_G: editor_key(RBLXSTBTE_act_toggle_grid); break;
            }
            if ((e->key.keysym.mod & KMOD_CTRL) && !(e->key.keysym.mod & ~KMOD_CTRL)) {
               switch (e->key.keysym.scancode) {
                  case SDL_SCANCODE_X: editor_key(RBLXSTBTE_act_cut  ); break;
                  case SDL_SCANCODE_C: editor_key(RBLXSTBTE_act_copy ); break;
                  case SDL_SCANCODE_V: editor_key(RBLXSTBTE_act_paste); break;
                  case SDL_SCANCODE_Z: editor_key(RBLXSTBTE_act_undo ); break;
                  case SDL_SCANCODE_Y: editor_key(RBLXSTBTE_act_redo ); break;
               }
            }
         }
         break;
   }
}

void editor_init(void)
{
   int i;
   edit_map = rblxstbte_create_map(20,14, 8, 16,16, 100);

   rblxstbte_set_background_tile(edit_map, T_empty);

   for (i=0; i < T__num_types; ++i) {
      if (i != T_reserved1 && i != T_entry && i != T_doorframe)
         rblxstbte_define_tile(edit_map, 0+i, 1, "Background");
   }
   rblxstbte_define_tile(edit_map, 256+O_player   , 8, "Char");
   rblxstbte_define_tile(edit_map, 256+O_robot    , 8, "Char");
   for (i=O_lockeddoor; i < O__num_types-2; ++i)
      if (i == O_platform || i == O_vplatform)
         rblxstbte_define_tile(edit_map, 256+i, 4, "Object");
      else
         rblxstbte_define_tile(edit_map, 256+i, 2, "Object");

   //rblxstbte_set_layername(edit_map, 0, "background");
   //rblxstbte_set_layername(edit_map, 1, "objects");
   //rblxstbte_set_layername(edit_map, 2, "platforms");
   //rblxstbte_set_layername(edit_map, 3, "characters");
}

static int is_platform(short *tiles)
{
   // platforms are only on layer #2
   return tiles[2] == 256 + O_platform || tiles[2] == 256 + O_vplatform;
}

static int is_object(short *tiles)
{
   return (tiles[1] >= 256 || tiles[2] >= 256 || tiles[3] >= 256);
}

static unsigned int prop_type(int n, short *tiles)
{
   if (is_platform(tiles)) {
      static unsigned int platform_types[RBLXSTBTE_MAX_PROPERTIES] = {
         RBLXSTBTE_PROP_bool,  // phantom
         RBLXSTBTE_PROP_int,   // x_adjust
         RBLXSTBTE_PROP_int,   // y_adjust
         RBLXSTBTE_PROP_float, // width
         RBLXSTBTE_PROP_float, // lspeed
         RBLXSTBTE_PROP_float, // rspeed
         RBLXSTBTE_PROP_bool,  // autoreturn
         RBLXSTBTE_PROP_bool,  // one-shot
         // remainder get 0, means 'no property in this slot'
      };
      return platform_types[n];
   } else if (is_object(tiles)) {
      if (n == 0)
         return RBLXSTBTE_PROP_bool;
   }
   return 0;
}

static char *prop_name(int n, short *tiles)
{
   if (is_platform(tiles)) {
      static char *platform_vars[RBLXSTBTE_MAX_PROPERTIES] = {
         "phantom",
         "x_adjust",
         "y_adjust",
         "width",
         "lspeed",
         "rspeed",
         "autoreturn",
         "one-shot",
      };
      return platform_vars[n];
   }
   return "phantom";
}

static float prop_range(int n, short *tiles, int is_max)
{
   if (is_platform(tiles)) {
      static float ranges[8][2] = {
         {   0,  1 }, // phantom-flag, range is ignored
         { -15, 15 }, // x_adjust
         { -15, 15 }, // y_adjust
         {   0,  6 }, // width
         {   0, 10 }, // lspeed
         {   0, 10 }, // rspeed
         {   0,  1 }, // autoreturn, range is ignored
         {   0,  1 }, // one-shot, range is ignored
      };
      return ranges[n][is_max];
   }
   return 0;
}

static int allow_link(short *src, short *dest)
{
   if (is_platform(src))
      return dest[1] == 256+O_lever;
   if (src[1] == 256+O_endpoint)
      return is_platform(dest);
   return 0;
}

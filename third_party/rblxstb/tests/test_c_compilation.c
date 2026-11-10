#define RBLXSTB_IMAGE_RESIZE_IMPLEMENTATION
#include "rblxstb_image_resize2.h"

#define RBLXSTB_SPRINTF_IMPLEMENTATION
#include "rblxstb_sprintf.h"

#define RBLXSTB_PERLIN_IMPLEMENTATION
#define RBLXSTB_IMAGE_WRITE_IMPLEMENTATION
#define RBLXSTB_C_LEXER_IMPLEMENTATIOn
#define RBLXSTB_DIVIDE_IMPLEMENTATION
#define RBLXSTB_IMAGE_IMPLEMENTATION
#define RBLXSTB_HERRINGBONE_WANG_TILE_IMEPLEMENTATIOn
#define RBLXSTB_RECT_PACK_IMPLEMENTATION
#define RBLXSTB_VOXEL_RENDER_IMPLEMENTATION
#define RBLXSTB_EASY_FONT_IMPLEMENTATION
#define RBLXSTB_DXT_IMPLEMENTATION
#define RBLXSTB_INCLUDE_IMPLEMENTATION

#include "rblxstb_herringbone_wang_tile.h"
#include "rblxstb_image.h"
#include "rblxstb_image_write.h"
#include "rblxstb_perlin.h"
#include "rblxstb_c_lexer.h"
#include "rblxstb_divide.h"
#include "rblxstb_rect_pack.h"
#include "rblxstb_dxt.h"
#include "rblxstb_include.h"

#include "rblxstb_ds.h"

#define RBLXSTBVOX_CONFIG_MODE 1
#include "rblxstb_voxel_render.h"

void RBLXSTBTE_DRAW_RECT(int x0, int y0, int x1, int y1, unsigned int color)
{
}

void RBLXSTBTE_DRAW_TILE(int x0, int y0, unsigned short id, int highlight, float *data)
{
}

#define RBLXSTB_TILEMAP_EDITOR_IMPLEMENTATION
//#include "rblxstb_tilemap_editor.h"   // @TODO: it's broken

int quicktest(void)
{
   char buffer[999];
   rblxstbsp_sprintf(buffer, "test%%test");
   return 0;
}
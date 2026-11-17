/*
 * Nuklear - v1.32.0 - public domain
 * no warrenty implied; use at your own risk.
 * authored from 2015-2017 by Micha Mettke
 */
/*
 * ==============================================================
 *
 *                              API
 *
 * ===============================================================
 */
#ifndef NK_LINEARWIN_GL2_H_
#define NK_LINEARWIN_GL2_H_

#include <LINEARWIN/linearwin.h>

enum nk_linearwin_init_state{
    NK_LINEARWIN3_DEFAULT = 0,
    NK_LINEARWIN3_INSTALL_CALLBACKS
};
NK_API struct nk_context*   nk_linearwin_init(LINEARWINwindow *win, enum nk_linearwin_init_state);
NK_API void                 nk_linearwin_font_stash_begin(struct nk_font_atlas **atlas);
NK_API void                 nk_linearwin_font_stash_end(void);

NK_API void                 nk_linearwin_new_frame(void);
NK_API void                 nk_linearwin_render(enum nk_anti_aliasing);
NK_API void                 nk_linearwin_shutdown(void);

NK_API void                 nk_linearwin_char_callback(LINEARWINwindow *win, unsigned int codepoint);
NK_API void                 nk_gflw3_scroll_callback(LINEARWINwindow *win, double xoff, double yoff);

#endif

/*
 * ==============================================================
 *
 *                          IMPLEMENTATION
 *
 * ===============================================================
 */
#ifdef NK_LINEARWIN_GL2_IMPLEMENTATION

#ifndef NK_LINEARWIN_TEXT_MAX
#define NK_LINEARWIN_TEXT_MAX 256
#endif
#ifndef NK_LINEARWIN_DOUBLE_CLICK_LO
#define NK_LINEARWIN_DOUBLE_CLICK_LO 0.02
#endif
#ifndef NK_LINEARWIN_DOUBLE_CLICK_HI
#define NK_LINEARWIN_DOUBLE_CLICK_HI 0.2
#endif

struct nk_linearwin_device {
    struct nk_buffer cmds;
    struct nk_draw_null_texture null;
    GLuint font_tex;
};

struct nk_linearwin_vertex {
    float position[2];
    float uv[2];
    nk_byte col[4];
};

static struct nk_linearwin {
    LINEARWINwindow *win;
    int width, height;
    int display_width, display_height;
    struct nk_linearwin_device ogl;
    struct nk_context ctx;
    struct nk_font_atlas atlas;
    struct nk_vec2 fb_scale;
    unsigned int text[NK_LINEARWIN_TEXT_MAX];
    int text_len;
    struct nk_vec2 scroll;
    double last_button_click;
    int is_double_click_down;
    struct nk_vec2 double_click_pos;
} linearwin;

NK_INTERN void
nk_linearwin_device_upload_atlas(const void *image, int width, int height)
{
    struct nk_linearwin_device *dev = &linearwin.ogl;
    glGenTextures(1, &dev->font_tex);
    glBindTexture(GL_TEXTURE_2D, dev->font_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, image);
}

NK_API void
nk_linearwin_render(enum nk_anti_aliasing AA)
{
    /* setup global state */
    struct nk_linearwin_device *dev = &linearwin.ogl;
    glPushAttrib(GL_ENABLE_BIT|GL_COLOR_BUFFER_BIT|GL_TRANSFORM_BIT);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* setup viewport/project */
    glViewport(0,0,(GLsizei)linearwin.display_width,(GLsizei)linearwin.display_height);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, linearwin.width, linearwin.height, 0.0f, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    {
        GLsizei vs = sizeof(struct nk_linearwin_vertex);
        size_t vp = offsetof(struct nk_linearwin_vertex, position);
        size_t vt = offsetof(struct nk_linearwin_vertex, uv);
        size_t vc = offsetof(struct nk_linearwin_vertex, col);

        /* convert from command queue into draw list and draw to screen */
        const struct nk_draw_command *cmd;
        const nk_draw_index *offset = NULL;
        struct nk_buffer vbuf, ebuf;

        /* fill convert configuration */
        struct nk_convert_config config;
        static const struct nk_draw_vertex_layout_element vertex_layout[] = {
            {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_linearwin_vertex, position)},
            {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_linearwin_vertex, uv)},
            {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct nk_linearwin_vertex, col)},
            {NK_VERTEX_LAYOUT_END}
        };
        NK_MEMSET(&config, 0, sizeof(config));
        config.vertex_layout = vertex_layout;
        config.vertex_size = sizeof(struct nk_linearwin_vertex);
        config.vertex_alignment = NK_ALIGNOF(struct nk_linearwin_vertex);
        config.null = dev->null;
        config.circle_segment_count = 22;
        config.curve_segment_count = 22;
        config.arc_segment_count = 22;
        config.global_alpha = 1.0f;
        config.shape_AA = AA;
        config.line_AA = AA;

        /* convert shapes into vertexes */
        nk_buffer_init_default(&vbuf);
        nk_buffer_init_default(&ebuf);
        nk_convert(&linearwin.ctx, &dev->cmds, &vbuf, &ebuf, &config);

        /* setup vertex buffer pointer */
        {const void *vertices = nk_buffer_memory_const(&vbuf);
        glVertexPointer(2, GL_FLOAT, vs, (const void*)((const nk_byte*)vertices + vp));
        glTexCoordPointer(2, GL_FLOAT, vs, (const void*)((const nk_byte*)vertices + vt));
        glColorPointer(4, GL_UNSIGNED_BYTE, vs, (const void*)((const nk_byte*)vertices + vc));}

        /* iterate over and execute each draw command */
        offset = (const nk_draw_index*)nk_buffer_memory_const(&ebuf);
        nk_draw_foreach(cmd, &linearwin.ctx, &dev->cmds)
        {
            if (!cmd->elem_count) continue;
            glBindTexture(GL_TEXTURE_2D, (GLuint)cmd->texture.id);
            glScissor(
                (GLint)(cmd->clip_rect.x * linearwin.fb_scale.x),
                (GLint)((linearwin.height - (GLint)(cmd->clip_rect.y + cmd->clip_rect.h)) * linearwin.fb_scale.y),
                (GLint)(cmd->clip_rect.w * linearwin.fb_scale.x),
                (GLint)(cmd->clip_rect.h * linearwin.fb_scale.y));
            glDrawElements(GL_TRIANGLES, (GLsizei)cmd->elem_count, GL_UNSIGNED_SHORT, offset);
            offset += cmd->elem_count;
        }
        nk_clear(&linearwin.ctx);
        nk_buffer_free(&vbuf);
        nk_buffer_free(&ebuf);
    }

    /* default OpenGL state */
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
}

NK_API void
nk_linearwin_char_callback(LINEARWINwindow *win, unsigned int codepoint)
{
    (void)win;
    if (linearwin.text_len < NK_LINEARWIN_TEXT_MAX)
        linearwin.text[linearwin.text_len++] = codepoint;
}

NK_API void
nk_gflw3_scroll_callback(LINEARWINwindow *win, double xoff, double yoff)
{
    (void)win; (void)xoff;
    linearwin.scroll.x += (float)xoff;
    linearwin.scroll.y += (float)yoff;
}

NK_API void
nk_linearwin_mouse_button_callback(LINEARWINwindow* window, int button, int action, int mods)
{
    double x, y;
    if (button != LINEARWIN_MOUSE_BUTTON_LEFT) return;
    linearwinGetCursorPos(window, &x, &y);
    if (action == LINEARWIN_PRESS)  {
        double dt = linearwinGetTime() - linearwin.last_button_click;
        if (dt > NK_LINEARWIN_DOUBLE_CLICK_LO && dt < NK_LINEARWIN_DOUBLE_CLICK_HI) {
            linearwin.is_double_click_down = nk_true;
            linearwin.double_click_pos = nk_vec2((float)x, (float)y);
        }
        linearwin.last_button_click = linearwinGetTime();
    } else linearwin.is_double_click_down = nk_false;
}

NK_INTERN void
nk_linearwin_clipbard_paste(nk_handle usr, struct nk_text_edit *edit)
{
    const char *text = linearwinGetClipboardString(linearwin.win);
    if (text) nk_textedit_paste(edit, text, nk_strlen(text));
    (void)usr;
}

NK_INTERN void
nk_linearwin_clipbard_copy(nk_handle usr, const char *text, int len)
{
    char *str = 0;
    (void)usr;
    if (!len) return;
    str = (char*)malloc((size_t)len+1);
    if (!str) return;
    NK_MEMCPY(str, text, (size_t)len);
    str[len] = '\0';
    linearwinSetClipboardString(linearwin.win, str);
    free(str);
}

NK_API struct nk_context*
nk_linearwin_init(LINEARWINwindow *win, enum nk_linearwin_init_state init_state)
{
    linearwin.win = win;
    if (init_state == NK_LINEARWIN3_INSTALL_CALLBACKS) {
        linearwinSetScrollCallback(win, nk_gflw3_scroll_callback);
        linearwinSetCharCallback(win, nk_linearwin_char_callback);
        linearwinSetMouseButtonCallback(win, nk_linearwin_mouse_button_callback);
    }
    nk_init_default(&linearwin.ctx, 0);
    linearwin.ctx.clip.copy = nk_linearwin_clipbard_copy;
    linearwin.ctx.clip.paste = nk_linearwin_clipbard_paste;
    linearwin.ctx.clip.userdata = nk_handle_ptr(0);
    nk_buffer_init_default(&linearwin.ogl.cmds);

    linearwin.is_double_click_down = nk_false;
    linearwin.double_click_pos = nk_vec2(0, 0);

    return &linearwin.ctx;
}

NK_API void
nk_linearwin_font_stash_begin(struct nk_font_atlas **atlas)
{
    nk_font_atlas_init_default(&linearwin.atlas);
    nk_font_atlas_begin(&linearwin.atlas);
    *atlas = &linearwin.atlas;
}

NK_API void
nk_linearwin_font_stash_end(void)
{
    const void *image; int w, h;
    image = nk_font_atlas_bake(&linearwin.atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
    nk_linearwin_device_upload_atlas(image, w, h);
    nk_font_atlas_end(&linearwin.atlas, nk_handle_id((int)linearwin.ogl.font_tex), &linearwin.ogl.null);
    if (linearwin.atlas.default_font)
        nk_style_set_font(&linearwin.ctx, &linearwin.atlas.default_font->handle);
}

NK_API void
nk_linearwin_new_frame(void)
{
    int i;
    double x, y;
    struct nk_context *ctx = &linearwin.ctx;
    struct LINEARWINwindow *win = linearwin.win;

    linearwinGetWindowSize(win, &linearwin.width, &linearwin.height);
    linearwinGetFramebufferSize(win, &linearwin.display_width, &linearwin.display_height);
    linearwin.fb_scale.x = (float)linearwin.display_width/(float)linearwin.width;
    linearwin.fb_scale.y = (float)linearwin.display_height/(float)linearwin.height;

    nk_input_begin(ctx);
    for (i = 0; i < linearwin.text_len; ++i)
        nk_input_unicode(ctx, linearwin.text[i]);

    /* optional grabbing behavior */
    if (ctx->input.mouse.grab)
        linearwinSetInputMode(linearwin.win, LINEARWIN_CURSOR, LINEARWIN_CURSOR_HIDDEN);
    else if (ctx->input.mouse.ungrab)
        linearwinSetInputMode(linearwin.win, LINEARWIN_CURSOR, LINEARWIN_CURSOR_NORMAL);

    nk_input_key(ctx, NK_KEY_DEL, linearwinGetKey(win, LINEARWIN_KEY_DELETE) == LINEARWIN_PRESS);
    nk_input_key(ctx, NK_KEY_ENTER, linearwinGetKey(win, LINEARWIN_KEY_ENTER) == LINEARWIN_PRESS);
    nk_input_key(ctx, NK_KEY_TAB, linearwinGetKey(win, LINEARWIN_KEY_TAB) == LINEARWIN_PRESS);
    nk_input_key(ctx, NK_KEY_BACKSPACE, linearwinGetKey(win, LINEARWIN_KEY_BACKSPACE) == LINEARWIN_PRESS);
    nk_input_key(ctx, NK_KEY_UP, linearwinGetKey(win, LINEARWIN_KEY_UP) == LINEARWIN_PRESS);
    nk_input_key(ctx, NK_KEY_DOWN, linearwinGetKey(win, LINEARWIN_KEY_DOWN) == LINEARWIN_PRESS);
    nk_input_key(ctx, NK_KEY_TEXT_START, linearwinGetKey(win, LINEARWIN_KEY_HOME) == LINEARWIN_PRESS);
    nk_input_key(ctx, NK_KEY_TEXT_END, linearwinGetKey(win, LINEARWIN_KEY_END) == LINEARWIN_PRESS);
    nk_input_key(ctx, NK_KEY_SCROLL_START, linearwinGetKey(win, LINEARWIN_KEY_HOME) == LINEARWIN_PRESS);
    nk_input_key(ctx, NK_KEY_SCROLL_END, linearwinGetKey(win, LINEARWIN_KEY_END) == LINEARWIN_PRESS);
    nk_input_key(ctx, NK_KEY_SCROLL_DOWN, linearwinGetKey(win, LINEARWIN_KEY_PAGE_DOWN) == LINEARWIN_PRESS);
    nk_input_key(ctx, NK_KEY_SCROLL_UP, linearwinGetKey(win, LINEARWIN_KEY_PAGE_UP) == LINEARWIN_PRESS);
    nk_input_key(ctx, NK_KEY_SHIFT, linearwinGetKey(win, LINEARWIN_KEY_LEFT_SHIFT) == LINEARWIN_PRESS||
                                    linearwinGetKey(win, LINEARWIN_KEY_RIGHT_SHIFT) == LINEARWIN_PRESS);

    if (linearwinGetKey(win, LINEARWIN_KEY_LEFT_CONTROL) == LINEARWIN_PRESS ||
        linearwinGetKey(win, LINEARWIN_KEY_RIGHT_CONTROL) == LINEARWIN_PRESS) {
        nk_input_key(ctx, NK_KEY_COPY, linearwinGetKey(win, LINEARWIN_KEY_C) == LINEARWIN_PRESS);
        nk_input_key(ctx, NK_KEY_PASTE, linearwinGetKey(win, LINEARWIN_KEY_V) == LINEARWIN_PRESS);
        nk_input_key(ctx, NK_KEY_CUT, linearwinGetKey(win, LINEARWIN_KEY_X) == LINEARWIN_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_UNDO, linearwinGetKey(win, LINEARWIN_KEY_Z) == LINEARWIN_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_REDO, linearwinGetKey(win, LINEARWIN_KEY_R) == LINEARWIN_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, linearwinGetKey(win, LINEARWIN_KEY_LEFT) == LINEARWIN_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, linearwinGetKey(win, LINEARWIN_KEY_RIGHT) == LINEARWIN_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_LINE_START, linearwinGetKey(win, LINEARWIN_KEY_B) == LINEARWIN_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_LINE_END, linearwinGetKey(win, LINEARWIN_KEY_E) == LINEARWIN_PRESS);
    } else {
        nk_input_key(ctx, NK_KEY_LEFT, linearwinGetKey(win, LINEARWIN_KEY_LEFT) == LINEARWIN_PRESS);
        nk_input_key(ctx, NK_KEY_RIGHT, linearwinGetKey(win, LINEARWIN_KEY_RIGHT) == LINEARWIN_PRESS);
        nk_input_key(ctx, NK_KEY_COPY, 0);
        nk_input_key(ctx, NK_KEY_PASTE, 0);
        nk_input_key(ctx, NK_KEY_CUT, 0);
        nk_input_key(ctx, NK_KEY_SHIFT, 0);
    }

    linearwinGetCursorPos(win, &x, &y);
    nk_input_motion(ctx, (int)x, (int)y);
    if (ctx->input.mouse.grabbed) {
        linearwinSetCursorPos(linearwin.win, (double)ctx->input.mouse.prev.x, (double)ctx->input.mouse.prev.y);
        ctx->input.mouse.pos.x = ctx->input.mouse.prev.x;
        ctx->input.mouse.pos.y = ctx->input.mouse.prev.y;
    }

    nk_input_button(ctx, NK_BUTTON_LEFT, (int)x, (int)y, linearwinGetMouseButton(win, LINEARWIN_MOUSE_BUTTON_LEFT) == LINEARWIN_PRESS);
    nk_input_button(ctx, NK_BUTTON_MIDDLE, (int)x, (int)y, linearwinGetMouseButton(win, LINEARWIN_MOUSE_BUTTON_MIDDLE) == LINEARWIN_PRESS);
    nk_input_button(ctx, NK_BUTTON_RIGHT, (int)x, (int)y, linearwinGetMouseButton(win, LINEARWIN_MOUSE_BUTTON_RIGHT) == LINEARWIN_PRESS);
    nk_input_button(ctx, NK_BUTTON_DOUBLE, (int)linearwin.double_click_pos.x, (int)linearwin.double_click_pos.y, linearwin.is_double_click_down);
    nk_input_scroll(ctx, linearwin.scroll);
    nk_input_end(&linearwin.ctx);
    linearwin.text_len = 0;
    linearwin.scroll = nk_vec2(0,0);
}

NK_API
void nk_linearwin_shutdown(void)
{
    struct nk_linearwin_device *dev = &linearwin.ogl;
    nk_font_atlas_clear(&linearwin.atlas);
    nk_free(&linearwin.ctx);
    glDeleteTextures(1, &dev->font_tex);
    nk_buffer_free(&dev->cmds);
    NK_MEMSET(&linearwin, 0, sizeof(linearwin));
}

#endif

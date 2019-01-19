// vdb.c
// Author: hyan23
// Date: 2018.03.24

#include <stdint.h>
#include <math.h>
#include "header.h"
#include "common.h"
#include "sdl-util.h"
#include "vdb.h"

static cnes_vdb_rgb_s vdb_colors[NumColors] = {
    { 0xff, 0xff, 0xff }, { 0xff, 0xff, 0xf0 }, { 0xff, 0xff, 0xfa }, 
    { 0x0, 0x0, 0x0 }, { 0xbe, 0xbe, 0xbe }, { 0xd3, 0xd3, 0xd3 }, 
    { 0xff, 0x0, 0x0 }, { 0xa5, 0x2a, 0x2a }, { 0xb2, 0x22, 0x22 }, 
        { 0xcd, 0x5c, 0x5c }, { 0xff, 0xc0, 0xcb }, 
    { 0x0, 0xff, 0x0 }, { 0x0, 0xff, 0x7f }, { 0x2e, 0x8b, 0x57 }, 
        { 0x6b, 0x8e, 0x23 }, 
    { 0x0, 0x0, 0xff }, { 0x46, 0x82, 0xb4 }, { 0x0, 0xff, 0xff }, 
        { 0xee, 0x82, 0xee }, 
    { 0xff, 0xff, 0x0 }, { 0xff, 0xd7, 0x0 }, { 0xff, 0xa5, 0x0 }, 
        { 0xd2, 0xb4, 0x8c }
};

Uint32 
cnes_vdb_color_MapRGB(const SDL_PixelFormat* format, 
cnes_vdb_color_t color) 
{
    assert(color < NumColors);
    cnes_vdb_rgb_s rgb;
    rgb.r = vdb_colors[color].r;
    rgb.g = vdb_colors[color].g;
    rgb.b = vdb_colors[color].b;
    return SDL_MapRGB(format, rgb.r, rgb.g, rgb.b);
}

Uint32 
cnes_vdb_color_MapRGBA(const SDL_PixelFormat* format, 
cnes_vdb_color_t color, byte alpha) 
{
    assert(color < NumColors);
    cnes_vdb_rgba_s rgba;
    rgba.r = vdb_colors[color].r;
    rgba.g = vdb_colors[color].g;
    rgba.b = vdb_colors[color].b;
    rgba.a = alpha;
    return SDL_MapRGBA(format, rgba.r, rgba.g, rgba.b, rgba.a);
}

float cnes_vdb_var_ratio(const cnes_vdb_var_s* var)
{
    switch (var->type) {
        case T_U8:
            return CNES_VDB_VAR_RATIO(*var, byte);
        case T_S8:
            return CNES_VDB_VAR_RATIO(*var, int8_t);
        case T_U16:
            return CNES_VDB_VAR_RATIO(*var, uint16_t);
        case T_S16:
            return CNES_VDB_VAR_RATIO(*var, int16_t);
        case T_U32:
            return CNES_VDB_VAR_RATIO(*var, uint32_t);
        case T_S32:
            return CNES_VDB_VAR_RATIO(*var, int32_t);
        case T_U64:
            return CNES_VDB_VAR_RATIO(*var, uint64_t);
        case T_S64:
            return CNES_VDB_VAR_RATIO(*var, int64_t);
        case T_FLOAT:
            return CNES_VDB_VAR_RATIO(*var, float);
        case T_DOULBE:
            return CNES_VDB_VAR_RATIO(*var, double);
        default:
            return .1f;
    }
}

LINKED_IMPL_LIST(cnes_vdb_VL_s, vl);

error_t 
cnes_vdb_init(cnes_vdb_s* vdb, 
SDL_Surface* parent, byte alpha, float w, float h, cnes_vdb_pos_t pos) 
{
    linked_create_vl(&vdb->vl); // vdb->v1 = NULL;
    vdb->count = 0;
    
    vdb->parent = parent;
    vdb->sur = NULL;
    vdb->pos = pos;
    vdb->bg = C_BLACK;
    vdb->alpha = alpha;
    vdb->w = w;
    vdb->h = h;
    vdb->sw = 0;
    vdb->sh = 0;
    
    vdb->thread = NULL;
    vdb->fps = 60;
    vdb->closed = TRUE;
    vdb->mutex = SDL_CreateMutex();
    assert(NOTNULL(vdb->mutex));
    
    if (NUL(vdb->parent)) {
        vdb->autonomy = TRUE;
        vdb->w = 1.0f;
        vdb->h = 1.0f;
        vdb->sw = 300;
        vdb->sh = 200;
        vdb->pos = P_UpperLeft;
    }
    return SUCCESS;
}

void cnes_vdb_close(cnes_vdb_s* vdb)
{
    if (vdb->autonomy) {
        if (NOTNULL(vdb->thread)) {
            vdb->closed = TRUE;
            SDL_WaitThread(vdb->thread, NULL);
        }
    }
    if (NOTNULL(vdb->mutex)) {
        SDL_DestroyMutex(vdb->mutex);
    }
    if (NOTNULL(vdb->sur)) {
        SDL_FreeSurface(vdb->sur);
    }
    if (vdb->autonomy) {
        if (NOTNULL(vdb->parent)) {
            SDL_FreeSurface(vdb->parent);
            SDL_Quit();
        }
    }
    if (NOTNULL(vdb->vl)) {
        linked_destroy_vl(&vdb->vl, NULL);
    }
}

static int vdb_callback(cnes_vdb_s* vdb)
{
    while (vdb->closed) { // wait for start
        SDL_Delay(20);
    }
    while (!vdb->closed) {
        SDL_FillRect(
            vdb->parent, NULL, 
            cnes_vdb_color_MapRGBA(vdb->parent->format, vdb->bg, 0xff));
        cnes_vdb_update(vdb);
        SDL_Flip(vdb->parent);
        SDL_Event evt;
        if (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT) {
                vdb->closed = TRUE;
            }
        }
        SDL_Delay(1000 / vdb->fps);
    }
    return 0;
}

static error_t vdb_openw(cnes_vdb_s* vdb) {
    if (SDL_Init(SDL_INIT_VIDEO) == -1) {
        return SDL_ERROR;
    }
    vdb->parent = SDL_SetVideoMode(vdb->sw, vdb->sh, 32, 
        SDL_SWSURFACE | SDL_DOUBLEBUF);
    if (NUL(vdb->parent)) {
        SDL_Quit();
        return SDL_ERROR;
    }
    SDL_WM_SetCaption("CNES Visual Debugger", NULL);
    
    vdb->thread = SDL_CreateThread(
        (int (SDLCALL *)(void*)) vdb_callback, vdb);
    if (NUL(vdb->thread)) {
        SDL_FreeSurface(vdb->parent);
        SDL_Quit();
        return SDL_ERROR;
    }
    return SUCCESS;
}

static error_t create_sur(cnes_vdb_s* vdb)
{
    sint pw = vdb->parent->w;
    sint ph = vdb->parent->h;
    // sint pd = vdb->parent->pitch;    // depth
    vdb->sw = pw * vdb->w; // surface width
    vdb->sh = ph * vdb->h;
    
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    Uint32 rmask = 0xff000000;
    Uint32 gmask = 0x00ff0000;
    Uint32 bmask = 0x0000ff00;
    Uint32 amask = 0x000000ff;
#else
    Uint32 rmask = 0x000000ff;
    Uint32 gmask = 0x0000ff00;
    Uint32 bmask = 0x00ff0000;
    Uint32 amask = 0xff000000;
#endif
    vdb->sur = SDL_CreateRGBSurface(
        SDL_SWSURFACE, 
        vdb->sw, vdb->sh, 32, 
        rmask, gmask, bmask, amask);
    if (NUL(vdb->sur)) {
        return SDL_ERROR;
    } else {
        return SUCCESS;
    }
}

error_t cnes_vdb_start(cnes_vdb_s* vdb)
{
    if (vdb->autonomy) {
        if (FAILED(vdb_openw(vdb))) {
            return SDL_ERROR;
        }
    }
    if (FAILED(create_sur(vdb))) {
        cnes_vdb_close(vdb);
        return SDL_ERROR;
    }
    vdb->closed = FALSE;
    return SUCCESS;
}

error_t 
cnes_vdb_watch_var(cnes_vdb_s* vdb, 
void* var, cnes_vdb_var_t type, double max, 
cnes_vdb_color_t color, byte alpha, cnes_vdb_rgb_s* custom) 
{
    cnes_vdb_var_s v1;
    v1.var = var;
    v1.type = type;
    v1.max = max;
    v1.color = color;
    v1.alpha = alpha;
    v1.custom = custom;
    return cnes_vdb_watch(vdb, v1);
}

error_t cnes_vdb_watch(cnes_vdb_s* vdb, cnes_vdb_var_s var)
{
    SDL_MUTEXP(vdb->mutex);
    error_t result = (NOTNULL(linked_append_vl(&vdb->vl, &var))) ? 
        SUCCESS : FAILURE;
    if (SUCCEED(result)) {
        vdb->count ++;
    }
    SDL_MUTEXV(vdb->mutex);
    return result;
}

static sint find_var(const cnes_vdb_VL_s* self, const cnes_vdb_VL_s* other)
{
    return self->data.var == other ? 0 : -1;
}

void cnes_vdb_remove(cnes_vdb_s* vdb, const void* var)
{
    SDL_MUTEXP(vdb->mutex);
    if (NOTNULL(linked_remove_vl(&vdb->vl, var, 
        (linked_cmp_func) find_var, LINKED_FIELD(cnes_vdb_VL_s, data)))) {
        vdb->count --;
    }
    SDL_MUTEXV(vdb->mutex);
}

static void draw_bars(cnes_vdb_s* vdb)
{
    SDL_FillRect(vdb->sur, NULL, cnes_vdb_color_MapRGBA(vdb->sur->format, 
        vdb->bg, vdb->alpha));
    if (vdb->count == 0) {
        return;
    }
    uint sh_rem = vdb->sh;      // remaining
    uint bar_rem = vdb->count;
    uint y = 0;
    cnes_vdb_VL_s* cur = vdb->vl;
    
    while (cur != NULL) {
        cnes_vdb_var_s* v1 = &cur->data;
        
        uint bh = sh_rem / bar_rem; // bar height
        sh_rem -= bh;
        bar_rem -= 1;
        assert(BETWEEN(bh, 1, vdb->sh) && 
            BETWEEN(sh_rem, 0, vdb->sh) && 
            BETWEEN(bar_rem, 0, vdb->count));
        
        SDL_Rect rect;
        rect.y = y;
        rect.x = 0;
        float ratio = cnes_vdb_var_ratio(v1);
        rect.w = abs((uint) (ratio * vdb->sw));
        rect.h = bh;
        if (CNES_VDB_VAR_SIGNED(v1->type)) {
            rect.w /= 2;
            if (ratio < 0) {
                rect.x = vdb->sw / 2 - rect.w;
            } else {
                rect.x = vdb->sw / 2;
            }
        }
        if (rect.w > 0) {
            Uint32 color = 0;
            if (NOTNULL(v1->custom)) {
                color = SDL_MapRGBA(
                    vdb->sur->format, 
                    v1->custom->r, v1->custom->g, v1->custom->b, 
                    v1->alpha);
            } else {
                color = cnes_vdb_color_MapRGBA(vdb->sur->format, 
                    v1->color, v1->alpha);
            }
            SDL_FillRect(vdb->sur, &rect, color);
        }
        y += bh;
        cur = cur->next;
    }
}

void cnes_vdb_update(cnes_vdb_s* vdb)
{
    SDL_MUTEXP(vdb->mutex);
    draw_bars(vdb);
    SDL_Rect rect;
    CLEAR(&rect);
    switch (vdb->pos) {
        default:
        case P_UpperLeft:
            rect.y = 0;
            rect.x = 0;
            break;
        case P_UpperRight:
            rect.y = 0;
            rect.x = vdb->parent->w - vdb->sw;
            break;
        case P_LowerLeft:
            rect.y = vdb->parent->h - vdb->sh;
            rect.x = 0;
            break;
        case P_LowerRigth:
            rect.y = vdb->parent->h - vdb->sh;
            rect.x = vdb->parent->w - vdb->sw;
            break;
    }
    SDL_BlitSurface(vdb->sur, NULL, vdb->parent, &rect);
    SDL_MUTEXV(vdb->mutex);
}
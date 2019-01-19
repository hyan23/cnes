// vdb.h
// Author: hyan23
// Date: 2018.03.24

#ifndef __VDB_H__
#define __VDB_H__

#include <SDL/SDL.h>
#include "header.h"

typedef enum CNES_VDB_COLOR {
    C_WHITE,    C_IVORY,    C_SNOW, 
    C_BLACK,    C_GREY,     C_LIGHTGRAY, 
    C_RED,      C_BROWN,    C_FIREBRICK,    C_INDIANRED,    C_PINK, 
    C_GREEN,    C_SPRINGGREEN,C_SEAGREEN,   C_OLIVEDRAB, 
    C_BLUE,     C_STEELBLUE,C_CYAN,         C_VOILET, 
    C_YELLOW,   C_GOLD,     C_ORANGE,       C_TAN, 
    NumColors
}
cnes_vdb_color_t;

extern Uint32 cnes_vdb_MapRGB(const SDL_PixelFormat* format, 
    cnes_vdb_color_t color);
extern Uint32 cnes_vdb_MapRGBA(const SDL_PixelFormat* format, 
    cnes_vdb_color_t color, byte alpha);

typedef struct CNES_VDB_RGB
{
    byte    r;
    byte    g;
    byte    b;
}
cnes_vdb_rgb_s;

typedef struct CNES_VDB_RGBA
{
    byte    r;
    byte    g;
    byte    b;
    byte    a;
}
cnes_vdb_rgba_s;

// variable
typedef enum CNES_VDB_VAR_TYPE {
    T_U8,   T_S8, 
    T_U16,  T_S16, 
    T_U32,  T_S32, 
    T_U64,  T_S64, 
    T_FLOAT,    T_DOULBE
}
cnes_vdb_var_t;

#define CNES_VDB_VAR_VAL(var, type) (*((type*) (var)))
#define CNES_VDB_VAR_RATIO(v1, type)    \
        (CNES_VDB_VAR_VAL((v1).var, type) / (v1).max);
#define CNES_VDB_VAR_SIGNED(type)       \
        ((type) == T_S8 || ((type) == T_S16) ||     \
        (type) == T_S32 || (type) == T_S64 ||       \
        (type) == T_FLOAT || (type) == T_DOULBE)

typedef struct CNES_VDB_VARIABLE
{
    const void*         var;
    double              max;    // [-max, +max]
    cnes_vdb_var_t      type;
    cnes_vdb_color_t    color;
    byte                alpha;
    cnes_vdb_rgb_s*     custom;
    // TODO: const char* name;
}
cnes_vdb_var_s;

extern float cnes_vdb_var_ratio(const cnes_vdb_var_s* var);

// variable list
typedef struct CNES_VDB_VAR_LIST
{
    cnes_vdb_var_s      data;
    struct CNES_VDB_VAR_LIST*   next;
}
cnes_vdb_VL_s;

// visual debugger
typedef enum CNES_VDB_POSITION {
    P_UpperLeft,    P_UpperRight,   P_LowerLeft,    P_LowerRigth
}
cnes_vdb_pos_t;

typedef struct CNES_VDB
{
    cnes_vdb_VL_s*      vl;
    uint            count;
    SDL_Surface*    parent;
    SDL_Surface*        sur;
    cnes_vdb_pos_t      pos;
    cnes_vdb_color_t    bg;
    byte    alpha;
    float   w;  /* ratios */
    float   h;
    uint    sw;     /* actual width */
    uint    sh;
    BOOL        autonomy;   /* will create window */
    SDL_Thread* thread;
    uint    fps;
    BOOL    closed;
    SDL_mutex*  mutex;
}
cnes_vdb_s;

extern error_t cnes_vdb_init(cnes_vdb_s* vdb, 
    SDL_Surface* parent, byte alpha, float w, float h, cnes_vdb_pos_t pos);
extern void cnes_vdb_close(cnes_vdb_s* vdb);
extern error_t cnes_vdb_start(cnes_vdb_s* vdb);
extern error_t cnes_vdb_watch_var(cnes_vdb_s* vdb, 
    void* var, cnes_vdb_var_t type, double max, 
    cnes_vdb_color_t color, byte alpha, cnes_vdb_rgb_s* custom);
extern error_t cnes_vdb_watch(cnes_vdb_s* vdb, cnes_vdb_var_s var);
extern void cnes_vdb_remove(cnes_vdb_s* vdb, const void* var);
extern void cnes_vdb_update(cnes_vdb_s* vdb);

#endif /* __VDB_H__ */
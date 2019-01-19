// video.h
// Author: hyan23
// Date: 2017.08.25

#ifndef __VIDEO__
#define __VIDEO__

#include <SDL/SDL.h>
#include "header.h"
#include "../2A03/vdb.h"

#define CNES_VIDEO_W            256
#define CNES_VIDEO_H            240
#define CNES_VIDEO_BPP          8
#define CNES_VIDEO_PALETTE      1

typedef struct CNES_VIDEO
{
    char    caption[BUFFER_MIDDLE];
    char    icon[FILENAME];     /* unused */
    float   scale;
    uint    w;  /* calculated */
    uint    h;
    float   frame_rate;
    float   fps;
    byte    frame[CNES_VIDEO_H][CNES_VIDEO_W];
    BOOL    frame_ready;
    SDL_Surface*    sur;
    BOOL show_audio;
    cnes_vdb_s  vdb;
    SDL_Thread* thread;
    BOOL    closed;
}
cnes_video_s;

extern error_t cnes_video_open(cnes_video_s* video, 
    const char* caption, float scale, float frame_rate, BOOL show_audio);
extern void cnes_video_close(cnes_video_s* video);
extern error_t cnes_video_start(cnes_video_s* video);

#endif /* __VIDEO_H__ */
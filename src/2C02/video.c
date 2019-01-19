// video.c
// Author: hyan23
// Date: 2017.08.25

#include <time.h>
#include <SDL/SDL.h>
#include "header.h"
#include "timer.h"
#include "palette.h"
#include "pp.h"
#include "video.h"

#if (CNES_VIDEO_PALETTE == 1)
static SDL_Color* palette = cnes_palette1;
#elif (CNES_VIDEO_PALETTE == 2)
#else
static SDL_Color* palette = cnes_palette2;
#endif /* CNES_VIDEO_PALETTE */

error_t 
cnes_video_open(cnes_video_s* video, 
const char* caption, float scale, float frame_rate, BOOL show_audio)
{
    strcpy(video->caption, caption);
    CLEAR(video->icon);
    assert(scale >= 1 && scale <= 4);
    video->scale = scale;
    video->w = video->scale * CNES_VIDEO_W;
    video->h = video->scale * CNES_VIDEO_H;
    CLEAR(video->frame);
    video->frame_ready = FALSE;
    video->thread = NULL;
    video->closed = FALSE;
    assert(frame_rate >= 5 && frame_rate <= 60);
    video->frame_rate = frame_rate;
    video->fps = video->frame_rate;
    video->show_audio = show_audio;
    
    if (SDL_Init(SDL_INIT_VIDEO) == -1) {
        return SDL_ERROR;
    }
    video->sur = SDL_SetVideoMode(video->w, video->h, CNES_VIDEO_BPP, 
        SDL_SWSURFACE | SDL_DOUBLEBUF);
    if (NUL(video->sur)) {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        SDL_Quit();
        return SDL_ERROR;
    }
#if (CNES_VIDEO_BPP == 8)
    assert(SDL_SetPalette(video->sur, 
        SDL_LOGPAL | SDL_PHYSPAL, 
        palette, 0, 0x40) != -1);
#endif /* CNES_VIDEO_BPP */
    SDL_WM_SetCaption(video->caption, video->icon);
    cnes_vdb_init(&video->vdb, video->sur, 100, 0.4, 0.2, P_UpperLeft);
    return SUCCESS;
}

void cnes_video_close(cnes_video_s* video)
{
    if (NOTNULL(video->thread)) {
        video->closed = TRUE;
        SDL_WaitThread(video->thread, NULL);
    }
    cnes_vdb_close(&video->vdb);
    SDL_FreeSurface(video->sur);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    SDL_Quit();
}

static void render_frame(cnes_video_s* video)
{
    for (int i = 0; i < video->h; i ++) {
        for (int j = 0; j < video->w; j ++) {
            byte color = video->frame[i * CNES_VIDEO_H / video->h]
                [j * CNES_VIDEO_W / video->w];
            assert(color < 0x40);
#if (CNES_VIDEO_BPP == 8)
            *(((byte*) video->sur->pixels) + i * video->w + j) = color;
#elif (CNES_VIDEO_BPP == 24)
            const SDL_Color* rgb_color = &palette[color];
            byte* pixel = ((byte*) video->sur->pixels) + 
                i * video->w * video->sur->format->BytesPerPixel + 
                j * video->sur->format->BytesPerPixel;
#if SDL_BYTEORDER == SDL_LIL_ENDIAN /* Not sure */
            pixel[0] = rgb_color->b;
            pixel[1] = rgb_color->g;
            pixel[2] = rgb_color->r;
#else
            pixel[0] = rgb_color->r;
            pixel[1] = rgb_color->g;
            pixel[2] = rgb_color->b;
#endif /* SDL_BYTEORDER */
#endif /* CNES_VIDEO_BPP */
        }
    }
#if CNES_VIDEO_BPP == 8
    // post processing
    // cnes_pp_blur(video->sur->pixels, video->w, video->h);
#endif /* CNES_VIDEO_BPP */
}

static int SDLCALL video_thread(cnes_video_s* video)
{
    while (!video->closed) {
        while (!video->frame_ready) {
            SDL_Delay(10);
        }
        render_frame(video);
        video->frame_ready = FALSE;
        if (video->show_audio) {
            cnes_vdb_update(&video->vdb);
        }
        SDL_Flip(video->sur);
    }
    return 0;
}

error_t cnes_video_start(cnes_video_s* video)
{
    video->thread = SDL_CreateThread(
        (int (SDLCALL *)(void*)) video_thread, video);
    if (NUL(video->thread)) {
        return SDL_ERROR;
    }
    cnes_vdb_start(&video->vdb);
    return SUCCESS;
}
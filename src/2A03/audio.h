// audio.h
// Author: hyan23
// Date: 2018.03.20

#ifndef __AUDIO_H__
#define __AUDIO_H__

#include <SDL/SDL.h>
#include <SDL/SDL_audio.h>
#include "header.h"

typedef struct CNES_AUDIO
{
    uint        freq;
    BOOL        stereo;
    uint        buffers;        // buf_size = buffers * obtained.size
    uint        buf_size;
    Uint8*      buffer;
    uint        buffer_pos;
    Uint8*          backup;
    SDL_AudioSpec   obtained;
    BOOL            pause;
    SDL_mutex*      mutex;
}
cnes_audio_s;

extern error_t cnes_audio_open(cnes_audio_s* audio, 
    uint freq, uint buffers, BOOL stereo);
extern void cnes_audio_close(cnes_audio_s* audio);
extern void cnes_audio_dump(const cnes_audio_s* audio);
extern void cnes_audio_play(cnes_audio_s* audio);
extern void cnes_audio_pause(cnes_audio_s* audio);
extern error_t cnes_audio_addsample(cnes_audio_s* audio, Uint8 sample);

#endif /* __AUDIO_H__ */
// audio.c
// Author: hyan23
// Date: 2018.03.20

#include <SDL/SDL.h>
#include <SDL/SDL_audio.h>
#include "header.h"
#include "sdl-util.h"
#include "audio.h"

// Note: len: size in bytes
// stream: stereo: LR.LR.LR..., mono: sample.sample.sample...
static void SDLCALL 
audio_callback(cnes_audio_s* audio, 
Uint8* stream, int len)
{
    if (audio->buffer_pos >= len) {
        memcpy(stream, audio->buffer, len);
        if (NOTNULL(audio->backup)) {
            memcpy(audio->backup, stream, len);
        }
        SDL_MUTEXP(audio->mutex);
        memcpy(audio->buffer, audio->buffer + len, audio->buffer_pos - len);
        audio->buffer_pos -= len;
        SDL_MUTEXV(audio->mutex);
    } else {
        if (NOTNULL(audio->backup)) {
            memcpy(stream, audio->backup, len);
        } else {
            // memset(stream, audio->obtained->silence, len);
            memset(stream, 0, len);
        }
    }
}

error_t 
cnes_audio_open(cnes_audio_s* audio, 
uint freq, uint buffers, BOOL stereo) 
{
    CLEAR(audio);
    assert(freq == 88200 || freq == 44100 || freq == 22050);
    audio->freq = freq;
    // assert(BETWEEN(buffers, 2, 10));
    audio->buffers = buffers;
    audio->stereo = stereo;
    
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1) {
        return SDL_ERROR;
    }
    SDL_AudioSpec desired;
    CLEAR(&desired);
    desired.freq = audio->freq;
    desired.format = AUDIO_S16LSB;
    desired.channels = audio->stereo ? 2 : 1;
    desired.samples = 1024;
    desired.callback = (void (SDLCALL *)(void*, Uint8*, int)) audio_callback;
    desired.userdata = audio;
    
    if (SDL_OpenAudio(&desired, &audio->obtained) == -1) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return SDL_ERROR;
    }
    
    // *Note*: min delay = samples / freq
    //      max delay   = (buf_size / size) / freq
    audio->buf_size = audio->buffers * audio->obtained.size;
    audio->buffer_pos = 0;
    audio->buffer = MALLOC(Uint8, audio->buf_size);
    if (NUL(audio->buffer)) {
        SDL_CloseAudio();
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return MEMORY_OVERFLOW;
    }
    
    audio->backup = MALLOC(Uint8, audio->obtained.size);
    // NULL is accepted
    if (NOTNULL(audio->backup)) {
        // audio->obtained.silence?
        memset(audio->backup, 0, audio->obtained.size);
    }
    audio->mutex = SDL_CreateMutex();
    assert(NOTNULL(audio->mutex));
    
    cnes_audio_pause(audio);
    cnes_audio_dump(audio);
    return SUCCESS;
}

void cnes_audio_close(cnes_audio_s* audio)
{
    // cnes_audio_pause(audio);
    SDL_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    FREE(audio->backup);
    FREE(audio->buffer);
    if (NOTNULL (audio->mutex)) {
        SDL_DestroyMutex(audio->mutex);
    }
}

void cnes_audio_dump(const cnes_audio_s* audio)
{
    const SDL_AudioSpec* audiospec = &audio->obtained;
    printf("freq: %d\n", audiospec->freq);
    printf("format: 0x%04x\n", audiospec->format);
    printf("channels: %u\n", audiospec->channels);
    printf("silence: %u\n", audiospec->silence);
    printf("samples: %u\n", audiospec->samples);
    printf("padding: %u\n", audiospec->padding);
    printf("size: %u\n", audiospec->size);
    printf("callback: %p\n", audiospec->callback);
    printf("userdata: %p\n", audiospec->userdata);
}

void cnes_audio_play(cnes_audio_s* audio)
{
    SDL_PauseAudio(0);
    audio->pause = FALSE;
}

void cnes_audio_pause(cnes_audio_s* audio)
{
    SDL_PauseAudio(1);
    audio->pause = TRUE;
    audio->buffer_pos = 0;
}

static error_t fill_buffer(cnes_audio_s* audio, Uint8 sample)
{
    if (audio->buffer_pos >= audio->buf_size) {
        return BUFFER_FULL;
    }
    SDL_MUTEXP(audio->mutex);
    audio->buffer[audio->buffer_pos ++] = sample;
    SDL_MUTEXV(audio->mutex);
    return SUCCESS;
}

error_t cnes_audio_addsample(cnes_audio_s* audio, Uint8 sample)
{
    Uint16 temp = sample * 0x80;
    error_t result = fill_buffer(audio, temp & 0xff);
    if (FAILED(result)) {
        return result;
    }
    result = fill_buffer(audio, temp >> 8);
    return result;
}
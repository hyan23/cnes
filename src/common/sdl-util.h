// sdl-util.h
// Author: hyan23
// Date: 2018.05.19

#ifndef __SDLUTIL_H__
#define __SDLUTIL_H__

#include <assert.h>
#include <SDL/SDL.h>

#define SDL_MUTEXP(mutex)   assert(SDL_mutexP((mutex)) != -1)
#define SDL_MUTEXV(mutex)   assert(SDL_mutexV((mutex)) != -1)

#endif /* __SDLUTIL_H__ */
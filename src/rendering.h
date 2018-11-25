

#ifndef PURPLERAIN_RENDERING_H
#define PURPLERAIN_RENDERING_H

#include <SDL2/SDL.h>

typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_GLContext *ctx;
  SDL_Event *event;
  uint32_t frame_start;
  uint32_t frame_time;
} sdl_rendering_t;

sdl_rendering_t *init_sdl(int w, int h, const char *title);

void shutdown_sdl(sdl_rendering_t *sdl);

void delay_frame(sdl_rendering_t *sdl);

#endif //PURPLERAIN_RENDERING_H

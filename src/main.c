#include <stdio.h>
#include "rendering.h"
#include <time.h>
#include <math.h>

#define PARTICLES_COUNT 500
#define WIDTH 640
#define HEIGHT 480
#define RAND_VELOCITY 10000
#define VELOCITY_FACTOR 10
#define LENGTH_FACTOR 5
#define MAX_WIDTH 2
#define MAX_COLORS 4
#define RGB 3
#define SPLASH_STEP M_PI / 240
#define SPLASH_RANGE 20

#define DEBUG 0

typedef struct {
  double x;
  double y;
  int length;
  int width;
  double velocity;
  uint8_t r;
  uint8_t g;
  uint8_t b;
} raindrop_t;

typedef struct {
  // Current splash coords
  double x;
  double y;
  // Data to calculate circlular motion
  // center of circle by Y is always HEIGHT
  // center by X needs to be calculated
  int r;
  int cx;
  double rad_current;
  // At which rad splash will be deactivated
  double rad_end;
  // Can be either positive or negative, depends on direction (L/R)
  double rad_step;
  // Don't process it if it's not active
  uint8_t is_active;
} splash_t;

void init_raindrop(raindrop_t *p, uint8_t ps_colors[MAX_COLORS][RGB]) {
  p->x = random() % WIDTH;
  p->y = -(random() % HEIGHT);
  p->width = (int) (random() % MAX_WIDTH + 1);
  p->length = p->width * LENGTH_FACTOR;
  double dv = (double) (random() % RAND_VELOCITY) / (double) RAND_VELOCITY;
  p->velocity = ((double) p->width / VELOCITY_FACTOR) + dv;

#if DEBUG == 1
  printf("V %lf DV %lf\n", p->velocity, dv);
#endif

  uint8_t *color = ps_colors[random() % 4];
  p->r = color[0];
  p->g = color[1];
  p->b = color[2];
}

void init_splash(splash_t *s, raindrop_t *p) {
  s->y = HEIGHT;
  s->x = (int) p->x;
  s->r = (int) (SPLASH_RANGE * p->velocity);
  s->cx = (int) p->x + s->r;
  // Decide in which direction splash will traverse
  if (random() % 2 == 0) {
    s->rad_current = 0;
    s->rad_step = SPLASH_STEP;
    s->rad_end = M_PI;
  } else {
    s->rad_current = M_PI;
    s->rad_step = -SPLASH_STEP;
    s->rad_end = 0;
  }
  s->is_active = 1;
}

int main() {

  sdl_rendering_t *sdl = init_sdl(WIDTH, HEIGHT, "Purple Rain");

  time_t t = time(NULL);
  srandom((uint32_t) t);

  uint8_t bg_colors[RGB] = {73, 55, 84};
  uint8_t ps_colors[MAX_COLORS][RGB] = {
      {145, 124, 155},
      {155, 123, 149},
      {130, 111, 139},
      {167, 154, 173}
  };

  raindrop_t **drops = malloc(sizeof(raindrop_t *) * PARTICLES_COUNT);
  splash_t **splashes = malloc(sizeof(raindrop_t *) * PARTICLES_COUNT);
  for (size_t i = 0; i < PARTICLES_COUNT; i++) {
    drops[i] = malloc(sizeof(raindrop_t));
    splashes[i] = malloc(sizeof(splash_t));
    init_raindrop(drops[i], ps_colors);
  }

  int is_running = 1;
  while (is_running != 0) {

    printf("Frametime %d\n", sdl->frame_time);

    // BG
    SDL_SetRenderDrawColor(sdl->renderer, bg_colors[0], bg_colors[1], bg_colors[2], SDL_ALPHA_OPAQUE);
    SDL_RenderClear(sdl->renderer);

    // Particles, splashes
    for (size_t i = 0; i < PARTICLES_COUNT; i++) {

      // If raindrop has reached it's destination - init splash and refresh raindrop props
      drops[i]->y = drops[i]->y + drops[i]->velocity;
      if (drops[i]->y > HEIGHT) {
        init_splash(splashes[i], drops[i]);
        init_raindrop(drops[i], ps_colors);
      }

      if (splashes[i]->is_active == 1) {

#if DEBUG == 2
        printf("Curr %zu splash rads: %lf %lf \n", i, splashes[i]->rad_current, splashes[i]->rad_end);
#endif

        splashes[i]->rad_current += splashes[i]->rad_step;
        // Splash reached it's destination (exceeded predefined boundary), don't process / draw it anymore
        if ((splashes[i]->rad_step > 0 && splashes[i]->rad_current > splashes[i]->rad_end) ||
            (splashes[i]->rad_step < 0 && splashes[i]->rad_current < splashes[i]->rad_end)) {

#if DEBUG == 2
          printf("Deactivating splash %zu\n", i);
#endif

          splashes[i]->is_active = 0;
        } else {
          // Recalc next point for circular motion
          double y = HEIGHT - sin(splashes[i]->rad_current) * splashes[i]->r;
          double x = splashes[i]->cx + cos(splashes[i]->rad_current) * splashes[i]->r;


#if DEBUG == 2
          printf("Curr %zu splash XY: %lf %lf \n", i, splashes[i]->x, splashes[i]->y);
#endif

          // Same color as ancestor's raindrop
          SDL_SetRenderDrawColor(sdl->renderer, drops[i]->r, drops[i]->g, drops[i]->b, SDL_ALPHA_OPAQUE);
          // Sometimes 2px line will be drawed instead of single point for better effect
          SDL_RenderDrawLine(sdl->renderer,
                             (int) splashes[i]->x, (int) splashes[i]->y,
                             (int) x, (int) y
          );

          splashes[i]->x = x;
          splashes[i]->y = y;
        }
      }

#if DEBUG == 1
      printf("RGB%d%d%d \n", ps[i]->r, ps[i]->g, ps[i]->b);
#endif

      SDL_SetRenderDrawColor(sdl->renderer, drops[i]->r, drops[i]->g, drops[i]->b, SDL_ALPHA_OPAQUE);
      SDL_RenderDrawLine(sdl->renderer,
                         (int) drops[i]->x, (int) drops[i]->y,
                         (int) drops[i]->x, (int) (drops[i]->y + drops[i]->length));
    }

    SDL_RenderPresent(sdl->renderer);

    if (SDL_PollEvent(sdl->event) != 0) {
      switch (sdl->event->type) {
        case SDL_QUIT:
          printf("Received a signal to quit\n");
          is_running = 0;
          break;
        default:
          break;
      }
    }
  }

  shutdown_sdl(sdl);
  return 0;
}
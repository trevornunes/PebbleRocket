#pragma once
#include <pebble.h>
  

#define SPRITE_MAX 8

typedef struct sprite {
  GBitmap  *img;
  int handle_id;
  int x;
  int y;
  int h;
  int w;
  int offset_x;
  int offset_y;
} sprite_t;


#define FPS30 34
#define FPS15 66
#define FPS10 100
#define FPS5  200
#define FPS1  1000

extern int TICK_RATE_MS;
  
void update_battery();
void update_time(char *buffer_p);

sprite_t *sprite_add( int id, int w, int h, int x, int y );
void      sprite_draw(int id, Layer *layer, GContext *ctx);
sprite_t *sprite_get(int id);
void      sprite_free( int id );




#include <pebble.h>
#include "utils.h"

static BatteryChargeState charge_state;

int TICK_RATE_MS = FPS30 ; 

static int      s_spriteCount = 0;
// static int      s_spritesActive[ SPRITE_MAX ];
static sprite_t s_sprite_q[ SPRITE_MAX ];
//
// update_battery
//
void update_battery()
{
   charge_state = battery_state_service_peek();
  
   if( charge_state.charge_percent >= 90)
            TICK_RATE_MS = 34;
  
  if( charge_state.charge_percent < 90)
            TICK_RATE_MS = 66;
  
  if( charge_state.charge_percent < 65)
            TICK_RATE_MS=100;
  
  if( charge_state.charge_percent < 35)
            TICK_RATE_MS=250;

  if( charge_state.charge_percent <= 10) 
            TICK_RATE_MS=500;
}


//
// update_time
//
void update_time(char *buffer_p) {
  
  if(!buffer_p)
    return;
  
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
        
  if(clock_is_24h_style() == true) {
      strftime(buffer_p, sizeof("00:00"), "%H:%M", tick_time);
  } else {
      strftime(buffer_p, sizeof("00:00"), "%I:%M", tick_time);
  }
}

//
//
//
sprite_t *sprite_add( int id, int w, int h, int x, int y )
{
    sprite_t *spr;
    if( s_spriteCount++ > SPRITE_MAX ) {
     APP_LOG(APP_LOG_LEVEL_ERROR,"sprite_add: max sprites reached");
     return NULL;
    }
  
    spr = &s_sprite_q[s_spriteCount];
  
    spr->handle_id = id;
    spr->img = gbitmap_create_with_resource(id);
    spr->x = x;
    spr->y = y;
    spr->h = h;
    spr->w = w;
    spr->offset_x = h/4;
    spr->offset_y = w/4;
  
    return spr;
}


//
//
//
void sprite_draw(int id, Layer *layer, GContext *ctx) {
  sprite_t *s = sprite_get(id);
  graphics_context_set_compositing_mode(ctx, GCompOpAssign); 
  graphics_draw_bitmap_in_rect(ctx, s->img , GRect( s->x, s->y, s->w, s->h));
}


//
//
//
sprite_t *sprite_get(int id) {
  int i = 0;
  
  for( i = 0; i < SPRITE_MAX; i++) {
         if( s_sprite_q[i].handle_id == id ) {
            return &s_sprite_q[i];
         }
  }
  
  APP_LOG( APP_LOG_LEVEL_ERROR,"sprite_get: NULL");
  return NULL;
}

//
//
//
void sprite_free(int id) {
    sprite_t *spr = sprite_get(id);
    if( spr->img ) { 
       gbitmap_destroy( spr->img );
       spr->handle_id = 0;
       spr->img = 0;
    }
}
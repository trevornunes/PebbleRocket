#include <pebble.h>
#include <math.h>
#include "comms.h"
#include "utils.h"
  

// Rocket patterns...
  
#define MODE_START   1
#define MODE_SHAKE   2
#define MODE_TAKEOFF 3
#define MODE_LAND    4
#define MODE_MAX     5

static int s_mode;


// Sprite/Gfx stuff ...
static sprite_t    *s_sprite;
static BitmapLayer *s_bitmap_layer;
static Layer       *s_canvas;
static Window      *s_main_window;
static GBitmap *s_bmap_bg;

  static int angle = 0;
  static float zigZagDepth = 3;
  static int enableZigZagging = 0;

  static int x_scaling_factor =   80000;
  static int y_scaling_factor = 1000000; 

// Timing crap ...

static int  s_oneSecondElapse = 0;
static int  s_enableParticles = 1;
static void tick_fast_handler(void *context);
static void updateFast(Layer *layer, GContext *ctx);

// Shitty Particle Fountain ...

#define MAX_DOTS 250
#define GRAVITY  9.82
  
int dotX[ MAX_DOTS ];
int dotY[ MAX_DOTS ];
float dotVelocity[ MAX_DOTS ];


//
// main_window_load
//
static void main_window_load(Window *window) {
  
  int i = 0;
  for( i = 0; i < MAX_DOTS;i++ ) {
     dotX[i] = -1;
     dotY[i] = -1;
     dotVelocity[i] = 1;
  }
  
   comms_init();  // 

  // Create black background ...
  s_bmap_bg = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_EARTH);
  // bitmap_layer_set_background_color( s_bmap_bg, GColorBlack );
  
  s_canvas = layer_create(GRect(0, 0, 144, 168));   
  s_sprite = sprite_add( RESOURCE_ID_IMAGE_ROCKET, 30, 42, 50, 100 );
  s_sprite->offset_y = 40;
  
  Layer* motherLayer = window_get_root_layer(s_main_window);
  layer_set_update_proc(s_canvas, updateFast);
  
  // Black background.
  // yer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bmap_bg));
  // Fast animation layer.
  layer_add_child(motherLayer, s_canvas);
  
  TICK_RATE_MS = FPS30;
  app_timer_register( TICK_RATE_MS , tick_fast_handler, NULL);
}


//
// main_window_unload
//
static void main_window_unload(Window *window) {
  sprite_free( RESOURCE_ID_IMAGE_ROCKET );
  layer_destroy(s_canvas);
  bitmap_layer_destroy(s_bitmap_layer);
}


// 
// tick_fast_handler
//
static void tick_fast_handler(void *context) {
   layer_mark_dirty(s_canvas); 
   app_timer_register( TICK_RATE_MS , tick_fast_handler, NULL);
}


static void update_state(void)
{
  static int timeDelta = 0;
  static int yMove = 1;
  
  timeDelta += TICK_RATE_MS;
  if( timeDelta >= 1000) {
      if( s_oneSecondElapse++ >= 5) {  // mode cycler ...
        s_oneSecondElapse = 0;
        if( s_mode++ >= MODE_MAX)
            s_mode = MODE_START;
      } 
      timeDelta = 0;
  }
  
  if( angle++ >= 360 ) {
     angle = 0;
  }
  
  switch(s_mode) {
      case MODE_SHAKE:
           s_sprite->x = 5 + sin( (angle * 180.0 / 3.1415967)) * 2;
           s_sprite->x = 5 + cos( (angle * 180.0 / 3.1415967)) * 2;
           zigZagDepth = 5.0;
           enableZigZagging = 1;
           break;
     
      case MODE_TAKEOFF:
           if(yMove-- < 1) {
             yMove = 1;
           }
           s_sprite->x = 55;
           s_sprite->y = yMove;
           zigZagDepth = 3.0;
           enableZigZagging = 1;
           break;
    
      case MODE_LAND:
           if(yMove++ >= 83) {
             yMove = 83;
             zigZagDepth = 10.0;
           }
           s_sprite->x = 55;
           s_sprite->y = yMove;
           zigZagDepth = 3;
           enableZigZagging = 1;
           break;
     
      case MODE_START:
           if( s_oneSecondElapse == 0 ) {
                s_sprite->y = 1 + rand() % 100;
                s_sprite->x = 1 + rand() % 80;
              zigZagDepth = 3;
           }
           break;
  }
  
}

 
// shitty particle fountain.
// 
// velocity = (9.82 * delta-time);  // gravity * time-elapsed which is 9.82M/s * (updateDots-Time-Between-Calls == 34ms)
// Y pos = y pos + (velocity * delta-time); // Y pos increase ie toward the bottom/ground is y+= velocity-calc * (updateDots-Time-Between-Calls == 34ms)
// We scale the value back to fit the pixel range .. it's munged.
  
static void update_particles(Layer *layer, GContext *ctx)
{
  int i = 0;       
  for( i = 0 ; i < MAX_DOTS; i++ ) {
        dotVelocity[i] += GRAVITY * TICK_RATE_MS;
       // dotY[i] += 1.0/(dotVelocity[i] * TICK_RATE_MS)*80000;   // fix these scaling hacks.
       // dotX[i] += 1.0 + (dotVelocity[i] * TICK_RATE_MS)/100000;
        dotY[i] += 1.0/(dotVelocity[i] * TICK_RATE_MS)*x_scaling_factor;   // fix these scaling hacks.
        dotX[i] += 1.0 + (dotVelocity[i] * TICK_RATE_MS)/y_scaling_factor;
    
     //   dotY[i] += (dotVelocity[i] * TICK_RATE_MS)/s_scaling_factor;   // fix these scaling hacks.
     //   dotX[i] +=  (dotVelocity[i] * TICK_RATE_MS)/s_scaling_factor;
    
      //  dotX[i] += sin( angle * 180.0/3.1415967) * 4;
    
      if( enableZigZagging ) {
          dotX[i] += sin( i * 180.0 / 3.1415967) * zigZagDepth; // give it some zig-zag.
          dotY[i] += cos( i * 180.0 / 3.1415967) * zigZagDepth; // give it some zig-zag.
      }


     if ( (dotX[i] >= 144)  || (dotX[i] <= 1) ) {
         dotX[i] = s_sprite->x + s_sprite->offset_x + rand() % 10; 
         dotY[i] = s_sprite->y + s_sprite->offset_y + rand() % 15;  
         dotVelocity[i] = 1 + (rand() % 2)/0.3;
     }
    
     if( (dotY[i]  >= 168) || (dotY[i] <= 1) ) {
         dotX[i] = s_sprite->x + s_sprite->offset_x + rand() % 10;  
         dotY[i] = s_sprite->y + s_sprite->offset_y + rand() % 15; 
         dotVelocity[i] = 1 + (rand() % 2)/0.3;
     } 
   graphics_draw_pixel(ctx,GPoint( dotX[i], dotY[i]));  
  }
}
  


static void updateFast(Layer *layer, GContext *ctx)
{  
update_state();    
  
graphics_context_set_compositing_mode(ctx, GCompOpAssign); 
graphics_draw_bitmap_in_rect(ctx, s_bmap_bg, GRect(0,0,144, 168));
  

 graphics_context_set_compositing_mode(ctx, GCompOpAssign); 
 sprite_draw( RESOURCE_ID_IMAGE_ROCKET, layer,ctx );
  
  if( s_enableParticles ) 
  {
     graphics_context_set_stroke_color(ctx,GColorWhite); // ugh, fixme.
     update_particles(layer,ctx);
  }
}


//
// init
//
static void init() {
  
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
 
  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  s_mode = MODE_START;
}

//
// deinit
//
static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

//
// main
//
int main(void) {
  init();
  app_event_loop();
  deinit();
}

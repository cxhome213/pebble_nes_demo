#include <pebble.h>
#include "nes/nes_main.h"

#define COLORS       true
#define ANTIALIASING true

static Window *s_main_window;
Layer *s_canvas_layer;

static GPoint s_center;

int flag = 1;

static bool animating = true;
static int animation_direction = 1;

static Animation* s_globe_animation;
#define ANIMATION_DURATION 50000000
#define ANIMATION_INITIAL_DELAY 50


static void anim_started_handler(Animation* anim, void* context) {
  
  animating = true;
  //animation_count = 0;
  tick_timer_service_unsubscribe();
}

static void anim_stopped_handler(Animation* anim, bool finished, void* context) {
  animation_destroy(anim);
  animating = false;
  //reset_ticks();
  //APP_LOG(APP_LOG_LEVEL_INFO, "Animation count %d", animation_count);
}


static void anim_update_handler(Animation* anim, AnimationProgress progress) {
  
  if(s_canvas_layer) {
    layer_mark_dirty(s_canvas_layer);
  }
}


static AnimationImplementation spin_animation = {
   .update = anim_update_handler
};

void spin_globe(int delay, int direction) {
  s_globe_animation = animation_create();
  animation_set_delay((Animation*)s_globe_animation, delay);
  animation_set_duration((Animation*)s_globe_animation, ANIMATION_DURATION);
  animation_set_curve((Animation*)s_globe_animation, delay != 0 ? AnimationCurveEaseInOut : AnimationCurveEaseOut);
  animation_set_handlers((Animation*)s_globe_animation, (AnimationHandlers) {
    .started = anim_started_handler,
    .stopped = anim_stopped_handler
  }, NULL);
  //animation_direction = direction;
  animation_set_implementation((Animation*)s_globe_animation, &spin_animation);
  animation_schedule((Animation*)s_globe_animation);
}


static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  // Redraw
  if(s_canvas_layer) {
    layer_mark_dirty(s_canvas_layer);
  }
}

static void update_proc(Layer *layer, GContext *ctx) {

	if(flag++ == 1)
	{
		nes_main(ctx);
	}

	NesFrameCycle(ctx);

	/*GPoint test_point = (GPoint) {
    .x = 10,
    .y = 10,
  };

	
	graphics_context_set_stroke_color(ctx, GColorFromHEX(0x00EBDB));
	test_point.x = flag;
	graphics_draw_pixel(ctx, test_point);*/

	//layer_mark_dirty(s_canvas_layer);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  s_center = grect_center_point(&window_bounds);

  s_canvas_layer = layer_create(window_bounds);
  layer_set_update_proc(s_canvas_layer, update_proc);
  layer_add_child(window_layer, s_canvas_layer);
}

static void window_unload(Window *window) {
	//tick_timer_service_unsubscribe();
  layer_destroy(s_canvas_layer);
}

static void init() {

  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_main_window, true);

	//tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

	spin_globe(ANIMATION_INITIAL_DELAY, 1);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}

#include <pebble.h>

static Window *window;
static TextLayer *text_layer;
static TextLayer *text_layer2;
static char buffer[] = "00:00";
static char buffer2[] = "00:00";

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  // Format the buffer string using tick_time as the time source
  if (clock_is_24h_style()) {
      strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
      strftime(buffer2, sizeof("00:00"), "%H:%M", tick_time);
  }
  else {
      strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
      strftime(buffer2, sizeof("00:00"), "%I:%M", tick_time);
  }

  // Change the TextLayer text to show the new time
  text_layer_set_text(text_layer, buffer);
  text_layer_set_text(text_layer2, buffer2);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, 70 } });
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  text_layer_set_background_color(text_layer, GColorBlack);
  text_layer_set_text_color(text_layer, GColorWhite);
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  text_layer2 = text_layer_create((GRect) { .origin = { 0, 75 }, .size = { bounds.size.w, 70 } });
  text_layer_set_text_alignment(text_layer2, GTextAlignmentCenter);
  text_layer_set_background_color(text_layer2, GColorBlack);
  text_layer_set_text_color(text_layer2, GColorWhite);
  text_layer_set_font(text_layer2, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(text_layer2));

  // Get a time structure so that the face doesn't start blank
  struct tm *t;
  time_t temp;
  temp = time(NULL);
  t = localtime(&temp);

  // Manually call the tick handler when the window is loading
  handle_minute_tick(t, MINUTE_UNIT);
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
  text_layer_destroy(text_layer2);
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_set_background_color(window, GColorBlack);
  window_stack_push(window, animated);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

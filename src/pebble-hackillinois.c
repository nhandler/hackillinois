#include <pebble.h>

static Window *window;
static Window *rocket_window;
static TextLayer *time_layer;
static TextLayer *rem_layer;
static BitmapLayer *logo_layer;
static GBitmap *logo;
static BitmapLayer *rocket_layer;
static GBitmap *rocket;
static char timeBuff[] = "00:00";
static char *remBuff;
static PropertyAnimation *prop_animation;
static const uint32_t const segments[] = { 400, 100, 400, 100, 400 };
static VibePattern pat = {
  .durations = segments,
  .num_segments = ARRAY_LENGTH(segments),
};
static BitmapLayer *star_layer;
static GBitmap *stars;
static PropertyAnimation *star_prop_animation;

static void animation_started(Animation *animation, void *data) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Animation Started");
  //vibes_long_pulse();
  //vibes_enqueue_custom_pattern(pat);
}

static void animation_stopped(Animation *animation, bool finished, void *data) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Animation Stopped");
  vibes_cancel();
  window_stack_remove(rocket_window, true);
}

static void destroy_property_animation(PropertyAnimation **prop_animation) {
  if (*prop_animation == NULL) {
    return;
  }

  if (animation_is_scheduled((Animation*) *prop_animation)) {
    animation_unschedule((Animation*) *prop_animation);
  }

  property_animation_destroy(*prop_animation);
  *prop_animation = NULL;
}

char *itoa(int num)
{
  static char buff[20] = {};
  int i = 0, temp_num = num, length = 0;
  char *string = buff;
  
  if(num >= 0) {
    // count how many characters in the number
    while(temp_num) {
      temp_num /= 10;
      length++;
    }
    
    // assign the number to the buffer starting at the end of the 
    // number and going to the begining since we are doing the
    // integer to character conversion on the last number in the
    // sequence
    for(i = 0; i < length; i++) {
      buff[(length-1)-i] = '0' + (num % 10);
      num /= 10;
    }
    buff[i] = '\0'; // can't forget the null byte to properly end our string
  }
  else
    return "Unsupported Number";
  
  return string;
}

static void takeoff() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "pushing");
  window_stack_push(rocket_window, true);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "done pushing");
}

static void updateRemTime() {
  time_t secs_epoch_time = time(NULL);
  int secs_epoch = secs_epoch_time + 18000; // Add 5 hours to deal with lack of timezones
  int end_epoch;
  int rem;

  if (secs_epoch < 1397257200) {  // If we are before April 11, 2014 @ 18:00:00
    end_epoch = 1397257200;
  }
  else {
    end_epoch = 1397401200; // If HackIllinois has started, end at April 13, 2014 @ 10:00:00
  }
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Secs: %d", secs_epoch);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "End: %d", end_epoch);
  if (secs_epoch >= end_epoch) {
    rem = 0;
  }
  else {
    rem = end_epoch - secs_epoch;
  }

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Rem: %d", rem);

  int secondsInAMinute = 60;
  int secondsInAnHour  = 60 * secondsInAMinute;
  int secondsInADay    = 24 * secondsInAnHour;

  int days = rem / secondsInADay;

  rem = rem % secondsInADay;

  int hours = rem / secondsInAnHour;

  rem = rem % secondsInAnHour;

  int minutes = rem / secondsInAMinute;

  rem = rem % secondsInAMinute;

  int seconds = rem;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Days: %d, Hours: %d, Minutes: %d, Seconds: %d", days, hours, minutes, seconds);

  strcpy(remBuff, "");
  if (days > 0) {
    strcat(remBuff, itoa(days));
    strcat(remBuff, "d");
  }
  strcat(remBuff, " ");
  if (hours > 0) {
    strcat(remBuff, itoa(hours));
    strcat(remBuff, "h");
  }
  strcat(remBuff, " ");
  if (minutes > 0) {
    strcat(remBuff, itoa(minutes));
    strcat(remBuff, "m");
  }
  /*strcat(remBuff, " ");
  if (seconds > 0) {
    strcat(remBuff, itoa(seconds));
    strcat(remBuff, "s");
  }*/
  APP_LOG(APP_LOG_LEVEL_DEBUG, "remBuff: %s", remBuff);
  text_layer_set_text(rem_layer, remBuff);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  if (clock_is_24h_style()) {
    strftime(timeBuff, sizeof("00:00"), "%H:%M", tick_time);
  }
  else {
    strftime(timeBuff, sizeof("00:00"), "%l:%M", tick_time);
  }

  // Change the TextLayer text to show the new time
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Setting timeBuff: %s", timeBuff);
  text_layer_set_text(time_layer, timeBuff);

  updateRemTime();
}

void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  takeoff();
}

static void rocket_window_load(Window *rocket_window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "rocket_window_load called");
  tick_timer_service_unsubscribe();
  accel_tap_service_unsubscribe();

  Layer *rocket_window_layer = window_get_root_layer(rocket_window);
  GRect bounds = layer_get_bounds(rocket_window_layer);

  stars = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STARS);

  star_layer = bitmap_layer_create((GRect) { .origin = { 0 , -168 }, .size = { bounds.size.w, bounds.size.h*2 } });
  bitmap_layer_set_bitmap(star_layer, stars);
  bitmap_layer_set_alignment(star_layer, GAlignCenter);
  bitmap_layer_set_background_color(star_layer, GColorBlack);
  layer_add_child(rocket_window_layer, bitmap_layer_get_layer(star_layer));

  rocket = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ROCKET);

  rocket_layer = bitmap_layer_create((GRect) { .origin = { 0 , 168+62 }, .size = { bounds.size.w, 62 } });
  bitmap_layer_set_bitmap(rocket_layer, rocket);
  bitmap_layer_set_alignment(rocket_layer, GAlignCenter);
  bitmap_layer_set_background_color(rocket_layer, GColorBlack);
  layer_add_child(rocket_window_layer, bitmap_layer_get_layer(rocket_layer));

  GRect stars_to_rect = GRect(0,0,bounds.size.w, bounds.size.h*2);
  destroy_property_animation(&star_prop_animation);
  star_prop_animation = property_animation_create_layer_frame(bitmap_layer_get_layer(star_layer), NULL, &stars_to_rect);
  animation_set_duration((Animation*) star_prop_animation, 2000);
  animation_set_curve((Animation*) star_prop_animation, AnimationCurveLinear);
  animation_set_handlers((Animation*) star_prop_animation, (AnimationHandlers) {
    .started = NULL,
    .stopped = NULL
  }, NULL /* callback data */);
  animation_schedule((Animation*) star_prop_animation);

  GRect to_rect = GRect(0,0-62,bounds.size.w, 62);
  destroy_property_animation(&prop_animation);
  prop_animation = property_animation_create_layer_frame(bitmap_layer_get_layer(rocket_layer), NULL, &to_rect);
  animation_set_duration((Animation*) prop_animation, 2000);
  animation_set_curve((Animation*) prop_animation, AnimationCurveEaseIn);
  animation_set_handlers((Animation*) prop_animation, (AnimationHandlers) {
    .started = (AnimationStartedHandler) animation_started,
    .stopped = (AnimationStoppedHandler) animation_stopped,
  }, NULL /* callback data */);
  animation_schedule((Animation*) prop_animation);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "rocket_window_load done");
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  remBuff = malloc(sizeof(char)*20);

  time_layer = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, 45 } });
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  text_layer_set_background_color(time_layer, GColorBlack);
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(time_layer));

  rem_layer = text_layer_create((GRect) { .origin = { 0, 45 }, .size = { bounds.size.w, 30 } });
  text_layer_set_text_alignment(rem_layer, GTextAlignmentCenter);
  text_layer_set_background_color(rem_layer, GColorBlack);
  text_layer_set_text_color(rem_layer, GColorWhite);
  text_layer_set_font(rem_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(rem_layer));

  logo = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_LOGO);

  // The bitmap layer holds the image for display
  logo_layer = bitmap_layer_create((GRect) { .origin = { 0 , 90 }, .size = { bounds.size.w, 70 } });
  bitmap_layer_set_bitmap(logo_layer, logo);
  bitmap_layer_set_alignment(logo_layer, GAlignCenter);
  bitmap_layer_set_background_color(logo_layer, GColorBlack);
  layer_add_child(window_layer, bitmap_layer_get_layer(logo_layer));

  // Get a time structure so that the face doesn't start blank
  struct tm *t;
  time_t temp;
  temp = time(NULL);
  t = localtime(&temp);

  // Manually call the tick handler when the window is loading
  handle_minute_tick(t, MINUTE_UNIT);
}

static void rocket_window_unload(Window *rocket_window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "rocket_window_unload called");
  gbitmap_destroy(stars);
  bitmap_layer_destroy(star_layer);
  gbitmap_destroy(rocket);
  bitmap_layer_destroy(rocket_layer);
  destroy_property_animation(&prop_animation);
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  accel_tap_service_subscribe(&accel_tap_handler);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "rocket_window_unload done");
}

static void window_unload(Window *window) {
  gbitmap_destroy(logo);
  bitmap_layer_destroy(logo_layer);
  text_layer_destroy(time_layer);
  free(remBuff);
  text_layer_destroy(rem_layer);
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

  rocket_window = window_create();
  window_set_window_handlers(rocket_window, (WindowHandlers) {
    .load = rocket_window_load,
    .unload = rocket_window_unload,
  });
  window_set_background_color(rocket_window, GColorBlack);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  accel_tap_service_subscribe(&accel_tap_handler);
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  accel_tap_service_unsubscribe();
  window_destroy(rocket_window);
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

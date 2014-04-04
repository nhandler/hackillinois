#include <pebble.h>

static Window *window;
static TextLayer *time_layer;
static TextLayer *rem_layer;
static BitmapLayer *image_layer;
static GBitmap *image;
static char timeBuff[] = "00:00";
static char *remBuff;

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

  image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_LOGO);

  // The bitmap layer holds the image for display
  image_layer = bitmap_layer_create((GRect) { .origin = { 0 , 90 }, .size = { bounds.size.w, 70 } });
  bitmap_layer_set_bitmap(image_layer, image);
  bitmap_layer_set_alignment(image_layer, GAlignCenter);
  bitmap_layer_set_background_color(image_layer, GColorBlack);
  layer_add_child(window_layer, bitmap_layer_get_layer(image_layer));


  // Get a time structure so that the face doesn't start blank
  struct tm *t;
  time_t temp;
  temp = time(NULL);
  t = localtime(&temp);

  // Manually call the tick handler when the window is loading
  handle_minute_tick(t, MINUTE_UNIT);
}

static void window_unload(Window *window) {
  gbitmap_destroy(image);
  bitmap_layer_destroy(image_layer);
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

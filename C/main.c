#include <pebble.h>

const int ACC_TAP = 0;
const int ACC_DATA = 1;
const int LIGHTS = 2;
const int RIGHT = 3;
const int LEFT = 4;

static Window *window;
static TextLayer *text_layer;
static int cnt = 0;

static void send_message(int key, int value){
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  if(!iter) return;

  dict_write_int(iter, key, &value, sizeof(int), true);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void send_comm_accel(char arr[]){
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  if(!iter) return;

  dict_write_cstring(iter, ACC_DATA, arr);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  vibes_long_pulse();
  send_message(ACC_TAP, 0);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(cnt == 0){
    text_layer_set_text(text_layer, "Sending Lights");
    send_message(LIGHTS, 1);
  }
  else if(cnt == 1){
    text_layer_set_text(text_layer, "Sending Right");
    send_message(RIGHT, 2);
  }
  else {
    text_layer_set_text(text_layer, "Sending Left");
    send_message(LEFT, 3);
  }
}

static void accel_data_handler(AccelData *data, uint32_t num_samples) {
  // Read sample 0's x, y, and z values
  int16_t x = data[0].x;
  int16_t y = data[0].y;
  int16_t z = data[0].z;

  static char s_buff[32];

  // Determine if the sample occured during vibration, and when it occured
  bool did_vibrate = data[0].did_vibrate;

  if(!did_vibrate) {
    snprintf(s_buff, sizeof(s_buff), "x=%d,y=%d,z=%d", (int)x, (int)y, (int)z);
    send_comm_accel(s_buff);
  } else {
    // Discard with a warning
    APP_LOG(APP_LOG_LEVEL_WARNING, "Vibration occured during collection");
  }
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(text_layer, "Up");
  if(cnt >= 2){
    cnt = 2;
  }
  else {
    cnt = cnt + 1;
  }

  if(cnt == 0) {
    text_layer_set_text(text_layer, "Lights");
  }
  else if(cnt == 1) {
    text_layer_set_text(text_layer, "Slide: right");
  }
  else {
    text_layer_set_text(text_layer, "Slide: left");
  }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(cnt <= 0){
    cnt = 0;
  }
  else {
    cnt = cnt - 1;
  }

  if(cnt == 0) {
    text_layer_set_text(text_layer, "Lights");
  }
  else if(cnt == 1) {
    text_layer_set_text(text_layer, "Slide: right");
  }
  else {
    text_layer_set_text(text_layer, "Slide: left");
  }
}

static void compass_heading_handler(CompassHeadingData heading_data) {
  // Is the compass calibrated?
  switch (heading_data.compass_status) {
    case CompassStatusDataInvalid:
      APP_LOG(APP_LOG_LEVEL_INFO, "Not yet calibrated.");
      break;
    case CompassStatusCalibrating:
      APP_LOG(APP_LOG_LEVEL_INFO, "Calibration in progress. Heading is %ld",
                TRIGANGLE_TO_DEG(TRIG_MAX_ANGLE - heading_data.magnetic_heading));
      break;
    case CompassStatusUnavailable:
      APP_LOG(APP_LOG_LEVEL_INFO, "Calibration is unavailable");
      break;
    case CompassStatusCalibrated:
      APP_LOG(APP_LOG_LEVEL_INFO, "Calibrated! Heading is %ld",
                TRIGANGLE_TO_DEG(TRIG_MAX_ANGLE - heading_data.magnetic_heading));
      break;
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create(GRect(0, 72, bounds.size.w, 20));
  text_layer_set_text(text_layer, "Lights");
  cnt = 0;
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  uint32_t num_samples = 1;  // Number of samples per batch/callback

  // Subscribe to batched data events
  //accel_data_service_subscribe(num_samples, accel_data_handler);
  compass_service_subscribe(compass_heading_handler);
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
  //accel_data_service_unsubscribe();
  compass_service_unsubscribe();
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init(void) {

  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_open(app_message_inbox_size_maximum() - 100, app_message_outbox_size_maximum() - 100);

  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
  accel_tap_service_subscribe(accel_tap_handler);
}

static void deinit(void) {
  window_destroy(window);
  accel_tap_service_unsubscribe();
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}

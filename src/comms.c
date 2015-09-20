#include <pebble.h>
#include "comms.h"

  
#define KEY_COUNT 5

static AppSync s_sync;
static uint8_t s_sync_buffer[32];

static void sync_changed_handler(const uint32_t key, const Tuple *new_tuple, const Tuple *old_tuple, void *context) {
  static char s_count_buffer[32];
  snprintf(s_count_buffer, sizeof(s_count_buffer), "Count: %d", (int)new_tuple->value->int32);
}

static void sync_error_handler(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  // An error occured!
  APP_LOG(APP_LOG_LEVEL_ERROR, "sync error!");
}



void comms_init() {

  // Setup AppSync
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  // Setup initial values
  Tuplet initial_values[] = {
    TupletInteger(KEY_COUNT, 0),
  };

  // Begin using AppSync
  app_sync_init(&s_sync, s_sync_buffer, sizeof(s_sync_buffer), initial_values, ARRAY_LENGTH(initial_values), sync_changed_handler, sync_error_handler, NULL);
}



void comms_shutdown() {
    // Finish using AppSync
  app_sync_deinit(&s_sync);
}


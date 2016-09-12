#ifndef _MOTION_
#define _MOTION_

#include <pebble.h>

// send motion mode to server
void send_motion_mode() {
  // text_layer_set_text(hello_layer, "Selected!");
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  int key = 0;
  // send the message "temperature" to the phone, using key #0
  Tuplet value = TupletCString(key, "proximity");
  dict_write_tuplet(iter, &value);
  app_message_outbox_send();
}

/* This is called when the select button is clicked */
void select_click_handler_motion(ClickRecognizerRef recognizer, void *context) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  int key = 0;
  // send the message "distance" to the phone, using key #0
  Tuplet value = TupletCString(key, "distance");
  dict_write_tuplet(iter, &value);
  app_message_outbox_send();
}

/* this registers the appropriate function to the appropriate button */
void config_provider_motion(void *context) {
  // BUTTON_ID_UP  BUTTON_ID_DOWN BUTTON_ID_SELECT
  window_single_click_subscribe(BUTTON_ID_SELECT,  // middle button
  select_click_handler_motion);
}

#endif // _MOTION_
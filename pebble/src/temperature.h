#ifndef _TEMPERATURE_
#define _TEMPERATURE_

#include <pebble.h>

void send_temp_mode() {
  // text_layer_set_text(hello_layer, "Selected!");
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  int key = 0;
  // send the message "temperature" to the phone, using key #0
  Tuplet value = TupletCString(key, "temp");
  dict_write_tuplet(iter, &value);
  app_message_outbox_send();
}

/* This is called when the select button is clicked */
void select_click_handler_temperature(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(hello_layer, "Selected!");
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  int key = 0;
  // send the message "temperature" to the phone, using key #0
  Tuplet value = TupletCString(key, "temperature");
  dict_write_tuplet(iter, &value);
  app_message_outbox_send();
}

/* This is called when the up button is clicked */
void up_click_handler_temperature(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(hello_layer, "Selected!");
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  int key = 0;
  // send the message "style" to the phone, using key #0
  Tuplet value = TupletCString(key, "style");
  dict_write_tuplet(iter, &value);
  app_message_outbox_send();
}

/* This is called when the down button is clicked */
void down_click_handler_temperature(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(hello_layer, "Selected!");
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  int key = 0;
  // send the message "mode" to the phone, using key #0
  Tuplet value = TupletCString(key, "mode");
  dict_write_tuplet(iter, &value);
  app_message_outbox_send();
}

/* this registers the appropriate function to the appropriate button */
void config_provider_temperature(void *context) {
  // BUTTON_ID_UP  BUTTON_ID_DOWN BUTTON_ID_SELECT
  window_single_click_subscribe(BUTTON_ID_SELECT,  // middle button
  select_click_handler_temperature);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler_temperature); // up button
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler_temperature); // down button
}

#endif // _TEMPERATURE_
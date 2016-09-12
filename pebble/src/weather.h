#ifndef _WEATHER_
#define _WEATHER_

#include <pebble.h>


/* This is called when the select button is clicked */
void select_click_handler_weather(ClickRecognizerRef recognizer, void *context) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  int key = 0;
  // send the message "hello?" to the phone, using key #0
  Tuplet value = TupletCString(key, "weather");
  dict_write_tuplet(iter, &value);
  app_message_outbox_send();
}

/* this registers the appropriate function to the appropriate button */
void config_provider_weather(void *context) {
  // BUTTON_ID_UP  BUTTON_ID_DOWN BUTTON_ID_SELECT
  window_single_click_subscribe(BUTTON_ID_SELECT,  // middle button
  select_click_handler_weather);
}

/* This is called when the select button is clicked */
void weather_sendback() {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  int key = 0;
  // send the message "hello?" to the phone, using key #0
  Tuplet value = TupletCString(key, "weather_sendback");
  dict_write_tuplet(iter, &value);
  app_message_outbox_send();
}

#endif // _WEATHER_
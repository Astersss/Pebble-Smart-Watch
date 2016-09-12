#include <pebble.h>
#include <temperature.h>
#include <movieinfo.h>
#include <motion.h>
#include <weather.h>

// root window with menu layer
static Window *root_window;
static MenuLayer *s_menu_layer;
// child windows with text layers
static Window *windows[4];
static TextLayer *text_layers;
// window state flag
static int window_state;


/**
  This part is for handling message sending and receiving
**/
void out_sent_handler(DictionaryIterator *sent, void *context) {
  // outgoing message was delivered -- do nothing
}

void out_failed_handler(DictionaryIterator *failed,
  AppMessageResult reason, void *context) {
  // outgoing message failed
  text_layer_set_text(text_layers, "Can not send message to cell phone!");
}

void in_dropped_handler(AppMessageResult reason, void *context) {
  // incoming message dropped
  text_layer_set_text(text_layers, "Can not receive message from cell phone!");
}

// temperature,motion message
static char temp_msg[100];
// movie message, weather message
static char* msg[50]; // receive message
static int movie_index = 0;
static int movie_length = 0;

/* receive incoming message */
void in_received_handler(DictionaryIterator *received, void *context) {
  if (window_state == 0 || window_state == 2) { // temperature & motion
    // looks for key #0 in the incoming message
    int key = 0;
    
    Tuple *text_tuple = dict_find(received, key);
    if (text_tuple) {
      if (text_tuple->value) {
        // put it in this global variable
        strcpy(temp_msg, text_tuple->value->cstring);
      } else strcpy(temp_msg, "no value!");
      text_layer_set_text(text_layers, temp_msg);
    } else {
      text_layer_set_text(text_layers, "no message!");
    }
  }
  if (window_state == 1 || window_state == 3) { // movie info & weather info
    // looks for key # in the incoming message
    int key = movie_index;
    Tuple *text_tuple = dict_find(received, key);
    if (text_tuple) {
      if (text_tuple->value) {
        if (key == 0) {
          movie_length = (int)text_tuple->value->int32;
        } else {
          // put it in this global variable
          char* moviename = text_tuple->value->cstring;
          msg[key-1] = malloc(strlen(moviename) + 1);
          strcpy(msg[key-1], moviename);
        }
      } else {
        text_layer_set_text(text_layers, "no value!");
      }
//       text_layer_set_text(text_layers, "Retrieving data");
    } else {
      text_layer_set_text(text_layers, "no message!");
    }
    movie_index ++;
    if (movie_index > movie_length) {
      // show the movies
      char* movies = malloc(2048);
      *movies = '\0';
      int i;
      for (i=0; i<movie_length; i++) {
        strcat(movies, msg[i]);
        strcat(movies, ";\n");
      }
      text_layer_set_text(text_layers, movies);
      free(movies);
      movie_index = 0;
      
      // send back weather info
      if (window_state == 3) {
        weather_sendback();
      }
    }
  }
//   if (window_state == 2) { // motion sensor
//     //TODO
//   }
//   if (window_state == 3) { // weather
//   }
}


/**
  This part is for menu layer which is in the root window.
**/
/* get the number of menu rows */
static uint16_t get_num_rows_callback(MenuLayer *menu_layer, 
                                      uint16_t section_index, void *context) {
  const uint16_t num_rows = 4;
  return num_rows;
}

/* set the name of menu rows */
static void draw_row_callback(GContext *ctx, const Layer *cell_layer, 
                                        MenuIndex *cell_index, void *context) {
  static char s_buff[16];
  if ((int)cell_index->row == 0) {
    snprintf(s_buff, sizeof(s_buff), "Temperature");
  } else if ((int)cell_index->row == 1) {
    snprintf(s_buff, sizeof(s_buff), "Movie Info");
  } else if ((int)cell_index->row == 2) {
    snprintf(s_buff, sizeof(s_buff), "Motion Sensor");
  } else if ((int)cell_index->row == 3) {
    snprintf(s_buff, sizeof(s_buff), "Weather Info");
  }

  // Draw this row's index
  menu_cell_basic_draw(ctx, cell_layer, s_buff, NULL, NULL);
}

/* set height of menu cell */
static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, 
                                        MenuIndex *cell_index, void *context) {
  const int16_t cell_height = 44;
  return cell_height;
}

/* calls when push select button */
static void select_callback(struct MenuLayer *menu_layer, 
                                        MenuIndex *cell_index, void *context) {
  // Do something in response to the button press
  if ((int)cell_index->row == 0) { // Temperature
    // set window state to temperature mode
    window_state = 0;
    // push temperature window to the window stack
    const bool animated = true;
    window_stack_push(windows[0], animated);
    text_layer_set_text(text_layers, "Temperature");
    // send temp command to server
    send_temp_mode();
  } else if ((int)cell_index->row == 1) { // Movie Info
    // set window state to movie info mode
    window_state = 1;
    // push movie window to the window stack
    const bool animated = true;
    window_stack_push(windows[1], animated);
    text_layer_set_text(text_layers, "Movie Info");
  } else if ((int)cell_index->row == 2) { // Motion Sensor
    // set window state to motion sensor mode
    window_state = 2;
    // push motion window to the window stack
    const bool animated = true;
    window_stack_push(windows[2], animated);
    text_layer_set_text(text_layers, "Motion Sensor");
    // send motion command to server
    send_motion_mode();
  } else if ((int)cell_index->row == 3) { // Weather
    // set window state to weather mode
    window_state = 3;
    // push weather window to the window stack
    const bool animated = true;
    window_stack_push(windows[3], animated);
    text_layer_set_text(text_layers, "Weather Info");   
  }
}

/**
  This part is for child windows layer configuration
**/
/* calls when child window loaded */
static void window_load(Window *window) {
  // Create a child window layer
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  // Create a text layer
  text_layers = text_layer_create((GRect)
  { .origin = { 0, 0 },
  .size = { bounds.size.w, 2000 } });
  text_layer_set_text_alignment(text_layers, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(text_layers));
}

/* calls when child window unloaded */
static void window_unload(Window *window) {
  text_layer_destroy(text_layers);
}

/**
  This part is for root window layer configuration
**/
/* calls when root window is loaded */
static void root_window_load(Window *window) {
  // Create a window layer
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  // Create the MenuLayer
  s_menu_layer = menu_layer_create(bounds);

  // Let it receive click events
  menu_layer_set_click_config_onto_window(s_menu_layer, window);

  // Set the callbacks for behavior and rendering
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = get_num_rows_callback,
    .draw_row = draw_row_callback,
    .get_cell_height = get_cell_height_callback,
    .select_click = select_callback,
  });

  // Add to the Window
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

/* calls when root window is unloaded */
static void root_window_unload(Window *window) {
  // Destroy the MenuLayer
  menu_layer_destroy(s_menu_layer);
}

/**
  This part is for overall windows configuration. 
**/
/* initialize windows and configurations */
static void init(void) {
  // initialize root window
  root_window = window_create();
  window_set_window_handlers(root_window, (WindowHandlers) {
    .load = root_window_load,
    .unload = root_window_unload,
  });
  
  // initialize child windows
  for (int i = 0; i < 4; i++) {
    windows[i] = window_create();
    window_set_window_handlers(windows[i], (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  // add listener to temperature window
  window_set_click_config_provider(windows[0], config_provider_temperature);
  // add listener to movie window
  window_set_click_config_provider(windows[1], config_provider_movie);
  // add listener to motion window
  window_set_click_config_provider(windows[2], config_provider_motion);
  // add listener to weather window
  window_set_click_config_provider(windows[3], config_provider_weather);
  
  // for registering AppMessage handlers
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_sent(out_sent_handler);
  app_message_register_outbox_failed(out_failed_handler);
  const uint32_t inbound_size = 64;
  const uint32_t outbound_size = 64;
  app_message_open(inbound_size, outbound_size);
  
  // push root window to the window stack
  const bool animated = true;
  window_stack_push(root_window, animated);
}

/* destroy windows */
static void deinit(void) {
  window_destroy(root_window);
  for (int i = 0; i < 4; i++) {
    window_destroy(windows[i]);
  }
}

int main(void) {
  // initialize windows and message registers
  init();
  // wait for user input
  app_event_loop();
  // destroy windows
  deinit();
}

/********************************************************************
* Project:          MicroRavezWatchface
* Description:      Simple Watchface with the Logo of MircoRavez as Background
* Developer:        GrimbiXcode (David Grimbichler)
* File:             main.c
* First Release:    2015-07-30
********************************************************************/


/********************************************************************
* Includes and defines
*
********************************************************************/
#include <pebble.h>
#include <pebble_fonts.h>
  
#define TAP_NOT_DATA false

/********************************************************************
* Variables
*
********************************************************************/
static Window *s_main_window;
static TextLayer *s_output_layer;

static GRect window_frame;
static bool isRunning;
static bool getStartValue;

/********************************************************************
* Routines
*
********************************************************************/
/*===================================================================
* Name:    data_handler(AccelData *data, uint32_t num_samples)
* Descr.:  
* Input:   AccelData *data, uint32_t num_samples 
* Output:  none
*==================================================================*/
static void data_handler(AccelData *data, uint32_t num_samples) {
  // Long lived buffer
  static char s_buffer[128];
  //tatic int32_t num_average[1];
  static int32_t dist;
  static int32_t x_zero;
  
  if(getStartValue)
    {
    x_zero = ((int32_t)(data[0].x)+(int32_t)(data[1].x)+(int32_t)(data[2].x)+(int32_t)(data[3].x)+(int32_t)(data[4].x))/5;
    getStartValue = false;
    dist = 0;
  }
  // Calculate average
  //num_average[0]=((int32_t)(data[0].x)+(int32_t)(data[1].x)+(int32_t)(data[2].x))/3;
  //num_average[1]=((int32_t)(data[0].y)+(int32_t)(data[1].y)+(int32_t)(data[2].y))/3;  
  //num_average[2]=((int32_t)(data[0].z)+(int32_t)(data[1].z)+(int32_t)(data[2].z))/3;
  for(uint32_t i=0;i<num_samples;i++)
    {
    dist += ((x_zero - (int32_t)(data[i].x))*9.810)/25/1000;
  }
  
  // Compose string of all data
  snprintf(s_buffer, sizeof(s_buffer), 
    "App is running\n\nZero: %d\nDist: %d mm",
    (int)x_zero,
    (int)dist
  );

  //Show the data
  text_layer_set_text(s_output_layer, s_buffer);
}

/*===================================================================
* Name:    data_handler(AccelData *data, uint32_t num_samples)
* Descr.:  
* Input:   AccelAxisType axis, int32_t direction
* Output:  none
*==================================================================*/
static void tap_handler(AccelAxisType axis, int32_t direction) {
  switch (axis) {
  case ACCEL_AXIS_X:
    if (direction > 0) {
      text_layer_set_text(s_output_layer, "X axis positive.");
    } else {
      text_layer_set_text(s_output_layer, "X axis negative.");
    }
    break;
  case ACCEL_AXIS_Y:
    if (direction > 0) {
      text_layer_set_text(s_output_layer, "Y axis positive.");
    } else {
      text_layer_set_text(s_output_layer, "Y axis negative.");
    }
    break;
  case ACCEL_AXIS_Z:
    if (direction > 0) {
      text_layer_set_text(s_output_layer, "Z axis positive.");
    } else {
      text_layer_set_text(s_output_layer, "Z axis negative.");
    }
    break;
  }
}


/*===================================================================
* Callback-Handler
===================================================================*/
void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  //... called on single click, and every 1000ms of being held ...
  //Window *window = (Window *)context;
  static char s_buffer[20];
  if(isRunning)
    {
    isRunning = false;
    accel_data_service_unsubscribe();
    // Compose string of all data
    snprintf(s_buffer, sizeof(s_buffer), 
      "App is stopped"
    );

  //Show the data
  text_layer_set_text(s_output_layer, s_buffer);
  }
  else
    {
    isRunning = true;
    getStartValue = true;
    // Subscribe to the accelerometer data service
    int num_samples = 5;
    accel_data_service_subscribe(num_samples, data_handler);

    // Choose update rate
    accel_service_set_sampling_rate(ACCEL_SAMPLING_25HZ);
  }
}

/*===================================================================
* Callback-Handler
===================================================================*/
void config_provider(Window *window) {
 // single click / repeat-on-hold config:
  window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);

}

/********************************************************************
* Init-code
*
********************************************************************/

/*===================================================================
* load main window
===================================================================*/
static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect frame = window_frame = layer_get_frame(window_layer);

  // Create output TextLayer
  s_output_layer = text_layer_create(GRect(5, 0, frame.size.w - 10, frame.size.h));
  text_layer_set_font(s_output_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text(s_output_layer, "Press select.");
  text_layer_set_overflow_mode(s_output_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(s_output_layer));
}

/*===================================================================
* unload main window
===================================================================*/
static void main_window_unload(Window *window) {
  // Destroy output TextLayer
  text_layer_destroy(s_output_layer);
}

/*===================================================================
* initialising Watchface
===================================================================*/
static void init(void) {
  // Create main Window
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);

  window_set_click_config_provider(s_main_window, (ClickConfigProvider) config_provider);
  
  isRunning = false;
  getStartValue = false;
  
  // Use tap service? If not, use data service
  if (TAP_NOT_DATA) {
    // Subscribe to the accelerometer tap service
    accel_tap_service_subscribe(tap_handler);
  } 
}

/*===================================================================
* deinitialising Watchface
===================================================================*/
static void deinit(void) {
  // Destroy main Window
  window_destroy(s_main_window);

  if (TAP_NOT_DATA) {
    accel_tap_service_unsubscribe();
  } else {
    accel_data_service_unsubscribe();
  }
}

/********************************************************************
* Main-code
********************************************************************/
int main(void) {
  init();
  app_event_loop();
  deinit();
}

/********************************************************************
* End of code
********************************************************************/
#include "pebble.h"
  
Window *window;
Layer *display_layer;
  
void draw_cell(GContext* ctx, GPoint center, bool filled) {
  // Each "cell" represents a binary digit or 0 or 1.
  if (filled) {
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, GRect(center.x - 18, center.y - 11, center.x + 18, center.y + 11), 0, GCornerNone);
  } else {
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(center.x - 18, center.y - 11, center.x + 18, center.y + 11), 0, GCornerNone);
  }
}

#define CELL_SIZE_WIDTH 36 
#define CELL_SIZE_HEIGHT 21
  
#define CELLS_PER_ROW 4
#define CELLS_PER_COLUMN 8
  
GPoint get_center_point_from_cell_location(unsigned short x, unsigned short y) {
  // Cell location (0,0) is upper left, location (4, 8) is lower right.
  return GPoint(
    (CELL_SIZE_WIDTH/2) + (CELL_SIZE_WIDTH * x),
    (CELL_SIZE_HEIGHT/2) + (CELL_SIZE_HEIGHT * y)
  );
}

void draw_cell_row_for_digit(GContext* ctx, unsigned short digit, unsigned short cell_row) {
  // Converts the supplied decimal digit into binary and draws a row of cells.
  // '1' binary values are filled, '0' binary values are not filled.
  int shifter = 0;
  if ((cell_row & 1) != 0) {
    shifter =+ CELLS_PER_ROW;
  }
  for (int cell_column_index = 0; cell_column_index < CELLS_PER_ROW; cell_column_index++) {
    draw_cell(ctx, get_center_point_from_cell_location(cell_column_index, cell_row), (digit >> (cell_column_index + shifter)) & 0x1);
  }
}

// The cell row offsets for each digit
#define HOURS_FIRST_DIGIT_ROW 0
#define HOURS_SECOND_DIGIT_ROW 1
#define MINUTES_FIRST_DIGIT_ROW 2
#define MINUTES_SECOND_DIGIT_ROW 3
#define DAY_FIRST_DIGIT_ROW 4
#define DAY_SECOND_DIGIT_ROW 5
#define MONTH_FIRST_DIGIT_ROW 6
#define MONTH_SECOND_DIGIT_ROW 7

void display_layer_update_callback(Layer *me, GContext* ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  draw_cell_row_for_digit(ctx, t->tm_hour, HOURS_FIRST_DIGIT_ROW);
  draw_cell_row_for_digit(ctx, t->tm_hour, HOURS_SECOND_DIGIT_ROW);
  draw_cell_row_for_digit(ctx, t->tm_min, MINUTES_FIRST_DIGIT_ROW);
  draw_cell_row_for_digit(ctx, t->tm_min, MINUTES_SECOND_DIGIT_ROW);
  draw_cell_row_for_digit(ctx, t->tm_mday, DAY_FIRST_DIGIT_ROW);
  draw_cell_row_for_digit(ctx, t->tm_mday, DAY_SECOND_DIGIT_ROW);
  draw_cell_row_for_digit(ctx, (t->tm_mon + 1), MONTH_FIRST_DIGIT_ROW);
  draw_cell_row_for_digit(ctx, (t->tm_mon + 1), MONTH_SECOND_DIGIT_ROW);
}

void handle_minutes_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(display_layer);
}

static void do_init(void) {
  window = window_create();
  window_stack_push(window, true);
  window_set_background_color(window, GColorBlack);
  Layer *root_layer = window_get_root_layer(window);
  GRect frame = layer_get_frame(root_layer);
  // Init the layer for the display
  display_layer = layer_create(frame);
  layer_set_update_proc(display_layer, &display_layer_update_callback);
  layer_add_child(root_layer, display_layer);
  tick_timer_service_subscribe(MINUTE_UNIT, &handle_minutes_tick);
}

static void do_deinit(void) {
  layer_destroy(display_layer);
  window_destroy(window);
}

int main(void) {
  do_init();
  app_event_loop();
  do_deinit();
}
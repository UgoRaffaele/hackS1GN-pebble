#include "pebble.h"
  
Window *window;
Layer *display_layer;
  
void draw_cell(GContext* ctx, GPoint center, bool filled) {
  // Each "cell" represents a binary digit or 0 or 1.
  if (filled) {
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, GRect(center.x - 18, center.y - 14, center.x + 18, center.y + 14), 0, GCornerNone);
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_draw_rect(ctx, GRect(center.x - 19, center.y - 15, center.x + 19, center.y + 15));
  } else {
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(center.x - 18, center.y - 14, center.x + 18, center.y + 14), 0, GCornerNone);
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_draw_rect(ctx, GRect(center.x - 19, center.y - 15, center.x + 19, center.y + 15));
  }
}

#define CELL_SIZE_WIDTH 36 
#define CELL_SIZE_HEIGHT 28
  
#define CELLS_PER_ROW 4
#define CELLS_PER_COLUMN 6
  
GPoint get_center_point_from_cell_location(unsigned short x, unsigned short y) {
  // Cell location (0,0) is upper left, location (4, 6) is lower right.
  return GPoint(
    (CELL_SIZE_WIDTH/2) + (CELL_SIZE_WIDTH * x),
    (CELL_SIZE_HEIGHT/2) + (CELL_SIZE_HEIGHT * y)
  );
}

void draw_cell_column_for_digit(GContext* ctx, unsigned short digit, unsigned short cell_column) {
  // Converts the supplied decimal digit into binary and draws a column of cells.
  // '1' binary values are filled, '0' binary values are not filled.
  for (int cell_row_index = 0; cell_row_index < CELLS_PER_COLUMN; cell_row_index++) {
    draw_cell(ctx, get_center_point_from_cell_location(cell_column, cell_row_index), (digit >> (cell_row_index)) & 0x1);
  }
}

// The cell column offset for each digit
#define HOURS_DIGIT_COLUMN 0
#define MINUTES_DIGIT_COLUMN 1
#define DAY_DIGIT_COLUMN 2
#define MONTH_DIGIT_COLUMN 3

void display_layer_update_callback(Layer *me, GContext* ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  draw_cell_column_for_digit(ctx, t->tm_hour, HOURS_DIGIT_COLUMN);
  draw_cell_column_for_digit(ctx, t->tm_min, MINUTES_DIGIT_COLUMN);
  draw_cell_column_for_digit(ctx, t->tm_mday, DAY_DIGIT_COLUMN);
  draw_cell_column_for_digit(ctx, (t->tm_mon + 1), MONTH_DIGIT_COLUMN);
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
#include <pebble.h>

#define SECOND_HAND_RADIUS		6
#define MINUTE_HAND_RADIUS		10
#define HOUR_HAND_RADIUS		10
#define SECONDS_CIRCLE_RADIUS	24
#define MINUTES_CIRCLE_RADIUS	72
#define HOURS_CIRCLE_RADIUS		48

// angle of 1/4 hour of hour hand movement
#define QUARTER_HOUR_ANGLE		TRIG_MAX_ANGLE / 48

const GPoint watch_center = { 144/2, 168/2 };

Window *window;
Layer *background_layer;
Layer *seconds_layer;
Layer *minutes_layer;
Layer *hours_layer;

void draw_circle_with_hand (GContext *ctx, int32_t angle, int32_t circle_radius, GColor circle_color,
                            int32_t hand_radius,   GColor hand_color )
{

	GPoint hand;
	int32_t circle_center = circle_radius - ( hand_radius / 2) - (hand_radius / 2);
	
	hand.y = (-cos_lookup(angle) * circle_center / TRIG_MAX_RATIO) + watch_center.y;
	hand.x = (sin_lookup(angle) * circle_center / TRIG_MAX_RATIO) + watch_center.x;
	
	graphics_context_set_fill_color(ctx, hand_color);
	graphics_context_set_stroke_color(ctx, hand_color);
	graphics_fill_circle(ctx, hand, hand_radius);
	
}	
                                            
// Draws the background layer, will only be done once.
void background_layer_update_callback(Layer *lyr, GContext *ctx) {

  	graphics_context_set_fill_color(ctx, GColorWhite);
  	graphics_context_set_stroke_color(ctx, GColorWhite);
	graphics_fill_circle(ctx, watch_center, MINUTES_CIRCLE_RADIUS);

  	graphics_context_set_fill_color(ctx, GColorBlack);
  	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_fill_circle(ctx, watch_center, HOURS_CIRCLE_RADIUS);

  	graphics_context_set_fill_color(ctx, GColorWhite);
  	graphics_context_set_stroke_color(ctx, GColorWhite);
	graphics_fill_circle(ctx, watch_center, SECONDS_CIRCLE_RADIUS);

}
                                            
void seconds_layer_update_callback(Layer *lyr, GContext *ctx) {

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
	int32_t angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
	draw_circle_with_hand(ctx, angle, (int32_t)SECONDS_CIRCLE_RADIUS, GColorWhite,
	                      (int32_t)SECOND_HAND_RADIUS, GColorBlack);
	
}

void minutes_layer_update_callback(Layer *lyr, GContext *ctx) {

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
	int32_t angle = TRIG_MAX_ANGLE * t->tm_min / 60;
	draw_circle_with_hand(ctx, angle, (int32_t)MINUTES_CIRCLE_RADIUS, GColorWhite,
	                      (int32_t)MINUTE_HAND_RADIUS, GColorBlack);
	
}

void hours_layer_update_callback(Layer *lyr, GContext *ctx) {

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
	int32_t angle = (TRIG_MAX_ANGLE * t->tm_hour / 12) + ((t->tm_min / 15) * QUARTER_HOUR_ANGLE);
	draw_circle_with_hand(ctx, angle, (int32_t)HOURS_CIRCLE_RADIUS, GColorBlack,
	                      (int32_t)HOUR_HAND_RADIUS, GColorWhite);

}

void handle_tick(struct tm *t, TimeUnits units_changed) {

	// Update the second hand every second
	layer_mark_dirty(seconds_layer);
	
	// Update the minute hand every minute
	if (t->tm_sec == 0) layer_mark_dirty(minutes_layer);

	// Update the hour hand four times (on the 15) per hour to be able to better see times
	if ((t->tm_min % 15) == 0) layer_mark_dirty(hours_layer);
	
}

void handle_init(void) {

	window = window_create();
	window_set_background_color(window, GColorWhite);
  
    background_layer = layer_create(GRect(0,0,144,168));
	layer_set_update_proc(background_layer, background_layer_update_callback);
	layer_add_child(window_get_root_layer(window), background_layer);
  
	minutes_layer = layer_create(GRect(0,0,144,168));
	layer_set_update_proc(minutes_layer, minutes_layer_update_callback);
	layer_add_child(background_layer, minutes_layer);
  
	hours_layer = layer_create(GRect(0,0,144,168));
	layer_set_update_proc(hours_layer, hours_layer_update_callback);
	layer_add_child(background_layer, hours_layer);
  
	seconds_layer = layer_create(GRect(0,0,144,168));
	layer_set_update_proc(seconds_layer, seconds_layer_update_callback);
	layer_add_child(background_layer, seconds_layer);
  
	tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
	window_stack_push(window, true /* Animated */);
	
	// Force the background and all three hands to be drawn the first time
	layer_mark_dirty(background_layer);
	layer_mark_dirty(seconds_layer);
	layer_mark_dirty(minutes_layer);
	layer_mark_dirty(hours_layer);
	
  
}

int main(void) {

  	handle_init();
	app_event_loop();
	
}
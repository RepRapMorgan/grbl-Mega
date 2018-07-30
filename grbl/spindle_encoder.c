#include "spindle_encoder.h"
#include "timekeeper.h"

static volatile spindle_encoder_state_t spindle_encoder;

ISR(INT4_vect)
{
	uint32_t now = get_timer_ticks();
	spindle_encoder.current_encoder_count++;
	spindle_encoder.speed = 60*1000000L / (spindle_encoder.ticks_per_rev * calculate_dt_micros(spindle_encoder.last_tick_time, now));
	spindle_encoder.last_tick_time = now;
}

// Initializes spindle pins and hardware PWM, if enabled.
void spindle_encoder_init() {
   //PORTE = (1<<PIN5)|(1<<PIN4);
   EICRB = (1<<ISC41); // trigger on falling edge
   EIMSK = (1<<INT4);
   spindle_encoder.current_encoder_count=0;
   spindle_encoder.revolution_counter=0;
   spindle_encoder.last_tick_time=0;
   spindle_encoder.ticks_per_rev=SPINDLE_ENCODER_TICKS_PER_REV;
}

// Returns current spindle angle in  degrees (estimated from encoder inputs and current speed).
float spindle_get_relative_angle() {
    uint32_t now = get_timer_ticks();
    float degrees_per_tick = 360/spindle_encoder.ticks_per_rev;
    // get rough angle based on encoder ticks
    float angle = (degrees_per_tick) * (spindle_encoder.current_encoder_count % spindle_encoder.ticks_per_rev);
    float angle2 = 360.0 * ((float)spindle_encoder.speed/60.0) * ((float)calculate_dt_micros(spindle_encoder.last_tick_time, now) / 1000000.0f );
    if (angle2>degrees_per_tick)  angle2 = degrees_per_tick; // clip angle if it goes too far
    return angle + angle2;
}

// Returns total amount of revolutions since last reset. Warning: this can overflow after some time, reset before use.
uint16_t spindle_get_revolutions() {
	return spindle_encoder.current_encoder_count/spindle_encoder.ticks_per_rev;
}

// Returns current spindle speed in RPM 
int16_t spindle_get_speed() {
	// if spindle is below 30 RPM, cut to 0
	if (calculate_dt_micros(spindle_encoder.last_tick_time, get_timer_ticks())>2000000L / spindle_encoder.ticks_per_rev) {
		spindle_encoder.speed=0;
	}
	return spindle_encoder.speed; 
}



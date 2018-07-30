#include "spindle_sync.h"

static  spindle_sync_state_t spindle_state;

void spindle_sync_update() {
  plan_block_t *plan = plan_get_current_block();
  if ((plan!=NULL) && (plan->condition & PL_COND_FLAG_FEED_PER_REV)) {
    float actual_spindle_speed = spindle_get_speed();
    // replace predefined spindle speed with actual speed
    plan->spindle_speed = actual_spindle_speed;
    plan_update_velocity_profile_parameters();
    plan_cycle_reinitialize();
  }
}

void spindle_sync_wait(float angle) {
    float adjusted_angle = angle;
    if (adjusted_angle<0.0f) { // angle has to be 0 <= angle < 360
        adjusted_angle = (adjusted_angle) + ((int)(-adjusted_angle/360.0f)+1) *360.0; 
    }
    if (adjusted_angle>=360.0f) { 
        adjusted_angle = adjusted_angle - ((int)(adjusted_angle/360.0f)) *360.0;         
    }
    // if we're already past the point, wait for wraparound
    while (spindle_get_relative_angle() > adjusted_angle);
    // wait for the crossing of given angle
    while (spindle_get_relative_angle() < adjusted_angle);
}

void spindle_sync_set_starting_point() {
    int32_t current_position[N_AXIS]; // Copy current state of the system position variable
    memcpy(current_position,sys_position,sizeof(sys_position));
    system_convert_array_steps_to_mpos(spindle_state.last_xyz, current_position);
}

void spindle_sync_wait_for_zero() {
    uint16_t current_revs = spindle_get_revolutions();
    
    while (current_revs == spindle_get_revolutions()) {  // wait spindle position changes
        
    }
    spindle_sync_set_starting_point();
}
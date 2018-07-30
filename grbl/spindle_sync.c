#include "spindle_sync.h"


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
    
}



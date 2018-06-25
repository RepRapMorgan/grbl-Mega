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


uint8_t mc_spindle_sync_cycle(float *target, plan_line_data_t *pl_data, uint8_t parser_flags)
{
  // TODO: Need to update this cycle so it obeys a non-auto cycle start.
  if (sys.state == STATE_CHECK_MODE) { return(GC_PROBE_CHECK_MODE); }

  // Finish all queued commands and empty planner buffer before starting probe cycle.
  protocol_buffer_synchronize();
  if (sys.abort) { return(GC_PROBE_ABORT); } // Return if system reset has been issued.

  // After syncing, check if probe is already triggered. If so, halt and issue alarm.
  // NOTE: This probe initialization error applies to all probing cycles.
  if ( probe_get_state() ) { // Check probe pin state.
    system_set_exec_alarm(EXEC_ALARM_PROBE_FAIL_INITIAL);
    protocol_execute_realtime();
    probe_configure_invert_mask(false); // Re-initialize invert mask before returning.
    return(GC_PROBE_FAIL_INIT); // Nothing else to do but bail.
  }

  // Setup and queue probing motion. Auto cycle-start should not start the cycle.
  mc_line(target, pl_data);

  // Activate the probing state monitor in the stepper module.
  sys_probe_state = PROBE_ACTIVE;

  // Perform probing cycle. Wait here until probe is triggered or motion completes.
  system_set_exec_state_flag(EXEC_CYCLE_START);
  do {
    protocol_execute_realtime();
    if (sys.abort) { return(GC_PROBE_ABORT); } // Check for system abort
  } while (sys.state != STATE_IDLE);


  protocol_execute_realtime();   // Check and execute run-time commands


}
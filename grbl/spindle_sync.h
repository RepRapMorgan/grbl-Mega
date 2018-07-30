#ifndef spindle_sync_h
#define spindle_sync_h

#include "grbl.h"

// data to keep track of axis synchronisation with spindle
typedef struct {
  float last_xyz[N_AXIS];    // keep track of linear axis movements
  float total_distance; // distance traveled since last G33 sync command. Feedrate will be continuously
                        // adjusted to keep total_distance a multiple of K at each full revolution.
} spindle_sync_state_t;

void spindle_sync_update();

// waits for spindle to reach a given angle
void spindle_sync_wait(float angle); 

// blocks until spindle crosses through zero
void spindle_sync_wait_for_zero(); 

#endif
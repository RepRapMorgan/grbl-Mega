#ifndef spindle_sync_h
#define spindle_sync_h

#include "grbl.h"

// data to keep track of axis synchronisation with spindle
typedef struct {
  float last_xyz[3];    // keep track of linear axis movements
  float total_distance; // distance traveled since last G33 sync command. Feedrate will be continuously
                        // adjusted to keep total_distance a multiple of K at each full revolution.
} spindle_sync_state_t;

void spindle_sync_update();
void spindle_sync_wait(float angle); 

#endif
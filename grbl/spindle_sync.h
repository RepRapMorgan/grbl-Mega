#ifndef spindle_sync_h
#define spindle_sync_h

#include "grbl.h"

void spindle_sync_update();
void spindle_sync_wait(float angle); 

#endif
/*
  spindle_control.c - spindle control methods
  Part of Grbl

  Copyright (c) 2012-2017 Sungeun K. Jeon for Gnea Research LLC
  Copyright (c) 2009-2011 Simen Svale Skogsrud

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef spindle_control_h
#define spindle_control_h

typedef struct {
    uint16_t current_encoder_count;
    uint16_t revolution_counter;
    
} spindle_encoder_state_t;

// Initializes spindle pins and hardware PWM, if enabled.
void spindle_encoder_init();

// Returns current spindle angle in 100x degrees (estimated from encoder inputs and current speed).
uint16_t spindle_get_relative_angle();

// Returns total amount of revolutions since last reset. Warning: this can overflow after some time, reset before use.
uint16_t spindle_get_revolutions();

// Returns current spindle speed in RPM 
int16_t spindle_get_speed();

#endif

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <asm/types.h>

#include "systemtime.h"

#define NAXIS 6
#define NBUTTONS 8

struct js_state {
  __s16 axis[NAXIS];
  __u8 button[NBUTTONS];
};


void joystickInit();

void joystickState(js_state *js);

Time lastJoystickUpdate();

#endif

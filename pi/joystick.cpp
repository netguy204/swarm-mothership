#include "joystick.h"
#include "systemtime.h"

#include <poll.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

struct js_event {
  __u32 time;     /* event timestamp in milliseconds */
  __s16 value;    /* value */
  __u8 type;      /* event type */
  __u8 number;    /* axis/button number */
};

#define JS_EVENT_BUTTON         0x01    /* button pressed/released */
#define JS_EVENT_AXIS           0x02    /* joystick moved */
#define JS_EVENT_INIT           0x80    /* initial state of device */

static int jsfd = 0;

void joystickInit() {
  jsfd = 0;
}

static Time lastTime;

Time lastJoystickUpdate() {
  return lastTime;
}

void joystickState(js_state *js) {
  static js_state state;
  static bool initialized = false;

  if(!initialized) {
    memset(&state, sizeof(js_state), 0);
    initialized = true;
  }

  if(jsfd <= 0) {
    // handle device reconnect
    jsfd = open ("/dev/input/js0", O_RDONLY);
    if(jsfd <= 0) {
      //it's normal for the joystick to be unplugged
      //fprintf(stderr, "Cannot read /dev/input/js0: %s\n", strerror(errno));
      return;
    }
  }

  pollfd pfd;
  pfd.fd = jsfd;
  pfd.events = POLLIN;
  int ret;
  while(0 < (ret = poll(&pfd, 1, 0))) {
    js_event event;
    ret = read(jsfd, &event, sizeof(struct js_event));
    if(ret < 0) {
      fprintf(stderr, "js0 read error: %s\n", strerror(errno));
    }
    if(ret != sizeof(struct js_event)) {
      // handle device disconnect
      close(jsfd);
      jsfd = 0;
      return;
    }
    lastTime = Time();
    event.type &= ~JS_EVENT_INIT;
    if(event.type == JS_EVENT_BUTTON && event.number < NBUTTONS) {
      state.button[event.number] = event.value;
    } else if(event.type == JS_EVENT_AXIS && event.number < NAXIS) {
      state.axis[event.number] = event.value;
    }
  }
  if(ret < 0) {
    fprintf(stderr, "js0 event failed: %s\n", strerror(errno));
  }

  *js = state;
}

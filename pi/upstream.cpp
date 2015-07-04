
#include "upstream.h"

const char* UpstreamStateStr[] = {
  UPSTREAM_STATE(UCREATE_STRING)
};

const TimeLength UpstreamFSM::DISCONNECTED_COOLDOWN_TIME = TimeLength::inSeconds(2);
const TimeLength UpstreamFSM::WRITE_AGAIN_COOLDOWN_TIME = TimeLength::inMilliseconds(1);
const TimeLength UpstreamFSM::READ_AGAIN_COOLDOWN_TIME = TimeLength::inMilliseconds(1);
const TimeLength UpstreamFSM::ACK_AGAIN_COOLDOWN_TIME = TimeLength::inMilliseconds(COMMAND_DURATION_MS/2);
const uint8_t UpstreamFSM::MAX_ACK_ATTEMPTS = 20;

TimeLength UpstreamFSM::delayRemaining() {
  return state_duration - (Time() - state_start);
}

bool UpstreamFSM::delayExpired() {
  return (Time() - state_start) > state_duration;
}

void UpstreamFSM::setDelay(const TimeLength& duration) {
  state_start = Time();
  state_duration = duration;
}


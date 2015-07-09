#ifndef MFSM_H
#define MFSM_H

#include "common.h"

#define MAGSTATES(m)   \
  m(STARTUP),      \
  m(CONFIGURE),    \
  m(REQUESTING),   \
  m(RECEIVING),    \
  m(MAX_STATE)

#include "tfsm.h"

class MotorCompensation {
  public:
    virtual const Vector<int>& center() const = 0;
};

class ComplexCompensation : public MotorCompensation {
  public:
    Vector<int> forward, reverse, right, left, stopped;
    TracksFSM& tfsm;

    ComplexCompensation(const Vector<int>& forward, const Vector<int>& reverse, const Vector<int>& right, const Vector<int>& left, TracksFSM& fsm);

    virtual const Vector<int>& center() const;
};

class SimpleCompensation : public MotorCompensation {
  public:
    Vector<int> _center;

    SimpleCompensation(const Vector<int>& _center);

    virtual const Vector<int>& center() const;
};

class MagFSM {
    unsigned long delay_end;
    double accumulator, hdg;

    uint8_t state;
    int8_t address;

    enum State {
      MAGSTATES(MAKE_ENUM)
    };

  public:
    int16_t x, y, z;
    uint8_t updated_data : 1;
    uint8_t first_measurement : 1;

    MagFSM(uint8_t address);

    void ackData();

    long pendingDelay();

    bool delayComplete();

    void update();

    double heading();

    double filteredHeading();
};

#endif



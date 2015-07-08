#ifndef SCONFIG_H
#define SCONFIG_H

// configuration parameters that are unique to an individual hunter

/* To determine:
 * Find the absolute max and min values for the x, y, z components
 * of the detected magnetic field. Then comute (min + max) / 2 for
 * each component.
 */
#define MAG_CENTER Vector<int>(-30, -56, 0)

/* To determine:
 * Run magnetometer calibration. Compute the mean x, y, z values for
 * each state.  Compute the distance of each state to the stopped
 * state.
 */
#define MAG_FORWARD Vector<int>(55, 85, -7)
#define MAG_REVERSE Vector<int>(68, 82, 18)
#define MAG_RIGHT   Vector<int>(28, 90, -12)
#define MAG_LEFT    Vector<int>(92, 84, -7)

/**
  * Who is this in the swarm?
  */
#define SWARM_ID 101

#endif



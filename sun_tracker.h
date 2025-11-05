/*
 * sun_tracker.h
 *
 *  Created on: 5 avr. 2019
 *      Author: Nirgal
 */

#ifndef SUN_TRACKER_SUN_TRACKER_H_
#define SUN_TRACKER_SUN_TRACKER_H_

void SUN_TRACKER_init(void);
void SUN_TRACKER_process_main(void);

void SUN_TRACKER_set_new_pos(double latitude, double longitude);

#endif /* SUN_TRACKER_SUN_TRACKER_H_ */

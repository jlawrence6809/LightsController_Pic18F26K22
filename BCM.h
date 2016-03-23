/* 
 * File:   BCM.h
 * Author: jeremy
 *
 * Created on November 29, 2014, 12:08 AM
 */

#ifndef BCM_H
#define	BCM_H

#ifdef	__cplusplus
extern "C" {
#endif

extern uint8_t BCM[8];
extern uint8_t brightnessVals[8];
extern void initBCM(void);
extern void writeToBCM(uint8_t, uint8_t);
void flashBCM(uint8_t);
void flashBCMStop(void);

#ifdef	__cplusplus
}
#endif

#endif	/* BCM_H */


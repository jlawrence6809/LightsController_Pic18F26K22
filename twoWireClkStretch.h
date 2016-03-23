/* 
 * File:   twoWireClkStretch.h
 * Author: jeremy
 *
 * Created on November 10, 2014, 10:35 PM
 */

#ifndef TWOWIRECLKSTRETCH_H
#define	TWOWIRECLKSTRETCH_H

#ifdef	__cplusplus
extern "C" {
#endif


    void setupSerial(void);
    bool txSerial(uint8_t);
    char rxSerial(unsigned long long, void (*callback)(char));


#ifdef	__cplusplus
}
#endif

#endif	/* TWOWIRECLKSTRETCH_H */


/* 
 * File:   errorsFlags.h
 * Author: jeremy
 *
 * Created on November 29, 2014, 1:01 AM
 */

#ifndef ERRORSFLAGS_H
#define	ERRORSFLAGS_H

#ifdef	__cplusplus
extern "C" {
#endif


extern uint8_t portConflictError;
extern uint8_t serialTimeOutFlag;
extern uint8_t* txByteBufferOverUnderflow;
extern uint8_t serialNoiseCounter;


#ifdef	__cplusplus
}
#endif

#endif	/* ERRORSFLAGS_H */


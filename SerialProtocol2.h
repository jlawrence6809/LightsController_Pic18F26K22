/* 
 * File:   SerialProtocol2.h
 * Author: jeremy
 *
 * Created on December 7, 2014, 10:18 AM
 */

#ifndef SERIALPROTOCOL2_H
#define	SERIALPROTOCOL2_H

#ifdef	__cplusplus
extern "C" {
#endif

extern void SerialProtocol2_Init(void*);
extern void msgRx(void);
extern void serialCycle(void);


#ifdef	__cplusplus
}
#endif

#endif	/* SERIALPROTOCOL2_H */


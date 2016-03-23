/*
  * File:   shared.h
  * Author: jeremy.lawrence
  *
  * Created on October 26, 2014, 5:12 PM
  */

#ifndef SHARED_H
#define SHARED_H

#ifdef  __cplusplus
extern "C" {
#endif
#include "ByteBuffer.h"
    #define delay_10ms(val) uint8_t tmptmp = val; while(tmptmp--)__delay_ms(10)

    extern uint8_t numberOfRestarts;
    extern uint8_t irDumpFlag;
    extern uint8_t irEvent;
    extern uint8_t irBuffer[80];
    extern uint8_t irBufferCnt;

    uint8_t lightToPin[8];

//    uint8_t async_Eusart_1_TxFlag;

    extern unsigned long long SystemCounter;

    uint8_t swTimerRegister(unsigned long long, void*);
    void swTimerUnregister(uint8_t);
    
    void remoteCommunication_Run(void);
    void remoteCommunication_Setup(void);
    void serverCommunication_Run(void);
    void serverCommunication_Setup(void);

    void async_Eusart_2_Setup(uint8_t , uint8_t );

#define Restarts_EEPROM_Start 0
#define BCM_EEPROM_Start 1 //to 9
#define LightToPin_EEPROM_Start 10
    
//    extern uint8_t eusartTxIntF;
//    extern uint8_t eusartRxIntF;
//    extern ByteBuffer txBuffer;
//    extern ByteBuffer rxBuffer;

#ifdef  __cplusplus
}
#endif

#endif  /* SHARED_H */

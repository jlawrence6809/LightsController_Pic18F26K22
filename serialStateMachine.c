#if defined(__XC)
#include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
    #include <htc.h>        /* HiTech General Include File */
#endif

#include <stdint.h>         /* For uint8_t definition */
#include <stdbool.h>        /* For true/false definition */

#include "system.h"
#include "user.h"
#include "shared.h"
#include "serialStateMachine.h"
#include "ByteBuffer.h"
#include "BCM.h"
#include "errorFlags.h"

#define TIMEOUTMS 100

#define A_READ PORTBbits.RB0
#define B_READ PORTBbits.RB1
#define READ() ((A_READ<<1)|B_READ)
#define A_WRITE(val) TRISBbits.TRISB0 = val
#define B_WRITE(val) TRISBbits.TRISB1 = val
//#define WRITE(val) TRISBbits.TRISB0=(val>>1);TRISBbits.TRISB1=val;
#define INT_WRITE(val) IOCBNbits.IOCBN0 = val


void setupSMSerial(void*);
void serialCycle(void);
void txByte(uint8_t);
void txBit(uint8_t);
void rxByte(uint8_t);
void rxBit(void);
void failMode(void);
uint8_t readUntilValueOrTimeOut(uint8_t);

ByteBuffer txByteBuffer;
uint8_t portUsedFlag = 0;
uint8_t rxBuffer = 0;
uint8_t rxCounter = 0;

uint8_t* txByteBufferOverUnderflow = &txByteBuffer.OverUnderflow;
uint8_t portConflictError = 0;
uint8_t serialTimeOutFlag = 0;
uint8_t serialNoiseCounter = 0;

void setupSMSerial(void* rxCallback){
    //SETUP PORT
    ANSELBbits.ANSB0 = 0; //digital
    ANSELBbits.ANSB1 = 0;

    LATBbits.LATB0 = 0;
    LATBbits.LATB1 = 0;

    A_WRITE(1);
    B_WRITE(1);
    while(READ() != 0b11);
    __delay_us(10);
}

void serialCycle(){
    if(READ() != 0b11){ //replace with interrupt flag?
        if(portUsedFlag) portConflictError = 1;
        else portUsedFlag = 1;
        rxBit();
        portUsedFlag = 0;
    }
    if(!portUsedFlag && !portUsedFlag && txByteBuffer.Count > 0){
            txByte(ByteBuffer_Read(&txByteBuffer));
    }
}

void txByte(uint8_t val){
    for(uint8_t i = 0; i < 8; i++){
        txBit(val);
        val = (val >> 1)|(val << 7);
    }
}

void txBit(uint8_t val){
    readUntilValueOrTimeOut(0b11);
    if(val & 1 == 1){
        B_WRITE(0);
        readUntilValueOrTimeOut(0b00);
        //do work
        B_WRITE(1);
    }else{
        A_WRITE(0);
        readUntilValueOrTimeOut(0b00);
        //do work
        A_WRITE(1);
    }
    readUntilValueOrTimeOut(0b11);
}

void rxByte(uint8_t val){
    rxBuffer = rxBuffer | val;
    rxBuffer = (rxBuffer>>1)|(rxBuffer<<7);
    rxCounter++;
    if(rxCounter == 8){
        processByte(rxBuffer);
        rxCounter = 0;
        rxBuffer = 0;
        portUsedFlag = 0;
    }
}

void rxBit(){
    uint8_t val;
    while((val = READ()) == 0b11);
    if(val == 0b10){
        A_WRITE(0);
        readUntilValueOrTimeOut(0b01);
        rxByte(1);
        A_WRITE(1);
    }else if(val == 0b01){
        B_WRITE(0);
        readUntilValueOrTimeOut(0b10);
        rxByte(0);
        B_WRITE(1);
    }else if(val == 0b00){
        //most likely noise on the line
        A_WRITE(1);
        B_WRITE(1);
        serialNoiseCounter++;
    }
}

void failMode(){
    rxBuffer = 0;
    rxCounter = 0;

}

uint8_t readUntilValueOrTimeOut(uint8_t val){
    uint8_t TO = 0;
    while(READ() != val){
        if(TO/*--*/ == TIMEOUTMS){
            serialTimeOutFlag++;
            return 1;
        }
        //__delay_ms(1);
    }
    return 0;
}
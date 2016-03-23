#if defined(__XC)
#include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
    #include <htc.h>        /* HiTech General Include File */
#endif

#include <stdint.h>         /* For uint8_t definition */
#include <stdbool.h>        /* For true/false definition */

#include "user.h"
#include "twoWireClkStretch.h"
#include "shared.h"

#define SCLK_READ PORTBbits.RB0
#define SDAT_READ PORTBbits.RB1
#define SCLK_WRITE(val) TRISBbits.TRISB0 = val
#define SDAT_WRITE(val) TRISBbits.TRISB1 = val
#define INT_WRITE(val) IOCBNbits.IOCBN0 = val

#define SADX 5
#define txWaitTime 0x00
#define bitsPerData 8
//#define bitsPerCRC 1
//#define bitsPerFrame 9

void setupSerial(void);
bool txSerial(uint8_t);
char rxSerial(unsigned long long, void (*callback)(char));

bool SCLK_INT = false;
unsigned long long rxTimeOut = 100100;
uint8_t whileLoopTimeOut = 0xFF;
unsigned long long lastCounter = 0;
bool checkRxTimeOutFlag = false;
uint8_t bitsRx = 0;
char RX_BUF = 0;

/*  1) Put dat
    2) Pull clk low
    3) Wait (<<1ms)
    4) Let clk high
    5) If clk high repeat 2-4 (timeout after 20th time, release dat)
    6) If clk low (rx ack)
    6B) Do whatever takes a while here
    7) Toggle data opposite (tx ack)
    8) Wait for clk high
    9) Let dat high
  */
bool txSerial(uint8_t d){
    INT_WRITE(0);
    for(uint8_t i = 0; i < bitsPerData; i++){
        SDAT_WRITE(d & 1);
        uint8_t whileLoopTimer = 0;
        do{
            if(whileLoopTimer == whileLoopTimeOut){
                SDAT_WRITE(1);
                SCLK_WRITE(1);
                return false;
            }
            SCLK_WRITE(0);
            for(uint8_t w = 0; w < txWaitTime; w++){
                NOP();
            }
            SCLK_WRITE(1);
            whileLoopTimer++;
        }while(SCLK_READ == 1);
        //Any overhead calcs start:
        d = (d >> 1) | (d << 7); //rotate left
        whileLoopTimer = 0;
        //end.
        SDAT_WRITE(d ^ 1);
        while(SCLK_READ == 0){
            whileLoopTimer++;
            if(whileLoopTimer == whileLoopTimeOut){
                SDAT_WRITE(1);
                SCLK_WRITE(1);
                return false;
            }
        }
        SDAT_WRITE(1);
    }
    INT_WRITE(1);
    return true;
}

/*
    1) Interrupt on clk low
    2) Read dat
    3) Disable clk low interrupt
    4) Pull clk low (rx ack)
    4B) Do whatever takes a while here
    5) Wait for dat to change (tx ack)
    6) Enable clk low interrupt
    7) Let clk high
  */
char rxSerial(unsigned long long counter, void (*callback)(char)){
    if(SCLK_INT){ //wait for clk low
        uint8_t d = SDAT_READ;
        INT_WRITE(0);
        SCLK_WRITE(0);
        //Any overhead calcs start:
        bitsRx++;
        lastCounter = counter;
        SCLK_INT = false;
        RX_BUF = RX_BUF | d;
        RX_BUF = (RX_BUF >> 1)|( RX_BUF << 7);
        if(bitsRx < bitsPerData){
            checkRxTimeOutFlag = true;
        }else{
            //RECIEVED A BYTE OF DATA!
            callback(RX_BUF);
            bitsRx = 0;
            RX_BUF = 0;
            checkRxTimeOutFlag = false;
        }
        uint8_t whileLoopTimer = 0;
        //end
        while(SDAT_READ == d){
            whileLoopTimer++;
            if(whileLoopTimer == whileLoopTimeOut){
                INT_WRITE(1);
                SCLK_WRITE(1);
                RX_BUF = 0;
                return 'b';
            }
        }
        INT_WRITE(1);
        SCLK_WRITE(1);
    }
    if(checkRxTimeOutFlag && (counter - lastCounter) >= rxTimeOut){
        checkRxTimeOutFlag = false;
        RX_BUF = 0;
        return 'r';
    }
    if(checkRxTimeOutFlag == false){
        return 'f';
    }
    return 'w';
}


void setupSerial(void){
    //SETUP PORT
    ANSELBbits.ANSB0 = 0; //digital
    ANSELBbits.ANSB1 = 0;

    LATBbits.LATB0 = 0;
    LATBbits.LATB1 = 0;

    SCLK_WRITE(1);
    SDAT_WRITE(1);

    //SETUP INTERRUPT
    INTCONbits.IOCIE = 1; //interrupt on change
    //IOCBPbits.IOCBP0 = 1; //rising edge
    IOCBNbits.IOCBN0 = 1; //falling edge
}

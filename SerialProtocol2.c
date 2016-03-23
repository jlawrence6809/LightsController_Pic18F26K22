#if defined(__XC)
#include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
    #include <htc.h>        /* HiTech General Include File */
#endif

#include <stdint.h>         /* For uint8_t definition */
#include <stdbool.h>        /* For true/false definition */

#include "system.h"
#include "ByteBuffer.h"
#include "shared.h"


enum RxState{
    LEN,
    DATA,
    CRC
};

void SerialProtocol2_Init(void*);
void msgRx(char);
void serialCycle(void);

enum RxState rxState = LEN;
uint8_t rxLen = 0;
uint8_t rxCRC = 0;
uint8_t rxMsgFlag = 0;
uint8_t rxMsgError = 0;
void* RxSrc;

void SerialProtocol2_Init(void* RxSrcIn){
    RxSrc = RxSrcIn;
}

void msgProcess(){

}

void serialCycle(){
//    if(eusartRxIntF){
//        while(rxBuffer.Count > 0){
//            msgRx(ByteBuffer_Read(&rxBuffer));
//        }
//        eusartRxIntF = 0;
//    }
//    if(eusartTxIntF){
//        eusartTxIntF--;
//    }
}

void msgRx(uint8_t in){
    rxCRC += in;
    switch(rxState){
        case LEN:
            rxLen = in;
            rxState = DATA;
            break;
        case DATA:
            rxCRC += rxLen;
            //ByteBuffer_Write(RCREG1);
            if(--rxLen){
                rxState = DATA;
            }else{
                rxState = CRC;
            }
            break;
        case CRC:
            rxCRC -= in;
            if(in == rxCRC){
                rxMsgFlag = 1;
            }else{
                rxMsgError++;
            }
            rxCRC = 0;
            rxState = LEN;
            break;
        default:
            rxMsgError++;
            break;
    }
}
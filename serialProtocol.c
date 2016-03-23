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
#include "BCM.h"
#include "errorFlags.h"

#define rxToTxPause 20


void processByte(uint8_t);

enum States{
    COMMAND,
    LIGHT,
    BRIGHTNESS,
    CHECK,
};
enum Command{
    LIGHTCMD=0,
    READ,
    FLAGS,
    COUNTER
};

enum States state = COMMAND;
uint8_t byteBuf[2];
uint8_t invalidCommandFlag = 0;

void processByte(uint8_t val){
    uint8_t check2;
    switch(state){
        case COMMAND:
            switch(val){
                case LIGHTCMD: //0x69
                    state = LIGHT;
                    break;
                case READ: //0xCC
                    //send all lights and values
                    ByteBuffer_Write(&txByteBuffer, 24);
                    for(uint8_t i = 0; i < 8; i++){
                        ByteBuffer_Write(&txByteBuffer, i);
                        ByteBuffer_Write(&txByteBuffer, brightnessVals[i]);
                        ByteBuffer_Write(&txByteBuffer, i + brightnessVals[i]);
                    }
                    delay_10ms(rxToTxPause);
                    state = COMMAND;
                    break;
                case FLAGS: //0x53
                    //send all flags
                    ByteBuffer_Write(&txByteBuffer, 20);
                    ByteBuffer_Write_Str(&txByteBuffer, "PCE");
                    ByteBuffer_Write(&txByteBuffer, portConflictError);
                    ByteBuffer_Write_Str(&txByteBuffer, "STO");
                    ByteBuffer_Write(&txByteBuffer, serialTimeOutFlag);
                    ByteBuffer_Write_Str(&txByteBuffer, "BOF");
                    ByteBuffer_Write(&txByteBuffer, (*txByteBufferOverUnderflow));
                    ByteBuffer_Write_Str(&txByteBuffer, "NOS");
                    ByteBuffer_Write(&txByteBuffer, serialNoiseCounter);
                    ByteBuffer_Write_Str(&txByteBuffer, "NVC");
                    ByteBuffer_Write(&txByteBuffer, invalidCommandFlag);
                    delay_10ms(rxToTxPause);
                    state = COMMAND;
                    break;
                case COUNTER: //0xAC
                    //send system counter
                    ByteBuffer_Write(&txByteBuffer, 4);
                    for(uint8_t i = 0; i < 4; i++){
                        ByteBuffer_Write(&txByteBuffer,(uint8_t) (SystemCounter>>(i*8)));
                    }
                    delay_10ms(rxToTxPause);
                    state = COMMAND;
                    break;
                default:
                    invalidCommandFlag++;
                    state = COMMAND;
                    break;
            }
            break;
        case LIGHT:
            byteBuf[0] = val;
            state = BRIGHTNESS;
        break;
        case BRIGHTNESS:
            byteBuf[1] = val;
            state = CHECK;
        break;
        case CHECK:
            check2 = byteBuf[0] + byteBuf[1];
            if(check2 == val){
                writeToBCM(byteBuf[0], byteBuf[1]);
            }else{
                val++;
            }
            state = COMMAND;
        break;
    }
//    txByteBuffer_Write(val + 20);
}

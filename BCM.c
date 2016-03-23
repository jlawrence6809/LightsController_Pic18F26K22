#if defined(__XC)
#include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
    #include <htc.h>        /* HiTech General Include File */
#endif

#include <stdio.h>
#include <stdint.h>         /* For uint8_t definition */
#include <stdbool.h>        /* For true/false definition */
#include "BCM.h"
#include "shared.h"
#include "system.h"

#define numLights 8
#define bcmRes 8
void grayToBCM(uint8_t[8], uint8_t[8]);
void writeToBCM(uint8_t, uint8_t);
void writeToBCM_Priv(uint8_t , uint8_t );
void flashBCM(uint8_t);
void flashBCMStop(void);
void flashBCMCallback(void);
uint8_t BCM[8] = {0,0,0,0,0,0,0,0};
uint8_t brightnessVals[8] = {   0b00000001,
                                0b00000010,
                                0b00000100,
                                0b00001000,
                                0b00010000,
                                0b00100000,
                                0b01000000,
                                0b10000000};
uint8_t lightToPin[8] = {0,1,2,3,4,5,6,7};

uint8_t flashIdx = 255;
uint8_t flashVal = 0;
uint8_t flashStore = 0;
uint8_t flashCnt = 0;
void flashBCM(uint8_t idx){
    if(flashIdx == 255){
        if(idx > 7 && idx != 'a'){
            idx -= '0';
            if(idx > 7) return;
        }
        flashStore = brightnessVals[idx];
        if(flashStore == 0) flashVal = 2;
        else flashVal = 0;
        flashCnt = 0;
        writeToBCM(idx, flashVal);
        flashIdx = idx;
        swTimerRegister(0x4FFF, &flashBCMCallback);
    }
}

void flashBCMStop(){
    flashIdx = 255;
}

void flashBCMCallback(){
    if(flashIdx != 255){
        if((flashCnt++) & 0x1 ){
            writeToBCM(flashIdx, flashStore);
        }else{
            writeToBCM(flashIdx, flashVal);
        }
        swTimerRegister(0x4FFF, &flashBCMCallback);
    }
}

void writeToBCM(uint8_t idx, uint8_t val){
    if(idx == 'a'){
        for(uint8_t i = 0; i < 8; i++){
            writeToBCM_Priv(i, val);
        }
        return;
    }
    if(idx > 7){
        idx -= '0';
        if(idx > 7) return;
    }
    writeToBCM_Priv(idx, val);
}

void writeToBCM_Priv(uint8_t idx, uint8_t val){
    val = ~val;
    idx = lightToPin[idx];
    if(brightnessVals[idx] != val){
        brightnessVals[idx] = val;
        eeprom_write(BCM_EEPROM_Start + idx, val);
        //val = ~val;
        uint8_t msk = 1 << idx;
        uint8_t nmsk = ~msk;
        for(uint8_t i = 0; i < 8; i++){
            if(val & 1){
                BCM[i] |= msk;
            }else{
                BCM[i] &= nmsk;
            }
            val = (val >> 1)|(val << 7);
        }
    }
}

void grayToBCM(uint8_t in[8], uint8_t out[8]){
    for(uint8_t i = 0; i < 8; i++){
        for(uint8_t j = 0; j < 8; j++){ //when j is zero then j-- results in 255
            out[i] |= in[j] & 1;
            in[j] = (in[j] >> 1)|(in[j] << 7);
            out[i] = (out[i] >> 1)|(out[i] << 7);
        }
    }
}

void initBCM(){
    //grayToBCM(brightnessVals, BCM);
    for(uint8_t i = 0; i < 8; i++){
        writeToBCM(i,(uint8_t) eeprom_read(BCM_EEPROM_Start + i));
        lightToPin[i] = (uint8_t) eeprom_read(LightToPin_EEPROM_Start + i);
    }

    //~~~~~~~~~~~~~~~~~~~~SETUP PORTS~~~~~~~~~~~~~~~~~~~~
    ANSELAbits.ANSA0 = 0; //use digital
    ANSELAbits.ANSA1 = 0;
    ANSELAbits.ANSA2 = 0;
    ANSELAbits.ANSA3 = 0;
    ANSELAbits.ANSA5 = 0;

    TRISA = 0;//xFF; //input, defaults to dim
    PORTA = 0xFF;

//        TRISAbits.TRISA0  = 0; //output
//        TRISAbits.TRISA1  = 0;
//        TRISAbits.TRISA2  = 0;
//        TRISAbits.TRISA3  = 0;
//        TRISAbits.TRISA4  = 0;
//        TRISAbits.TRISA5  = 0;
//        TRISAbits.TRISA6  = 0;
//        TRISAbits.TRISA7  = 0;
//        ODCONAbits.ODA0   = 1; //can only sink current, cannot source current (input only)
//        SLRCONAbits.SLRA0 = 0; //max slew rate
//        SLRCONAbits.SLRA1 = 0;
//        SLRCONAbits.SLRA2 = 0;
//        SLRCONAbits.SLRA3 = 0;
//        SLRCONAbits.SLRA4 = 0;
//        SLRCONAbits.SLRA5 = 0;
//        SLRCONAbits.SLRA6 = 0;
//        SLRCONAbits.SLRA7 = 0;


    //~~~~~~~~~~~~~~~~~~~~SETUP TIMER~~~~~~~~~~~~~~~~~~~~
   /*
     T2OUTPS<3:0>: Timer2 Output Postscaler Select bits
         1111 = 1:16 Postscaler
         1110 = 1:15 Postscaler
         ...
         0001 = 1:2 Postscaler
         0000 = 1:1 Postscaler
     T2CKPS<1:0>: Timer2 Clock Prescale Select bits
         1x = Prescaler is 16
         01 = Prescaler is 4
         00 = Prescaler is 1
    */
    PIE1bits.TMR2IE     = 1; //timer 2 interrupt enable
    IPR1bits.TMR2IP     = 0; //low priority
    T2CONbits.T2CKPS    = 0b11; //prescale
    T2CONbits.T2OUTPS   = 0b0000; //postscaler
    PR2                 = 0b00000001; //what count to interrupt on
    T2CONbits.TMR2ON    = 1; //timer 2 on
}

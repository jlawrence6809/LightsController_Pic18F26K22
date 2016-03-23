/******************************************************************************/
/*Files to Include                                                            */
/******************************************************************************/

#if defined(__XC)
#include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
#include <htc.h>        /* HiTech General Include File */
#endif

#include <stdint.h>         /* For uint8_t definition */
#include <stdbool.h>        /* For true/false definition */

/******************************************************************************/
/* Interrupt Routines                                                         */
/******************************************************************************/

/* Baseline devices don't have interrupts. Note that some PIC16's
 * are baseline devices.  Unfortunately the baseline detection macro is
 * _PIC12 */
#ifndef _PIC12

#include "shared.h"
#include "BCM.h"

uint8_t eusartTxIntF;
uint8_t eusartRxIntF;
uint8_t tmr2Cnt = 0;
uint8_t tmr2Len = 0x01;
//uint8_t pr2Vals[8] = {1,3,7,15,31,63,127,255};

void interrupt low_priority isrl(void) {
    if(PIR1bits.TMR2IF){ //timer 2
        tmr2Cnt = (tmr2Cnt + 1) & 0x07;
        PORTA = BCM[tmr2Cnt];
        //TRISA = BCM[tmr2Cnt];
        tmr2Len = (tmr2Len << 1) | (tmr2Len >> 7);
        PR2 = (tmr2Len-1)|tmr2Len;
        PIR1bits.TMR2IF = 0;
        TMR2 = 0;
    }
}

#define irSampleSize 1200
uint16_t irSampleCnt = 0;//80ms*10sp/ms = 800samples;
uint8_t lastRB5 = 0;
uint8_t curRB5Cnt = 0;
uint8_t irBuffer[80];
uint8_t irBufferCnt = 0;
uint8_t irEvent = 0;
uint8_t runTmrF = 0;

void interrupt high_priority isrh(void){
        //tx eusart
        if(PIR1bits.TX1IF){
            if(txBuffer1.Count > 0){
                TXREG1 = ByteBuffer_Read(&txBuffer1);
            }else{
                PIE1bits.TX1IE = 0; //disable interrupt
            }
        }
        //rx eusart
        if(PIR1bits.RC1IF){
            ByteBuffer_Write(&rxBuffer1, RCREG1);
            PIR1bits.RC1IF = 0;
        }


//        //tx eusart
//        if(PIR3bits.TX2IF){
//            if(txBuffer2.Count > 0){
//                TXREG2 = ByteBuffer_Read(&txBuffer2);
//            }else{
//                PIE3bits.TX2IE = 0; //disable interrupt
//            }
//        }
//        //rx eusart
//        if(PIR3bits.RC2IF){
//            ByteBuffer_Write(&rxBuffer2, RCREG2);
//            PIR3bits.RC2IF = 0;
//        }
        
        if(PIR5bits.TMR4IF && runTmrF){
            TMR4 = 0;
            PR4 = 0xFF;
            PIR5bits.TMR4IF = 0;
            if(irSampleCnt++ >= irSampleSize){
                irSampleCnt = 0; //reset counter
                PIE5bits.TMR4IE = 0; //turn off timer interrupt
                T4CONbits.TMR4ON = 0; //turn off timer
                IOCBbits.IOCB5 = 1; //enable RB5 interrupt
                irBuffer[irBufferCnt] = curRB5Cnt;
                irEvent++;
                INTCONbits.RBIF = 0;
                lastRB5 = PORTBbits.RB5;
                runTmrF = 0;
            }else{
                uint8_t tmpRB5 = PORTBbits.RB5;
                if(tmpRB5 != lastRB5){
                    lastRB5 = tmpRB5;
                    irBuffer[irBufferCnt++] = curRB5Cnt;
                    curRB5Cnt = 0;
                }else{
                    if(curRB5Cnt < 255){
                        curRB5Cnt++;
                    }
                }
                //reset for counting again
            }
        }else if(INTCONbits.RBIF){
            INTCONbits.RBIF = 0;
            if(!irEvent && !PORTBbits.RB5){
                runTmrF = 1;
                irBufferCnt = 0;
                lastRB5 = PORTBbits.RB5;
                IOCBbits.IOCB5 = 0; //RB5 disable interrupt
                PIE5bits.TMR4IE = 1; //timer 2 interrupt enable
                T4CONbits.TMR4ON = 1; //timer 2 on
            }
        }
}


//uint8_t pulses

#endif

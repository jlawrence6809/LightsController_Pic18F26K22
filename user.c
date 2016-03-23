/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#if defined(__XC)
    #include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
    #include <htc.h>        /* HiTech General Include File */
#endif

#include <stdint.h>         /* For uint8_t definition */
#include <stdbool.h>        /* For true/false definition */

#include "user.h"

/******************************************************************************/
/* User Functions                                                             */
/******************************************************************************/

/* <Initialize variables in user.h and insert code for user algorithms.> */

void InitApp(void)
{
    INTCONbits.GIE  = 1; //enable global interrupts
    INTCONbits.PEIE = 1; //perhipheral interrupts enabled
    RCONbits.IPEN = 1; //allow interrupt priorities
    INTCONbits.GIEH = 1; //high
    INTCONbits.GIEL = 1; //low

    configPorts();
    configLightsTimer(); //must be done last
}

void configPorts(void){
        ANSELAbits.ANSA0 = 0; //use digital
        ANSELAbits.ANSA1 = 0;
        ANSELAbits.ANSA2 = 0;
        ANSELAbits.ANSA3 = 0;
        ANSELAbits.ANSA5 = 0;
        
        TRISA = 0xFF; //input, defaults to dim
        PORTA = 0xFF;

//        TRISAbits.TRISA0 = 0; //output
//        TRISAbits.TRISA1 = 0;
//        TRISAbits.TRISA2 = 0;
//        TRISAbits.TRISA3 = 0;
//        TRISAbits.TRISA4 = 0;
//        TRISAbits.TRISA5 = 0;
//        TRISAbits.TRISA6 = 0;
//        TRISAbits.TRISA7 = 0;
    //  ODCONAbits.ODA0  = 1; //can only sink current, cannot source current (input only)
//        SLRCONAbits.SLRA0 = 0; //max slew rate
//        SLRCONAbits.SLRA1 = 0;
//        SLRCONAbits.SLRA2 = 0;
//        SLRCONAbits.SLRA3 = 0;
//        SLRCONAbits.SLRA4 = 0;
//        SLRCONAbits.SLRA5 = 0;
//        SLRCONAbits.SLRA6 = 0;
//        SLRCONAbits.SLRA7 = 0;
}


void configLightsTimer(void){
        /*
                 T2OUTPS<3:0>: Timer2 Output Postscaler Select bits
                     1111 = 1:16 Postscaler
                     1110 = 1:15 Postscaler
                     1101 = 1:14 Postscaler
                     1100 = 1:13 Postscaler
                     1011 = 1:12 Postscaler
                     1010 = 1:11 Postscaler
                     1001 = 1:10 Postscaler
                     1000 = 1:9 Postscaler
                     0111 = 1:8 Postscaler
                     0110 = 1:7 Postscaler
                     0101 = 1:6 Postscaler
                     0100 = 1:5 Postscaler
                     0011 = 1:4 Postscaler
                     0010 = 1:3 Postscaler
                     0001 = 1:2 Postscaler
                     0000 = 1:1 Postscaler
                 T2CKPS<1:0>: Timer2 Clock Prescale Select bits
                     1x = Prescaler is 16
                     01 = Prescaler is 4
                     00 = Prescaler is 1
              */
    PIE1bits.TMR2IE     = 1; //timer 2 interrupt enable
    IPR1bits.TMR2IP     = 0; //low priority
    T2CONbits.T2CKPS    = 0b01; //prescale
    T2CONbits.T2OUTPS   = 0b0000; //postscaler
    PR2                 = 0b00000001; //what count to interrupt on
    T2CONbits.TMR2ON    = 1; //timer 2 on
}
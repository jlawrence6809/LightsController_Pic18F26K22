/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#if defined(__XC)
    #include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
    #include <htc.h>        /* HiTech General Include File */
#endif

#include <stdio.h>
#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */
#include <conio.h>

#define SYS_FREQ        64000000L
#define _XTAL_FREQ      SYS_FREQ
#define FCY             SYS_FREQ/4
#include "ByteBuffer.h"
#include "shared.h"
#include "BCM.h"
//#include "nokia5110.c"
//#include "ILI9341_LCD.h"
/******************************************************************************/
/* User Global Variable Declaration                                           */
/******************************************************************************/

void ConfigureOscillator(void);
void InitApp(void);
char irVal2But(uint8_t*);
uint8_t swTimerRegister(unsigned long long, void*);
void swTimerCycle(void);

unsigned long long SystemCounter = 0;
uint8_t numberOfRestarts = 0;

/******************************************************************************/
/* Main Program                                                               */
/******************************************************************************/
//char tmpMsg[65];

void main(void)
{
    ConfigureOscillator();
    InitApp();
    initBCM();
    serverCommunication_Setup();
    remoteCommunication_Setup();
    
//    LCDInit();
//   ILI9341_Init();
    
    numberOfRestarts = eeprom_read(Restarts_EEPROM_Start) + 1;
    eeprom_write(Restarts_EEPROM_Start, numberOfRestarts);
    while(1){
        SystemCounter++;
//        ILI9341_Loop();
//        LCDLoop();
        swTimerCycle();
        serverCommunication_Run();
        remoteCommunication_Run();
    }
}

#define swTimersLength 5
unsigned long long swTimers[swTimersLength];
void (*swTimerCallbacks[swTimersLength]);
uint8_t swTimerRegister(unsigned long long cycles, void* callback){
    for(uint8_t i = 0; i < swTimersLength; i++){
        if(swTimers[i] == 0){
            swTimers[i] = cycles;
            swTimerCallbacks[i] = callback;
            return i;
        }
    }
    return 255;
}

void swTimerUnregister(uint8_t timer){
    if(timer < swTimersLength){
        swTimers[timer] = 0;
    }
}

void swTimerCycle(){
    for(uint8_t i = 0; i < swTimersLength; i++){
        switch(swTimers[i]){
            case 0:
                break;
            case 1:
                NOP();
                void (*func)() = swTimerCallbacks[i];
                (*func)();
            default:
                swTimers[i]--;
                break;
        }
    }
}

void ConfigureOscillator(void){
    //sets clock to 64MHz
    OSCCONbits.SCS = 0b00;
    OSCCONbits.IRCF = 0b111; //16MHz
    OSCTUNEbits.PLLEN = 1; //pll on
    OSCTUNEbits.TUN = 0b011111; //maximum overclock B)
}

void InitApp(void){
    INTCONbits.GIE  = 1; //enable global interrupts
    INTCONbits.PEIE = 1; //perhipheral interrupts enabled
    RCONbits.IPEN = 1; //allow interrupt priorities
    INTCONbits.GIEH = 1; //high
    INTCONbits.GIEL = 1; //low
}


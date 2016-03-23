
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
#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp */
#include "ByteBuffer.h"
#include "shared.h"
#include "BCM.h"

#define irLoMin 0x07
#define irLoMax 0x0E
#define irHiMin 0x1C
#define irHiMax 0x24
#define irbV(v) ((v < irLoMax && v > irLoMin)?0:((v < irHiMax && v > irHiMin)?1:3))

void remoteCommunication_Run(void);
void remoteCommunication_Setup(void);
void waterHeaterRoomTimeout(void);
void bathroomTimeout(void);
char irVal2But(uint8_t*);

#define CODESIZE 5
#define NUMBUTTONS  17


uint8_t irDumpFlag = 0;
uint8_t lightState = 0;
uint8_t lightSelected = 0;

uint8_t waterHeaterRoomFlag = 0;
void waterHeaterRoomTimeout(){
    writeToBCM(0,0);
    waterHeaterRoomFlag = 0;
}

void bathroomTimeout(){

}

void remoteCommunication_Run(){
    if(RCSTA2bits.OERR){//rx overrun, clear buffer
        RCSTA2bits.CREN = 0;
        RCSTA2bits.CREN = 1;
        printf("OVERUN\n");
    }
    if(irEvent){
        irEvent = 0;
        uint8_t passed = 1;
        //check that data from index 19 to 34 is 1010101010101010
        for(uint8_t i = 19; i < 35; i++){
            uint8_t chk = irbV(irBuffer[i]);
            irBuffer[i] = 0;
            if(chk != i%2){
                passed = 0;
                break;
            }
        }
        if(passed){
            //parse the signal data
            uint8_t relevantData[9];
            for(uint8_t i = 0; i < 9; i++){
                relevantData[i] = irbV(irBuffer[i+35]);
                if(relevantData[i] == 3){
                    passed = 0;
                    break;
                }
            }
            if(passed){
                char button = irVal2But(relevantData);
                printf("button:'%c'\n",button);
                if(button >= '0' && button <= '9'){
                    switch(lightState){
                        case 0:
                            lightState = 1;
                            flashBCM(button);
                            lightSelected = button;
                            break;
                        case 1:
                            lightState = 0;
                            flashBCMStop();
                            if(button == '9'){
                                writeToBCM(lightSelected, 255);
                            }else{
                                writeToBCM(lightSelected, 28*(button-'0'));
                            }
                            break;
                        default:
                            break;
                    }
                }else{
                    if(lightState != 0){
                        lightState = 0;
                        flashBCMStop();
                    }
                    switch(button){
                        case '*':
                            if(!waterHeaterRoomFlag){
                                writeToBCM(0, 255);
                                swTimerRegister(0xFFFFFF, &waterHeaterRoomTimeout);
                                waterHeaterRoomFlag = 1;
                            }
                            break;
                        case '#':

                            break;
                        case 'U':
                            writeToBCM('a',255);
                            break;
                        case 'D':
                            writeToBCM('a',0);
                            break;
                        case 'L':
                            writeToBCM('a',1);
                            break;
                        case 'R':
                            writeToBCM('a',64);
                            break;
                        case 'K':

                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }
    if(irDumpFlag){
        irDumpFlag = 0;
        printf("dump:{");
        for(uint8_t i = 0; i < 80; i++){
            printf("0x%X,",irBuffer[i]);
        }
        printf("]}\n");
    }
}

/**
 * Button signals encoded in boolean if statements
 * @param v
 * @return
 */
char irVal2But(uint8_t* v){
      if(v[1] || v[3] || v[5] || v[7]){
            return 'X';
      }

      if(v[0]){
            if(v[2]){
                  return 'R'; //11
            }
            if(v[4]){
                  if(v[6]){
                        return '3';//1011
                  }
                  return 'D';//1010
            }
            return '2';//1000
      }
      if(v[2]){
            if(v[4]){
                  if(v[6]){
                        return '6';//0111
                  }
                  if(v[8]){
                        return '1';//01101
                  }
                  return 'U';//01100
            }
            if(v[6]){
                  if(v[8]){
                        return '9';//01011
                  }
                  return '#';//01010
            }
            if(v[8]){
                  return '0';//01001
            }
            return '*';//01000
      }
      if(v[4]){
            if(v[6]){
                  if(v[8]){
                        return '8';//00111
                  }
                  return '4';//00110
            }
            return 'L';//00100
      }
      if(v[6]){
            if(v[8]){
                  return '5';//00011
            }
            return '7';//00010
      }
      return 'K';//0000
}

void remoteCommunication_Setup(){
    ANSELBbits.ANSB5 = 0;
    TRISBbits.TRISB5 = 1; //input

    uint8_t throwAway = PORTBbits.RB5; //read port once
    INTCONbits.RBIE = 1;//allow pin change interrupts
    IOCBbits.IOCB5 = 1; //enable interrupt
    INTCON2bits.RBIP = 1; //high priority
    
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
    IPR5bits.TMR4IP     = 1; //high priority
    PR4                 = 0xFF; //what count to interrupt on
    T4CONbits.TMR4ON    = 1; //timer 4 off
    T4CONbits.T4CKPS    = 0b00; //prescale
    T4CONbits.T4OUTPS   = 0b0010; //postscaler

}

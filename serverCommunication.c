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

ByteBuffer txBuffer1;
ByteBuffer rxBuffer1;

void serverCommunication_Setup(void);
void serverCommunication_Run(void);
void processCommand(char);
void setLight(char);
void changeLightToPinMapping(char);
void async_Eusart_1_Setup(uint8_t , uint8_t );
uint8_t hexCharToNum(char);

char undefStr[6] = "undef\0";
char numToCharArray[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
uint8_t lightSetMode = 0;
void (*runFunc)(char) = &processCommand;

void serverCommunication_Setup(){
    async_Eusart_1_Setup(0x06, 0x82); //9600 baud
    ByteBuffer_Construct(&txBuffer1);//, void);
    ByteBuffer_Construct(&rxBuffer1);//, &serverCommunication_Run);
    __delay_ms(1);
    printf("start...\n");
}

void serverCommunication_Run(){
    if(RCSTA1bits.OERR){//rx overrun, clear buffer
        RCSTA1bits.CREN = 0;
        RCSTA1bits.CREN = 1;
    }
    if(RCSTAbits.FERR && !PORTCbits.RC7){
        printf("BL_reset...\n");
        Reset();
    }
    if(rxBuffer1.Count > 0){
//        PIE1bits.RC1IE = 0;
        char in = ByteBuffer_Read(&rxBuffer1);
//        PIE1bits.RC1IE = 1;
        runFunc(in);
    }
}

void processCommand(char in){
    switch(in){
        case 's':
            setLight(in);
            return;
        case 'm':
            printf("lightSetMode:'%c'", (lightSetMode)?'H':'#');
            lightSetMode = !lightSetMode;
            break;
        case 'q': //query lights
            printf("'lvs':{");
            for(uint8_t i = 0; i < 8; i++){
                printf("%X:0x%X,", i, brightnessVals[i]);
            }
            printf("}");
            break;
        case 'e':
            Nop();
            uint8_t flags0 = RCONbits.BOR;
            flags0 = (flags0 << 1) + RCONbits.POR;
            flags0 = (flags0 << 1) + RCONbits.TO;
            flags0 = (flags0 << 1) + RCONbits.PD;
            flags0 = (flags0 << 1) + STKPTRbits.STKFUL;
            flags0 = (flags0 << 1) + STKPTRbits.STKOVF;
            flags0 = (flags0 << 1) + STKPTRbits.STKUNF;
            printf("sysF:0x%X", flags0);
            break;
        case 'r':
            printf("reset...");
            Reset();
            break;
        case 'c':
           printf("C:0x");
           for(uint8_t i = 0; i < 8; i++){
                SystemCounter = (SystemCounter << 4) | (SystemCounter >> 28);
                uint8_t tmpSC8 = ((uint8_t) SystemCounter) & 0x0F;
                putch(numToCharArray[tmpSC8]);
            }
            break;
        case 'i':
            printf("#Rst:0x%X", numberOfRestarts);
            break;
        case 'k':
            writeToBCM(7, 0);
            for(uint8_t i = 0; i < 7; i++){
                writeToBCM(i, 0);
            }
            printf("kill...");
            break;
        case 'd':
            irDumpFlag++;
            break;
        case 'h':
            printf("cdeikmpqrs[");//smqercikd");//cmd:{s:'set',m:'lightSetMode',q:'query',e:'errors',r:'reset',c:'sysCnt',i:'#Rst',k:'kill',d:'irDump'}");
            break;
        case 'p':
            changeLightToPinMapping(in);
            break;
        case '[':
            printf("qmap:{");
            for(uint8_t i = 0; i < 8; i++){
                printf("%x:%x,", i, lightToPin[i]);
            }
            putch('}');
            break;
        default:
            printf(undefStr);
            printf(":%c", in);
            break;
    }
    putch('\n');
}


typedef struct SetLightStateS{
    char state;
    char l;
    char v;
}SetLightState;
SetLightState setLightState;
void setLight(char in){
    switch(setLightState.state){
        case 0:
            printf("s:");
            setLightState.state = 1;
            runFunc = &setLight;
            break;
        case 1:
            if(in == 'a' || (in >= '0' && in <= '7')){
                printf("{%c:", in);
                setLightState.l = in;
                setLightState.state = (lightSetMode)?3:2;
            }else{
                printf(undefStr);
                runFunc = &processCommand;
                setLightState.state = 0;
            }
            break;
        case 2:
            in = hexCharToNum(in);
            if(in == 0xFF){
                printf(undefStr);
                printf("}\n");
                runFunc = &processCommand;
                setLightState.state = 0;
            }else{
                setLightState.v = in;
                setLightState.state = 3;
            }
            break;
        case 3:
            if(!lightSetMode){
                in = hexCharToNum(in);
                if(in == 0xFF){
                    printf(undefStr);
                    printf("}\n");
                    runFunc = &processCommand;
                    setLightState.state = 0;
                    break;
                }
            }
            in = (setLightState.v << 4) + in;
            writeToBCM(setLightState.l, in);
            printf("0x%X}\n", in);
            setLightState.v = 0;
            setLightState.state = 0;
            runFunc = &processCommand;
            break;
    }
}

uint8_t changePinState = 0;
uint8_t light = 0;
void changeLightToPinMapping(char in){
    switch(changePinState){
        case 0:
            printf("map:");
            runFunc = &changeLightToPinMapping;
            changePinState = 1;
            break;
        case 1:
            if(in >= '0' && in <= '7'){
                printf("{%c:", in);
                light = in - '0';
                changePinState = 2;
            }else{
                printf("undef\n");
                runFunc = &processCommand;
                changePinState = 0;
            }
            break;
        case 2:
            if(in >= '0' && in <= '7'){
                printf("%c}\n",in);
                uint8_t pin = in - '0';
                lightToPin[light] = pin;
                eeprom_write(LightToPin_EEPROM_Start + light, pin);
            }else{
                printf("undef\n");
            }
            runFunc = &processCommand;
            changePinState = 0;
            break;

    }
}

void putch(char in){
    while(!ByteBuffer_Write(&txBuffer1, in)); //blocking
    PIE1bits.TX1IE = 1; //enable interrupt
}

void async_Eusart_1_Setup(uint8_t baudH, uint8_t baudL){
    TXSTA1bits.BRGH     = 1;
    BAUDCON1bits.BRG16  = 1;

    SPBRGH1bits.SP1BRGH = baudH;//0x06;
    SPBRG1bits.SP1BRG   = baudL;//0x82;

    ANSELCbits.ANSC6    = 0;//set to not analog
    ANSELCbits.ANSC7    = 0;
    TRISCbits.RC6       = 1;//set to input, the eusart circuitry handles setting these
    TRISCbits.RC7       = 1;
    TXSTA1bits.SYNC     = 0; //async mode
    TXSTA1bits.TXEN     = 1; //tx enable
    RCSTA1bits.CREN     = 1; //rx enable

    RCSTA1bits.SPEN     = 1; //serial port enable

    PIE1bits.TX1IE = 1; //interrupt enable
    PIE1bits.RC1IE = 1;
    IPR1bits.TX1IP = 1; //high
    IPR1bits.RC1IP = 1;
}

uint8_t hexCharToNum(char v){
    if(v >= '0' && v <= '9'){
        return v - '0';
    }else if(v >= 'a' && v <= 'f'){
        return v - 'a' + 10;
    }else if(v >= 'A' && v <='F'){
        return v - 'A' + 10;
    }
    return 0xFF;
}

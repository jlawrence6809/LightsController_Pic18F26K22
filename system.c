/******************************************************************************/
/*Files to Include                                                            */
/******************************************************************************/

#if defined(__XC)
    #include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
    #include <htc.h>        /* HiTech General Include File */
#endif

#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */

#include "system.h"

void ConfigureOscillator(void){
    //sets clock to 64MHz
    OSCCONbits.SCS = 0b00;
    OSCCONbits.IRCF = 0b111; //16MHz
    OSCTUNEbits.PLLEN = 1; //pll on
    OSCTUNEbits.TUN = 0b011111; //maximum overclock B)
}

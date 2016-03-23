/***************************************************
  This is our library for the Adafruit ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#if defined(__XC)
#include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
#include <htc.h>        /* HiTech General Include File */
#endif

#include <stdio.h>
#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */
#include <conio.h>
#include "shared.h"
#include "system.h"
#include "ILI9341_LCD.h"

#define LOW 0
#define HIGH 1

#define _cs PORTBbits.RB0
#define _dc PORTBbits.RB2
#define _mosi PORTBbits.RB3
#define _miso PORTBbits.RB5
#define _sclk PORTBbits.RB4
#define _rst PORTBbits.RB1

#define _cs_m TRISBbits.TRISB0
#define _dc_m TRISBbits.TRISB2
#define _mosi_m TRISBbits.TRISB3
#define _miso_m TRISBbits.TRISB5
#define _sclk_m TRISBbits.TRISB4
#define _rst_m TRISBbits.TRISB1

#define digitalWrite(pin,val) (pin = val)

void ILI9341_Loop(void);
void ILI9341_Init(void);
void spiwrite(uint8_t);
void writecommand(uint8_t);
void writedata(uint8_t);
static inline void spi_begin(void);
static inline void spi_begin(void);
static inline void spi_end(void);
static inline void spi_end(void);
void commandList(uint8_t *);
void setAddrWindow(uint16_t , uint16_t , uint16_t , uint16_t);
void pushColor(uint16_t);
void drawPixel(int16_t , int16_t , uint16_t);
void drawFastVLine(int16_t , int16_t , int16_t , uint16_t);
void drawFastHLine(int16_t , int16_t , int16_t , uint16_t);
void fillScreen(uint16_t);
void Rect(int16_t , int16_t , int16_t , int16_t , uint16_t);
uint16_t color565(uint8_t , uint8_t , uint8_t);
void setRotation(uint8_t );
void invertDisplay(bool );
uint8_t spiread(void);
uint8_t readdata(void);
uint8_t readcommand8(uint8_t , uint8_t);
uint8_t digitalRead(void);

uint8_t digitalRead(void){
    _miso_m = 1;
    NOP();
    NOP();
    NOP();
    uint8_t out = _miso;
    _miso_m = 0;
    return out;
}

uint8_t _height = 320;
uint8_t _width = 240;

void spiwrite(uint8_t c) {

    //Serial.print("0x"); Serial.print(c, HEX); Serial.print(", ");

    // Fast SPI bitbang swiped from LPD8806 library
    for (uint8_t bt = 0x80; bt; bt >>= 1) {
        if (c & bt) {
            digitalWrite(_mosi, HIGH);
        } else {
            digitalWrite(_mosi, LOW);
        }
        digitalWrite(_sclk, HIGH);
        digitalWrite(_sclk, LOW);
    }
}

void writecommand(uint8_t c) {
    //*dcport &=  ~dcpinmask;
    digitalWrite(_dc, LOW);
    //*clkport &= ~clkpinmask; // clkport is a NULL pointer when hwSPI==true
    //digitalWrite(_sclk, LOW);
    //*csport &= ~cspinmask;
    digitalWrite(_cs, LOW);

    spiwrite(c);

    //*csport |= cspinmask;
    digitalWrite(_cs, HIGH);
}

void writedata(uint8_t c) {
    //*dcport |=  dcpinmask;
    digitalWrite(_dc, HIGH);
    //*clkport &= ~clkpinmask; // clkport is a NULL pointer when hwSPI==true
    //digitalWrite(_sclk, LOW);
    //*csport &= ~cspinmask;
    digitalWrite(_cs, LOW);

    spiwrite(c);

    digitalWrite(_cs, HIGH);
    //*csport |= cspinmask;
}

// Rather than a bazillion writecommand() and writedata() calls, screen
// initialization commands and arguments are organized in these tables
// stored in PROGMEM.  The table may look bulky, but that's mostly the
// formatting -- storage-wise this is hundreds of bytes more compact
// than the equivalent code.  Companion function follows.
#define DELAY 0x80


// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.

//void commandList(uint8_t *addr) {
//
//    uint8_t numCommands, numArgs;
//    uint16_t ms;
//
//    numCommands = eeprom_read(addr++); // Number of commands to follow
//    while (numCommands--) { // For each command...
//        writecommand(eeprom_read(addr++)); //   Read, issue command
//        numArgs = eeprom_read(addr++); //   Number of args to follow
//        ms = numArgs & DELAY; //   If hibit set, delay follows args
//        numArgs &= ~DELAY; //   Mask out delay bit
//        while (numArgs--) { //   For each argument...
//            writedata(eeprom_read(addr++)); //     Read, issue argument
//        }
//
//        if (ms) {
//            ms = eeprom_read(addr++); // Read post-command delay time (ms)
//            if (ms == 255) ms = 500; // If 255, delay for 500 ms
//            __delay_ms(ms);
//        }
//    }
//}


void ILI9341_Loop(void){
   uint8_t blah = readcommand8(11,11);
   Rect(0, 0, 20, 20, ILI9341_RED);
  delay_10ms(100);

  fillScreen(ILI9341_BLACK);
  delay_10ms(50);
  fillScreen(ILI9341_RED);
  delay_10ms(50);
  fillScreen(ILI9341_GREEN);
  delay_10ms(50);
  fillScreen(ILI9341_BLUE);
  delay_10ms(50);
  fillScreen(ILI9341_BLACK);
  delay_10ms(50);
}

void ILI9341_Init(void) {
    PORTBbits.RB0 = 0;
    PORTBbits.RB1 = 0;
    PORTBbits.RB2 = 0;
    PORTBbits.RB3 = 0;
    PORTBbits.RB4 = 0;

    ANSELBbits.ANSB0 = 0;
    ANSELBbits.ANSB1 = 0;
    ANSELBbits.ANSB2 = 0;
    ANSELBbits.ANSB3 = 0;
    ANSELBbits.ANSB4 = 0;

    TRISBbits.TRISB0 = 0;
    TRISBbits.TRISB1 = 0;
    TRISBbits.TRISB2 = 0;
    TRISBbits.TRISB3 = 0;
    TRISBbits.TRISB4 = 0;

        digitalWrite(_rst, HIGH);
        __delay_ms(5);
        digitalWrite(_rst, LOW);
        delay_10ms(2);
        digitalWrite(_rst, HIGH);
        delay_10ms(15);

    /*
    uint8_t x = readcommand8(ILI9341_RDMODE);
    Serial.print("\nDisplay Power Mode: 0x"); Serial.println(x, HEX);
    x = readcommand8(ILI9341_RDMADCTL);
    Serial.print("\nMADCTL Mode: 0x"); Serial.println(x, HEX);
    x = readcommand8(ILI9341_RDPIXFMT);
    Serial.print("\nPixel Format: 0x"); Serial.println(x, HEX);
    x = readcommand8(ILI9341_RDIMGFMT);
    Serial.print("\nImage Format: 0x"); Serial.println(x, HEX);
    x = readcommand8(ILI9341_RDSELFDIAG);
    Serial.print("\nSelf Diagnostic: 0x"); Serial.println(x, HEX);
     */
    //if(cmdList) commandList(cmdList);

    writecommand(0xEF);
    writedata(0x03);
    writedata(0x80);
    writedata(0x02);

    writecommand(0xCF);
    writedata(0x00);
    writedata(0XC1);
    writedata(0X30);

    writecommand(0xED);
    writedata(0x64);
    writedata(0x03);
    writedata(0X12);
    writedata(0X81);

    writecommand(0xE8);
    writedata(0x85);
    writedata(0x00);
    writedata(0x78);

    writecommand(0xCB);
    writedata(0x39);
    writedata(0x2C);
    writedata(0x00);
    writedata(0x34);
    writedata(0x02);

    writecommand(0xF7);
    writedata(0x20);

    writecommand(0xEA);
    writedata(0x00);
    writedata(0x00);

    writecommand(ILI9341_PWCTR1); //Power control
    writedata(0x23); //VRH[5:0]

    writecommand(ILI9341_PWCTR2); //Power control
    writedata(0x10); //SAP[2:0];BT[3:0]

    writecommand(ILI9341_VMCTR1); //VCM control
    writedata(0x3e); //
    writedata(0x28);

    writecommand(ILI9341_VMCTR2); //VCM control2
    writedata(0x86); //--

    writecommand(ILI9341_MADCTL); // Memory Access Control
    writedata(0x48);

    writecommand(ILI9341_PIXFMT);
    writedata(0x55);

    writecommand(ILI9341_FRMCTR1);
    writedata(0x00);
    writedata(0x18);

    writecommand(ILI9341_DFUNCTR); // Display Function Control
    writedata(0x08);
    writedata(0x82);
    writedata(0x27);

    writecommand(0xF2); // 3Gamma Function Disable
    writedata(0x00);

    writecommand(ILI9341_GAMMASET); //Gamma curve selected
    writedata(0x01);

    writecommand(ILI9341_GMCTRP1); //Set Gamma
    writedata(0x0F);
    writedata(0x31);
    writedata(0x2B);
    writedata(0x0C);
    writedata(0x0E);
    writedata(0x08);
    writedata(0x4E);
    writedata(0xF1);
    writedata(0x37);
    writedata(0x07);
    writedata(0x10);
    writedata(0x03);
    writedata(0x0E);
    writedata(0x09);
    writedata(0x00);

    writecommand(ILI9341_GMCTRN1); //Set Gamma
    writedata(0x00);
    writedata(0x0E);
    writedata(0x14);
    writedata(0x03);
    writedata(0x11);
    writedata(0x07);
    writedata(0x31);
    writedata(0xC1);
    writedata(0x48);
    writedata(0x08);
    writedata(0x0F);
    writedata(0x0C);
    writedata(0x31);
    writedata(0x36);
    writedata(0x0F);

    writecommand(ILI9341_SLPOUT); //Exit Sleep
    delay_10ms(12);
    writecommand(ILI9341_DISPON); //Display on

}

void setAddrWindow(uint16_t x0,   uint16_t y0, uint16_t x1, uint16_t y1) {

    writecommand(ILI9341_CASET); // Column addr set
    writedata(x0 >> 8);
    writedata(x0 & 0xFF); // XSTART
    writedata(x1 >> 8);
    writedata(x1 & 0xFF); // XEND

    writecommand(ILI9341_PASET); // Row addr set
    writedata(y0 >> 8);
    writedata(y0); // YSTART
    writedata(y1 >> 8);
    writedata(y1); // YEND

    writecommand(ILI9341_RAMWR); // write to RAM
}

void pushColor(uint16_t color) {
    digitalWrite(_dc, HIGH);
    //*dcport |=  dcpinmask;
    digitalWrite(_cs, LOW);
    //*csport &= ~cspinmask;

    spiwrite(color >> 8);
    spiwrite(color);

    //*csport |= cspinmask;
    digitalWrite(_cs, HIGH);
}

void drawPixel(int16_t x, int16_t y, uint16_t color) {

    if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height)) return;

    setAddrWindow(x, y, x + 1, y + 1);

    digitalWrite(_dc, HIGH);
    //*dcport |=  dcpinmask;
    digitalWrite(_cs, LOW);
    //*csport &= ~cspinmask;

    spiwrite(color >> 8);
    spiwrite(color);

    //*csport |= cspinmask;
    digitalWrite(_cs, HIGH);
}

void drawFastVLine(int16_t x, int16_t y, int16_t h,
        uint16_t color) {

    // Rudimentary clipping
    if ((x >= _width) || (y >= _height)) return;

    if ((y + h - 1) >= _height)
        h = _height - y;

    setAddrWindow(x, y, x, y + h - 1);

    uint8_t hi = color >> 8, lo = color;

    //*dcport |=  dcpinmask;
    digitalWrite(_dc, HIGH);
    //*csport &= ~cspinmask;
    digitalWrite(_cs, LOW);

    while (h--) {
        spiwrite(hi);
        spiwrite(lo);
    }
    //*csport |= cspinmask;
    digitalWrite(_cs, HIGH);
}

void drawFastHLine(int16_t x, int16_t y, int16_t w,
        uint16_t color) {

    // Rudimentary clipping
    if ((x >= _width) || (y >= _height)) return;
    if ((x + w - 1) >= _width) w = _width - x;
    setAddrWindow(x, y, x + w - 1, y);

    uint8_t hi = color >> 8, lo = color;
    //*dcport |=  dcpinmask;
    //*csport &= ~cspinmask;
    digitalWrite(_dc, HIGH);
    digitalWrite(_cs, LOW);
    while (w--) {
        spiwrite(hi);
        spiwrite(lo);
    }
    //*csport |= cspinmask;
    digitalWrite(_cs, HIGH);
}

void fillScreen(uint16_t color) {
    Rect(0, 0, _width, _height, color);
}

// fill a rectangle

void Rect(int16_t x, int16_t y, int16_t w, int16_t h,
        uint16_t color) {

    // rudimentary clipping (drawChar w/big text requires this)
    if ((x >= _width) || (y >= _height)) return;
    if ((x + w - 1) >= _width) w = _width - x;
    if ((y + h - 1) >= _height) h = _height - y;

    setAddrWindow(x, y, x + w - 1, y + h - 1);

    uint8_t hi = color >> 8, lo = color;

    digitalWrite(_dc, HIGH);
    digitalWrite(_cs, LOW);

    for (y = h; y > 0; y--) {
        for (x = w; x > 0; x--) {
            spiwrite(hi);
            spiwrite(lo);
        }
    }
    digitalWrite(_cs, HIGH);
}


// Pass 8-bit (each) R,G,B, get back 16-bit packed color

uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}


#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

void setRotation(uint8_t m) {

    writecommand(ILI9341_MADCTL);
    uint8_t rotation = m % 4; // can't be higher than 3
    switch (rotation) {
        case 0:
            writedata(MADCTL_MX | MADCTL_BGR);
            _width = ILI9341_TFTWIDTH;
            _height = ILI9341_TFTHEIGHT;
            break;
        case 1:
            writedata(MADCTL_MV | MADCTL_BGR);
            _width = ILI9341_TFTHEIGHT;
            _height = ILI9341_TFTWIDTH;
            break;
        case 2:
            writedata(MADCTL_MY | MADCTL_BGR);
            _width = ILI9341_TFTWIDTH;
            _height = ILI9341_TFTHEIGHT;
            break;
        case 3:
            writedata(MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
            _width = ILI9341_TFTHEIGHT;
            _height = ILI9341_TFTWIDTH;
            break;
    }
}

void invertDisplay(bool i) {
    writecommand(i ? ILI9341_INVON : ILI9341_INVOFF);
}


////////// stuff not actively being used, but kept for posterity

uint8_t spiread(void) {
    uint8_t r = 0;

        for (uint8_t i = 0; i < 8; i++) {
            digitalWrite(_sclk, LOW);
            digitalWrite(_sclk, HIGH);
            r <<= 1;
            if (digitalRead())
                r |= 0x1;
        }

    //Serial.print("read: 0x"); Serial.print(r, HEX);

    return r;
}

uint8_t readdata(void) {
    digitalWrite(_dc, HIGH);
    digitalWrite(_cs, LOW);
    uint8_t r = spiread();
    digitalWrite(_cs, HIGH);

    return r;
}

uint8_t readcommand8(uint8_t c, uint8_t index) {
    digitalWrite(_dc, LOW); // command
    digitalWrite(_cs, LOW);
    spiwrite(0xD9); // woo sekret command?
    digitalWrite(_dc, HIGH); // data
    spiwrite(0x10 + index);
    digitalWrite(_cs, HIGH);

    digitalWrite(_dc, LOW);
    digitalWrite(_sclk, LOW);
    digitalWrite(_cs, LOW);
    spiwrite(c);

    digitalWrite(_dc, HIGH);
    uint8_t r = spiread();
    digitalWrite(_cs, HIGH);
    return r;
}

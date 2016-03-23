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

uint8_t ByteBuffer_Read(ByteBuffer*);
uint8_t ByteBuffer_Write(ByteBuffer*, uint8_t);
void ByteBuffer_Construct(ByteBuffer*);//, void*, void*);
void ByteBuffer_Write_Str(ByteBuffer*, char*);


void ByteBuffer_Construct(ByteBuffer* bb){//, void* OnNotEmpty){
//    bb->OnEmpty = OnEmpty;
    //bb->OnNotEmpty = OnNotEmpty;
    bb->ReadIdx = ByteBufferSizeMask;
    bb->WriteIdx = ByteBufferSizeMask;
}

uint8_t ByteBuffer_Read(ByteBuffer* bb){
    if(bb->Count == 0) return 0;
    bb->Count--;
    bb->ReadIdx = (bb->ReadIdx+1)&ByteBufferSizeMask;
    return bb->Data[bb->ReadIdx];
}

void ByteBuffer_Write_Str(ByteBuffer* bb, char* str){
    while(*str != '\0'){
        ByteBuffer_Write(bb,(uint8_t) *str);
        str++;
    }
}

uint8_t ByteBuffer_Write(ByteBuffer* bb, uint8_t val){
    if(bb->Count == ByteBufferSizeMask) return 0;
    bb->Count++;
    bb->WriteIdx = (bb->WriteIdx+1)&ByteBufferSizeMask;
    bb->Data[bb->WriteIdx] = val;
    return 1;
}

/*
  * File:   shared.h
  * Author: jeremy.lawrence
  *
  * Created on October 26, 2014, 5:12 PM
  */

#ifndef SHARED_H
#define SHARED_H

#ifdef  __cplusplus
extern "C" {
#endif
#include "ByteBuffer.h"

void LCDInit(void);
void LCDLoop();

#ifdef  __cplusplus
}
#endif

#endif  /* SHARED_H */

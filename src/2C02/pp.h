// pp.h
// Author: hyan23
// Date: 2018.03.30

#ifndef __PP_H__

#include "header.h"

#define CNES_PP_BUFFER      (640*480)

extern void cnes_pp_blur(byte* img, uint w, uint h);
extern void cnes_pp_filter(byte* img, byte kernel[3][3], uint w, uint h);

#endif /* __PP_H__ */
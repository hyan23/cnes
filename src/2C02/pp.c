// pp.c
// Author: hyan23
// Date: 2018.03.30

#include "header.h"
#include "pp.h"

static byte* sort_half(byte* array, uint size)
{
    for (uint i = 0; i < size / 2; i ++) {
        byte min = ~0;
        uint index = 0;
        for (uint j = i; j < size; j ++) {
            if (array[j] < min) {
                min = array[j];
                index = j;
            }
        }
        array[index] = array[i];
        array[i] = min;
    }
    return array;
}

#define NEIGHBORS   4

static sint visits[NEIGHBORS][2] = {
    { -1, 0 }, { 0, -1 }, { 0, 1 }, { 1, 0 }
};

void cnes_pp_blur(byte* img, uint w, uint h)
{
    assert(w * h < CNES_PP_BUFFER);
    byte buf[CNES_PP_BUFFER];
    byte* temp = buf;
    for (uint i = 0; i < h; i ++) {
        for (uint j = 0; j < w; j ++) {
            byte array[NEIGHBORS];
            for (uint k = 0; k < NEIGHBORS; k ++) {
                sint x = (j + visits[k][1]);
                sint y = (i + visits[k][0]);
                if (x >= w) x = w - 1;
                if (y >= h) y = h - 1;
                if (x < 0)  x = 0;
                if (y < 0)  y = 0;
                array[k] = *(img + y * w + x);
            }
            *(temp + i * w + j) = sort_half(array, 4)[NEIGHBORS / 2];
        }
    }
    memcpy(img, temp, w * h);
}

void cnes_pp_filter(byte* img, byte kernel[3][3], uint w, uint h)
{
    // TODO: 
}
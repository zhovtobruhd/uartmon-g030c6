#ifndef __FONTS_H
#define __FONTS_H

#include "stdint.h"

// font parameters main structure
typedef struct {
    const unsigned char FontWidth;
    const unsigned char FontHeight;
    const uint16_t *fontEn;
} FontDef;

// some available fonts
extern FontDef Font_7x9;
extern FontDef Font_7x10;
extern FontDef Font_11x18;
extern FontDef Font_16x26;

#endif

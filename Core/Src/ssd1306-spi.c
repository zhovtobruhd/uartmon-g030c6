#include "ssd1306-spi.h"

uint8_t SSD1306_GRAM[SSD1306_WIDTH][SSD1306_HEIGHT / 8];

static SSD1306_t SSD1306;

void OLED_WR_Byte(uint8_t dat,uint8_t cmd) {
    if (cmd) {
        DC_SET;
    } else {
        DC_CLR;
    }
#if SSD1306_USE_CS
    CS_CLR;
#endif
    for (uint8_t i = 0; i < 8; i++) {
        SCL_CLR;
        if (dat & 0x80) SDA_SET;
        else
            SDA_CLR;
        SCL_SET;
        dat <<= 1;
    }
#if SSD1306_USE_CS
    CS_SET;
#endif
    DC_SET;
}

void ssd1306_UpdateScreen(void) {
    for (uint8_t i = 0; i < 8; i++) {
        uint8_t commands[3] = {0xB0+i, 0x00, 0x10};
        ssd1306_SendToDisplay(Commands, commands, 3);
        for (uint8_t n = 0; n < 128; n++) {
            ssd1306_SendToDisplay(Datas, &SSD1306_GRAM[n][i], 1);
        }
    }
}

void ssd1306_SendToDisplay(SSD1306_DATA_TYPE type, uint8_t *data, uint8_t length) {
    for (uint8_t k = 0; k < length; ++k){
        OLED_WR_Byte(data[k], type == Commands? 0 : 1);
    }
}

void ssd1306_Display_On(void) {
    uint8_t commands[3] = {0x8D, 0x14, 0xAF};
    ssd1306_SendToDisplay(Commands, commands, 3);
}

void ssd1306_Display_Off(void) {
    uint8_t commands[3] = {0x8D, 0x10, 0xAE};
    ssd1306_SendToDisplay(Commands, commands, 3);
}

void ssd1306_Fill(SSD1306_COLOR color) {
    for (uint8_t i = 0; i < 8; i++) {
        for (uint8_t n = 0; n < 128; n++) {
            SSD1306_GRAM[n][i] = color ? 0xFF : 0x00;
        }
    }
    ssd1306_UpdateScreen();
}

void ssd1306_DrawPixel(uint32_t x, uint32_t y, SSD1306_COLOR color) {
    uint8_t pos, bx, temp = 0;
    if (x > 127 || y > 63) return;
    pos = 7 - y / 8;
    bx = y % 8;
    temp = 1 << (7 - bx);
    if (color) {
        SSD1306_GRAM[x][pos] |= temp;
    } else {
        SSD1306_GRAM[x][pos] &= ~temp;
    }
}

void ssd1306_FillRect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1306_COLOR color) {
    for (uint8_t x = x1; x <= x2; x++) {
        for (uint8_t y = y1; y <= y2; y++) {
            ssd1306_DrawPixel(x, y, color);
        }
    }
}

void ssd1306_Init(void) {
    SDA_SET;
    SCL_SET;

#if SSD1306_USE_CS
    CS_SET;
#endif

    DC_SET;

    RST_CLR;
    HAL_Delay(100);
    RST_SET;
    HAL_Delay(100);

    uint8_t commands[25] = {0xAE, 0xD5, 0x50, 0xA8, 0x3F, 0xD3, 0x00,
                          0x40,
                          0x8D, 0x14, 0x20, 0x02, 0xA1, 0xC0, 0xDA, 0x12,
                          0x81, 0xEF, 0xD9, 0xF1, 0xDB, 0x30, 0xA4, 0xA6, 0xAF};

    ssd1306_SendToDisplay(Commands, commands, 25);

    ssd1306_Fill(Black);
}

void ssd1306_SetContrast(uint8_t contrast) {
    //contrast command with empty data
    uint8_t contrast_data[2] = {0x81};
    //adding contrast data
    contrast_data[1] = contrast;
    //and send to display
    ssd1306_SendToDisplay(Commands, contrast_data, 2);
}

uint16_t ssd1306_absDifference(uint16_t a, uint16_t b) {
    return a > b ? a - b : b - a;
}

void ssd1306_DrawLine(int x1, int y1, int x2, int y2, SSD1306_COLOR color) {
    const int deltaX = ssd1306_absDifference(x2, x1);
    const int deltaY = ssd1306_absDifference(y2, y1);
    const int signX = x1 < x2 ? 1 : -1;
    const int signY = y1 < y2 ? 1 : -1;

    int error = deltaX - deltaY;

    ssd1306_DrawPixel(x2, y2, color);
    while(x1 != x2 || y1 != y2) {
        ssd1306_DrawPixel(x1, y1, color);
        const int error2 = error * 2;

        if(error2 > -deltaY) {
            error -= deltaY;
            x1 += signX;
        }
        if(error2 < deltaX) {
            error += deltaX;
            y1 += signY;
        }
    }
}

void ssd1306_SetCursor(uint32_t x, uint32_t y) {
    SSD1306.CurrentX = x;
    SSD1306.CurrentY = y;
}

void ssd1306_WriteChar(char ch, FontDef Font, SSD1306_COLOR color) {
    uint32_t pixel, x;

    if ((((SSD1306.CurrentX + Font.FontWidth) < SSD1306_WIDTH) &&
         ((SSD1306.CurrentY + Font.FontHeight) < SSD1306_HEIGHT))) {
        // write char to display buffer
        for (uint32_t y = 0; y < Font.FontHeight; y++) {
            pixel = Font.fontEn[(ch - 32) * Font.FontHeight + y];

            // write pixel to display buffer
            x = Font.FontWidth;
            while (x--) {
                if (pixel & 0x0001) {
                    ssd1306_DrawPixel(SSD1306.CurrentX + x, (SSD1306.CurrentY + y), color);
                } else {
                    ssd1306_DrawPixel(SSD1306.CurrentX + x, (SSD1306.CurrentY + y), !color);
                }
                pixel >>= 1;
            }
        }
    }

    // going to next position
    SSD1306.CurrentX += Font.FontWidth;
}

void ssd1306_WriteString(char* str, FontDef Font, SSD1306_COLOR color) {
    while (*str) {
        ssd1306_WriteChar(*str, Font, color);
        str++;
    }
}

void ssd1306_WriteStringLen(char* str, uint32_t length, FontDef Font, SSD1306_COLOR color) {
    for (uint32_t i = 0; i < length; ++i) {
        ssd1306_WriteChar(*str, Font, color);
        str++;
    }
}

void ssd1306_ShiftUp() {
    for (uint16_t row = 0; row < SSD1306_HEIGHT - 1; row++) {
        for (uint16_t i = 0; i < SSD1306_WIDTH; ++i) {
            SSD1306_GRAM[i][row] = SSD1306_GRAM[i][row + 1];
        }
    }
    for (uint16_t i = 0; i < SSD1306_WIDTH; ++i) {
        SSD1306_GRAM[i][SSD1306_HEIGHT - 1] = 0;
    }

}

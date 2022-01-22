#ifndef SSD1306_SPI_H
#define SSD1306_SPI_H

#include <stdlib.h>

#include "stm32g0xx_hal.h"

#include "fonts.h"

#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64

// Set this to 1 if you have CS pin on your display
#define SSD1306_USE_CS 0

// Change defines
#define SCL_Pin GPIO_PIN_6
#define SCL_GPIO_Port GPIOB

#define SDA_Pin GPIO_PIN_7
#define SDA_GPIO_Port GPIOB

#define RST_Pin GPIO_PIN_8
#define RST_GPIO_Port GPIOB

#define DC_Pin GPIO_PIN_9
#define DC_GPIO_Port GPIOB

#if SSD1306_USE_CS
#define CS_Pin GPIO_PIN_7
#define CS_GPIO_Port GPIOB
#endif

#define GPIO_SET(Port, Pin) Port->ODR|=Pin
#define GPIO_CLR(Port, Pin) Port->ODR&=~Pin

#define DC_SET GPIO_SET(DC_GPIO_Port, DC_Pin)
#define DC_CLR GPIO_CLR(DC_GPIO_Port, DC_Pin)

#define RST_SET GPIO_SET(RST_GPIO_Port, RST_Pin)
#define RST_CLR GPIO_CLR(RST_GPIO_Port, RST_Pin)

#define SDA_SET GPIO_SET(SDA_GPIO_Port, SDA_Pin)
#define SDA_CLR GPIO_CLR(SDA_GPIO_Port, SDA_Pin)

#define SCL_SET GPIO_SET(SCL_GPIO_Port, SCL_Pin)
#define SCL_CLR GPIO_CLR(SCL_GPIO_Port, SCL_Pin)

#if SSD1306_USE_CS
#define CS_SET GPIO_SET(CS_GPIO_Port, CS_Pin)
#define CS_CLR GPIO_CLR(CS_GPIO_Port, CS_Pin)
#endif

//main struct with display config data
typedef struct {
    uint32_t CurrentX;
    uint32_t CurrentY;
} SSD1306_t;
// available colors
typedef enum {
    Black = 0x00, // pixel off
    White = 0x01  // pixel on
} SSD1306_COLOR;

// available display communication data types
typedef enum {
    Datas = 0x01,
    Commands = 0x00
} SSD1306_DATA_TYPE;

// initialize display
void ssd1306_Init(void);

// fill display buffer with chosen color
void ssd1306_Fill(SSD1306_COLOR color);
// fill rectangle with color
void ssd1306_FillRect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1306_COLOR color);
// draw single pixel to display buffer
void ssd1306_DrawPixel(uint32_t x, uint32_t y, SSD1306_COLOR color);
// draw char symbol to display buffer
void ssd1306_WriteChar(char ch, FontDef Font, SSD1306_COLOR color);
// draw char  string to display buffer
void ssd1306_WriteString(char* str, FontDef Font, SSD1306_COLOR color);
// draw string of specified length to display buffer
void ssd1306_WriteStringLen(char* str, uint32_t length, FontDef Font, SSD1306_COLOR color);
// set cursor for drawing
void ssd1306_SetCursor(uint32_t x, uint32_t y);
// Bresenham algorithm for drawing lines
void ssd1306_DrawLine(int x1, int y1, int x2, int y2, SSD1306_COLOR color);

// update information on display
void ssd1306_UpdateScreen(void);

// set contrast of display (0 to 255)
void ssd1306_SetContrast(uint8_t contrast);
// turns the display OFF
void ssd1306_Display_Off(void);
// turns the display ON
void ssd1306_Display_On(void);

void ssd1306_ShiftUp();
// internal functions

// send data or command buffer to display via softSPI interface
void ssd1306_SendToDisplay(SSD1306_DATA_TYPE type, uint8_t *data, uint8_t length);
// calculate absolute difference between two values
uint16_t ssd1306_absDifference(uint16_t a, uint16_t b);

#endif //SSD1306_SPI_H

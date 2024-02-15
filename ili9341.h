// MIT License

// Copyright (c) 2024 phonght32

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef __ILI9341_H__
#define __ILI9341_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "err_code.h"
#include "fonts.h"

typedef err_code_t (*ili9341_spi_send)(uint8_t *buf_send, uint32_t len, uint32_t timeout_ms);
typedef err_code_t (*ili9341_set_dc)(uint8_t level);
typedef err_code_t (*ili9341_set_rst)(uint8_t level);
typedef err_code_t (*ili9341_delay)(uint32_t delay_ms);

/**
 * @brief   Handle structure.
 */
typedef struct ili9341 *ili9341_handle_t;

/**
 * @brief   Configuration structure.
 */
typedef struct {
    uint16_t                height;             /*!< Screen height */
    uint16_t                width;              /*!< Screen width */
    ili9341_spi_send        spi_send;           /*!< Function send SPI */
    ili9341_set_dc          set_dc;             /*!< Function set pin DC */
    ili9341_set_rst         set_rst;            /*!< Function set pin RST */
    ili9341_delay           delay;              /*!< Function delay */
} ili9341_cfg_t;

/*
 * @brief   Initialize ILI9341 with default parameters.
 *
 * @note    This function must be called first.
 *
 * @param   None.
 *
 * @return
 *      - Handle structure: Success.
 *      - Others:           Fail.
 */
ili9341_handle_t ili9341_init(void);

/*
 * @brief   Set configuration parameters.
 *
 * @param   handle Handle structure.
 * @param   config Configuration structure.
 *
 * @return
 *      - ERR_CODE_SUCCESS: Success.
 *      - Others:           Fail.
 */
err_code_t ili9341_set_config(ili9341_handle_t handle, ili9341_cfg_t config);

/*
 * @brief   Configure ILI9341 to run.
 *
 * @param   handle Handle structure.
 *
 * @return
 *      - ERR_CODE_SUCCESS: Success.
 *      - Others:           Fail.
 */
err_code_t ili9341_config(ili9341_handle_t handle);

/*
 * @brief   Refresh screen.
 *
 * @param   handle Handle structure.
 *
 * @return
 *      - ERR_CODE_SUCCESS: Success.
 *      - Others:           Fail.
 */
err_code_t ili9341_refresh(ili9341_handle_t handle);

/**
 * @brief   Fill screen with color.
 *
 * @param   handle Handle structure.
 * @param   color Color.
 *
 * @return
 *      - ERR_CODE_SUCCESS: Success.
 *      - Others:           Fail.
 */
err_code_t ili9341_fill(ili9341_handle_t handle, uint32_t color);

/**
 * @brief   Write character.
 *
 * @param   handle Handle structure.
 * @param   font_size Font size.
 * @param   chr Character.
 * @param   color Color.
 *
 * @return
 *      - ERR_CODE_SUCCESS: Success.
 *      - Others:           Fail.
 */
err_code_t ili9341_write_char(ili9341_handle_t handle, font_size_t font_size, uint8_t chr, uint32_t color);

/**
 * @brief   Write string.
 *
 * @param   handle Handle structure.
 * @param   font_size Font size.
 * @param   str Pointer references to the data.
 * @param   color Color.
 *
 * @return
 *      - ERR_CODE_SUCCESS: Success.
 *      - Others:           Fail.
 */
err_code_t ili9341_write_string(ili9341_handle_t handle, font_size_t font_size, uint8_t *str, uint32_t color);

/**
 * @brief   Draw pixel.
 *
 * @param   handle Handle structure.
 * @param   x Horizontal position.
 * @param   y Vertical position.
 * @param   color Color.
 *
 * @return
 *      - ERR_CODE_SUCCESS: Success.
 *      - Others:           Fail.
 */
err_code_t ili9341_draw_pixel(ili9341_handle_t handle, uint16_t x, uint16_t y, uint32_t color);

/**
 * @brief   Draw line.
 *
 * @param   handle Handle structure.
 * @param   x1 The first horizontal position.
 * @param   y1 The first vertical postion.
 * @param   x2 The second horizontal position.
 * @param   y2 The second vertical position.
 * @param   color Color.
 *
 * @return
 *      - ERR_CODE_SUCCESS: Success.
 *      - Others:           Fail.
 */
err_code_t ili9341_draw_line(ili9341_handle_t handle, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color);

/**
 * @brief   Draw rectangle.
 *
 * @param   handle Handle structure.
 * @param   x_origin Origin horizontal position.
 * @param   y_origin Origin vertical position.
 * @param   width Width in pixel.
 * @param   height Height in pixel.
 * @param   color Color.
 *
 * @return
 *      - ERR_CODE_SUCCESS: Success.
 *      - Others:           Fail.
 */
err_code_t ili9341_draw_rectangle(ili9341_handle_t handle, uint16_t x_origin, uint16_t y_origin, uint16_t width, uint16_t height, uint32_t color);

/**
 * @brief   Draw Circle.
 *
 * @param   handle Handle structure.
 * @param   x_origin Origin horizontal position.
 * @param   y_origin Origin vertical position.
 * @param   radius Radius in pixel.
 * @param   color Color.
 *
 * @return
 *      - ERR_CODE_SUCCESS: Success.
 *      - Others:           Fail.
 */
err_code_t ili9341_draw_circle(ili9341_handle_t handle, uint16_t x_origin, uint16_t y_origin, uint16_t radius, uint32_t color);

/**
 * @brief   Set current position.
 *
 * @param   handle Handle structure.
 * @param   x Horizontal position.
 * @param   y Vertical position.
 *
 * @return
 *      - ERR_CODE_SUCCESS: Success.
 *      - Others:           Fail.
 */
err_code_t ili9341_set_position(ili9341_handle_t handle, uint16_t x, uint16_t y);

/**
 * @brief   Get current position.
 *
 * @param   handle Handle structure.
 * @param   x Pointer references to the horizontal position.
 * @param   y Pointer references to the vertical position.
 *
 * @return
 *      - ERR_CODE_SUCCESS: Success.
 *      - Others:           Fail.
 */
err_code_t ili9341_get_position(ili9341_handle_t handle, uint16_t *x, uint16_t *y);

/*
 * @brief   Get screen buffer.
 *
 * @param   handle Handle structure.
 *
 * @return
 *      - Screen buffer address.
 *      - NULL: Fail.
 */
uint8_t* ili9341_get_buffer(ili9341_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif /* __ILI9341_H__ */

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

typedef err_code_t (*ili9341_spi_send)(uint8_t *buf_send, uint32_t len);
typedef err_code_t (*ili9341_set_gpio)(uint8_t level);
typedef void (*ili9341_delay)(uint32_t delay_ms);

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
    uint8_t                 *screen_buffer;     /*!< Screen buffer*/
    ili9341_spi_send        spi_send;           /*!< Function send SPI */
    ili9341_set_gpio        set_dc;             /*!< Function set pin DC */
    ili9341_set_gpio        set_rst;            /*!< Function set pin RST */
    ili9341_set_gpio        set_bckl;           /*!< Function on/off LED backlight */
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
err_code_t ili9341_set_position(ili9341_handle_t handle, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

/**
 * @brief   Turn on LED backlight.
 *
 * @param   handle Handle structure.
 *
 * @return
 *      - ERR_CODE_SUCCESS: Success.
 *      - Others:           Fail.
 */
err_code_t ili9341_set_bckl_on(ili9341_handle_t handle);

/**
 * @brief   Turn off LED backlight.
 *
 * @param   handle Handle structure.
 *
 * @return
 *      - ERR_CODE_SUCCESS: Success.
 *      - Others:           Fail.
 */
err_code_t ili9341_set_bckl_off(ili9341_handle_t handle);

/*
 * @brief   Get screen buffer.
 *
 * @param   handle Handle structure.
 *
 * @return
 *      - Screen buffer address.
 *      - NULL: Fail.
 */
uint8_t* ili9341_get_screen_buffer(ili9341_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif /* __ILI9341_H__ */

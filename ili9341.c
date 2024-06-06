#include "stdio.h"
#include "stdlib.h"
#include "ili9341.h"

#define SPI_PARALLEL_LINES  		16

#define ILI3941_RST_ACTIVE_LEVEL 	0
#define ILI3941_RST_UNACTIVE_LEVEL 	1

#define ILI3941_CS_ACTIVE_LEVEL 	0
#define ILI3941_CS_UNACTIVE_LEVEL 	1


/**
 * @struct  LCD configuration structure.
 */
typedef struct {
	uint8_t cmd;
	uint8_t data[16];
	uint8_t databytes; 		/*!< No of data in data; bit 7 = delay after set; 0xFF = end of cmds. */
} lcd_init_cmd_t;

/**
 * @struct 	LCD initialization commands.
 */
lcd_init_cmd_t ili_init_cmds[] = {
	/* Power contorl B, power control = 0, DC_ENA = 1 */
	{0xCF, {0x00, 0x83, 0X30}, 3},
	/* Power on sequence control,
	 * cp1 keeps 1 frame, 1st frame enable
	 * vcl = 0, ddvdh=3, vgh=1, vgl=2
	 * DDVDH_ENH=1
	 */
	{0xED, {0x64, 0x03, 0X12, 0X81}, 4},
	/* Driver timing control A,
	 * non-overlap=default +1
	 * EQ=default - 1, CR=default
	 * pre-charge=default - 1
	 */
	{0xE8, {0x85, 0x01, 0x79}, 3},
	/* Power control A, Vcore=1.6V, DDVDH=5.6V */
	{0xCB, {0x39, 0x2C, 0x00, 0x34, 0x02}, 5},
	/* Pump ratio control, DDVDH=2xVCl */
	{0xF7, {0x20}, 1},
	/* Driver timing control, all=0 unit */
	{0xEA, {0x00, 0x00}, 2},
	/* Power control 1, GVDD=4.75V */
	{0xC0, {0x26}, 1},
	/* Power control 2, DDVDH=VCl*2, VGH=VCl*7, VGL=-VCl*3 */
	{0xC1, {0x11}, 1},
	/* VCOM control 1, VCOMH=4.025V, VCOML=-0.950V */
	{0xC5, {0x35, 0x3E}, 2},
	/* VCOM control 2, VCOMH=VMH-2, VCOML=VML-2 */
	{0xC7, {0xBE}, 1},
	/* Memory access contorl, MX=MY=0, MV=1, ML=0, BGR=1, MH=0 */
	{0x36, {0x28}, 1},
	/* Pixel format, 16bits/pixel for RGB/MCU interface */
	{0x3A, {0x55}, 1},
	/* Frame rate control, f=fosc, 70Hz fps */
	{0xB1, {0x00, 0x1B}, 2},
	/* Enable 3G, disabled */
	{0xF2, {0x08}, 1},
	/* Gamma set, curve 1 */
	{0x26, {0x01}, 1},
	/* Positive gamma correction */
	{0xE0, {0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0X87, 0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00}, 15},
	/* Negative gamma correction */
	{0XE1, {0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F}, 15},
	/* Column address set, SC=0, EC=0xEF */
	{0x2A, {0x00, 0x00, 0x00, 0xEF}, 4},
	/* Page address set, SP=0, EP=0x013F */
	{0x2B, {0x00, 0x00, 0x01, 0x3f}, 4},
	/* Memory write */
	{0x2C, {0}, 0},
	/* Entry mode set, Low vol detect disabled, normal display */
	{0xB7, {0x07}, 1},
	/* Display function control */
	{0xB6, {0x0A, 0x82, 0x27, 0x00}, 4},
	/* Sleep out */
	{0x11, {0}, 0x80},
	/* Display on */
	{0x29, {0}, 0x80},
	{0, {0}, 0xff},
};

typedef struct {
	uint16_t *data;
} lines_t;

typedef struct ili9341 {
	uint16_t 				height;					/*!< Screen height */
	uint16_t 				width;					/*!< Screen width */
	ili9341_spi_send 		spi_send;				/*!< Function send SPI */
	ili9341_set_gpio        set_cs;             	/*!< Function set pin CS */
	ili9341_set_gpio        set_dc;             	/*!< Function set pin DC */
	ili9341_set_gpio        set_rst;            	/*!< Function set pin RST */
	ili9341_set_gpio        set_bckl;           	/*!< Function on/off LED */
	ili9341_delay 			delay;					/*!< Function delay */
	uint8_t 				*data;					/*!< Screen buffer */
	lines_t 				lines;					/*!< Lines buffer */
	uint16_t 				pos_x;					/*!< Current position x */
	uint16_t 				pos_y;					/*!< Current position y */
} ili9341_t;

static void convert_pixel_to_lines(ili9341_handle_t handle, int height_idx)
{
	/* Convert pixel data to RGB565 format */
	for (int idx = 0; idx < (handle->width * SPI_PARALLEL_LINES); idx++) {
		uint8_t *p_src = handle->data + handle->width * height_idx * 3 + idx * 3;
		uint16_t *p_desc = handle->lines.data + idx;

		uint16_t color_565 = (((uint16_t)p_src[0] & 0x00F8) << 8) |
		                     (((uint16_t)p_src[1] & 0x00FC) << 3) |
		                     ((uint16_t)p_src[2] >> 3);
		uint16_t swap565 = ((color_565 << 8) & 0xFF00) | ((color_565 >> 8) & 0x00FF);

		*p_desc = swap565;
	}
}

static void write_pixel(ili9341_handle_t handle, uint16_t x, uint16_t y, uint32_t color)
{
	uint8_t *p = handle->data + (x + y * handle->width) * 3;
	p[0] = (color >> 16) & 0xFF;
	p[1] = (color >> 8) & 0xFF;
	p[2] = (color >> 0) & 0xFF;
}

static void write_line(ili9341_handle_t handle, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color)
{
	int32_t deltaX = abs(x2 - x1);
	int32_t deltaY = abs(y2 - y1);
	int32_t signX = ((x1 < x2) ? 1 : -1);
	int32_t signY = ((y1 < y2) ? 1 : -1);
	int32_t error = deltaX - deltaY;
	int32_t error2;

	write_pixel(handle, x2, y2, color);

	while ((x1 != x2) || (y1 != y2))
	{
		write_pixel(handle, x1, y1, color);

		error2 = error * 2;
		if (error2 > -deltaY) {
			error -= deltaY;
			x1 += signX;
		} else {
			/*nothing to do*/
		}

		if (error2 < deltaX) {
			error += deltaX;
			y1 += signY;
		} else {
			/*nothing to do*/
		}
	}
}

static err_code_t ili9341_write_cmd(ili9341_handle_t handle, uint8_t cmd)
{
	if (handle->set_cs != NULL)
	{
		handle->set_cs(ILI3941_CS_ACTIVE_LEVEL);
	}

	/* DC level equal to 0 when write SPI command */
	handle->set_dc(0);

	/* Transfer command */
	handle->spi_send(&cmd, 1);

	if (handle->set_cs != NULL)
	{
		handle->set_cs(ILI3941_CS_UNACTIVE_LEVEL);
	}

	return ERR_CODE_SUCCESS;
}

static err_code_t ili9341_write_data(ili9341_handle_t handle, uint8_t *data, uint32_t len)
{
	if (handle->set_cs != NULL)
	{
		handle->set_cs(ILI3941_CS_ACTIVE_LEVEL);
	}

	/* DC level equal to 1 when write SPI data */
	handle->set_dc(1);

	/* Transfer data */
	handle->spi_send(data, len);

	if (handle->set_cs != NULL)
	{
		handle->set_cs(ILI3941_CS_UNACTIVE_LEVEL);
	}

	return ERR_CODE_SUCCESS;
}

static err_code_t ili9341_display_lines(ili9341_handle_t handle, uint16_t ypos, uint16_t parallel_line, uint16_t *lines_data)
{
	uint8_t buf[4] = {0, 0, 0, 0};

	/* Command set column address */
	ili9341_write_cmd(handle, 0x2A);

	buf[0] = 0;									/* Start column high */
	buf[1] = 0;									/* Start column low */
	buf[2] = handle->width >> 8;				/* End column high */
	buf[3] = handle->width & 0xFF;				/* End column low */
	ili9341_write_data(handle, buf, 4);

	/* Command set page address */
	ili9341_write_cmd(handle, 0x2B);

	buf[0] = ypos >> 8;							/* Start page high */
	buf[1] = ypos & 0xFF;						/* Start page low */
	buf[2] = (ypos + parallel_line) >> 8;		/* End page high */
	buf[3] = (ypos + parallel_line) & 0xff;		/* End page low */
	ili9341_write_data(handle, buf, 4);

	/* Command set data */
	ili9341_write_cmd(handle, 0x2C);

	/* Transfer screen data */
	ili9341_write_data(handle, (uint8_t*)lines_data, handle->width * sizeof(uint16_t) * parallel_line);

	return ERR_CODE_SUCCESS;
}

ili9341_handle_t ili9341_init(void)
{
	ili9341_handle_t handle = calloc(1, sizeof(ili9341_t));

	/* Check if handle structure is NULL */
	if (handle == NULL)
	{
		return NULL;
	}

	return handle;
}

err_code_t ili9341_set_config(ili9341_handle_t handle, ili9341_cfg_t config)
{
	/* Check if handle structure is NULL */
	if (handle == NULL)
	{
		return ERR_CODE_NULL_PTR;
	}

	handle->width = config.width;
	handle->height = config.height;
	handle->data = config.screen_buffer;
	handle->spi_send = config.spi_send;
	handle->set_cs = config.set_cs;
	handle->set_dc = config.set_dc;
	handle->set_rst = config.set_rst;
	handle->set_bckl = config.set_bckl;
	handle->delay = config.delay;


	return ERR_CODE_SUCCESS;
}

err_code_t ili9341_config(ili9341_handle_t handle)
{
	/* Check if handle structure is NULL */
	if (handle == NULL)
	{
		return ERR_CODE_NULL_PTR;
	}

	/* Allocate memory for lines buffer. These buffer will be used to store
	   temporarily data of screen buffer */
	handle->lines.data = calloc(handle->width * SPI_PARALLEL_LINES, sizeof(uint16_t));

	handle->set_rst(ILI3941_RST_ACTIVE_LEVEL);
	handle->delay(100);
	handle->set_rst(ILI3941_RST_UNACTIVE_LEVEL);
	handle->delay(100);

	int cmd = 0;
	lcd_init_cmd_t* lcd_init_cmds = ili_init_cmds;

	/* Configure screen */
	while (lcd_init_cmds[cmd].databytes != 0xff)
	{
		/* Transfer command mode */
		ili9341_write_cmd(handle, lcd_init_cmds[cmd].cmd);

		if (lcd_init_cmds[cmd].databytes == 0x80)
		{
			handle->delay(100);
		}
		else if (lcd_init_cmds[cmd].databytes != 0)
		{
			/* Transfer command data */
			ili9341_write_data(handle, lcd_init_cmds[cmd].data, lcd_init_cmds[cmd].databytes & 0x1F);
		}
		else
		{

		}

		cmd++;
	}

	return ERR_CODE_SUCCESS;
}

err_code_t ili9341_refresh(ili9341_handle_t handle)
{
	/* Check if handle structure is NULL */
	if (handle == NULL)
	{
		return ERR_CODE_NULL_PTR;
	}

	/* Display all data from screen buffer to screen. Every cycle, SPI_PARALLEL_LINES
	   rows will be updated */
	for (int y = 0; y < handle->height; y += SPI_PARALLEL_LINES)
	{
		/* Convert buffer data from RGB888 to RGB565 and put to lines buffer */
		convert_pixel_to_lines(handle, y);

		/* Display data to screen */
		ili9341_display_lines(handle, y, SPI_PARALLEL_LINES, handle->lines.data);
	}

	return ERR_CODE_SUCCESS;
}

err_code_t ili9341_fill(ili9341_handle_t handle, uint32_t color)
{
	/* Check if handle structure is NULL */
	if (handle == NULL)
	{
		return ERR_CODE_NULL_PTR;
	}

	/* Write RGB888 color to data buffer */
	for (int idx = 0; idx < (handle->width * handle->height); idx++)
	{
		uint8_t *p = handle->data + idx * 3;
		p[0] = (color >> 16) & 0xFF;
		p[1] = (color >> 8) & 0xFF;
		p[2] = (color >> 0) & 0xFF;
	}

	return ERR_CODE_SUCCESS;
}

err_code_t ili9341_write_char(ili9341_handle_t handle, font_size_t font_size, uint8_t chr, uint32_t color)
{
	/* Check if handle structure is NULL */
	if (handle == NULL)
	{
		return ERR_CODE_NULL_PTR;
	}

	/* Get font data */
	font_t font;
	if (get_font(chr, font_size, &font) <= 0)
	{
		return ERR_CODE_FAIL;
	}

	/* Write character pixel data to buffer */
	uint16_t num_byte_per_row = font.data_len / font.height;
	for (uint16_t height_idx = 0; height_idx < font.height; height_idx ++)
	{
		for ( uint8_t byte_idx = 0; byte_idx < num_byte_per_row; byte_idx++)
		{
			for (uint16_t width_idx = 0; width_idx < 8; width_idx++)
			{
				uint16_t x = handle->pos_x + width_idx + byte_idx * 8;
				uint16_t y = handle->pos_y + height_idx;
				if (((font.data[height_idx * num_byte_per_row + byte_idx] << width_idx) & 0x80) == 0x80)
				{
					write_pixel(handle, x, y, color);
				}
			}
		}
	}
	handle->pos_x += font.width + num_byte_per_row;

	return ERR_CODE_SUCCESS;
}

err_code_t ili9341_write_string(ili9341_handle_t handle, font_size_t font_size, uint8_t *str, uint32_t color)
{
	/* Check if handle structure is NULL */
	if (handle == NULL)
	{
		return ERR_CODE_NULL_PTR;
	}

	while (*str) {
		font_t font;
		if (get_font(*str, font_size, &font) <= 0)
		{
			return ERR_CODE_FAIL;
		}

		uint16_t num_byte_per_row = font.data_len / font.height;
		for (uint16_t height_idx = 0; height_idx < font.height; height_idx ++)
		{
			for ( uint16_t byte_idx = 0; byte_idx < num_byte_per_row; byte_idx++)
			{
				for (uint16_t width_idx = 0; width_idx < 8; width_idx++)
				{
					uint16_t x = handle->pos_x + width_idx + byte_idx * 8;
					uint16_t y = handle->pos_y + height_idx;
					if (((font.data[height_idx * num_byte_per_row + byte_idx] << width_idx) & 0x80) == 0x80)
					{
						write_pixel(handle, x, y, color);
					}
				}
			}
		}
		handle->pos_x += font.width + 1;
		str++;
	}

	return ERR_CODE_SUCCESS;
}

err_code_t ili9341_draw_pixel(ili9341_handle_t handle, uint16_t x, uint16_t y, uint32_t color)
{
	/* Check if handle structure is NULL */
	if (handle == NULL)
	{
		return ERR_CODE_NULL_PTR;
	}

	write_pixel(handle, x, y, color);

	return ERR_CODE_SUCCESS;
}

err_code_t ili9341_draw_line(ili9341_handle_t handle, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color)
{
	/* Check if handle structure is NULL */
	if (handle == NULL)
	{
		return ERR_CODE_NULL_PTR;
	}

	write_line(handle, x1, y1, x2, y2, color);

	return ERR_CODE_SUCCESS;
}

err_code_t ili9341_draw_rectangle(ili9341_handle_t handle, uint16_t x_origin, uint16_t y_origin, uint16_t width, uint16_t height, uint32_t color)
{
	/* Check if handle structure is NULL */
	if (handle == NULL)
	{
		return ERR_CODE_NULL_PTR;
	}

	write_line(handle, x_origin, y_origin, x_origin + width, y_origin, color);
	write_line(handle, x_origin + width, y_origin, x_origin + width, y_origin + height, color);
	write_line(handle, x_origin + width, y_origin + height, x_origin, y_origin + height, color);
	write_line(handle, x_origin, y_origin + height, x_origin, y_origin, color);

	return ERR_CODE_SUCCESS;
}

err_code_t ili9341_draw_circle(ili9341_handle_t handle, uint16_t x_origin, uint16_t y_origin, uint16_t radius, uint32_t color)
{
	/* Check if handle structure is NULL */
	if (handle == NULL)
	{
		return ERR_CODE_NULL_PTR;
	}

	int32_t x = -radius;
	int32_t y = 0;
	int32_t err = 2 - 2 * radius;
	int32_t e2;

	do {
		write_pixel(handle, x_origin - x, y_origin + y, color);
		write_pixel(handle, x_origin + x, y_origin + y, color);
		write_pixel(handle, x_origin + x, y_origin - y, color);
		write_pixel(handle, x_origin - x, y_origin - y, color);

		e2 = err;
		if (e2 <= y) {
			y++;
			err = err + (y * 2 + 1);
			if (-x == y && e2 <= x) {
				e2 = 0;
			}
			else {
				/*nothing to do*/
			}
		} else {
			/*nothing to do*/
		}

		if (e2 > x) {
			x++;
			err = err + (x * 2 + 1);
		} else {
			/*nothing to do*/
		}
	} while (x <= 0);

	return ERR_CODE_SUCCESS;
}

err_code_t ili9341_set_position(ili9341_handle_t handle, uint16_t x, uint16_t y)
{
	/* Check if handle structure is NULL */
	if (handle == NULL)
	{
		return ERR_CODE_NULL_PTR;
	}

	handle->pos_x = x;
	handle->pos_y = y;

	return ERR_CODE_SUCCESS;
}

err_code_t ili9341_get_position(ili9341_handle_t handle, uint16_t *x, uint16_t *y)
{
	/* Check if handle structure is NULL */
	if (handle == NULL)
	{
		return ERR_CODE_NULL_PTR;
	}

	*x = handle->pos_x;
	*y = handle->pos_y;

	return ERR_CODE_SUCCESS;
}

err_code_t ili9341_set_bckl_on(ili9341_handle_t handle)
{
	/* Check if handle structure is NULL */
	if (handle == NULL)
	{
		return ERR_CODE_NULL_PTR;
	}

	if (handle->set_bckl != NULL)
	{
		handle->set_bckl(1);
	}

	return ERR_CODE_SUCCESS;
}

err_code_t ili9341_set_bckl_off(ili9341_handle_t handle)
{
	/* Check if handle structure is NULL */
	if (handle == NULL)
	{
		return ERR_CODE_NULL_PTR;
	}

	if (handle->set_bckl != NULL)
	{
		handle->set_bckl(0);
	}

	return ERR_CODE_SUCCESS;
}

uint8_t* ili9341_get_screen_buffer(ili9341_handle_t handle)
{
	/* Check if handle structure is NULL */
	if (handle == NULL)
	{
		return NULL;
	}

	return handle->data;
}

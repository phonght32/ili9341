#include "stdio.h"
#include "stdlib.h"
#include "ili9341.h"

#define ILI3941_RST_ACTIVE_LEVEL 	0
#define ILI3941_RST_UNACTIVE_LEVEL 	1

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

typedef struct ili9341 {
	uint16_t 				height;					/*!< Screen height */
	uint16_t 				width;					/*!< Screen width */
	ili9341_spi_send 		spi_send;				/*!< Function send SPI */
	ili9341_set_gpio        set_dc;             	/*!< Function set pin DC */
	ili9341_set_gpio        set_rst;            	/*!< Function set pin RST */
	ili9341_set_gpio        set_bckl;           	/*!< Function on/off LED */
	ili9341_delay 			delay;					/*!< Function delay */
	uint8_t 				*data;					/*!< Screen buffer */
	uint16_t 				pos_x;					/*!< Current position x */
	uint16_t 				pos_y;					/*!< Current position y */
} ili9341_t;

static err_code_t ili9341_write_cmd(ili9341_handle_t handle, uint8_t cmd)
{
	/* DC level equal to 0 when write SPI command */
	handle->set_dc(0);

	/* Transfer command */
	handle->spi_send(&cmd, 1);

	return ERR_CODE_SUCCESS;
}

static err_code_t ili9341_write_data(ili9341_handle_t handle, uint8_t *data, uint32_t len)
{
	/* DC level equal to 1 when write SPI data */
	handle->set_dc(1);

	/* Transfer data */
	handle->spi_send(data, len);

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

err_code_t ili9341_set_position(ili9341_handle_t handle, uint16_t x, uint16_t y)
{
	/* Check if handle structure is NULL */
	if (handle == NULL)
	{
		return ERR_CODE_NULL_PTR;
	}

	uint8_t cmd;
	uint8_t data[4];

	/* Set column address (X) */
	cmd = 0x2A; 							/*!< Column Address Set */
	ili9341_write_cmd(handle, cmd);

	data[0] = x >> 8;						/*!< X start high byte */
	data[1] = x & 0xFF;						/*!< X start low byte */
	data[2] = (handle->width - 1) >> 8;		/*!< X end high byte */
	data[3] = (handle->width - 1) & 0xFF;	/*!< X end low byte */
	ili9341_write_data(handle, data, 4);

	/* Set page address (Y) */
	cmd = 0x2B;								/*!< Page Address Set */
	ili9341_write_cmd(handle, cmd);

	data[0] = y >> 8; 						/*!< Y start high byte */
	data[1] = y & 0xFF; 					/*!< Y start low byte */
	data[2] = (handle->height - 1) >> 8; 	/*!< Y end high byte */
	data[3] = (handle->height - 1) & 0xFF;	/*!< Y end low byte */
	ili9341_write_data(handle, data, 4);

	/* Write Memory Start */
	cmd = 0x2C;
	ili9341_write_cmd(handle, cmd);

	handle->set_dc(1);

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

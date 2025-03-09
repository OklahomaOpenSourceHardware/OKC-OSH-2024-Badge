#include <Arduino.h>

#include <stdint.h>

#define SSD1306_W 128
#define SSD1306_H 32
#define SSD1306_FULLUSE
#define SSD1306_OFFSET 0
#define SSD1306_HALF
uint8_t ssd1306_pkt_send(uint8_t *data, uint8_t sz, uint8_t cmd);
void ssd1306_rst(void);
#include "ssd1306.h"

#include <memory.h>
#include "utility/twi.h"
#include "i2c_mini.h"
#include "ssd1306_mini.h"

/*
 * high-level packet send for I2C
 */
uint8_t ssd1306_pkt_send(uint8_t *data, uint8_t sz, uint8_t cmd)
{
	uint8_t pkt[33];
	
	/* build command or data packets */
	if(cmd)
	{
		pkt[0] = 0;
		pkt[1] = *data;
	}
	else
	{
		pkt[0] = 0x40;
		memcpy(&pkt[1], data, sz);
	}
	return ssd1306Send(pkt, sz+1);
}


/*
 * reset is not used for SSD1306 I2C interface
 */
void ssd1306_rst(void)
{
}


void ssd1306_refresh_half(int part)
{
	uint16_t i;
	
	ssd1306_cmd(SSD1306_COLUMNADDR);
	ssd1306_cmd(SSD1306_OFFSET);   // Column start address (0 = reset)
	ssd1306_cmd(SSD1306_OFFSET+SSD1306_W-1); // Column end address (127 = reset)
	
	ssd1306_cmd(SSD1306_PAGEADDR);
	ssd1306_cmd(part ? 4 : 0);
	ssd1306_cmd(part ? 7 : 3);

	/* for fully used rows just plow thru everything */
    for(i=0;i<sizeof(ssd1306_buffer);i+=SSD1306_PSZ)
	{
		/* send PSZ block of data */
		ssd1306_data(&ssd1306_buffer[i], SSD1306_PSZ);
	}
}

void oledDrawText(int x, int y, const char* str, int color, int scale) {
	ssd1306_drawstr_sz(x,y, (char*)str, color, (font_size_t)scale);
}

void oledInit() {
	ssd1306_init();
	ssd1306_refresh_half(1);
}

void oledClearBuffer(int color){
	ssd1306_setbuf(0);
}

void oledRefresh() {
	ssd1306_refresh_half(0);
}

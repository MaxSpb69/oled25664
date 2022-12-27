/*
 * drv_SSD1362.h
 *
 *  Created on: 16 авг. 2022 г.
 *      Author: maxsp
 */

#ifndef DRV_SSD1362_H_
#define DRV_SSD1362_H_


void SSD1362_init(void);
void update_gram(void);
void draw_pixel(uint16_t x, uint16_t y, uint8_t color);
void draw_fill_rect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color);



#endif /* DRV_SSD1362_H_ */

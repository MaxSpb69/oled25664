/*
 * drv_SSD1362.c
 *
 *  Created on: 16 авг. 2022 г.
 *      Author: maxsp
 */

#include "main.h"
#include <stdbool.h>
#include <stdlib.h>

static volatile bool is_transfer_flag = false;

extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_spi1_tx;

uint8_t gram[8192];

typedef struct
{
	uint8_t X_Cmd;
	uint8_t X_Start;
	uint8_t X_End;
	uint8_t Y_Cmd;
	uint8_t Y_Start;
	uint8_t Y_End;

}__fill_rect_command_t;





//-------------------------------------------------
//     WRITE_COMMAND
//-------------------------------------------------
void Write_command(uint8_t command)
{

	HAL_GPIO_WritePin(SPI_CS_GPIO_Port,SPI_CS_Pin, GPIO_PIN_RESET);		// CS = 0
	HAL_GPIO_WritePin(SPI_DC_GPIO_Port,SPI_DC_Pin, GPIO_PIN_RESET);		// DC = 0

	HAL_SPI_Transmit(&hspi1, &command, 1, 100);

//	HAL_GPIO_WritePin(SPI_DC_GPIO_Port,SPI_DC_Pin, GPIO_PIN_SET);		// DC = 1
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port,SPI_CS_Pin, GPIO_PIN_SET);		// CS = 1

}



//==================================
//      initial SSD1362
//==================================
void SSD1362_init(void)
{
	HAL_GPIO_WritePin(RESET_GPIO_Port,RESET_Pin, GPIO_PIN_RESET);
	HAL_Delay(200);
	HAL_GPIO_WritePin(RESET_GPIO_Port,RESET_Pin, GPIO_PIN_SET);

	Write_command(0XFD); //Set Command Lock
	Write_command(0X12); //(12H=Unlock,16H=Lock)

	Write_command(0XAE); //Display OFF(Sleep Mode)

	Write_command(0X15); //Set column Address
	Write_command(0X00); //Start column Address
	Write_command(0X7F); //End column Address

	Write_command(0X75); //Set Row Address
	Write_command(0X00); //Start Row Address
	Write_command(0X3F); //End Row Address

	Write_command(0X81); //Set contrast
	Write_command(0x2f);

	Write_command(0XA0); //Set Remap
	Write_command(0Xc3);

	Write_command(0XA1); //Set Display Start Line
	Write_command(0X00);

	Write_command(0XA2); //Set Display Offset
	Write_command(0X00);

	Write_command(0XA4); //Normal Display

	Write_command(0XA8); //Set Multiplex Ratio
	Write_command(0X3F);

	Write_command(0XAB); //Set VDD regulator
	Write_command(0X01); //Regulator Enable

	Write_command(0XAD); //External /Internal IREF Selection
	Write_command(0X8E);

	Write_command(0XB1); //Set Phase Length
	Write_command(0X22);

	Write_command(0XB3); //Display clock Divider
	Write_command(0XA0);

	Write_command(0XB6); //Set Second precharge Period
	Write_command(0X04);

	Write_command(0XB9); //Set Linear LUT

	Write_command(0XBc); //Set pre-charge voltage level
	Write_command(0X10); //0.5*Vcc

	Write_command(0XBD); //Pre-charge voltage capacitor Selection
	Write_command(0X01);

	Write_command(0XBE); //Set cOM deselect voltage level
	Write_command(0X07); //0.82*Vcc

	Write_command(0XAF); //Display ON
}


void update_gram(void)
{
	__fill_rect_command_t Rect_Command;

	Rect_Command.X_Cmd = 0x15;
	Rect_Command.X_Start = 0;
	Rect_Command.X_End = 127;
	Rect_Command.Y_Cmd = 0x75;
	Rect_Command.Y_Start = 0;
	Rect_Command.Y_End = 63;

	is_transfer_flag = true;

	HAL_GPIO_WritePin(SPI_DC_GPIO_Port,SPI_DC_Pin, GPIO_PIN_RESET);		// DC = 0
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port,SPI_CS_Pin, GPIO_PIN_RESET);		// CS = 0

	HAL_SPI_Transmit_DMA(&hspi1, (uint8_t *)&Rect_Command, sizeof(Rect_Command));

	while(is_transfer_flag)
	  continue;

	is_transfer_flag = true;

	HAL_GPIO_WritePin(SPI_DC_GPIO_Port,SPI_DC_Pin, GPIO_PIN_SET);		// DC = 1
    HAL_GPIO_WritePin(SPI_CS_GPIO_Port,SPI_CS_Pin, GPIO_PIN_RESET);		// CS = 0

	HAL_SPI_Transmit_DMA(&hspi1, gram, sizeof(gram));

//	while(is_transfer_flag)
//	  continue;

//	HAL_GPIO_WritePin(SPI_CS_GPIO_Port,SPI_CS_Pin, GPIO_PIN_SET);		// CS = 1

}



void draw_pixel(uint16_t x, uint16_t y, uint8_t color)
{
	uint8_t * buffer_address;

	buffer_address = (gram + ((x / 2)  + (y * 128)));
	if (x & 0x01)
		*buffer_address = (*buffer_address & 0x0F) | (color << 4);
	else
		*buffer_address = (*buffer_address & 0xF0) | (color & 0x0F);
}


void draw_fill_rect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color)
{
	uint16_t x, y;

	for(y = y1; y <= y2; y++)
		for(x = x1; x <= x2; x++)
			draw_pixel(x, y, color);
}





void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{

	if (hspi == &hspi1)
	{
		is_transfer_flag = false;
		HAL_GPIO_WritePin(SPI_CS_GPIO_Port,SPI_CS_Pin, GPIO_PIN_SET);		// CS = 1
	}

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SPI_TxCpltCallback should be implemented in the user file
   */
}


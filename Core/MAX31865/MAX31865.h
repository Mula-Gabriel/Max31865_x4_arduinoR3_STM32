#pragma once

#include <stdbool.h>

#include "stm32f4xx_hal.h"

#include "MAX31865_Register.h"

#define R_Alpha_IEC751 	(0.00385)
#define R_Alpha_SAMA 	(0.00392)




typedef struct MAX31865_Init_s
{
	SPI_HandleTypeDef * Handler_Spi;

	GPIO_TypeDef * CS_GPIOPort;
	uint16_t	   CS_GPIO_PIn;

	float R_Ref;
	float R_Nominal;
	float R_Alpha;



}MAX31865_Init_ts;


bool MAX31865_Create(MAX31865_Init_ts * MAX31865_Init);

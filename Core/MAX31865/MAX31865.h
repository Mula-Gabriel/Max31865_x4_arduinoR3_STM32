#pragma once

#include <stdbool.h>

#include "stm32f4xx_hal.h"

#include "MAX31865_Register.h"


//https://rdccontrol.com/fr/thermocouple/rtd-101/calcul-temperature/
#define A_IEC751 	(3.9083E-3)
#define B_IEC751 	(-5.775E-7)
#define C_IEC751 	(-4.18301E-12)

#define A_US_IND 	(3.9692E-3)
#define B_US_IND 	(-5,8495E-7)
#define C_US_IND 	(-4,2325E-12)

#define A_US 		(3.9848E-3)
#define B_US 		(-5,870E-7)
#define C_US 		(-4.000E-12)

#define R0_PT100 	(100.0)
#define R0_PT1000 	(1000.0)


typedef struct MAX31865_handler_s * MAX31865_handler;

typedef struct PT100x_Parameters_s
{
	double R0;
	double A;
	double B;
	//double C;		//TODO negative Temperature

}PT100x_Parameters_ts;

typedef struct MAX31865_Init_s
{
	SPI_HandleTypeDef * Handler_Spi;

	GPIO_TypeDef * CS_GPIOPort;
	uint16_t	   CS_GPIO_PIn;

	PT100x_Parameters_ts PT100x_Parameters;
	double	R_Ref;

}MAX31865_Init_ts;


MAX31865_handler MAX31865_Create(const MAX31865_Init_ts * MAX31865_conf);

bool MAX31865_Init(MAX31865_handler Max31865_handler,const Configuration_Register_ts *Configuration_Register);

//took
double MAX31865_GetTemperatureSingleShot(MAX31865_handler Max31865_handler);




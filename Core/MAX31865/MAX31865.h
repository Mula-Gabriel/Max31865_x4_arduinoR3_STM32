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
	float R0;
	float A;
	float B;
	//float C;		//TODO negative Temperature

}PT100x_Parameters_ts;

typedef struct MAX31865_Init_s
{
	SPI_HandleTypeDef * Handler_Spi;

	PT100x_Parameters_ts PT100x_Parameters;
	float	R_Ref;

	//set to nan to disable
	float HighTempthreshold;
	float LowTempthreshold;

}MAX31865_Init_ts;


MAX31865_handler MAX31865_Create(const MAX31865_Init_ts * MAX31865_conf, GPIO_TypeDef * CS_GPIOPort, uint16_t CS_GPIO_PIn);

void MAX31865_Init(MAX31865_handler Max31865_handler,const Configuration_Register_ts *Configuration_Register);
void MAX31865_Disable(MAX31865_handler Max31865_handler);
//took
float MAX31865_ReadTemperatureSingleShot(MAX31865_handler Max31865_handler,bool *FaultFlag);
float MAX31865_ReadTemperatureAuto(MAX31865_handler Max31865_handler,bool *FaultFlag);
void MAX31865_ClearFault(MAX31865_handler Max31865_handler);
void MAX31865_AutomaticConversionMode(MAX31865_handler Max31865_handler, bool Enable);
Fault_Status_Register_ts MAX31865_DoFaultDetectionCycle(MAX31865_handler Max31865_handler);
Fault_Status_Register_ts MAX31865_GetFaultRegister(MAX31865_handler Max31865_handler);


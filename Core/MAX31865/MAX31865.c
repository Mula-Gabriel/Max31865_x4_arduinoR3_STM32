//
#include "MAX31865.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>


typedef struct MAX31865_handler_s
{
	SPI_HandleTypeDef * Handler_Spi;

	Max31865_registers_ts Max31865_registers;

	GPIO_TypeDef * CS_GPIOPort;
	uint16_t	   CS_GPIO_PIn;

	PT100x_Parameters_ts PT100x_Parameters;

	double MesureResistance;
	double Temperature;
	bool FaultFlag;
	double R_Ref;
	uint16_t ADC_Value;
}MAX31865_handler_ts;


static double ConvertADCToDegreeCelcius(const PT100x_Parameters_ts * p, double RRef, uint16_t ADCValue );
static void _ReadRegisters(MAX31865_handler h, uint8_t RegAddr, uint8_t num);
static void _WriteRegisters(MAX31865_handler h, uint8_t RegAddr, uint8_t num);

MAX31865_handler MAX31865_Create(const MAX31865_Init_ts * MAX31865_conf)
{
	MAX31865_handler handler = NULL;
	if(MAX31865_conf != NULL && MAX31865_conf->Handler_Spi != NULL && MAX31865_conf->CS_GPIOPort != NULL)
	{
		handler = malloc( sizeof(MAX31865_handler_ts) );

		if(handler != NULL )
		{
			memset(handler,0,sizeof(MAX31865_handler_ts));
			handler->Handler_Spi = MAX31865_conf->Handler_Spi;
			handler->CS_GPIOPort = MAX31865_conf->CS_GPIOPort;
			handler->CS_GPIO_PIn = MAX31865_conf->CS_GPIO_PIn;
			handler->PT100x_Parameters = MAX31865_conf->PT100x_Parameters;
			handler->R_Ref = MAX31865_conf->R_Ref;
		}
	}


	return handler;
}
//
//Callendar-Van Dusen (CVD)
 double ConvertADCToDegreeCelcius(const PT100x_Parameters_ts * p, double RRef, uint16_t ADCValue )
{
	//RRTD = (ADC Code x RREF)/2E15
	double RRTD = ( ADCValue * RRef ) / ((double)0x7FFF);

	if( RRTD >= p->R0 )
	{
		return (  -p->A + sqrt( pow(p->A,2) - 4*p->B*( 1 - (RRTD / p->R0)))) / (2 * p->B);
	}
	else	//for negative, just use aproximation from max31865 doc
	{
		return (ADCValue / 32) - 256;
	}
}

 bool MAX31865_Init(MAX31865_handler h,const Configuration_Register_ts *Configuration_Register)
{
	HAL_GPIO_WritePin(h->CS_GPIOPort, h->CS_GPIO_PIn, GPIO_PIN_RESET);

	h->Max31865_registers.Configuration = *Configuration_Register;
	_WriteRegisters(h,MAX31865_CONFIG_REG,1);

	_ReadRegisters(h,MAX31865_CONFIG_REG, MAX31865_REG_COUNG);

	return false;
}
//

double MAX31865_GetTemperatureSingleShot(MAX31865_handler h)
{
	_ReadRegisters(h,MAX31865_CONFIG_REG, MAX31865_REG_COUNG);
	h->Max31865_registers.Configuration.OneShot = 1;
	_WriteRegisters(h,MAX31865_CONFIG_REG,1);

	uint32_t Delayms = h->Max31865_registers.Configuration.Filter_Select == 0 ? 55 : 66;
	HAL_Delay(Delayms);

	_ReadRegisters(h, MAX31865_RTD_MSB, 2);
	h->ADC_Value = h->Max31865_registers.RTD_MSB << 7 | h->Max31865_registers.RTD_LSB.LSB;
	h->FaultFlag = h->Max31865_registers.RTD_LSB.Fault;
	h->Temperature = ConvertADCToDegreeCelcius(&h->PT100x_Parameters,h->R_Ref,h->ADC_Value);
	return h->Temperature;
}



 void _ReadRegisters(MAX31865_handler h, uint8_t RegAddr, uint8_t num)
{
	if(RegAddr + num > MAX31865_REG_COUNG ) { num = MAX31865_REG_COUNG - RegAddr; }

	RegAddr |= MAX31865_READ_MASK;
	uint8_t *Rptr = &h->Max31865_registers.asU8Array[RegAddr];
	HAL_GPIO_WritePin(h->CS_GPIOPort, h->CS_GPIO_PIn, GPIO_PIN_RESET);
	HAL_SPI_Transmit(h->Handler_Spi,&(RegAddr ), 1, 100);
	HAL_SPI_Receive(h->Handler_Spi, Rptr, num, 100);
	HAL_GPIO_WritePin(h->CS_GPIOPort, h->CS_GPIO_PIn, GPIO_PIN_SET);
}

 void _WriteRegisters(MAX31865_handler h, uint8_t RegAddr, uint8_t num)
{
	if(RegAddr + num > MAX31865_REG_COUNG ) { num = MAX31865_REG_COUNG - RegAddr; }

	uint8_t *Wptr = &h->Max31865_registers.asU8Array[RegAddr];
	RegAddr |= MAX31865_WRITE_MASK;

	HAL_GPIO_WritePin(h->CS_GPIOPort, h->CS_GPIO_PIn, GPIO_PIN_RESET);
	HAL_SPI_Transmit(h->Handler_Spi,&RegAddr , 1, 100);
	HAL_SPI_Transmit(h->Handler_Spi,Wptr , num , 100);
	HAL_GPIO_WritePin(h->CS_GPIOPort, h->CS_GPIO_PIn, GPIO_PIN_SET);
}
//

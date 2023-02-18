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

	float MesureResistance;
	float R_Ref;
	uint16_t ADC_Value;

	float HighTempthreshold;
	float LowTempthreshold;
}MAX31865_handler_ts;


static float ConvertADCToDegreeCelcius(const PT100x_Parameters_ts * p, float RRef, uint16_t ADCValue );
static void _ReadRegisters(MAX31865_handler h, uint8_t RegAddr, uint8_t num);
static void _WriteRegisters(MAX31865_handler h, uint8_t RegAddr, uint8_t num);

MAX31865_handler MAX31865_Create(const MAX31865_Init_ts * MAX31865_conf ,GPIO_TypeDef * CS_GPIOPort ,uint16_t CS_GPIO_PIn)
{
	MAX31865_handler handler = NULL;
	if(MAX31865_conf != NULL && MAX31865_conf->Handler_Spi != NULL)
	{
		handler = malloc( sizeof(MAX31865_handler_ts) );

		if(handler != NULL )
		{
			memset(handler,0,sizeof(MAX31865_handler_ts));
			handler->Handler_Spi = MAX31865_conf->Handler_Spi;
			handler->CS_GPIOPort = CS_GPIOPort;
			handler->CS_GPIO_PIn = CS_GPIO_PIn;
			handler->PT100x_Parameters = MAX31865_conf->PT100x_Parameters;
			handler->R_Ref = MAX31865_conf->R_Ref;

			handler->HighTempthreshold = MAX31865_conf->HighTempthreshold;
			handler->LowTempthreshold = MAX31865_conf->LowTempthreshold;

		}
	}


	return handler;
}
//
//Callendar-Van Dusen (CVD)
 float ConvertADCToDegreeCelcius(const PT100x_Parameters_ts * p, float RRef, uint16_t ADCValue )
{
	//RRTD = (ADC Code x RREF)/2E15
	float RRTD = ( ADCValue * RRef ) / ((float)0x7FFF);

	if( RRTD >= p->R0 )
	{
		return (  -p->A + sqrt( pow(p->A,2) - 4*p->B*( 1 - (RRTD / p->R0)))) / (2 * p->B);
	}
	else	//for negative, just use aproximation from max31865 doc
	{
		return (ADCValue / 32) - 256;
	}
}

 int16_t ConvertDegreeCelciusToAdc( float Degree )
{
	 return ( ((int16_t)(Degree + 256)*32)) & 0x7FFF;	//15 bits maximum
}

 void MAX31865_Init(MAX31865_handler h,const Configuration_Register_ts *Configuration_Register)
{
	HAL_GPIO_WritePin(h->CS_GPIOPort, h->CS_GPIO_PIn, GPIO_PIN_RESET);
	h->Max31865_registers.Configuration = *Configuration_Register;

	if(isnan(h->LowTempthreshold) == false)
	{
		int16_t ADCCode = ConvertDegreeCelciusToAdc(h->LowTempthreshold);
		h->Max31865_registers.LFT_LSB_Register.LSB = ADCCode & 0x7F;
	    h->Max31865_registers.LFT_MSB_Register = ADCCode >> 7;
	}

	if(isnan(h->HighTempthreshold) == false)
	{
		int16_t ADCCode = ConvertDegreeCelciusToAdc(h->HighTempthreshold);
		h->Max31865_registers.HFT_LSB_Register.LSB = ADCCode & 0x7F;
	    h->Max31865_registers.HFT_MSB_Register = ADCCode >> 7;
	}

	_WriteRegisters(h,MAX31865_CONFIG_REG,1);
	//Write both threshold register
	_WriteRegisters(h,MAX31865_HIGH_FAULT_THRESHOLD_MSB,4);
	_ReadRegisters(h,MAX31865_CONFIG_REG, MAX31865_REG_COUNT);

}

void MAX31865_Disable(MAX31865_handler h)
{
	h->Max31865_registers.Configuration.Vbias = 0;
	h->Max31865_registers.Configuration.ConversionMode = 0;
	h->Max31865_registers.Configuration.OneShot = 0;
	_WriteRegisters(h,MAX31865_CONFIG_REG,1);
}

float MAX31865_ReadTemperatureSingleShot(MAX31865_handler h,bool *FaultFlag)
{
	_ReadRegisters(h,MAX31865_CONFIG_REG, MAX31865_REG_COUNT);
	h->Max31865_registers.Configuration.Vbias = 1;
	h->Max31865_registers.Configuration.ConversionMode = 0;
	h->Max31865_registers.Configuration.OneShot = 1;
	_WriteRegisters(h,MAX31865_CONFIG_REG,1);

	uint32_t Delayms = h->Max31865_registers.Configuration.Filter_Select == 0 ? 55 : 66;
	HAL_Delay(Delayms);

	_ReadRegisters(h, MAX31865_RTD_MSB, 2);
	h->ADC_Value = h->Max31865_registers.RTD_MSB << 7 | h->Max31865_registers.RTD_LSB.LSB;

	if(FaultFlag != NULL)	*FaultFlag =  h->Max31865_registers.RTD_LSB.Fault;

	return ConvertADCToDegreeCelcius(&h->PT100x_Parameters,h->R_Ref,h->ADC_Value) ;;
}

float MAX31865_ReadTemperatureAuto(MAX31865_handler h,bool *FaultFlag)
{
	_ReadRegisters(h, MAX31865_RTD_MSB, 2);
	h->ADC_Value = h->Max31865_registers.RTD_MSB << 7 | h->Max31865_registers.RTD_LSB.LSB;


	if(FaultFlag != NULL)	*FaultFlag =  h->Max31865_registers.RTD_LSB.Fault;
	return ConvertADCToDegreeCelcius(&h->PT100x_Parameters,h->R_Ref,h->ADC_Value) ;;
}

void MAX31865_AutomaticConversionMode(MAX31865_handler h, bool Enable)
{
	_ReadRegisters(h,MAX31865_CONFIG_REG, MAX31865_REG_COUNT);
	h->Max31865_registers.Configuration.Vbias = 1;
	h->Max31865_registers.Configuration.ConversionMode = Enable;
	_WriteRegisters(h,MAX31865_CONFIG_REG,1);
}


void MAX31865_ClearFault(MAX31865_handler h)
{
	h->Max31865_registers.Configuration.FaultStatusCLear = 1;
	_WriteRegisters(h,MAX31865_CONFIG_REG,1);
}

Fault_Status_Register_ts MAX31865_GetFaultRegister(MAX31865_handler h)
{
	_ReadRegisters(h,MAX31865_FAULT_STATUS, 1);
	return h->Max31865_registers.Fault_Status_Register;
}

Fault_Status_Register_ts MAX31865_DoFaultDetectionCycle(MAX31865_handler h)
{
	h->Max31865_registers.Configuration.Fault_Detection_Cycle_Control = Fault_Detection_Ctl_Auto;
	_WriteRegisters(h,MAX31865_CONFIG_REG,1);

	do
	{
		HAL_Delay(1);
		_ReadRegisters(h,MAX31865_CONFIG_REG , 1);
	}while(h->Max31865_registers.Configuration.Fault_Detection_Cycle_Control == Fault_Detection_Status_AutoFaultRunning);

	return MAX31865_GetFaultRegister(h);
}


 void _ReadRegisters(MAX31865_handler h, uint8_t RegAddr, uint8_t num)
{
	if(RegAddr + num > MAX31865_REG_COUNT ) { num = MAX31865_REG_COUNT - RegAddr; }

	RegAddr |= MAX31865_READ_MASK;
	uint8_t *Rptr = &h->Max31865_registers.asU8Array[RegAddr];
	HAL_GPIO_WritePin(h->CS_GPIOPort, h->CS_GPIO_PIn, GPIO_PIN_RESET);
	HAL_SPI_Transmit(h->Handler_Spi,&(RegAddr ), 1, 100);
	HAL_SPI_Receive(h->Handler_Spi, Rptr, num, 100);
	HAL_GPIO_WritePin(h->CS_GPIOPort, h->CS_GPIO_PIn, GPIO_PIN_SET);
}

 void _WriteRegisters(MAX31865_handler h, uint8_t RegAddr, uint8_t num)
{
	if(RegAddr + num > MAX31865_REG_COUNT ) { num = MAX31865_REG_COUNT - RegAddr; }

	uint8_t *Wptr = &h->Max31865_registers.asU8Array[RegAddr];
	RegAddr |= MAX31865_WRITE_MASK;

	HAL_GPIO_WritePin(h->CS_GPIOPort, h->CS_GPIO_PIn, GPIO_PIN_RESET);
	HAL_SPI_Transmit(h->Handler_Spi,&RegAddr , 1, 100);
	HAL_SPI_Transmit(h->Handler_Spi,Wptr , num , 100);
	HAL_GPIO_WritePin(h->CS_GPIOPort, h->CS_GPIO_PIn, GPIO_PIN_SET);
}
//

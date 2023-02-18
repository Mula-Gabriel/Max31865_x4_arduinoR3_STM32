#pragma once

#include <stdint.h>


//The MAX31865 supports SPI modes 1 and 3



//Or mask apply on cmd byte
#define MAX31865_WRITE_MASK					0x80
#define MAX31865_READ_MASK					0x00

//RW Register
#define MAX31865_CONFIG_REG					0x00
#define MAX31865_HIGH_FAULT_THRESHOLD_MSB	0x03
#define MAX31865_HIGH_FAULT_THRESHOLD_LSB	0x04
#define MAX31865_LOW_FAULT_THRESHOLD_MSB		0x05
#define MAX31865_LOW_FAULT_THRESHOLD_LSB		0x06

//RO Register
#define MAX31865_RTD_MSB						0x01
#define MAX31865_RTD_LSB						0x02
#define MAX31865_FAULT_STATUS				0x07

typedef enum Fault_Detection_Cycle_Control_e
{
	Fault_Detection_Ctl_NoAction = 0,
	Fault_Detection_Status_Finished = 0,
	Fault_Detection_Ctl_Auto = 1,
	Fault_Detection_Status_AutoFaultRunning = 1,
	Fault_Detection_Ctl_RunManualFault_Cycle_1 = 2,
	Fault_Detection_Status_ManualCycle1Running = 2,
	Fault_Detection_Ctl_RunManualFault_Cycle_2 = 3,
	Fault_Detection_Status_ManualCycle2Running = 3,

}Fault_Detection_Cycle_Control_te;

typedef struct __attribute__((packed)) Configuration_Register_s
{
	union
	{
		uint8_t asU8;
		struct
		{

			//50/60Hz filter select
			//	1 = 50Hz
			//	0 = 60Hz
			uint8_t Filter_Select : 1;
			//Fault Status Clear
			//	1 = Clear (auto-clear)
			uint8_t FaultStatusCLear : 1;
			//Fault Detection Cycle Control
			Fault_Detection_Cycle_Control_te Fault_Detection_Cycle_Control : 2;		//When write => Control, When Read => Status
			//3-wire
			//	1 = 3-wire RTD
			//	0 = 2-wire or 4-wire
			uint8_t RTD_3Wire : 1;
			//1-shot
			//	1 = 1-shot (auto-clear)
			uint8_t OneShot : 1;
			//Conversion mode
			//	1 = Auto
			//	0 = Normally off
			uint8_t ConversionMode : 1;
			//VBIAS
			//	1 = ON
			//	0 = OFF
			uint8_t Vbias: 1;

		};
	};

}Configuration_Register_ts;

typedef struct __attribute__((packed))RTD_LSB_Register_s
{
	union
	{
		uint8_t asU8;
		struct
		{
			uint8_t Fault : 1;
			uint8_t LSB: 7;
		};
	};

}RTD_LSB_Register_ts;


typedef struct __attribute__((packed)) xFT_LSB_Register_s
{
	union
	{
		uint8_t asU8;
		struct
		{
			uint8_t DontCare : 1;
			uint8_t LSB: 7;
		};
	};

}HFT_LSB_Register_ts,LFT_LSB_Register_ts;

typedef struct __attribute__((packed)) Fault_Status_Register_s
{
	union
	{
		uint8_t asU8;
		struct
		{
			uint8_t DontCare : 2;
			uint8_t VoltageFault: 1;
			uint8_t RTDinInfVbias: 1;
			uint8_t REFINInf085Vbias: 1;
			uint8_t REFINSup085Vbias: 1;
			uint8_t RTD_LowThreshold: 1;
			uint8_t RTD_HighThreshold: 1;

		};
	};

}Fault_Status_Register_ts;

#define MAX31865_REG_COUNT 8

typedef struct __attribute__((packed)) Max31865_registers_s
{
	union
	{
		uint8_t asU8Array[MAX31865_REG_COUNT];
		struct
		{
			Configuration_Register_ts Configuration;

			uint8_t 			RTD_MSB;
			RTD_LSB_Register_ts RTD_LSB;

			uint8_t 			HFT_MSB_Register;
			HFT_LSB_Register_ts HFT_LSB_Register;

			uint8_t 			LFT_MSB_Register;
			LFT_LSB_Register_ts LFT_LSB_Register;

			Fault_Status_Register_ts Fault_Status_Register;
		};
	};

}Max31865_registers_ts;


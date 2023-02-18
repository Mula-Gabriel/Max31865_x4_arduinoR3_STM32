#pragma once

#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "MAX31865.h"

#define Shield_Channel_Count 4


typedef struct Shield_4_MAX31865_s * Shield_4_MAX31865_Handler;



Shield_4_MAX31865_Handler Shield_4_MAX31865_Create(MAX31865_handler *MAX31865_handlerArray ,Configuration_Register_ts *Configuration_Register);

//Channel de 1 a 4
void Shield_4_MAX31865_DisableChannel(Shield_4_MAX31865_Handler h ,uint8_t Channel);
void Shield_4_MAX31865_EnableChannel(Shield_4_MAX31865_Handler h ,uint8_t Channel);
void Shield_4_MAX31865_ClearError(Shield_4_MAX31865_Handler h ,uint8_t Channel);
void Shield_4_MAX31865_Init(Shield_4_MAX31865_Handler h,void (*Fptr_Callback)(Shield_4_MAX31865_Handler h,uint8_t Channel,float Temp,bool fault, Fault_Status_Register_ts Fault_Status_Register) );
void Shield_4_MAX31865_SetFilterCoef(Shield_4_MAX31865_Handler h ,uint8_t Channel,float coef);
void Shield_4_MAX31865_Update(Shield_4_MAX31865_Handler h);


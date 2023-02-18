//
#include "Shield_4_MAX31865.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

typedef struct Shield_4_MAX31865_s
{
	bool Enable[Shield_Channel_Count];
	bool FaultCondition[Shield_Channel_Count];
	float Temperature[Shield_Channel_Count];
	float fTemperature[Shield_Channel_Count];
	float Filter_coef[Shield_Channel_Count];
	MAX31865_handler MAX31865_handlerArray[Shield_Channel_Count];
	Configuration_Register_ts Configuration_Register[Shield_Channel_Count];
	void (*Fptr_Callback)(Shield_4_MAX31865_Handler h ,uint8_t Channel,float Temp,bool fault, Fault_Status_Register_ts Fault_Status_Register);
}Shield_4_MAX31865_ts;


Shield_4_MAX31865_Handler Shield_4_MAX31865_Create(MAX31865_handler *MAX31865_handlerArray ,Configuration_Register_ts *Configuration_Register)
{
	Shield_4_MAX31865_ts * handler = malloc( sizeof(Shield_4_MAX31865_ts) );
	memset(handler , 0 , sizeof(Shield_4_MAX31865_ts));
	memcpy(handler->MAX31865_handlerArray,MAX31865_handlerArray,sizeof(MAX31865_handler) * Shield_Channel_Count);
	memcpy(handler->Configuration_Register,Configuration_Register,sizeof(Configuration_Register_ts) * Shield_Channel_Count);

	return handler;
}

void Shield_4_MAX31865_DisableChannel(Shield_4_MAX31865_Handler h ,uint8_t Channel)
{
	Channel--;
	if(Channel < Shield_Channel_Count)
	{
		MAX31865_Disable(h->MAX31865_handlerArray[Channel]);
		h->Enable[Channel] = false;
	}
}

void Shield_4_MAX31865_SetFilterCoef(Shield_4_MAX31865_Handler h ,uint8_t Channel,float coef)
{
	Channel--;
	if(Channel < Shield_Channel_Count && coef >= 0.0 && coef <= 1.0)
	{
		h->Filter_coef[Channel] = coef;
	}
}

void Shield_4_MAX31865_EnableChannel(Shield_4_MAX31865_Handler h ,uint8_t Channel)
{
	Channel--;
	if(Channel < Shield_Channel_Count)
	{
		MAX31865_Init(h->MAX31865_handlerArray[Channel],&h->Configuration_Register[Channel]);
		MAX31865_AutomaticConversionMode(h->MAX31865_handlerArray[Channel],true);
		h->Filter_coef[Channel] = 1;
		h->fTemperature[Channel] = 0;
		h->Enable[Channel] = true;
	}
}

void Shield_4_MAX31865_ClearError(Shield_4_MAX31865_Handler h ,uint8_t Channel)
{
	Channel--;
	if(Channel < Shield_Channel_Count)
	{
		MAX31865_ClearFault(h->MAX31865_handlerArray[Channel]);
	}
}

void Shield_4_MAX31865_Init(Shield_4_MAX31865_Handler h,void (*Fptr_Callback)(Shield_4_MAX31865_Handler h ,uint8_t Channel,float Temp,bool fault, Fault_Status_Register_ts Fault_Status_Register) )
{
	h->Fptr_Callback = Fptr_Callback;
	for(uint8_t i = 1; i <= Shield_Channel_Count; i++)
	{
		  Shield_4_MAX31865_EnableChannel(h,i);
	}
}

void Shield_4_MAX31865_Update(Shield_4_MAX31865_Handler h)
{
	for(uint8_t i = 0; i < Shield_Channel_Count; i++)
	{
		if(h->Enable[i])
		{
		  Fault_Status_Register_ts Fsr = {.asU8 = 0};
		  h->Temperature[i] = MAX31865_ReadTemperatureAuto(h->MAX31865_handlerArray[i],&h->FaultCondition[i]);
		  h->fTemperature[i] = h->Temperature[i] * h->Filter_coef[i] + (1-h->Filter_coef[i]) * h->fTemperature[i];
		  if(h->FaultCondition[i] == true)	{	Fsr = MAX31865_DoFaultDetectionCycle(h->MAX31865_handlerArray[i]); }

		  if(h->Fptr_Callback != NULL) h->Fptr_Callback(h,i + 1, h->fTemperature[i] ,h->FaultCondition[i],Fsr);

		}
	}
}

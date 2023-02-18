/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "main.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "MAX31865.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */




  MAX31865_Init_ts MAX31865_Init_struct =
  {
	.Handler_Spi = &hspi1,
	.CS_GPIOPort = CS1_GPIO_Port,
	.CS_GPIO_PIn = CS1_Pin,
	.PT100x_Parameters = {	.R0 = 100.0, .A= A_IEC751 , .B = B_IEC751} ,
	.R_Ref = 400.0,
	.HighTempthreshold = 30,
	.LowTempthreshold = -10,
  };

  MAX31865_handler MAX31865[4];

  MAX31865[0] = MAX31865_Create( &MAX31865_Init_struct );

  MAX31865_Init_struct.CS_GPIOPort = CS2_GPIO_Port;
  MAX31865_Init_struct.CS_GPIO_PIn = CS2_Pin;
  MAX31865[1]= MAX31865_Create( &MAX31865_Init_struct );

  MAX31865_Init_struct.CS_GPIOPort = CS3_GPIO_Port;
  MAX31865_Init_struct.CS_GPIO_PIn = CS3_Pin;
  MAX31865[2] = MAX31865_Create( &MAX31865_Init_struct );

  MAX31865_Init_struct.CS_GPIOPort = CS4_GPIO_Port;
  MAX31865_Init_struct.CS_GPIO_PIn = CS4_Pin;
  MAX31865[3] = MAX31865_Create( &MAX31865_Init_struct );

  Configuration_Register_ts MaxConf =
  {
	  .RTD_3Wire = 1,
	  .Filter_Select = 1,
	  .FaultStatusCLear = 0,
	  .Fault_Detection_Cycle_Control = Fault_Detection_Ctl_NoAction,
	  .OneShot = 0,
	  .Vbias = 1,
  };


  for(uint8_t i = 0; i < 4; i++)
  {
	  MAX31865_Init(MAX31865[i],&MaxConf);
	  MAX31865_AutomaticConversionMode(MAX31865[i],true);
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  float t[4] = {0};
	  bool Fault[4] = {false};

	  for(uint8_t i = 0; i < 4; i++)
	  {
		  t[i] = MAX31865_GetTemperatureAuto(MAX31865[i],&Fault[i]);
		  printf("Ch%d=%.1f (Fault=%d) ",i+1,t[i],Fault[i]);

		  if(Fault[i] == true)
		  {
			  Fault_Status_Register_ts Fsr = MAX31865_DoFaultDetectionCycle(MAX31865[i]);

			  printf("VoltageFault=%d RTDinInfVbias=%d REFINInf085Vbias=%d REFINSup085Vbia=%d RTD_LowThreshold=%d RTD_HighThreshold=%d",
					  Fsr.VoltageFault,Fsr.RTDinInfVbias,Fsr.REFINInf085Vbias,Fsr.REFINSup085Vbias,Fsr.RTD_LowThreshold,Fsr.RTD_HighThreshold);

			  //Press button to clear error
			  if(HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET )
			  {
				  MAX31865_ClearFault(MAX31865[i]);
			  }
		  }

		  printf("\n");
	  }

	  HAL_Delay(100);


	  printf("\r\n");

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */





 int __io_putchar(int ch)
{
	 HAL_UART_Transmit(&huart2,(const uint8_t*) &ch,1,100);
	 return 1;
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

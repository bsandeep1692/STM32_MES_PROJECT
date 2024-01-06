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
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "console.h"  //for the cmd line interface
#include <math.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define PI 3.14159
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

DAC_HandleTypeDef hdac;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim9;
TIM_HandleTypeDef htim11;
TIM_HandleTypeDef htim13;

UART_HandleTypeDef huart3;

PCD_HandleTypeDef hpcd_USB_OTG_FS;

/* USER CODE BEGIN PV */
uint8_t BlinkSpeed = 0;
uint8_t msg[20];
uint8_t debounceRequest =0;
uint8_t debounceCount = 0;
uint8_t rx_buffer[10];
uint16_t Value_DAC =0;
uint16_t Value_DAC_SPI =0;
uint32_t phase = 0;
uint16_t spi_data;
uint32_t lut[1024];
uint32_t Value_ARR =1999;
Waveform_T Wave = 0;
Sates_T Current_state=0;
uint8_t Input_Updated = 0;
int16_t Frequency = 1000;
int16_t Amplitude = 4;
Events_T event= 0;
uint32_t Delta_Phase =1;
uint32_t fs= 100000; //Sampling frequency

uint16_t lut1[256]={2048,2098,2149,2199,2249,2300,2350,2399,2449,2498,2547,2596,2644,2693,2740,2787,2834,2881,2926,2972,3016,3061,3104,3147,3189,3231,3272,3312,3351,3389,3427,3464,3500,3535,3569,
		3603,3635,3666,3697,3726,3754,3782,3808,3833,3857,3880,3902,3923,3943,3961,3979,3995,4010,4024,4036,4048,4058,4067,4074,4081,4086,4090,4093,4095,4095,4094,4092,4088,4084,4078,4071,4062,4053,
		4042,4030,4017,4002,3987,3970,3952,3933,3913,3892,3869,3845,3821,3795,3768,3740,3711,3682,3651,3619,3586,3552,3518,3482,3446,3408,3370,3331,3292,3251,3210,3168,3126,3082,3039,2994,2949,2904,
		2857,2811,2764,2716,2669,2620,2572,2523,2474,2424,2374,2325,2275,2224,2174,2124,2073,2023,1972,1922,1872,1821,1771,1722,1672,1622,1573,1524,1476,1427,1380,1332,1285,1239,1192,1147,1102,1057,
		1014,970,928,886,845,804,765,726,688,650,614,578,544,510,477,445,414,385,356,328,301,275,251,227,204,183,163,144,126,109,94,79,66,54,43,34,25,18,12,8,4,2,1,1,3,6,10,15,22,29,38,48,60,72,86,
		101,117,135,153,173,194,216,239,263,288,314,342,370,399,430,461,493,527,561,596,632,669,707,745,784,824,865,907,949,992,1035,1080,1124,1170,1215,1262,1309,1356,1403,1452,1500,1549,1598,1647,1697,1746,1796,1847,1897,1947,1998,2048,};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_USB_OTG_FS_PCD_Init(void);
static void MX_TIM13_Init(void);
static void MX_DAC_Init(void);
static void MX_TIM11_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM9_Init(void);
/* USER CODE BEGIN PFP */
void SPI_Transmit (uint16_t *data, int size);
void SPI_Enable (void);
void Change_Wave(void);
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
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_TIM13_Init();
  MX_DAC_Init();
  MX_TIM11_Init();
  MX_SPI1_Init();
  MX_TIM9_Init();
  /* USER CODE BEGIN 2 */

  Delta_Phase = pow(2, 32) * ((uint32_t)1000) / 100000;
  for (uint16_t jk=0; jk< 1024; jk++)
  {
     lut[jk]= (2048.0+(2047.0-150.0)*((float)Amplitude/5.0)*sin(2*PI*jk/1024));
  }

  // Start timer
  SPI_Enable();
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
  HAL_TIM_Base_Start_IT(&htim13);
  HAL_TIM_Base_Start_IT(&htim11);
  HAL_TIM_Base_Start_IT(&htim9);
  HAL_DAC_Start(&hdac, DAC_CHANNEL_1);

  HAL_UART_Transmit(&huart3, "Main function\n\r" , strlen("Main function\n\r"),1000);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  ConsoleInit();
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);

  while (1)
  {
	  ConsoleProcess();

      // Input state variables - Mode,frequency,amplitude
	  //Output state variable - delta phase, LUT, and amplitude(inturn update LUT)
	  if (event != NO_EVENT)
	  {
		  switch(Current_state){
		  case STATE_INIT:
			  if (event == CHANGE_WAVE)
			  {
				  Change_Wave();
			  }
			  else if (event == CHANGE_FREQUENCY)
			  {

				  Delta_Phase = pow(2, 32) * ((uint32_t)Frequency) / fs;
			  }
			  else if (event == CHANGE_AMPLITUDE)
			  {
				  for (uint16_t jk=0; jk< 1024; jk++)
				  {
					  lut[jk]= (2048.0+(2047.0-150.0)*((float)Amplitude/5.0)*sin(2*PI*jk/1024));
				  }
			  }
			  break;


		  case STATE_SINE_WAVE:
			  if (event == CHANGE_WAVE)
			  {
				  Change_Wave();
			  }
			  else if (event == CHANGE_FREQUENCY)
			  {
				  Delta_Phase = pow(2, 32) * ((uint32_t)Frequency) / fs;
			  }
			  else if (event == CHANGE_AMPLITUDE)
			  {
				  for (uint16_t jk=0; jk< 1024; jk++)
				  {
					  lut[jk]= (2048.0+(2047.0-150.0)*((float)Amplitude/5.0)*sin(2*PI*jk/1024));
				  }
			  }
			  break;

		  case STATE_SAWTOOTH_WAVE:
			  if (event == CHANGE_WAVE)
			  {
				  Change_Wave();
			  }
			  else if (event == CHANGE_FREQUENCY)
			  {
				  Delta_Phase = pow(2, 32) * ((uint32_t)Frequency) / fs;
			  }
			  else if (event == CHANGE_AMPLITUDE)
			  {
				  for (uint16_t jk=0; jk< 1024; jk++)
				  {
					  lut[jk]= (jk* 4)*(Amplitude/5.25);
				  }
			  }
			  break;
		  case STATE_SQUARE_WAVE:
			  if (event == CHANGE_WAVE)
			  {
				  Change_Wave();
			  }
			  else if (event == CHANGE_FREQUENCY)
			  {
				  Delta_Phase = pow(2, 32) * ((uint32_t)Frequency) / fs;
			  }
			  else if (event == CHANGE_AMPLITUDE)
			  {
				  for (uint16_t jk=0; jk< 512; jk++)
				  {
					  lut[jk]= (2048.0+(2047.0-150.0)*((float)Amplitude/5.0)*1);
				  }

				  for (uint16_t jk=512; jk< 1024; jk++)
				  {
					  lut[jk]= (2048.0+(2047.0-150.0)*((float)Amplitude/5.0)*(-1));
				  }
			  }
			  break;
		  case STATE_DC:
			  if (event == CHANGE_WAVE)
			  {
				  Change_Wave();
			  }
			  else if (event == CHANGE_AMPLITUDE)
			  {
				  for (uint16_t jk=0; jk< 1024; jk++)
				  {
					  lut[jk]= -(2048*(((float)Amplitude-5.0)/5.0));
				  }
			  }
			  break;
		  }
		  event = NO_EVENT;
	  }

	  if(BlinkSpeed == 0)
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, 0);

	  }
	  else if(BlinkSpeed == 1)
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, 1);

	  }
	  else if(BlinkSpeed == 2)
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, 0);
	  }
	  //HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14); //red
	  HAL_Delay(50);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief DAC Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC_Init(void)
{

  /* USER CODE BEGIN DAC_Init 0 */

  /* USER CODE END DAC_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC_Init 1 */

  /* USER CODE END DAC_Init 1 */

  /** DAC Initialization
  */
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT1 config
  */
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC_Init 2 */

  /* USER CODE END DAC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_16BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM9 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM9_Init(void)
{

  /* USER CODE BEGIN TIM9_Init 0 */

  /* USER CODE END TIM9_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};

  /* USER CODE BEGIN TIM9_Init 1 */

  /* USER CODE END TIM9_Init 1 */
  htim9.Instance = TIM9;
  htim9.Init.Prescaler = 1-1;
  htim9.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim9.Init.Period = 2160-1;
  htim9.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim9.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim9) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim9, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM9_Init 2 */

  /* USER CODE END TIM9_Init 2 */

}

/**
  * @brief TIM11 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM11_Init(void)
{

  /* USER CODE BEGIN TIM11_Init 0 */

  /* USER CODE END TIM11_Init 0 */

  /* USER CODE BEGIN TIM11_Init 1 */

  /* USER CODE END TIM11_Init 1 */
  htim11.Instance = TIM11;
  htim11.Init.Prescaler = 108-1;
  htim11.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim11.Init.Period = 2000-1;
  htim11.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim11.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim11) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM11_Init 2 */

  /* USER CODE END TIM11_Init 2 */

}

/**
  * @brief TIM13 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM13_Init(void)
{

  /* USER CODE BEGIN TIM13_Init 0 */

  /* USER CODE END TIM13_Init 0 */

  /* USER CODE BEGIN TIM13_Init 1 */

  /* USER CODE END TIM13_Init 1 */
  htim13.Instance = TIM13;
  htim13.Init.Prescaler = 108-1;
  htim13.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim13.Init.Period = 5000-1;
  htim13.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim13.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim13) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM13_Init 2 */

  /* USER CODE END TIM13_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief USB_OTG_FS Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_OTG_FS_PCD_Init(void)
{

  /* USER CODE BEGIN USB_OTG_FS_Init 0 */

  /* USER CODE END USB_OTG_FS_Init 0 */

  /* USER CODE BEGIN USB_OTG_FS_Init 1 */

  /* USER CODE END USB_OTG_FS_Init 1 */
  hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
  hpcd_USB_OTG_FS.Init.dev_endpoints = 6;
  hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_OTG_FS.Init.Sof_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.battery_charging_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.vbus_sensing_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_OTG_FS_Init 2 */

  /* USER CODE END USB_OTG_FS_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|SPI_CS_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_Btn_Pin */
  GPIO_InitStruct.Pin = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin LD3_Pin SPI_CS_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin|SPI_CS_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/* Timer13(APB1 -108Mhz) interupt that fires every 5 ms to check push button press and handle debouncing*/
/* Timer11(APB2 - 216Mhz) interupt that fires every 1 ms */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

	if (htim == &htim13 )/* Timer13 interupt that fires every 5 ms*/
	{
		if (!debounceRequest)
		{
			if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == 1)
			{
				HAL_UART_Transmit(&huart3, "Button Pressed\n\r" , strlen("Button Pressed\n\r"),1000);
				if(BlinkSpeed == 2)
				{
					BlinkSpeed = 0;
				}
				else
				{
					BlinkSpeed ++;

				}
				debounceRequest = 1;
			}
		}
		// Handle debounce
		else
		{
			if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == 1)
				debounceCount = 0;
			else
			{
				debounceCount ++;
				if (debounceCount == 10)
				{
					debounceCount = 0;
					debounceRequest = 0;
				}
			}
		}



	}


	if (htim == &htim11 ) /* Timer11 interupt that fires every 1 ms */
	{

	}

	if (htim == &htim9 ) /* Timer9 interupt that fires every 10 us(1000KHz) */
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14); //red
		//SPI_Enable();
		//spi_data = 0x3000|Value_DAC_SPI;
		phase = phase + Delta_Phase;
		Value_DAC_SPI = phase>>22;
		spi_data = 0x3000|lut[Value_DAC_SPI];
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
		SPI_Transmit((uint16_t*)&spi_data, 1);
		//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);

		/*if (Value_DAC_SPI<255)
		{
			Value_DAC_SPI = Value_DAC_SPI + 5;
		}
		else{
			Value_DAC_SPI = 0;
			//HAL_UART_Transmit(&huart3, "DAC Pressed\n\r" , strlen("DAC Pressed\n\r"),HAL_MAX_DELAY);
		}*/
	}


}

void SPI_Transmit (uint16_t *data, int size)
{

	/************** STEPS TO FOLLOW *****************
	1. Wait for the TXE bit to set in the Status Register
	2. Write the data to the Data Register
	3. After the data has been transmitted, wait for the BSY bit to reset in Status Register
	4. Clear the Overrun flag by reading DR and SR
	************************************************/

	int i=0;
	while (i<size)
	{
	   while (!((SPI1->SR)&(1<<1))) {};  // wait for TXE bit to set -> This will indicate that the buffer is empty
	   SPI1->DR = data[i];  // load the data into the Data Register
	   i++;
	}


/*During discontinuous communications, there is a 2 APB clock period delay between the
write operation to the SPI_DR register and BSY bit setting. As a consequence it is
mandatory to wait first until TXE is set and then until BSY is cleared after writing the last
data.
*/
	//while (!((SPI1->SR)&(1<<1))) {};  // wait for TXE bit to set -> This will indicate that the buffer is empty
	//while (((SPI1->SR)&(1<<7))) {};  // wait for BSY bit to Reset -> This will indicate that SPI is not busy in communication

	//  Clear the Overrun flag by reading DR and SR
	uint8_t temp = SPI1->DR;
					temp = SPI1->SR;

}

void SPI_Enable (void)
{
	SPI1->CR1 |= (1<<6);   // SPE=1, Peripheral enabled
}

void Change_Wave(void)
{
	if (Wave == SINE)
	{
		for (uint16_t jk=0; jk< 1024; jk++)
		{
			lut[jk]= (2048.0+(2047.0-150.0)*((float)Amplitude/5.0)*sin(2*PI*jk/1024));
		}
		Current_state = STATE_SINE_WAVE;

	}
	if (Wave == SAWTOOTH)
	{
		for (uint16_t jk=0; jk< 1024; jk++)
		{
			lut[jk]= (jk* 4)*(Amplitude/5.25);
		}
		Current_state = STATE_SAWTOOTH_WAVE;
	}
	if (Wave == SQUARE)
	{
		for (uint16_t jk=0; jk< 512; jk++)
		{
			lut[jk]= (2048.0+(2047.0-150.0)*((float)Amplitude/5.0)*1);
		}

		for (uint16_t jk=512; jk< 1024; jk++)
		{
			lut[jk]= (2048.0+(2047.0-150.0)*((float)Amplitude/5.0)*(-1));
		}
		Current_state = STATE_SQUARE_WAVE;

	}
	if (Wave == DC)
	{
		for (uint16_t jk=0; jk< 1024; jk++)
		{
			lut[jk]= -(2048*(((float)Amplitude-5.0)/5.0));
		}
		Current_state = STATE_DC;
	}
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

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "app_touchgfx.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stm32746g_discovery_qspi.h>
#include <stdio.h>
#include "queue.h"
#include "event_groups.h"
#include "myTasks.h"
#include "qPackages.h"
#include "FreeRTOS.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define REFRESH_COUNT        1835

#define SDRAM_TIMEOUT                            ((uint32_t)0xFFFF)
#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)



/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

CRC_HandleTypeDef hcrc;

DMA2D_HandleTypeDef hdma2d;

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c3;

LTDC_HandleTypeDef hltdc;

QSPI_HandleTypeDef hqspi;

TIM_HandleTypeDef htim7;

UART_HandleTypeDef huart7;

SDRAM_HandleTypeDef hsdram1;

osThreadId TouchGFXTaskHandle;
/* USER CODE BEGIN PV */
static FMC_SDRAM_CommandTypeDef Command;
QueueHandle_t qhGUItoEncoderControl;
QueueHandle_t qhEncoderControlToReport;
QueueHandle_t qhReportToTouchGFX;
QueueHandle_t qhTouchGFXToRelaySetup;
QueueHandle_t qhStatusReport;
QueueHandle_t qhGUItoWriteSettings;
QueueHandle_t qhSettingsToGUI;
QueueHandle_t qhUARTtoConsole;
QueueHandle_t qhUARTtoGRBL;
QueueHandle_t qhTouchGFXToGRBLControl;
QueueHandle_t qhGRBLControlToTouchGFX;
QueueHandle_t qhTIM3_ISRtoTasks;

EventGroupHandle_t ehEvents;
EventGroupHandle_t ehEventsSoftwareTimer;
//EventGroupHandle_t ehEventsGRBL;

TaskHandle_t thEncoderControl;
TaskHandle_t thReport;
TaskHandle_t thRelaySetup;
TaskHandle_t thSingleEvent;
TaskHandle_t thWriteSettings;
TaskHandle_t thGrblCommunicationTask;

char UARTRxGlobalBuffer[UART_RX_GLOBAL_BUFFER_SIZE];
char* pUARTRxGlobalBuffer;

enum grblConn_t grblConnectionStatus;

//const char* eepromStatus_strings[]  = {
//				"EEPROM_SUCCESS",
//				"EEPROM_TIMEOUT_2",
//				"EEPROM_TIMEOUT_3",
//				"EEPROM_TIMEOUT_3",
//				"EEPROM_TIMEOUT_4",
//				"EEPROM_BUSY",
//				"EEPROM_TXFULL",
//				"EEPROM_ERR"
//};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_CRC_Init(void);
static void MX_DMA2D_Init(void);
static void MX_FMC_Init(void);
static void MX_I2C3_Init(void);
static void MX_LTDC_Init(void);
static void MX_QUADSPI_Init(void);
static void MX_I2C1_Init(void);
static void MX_UART7_Init(void);
static void MX_TIM7_Init(void);
void StartTouchGFXTask(void const * argument);

/* USER CODE BEGIN PFP */
void TIM3_Init(void);	// encoder
void TIM4_Init(void);	// forwarder to other timers
void TIM1_Init(void);	// RLY1
void TIM12_Init(void);	// RLY2
void TIM8_Init(void);	// RLY3
void TIM5_Init(void);	// Amount counter
void TIM7_Init(void);	// Velocity tracking

// TIM6 is used for HAL_GetTick();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//#include "stdio.h"

int _write(int file, char *ptr, int len) {
    for (int i = 0; i < len; i++) {
        ITM_SendChar(ptr[i]); // Send character to SWO
    }
    return len;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	uint32_t i = 0;
	while(i < 100000){ // apparently this pause fixes the problem when UI freezes (does not update)
		i++;
	}

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

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
  MX_CRC_Init();
  MX_DMA2D_Init();
  MX_FMC_Init();
  MX_I2C3_Init();
  MX_LTDC_Init();
  MX_QUADSPI_Init();
  MX_I2C1_Init();
  MX_UART7_Init();
  MX_TIM7_Init();
  MX_TouchGFX_Init();
  /* USER CODE BEGIN 2 */
  TIM3_Init();	// encoder
  TIM4_Init();	// forwarder for other timers
  TIM1_Init();	// RLY1
  TIM12_Init();	// RLY2
  TIM8_Init();	// RLY3
  TIM5_Init();  // Amount counter
  TIM7_Init(); // Velocity tracking
  // Stop timers in debug
  DBGMCU->APB1FZ |= 0x1FF;
  DBGMCU->APB2FZ |= (DBGMCU_APB2_FZ_DBG_TIM1_STOP | DBGMCU_APB2_FZ_DBG_TIM8_STOP | DBGMCU_APB2_FZ_DBG_TIM9_STOP | DBGMCU_APB2_FZ_DBG_TIM10_STOP | DBGMCU_APB2_FZ_DBG_TIM11_STOP);

  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  qhGUItoEncoderControl = xQueueCreate(1, sizeof(qPackage_encoderControl_t));
  qhEncoderControlToReport = xQueueCreate(1, sizeof(qPackage_encoderControl_t));
  qhReportToTouchGFX = xQueueCreate(1 , sizeof(qPackage_report_t)); 	// reports current length and amount
  qhTouchGFXToRelaySetup = xQueueCreate(1, sizeof(qPackage_relaySetup_t));
  qhStatusReport = xQueueCreate(20, sizeof(qPackage_statusReport_t)); // can be called from 5 higher priority tasks
  qhGUItoWriteSettings = xQueueCreate(3, sizeof(qPackage_settings_t));
  qhSettingsToGUI = xQueueCreate(1,sizeof(qPackage_settings_t));
  qhUARTtoConsole = xQueueCreate(20, sizeof(qPackage_UART));
  qhUARTtoGRBL = xQueueCreate(20, sizeof(qPackage_UART));
  qhTouchGFXToGRBLControl = xQueueCreate(5, sizeof(qPackage_laserParams_t));
  qhGRBLControlToTouchGFX = xQueueCreate(2, sizeof(qPackage_laserParams_t));
  qhTIM3_ISRtoTasks = xQueueCreate(1, sizeof(double));

//  ehEventsGRBL = xEventGroupCreate();
  ehEvents = xEventGroupCreate();
  ehEventsSoftwareTimer = xEventGroupCreate();
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of TouchGFXTask */
  osThreadDef(TouchGFXTask, StartTouchGFXTask, osPriorityNormal, 0, 8192);
  TouchGFXTaskHandle = osThreadCreate(osThread(TouchGFXTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  xTaskCreate(encoderControlTask,"encoderControlTask",256,NULL,3+2 ,&thEncoderControl);
  xTaskCreate(reportTask, "reportTask",256,NULL,3+1,&thReport);
  xTaskCreate(relaySetupTask, "relaySetupTask", 256, NULL,3+2, &thRelaySetup);
  xTaskCreate(singleEventTask, "singleEventTask", 512, NULL, 3+2, &thSingleEvent);
  xTaskCreate(writeSettingsTask, "writeSettingsTask", 512, NULL,3+2 , &thWriteSettings);
  xTaskCreate(grblCommunicationTask, "grblCommunicationTask", 512, NULL,3+3 , &thGrblCommunicationTask);

  HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_RESET);

  pUARTRxGlobalBuffer = UARTRxGlobalBuffer;

//  HAL_UARTEx_ReceiveToIdle_IT(&huart7, pUARTRxGlobalBuffer, UART_RX_GLOBAL_BUFFER_SIZE); // move this to appropriate place in code

  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 400;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC|RCC_PERIPHCLK_UART7
                              |RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_I2C3;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 384;
  PeriphClkInitStruct.PLLSAI.PLLSAIR = 5;
  PeriphClkInitStruct.PLLSAI.PLLSAIQ = 2;
  PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV2;
  PeriphClkInitStruct.PLLSAIDivQ = 1;
  PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_8;
  PeriphClkInitStruct.Uart7ClockSelection = RCC_UART7CLKSOURCE_PCLK1;
  PeriphClkInitStruct.I2c1ClockSelection = RCC_I2C1CLKSOURCE_SYSCLK;
  PeriphClkInitStruct.I2c3ClockSelection = RCC_I2C3CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief DMA2D Initialization Function
  * @param None
  * @retval None
  */
static void MX_DMA2D_Init(void)
{

  /* USER CODE BEGIN DMA2D_Init 0 */

  /* USER CODE END DMA2D_Init 0 */

  /* USER CODE BEGIN DMA2D_Init 1 */

  /* USER CODE END DMA2D_Init 1 */
  hdma2d.Instance = DMA2D;
  hdma2d.Init.Mode = DMA2D_M2M;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_ARGB8888;
  hdma2d.Init.OutputOffset = 0;
  hdma2d.LayerCfg[1].InputOffset = 0;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0;
  if (HAL_DMA2D_Init(&hdma2d) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DMA2D_Init 2 */

  /* USER CODE END DMA2D_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x109035B7;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */
	// 	Enable
  I2C1->CR1 |= I2C_CR1_PE_Msk;

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C3_Init(void)
{

  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */
  hi2c3.Instance = I2C3;
  hi2c3.Init.Timing = 0x00C0EAFF;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

}

/**
  * @brief LTDC Initialization Function
  * @param None
  * @retval None
  */
static void MX_LTDC_Init(void)
{

  /* USER CODE BEGIN LTDC_Init 0 */

  /* USER CODE END LTDC_Init 0 */

  LTDC_LayerCfgTypeDef pLayerCfg = {0};

  /* USER CODE BEGIN LTDC_Init 1 */

  /* USER CODE END LTDC_Init 1 */
  hltdc.Instance = LTDC;
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  hltdc.Init.HorizontalSync = 40;
  hltdc.Init.VerticalSync = 9;
  hltdc.Init.AccumulatedHBP = 53;
  hltdc.Init.AccumulatedVBP = 11;
  hltdc.Init.AccumulatedActiveW = 533;
  hltdc.Init.AccumulatedActiveH = 283;
  hltdc.Init.TotalWidth = 565;
  hltdc.Init.TotalHeigh = 285;
  hltdc.Init.Backcolor.Blue = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Red = 0;
  if (HAL_LTDC_Init(&hltdc) != HAL_OK)
  {
    Error_Handler();
  }
  pLayerCfg.WindowX0 = 0;
  pLayerCfg.WindowX1 = 480;
  pLayerCfg.WindowY0 = 0;
  pLayerCfg.WindowY1 = 272;
  pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
  pLayerCfg.Alpha = 255;
  pLayerCfg.Alpha0 = 0;
  pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
  pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
  pLayerCfg.FBStartAdress = 0xC0000000;
  pLayerCfg.ImageWidth = 480;
  pLayerCfg.ImageHeight = 272;
  pLayerCfg.Backcolor.Blue = 0;
  pLayerCfg.Backcolor.Green = 0;
  pLayerCfg.Backcolor.Red = 0;
  if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LTDC_Init 2 */

  /* USER CODE END LTDC_Init 2 */

}

/**
  * @brief QUADSPI Initialization Function
  * @param None
  * @retval None
  */
static void MX_QUADSPI_Init(void)
{

  /* USER CODE BEGIN QUADSPI_Init 0 */

  /* USER CODE END QUADSPI_Init 0 */

  /* USER CODE BEGIN QUADSPI_Init 1 */

  /* USER CODE END QUADSPI_Init 1 */
  /* QUADSPI parameter configuration*/
  hqspi.Instance = QUADSPI;
  hqspi.Init.ClockPrescaler = 1;
  hqspi.Init.FifoThreshold = 4;
  hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
  hqspi.Init.FlashSize = 24;
  hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_6_CYCLE;
  hqspi.Init.ClockMode = QSPI_CLOCK_MODE_0;
  hqspi.Init.FlashID = QSPI_FLASH_ID_1;
  hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
  if (HAL_QSPI_Init(&hqspi) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN QUADSPI_Init 2 */
  BSP_QSPI_Init();

  BSP_QSPI_MemoryMappedMode();
  HAL_NVIC_DisableIRQ(QUADSPI_IRQn);
  /* USER CODE END QUADSPI_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 0;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 65535;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief UART7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART7_Init(void)
{

  /* USER CODE BEGIN UART7_Init 0 */

  /* USER CODE END UART7_Init 0 */

  /* USER CODE BEGIN UART7_Init 1 */

  /* USER CODE END UART7_Init 1 */
  huart7.Instance = UART7;
  huart7.Init.BaudRate = 115200;
  huart7.Init.WordLength = UART_WORDLENGTH_8B;
  huart7.Init.StopBits = UART_STOPBITS_1;
  huart7.Init.Parity = UART_PARITY_NONE;
  huart7.Init.Mode = UART_MODE_TX_RX;
  huart7.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart7.Init.OverSampling = UART_OVERSAMPLING_16;
  huart7.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart7.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart7) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART7_Init 2 */

  /* USER CODE END UART7_Init 2 */

}

/* FMC initialization function */
static void MX_FMC_Init(void)
{

  /* USER CODE BEGIN FMC_Init 0 */

  /* USER CODE END FMC_Init 0 */

  FMC_SDRAM_TimingTypeDef SdramTiming = {0};

  /* USER CODE BEGIN FMC_Init 1 */

  /* USER CODE END FMC_Init 1 */

  /** Perform the SDRAM1 memory initialization sequence
  */
  hsdram1.Instance = FMC_SDRAM_DEVICE;
  /* hsdram1.Init */
  hsdram1.Init.SDBank = FMC_SDRAM_BANK1;
  hsdram1.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_8;
  hsdram1.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_12;
  hsdram1.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16;
  hsdram1.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  hsdram1.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_3;
  hsdram1.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  hsdram1.Init.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;
  hsdram1.Init.ReadBurst = FMC_SDRAM_RBURST_ENABLE;
  hsdram1.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_0;
  /* SdramTiming */
  SdramTiming.LoadToActiveDelay = 2;
  SdramTiming.ExitSelfRefreshDelay = 7;
  SdramTiming.SelfRefreshTime = 4;
  SdramTiming.RowCycleDelay = 7;
  SdramTiming.WriteRecoveryTime = 3;
  SdramTiming.RPDelay = 2;
  SdramTiming.RCDDelay = 2;

  if (HAL_SDRAM_Init(&hsdram1, &SdramTiming) != HAL_OK)
  {
    Error_Handler( );
  }

  /* USER CODE BEGIN FMC_Init 2 */
  __IO uint32_t tmpmrd = 0;

    /* Step 1: Configure a clock configuration enable command */
    Command.CommandMode            = FMC_SDRAM_CMD_CLK_ENABLE;
    Command.CommandTarget          =  FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);

    /* Step 2: Insert 100 us minimum delay */
    /* Inserted delay is equal to 1 ms due to systick time base unit (ms) */
    HAL_Delay(1);

    /* Step 3: Configure a PALL (precharge all) command */
    Command.CommandMode            = FMC_SDRAM_CMD_PALL;
    Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);

    /* Step 4: Configure an Auto Refresh command */
    Command.CommandMode            = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber      = 8;
    Command.ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);

    /* Step 5: Program the external memory mode register */
    tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1 | \
             SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL    | \
             SDRAM_MODEREG_CAS_LATENCY_3            | \
             SDRAM_MODEREG_OPERATING_MODE_STANDARD  | \
             SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

    Command.CommandMode            = FMC_SDRAM_CMD_LOAD_MODE;
    Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = tmpmrd;

    /* Send the command */
    HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);

    /* Step 6: Set the refresh rate counter */
    /* Set the device refresh rate */
    HAL_SDRAM_ProgramRefreshRate(&hsdram1, REFRESH_COUNT);
    
    //Deactivate speculative/cache access to first FMC Bank to save FMC bandwidth
    FMC_Bank1->BTCR[0] = 0x000030D2;
  /* USER CODE END FMC_Init 2 */
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOJ_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOK_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_BL_CTRL_GPIO_Port, LCD_BL_CTRL_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_DISP_GPIO_Port, LCD_DISP_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : LCD_BL_CTRL_Pin */
  GPIO_InitStruct.Pin = LCD_BL_CTRL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_BL_CTRL_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ERROR_LED_Pin LCD_DISP_Pin */
  GPIO_InitStruct.Pin = ERROR_LED_Pin|LCD_DISP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pin : RLY1_Pin */
  GPIO_InitStruct.Pin = RLY1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
  HAL_GPIO_Init(RLY1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ENC2_Pin ENC1_Pin */
  GPIO_InitStruct.Pin = ENC2_Pin|ENC1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : IMM_CUT_PIN_Pin */
  GPIO_InitStruct.Pin = IMM_CUT_PIN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(IMM_CUT_PIN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : RLY2_Pin */
  GPIO_InitStruct.Pin = RLY2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF9_TIM12;
  HAL_GPIO_Init(RLY2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : RLY3_Pin */
  GPIO_InitStruct.Pin = RLY3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF3_TIM8;
  HAL_GPIO_Init(RLY3_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 4 */

void reportErrorToPopUp(enum statusId_t statusId, char* description, uint32_t data){
	qPackage_statusReport_t packageToSend;
	packageToSend.statusId = statusId;
	packageToSend.data = data;
	sprintf(packageToSend.message, description);
	BaseType_t xStatus = xQueueSend(qhStatusReport, &packageToSend, pdMS_TO_TICKS(10));
	if(xStatus == errQUEUE_FULL){
		//TODO: report a problem
		while(1){
			HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
		};
	}
}

void reportErrorToPopUpFromISR(enum statusId_t statusId, char* description, uint32_t data){
	qPackage_statusReport_t packageToSend;
	packageToSend.statusId = statusId;
	packageToSend.data = data;
	sprintf(packageToSend.message, description);
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	BaseType_t xStatus = xQueueSendFromISR(qhStatusReport, &packageToSend, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	if(xStatus == errQUEUE_FULL){
		//TODO: report a problem
		while(1){
			HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
		};
	}
}


/*
 * Init TIM4 timer to be used as forward timer from TIM3 (encoder) to TIM1, TIM8, TIM12
 */
void TIM4_Init(void){
	// activate clock for peripheral TIM4
	RCC->APB1ENR |= 1UL << RCC_APB1ENR_TIM4EN_Pos;
	// One pulse mode
	TIM4->CR1 = 1UL << TIM_CR1_OPM_Pos;
	// Master mode selection: CNT_EN as trigger output (TRGO)
	TIM4->CR2 = 0b001 << TIM_CR2_MMS_Pos;
	// Trigger selection: ITR2 (TIM3), Slave mode selection: Trigger mode - the counter starts on rising edge of TRGI
	TIM4->SMCR = 0b110 << TIM_SMCR_SMS_Pos | 0b010 << TIM_SMCR_TS_Pos;
	// Let it stop immediately.
	TIM4->ARR = 1;
	// Enable interrupt to be used in case of SW timers
	TIM4->DIER |= TIM_DIER_UIE;  // Enable update interrupt

}

/*
 * Init TIM1 timer to control RLY1
 */
void TIM1_Init(void){
	// activate clock for peripheral TIM1
	RCC->APB2ENR |= 1UL << RCC_APB2ENR_TIM1EN_Pos;
	// Lower its frequency from 100 MHz to 1.525879 kHz
	TIM1->PSC = 0xFFFF;
	// One pulse mode, ARR pre-load active: UG event needs to be issued after updating.
	//	This ensures timer does not run past ARR should ARR be decreased just before being reached
	TIM1->CR1 = 1UL << TIM_CR1_OPM_Pos | 1UL << TIM_CR1_ARPE_Pos;
	// Slave mode selection: Trigger mode - counter starts counting on rising edge of TRGI, TS is TIM4
	TIM1->SMCR = 0b110 << TIM_SMCR_SMS_Pos | 0b011 << TIM_SMCR_TS_Pos;

	// Master enable output MOE, Off-state selection for Run and Idle mode: Inactive level when in off state
	TIM1->BDTR = 1UL << TIM_BDTR_MOE_Pos;
	// force inactive level: PWM Mode 2 on channel 1 set in relaySetupTask when relay activated, Output Compare 1 pre-load enable
	TIM1->CCMR1 = 0b100 << TIM_CCMR1_OC1M_Pos | 1UL << TIM_CCMR1_OC1PE_Pos;

	// Set ARR and CC1 to max
	TIM1->ARR = 0xFFFF;
	TIM1->CCR1 = 0xFFFF;
	TIM1->CCR3 = 0xFFFF;
	// Update event generation as ARR and CCR1 are preloaded
	TIM1->EGR = 1UL << TIM_EGR_UG_Pos;
	// Output Compare 1 output enable
	TIM1->CCER = 1UL << TIM_CCER_CC1E_Pos;

    // 1. Enable update interrupt and capture compare interrupt. Used to initiate laser cutting.
    // Enable capture/compare interrupt on channel 1 for laser initiation.
    // Enable capture/compare interrupt on channel 3 for message assembly and transfer trigger.

    TIM1->DIER |= TIM_DIER_CC1IE | TIM_DIER_CC3IE;

}

/*
 * Init TIM12 timer to control RLY2
 */
void TIM12_Init(void){
	// activate clock for peripheral TIM12
	RCC->APB1ENR |= 1UL << RCC_APB1ENR_TIM12EN_Pos;
	// Lower its frequency from 100 MHz to 1.525879 kHz
	TIM12->PSC = 0xFFFF;
	// One pulse mode, ARR pre-load active: UG event needs to be issued after updating. This ensures timer does not run past ARR should ARR be decreased just being reached
	TIM12->CR1 = 1UL << TIM_CR1_OPM_Pos | 1UL << TIM_CR1_ARPE_Pos;
	// Slave mode selection: Trigger mode - counter starts counting on rising edge of TRGI, TS is TIM4
	TIM12->SMCR = 0b110 << TIM_SMCR_SMS_Pos | 0b000 << TIM_SMCR_TS_Pos;
	// Master enable output MOE
	TIM12->BDTR = 1UL << TIM_BDTR_MOE_Pos;
	// force inactive level: PWM Mode 2 on channel 1 set in relaySetupTask when relay activated, Output Compare 1 pre-load enable
	TIM12->CCMR1 = 0b100 << TIM_CCMR1_OC1M_Pos | 1UL << TIM_CCMR1_OC1PE_Pos;
	// Set ARR and CC1 to max
	TIM12->ARR = 0xFFFF;
	TIM12->CCR1 = 0xFFFF;
	// Update event generation as ARR and CCR1 are preloaded
	TIM12->EGR = 1UL << TIM_EGR_UG_Pos;
	// Output Compare 1 output enable
	TIM12->CCER = 1UL << TIM_CCER_CC1E_Pos;
}

/*
 * Init TIM8 timer to control RLY3
 */
void TIM8_Init(void){
	// activate clock for peripheral TIM8
	RCC->APB2ENR |= 1UL << RCC_APB2ENR_TIM8EN_Pos;
	// Lower its frequency from 100 MHz to 1.525879 kHz
	TIM8->PSC = 0xFFFF;
	// One pulse mode, ARR pre-load active: UG event needs to be issued after updating. This ensures timer does not run past ARR should ARR be decreased just being reached
	TIM8->CR1 = 1UL << TIM_CR1_OPM_Pos | 1UL << TIM_CR1_ARPE_Pos;
	// Slave mode selection: Trigger mode - counter starts counting on rising edge of TRGI, TS is TIM4
	TIM8->SMCR = 0b110 << TIM_SMCR_SMS_Pos | 0b010 << TIM_SMCR_TS_Pos;
	// Master enable output MOE
	TIM8->BDTR = 1UL << TIM_BDTR_MOE_Pos;
	// force inactive level: PWM Mode 2 on channel 1 set in relaySetupTask when relay activated, Output Compare 1 pre-load enable
	TIM8->CCMR2 = 0b100 << TIM_CCMR2_OC3M_Pos | 1UL << TIM_CCMR2_OC3PE_Pos;
	// Set ARR and CC3 to max
	TIM8->ARR = 0xFFFF;
	TIM8->CCR3 = 0xFFFF;
	// Update event generation as ARR and CCR1 are preloaded
	TIM8->EGR = 1UL << TIM_EGR_UG_Pos;
	// Output Compare 1 output enable
	TIM8->CCER = 1UL << TIM_CCER_CC3NE_Pos;

}




/*
 * Init TIM5 timer to count amount
 */
void TIM5_Init(void){
	// activate clock for peripheral TIM5
	RCC->APB1ENR |= 1UL << RCC_APB1ENR_TIM5EN_Pos;

	// Interrupt generation on trigger input
	TIM5->DIER = 1UL << TIM_DIER_TIE_Pos;

	// SMS: External clock mode 1, TS: Internal trigger 2: TIM4
	TIM5->SMCR = 0b010 << TIM_SMCR_TS_Pos | 0b111 << TIM_SMCR_SMS_Pos;

	// Counter Enable
	TIM5->CR1 = 1UL << TIM_CR1_CEN_Pos;

}

/*
 * Init TIM3 timer to be used as encoder
 * Encoder will count two pulses when a single pulse happens because it counts on both rising and falling edge!
 */
void TIM3_Init(void){
	// activate clock for peripheral TIM3
	RCC->APB1ENR |= 1UL << RCC_APB1ENR_TIM3EN_Pos;
	// Master mode selection: Update Event as trigger output (TRGO)
	TIM3->CR2 = 0b010 << TIM_CR2_MMS_Pos;
	// Slave mode selection: Encoder mode 2 - Counter counts up/down on TI2FP2 (PC7) edge depending on TI1FP1 (PC6) level.
	TIM3->SMCR = 0b010 << TIM_SMCR_SMS_Pos;
	// Capture/Compare 2 & 1 Selection: CC2 & CC1 channels are configured as input, IC2/IC1 is mapped on TI2/TI1
	TIM3->CCMR1 = (0b01 << TIM_CCMR1_CC2S_Pos) | (0b01 << TIM_CCMR1_CC1S_Pos);
	// Capture/Compare 2 output polarity: inverted/falling edge.
	TIM3->CCER = 1UL << TIM_CCER_CC2P_Pos;

	// Set input sampling clock f_DTS and filtering:
//	f_CK_INT = 100MHz, DTS = 01 for f_DTS = 50Mhz, 10 for f_DTS = 25MHz
 	TIM3->CR1 |= (0b10 << TIM_CR1_CKD_Pos);
	// 0b1111 for sampling N=8 @ f_DTS/32. signal needs to be stable at least 0.01ms (period is 0.5ms)
//	TIM3->CCMR1 = (0b1111 << TIM_IC1F_Pos);
	// 0b1100 for sampling N=8 @ f_DTS/16. signal needs to be stable at least 0.005ms (period is 0.5ms)
	TIM3->CCMR1 |= (0b1100 << TIM_CCMR1_IC2F_Pos) | (0b1100 << TIM_CCMR1_IC1F_Pos);

	// TODO: Add possibility to invert encoder direction
}

//#define TIM7_TIMEBASE_MS (double)100.0


/*
 * Init TIM7 timer to calculate velocity. Input clock 100 MHz.
 */
void TIM7_Init(void) {
    // Enable clock for TIM7
    RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;

    // Configure TIM7
    TIM7->PSC = 9999;  // Prescaler (10,000 - 1)
//    TIM7->ARR = 999;   // Auto-reload (1,000 - 1)// 100ms
    TIM7->ARR = 9999;   // Auto-reload (1,000 - 1) // 1000ms
//    uint16_t arr = (((uint32_t)TIM7_TIMEBASE_MS*100*1e6)/(1000*((uint32_t)TIM7->PSC+1))-1);
//    TIM7->ARR = arr;

    // Enable update interrupt
    TIM7->DIER |= TIM_DIER_UIE;

    // Enable TIM7 counter
    TIM7->CR1 |= TIM_CR1_CEN;

}

void enableVelocityTracking(void) {
    HAL_NVIC_SetPriority(TIM7_IRQn, 6, 0);  // Priority level (1, 0)
//    HAL_NVIC_SetPriority(TIM7_IRQn, 2, 0);  // Priority level (1, 0)
    NVIC_EnableIRQ(TIM7_IRQn);
}

void disableVelocityTracking(void) {
    NVIC_DisableIRQ(TIM7_IRQn);
}


/*
 * For velocity calculation. Every 1000 ms.
 * */
void TIM7_IRQHandler(void) {
    if (TIM7->SR & TIM_SR_UIF) {  // Check if update interrupt flag is set
        TIM7->SR &= ~TIM_SR_UIF;   // Clear the interrupt flag
//        uint32_t TIM3_CNT = TIM3->CNT;
//        uint16_t currentTicks = (TIM3_CNT & 0x0000FFFF);
        uint16_t currentTicks = TIM3->CNT;
//        uint32_t overflow = TIM3_CNT >> TIM_CNT_UIFCPY_Pos;
//        uint16_t currentTicks_time = TIM7->CNT; // should reset at ARR = 999
        uint32_t currentTicks_time = HAL_GetTick(); // should reset at ARR = 999

        static uint16_t prevTicks = 0;
        static uint32_t prevTicks_time = 0;
		static uint16_t lastTime = 0;

        uint32_t difference_time = currentTicks_time - prevTicks_time;

        double velocity_ticks_per_100ms = 0;

//        int64_t difference = (int64_t)currentTicks - (int64_t)prevTicks;

//		uint16_t difference = prevTicks - currentTicks;
//		uint16_t difference = currentTicks - prevTicks;


        if (TIM3->CR1 & TIM_CR1_DIR) {
            // Counter is counting down (Reverse direction)
			uint32_t overflowCorrection = TIM3->ARR;
			uint16_t prevTicks_beforeCorrection = prevTicks;
			if(prevTicks < currentTicks){ // overflow has occurred (UIFREMAP) this can be indicated by UIFREMAP
//			if(overflow){ // overflow has occurred (UIFREMAP) this can be indicated by UIFREMAP
//				currentTicks -= TIM3->ARR;
				prevTicks += overflowCorrection;
//				prevTicks += TIM3->ARR;
				printf("Correction DOWN!\n");
			}
			uint16_t difference = currentTicks - prevTicks;
			int16_t difference_int = (int16_t)difference;
			velocity_ticks_per_100ms = (double)difference_int;


//			if(lastTime + 0 < HAL_GetTick()){
////				printf("Velocity: Direction down: %f ticks/100ms\n", velocity_ticks_per_100ms);
////				printf("Velocity: Dir down: %.0f/%u ticks (sig: %d)\n", velocity_ticks_per_100ms, currentTicks_time,difference_int);
//				printf("V dir DOWN: curr-prev: %u - %u (%u) = %.0f (T:%lu) \n", currentTicks, prevTicks, prevTicks_beforeCorrection, velocity_ticks_per_100ms, difference_time);
//
//				lastTime = HAL_GetTick();
//			}
//			if(difference_int < -3000){
////			if(difference_int < -300){
//				printf("Difference < -300! \n");
//			}

        } else {
            // Counter is counting up (Forward direction)
			uint32_t overflowCorrection = TIM3->ARR;
			uint16_t prevTicks_beforeCorrection = prevTicks;
//        	uint32_t overflowCorrection;
        	if(prevTicks > currentTicks){ // overflow has occured
//        	if(overflow){ // overflow has occured
//				currentTicks += TIM3->ARR;
				prevTicks -= overflowCorrection;
				printf("Correction UP!\n");
//				prevTicks -= TIM3->ARR;
			}
			uint16_t difference = currentTicks - prevTicks;
			velocity_ticks_per_100ms = (double)difference;


//			if(lastTime + 0 < HAL_GetTick()){
////				printf("Velocity: Direction up: %f ticks/100ms\n", velocity_ticks_per_100ms);
////				printf("Velocity: Dir up: %.0f/%u ticks (usig:%u)\n", velocity_ticks_per_100ms, currentTicks_time, difference);
//				printf("V dir UP: curr-prev: %u - %u (%u) = %.0f (T:%lu)\n", currentTicks, prevTicks, prevTicks_beforeCorrection, velocity_ticks_per_100ms, difference_time);
//				lastTime = HAL_GetTick();
//			}
//
//			if(difference > 3000){
////			if(difference > 300){
//				printf("Difference > 300! \n");
//			}

        }

        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		xQueueOverwriteFromISR(qhTIM3_ISRtoTasks, &velocity_ticks_per_100ms, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

		prevTicks = currentTicks;
		prevTicks_time = currentTicks_time;
    }
}


void TIM4_IRQHandler(void) {
    if (TIM4->SR & TIM_SR_UIF) {     // Check update interrupt flag
        TIM4->SR &= ~TIM_SR_UIF;     // Clear the flag by writing 0

        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		xEventGroupSetBitsFromISR(ehEventsSoftwareTimer, EVENT_SW_TIMER_BIT_LAUNCH, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
        // Your interrupt handling code here
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
#ifdef IMM_CUT
	if(GPIO_Pin == IMM_CUT_PIN_Pin){
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		xEventGroupSetBitsFromISR(ehEvents, EVENT_BIT_IMM_CUT, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
#endif
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartTouchGFXTask */
/**
  * @brief  Function implementing the TouchGFXTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartTouchGFXTask */
void StartTouchGFXTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
  MX_TouchGFX_Process();
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();
  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x90000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_256MB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress = 0x90000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_16MB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}
/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

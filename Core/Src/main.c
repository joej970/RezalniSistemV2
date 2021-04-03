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
#include "queue.h"
#include "event_groups.h"
#include "myTasks.h"
#include "qPackages.h"
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

#define EEPROM_ADDR		(uint8_t)0b10100000

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

EventGroupHandle_t ehEvents;

TaskHandle_t thEncoderControl;
TaskHandle_t thReport;
TaskHandle_t thRelaySetup;
TaskHandle_t thSingleEvent;
TaskHandle_t thWriteSettings;

const char* eepromStatus_strings[]  = {
				"EEPROM_SUCCESS",
				"EEPROM_TIMEOUT_2",
				"EEPROM_TIMEOUT_3",
				"EEPROM_TIMEOUT_3",
				"EEPROM_TIMEOUT_4",
				"EEPROM_BUSY",
				"EEPROM_TXFULL",
				"EEPROM_ERR"
};
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
void StartTouchGFXTask(void const * argument);

/* USER CODE BEGIN PFP */
void TIM3_Init(void);	// encoder
void TIM4_Init(void);	// forwarder to other timers
void TIM1_Init(void);	// RLY1
void TIM12_Init(void);	// RLY2
void TIM8_Init(void);	// RLY3
void TIM5_Init(void);	// Ammount counter
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
  MX_TouchGFX_Init();
  /* USER CODE BEGIN 2 */
  TIM3_Init();	// encoder
  TIM4_Init();	// forwarder for other timers
  TIM1_Init();	// RLY1
  TIM12_Init();	// RLY2
  TIM8_Init();	// RLY3
  TIM5_Init();  // Ammount counter
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
  qhStatusReport = xQueueCreate(5, sizeof(qPackage_statusReport_t)); // can be called from 5 higher priority tasks
  qhGUItoWriteSettings = xQueueCreate(1, sizeof(qPackage_settings_t));
  qhSettingsToGUI = xQueueCreate(1,sizeof(qPackage_settings_t));

  ehEvents = xEventGroupCreate();
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
  xTaskCreate(singleEventTask, "singleEventTask", 256, NULL, 3+2, &thSingleEvent);
  xTaskCreate(writeSettingsTask, "writeSettingsTask", 256, NULL,3+2 , &thWriteSettings);


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
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC|RCC_PERIPHCLK_I2C1
                              |RCC_PERIPHCLK_I2C3;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 384;
  PeriphClkInitStruct.PLLSAI.PLLSAIR = 5;
  PeriphClkInitStruct.PLLSAI.PLLSAIQ = 2;
  PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV2;
  PeriphClkInitStruct.PLLSAIDivQ = 1;
  PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_8;
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
  HAL_GPIO_WritePin(LCD_DISP_GPIO_Port, LCD_DISP_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(TRIGGER_OUT_GPIO_Port, TRIGGER_OUT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LCD_BL_CTRL_Pin */
  GPIO_InitStruct.Pin = LCD_BL_CTRL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_BL_CTRL_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_DISP_Pin */
  GPIO_InitStruct.Pin = LCD_DISP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_DISP_GPIO_Port, &GPIO_InitStruct);

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

  /*Configure GPIO pin : TRIGGER_OUT_Pin */
  GPIO_InitStruct.Pin = TRIGGER_OUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(TRIGGER_OUT_GPIO_Port, &GPIO_InitStruct);

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

}

/* USER CODE BEGIN 4 */

/*
 * Init TIM3 timer to be used as encoder
 */
void TIM3_Init(void){
	// activate clock for peripheral TIM3
	RCC->APB1ENR |= 0b1 << RCC_APB1ENR_TIM3EN_Pos;
	// Master mode selection: Update Event as trigger output (TRGO)
	TIM3->CR2 = 0b010 << TIM_CR2_MMS_Pos;
	// Slave mode selection: Encoder mode 2 - Counter counts up/down on TI2FP2 (PC7) edge depending on TI1FP1 (PC6) level.
	TIM3->SMCR = 0b010 << TIM_SMCR_SMS_Pos;
	// Capture/Compare 2 & 1 Selection: CC2 & CC1 channels are configured as input, IC2/IC1 is mapped on TI2/TI1
	TIM3->CCMR1 = (0b01 << TIM_CCMR1_CC2S_Pos) | (0b01 << TIM_CCMR1_CC1S_Pos);
	// Capture/Compare 2 output polarity: inverted/falling edge.
	TIM3->CCER = 0b1 << TIM_CCER_CC2P_Pos;

	// TODO: Add possibility to inverse encoder direction
}



/*
 * Init TIM4 timer to be used as trigger timer for TIM1, TIM8, TIM12
 */
void TIM4_Init(void){
	// activate clock for peripheral TIM4
	RCC->APB1ENR |= 0b1 << RCC_APB1ENR_TIM4EN_Pos;
	// One pulse mode
	TIM4->CR1 = 0b1 << TIM_CR1_OPM_Pos;
	// Master mode selection: CNT_EN as trigger output (TRGO)
	TIM4->CR2 = 0b001 << TIM_CR2_MMS_Pos;
	// Trigger selection: ITR2 (TIM3), Slave mode selection: Trigger mode - the counter starts on rising edge of TRGI
	TIM4->SMCR = 0b110 << TIM_SMCR_SMS_Pos | 0b010 << TIM_SMCR_TS_Pos;
	// Let it stop immediately.
	TIM4->ARR = 1;

}

/*
 * Init TIM1 timer to control RLY1
 */
void TIM1_Init(void){
	// activate clock for peripheral TIM1
	RCC->APB2ENR |= 0b1 << RCC_APB2ENR_TIM1EN_Pos;
	// Lower its frequency from 100 MHz to 1.525879 kHz
	TIM1->PSC = 0xFFFF;
	// One pulse mode, ARR pre-load active: UG event needs to be issued after updating. This ensures timer does not run past ARR should ARR be decreased just being reached
	TIM1->CR1 = 0b1 << TIM_CR1_OPM_Pos | 0b1 << TIM_CR1_ARPE_Pos;
	// Slave mode selection: Trigger mode - counter starts counting on rising edge of TRGI, TS is TIM4
	TIM1->SMCR = 0b110 << TIM_SMCR_SMS_Pos | 0b011 << TIM_SMCR_TS_Pos;


	/* DEBUG ONLY: Configure the TIM1 IRQ priority */
	//HAL_NVIC_SetPriority(TIM1_TRG_COM_TIM11_IRQn, 2 ,0);

	/* Enable the TIM1 global Interrupt */
	//HAL_NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn);
	// Interrupt generation on trigger input
	//TIM1->DIER = 0b1 << TIM_DIER_TIE_Pos;


	// Master enable output MOE, Off-state selection for Run and Idle mode: Inactive level when in off state
	TIM1->BDTR = 0b1 << TIM_BDTR_MOE_Pos;
	// force inactive level: PWM Mode 2 on channel 1 set in relaySetupTask when relay activated, Output Compare 1 pre-load enable
	TIM1->CCMR1 = 0b100 << TIM_CCMR1_OC1M_Pos | 0b1 << TIM_CCMR1_OC1PE_Pos;


	// Set ARR and CC1 to max
	TIM1->ARR = 0xFFFF;
	TIM1->CCR1 = 0xFFFF;
	// Update event generation as ARR and CCR1 are preloaded
	TIM1->EGR = 0b1 << TIM_EGR_UG_Pos;
	// Output Compare 1 output enable
	TIM1->CCER = 0b1 << TIM_CCER_CC1E_Pos;


}

/*
 * Init TIM12 timer to control RLY2
 */
void TIM12_Init(void){
	// activate clock for peripheral TIM12
	RCC->APB1ENR |= 0b1 << RCC_APB1ENR_TIM12EN_Pos;
	// Lower its frequency from 100 MHz to 1.525879 kHz
	TIM12->PSC = 0xFFFF;
	// One pulse mode, ARR pre-load active: UG event needs to be issued after updating. This ensures timer does not run past ARR should ARR be decreased just being reached
	TIM12->CR1 = 0b1 << TIM_CR1_OPM_Pos | 0b1 << TIM_CR1_ARPE_Pos;
	// Slave mode selection: Trigger mode - counter starts counting on rising edge of TRGI, TS is TIM4
	TIM12->SMCR = 0b110 << TIM_SMCR_SMS_Pos | 0b000 << TIM_SMCR_TS_Pos;
	// Master enable output MOE
	TIM12->BDTR = 0b1 << TIM_BDTR_MOE_Pos;
	// force inactive level: PWM Mode 2 on channel 1 set in relaySetupTask when relay activated, Output Compare 1 pre-load enable
	TIM12->CCMR1 = 0b100 << TIM_CCMR1_OC1M_Pos | 0b1 << TIM_CCMR1_OC1PE_Pos;
	// Set ARR and CC1 to max
	TIM12->ARR = 0xFFFF;
	TIM12->CCR1 = 0xFFFF;
	// Update event generation as ARR and CCR1 are preloaded
	TIM12->EGR = 0b1 << TIM_EGR_UG_Pos;
	// Output Compare 1 output enable
	TIM12->CCER = 0b1 << TIM_CCER_CC1E_Pos;
}

/*
 * Init TIM8 timer to control RLY3
 */
void TIM8_Init(void){
	// activate clock for peripheral TIM8
	RCC->APB2ENR |= 0b1 << RCC_APB2ENR_TIM8EN_Pos;
	// Lower its frequency from 100 MHz to 1.525879 kHz
	TIM8->PSC = 0xFFFF;
	// One pulse mode, ARR pre-load active: UG event needs to be issued after updating. This ensures timer does not run past ARR should ARR be decreased just being reached
	TIM8->CR1 = 0b1 << TIM_CR1_OPM_Pos | 0b1 << TIM_CR1_ARPE_Pos;
	// Slave mode selection: Trigger mode - counter starts counting on rising edge of TRGI, TS is TIM4
	TIM8->SMCR = 0b110 << TIM_SMCR_SMS_Pos | 0b010 << TIM_SMCR_TS_Pos;
	// Master enable output MOE
	TIM8->BDTR = 0b1 << TIM_BDTR_MOE_Pos;
	// force inactive level: PWM Mode 2 on channel 1 set in relaySetupTask when relay activated, Output Compare 1 pre-load enable
	TIM8->CCMR2 = 0b100 << TIM_CCMR2_OC3M_Pos | 0b1 << TIM_CCMR2_OC3PE_Pos;
	// Set ARR and CC3 to max
	TIM8->ARR = 0xFFFF;
	TIM8->CCR3 = 0xFFFF;
	// Update event generation as ARR and CCR1 are preloaded
	TIM8->EGR = 0b1 << TIM_EGR_UG_Pos;
	// Output Compare 1 output enable
	TIM8->CCER = 0b1 << TIM_CCER_CC3NE_Pos;

}


/*
 * Init TIM5 timer to count ammount
 */
void TIM5_Init(void){
	// activate clock for peripheral TIM5
	RCC->APB1ENR |= 0b1 << RCC_APB1ENR_TIM5EN_Pos;

	/* DEBUG ONLY: Configure the TIM1 IRQ priority */
	HAL_NVIC_SetPriority(TIM5_IRQn, 2 ,0);

	/* Enable the TIM1 global Interrupt */
	HAL_NVIC_EnableIRQ(TIM5_IRQn);
	// Interrupt generation on trigger input
	TIM5->DIER = 0b1 << TIM_DIER_TIE_Pos;

	// SMS: External clock mode 1, TS: Internal trigger 2: TIM4
	TIM5->SMCR = 0b010 << TIM_SMCR_TS_Pos | 0b111 << TIM_SMCR_SMS_Pos;

	// Counter Enable
	TIM5->CR1 = 0b1 << TIM_CR1_CEN_Pos;

}


/*
 * Send <nr> of bytes to EEPROM over I2C1. Blocking function
 * */
enum eepromStatus_t bytesWriteToEEPROM(uint8_t dataAddr, uint8_t *srcBuffer, uint8_t nr){
	static uint32_t lastWriteTimestamp = 0;
	//	Wait for 5ms after last write
	while(lastWriteTimestamp + 5 >= HAL_GetTick()){
	}
	uint32_t startTime = HAL_GetTick();
	//  Wait if busy
	while(I2C1->ISR & I2C_ISR_BUSY_Msk) {
		if(startTime + 100 < HAL_GetTick()) {
			I2C1->CR1 &= ~I2C_CR1_PE;
			for(uint8_t i = 0; i<10; i++){
				asm("NOP");
			}
			I2C1->CR1 |= I2C_CR1_PE;
			return EEPROM_BUSY;
		}
	}
	uint32_t icr = I2C1->ICR;
	uint32_t tries = 0;
	I2C1->ICR = I2C_ICR_NACKCF_Msk;
	// 	AUTOEND: STOP is sent after NBYTES are transferred; NBYTES: data address + data; SADD: slave address; WRITE = 0
	//I2C1->CR2 = I2C_CR2_AUTOEND | (uint8_t) (nr+1) << I2C_CR2_NBYTES_Pos | EEPROM_ADDR << I2C_CR2_SADD_Pos;
	while(1){
		tries++;
		//	Send START, AUTOEND: STOP is sent after NBYTES are transferred; NBYTES: data address + data; SADD: slave address; WRITE = 0
		I2C1->CR2 = I2C_CR2_START | I2C_CR2_AUTOEND | (uint8_t) (nr+1) << I2C_CR2_NBYTES_Pos | EEPROM_ADDR << I2C_CR2_SADD_Pos;
		//	Wait until address + write bit transmission is finished to check if EEPROM has acknoledged
		while(I2C1->CR2 & I2C_CR2_START_Msk){
			if(startTime + 100 < HAL_GetTick()) {
				return EEPROM_TIMEOUT_1;
			}
		}
		//	Wait for TXIS (ack received) or NACK (nack received)
		while(1){
			if((I2C1->ISR & I2C_ISR_TXIS_Msk) || (I2C1->ISR & I2C_ISR_NACKF_Msk)){
				break;
			}
		}

		//	If NACK is not set (ACK was received) break out of loop
		if(!(I2C1->ISR & I2C_ISR_NACKF_Msk)){
			break;
		}
		// 	Clear NACK & STOP flag for the next attempt
		I2C1->ICR = I2C_ICR_NACKCF_Msk | I2C_ICR_STOPCF_Msk;
		HAL_Delay(1);
	}
	// 	Transfer data address
	I2C1->TXDR = (uint32_t)dataAddr;

	/* Transfer data */
	for(uint8_t i = 0; i < nr; i++){
		//	Wait if next byte needs to be written to TXDR
			while(!(I2C1->ISR & I2C_ISR_TXIS_Msk)){
				if(startTime + 100 < HAL_GetTick()) {
					return EEPROM_TIMEOUT_2;
				}
			}
			// 	Transfer data
			I2C1->TXDR = (uint32_t)srcBuffer[i];
	}

	uint32_t blabla = 0;
	// 	Wait if STOPF is not set
	while(!(I2C1->ISR & I2C_ISR_STOPF_Msk)){
		blabla++;
		if(startTime + 100 < HAL_GetTick()) {
			return EEPROM_TIMEOUT_3;
		}
	}
	//	Clear STOP flag
	I2C1->ICR = I2C_ICR_STOPCF_Msk;
	I2C1->CR2 = 0;

	//HAL_Delay(10);
	//	Timestamp last write
	lastWriteTimestamp = HAL_GetTick();
	return EEPROM_SUCCESS;

}

/*
 * Set EEPROM to <dataAddr> and read <nr> of bytes from EEPROM and load it to <*dstBuffer>. Blocking function (max 100ms).
 * */
enum eepromStatus_t bytesReadFromEEPROM(uint8_t dataAddr, uint8_t *dstBuffer, uint8_t nr){
	uint32_t startTime = HAL_GetTick();
	//  Wait if busy
	while(I2C1->ISR & I2C_ISR_BUSY_Msk) {
		if(startTime + 100 < HAL_GetTick()) {
			I2C1->CR1 &= ~I2C_CR1_PE;
			for(uint8_t i = 0; i<10; i++){
				asm("NOP");
			}
			I2C1->CR1 |= I2C_CR1_PE;
			return EEPROM_BUSY;
		}
	}

	/* Has EEPROM finished with previous write cycle?*/
	/* Set data address */

	//I2C1->CR2 = (uint8_t) 1 << I2C_CR2_NBYTES_Pos | EEPROM_ADDR << I2C_CR2_SADD_Pos;
	uint32_t nrOfAttemtps = 0;
	while(1){
		nrOfAttemtps++;
		//	Send START; NBYTES: data address; SADD: slave address; WRITE = 0
		I2C1->CR2 = I2C_CR2_START | (uint8_t) 1 << I2C_CR2_NBYTES_Pos | EEPROM_ADDR << I2C_CR2_SADD_Pos;
		//	Wait until address + write bit is sent to check if EEPROM has acknoledged
		while(I2C1->CR2 & I2C_CR2_START_Msk){
			if(startTime + 100 < HAL_GetTick()) {
				return EEPROM_TIMEOUT_1;
			}
		}
		//	If NACK is not set (ACK was received) break out of loop
		if(!(I2C1->ISR & I2C_ISR_NACKF_Msk)){
			break;
		}
		// 	Clear NACK & STOP flag for the next attempt
		I2C1->ICR = I2C_ICR_NACKCF_Msk | I2C_ICR_STOPCF_Msk;
		HAL_Delay(1);
	}
//	//SET
//	TRIGGER_OUT_GPIO_Port->BSRR |= TRIGGER_OUT_Pin;
//	//RESET
//	TRIGGER_OUT_GPIO_Port->BSRR |= (TRIGGER_OUT_Pin) << 16;

	//	Send address
	I2C1->TXDR = (uint32_t)dataAddr;

	// 	Wait if TC bit is not set
	while(!(I2C1->ISR & I2C_ISR_TC_Msk)){
		if(startTime + 100 < HAL_GetTick()) {
			return EEPROM_TIMEOUT_2;
		}
	}
	/* Read bytes */
	//  Send START;	AUTOEND: STOP is sent after NBYTES are transferred; NBYTES: data; RD_WRN: read opearation; SADD: slave address;
//	I2C1->CR2 = I2C_CR2_AUTOEND | nr << I2C_CR2_NBYTES_Pos | I2C_CR2_RD_WRN | EEPROM_ADDR << I2C_CR2_SADD_Pos;
	I2C1->ICR = I2C_ICR_STOPCF_Msk;
	I2C1->CR2 = I2C_CR2_START | I2C_CR2_AUTOEND | nr << I2C_CR2_NBYTES_Pos | I2C_CR2_RD_WRN | EEPROM_ADDR << I2C_CR2_SADD_Pos;
	/* Have NBYTES received */
	for(uint8_t i = 0; i < nr; i++){
		// Wait if receive buffer is not not-empty = wait if receive buffer not full
		while(!(I2C1->ISR & I2C_ISR_RXNE)){
			if(startTime + 100 < HAL_GetTick()){
				return EEPROM_TIMEOUT_3;
			}
		}
		dstBuffer[i] = I2C1->RXDR;
	}

//	// 	Wait if TC bit is not set
//	while(!(I2C1->ISR & I2C_ISR_TC_Msk)){
//		if(startTime + 100 < HAL_GetTick()) {
//			return EEPROM_TIMEOUT;
//		}
//	}
//	if(I2C1->ISR & I2C_ISR_STOPF_Msk){
//		// should not be set yet
//		uint32_t isr_early = I2C1->ISR;
//		UNUSED(isr_early);
//		uint32_t smt;
//		UNUSED(smt);
//		return EEPROM_ERR;
//	}
//
//	// Stop the transfer now
//	I2C1->CR2 = I2C_CR2_STOP;

	uint32_t blabla = 0;
	// 	Wait if STOPF is not set
	while(1){
		if(I2C1->ISR & I2C_ISR_STOPF_Msk){
			break;
		}
		if(startTime + 100 < HAL_GetTick()) {
			return EEPROM_TIMEOUT_4;
		}
		blabla++;
	}
	//	Clear STOP
	I2C1->ICR = I2C_ICR_STOPCF_Msk;
	I2C1->CR2 = 0;

//	if(I2C1->ISR & I2C_ISR_BUSY_Msk) {
//			uint32_t isr = I2C1->ISR;
//			UNUSED(isr);
//			return EEPROM_BUSY;
//		}
	return EEPROM_SUCCESS;
}

enum eepromStatus_t getResolutionRadiusFromEEPROM(uint16_t* resolution, uint16_t* radius_01mm){
	enum eepromStatus_t status = EEPROM_SUCCESS;
	uint8_t dstBuffer[] = {0,0,0,0};
	uint8_t nr = sizeof(dstBuffer)/sizeof(dstBuffer[0]);

	status = bytesReadFromEEPROM((uint8_t)RESOLUTION_RADIUS, dstBuffer, nr);
	if(status == EEPROM_SUCCESS){
		//	Save resolution, radius data and entryIdx
		*resolution = (uint16_t) (dstBuffer[0] << 8 | dstBuffer[1]);
		*radius_01mm= (uint16_t) (dstBuffer[2] << 8 | dstBuffer[3]);
	}
	return status;
}

enum eepromStatus_t getSettingsFromEEPROM(uint16_t* resolution, uint16_t* radius_01mm, uint32_t* setLength_01mm, uint32_t *relayData[6]){
	enum eepromStatus_t status = EEPROM_SUCCESS;
	uint8_t dstBuffer[32];
	uint8_t nr = sizeof(dstBuffer)/sizeof(dstBuffer[0]);

	status = bytesReadFromEEPROM((uint8_t)RESOLUTION_RADIUS, dstBuffer, nr);
	if(status == EEPROM_SUCCESS){
		//	Save resolution, radius data and setLength
		*resolution 	= (uint16_t) (dstBuffer[0] << 8 | dstBuffer[1]);
		*radius_01mm	= (uint16_t) (dstBuffer[2] << 8 | dstBuffer[3]);
		*setLength_01mm = (uint32_t) (dstBuffer[4] << 8 | dstBuffer[5] << 16 | dstBuffer[6] << 8 | dstBuffer[7]);
		//	Save relay data to an array of pointers
		for(int8_t i = 0; i < 6; i++){
			*relayData[i]	= (uint32_t) (dstBuffer[8+i*4] << 8 | dstBuffer[9+i*4] << 16 | dstBuffer[10+i*4] << 8 | dstBuffer[11+i*4]);
		}

	}
	return status;
}


enum eepromStatus_t saveResolutionRadiusToEEPROM(uint16_t* resolution, uint16_t* radius_01mm){
	enum eepromStatus_t status = EEPROM_SUCCESS;
	uint8_t srcBuffer[4];
	uint8_t nr = sizeof(srcBuffer)/sizeof(srcBuffer[0]);

	srcBuffer[0] = (uint8_t) (*resolution >> 8);
	srcBuffer[1] = (uint8_t) (*resolution);
	srcBuffer[2] = (uint8_t) (*radius_01mm >> 8);
	srcBuffer[3] = (uint8_t) (*radius_01mm);

	status = bytesWriteToEEPROM((uint8_t)RESOLUTION_RADIUS, srcBuffer, nr);

	return status;
}

enum eepromStatus_t saveSetLengthToEEPROM(uint32_t* setLength){
	enum eepromStatus_t status = EEPROM_SUCCESS;
	uint8_t srcBuffer[4];
	uint8_t nr = sizeof(srcBuffer)/sizeof(srcBuffer[0]);

	srcBuffer[0] = (uint8_t) (*setLength >> 24);
	srcBuffer[1] = (uint8_t) (*setLength >> 16);
	srcBuffer[2] = (uint8_t) (*setLength >> 8);
	srcBuffer[3] = (uint8_t) (*setLength >> 0);

	status = bytesWriteToEEPROM((uint8_t)SET_LENGTH, srcBuffer, nr);

	return status;
}

enum eepromStatus_t saveRelayDataToEEPROM(uint32_t* duration, uint32_t* delay, uint8_t relayIdx){
	enum eepromStatus_t status = EEPROM_SUCCESS;
	uint8_t srcBuffer[8];
	uint8_t nr = sizeof(srcBuffer)/sizeof(srcBuffer[0]);

	srcBuffer[0] = (uint8_t) (*duration >> 24);
	srcBuffer[1] = (uint8_t) (*duration >> 16);
	srcBuffer[2] = (uint8_t) (*duration >> 8);
	srcBuffer[3] = (uint8_t) (*duration >> 0);
	srcBuffer[4] = (uint8_t) (*delay >> 24);
	srcBuffer[5] = (uint8_t) (*delay >> 16);
	srcBuffer[6] = (uint8_t) (*delay >> 8);
	srcBuffer[7] = (uint8_t) (*delay >> 0);

	status = bytesWriteToEEPROM((uint8_t)RLY1_DUR + 8*(relayIdx-1), srcBuffer, nr);

	return status;
}

//enum eepromStatus_t getLatestEntryFromEEPROM(uint8_t *latestEntryIdx, uint16_t *resolution, uint16_t *radius_01mm){
//	enum eepromStatus_t status = EEPROM_SUCCESS;
//	uint8_t dstBuffer[] = {0,0,0,0};
//	uint8_t nr = sizeof(dstBuffer)/sizeof(dstBuffer[0]);
//
//	uint8_t largestEntryIdx = 0;
//	uint8_t entryIdx = 0;
//	uint32_t memoryValue = 0;
//
//	//	Iterate through all 4 memory locations
//	for(uint8_t i = 0; i < 4; i++ ){
//		status = bytesReadFromEEPROM(i*OFFSET+RES_ENTRYIDX_RADIUS_ENTRYVALID, dstBuffer, nr);
//		if(status != EEPROM_SUCCESS){
//			return status;
//		}else{
//			//  Reconstruct 4 byte memory data
//			memoryValue = dstBuffer[0] << 24 | dstBuffer[1] << 16 | dstBuffer[2] << 8 | dstBuffer[3];
//			//  Is data entry valid?
//			if(memoryValue & ENTRY_VALID_Msk){
//				//	Is this the most recent entry
//				entryIdx = (memoryValue & ENTRY_IDX_Msk) >> ENTRY_IDX_Pos;
//				if( entryIdx >= largestEntryIdx ){
//					//	Save resolution, radius data and entryIdx
//					*resolution = (memoryValue & RESOLUTION_Msk) >> RESOLUTION_Pos;
//					*radius_01mm= (memoryValue & RADIUS_Msk) >> RADIUS_Pos;
//					*latestEntryIdx = entryIdx;
//					//	Set a new largest entry idx
//					largestEntryIdx = entryIdx;
//				}
//			}
//
//		}
//
//	}
//	/* When writing, if last read location was 0b11, then erase whole memory */
//	return status;
//}




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
	UBaseType_t basePriority = uxTaskPriorityGet( NULL );
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

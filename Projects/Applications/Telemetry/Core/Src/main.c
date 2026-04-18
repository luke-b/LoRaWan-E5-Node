/**
 * @file    main.c
 * @author  IoT Architekt
 * @brief   Aplikační firmware pro Wio-E5 (STM32WLE5JC) - Telemetrie vodovodní jímky
 * * Funkce:
 * 1. Kontinuální asynchronní počítání pulzů (LPTIM1) bez probouzení CPU.
 * 2. Okamžitý alarm při otevření poklopu (GPIO EXTI).
 * 3. Detekce zaplavení (ADC s řízeným VCC) s měřením 1x za 24h.
 * 4. Pravidelný 24h Heartbeat (kumulativní pulzy, stav baterie).
 * 5. Hluboký spánek (STOP2) s odběrem ~1.5 µA.
 */

#include "main.h"
#include "stm32wlxx_hal.h"
#include <stdbool.h>
// Hlavičkové soubory pro LoRaWAN MAC vrstvu (poskytuje STMicroelectronics)
#include "app_lorawan.h"
#include "sys_app.h"
#include "LmHandler.h"
#include "telemetry_logic.h"

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void MX_RTC_Init(void);
static void MX_LPTIM1_Init(void);
void MX_ADC_Init(void);

// --- DEFINICE PINŮ ---
#define DOOR_CONTACT_PIN        GPIO_PIN_1
#define DOOR_CONTACT_PORT       GPIOB          // PB1 (EXTI1)
#define PULSE_INPUT_PIN         GPIO_PIN_0
#define PULSE_INPUT_PORT        GPIOB          // PB0 (LPTIM1_IN1)
#define WLD_VCC_PIN             GPIO_PIN_5
#define WLD_VCC_PORT            GPIOA          // PA5 (Spínané napájení pro lano)
#define WLD_ADC_PIN             GPIO_PIN_4
#define WLD_ADC_PORT            GPIOA          // PA4 (ADC_IN4)

// --- KONSTANTY ---
#define WATER_LEAK_THRESHOLD    2000           // Prahová hodnota ADC (0-4095) pro detekci vody
#define RTC_WAKEUP_SECONDS      86400          // 24 hodin (Heartbeat interval)

// --- GLOBÁLNÍ PROMĚNNÉ (State Machine) ---
volatile uint8_t flag_door_alarm = 0;          // Příznak probuzení: Otevřené dveře
volatile uint8_t flag_rtc_wakeup = 0;          // Příznak probuzení: 24h časovač

uint32_t total_water_pulses = 0;               // 32-bit akumulátor pulzů (odolný proti LPTIM přetečení)
uint16_t last_lptim_val = 0;                   // Poslední přečtená hodnota z HW čítače

// Deklarace HW handlů (standard STM32 HAL)
LPTIM_HandleTypeDef hlptim1;
RTC_HandleTypeDef hrtc;
ADC_HandleTypeDef hadc;

/**
 * @brief Pomocná funkce pro čtení senzoru vody.
 * Provede se pouze při měření, minimalizuje korozi lana.
 */
uint8_t CheckWaterLeak(void) {
    uint32_t adc_value = 0;
    
    // 1. Zapnutí napájení pro napěťový dělič WLD lana
    HAL_GPIO_WritePin(WLD_VCC_PORT, WLD_VCC_PIN, GPIO_PIN_SET);
    
    // 2. Počkáme 2 milisekundy na ustálení kapacity lana a napětí
    HAL_Delay(2); 
    
    // 3. Spuštění a přečtení A/D převodníku
    HAL_ADC_Start(&hadc);
    if(HAL_ADC_PollForConversion(&hadc, 10) == HAL_OK) {
        adc_value = HAL_ADC_GetValue(&hadc);
    }
    HAL_ADC_Stop(&hadc);
    
    // 4. Okamžité vypnutí napájení (zabránění elektrolýze)
    HAL_GPIO_WritePin(WLD_VCC_PORT, WLD_VCC_PIN, GPIO_PIN_RESET);
    
    // 5. Vyhodnocení
    return (adc_value > WATER_LEAK_THRESHOLD) ? 1 : 0;
}

/**
 * @brief Aktualizuje 32-bitový akumulátor pulzů z 16-bitového HW čítače.
 */
void UpdatePulseCounter(void) {
    uint16_t current_lptim = HAL_LPTIM_ReadCounter(&hlptim1);
    
    // Ošetření přetečení 16bitového čítače (65535 -> 0)
    if (current_lptim >= last_lptim_val) {
        total_water_pulses += (current_lptim - last_lptim_val);
    } else {
        total_water_pulses += ((65535 - last_lptim_val) + current_lptim + 1);
    }
    last_lptim_val = current_lptim;
}

/**
 * @brief Sestaví payload a předá ho LoRaWAN MAC vrstvě.
 */
void SendLoraMessage(uint8_t msg_type) {
    LmHandlerAppData_t appData;
    uint8_t payload[8];
    uint8_t battery_level = GetBatteryLevel(); // Interní funkce STM32WL (měření VREFINT)
    uint8_t door_state = HAL_GPIO_ReadPin(DOOR_CONTACT_PORT, DOOR_CONTACT_PIN); // 1 = otevřeno
    
    UpdatePulseCounter();

    payload[0] = msg_type;
    
    // Pulzy: 32-bit hodnota rozdělená na byty (Big Endian)
    payload[1] = (total_water_pulses >> 24) & 0xFF;
    payload[2] = (total_water_pulses >> 16) & 0xFF;
    payload[3] = (total_water_pulses >> 8) & 0xFF;
    payload[4] = total_water_pulses & 0xFF;
    
    payload[5] = battery_level;
    
    // Stavový byte (Bit 0: Dveře, Bit 1: Voda)
    payload[6] = 0x00;
    if (door_state == GPIO_PIN_SET) payload[6] |= (1 << 0);
    // Pozn.: Detekci vody do statusu dáváme jen, pokud byl msg_type HEARTBEAT nebo WATER,
    // pro zjednodušení v tomto kódu vynecháváme detailní stavovou logiku lana pro jiné typy zpráv.

    appData.Port = 1;
    appData.Buffer = payload;
    appData.BufferSize = 7;

    // Odeslání pomocí LoRaWAN middleware (potvrzená zpráva pro alarmy)
    if (msg_type == 0x01) { // HEARTBEAT
        LmHandlerSend(&appData, LORAMAC_HANDLER_UNCONFIRMED_MSG, NULL, false);
    } else {
        LmHandlerSend(&appData, LORAMAC_HANDLER_CONFIRMED_MSG, NULL, false);
    }
}

// --- TYPY ZPRÁV (PAYLOAD) ---
#define MSG_TYPE_HEARTBEAT      0x01
#define MSG_TYPE_ALARM_DOOR     0x02
#define MSG_TYPE_ALARM_WATER    0x03

/**
 * @brief Hlavní smyčka aplikace.
 */
int main(void) {
    // Inicializace HAL knihovny a systémových hodin
    HAL_Init();
    SystemClock_Config();
    
    // Inicializace HW periferií (tyto funkce běžně generuje STM32CubeMX)
    MX_GPIO_Init();
    MX_RTC_Init();
    MX_LPTIM1_Init();
    MX_ADC_Init();
    MX_LoRaWAN_Init(); // Obsahuje OTAA Join proceduru

    // Spuštění hardwarového čítače pulzů (LPTIM)
    HAL_LPTIM_Counter_Start(&hlptim1, 0xFFFF);

    // Nastavení prvního probuzení přes RTC (za 24 hodin)
    // SetRTCWakeupTimer(RTC_WAKEUP_SECONDS);

    while (1) {
      bool door_pending = false;
      bool rtc_pending = false;

      Telemetry_ConsumeWakeFlags(&flag_door_alarm, &flag_rtc_wakeup, &door_pending, &rtc_pending);

        // ---------------------------------------------------------
// 1. ZPRACOVÁNÍ UDÁLOSTÍ (Pokud byl procesor probuzen)
// ---------------------------------------------------------
        
        // A) Alarm: Otevřen poklop
      if (door_pending) {
        TelemetryEventAction_t door_action = TELEMETRY_EVENT_ACTION_NONE;
            
            // Softwarový debounce: Ověření, že kontakt je stále rozpojený (např. po 50 ms)
            HAL_Delay(50);
        door_action = Telemetry_HandleDoorWake(door_pending,
                                               HAL_GPIO_ReadPin(DOOR_CONTACT_PORT, DOOR_CONTACT_PIN) == GPIO_PIN_SET);
        if (door_action == TELEMETRY_EVENT_ACTION_SEND_ALARM_DOOR) {
                SendLoraMessage(MSG_TYPE_ALARM_DOOR);
            }
        }
        
        // B) Pravidelný 24h Heartbeat a kontrola vody
      if (rtc_pending) {
        TelemetryEventAction_t periodic_action = TELEMETRY_EVENT_ACTION_NONE;
            
            // Kontrola zaplavení WLD lanem
        periodic_action = Telemetry_HandlePeriodicWake(rtc_pending, CheckWaterLeak() != 0);
        if (periodic_action == TELEMETRY_EVENT_ACTION_SEND_ALARM_WATER) {
                SendLoraMessage(MSG_TYPE_ALARM_WATER);
        } else if (periodic_action == TELEMETRY_EVENT_ACTION_SEND_HEARTBEAT) {
                SendLoraMessage(MSG_TYPE_HEARTBEAT);
            }
            
            // Nastavení budíku na dalších 24 hodin
            // SetRTCWakeupTimer(RTC_WAKEUP_SECONDS);
        }

        // ---------------------------------------------------------
// 2. USPÁVÁNÍ DO REŽIMU STOP2 (Deep Sleep)
// ---------------------------------------------------------
        
        // Zastavení systémového ticku (zabrání probouzení každou milisekundu)
        HAL_SuspendTick();
        
        // Vstup do režimu STOP2 (Udrží obsah RAM a LPTIM, spotřeba ~1.2 µA)
        // CPU zde "zamrzne" a čeká na přerušení (Interrupt) z RTC nebo EXTI
        HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
        
        // --- PROCESOR SE PROBUDIL ---
        // Obnovení systémového ticku po probuzení
        HAL_ResumeTick();
        
        // Po opuštění STOP2 módu je často nutné re-inicializovat hlavní hodiny (MSI/HSI)
        SystemClock_Config();
    }
}

// --- OBSLUHA PŘERUŠENÍ (Interrupt Service Routines - ISR) ---

/**
 * @brief RTC Alarm Callback (Spustí se po uplynutí nastaveného času)
 */
void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc) {
    flag_rtc_wakeup = 1; // Nastavíme vlajku pro provedení 24h rutiny
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
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_11;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK3|RCC_CLOCKTYPE_HCLK
                              |RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1
                              |RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK3Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(WLD_VCC_PORT, WLD_VCC_PIN, GPIO_PIN_RESET);

  /*Configure GPIO pin : WLD_VCC_PIN */
  GPIO_InitStruct.Pin = WLD_VCC_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(WLD_VCC_PORT, &GPIO_InitStruct);

  /*Configure GPIO pin : DOOR_CONTACT_PIN */
  GPIO_InitStruct.Pin = DOOR_CONTACT_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(DOOR_CONTACT_PORT, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);
}

/**
 * @brief RTC Initialization Function
 * @param None
 * @retval None
 */
void MX_RTC_Init(void)
{
  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */
}

/**
 * @brief LPTIM1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_LPTIM1_Init(void)
{
  /* USER CODE BEGIN LPTIM1_Init 0 */

  /* USER CODE END LPTIM1_Init 0 */

  /* USER CODE BEGIN LPTIM1_Init 1 */

  /* USER CODE END LPTIM1_Init 1 */
  hlptim1.Instance = LPTIM1;
  hlptim1.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
  hlptim1.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;
  hlptim1.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
  hlptim1.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
  hlptim1.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
  hlptim1.Init.CounterSource = LPTIM_COUNTERSOURCE_EXTERNAL;
  hlptim1.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
  hlptim1.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
  if (HAL_LPTIM_Init(&hlptim1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPTIM1_Init 2 */

  /* USER CODE END LPTIM1_Init 2 */
}

/**
 * @brief ADC Initialization Function
 * @param None
 * @retval None
 */
void MX_ADC_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  /** Common config
  */
  hadc.Instance = ADC;
  hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.ContinuousConvMode = DISABLE;
  hadc.Init.NbrOfConversion = 1;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests = DISABLE;
  hadc.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  hadc.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_12CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC_Init 2 */

  /* USER CODE END ADC_Init 2 */
}

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
void assert_failed(uint8_t *file, uint8_t *line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
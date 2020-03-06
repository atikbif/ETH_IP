/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2019 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "can.h"
#include "iwdg.h"
#include "lwip.h"
#include "rng.h"
#include "rtc.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <string.h>
#include "canLogger.h"
#include "fsmc_flash.h"
#include "eeprom.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

//#define FLASH_TEST	1

#define NAND_PAGE_SIZE          ((uint16_t)0x0800) /* 2 * 1024 bytes per page w/o Spare Area */
#define BUFFER_SIZE             (NAND_PAGE_SIZE)

#define	EEPROM_KEY_VALUE	0x3238

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

extern unsigned short bl_num;
extern unsigned short page_num;

extern NAND_HandleTypeDef hnand1;

uint16_t VirtAddVarTab[NB_OF_VAR]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

NAND_IDTypeDef NAND_ID;

#ifdef FLASH_TEST
NAND_AddressTypeDef NAND_Address;

static uint8_t TxBuffer[BUFFER_SIZE];
static uint8_t RxBuffer[BUFFER_SIZE];
static uint8_t spare[64];
uint16_t i=0,j=0;
uint16_t p_num=0;

#endif
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint8_t conf[64];	// буфер с настройками

static void EnableBKUPmem(void)
{
   RCC->APB1ENR |= RCC_APB1ENR_PWREN;
   /* Enable backup SRAM Clock */
   RCC->AHB1ENR |= RCC_AHB1ENR_BKPSRAMEN;
   /* Allow access to backup domain */
   HAL_PWR_EnableBkUpAccess();
   /* Enable the Backup SRAM low power Regulator */
   /* This will allow data to stay when using VBat mode */
   //PWR_BackupRegulatorCmd(ENABLE);
   *(__IO uint32_t *) CSR_BRE_BB = (uint32_t) ENABLE;
   /* Wait for backup regulator to be ready  */
   while (!(PWR->CSR & (PWR_FLAG_BRR)));
}

__attribute__((unused)) static void DisableBKUPmem(void)
{
   HAL_PWR_DisableBkUpAccess();
}

uint8_t read_ip_from_conf(uint8_t num) {
	if(num==1) {
		return (conf[11]-'0')*100 + (conf[12]-'0')*10 + (conf[13]-'0');
	}else if(num==2) {
		return (conf[14]-'0')*100 + (conf[15]-'0')*10 + (conf[16]-'0');
	}else if(num==3) {
		return (conf[17]-'0')*100 + (conf[18]-'0')*10 + (conf[19]-'0');
	}else if(num==4) {
		return (conf[20]-'0')*100 + (conf[21]-'0')*10 + (conf[22]-'0');
	}
	return 0;
}

uint8_t read_mask_from_conf(uint8_t num) {
	if(num==1) {
		return (conf[23]-'0')*100 + (conf[24]-'0')*10 + (conf[25]-'0');
	}else if(num==2) {
		return (conf[26]-'0')*100 + (conf[27]-'0')*10 + (conf[28]-'0');
	}else if(num==3) {
		return (conf[29]-'0')*100 + (conf[30]-'0')*10 + (conf[31]-'0');
	}else if(num==4) {
		return (conf[32]-'0')*100 + (conf[33]-'0')*10 + (conf[34]-'0');
	}
	return 0;
}

uint8_t read_gate_from_conf(uint8_t num) {
	if(num==1) {
		return (conf[35]-'0')*100 + (conf[36]-'0')*10 + (conf[37]-'0');
	}else if(num==2) {
		return (conf[38]-'0')*100 + (conf[39]-'0')*10 + (conf[40]-'0');
	}else if(num==3) {
		return (conf[41]-'0')*100 + (conf[42]-'0')*10 + (conf[43]-'0');
	}else if(num==4) {
		return (conf[44]-'0')*100 + (conf[45]-'0')*10 + (conf[46]-'0');
	}
	return 0;
}

uint16_t read_id_from_conf() {
	uint16_t res = 0;
	res = ((uint16_t)conf[47]-'0')*10000;
	res+= ((uint16_t)conf[48]-'0')*1000;
	res+= ((uint16_t)conf[49]-'0')*100;
	res+= ((uint16_t)conf[50]-'0')*10;
	res+= ((uint16_t)conf[51]-'0');
	return res;
}

void read_conf() {
	EnableBKUPmem();
	memcpy(conf,(uint32_t*) BKPSRAM_BASE, sizeof(conf));
	//DisableBKUPmem();
}

void write_conf() {
	EnableBKUPmem();
	memcpy((uint32_t*)BKPSRAM_BASE, conf, sizeof(conf));
	//DisableBKUPmem();
}

void set_default_conf() {
	// start key
	conf[0] = 0xA6;
	conf[1] = 0x4D;
	// version
	conf[2] = '1';
	conf[3] = '0';

	conf[4] = '1';// node 0 (PC21-1)
	conf[5] = '2';// node 1 (PC21-CD)
	conf[6] = '0';// node 2
	conf[7] = '3';// node 3 (PC21-MC)
	conf[8] = '0';// node 4
	conf[9] = '0';// node 5
	conf[10] = '0';// node 6

	// IP адрес
	conf[11] = '1';conf[12] = '9';conf[13] = '2';
	conf[14] = '1';conf[15] = '6';conf[16] = '8';
	conf[17] = '0';conf[18] = '0';conf[19] = '1';
	conf[20] = '0';conf[21] = '0';conf[22] = '2';

	// маска
	conf[23] = '2';conf[24] = '5';conf[25] = '5';
	conf[26] = '2';conf[27] = '5';conf[28] = '5';
	conf[29] = '2';conf[30] = '5';conf[31] = '5';
	conf[32] = '0';conf[33] = '0';conf[34] = '0';

	// шлюз
	conf[35] = '1';conf[36] = '9';conf[37] = '2';
	conf[38] = '1';conf[39] = '6';conf[40] = '8';
	conf[41] = '0';conf[42] = '0';conf[43] = '1';
	conf[44] = '0';conf[45] = '0';conf[46] = '1';



	conf[47]='0';conf[48]='0';conf[49]='0';conf[50]='0';conf[51]='0';

	conf[52] = 'C';conf[53]='F';conf[54]='7';conf[55]='7';

	write_conf();
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  uint16_t ee_key = 0;

  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  HAL_FLASH_Unlock();
  EE_Init();

  EE_ReadVariable(VirtAddVarTab[0],  &ee_key);
  if(ee_key==EEPROM_KEY_VALUE) {
  	EE_ReadVariable(VirtAddVarTab[1],  &ee_key);
  	if(ee_key!=2) EE_WriteVariable(VirtAddVarTab[1],2);
  }

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_RTC_Init();
  MX_CAN1_Init();
  //MX_IWDG_Init();
  MX_RNG_Init();
  MX_FSMC_Init();
  /* USER CODE BEGIN 2 */

  __enable_irq();
  HAL_NAND_Read_ID(&hnand1, &NAND_ID);


#ifdef FLASH_TEST

   uint16_t err = 0;
   bl_num = 0;
   p_num = 0;

   for(i=0;i<1024;i++) {
 	  NAND_Address.Plane = 0;
 	  NAND_Address.Block = i;
 	  NAND_Address.Page = p_num;
 	  HAL_NAND_Erase_Block(&hnand1, &NAND_Address);
   }

   /*NAND_Address.Plane = 0;
   NAND_Address.Block = 489;
   NAND_Address.Page = 0;
   for(j=0;j<64;j++) spare[j]=0;
   NAND_Address.Plane = 0;
   NAND_Address.Block = 489;
   NAND_Address.Page = j;
   HAL_NAND_Write_SpareArea_8b(&hnand1, &NAND_Address,spare,1);*/

   HAL_Delay(100);

   while(1) {
 	if(bl_num>=1024) break;
 	NAND_Address.Plane = 0;
 	NAND_Address.Block = bl_num;
 	NAND_Address.Page = p_num;
 	spare[0] = 0;
 	if(p_num==0) {
 		HAL_NAND_Read_SpareArea_8b(&hnand1, &NAND_Address, spare, 1);
 		if(spare[0]==0xFF) {
 			HAL_NAND_Erase_Block(&hnand1, &NAND_Address);
 		}else {
 			err++;
 			bl_num++;p_num=0;
 			continue;
 		}
 	}
 	for (uint32_t i = 0; i < BUFFER_SIZE; i++) {
 		TxBuffer[i] = i;
 		RxBuffer[i] = 0;
 	}
 	if(HAL_NAND_Write_Page(&hnand1, &NAND_Address, TxBuffer, 1)==HAL_TIMEOUT) {
 		hnand1.State = HAL_NAND_STATE_RESET;
 	}
 	HAL_NAND_Read_Page(&hnand1, &NAND_Address, RxBuffer, 1);


 	if(memcmp(TxBuffer, RxBuffer,2048) == 0 )  {
 		if(p_num==0) {HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);}
 	}else {
 		NAND_Address.Plane = 0;
 		NAND_Address.Block = bl_num;
 		NAND_Address.Page = 0;
 		HAL_NAND_Write_SpareArea_8b(&hnand1,&NAND_Address,spare,1);
 		bl_num++;
 		p_num=0;
 		err++;
 		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_7);

 	}
 	p_num++;
 	if(p_num>=64) {
 		p_num = 0;
 		bl_num++;
 	}

   }
   if(err) {
 	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6,GPIO_PIN_RESET);
 	  HAL_Delay(3000);
 	  for(uint32_t i = 0; i < err*2; i++) {
 		  HAL_Delay(1000);
 		  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
 	  }
 	  HAL_Delay(3000);
   }

#endif

  bl_num = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR2);
  page_num = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR3);

  if(bl_num>=1024) bl_num=0;
  if(page_num>=64) page_num=0;

  initLogger();

  read_conf();
  if(conf[0]!=0xA6 || conf[1]!=0x4D) set_default_conf();

  HAL_Delay(100);



  reset_flash();
  HAL_Delay(100);

  /* USER CODE END 2 */

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

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
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 256;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
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
	while(1);
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

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define STAT1_Pin GPIO_PIN_9
#define STAT1_GPIO_Port GPIOB
#define STAT1_EXTI_IRQn EXTI4_15_IRQn
#define CLK_TX_Pin GPIO_PIN_0
#define CLK_TX_GPIO_Port GPIOA
#define MOSI_IN_Pin GPIO_PIN_2
#define MOSI_IN_GPIO_Port GPIOA
#define SEL_SRC_Pin GPIO_PIN_3
#define SEL_SRC_GPIO_Port GPIOA
#define DATA_TX_Pin GPIO_PIN_4
#define DATA_TX_GPIO_Port GPIOA
#define CLK_IN_Pin GPIO_PIN_5
#define CLK_IN_GPIO_Port GPIOA
#define PI_MISO_Pin GPIO_PIN_6
#define PI_MISO_GPIO_Port GPIOA
#define SEL_READ_WRITE_Pin GPIO_PIN_1
#define SEL_READ_WRITE_GPIO_Port GPIOB
#define STAT2_Pin GPIO_PIN_8
#define STAT2_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

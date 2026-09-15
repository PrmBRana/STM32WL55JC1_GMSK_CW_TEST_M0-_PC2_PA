/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    radio_board_if.c
  * @author  MCD Application Team
  * @brief   This file provides an interface layer between MW and Radio Board
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
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
#include "radio_board_if.h"
#include "stm32wlxx_hal.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

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
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

static uint8_t s_pa_enabled = 1U;

void RBI_EnablePA(uint8_t enable)
{
    s_pa_enabled = enable ? 1U : 0U;
    if (s_pa_enabled)
    {
        HAL_GPIO_WritePin(AMP_3V3_EN_PORT, AMP_3V3_EN_PIN, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(AMP_3V3_EN_PORT, AMP_3V3_EN_PIN, GPIO_PIN_RESET);
    }
}

uint8_t RBI_IsPAEnabled(void)
{
    return s_pa_enabled;
}

/* Exported functions --------------------------------------------------------*/
int32_t RBI_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIOC clock in CPU1 & CPU2 domains (Power 4 is ON, PA8 not used) */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    RCC->AHB2ENR   |= (1UL << 2);
    RCC->C2AHB2ENR |= (1UL << 2);

    /* Configure 3.3V PA Enable (PC2), RF Switch FE_CTRL1 (PC4), RF Switch FE_CTRL2 (PC5) */
    GPIO_InitStruct.Pin   = AMP_3V3_EN_PIN | FE_CTRL1_PIN | FE_CTRL2_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* Default: All OFF (Standby: PA disabled, RF Switch unasserted) */
    HAL_GPIO_WritePin(GPIOC, AMP_3V3_EN_PIN | FE_CTRL1_PIN | FE_CTRL2_PIN, GPIO_PIN_RESET);

    return 0;
}

int32_t RBI_DeInit(void)
{
    /* Safe shutdown: deassert all controls */
    HAL_GPIO_WritePin(GPIOC, AMP_3V3_EN_PIN | FE_CTRL1_PIN | FE_CTRL2_PIN, GPIO_PIN_RESET);
    HAL_GPIO_DeInit(GPIOC, AMP_3V3_EN_PIN | FE_CTRL1_PIN | FE_CTRL2_PIN);

    return 0;
}

int32_t RBI_ConfigRFSwitch(RBI_Switch_TypeDef Config)
{
    /* Ensure GPIOC clock is active in CPU2 domain */
    RCC->C2AHB2ENR |= (1UL << 2);

    switch (Config)
    {
    case RBI_SWITCH_RFO_HP:
    case RBI_SWITCH_RFO_LP:
        /*
         * TRANSMIT MODE:
         * 1. RF Switch -> TX Path (FE_CTRL1 = 0, FE_CTRL2 = 1)
         * 2. Assert 3.3V External Power Amplifier Enable (PC2 / SO2 = 1)
         */
        HAL_GPIO_WritePin(FE_CTRL1_PORT, FE_CTRL1_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(FE_CTRL2_PORT, FE_CTRL2_PIN, GPIO_PIN_SET);

        if (s_pa_enabled)
        {
            HAL_GPIO_WritePin(AMP_3V3_EN_PORT, AMP_3V3_EN_PIN, GPIO_PIN_SET);
        }
        else
        {
            HAL_GPIO_WritePin(AMP_3V3_EN_PORT, AMP_3V3_EN_PIN, GPIO_PIN_RESET);
        }

        /* 3. Settle delay (~200 us) for PA bias network & RF switch to fully stabilize */
        for (volatile uint32_t i = 0; i < 2500; i++) { __NOP(); }
        break;

    case RBI_SWITCH_RX:
        /*
         * RECEIVE MODE:
         * 1. Disable 3.3V External PA (PC2 = 0)
         * 2. RF Switch -> RX Path (FE_CTRL1 = 1, FE_CTRL2 = 0)
         */
        HAL_GPIO_WritePin(AMP_3V3_EN_PORT, AMP_3V3_EN_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(FE_CTRL1_PORT, FE_CTRL1_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(FE_CTRL2_PORT, FE_CTRL2_PIN, GPIO_PIN_RESET);

        /* 3. Settle delay (~50 us) */
        for (volatile uint32_t i = 0; i < 500; i++) { __NOP(); }
        break;

    case RBI_SWITCH_OFF:
    default:
        /*
         * STANDBY / OFF:
         * Deassert 3.3V PA enable and both RF switch control pins
         */
        HAL_GPIO_WritePin(AMP_3V3_EN_PORT, AMP_3V3_EN_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(FE_CTRL1_PORT, FE_CTRL1_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(FE_CTRL2_PORT, FE_CTRL2_PIN, GPIO_PIN_RESET);
        break;
    }

    return 0;
}

int32_t RBI_GetTxConfig(void)
{
    /* Support both Low Power and High Power configurations */
    return RBI_CONF_RFO_LP_HP;
}

int32_t RBI_IsTCXO(void)
{
    return IS_TCXO_SUPPORTED;
}

int32_t RBI_IsDCDC(void)
{
    return IS_DCDC_SUPPORTED;
}

int32_t RBI_GetRFOMaxPowerConfig(RBI_RFOMaxPowerConfig_TypeDef Config)
{
  if (Config == RBI_RFO_LP_MAXPOWER)
  {
    return 14; /* 14 dBm: optimal 0x04 duty cycle on SX1262 RFO_LP */
  }
  else
  {
    return 22; /* 22 dBm */
  }
}
/* USER CODE BEGIN EF */

/* USER CODE END EF */

/* Private Functions Definition -----------------------------------------------*/
/* USER CODE BEGIN PrFD */

/* USER CODE END PrFD */

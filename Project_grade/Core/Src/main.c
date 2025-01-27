/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <wchar.h>
#include <stdio.h>
#include <wchar.h>
#include "stepper_motor.h"
#include "hagl.h"
#include "font6x9.h"
#include "rgb565.h"
#include "bh1750_config.h"
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
volatile _Bool USER_Button_state = 0;
GPIO_PinState LD1_State = GPIO_PIN_RESET;
/* USER CODE BEGIN PV */
int light =0;
int value = 0;
int mode = 0;
int pulse = 0;
uint8_t ster = 0;      // Control variable (1 = diode, 0 = stepper motor)
uint8_t engineR = 0;   // Stepper motor control (1 = closing, 0 = opening)
float currentAngle = 0;  // Stepper motor current position
int maxAngle = 875;    // Max rotation angle
int minAngle = 20;     // Min rotation angle
// PID constants (tune these values)
float Kp = 4.65;
float Ki = 3;
float Kd = 0.5;

// PID variables
float error = 0;
float previous_error = 0;
float integral = 0;
float derivative = 0;
float output = 0;
float dt = 1;  // Sampling time in seconds
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi == &hspi2)
	{
		lcd_transfer_done();
	}
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim==&htim7){

						light = BH1750_ReadIlluminance_lux(&hbh1750A);
					}
	if(htim==&htim6){

				hagl_fill_rectangle(30, 45, 80, 100, BLACK);
				value =10* __HAL_TIM_GET_COUNTER(&htim3);
				wchar_t str[10];
				swprintf(str, sizeof(str) / sizeof(wchar_t), L"%d", value);
				hagl_put_text(str, 40, 55, YELLOW, font6x9);
				//light = BH1750_ReadIlluminance_lux(&hbh1750A);
				wchar_t lig[10];
				swprintf(lig, sizeof(lig) / sizeof(wchar_t), L"%d", light);
				hagl_put_text(lig, 40, 85, RED, font6x9);
				while (lcd_is_busy()) {}
				lcd_copy();
				printf("Measurement: %d, Set value: %d\n\n", light, value);



	}

}

// Define constants for readability
#define PWM_MAX 10000
#define PWM_MIN 0
#define INTEGRAL_MAX 10000
#define INTEGRAL_MIN -10000
#define LIGHT_RANGE_FACTOR 0.5
#define STEPPER_STEP_SIZE 10

void updatePID() {
    // Calculate error
    error = value - light;

    // Update integral with anti-windup
    integral += error * dt;
    if (integral > INTEGRAL_MAX) integral = INTEGRAL_MAX;
    if (integral < INTEGRAL_MIN) integral = INTEGRAL_MIN;

    // Calculate derivative
    derivative = (error - previous_error) / dt;

    // Compute PID output
    output = (Kp * error) + (Ki * integral) + (Kd * derivative);

    // Clamp output to valid PWM range
    if (output > PWM_MAX) output = PWM_MAX;
    if (output < PWM_MIN) output = PWM_MIN;

    // Store error for next iteration
    previous_error = error;
}

void control_logic() {
    // Determine control mode (diode or stepper motor)
//    if (currentAngle >=maxAngle || light<value) {
//        ster = 1;  // Control diode
//    } else {
//        ster = 0;  // Control stepper motor
//    }
//
//    if (ster == 1) {
        // Run PID control for the diode
        updatePID();
        TIM1->CCR1 = (uint32_t)output;  // Set PWM output
  // } else {
        // Control stepper motor based on light
        if (light >= value) {
            engineR = 1;  // Close
        } else {
            engineR = 0;  // Open
        }

        // Stepper motor movement with angle constraints
        if (engineR == 1 && currentAngle <= maxAngle) {
            stepper_step_angle(STEPPER_STEP_SIZE, 0, 12);
            currentAngle += STEPPER_STEP_SIZE;
        } else if (engineR == 0 && currentAngle >= minAngle) {
            stepper_step_angle(STEPPER_STEP_SIZE, 1, 12);
            currentAngle -= STEPPER_STEP_SIZE;
        }
    }


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
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_SPI2_Init();
  MX_TIM3_Init();
  MX_I2C1_Init();
  MX_TIM6_Init();
  MX_TIM1_Init();
  MX_TIM15_Init();
  MX_TIM7_Init();
  /* USER CODE BEGIN 2 */
  lcd_init();
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
    HAL_TIM_Base_Start_IT(&htim6);
    HAL_TIM_Base_Start_IT(&htim7);
    HAL_TIM_Base_Start(&htim15);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    BH1750_Init(&hbh1750A);
    for (int i = 0; i < 8; i++) {
      hagl_draw_rounded_rectangle(2+i, 2+i, 158-i, 126-i, 8-i, rgb565(0, 0, i*16));
    }
    lcd_copy();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */



  while (1)
  {


	 // control_logic();
	  //        HAL_Delay(1000);  // Run control loop periodically

	  HAL_Delay(10);
	    	if (mode==1){
	    		pulse = (pulse + 5 > 10000) ? 10000 : pulse + 1; // increase by 1
	    	} else {
	    		pulse -= 10; // Decrease pulse by 5
	    		        if (pulse < 0) {
	    		            pulse = 0; // Clamp pulse to 0
	    		        }
	    	}
	    	if (light >= value) {
	    		mode = 0;
	    	}
	    	if (light < value) {
	    		mode = 1;
	    	}
	    	TIM1->CCR1 = pulse;

	    	if (light >= value) {
	    	            engineR = 1;  // Close
	    	        } else if(light < 0.8*value){
	    	            engineR = 0;  // Open
	    	        }

	    	        // Stepper motor movement with angle constraints
	    	        if (engineR == 1 && currentAngle <= maxAngle) {
	    	            stepper_step_angle(STEPPER_STEP_SIZE, 0, 6);
	    	            currentAngle += STEPPER_STEP_SIZE;
	    	        } else if (engineR == 0 && currentAngle >= minAngle) {
	    	            stepper_step_angle(STEPPER_STEP_SIZE, 1, 6);
	    	            currentAngle -= STEPPER_STEP_SIZE;
	    	        }

}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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

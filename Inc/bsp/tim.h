#include "main.h"


extern TIM_HandleTypeDef htim2;

extern TIM_HandleTypeDef htim3;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void PWM_Output_Init(void);
void PWM_Input_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
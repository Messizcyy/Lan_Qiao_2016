#include "main.h"
#include "rcc.h"
#include "lcd.h"
#include "tim.h"
#include "adc.h"
#include "rtc.h"
#include "usart.h"
#include "i2c_hal.h"
#include "key_led.h"

void Key_Proc(void);
void Led_Proc(void);
void Lcd_Proc(void);
void PWM_Proc(void);
void Measure_Proc(void);

//for delay
__IO uint32_t uwTick_key, uwTick_led, uwTick_lcd, uwTick_mea, uwTick_pwm;

//for led
uint8_t led;

//for key
uint8_t key_val, key_old, key_down, key_up;

//for lcd
uint8_t string_lcd[21];

//for eeprom
uint8_t eeprom_buf_W[5] = {'h', 60, 90, 1, 1};
uint8_t eeprom_buf_R[5];

//for pwm
uint32_t pwm_d;
float pwm_duty;
uint32_t pwm_t;
float freq_disp = 1;
float freq_real = 1;

//for usart
uint8_t rx_buf;
char string_uart[100];

//for R37
float r37_v;

//for rtc
RTC_TimeTypeDef rtc_time;
RTC_DateTypeDef rtc_date;
uint32_t Sec[100];
uint32_t Min[100];
uint32_t Hour[100];

//for flags
uint8_t lcd_index = 0; //0-para  1-setting
uint8_t which_para = 0;

//for task
int16_t wendu_max_disp = 60;
int16_t shidu_max_disp = 90;

int16_t wendu_max_real = 60;
int16_t shidu_max_real = 90;
//wendu
float T;
int8_t b = -20;
float k = 24.24242424;
//shidu
float H;
float m = 8.888889;
float n = 1.111111;
//measure
uint8_t measure_gap_disp = 1;
uint8_t measure_gap_real = 1;
uint32_t record_counter = 0;
float record_T[100];
float record_H[100];

int main(void)
{

  HAL_Init();
	//HAL_Delay(2);
  SystemClock_Config();
	
	Key_Led_Init();
	
	ADC2_Init();
	
	I2CInit();
	
	UART_Init();
	HAL_UART_Receive_IT(&huart1, &rx_buf, 1);
	
	eeprom_read(eeprom_buf_R, 0, 5);
	HAL_Delay(10);
	if(eeprom_buf_R[0] != 'h')
	{
		eeprom_write(eeprom_buf_W, 0, 5);
//		wendu_max_real = eeprom_buf_W[1];
//		shidu_max_real = eeprom_buf_W[2];
//		measure_gap_real = eeprom_buf_W[3];
//		freq_real = eeprom_buf_W[4];
	}

	else 
	{
		eeprom_read(eeprom_buf_R, 0, 5);
		wendu_max_real = eeprom_buf_R[1];
		shidu_max_real = eeprom_buf_R[2];
		measure_gap_real = eeprom_buf_R[3];
		freq_real = eeprom_buf_R[4];
		
		wendu_max_disp = eeprom_buf_R[1];
		shidu_max_disp = eeprom_buf_R[2];
		measure_gap_disp = eeprom_buf_R[3];
		freq_disp = eeprom_buf_R[4];
	}

	
	RTC_Init();
	
	PWM_Output_Init();
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
	PWM_Input_Init();
	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2);
	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
	HAL_TIM_Base_Start(&htim3);
	
	LCD_Init();
	LCD_Clear(Black);
	LCD_SetTextColor(White);
	LCD_SetBackColor(Black);

  while (1)
  {
		Key_Proc();
		Led_Proc();
		Lcd_Proc();
		PWM_Proc();
		Measure_Proc();
  }

}

void Key_Proc(void)
{
	if(uwTick - uwTick_key < 50)	return;
	uwTick_key = uwTick;
	
	key_val = Read_Key();
	key_down = key_val & (key_val ^ key_old);
	key_up  = ~key_val & (key_val ^ key_old);
	key_old = key_val;
	
	if(key_down == 1)
	{
		lcd_index ^= 1;
		LCD_Clear(Black);
		which_para = 0;
		
		if(lcd_index == 0)
		{
			wendu_max_real = wendu_max_disp;
			shidu_max_real = shidu_max_disp;
			measure_gap_real = measure_gap_disp;
			freq_real = freq_disp;
			
			eeprom_buf_W[0] = wendu_max_real;
			eeprom_buf_W[1] = shidu_max_real;
			eeprom_buf_W[2] = measure_gap_real;
			eeprom_buf_W[3] = freq_real;
			eeprom_write(eeprom_buf_W, 1, 4);
		}
	}
	
	else if(key_down == 2)
	{
		if(lcd_index == 1)
		{
			if(which_para <4)
				which_para++;
			if(which_para == 4)
				which_para = 0;
		}
	}
	
	else if(key_down == 3)
	{
		if(lcd_index == 1)
		{
			switch(which_para)
			{
				case 0:	if(wendu_max_disp<60)	wendu_max_disp++;	break;
				case 1:	if(shidu_max_disp<90)	shidu_max_disp+=5;	break;		
				case 2:	if(measure_gap_disp<5)	measure_gap_disp++;	break;
				case 3:	if(freq_disp<10)	freq_disp+=0.5;	break;			
			}
		}
	}
	
	
	else if(key_down == 4)
	{
		if(lcd_index == 1)
		{
			switch(which_para)
			{
				case 0:	if(wendu_max_disp>-20)	wendu_max_disp--;	break;
				case 1:	if(shidu_max_disp>10)	shidu_max_disp-=5;	break;		
				case 2:	if(measure_gap_disp>1)	measure_gap_disp--;	break;
				case 3:	if(freq_disp>1.1)	freq_disp-=0.5;	break;			
			}
		}
	}	
	
}

void PWM_Proc(void)
{
	if(uwTick - uwTick_pwm < 50)	return;
	uwTick_pwm = uwTick;

	__HAL_TIM_SetAutoreload(&htim2, __HAL_TIM_GetAutoreload(&htim2)/(freq_real*1000));
	__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_2, __HAL_TIM_GetAutoreload(&htim2)*0.5);

}

void Measure_Proc(void)
{
	if((uwTick - uwTick_mea) < (measure_gap_disp*1000))	return;
	uwTick_mea = uwTick;

	r37_v = get_adc2()*3.3/4096;
	T = k*r37_v+b;
	
	H = m*freq_real+n;
	
	if(record_counter<100)
	{
		record_T[record_counter] = T;
		record_H[record_counter] = H;
		Sec[record_counter] = rtc_time.Seconds;
		Min[record_counter] = rtc_time.Minutes;
		Hour[record_counter] = rtc_time.Hours;
		
		record_counter++;
	}

	//__HAL_TIM_GET_AUTORELOAD(&htim3);

}


void Led_Proc(void)
{
	if(uwTick - uwTick_led < 50)	return;
	uwTick_led = uwTick;

	if(T > wendu_max_real)
		led ^= 0x01;
	else
		led &= (~0x01);	
	if(H > shidu_max_real)
		led ^= 0x02;
	else
		led &= (~0x02);
	if(record_counter < 100)
		led ^= 0x04;
	else
		led &= (~0x04);
	
	Led_Disp(led) ;


}


void Lcd_Proc(void)
{
	if(uwTick - uwTick_lcd < 50)	return;
	uwTick_lcd = uwTick;
	
	HAL_RTC_GetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &rtc_date, RTC_FORMAT_BIN);

	if(lcd_index == 0)
	{
		LCD_SetTextColor(White);
		
		sprintf((char *)string_lcd, "       DATA");
		LCD_DisplayStringLine(Line0, string_lcd);
		
		sprintf((char *)string_lcd, "current_T:  %2.0f C ", T);
		LCD_DisplayStringLine(Line2, string_lcd);
						
		sprintf((char *)string_lcd, "current_H:  %2.0f %% ", H);
		LCD_DisplayStringLine(Line4, string_lcd);
		
		sprintf((char *)string_lcd, "RTC: %02d-%02d-%02d", (uint32_t)rtc_time.Hours, (uint32_t)rtc_time.Minutes, (uint32_t)rtc_time.Seconds);
		LCD_DisplayStringLine(Line6, string_lcd);	
		
		sprintf((char *)string_lcd, "         counter:%d ", record_counter);
		LCD_DisplayStringLine(Line9, string_lcd);		


	}	

	else 
	{
		LCD_SetTextColor(White);
		sprintf((char *)string_lcd, "       PARA");
		LCD_DisplayStringLine(Line0, string_lcd);
		
		if(which_para == 0)
			LCD_SetTextColor(Green);
		else
			LCD_SetTextColor(White);
		sprintf((char *)string_lcd, "MAX_T:  %2d C ", wendu_max_disp);
		LCD_DisplayStringLine(Line2, string_lcd);
				
		if(which_para == 1)
			LCD_SetTextColor(Green);
		else
			LCD_SetTextColor(White);		
		sprintf((char *)string_lcd, "MAX_H:  %2d %% ", shidu_max_disp);
		LCD_DisplayStringLine(Line4, string_lcd);
		
		if(which_para == 2)
			LCD_SetTextColor(Green);
		else
			LCD_SetTextColor(White);
		sprintf((char *)string_lcd, "measure_gap_disp: %dS", measure_gap_disp);
		LCD_DisplayStringLine(Line6, string_lcd);		
		
		if(which_para == 3)
			LCD_SetTextColor(Green);
		else
			LCD_SetTextColor(White);
		sprintf((char *)string_lcd, "signal freq: %.1fKHz", freq_disp);
		LCD_DisplayStringLine(Line8, string_lcd);		
	
	}

}





void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM3)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			pwm_t = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_2)+1;
			pwm_duty = (float)pwm_d/pwm_t;
		}
	
		else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			pwm_d = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_1)+1;
		}
	
	}
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	int i;
	
	if(rx_buf == 'C')
	{
		sprintf(string_uart, "wendu:%2d C\r\nshidu:%2d C\r\ntime:%d-%d-%d\r\n", wendu_max_real, shidu_max_real, (uint32_t)rtc_time.Hours, (uint32_t)rtc_time.Minutes, (uint32_t)rtc_time.Seconds);
		HAL_UART_Transmit(&huart1, (uint8_t *)string_uart, strlen(string_uart), 50);
	
	}

	else if(rx_buf == 'T')
	{
		for(i=0; i<record_counter; i++)
		{
			sprintf(string_uart, "wendu:%2.0f C\r\nshidu:%2.0f C\r\ntime:%d-%d-%d\r\n", record_T[i], record_H[i], Sec[i], Min[i], Hour[i]);
			HAL_UART_Transmit(&huart1, (uint8_t *)string_uart, strlen(string_uart), 50);		
		}
	}
	
	HAL_UART_Receive_IT(&huart1, &rx_buf, 1);

}




void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}



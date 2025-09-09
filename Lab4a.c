/*
 * userApp.c
 *
 *  Created on: Dec 8, 2023
 *      Author: Niall.OKeeffe@atu.ie
 */

#include "userApp.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>

//--------------------------------------------------------------
//used for real time stats, do not delete code from this section
extern TIM_HandleTypeDef htim7;
extern volatile unsigned long ulHighFrequencyTimerTicks;
void configureTimerForRunTimeStats(void)
{
    ulHighFrequencyTimerTicks = 0;
    HAL_TIM_Base_Start_IT(&htim7);
}
unsigned long getRunTimeCounterValue(void)
{
	return ulHighFrequencyTimerTicks;
}
//end of real time stats code
//----------------------------------------------------------------

extern UART_HandleTypeDef huart1;

TaskHandle_t buttonTaskHandle = NULL;
static void buttonTask(void * pvParameters);
static void ledTask(void * pvParameters);

// _write function used for printf
int _write(int file, char *ptr, int len) {
	HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
	return len;
}


void userApp() {
	printf("Starting application\r\n\n");

	//HAL_UART_Receive_IT(&huart1, &ch, 1);
	//HAL_TIM_Base_Start_IT(&htim6);

	xTaskCreate(buttonTask, "Button Task ", 200, NULL, 2, &buttonTaskHandle);
	xTaskCreate(ledTask, "LED Task ", 200, NULL, 1, NULL);
	vTaskStartScheduler();

	while(1) {
	}
}

void buttonTask(void * pvParameters) {
	printf("Starting button task \r\n\n");
	while(1) {
		//wait for switch press
		while(HAL_GPIO_ReadPin(BUTTON_EXTI13_GPIO_Port, BUTTON_EXTI13_Pin)== 1);
		printf("Switch press detected, suspending task\r\n\n");
		//suspend task
		vTaskSuspend(NULL);
		//wait for switch release
		while(HAL_GPIO_ReadPin(BUTTON_EXTI13_GPIO_Port, BUTTON_EXTI13_Pin)== 0);
	}
}

void ledTask(void * pvParameters) {
	printf("Starting LED task \r\n\n");
	while(1) {
		printf("Toggling LED2 and resuming button task\r\n\n");
		//toggle LED2
		HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
		//resume the button task
	    vTaskResume(buttonTaskHandle);
	}
}


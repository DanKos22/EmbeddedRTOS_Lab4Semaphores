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


static void buttonTask(void * pvParameters);
static void displayTask(void * pvParameters);
static void timerTask(void * pvParameters);
SemaphoreHandle_t buttonSemaphore = NULL;
SemaphoreHandle_t timerSemaphore = NULL;

// _write function used for printf
int _write(int file, char *ptr, int len) {
	HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
	return len;
}


void userApp() {
	printf("Starting application\r\n\n");

	xTaskCreate(buttonTask, "Button Task", 200, NULL, 1, NULL);
	xTaskCreate(displayTask, "Display Task", 200, NULL, 3, NULL);
	xTaskCreate(timerTask, "Timer Task", 200, NULL, 2, NULL);
	buttonSemaphore = xSemaphoreCreateBinary();
	vQueueAddToRegistry(buttonSemaphore, "Button Semaphore");
	timerSemaphore = xSemaphoreCreateBinary();
	vQueueAddToRegistry(timerSemaphore, "Timer Semaphore");
	vTaskStartScheduler();

	while(1) {
	}
}



void buttonTask(void * pvParameters) {
	printf("Starting button task \r\n\n");
	while(1) {
		//wait for switch press
		if(HAL_GPIO_ReadPin(BUTTON_EXTI13_GPIO_Port, BUTTON_EXTI13_Pin) == 0){
			printf("Switch press detected, giving semaphore\r\n\n");
			//Give Semaphore
			xSemaphoreGive(buttonSemaphore);

			//wait for switch release
			while(HAL_GPIO_ReadPin(BUTTON_EXTI13_GPIO_Port, BUTTON_EXTI13_Pin) == 0);
		}
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

void displayTask(void * pvParameters) {
	uint32_t sec = 0;
	printf("Starting Display task \r\n\n");
	printf("Time: %03lu\r\n\n", sec);
	while(1) {
		if(xSemaphoreTake(buttonSemaphore, 0) == pdTRUE){
			printf("Toggling LED 2...\r\n\n");
			//toggle LED2
			HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
		}
		else if(xSemaphoreTake(timerSemaphore, 0) == pdTRUE);
		sec++;
		printf("Time: %03lu\r\n", sec);

	}
	vTaskDelay(pdMS_TO_TICKS(10));
}

void timerTask(void * pvParameters) {
	printf("Starting timer task \r\n\n");
	while(1) {
		vTaskDelay(pdMS_TO_TICKS(1000));
	    xSemaphoreGive(timerSemaphore);
	}
}



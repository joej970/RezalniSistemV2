/*
 * GRBL_control.c
 *
 *  Created on: Feb 6, 2025
 *      Author: Asus
 */

#include <stdint.h>
#include <string.h>

#include "FreeRTOS.h"
#include "main.h"
#include "qPackages.h"
#include "queue.h"
#include "event_groups.h"
//#include "stm32f7xx_hal_uart.h"

//#include <gui_generated/containers/popUpUartConsoleContainerBase.hpp>
#include "GRBL_control.h"


void initiateGrblController(){

}

void enableGRBLcontrol() {
    // 2. Enable Capture Compare Interrupt for TIM1 in the NVIC
    HAL_NVIC_SetPriority(TIM1_CC_IRQn, 4, 0);  // Priority level (1, 0)
    HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);
}

void disableGRBLcontrol() {
    // 2. Disable Capture Compare Interrupt for TIM1 in the NVIC
    HAL_NVIC_DisableIRQ(TIM1_CC_IRQn);
}

void TIM1_CC_IRQHandler(void) {
    // Check if the capture compare interrupt flag is set
    if (TIM1->SR & TIM_SR_CC1IF) {
        // Clear the interrupt flag by writing 0
        TIM1->SR &= ~TIM_SR_CC1IF;

        // OPTION 1
        {
        extern EventGroupHandle_t ehEvents;
        xEventGroupSetBits(ehEvents, EVENT_BIT_INITIATE_GRBL_CONTROLLER);
        }
        // OR OPTION 2
        {
//        	sendUartMessage("~"); // send symbol to continue
        }

    }
}



// I use receive to IDLE
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{

	extern QueueHandle_t qhUARTcallbackToGRBL;
	extern char* pUARTRxGlobalBuffer;
	qPackage_UART_RX uartPayload;

	if(!(Size < UART_RX_GLOBAL_BUFFER_SIZE)){ // if size is not smaller than capacity; there must be one more character for terminating character
		return;
	}

	strncpy(uartPayload.data, pUARTRxGlobalBuffer, Size);
	uartPayload.data[Size] = '\0';
	uartPayload.length = Size + 1;

	BaseType_t xStatus = xQueueSend(qhUARTcallbackToGRBL, &uartPayload, pdMS_TO_TICKS(0));
	if(xStatus != pdTRUE){
		//TODO: report a problem
		while(1){
			HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_PIN_SET);
		};
	}



	// re-enable reception
	//HAL_UART_Receive_IT(&huart1, rx_buffer, sizeof(rx_buffer));
	HAL_UARTEx_ReceiveToIdle_IT(huart, (uint8_t*)pUARTRxGlobalBuffer, UART_RX_GLOBAL_BUFFER_SIZE);



}

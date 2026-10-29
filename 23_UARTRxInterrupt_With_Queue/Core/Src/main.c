/* Date : 31-07-2025

1. UART uses interrupts it can receive data.
2. FreeRTOS queue is created to transfer received bytes from the ISR to a task.
3. Task (task_Handler) blocks and waits for data from the queue.
4. When a byte arrives on UART,the hardware triggers an interrupt.
5. ISR (HAL_USART_RxCpltCallback)runs:
   * Reads the received byte into "rec_data".
   * Sends "rec_data" to the FreeRTOS queue using "xQueueSendFromISR()".
   * Re-enables UART to receive the next byte.
   * If the waiting task has higher priority, sets a flag to switch to it immediately.
6. After ISR completes,if a higher-priority task was waiting,"portYIELD_FROM_ISR()" causes an immediate context switch.
7. The waiting task unblocks, retrieves the byte from the queue.
8. This cycle repeats for every byte received.
*/

#include "main.h"
#include "cmsis_os.h"

USART_HandleTypeDef husart2;
QueueHandle_t queue_handler;
uint8_t rec_data;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_Init(void);
void task_Handler();
void StartDefaultTask(void *argument);

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART2_Init();

  //xTaskCreate(task_Handler, "task handler", 100, NULL, 1, 0);

  // Create a queue to hold 1 byte (uint8_t) at a time
  //queue_handler = xQueueCreate(1, sizeof(uint8_t));

  // Enable UART receive interrupt for the first byte
  HAL_USART_Receive_IT(&husart2, &rec_data, 1);
 // vTaskStartScheduler();
  while (1)
  {
  }
}

void HAL_USART_RxCpltCallback(USART_HandleTypeDef *husart)
{
    if (husart->Instance == USART2)
    {
        //printf("Received: %c\r\n", rec_data); // Or transmit back to Tera Term
        HAL_USART_Receive_IT(&husart2, &rec_data, 1); // Re-arm for next byte
    }
}


/* Callback called automatically by HAL when a byte is received via USART2 */
/*void HAL_USART_RxCpltCallback(USART_HandleTypeDef *husart)
{
    /*This variable tells FreeRTOS whether a higher-priority task should run after ISR
	portBASE_TYPE HigherPriorityTaskWoken = pdFALSE;

	 // Ensure the interrupt is from USART2
	if (husart->Instance == USART2)
	{
		 Put the received byte into the queue.
		 If the task waiting on this queue has a higher priority than the current
		 running task,FreeRTOS will set HigherPriorityTaskWoken = pdTRUE to request
		 an immediate context switch.
		xQueueSendFromISR(queue_handler,&rec_data,&HigherPriorityTaskWoken);

		// Re-enable UART interrupt to receive the next byte
		HAL_USART_Receive_IT(&husart2, &rec_data, 1);
	}
	 If HigherPriorityTaskWoken is pdTRUE, FreeRTOS will switch
	   to the higher-priority task immediately after the ISR finishes
	portYIELD_FROM_ISR(HigherPriorityTaskWoken);
}*/

/*char recFromQueue;
void task_Handler(void)
{
	while(1)
	{
		// Wait here (blocked) until ISR sends a byte to the queue
		xQueueReceive(queue_handler, &recFromQueue, portMAX_DELAY);
	}

}
*/
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}
static void MX_USART2_Init(void)
{
  husart2.Instance = USART2;
  husart2.Init.BaudRate = 115200;
  husart2.Init.WordLength = USART_WORDLENGTH_8B;
  husart2.Init.StopBits = USART_STOPBITS_1;
  husart2.Init.Parity = USART_PARITY_NONE;
  husart2.Init.Mode = USART_MODE_TX_RX;
  husart2.Init.CLKPolarity = USART_POLARITY_LOW;
  husart2.Init.CLKPhase = USART_PHASE_1EDGE;
  husart2.Init.CLKLastBit = USART_LASTBIT_DISABLE;
  if (HAL_USART_Init(&husart2) != HAL_OK)
  {
    Error_Handler();
  }
}
static void MX_GPIO_Init(void)
{
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
}
void StartDefaultTask(void *argument)
{
}
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif

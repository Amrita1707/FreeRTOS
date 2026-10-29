
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "main.h"
#include "cmsis_os.h"

TaskHandle_t  sender1_handler,sender2_handler,receiverhandler;
USART_HandleTypeDef husart2;

int _io__putchar( int ch);
int uart2_write( int ch );

/*function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_Init(void);
void senderTask1(void *PvParameters);
void senderTask2(void *PvParameters);
void receiverTask(void *PvParameters);

  //declare two queues
  static QueueHandle_t queue1 = NULL, queue2 = NULL;

  //declare queue set
  //we have to set queueset in freertosconfig.h file
  static QueueSetHandle_t queueset = NULL;

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART2_Init();

  /* USER CODE BEGIN WHILE */
  // Create two queues, each can hold one item: a pointer to a character string
  queue1 = xQueueCreate(1, sizeof(char *));
  queue2 = xQueueCreate(1, sizeof(char *));

  /// Create a queue set that can hold two queue handles
  // Total space = number of queues you want to monitor (not the number of items in each queue)
  queueset = xQueueCreateSet(2);

  //add the two queues to the queue set
  xQueueAddToSet(queue1, queueset);
  xQueueAddToSet(queue2, queueset);

  //create two sender task with same priority
  xTaskCreate(senderTask1, "senderTask1",100,NULL, 1, &sender1_handler);
  xTaskCreate(senderTask2, "senderTask2",100,NULL, 1, &sender2_handler);

  //create receiver handler with highest priority
  xTaskCreate(receiverTask, "receiver task", 100, NULL, 2, &receiverhandler);

  vTaskStartScheduler();
  while (1)
  {

  }

}

// Function to send a byte via USART2
int uart2_write( int ch )
{
	/*Wait until Transmit Data Register Empty (TXE = 1)
	  While TXE is 0, it means the previous data has not yet been moved
	*/
	while( !(USART2->SR & 0x0080)){}

	//Write the lower 8 bits of 'ch' into the Data Register (DR) for transmission
	USART2-> DR = ( ch & 0xFF );
	return ch;
}

int _io__putchar( int ch)
{
	uart2_write( ch);
	return ch;
}

// Task1 to send data to the queue
void senderTask1(void *PvParameters)
{
	const TickType_t time = pdMS_TO_TICKS(100);
	char * msg = "msg from senderTask1";
	while(1)
	{
		//block of 100ms
		vTaskDelay(time);
		xQueueSend(queue1, &msg, 0);
		/*if(qstatus != pdPASS)
		{
			// Print error if queue is full
			printf("Error: data could not be send....\r\n");
			for(int i=0; i<1000000; i++);
		}*/
	}
}

// Task1 to send data to the queue
void senderTask2(void *PvParameters)
{
	const TickType_t time = pdMS_TO_TICKS(200);
	char * msg = "msg from senderTask2";
	while(1)
	{
		//block of 200ms
		vTaskDelay(time);
		xQueueSend(queue2, &msg, 0);
	}
}


// Task to receive data from any queue in the queue set
void receiverTask(void *PvParameters)
{
	char * received_data ;
    // This will store the handle of the queue which has data available
	QueueHandle_t queuecontains_data;
	while(1)
	{
		// Wait for any queue in the queue set to have data
		queuecontains_data = xQueueSelectFromSet(queueset, portMAX_DELAY);

		// Receive data from the specific queue that has data
		xQueueReceive(queuecontains_data, &received_data, 0);
		printf("%s", received_data);
	}
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 50;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
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
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */

  /* USER CODE BEGIN 5 */
  /* Infinite loop */

  /* USER CODE END 5 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */

  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */

  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */


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

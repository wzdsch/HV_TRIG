/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-07-30 21:49:07
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-08-06 10:18:52
 * @FilePath: \code\Proj\Components\bsp\bsp_uart.c
 * @Description: 
 * 本项目只用于控制THz专用激励源的控制，切勿用于其他用途，否则西安理工大学及开发者不承担任何责任。
 * Copyright (c) 2026 by Jiang Tianhang 1919524828@qq.com, All Rights Reserved. 
 */
#include "bsp_uart.h"
#include "usart.h"
#include "cmsis_os2.h"
#include "dma.h"
#include "freertos.h"
#include "task.h"
#include "string.h"
#include "queue.h"
#include "semphr.h"

#pragma pack(1)

/// @brief uart message transmitted by queue, only used in this moudle
typedef struct {
  uint16_t size;
  uint8_t* p_data;
} BSP_UART_TxMsg_t;

#pragma pack()

// receive buffer
#define UART_RX_BUF_SIZE 256
uint8_t uart_rx_buf[UART_RX_BUF_SIZE];

// task
osThreadId_t bsp_uart_task;
osThreadAttr_t bsp_uart_task_attributes = {
  .name = "bsp_uart_task",
  .stack_size = 512,
  .priority = (osPriority_t) osPriorityNormal,
};

// uart message queue
osMessageQueueId_t uart_queue;

QueueHandle_t uart_queue_handle;

/* OK应答信号量：屏幕回复"OK\r\n"后释放，BSP_UART_Task 据此流控 */
static SemaphoreHandle_t uart_ok_sem = NULL;

void (*BSP_UART_ReceiveToIdleCallback)(uint16_t Size) = NULL;

extern void BSP_UART_Task(void *argument);

void BSP_UART_Init(void) {
  // create uart queue
  uart_queue = osMessageQueueNew(20, sizeof(BSP_UART_TxMsg_t*), NULL);
  uart_queue_handle = (QueueHandle_t)uart_queue;
  // create uart task
  bsp_uart_task = osThreadNew(BSP_UART_Task, NULL, &bsp_uart_task_attributes);

  // clear task state
  xTaskNotifyStateClear((TaskHandle_t)bsp_uart_task);
  ulTaskNotifyValueClear((TaskHandle_t)bsp_uart_task, 0xFFFFFFFF);

  // create OK semaphore (binary, initial 0)
  uart_ok_sem = xSemaphoreCreateBinary();

  // start uart receive to idle interrupt
  HAL_UARTEx_ReceiveToIdle_IT(&huart1, uart_rx_buf, UART_RX_BUF_SIZE);
}

void BSP_UART_SetReceiveToIdleCallback(void (*callback)(uint16_t Size)) {
  BSP_UART_ReceiveToIdleCallback = callback;
}

uint8_t BSP_UART_Transmit(uint8_t* p_data, uint16_t size) {
  uint8_t malloc_err_flg = 0;

  // param error, return 1
  if (p_data == NULL || size == 0) {
    return 1;
  }

  // malloc memory for tx message
  BSP_UART_TxMsg_t* p_txmsg = pvPortMalloc(sizeof(BSP_UART_TxMsg_t));
  if (p_txmsg != NULL) {
    p_txmsg->p_data = pvPortMalloc(size);
    if (p_txmsg->p_data == NULL) {
      vPortFree(p_txmsg);
      malloc_err_flg = 1;
    }
  }
  else {
    malloc_err_flg = 1;
  }

  // malloc error, return 1
  if (malloc_err_flg) {
    return 1;
  }

  // assign
  memcpy(p_txmsg->p_data, p_data, size);
  p_txmsg->size = size;

  // send to queue
  if(osMessageQueuePut(uart_queue, &p_txmsg, 0, 0) != osOK) {
    // send to queue error, free memory and return 1
    vPortFree(p_txmsg->p_data);
    vPortFree(p_txmsg);
    return 1;
  }

  // success, return 0
  return 0;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
  if(huart->Instance == USART1) {
    // uart transmit complete, notify uart task, and yield if necessary
    BaseType_t yield_flg = pdFALSE;
    vTaskNotifyGiveFromISR((TaskHandle_t)bsp_uart_task, &yield_flg);
    if(yield_flg == pdTRUE) {
      portYIELD_FROM_ISR(yield_flg);
    }
  }
}

void BSP_UART_Task(void *argument) {
  for(;;) {
    BSP_UART_TxMsg_t *p_txmsg = NULL;

    // get message from queue, transmit and wait for transmit complete notification
    osMessageQueueGet(uart_queue, &p_txmsg, NULL, osWaitForever);
    while(HAL_UART_Transmit_DMA(&huart1, p_txmsg->p_data, p_txmsg->size) != HAL_OK);
    ulTaskNotifyTake(pdTRUE, 1000);

    // wait for screen "OK\r\n" response, timeout to avoid deadlock
    if (xSemaphoreTake(uart_ok_sem, pdMS_TO_TICKS(200)) != pdTRUE) {
      // timeout: screen didn't respond, continue anyway
    }

    // after transmit complete, free memory
    vPortFree(p_txmsg->p_data);
    vPortFree(p_txmsg);
  }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
  if(huart->Instance == USART1) {
    // check for "OK\r\n" at the end of received data
    if (Size >= 4) {
      uint8_t *tail = &uart_rx_buf[Size - 4];
      if (tail[0] == 'O' && tail[1] == 'K' && tail[2] == '\r' && tail[3] == '\n') {
        BaseType_t yield_flg = pdFALSE;
        xSemaphoreGiveFromISR(uart_ok_sem, &yield_flg);
        if (yield_flg == pdTRUE) {
          portYIELD_FROM_ISR(yield_flg);
        }
      }
    }
    // restart RX
    HAL_UARTEx_ReceiveToIdle_IT(&huart1, uart_rx_buf, UART_RX_BUF_SIZE);
  }
}


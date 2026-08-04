/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-07-30 21:49:07
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-08-04 21:59:42
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

#pragma pack(1)

/// @brief uart message transmitted by queue, only used in this moudle
typedef struct {
  uint8_t size;
  uint8_t* p_data;
} BSP_UART_TxMsg_t;

#pragma pack()

// task
osThreadId_t bsp_uart_task;
osThreadAttr_t bsp_uart_task_attributes = {
  .name = "bsp_uart_task",
  .stack_size = 512,
  .priority = (osPriority_t) osPriorityNormal,
};

// uart message queue
osMessageQueueId_t uart_queue;

void BSP_UART_Task(void *argument);

void BSP_UART_Init(void) {
  // create uart queue
  uart_queue = osMessageQueueNew(10, sizeof(BSP_UART_TxMsg_t*), NULL);

  // create uart task
  bsp_uart_task = osThreadNew(BSP_UART_Task, NULL, &bsp_uart_task_attributes);

  // clear task state
  xTaskNotifyStateClear((TaskHandle_t)bsp_uart_task);
  ulTaskNotifyValueClear((TaskHandle_t)bsp_uart_task, 0xFFFFFFFF);
}


uint8_t BSP_UART_Transmit(uint8_t* p_data, uint8_t size) {
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
    if (HAL_UART_Transmit_DMA(&huart1, p_txmsg->p_data, p_txmsg->size) != HAL_OK) {
            vPortFree(p_txmsg->p_data);
            vPortFree(p_txmsg);
            continue;
        }
    if (ulTaskNotifyTake(pdTRUE, 1000))

    // after transmit complete, free memory
    vPortFree(p_txmsg->p_data);
    vPortFree(p_txmsg);
    osDelay(1);
  }
}


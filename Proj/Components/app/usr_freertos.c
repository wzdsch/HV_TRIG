/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-07-30 18:16:22
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-08-04 16:53:21
 * @FilePath: \code\Proj\Components\app\usr_freertos.c
 * @Description: 
 * 本项目只用于控制THz专用激励源的控制，切勿用于其他用途，否则西安理工大学及开发者不承担任何责任。
 * Copyright (c) 2026 by Jiang Tianhang 1919524828@qq.com, All Rights Reserved. 
 */
#include "usr_freertos.h"
#include "cmsis_os2.h"
#include "usr_tasks.h"
#include "bsp_uart.h"
#include "key_scan.h"
#include "encoder.h"

void Main_Task(void *argument);

osThreadAttr_t main_task_attr = {
  .name = "MainTask",
  .stack_size = 2048,
  .priority = osPriorityLow,
};

void Usr_FreeRTOS_Init(void) {
	osThreadNew(Main_Task, NULL, &main_task_attr);
  BSP_UART_Init();
  Key_Init();
  Encoder_Init();
}

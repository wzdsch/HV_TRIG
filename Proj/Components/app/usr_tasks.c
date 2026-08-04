/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-07-30 17:22:02
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-08-04 22:15:54
 * @FilePath: \code\Proj\Components\app\usr_tasks.c
 * @Description: 
 * 本项目只用于控制THz专用激励源的控制，切勿用于其他用途，否则西安理工大学及开发者不承担任何责任。
 * Copyright (c) 2026 by Jiang Tianhang 1919524828@qq.com, All Rights Reserved. 
 */
#include "FreeRTOS.h"
#include "usr_tasks.h"
#include "main.h"
#include "cmsis_os2.h"
#include "usart.h"
#include "bsp_uart.h"
#include "task.h"
#include "logic.h"
#include "EEPROM.h"

void Main_Task(void *argument) {
  Display_UpdateSettingsData(&settings_data);
  Display_ShowMain();
  osDelay(1000);
  Display_ShowSettings();
  for(;;) {
    osDelay(100);
  }
}

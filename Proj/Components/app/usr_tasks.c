/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-07-30 17:22:02
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-08-05 22:38:57
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
#include "bsp_uart.h"
#include <string.h>
#include "display.h"


void Main_Task(void *argument) {
  Display_Init();
  // osDelay(1000);
  // Display_ShowSettings();
  // uint8_t p_data[] = "CLR(0);DIR(1);SBC(15);FSIMG(2177400,0,0,220,176,0);DCV16(15,38,'default',0);DCV16(15,72,'preset1',0);DCV16(15,108,'preset2',0);SBC(13);DCV16(54,10,'000',16);DCV16(50,151,'00.0',16);DCV16(132,10,'00.0',16);DCV16(140,151,'50',16);DCV16(54,10,'0',16);DCV16(62,10,'0',16);DCV16(70,10,'0',16);DCV16(50,151,'0',16);DCV16(58,151,'0',16);DCV16(74,151,'0',16);DCV16(140,151,'0',16);DCV16(148,151,'0',16);\r\n";
  // HAL_UART_Transmit_DMA(&huart1, p_data, strlen((char*)p_data));
  // BSP_UART_Transmit(p_data, strlen((char*)p_data));
  Display_ShowMain_Bg();
  for(;;) {
    Display_ShowMain_Measured();
    Display_ShowMain_SetValues();
    osDelay(150);
  }
}

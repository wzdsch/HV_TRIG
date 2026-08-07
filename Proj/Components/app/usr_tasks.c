/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-07-30 17:22:02
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-08-07 11:43:40
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

Display_State_e g_display_state = DISPLAY_STATE_MAIN;
Display_OutState_e g_display_out_state = DISPLAY_OUTSTATE_OFF;
Display_SelBit_e g_cursor = DISPLAY_SELBIT_NONE;
Display_SettingsRow_e g_settings_row = DISPLAY_SETTINGS_ROW_DEFAULT;
Display_MainData_t g_main_data = {
  .set_voltage = 0,
  .set_freq = 0,
  .set_duty = 0,
  .measured_voltage = 0,
  .measured_current = 0,
  .measured_power = 0,
  .input_freq = 0,
  .output_enabled = DISPLAY_OUTSTATE_OFF
};
Display_SettingsData_t g_settings_data = {
  .default_preset = {
    .voltage = 0,
    .freq = 0,
    .duty = 0
  },
  .preset1 = {
    .voltage = 0,
    .duty = 0,
    .freq = 0
  },
  .preset2 = {
    .voltage = 0,
    .duty = 0,
    .freq = 0
  }
};

void Main_Task(void *argument) {
  Display_Init(&g_display_state, &g_display_out_state ,&g_cursor, &g_main_data, \
               &g_settings_row, &g_settings_data);
  // Display_ShowMain_Bg();
  // for(;;) {
  //   uint32_t start_tick = osKernelGetTickCount();

  //   Display_ShowMain_Measured();
  //   Display_Show_CornerValues();
	// 	Display_ShowMain_OutState();

  //   osDelayUntil(start_tick + 200);
  // }
  // Display_ShowSettings_Bg();
  for(;;) {
    // uint32_t start_tick = osKernelGetTickCount();

    // Display_Show_CornerValues();

    // Display_ShowSettings_PresetGroup(DISPLAY_SETTINGS_ROW_DEFAULT);
    // Display_ShowSettings_PresetGroup(DISPLAY_SETTINGS_ROW_PRESET1);
    // Display_ShowSettings_PresetGroup(DISPLAY_SETTINGS_ROW_PRESET2);

    // osDelay(200);
  }
}

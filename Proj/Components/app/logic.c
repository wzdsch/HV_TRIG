/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-08-04 15:26:52
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-08-04 19:50:00
 * @FilePath: \code\Proj\Components\app\logic.c
 * @Description: 
 * 本项目只用于控制THz专用激励源的控制，切勿用于其他用途，否则西安理工大学及开发者不承担任何责任。
 * Copyright (c) 2026 by Jiang Tianhang 1919524828@qq.com, All Rights Reserved. 
 */
#include "logic.h"

Display_MainData_t main_data = {
  .set_voltage = 0,
  .set_frequency = 0,
  .set_duty_cycle = 0,
};

Display_SettingsData_t settings_data = {
  .default_preset.voltage = 0,
  .default_preset.frequency = 0,
  .default_preset.duty_cycle = 0,

  .preset1.voltage = 0,
  .preset1.frequency = 0,
  .preset1.duty_cycle = 0,

  .preset2.voltage = 0,
  .preset2.frequency = 0,
  .preset2.duty_cycle = 0,
};

Display_MainCursor_t main_cursor = DISPLAY_CURSOR_NONE;
Display_SettingsCol_t setting_sel_col = DISPLAY_SETTINGS_COL_NONE;
Display_SettingsRow_t setting_sel_row = DISPLAY_SETTINGS_ROW_CURRENT;


void Key0_Down_Logic(void) {

}

void Key3_Down_Logic(void) {
  
}

void Key4_Down_Logic(void) {
  
}

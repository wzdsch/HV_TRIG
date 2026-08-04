/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-08-04 09:34:09
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-08-04 22:14:41
 * @FilePath: \code\Proj\Components\module\display.c
 * @Description: MF022 UART串口屏显示模块实现
 *  通过BSP_UART队列发送AT指令控制MF022串口屏（220x176）
 *  命令间用';'分隔，以'\r\n'结尾
 *  本项目只用于控制THz专用激励源的控制，切勿用于其他用途，否则西安理工大学及开发者不承担任何责任。
 * Copyright (c) 2026 by Jiang Tianhang 1919524828@qq.com, All Rights Reserved. 
 */
#include "display.h"
#include "bsp_uart.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// ============================================================
// 图片资源地址（预先烧录在屏幕Flash中）
// ============================================================
#define IMG_MAIN_BG     2099960   // 主界面背景 220x176
#define IMG_SETTINGS_BG 2177400   // 设置界面背景 220x176
#define IMG_OUTPUT_ON   2098556   // 输出开启图标 26x27
#define IMG_OUTPUT_OFF  2097152   // 输出关闭图标 26x27
#define IMG_TOP_LEFT    2099456   // 左上角图标 24x24
#define IMG_TOP_RIGHT   2098304   // 右上角图标 24x24

// ============================================================
// 颜色常量（参考原程序）
// ============================================================
#define COLOR_BLACK         0
#define COLOR_HIGHLIGHT     1    // 设置界面选中位高亮色
#define COLOR_MEAS_POWER    3    // 实测功率颜色
#define COLOR_SETTINGS_HL   7    // 设置界面行高亮背景色
#define COLOR_MEAS_CURRENT  10   // 实测电流颜色
#define COLOR_MAIN_VALUE    13   // 主界面设定值背景色
#define COLOR_SETTINGS_BG   15   // 设置界面非高亮背景色
#define COLOR_MAIN_TOP      16   // 主界面上方设定值颜色
#define COLOR_CURSOR_BLINK  30   // 光标闪烁色

// ============================================================
// 屏幕布局坐标常量（参考原程序）
// ============================================================

// ---- 主界面上方（y=0~24） ----
#define MAIN_SET_V_X        54   // 设定电压起始X
#define MAIN_SET_V_Y        10   
#define MAIN_INPUT_F_X      132  // 输入频率起始X
#define MAIN_INPUT_F_Y      10

// ---- 主界面下方（y=151~176） ----
#define MAIN_SET_F_X        50   // 设定频率起始X
#define MAIN_SET_F_Y        151
#define MAIN_SET_D_X        140  // 设定占空比起始X
#define MAIN_SET_D_Y        151

// ---- 主界面左侧实测区（y=24~151） ----
#define MAIN_MEAS_V_X       88   // 实测电压X
#define MAIN_MEAS_V_Y       41
#define MAIN_MEAS_C_X       88   // 实测电流X
#define MAIN_MEAS_C_Y       76
#define MAIN_MEAS_P_X       82   // 实测功率X
#define MAIN_MEAS_P_Y       111

// ---- 输出图标 ----
#define MAIN_ICON_X         170
#define MAIN_ICON_Y         76
#define MAIN_ICON_W         26
#define MAIN_ICON_H         27

// ---- 设置界面 ----
#define SETTINGS_LABEL_X    15
#define SETTINGS_LABEL_Y1   38   // "当前设定"
#define SETTINGS_LABEL_Y2   72   // "预设1"
#define SETTINGS_LABEL_Y3   108  // "预设2"

#define SETTINGS_ROW_Y1     54   // 当前设定数据行
#define SETTINGS_ROW_Y2     88   // 预设1数据行
#define SETTINGS_ROW_Y3     124  // 预设2数据行

// 每行数据位X坐标: V百,V十,V个,  F十,F个,F小,  D十,D个
#define SETTINGS_V100_X     15
#define SETTINGS_V10_X      23
#define SETTINGS_V1_X       31
#define SETTINGS_F10_X      55
#define SETTINGS_F1_X       63
#define SETTINGS_F01_X      79
#define SETTINGS_D10_X      103
#define SETTINGS_D1_X       111

// ============================================================
// 模块内部状态
// ============================================================
static Display_State_t        s_display_state  = DISPLAY_STATE_MAIN;
static Display_MainCursor_t   s_display_main_cursor    = DISPLAY_CURSOR_NONE;
static Display_SettingsRow_t  s_display_settings_row   = DISPLAY_SETTINGS_ROW_CURRENT;
static uint8_t                s_display_settings_col   = DISPLAY_SETTINGS_COL_NONE;  // 0~8, 0=未选择任何位

// 缓存当前的设定值，用于光标切换时恢复显示
static Display_MainData_t     cached_main_data;
static Display_SettingsData_t cached_settings_data;

// 互斥锁
static osMutexId_t display_mutex;

// 命令缓冲
static char cmd_buf[512];

// ============================================================
// 内部辅助函数
// ============================================================

/// @brief 发送命令字符串到屏幕（通过BSP_UART队列，非阻塞）
static void Display_SendCmd(const char *cmd) {
  if (cmd == NULL || cmd[0] == '\0') return;
  BSP_UART_Transmit((uint8_t *)cmd, (uint8_t)strlen(cmd));
}

/// @brief 格式化发送命令字符串
static void Display_SendCmdF(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int len = vsnprintf(cmd_buf, sizeof(cmd_buf), fmt, args);
  va_end(args);
  if (len > 0 && len < (int)sizeof(cmd_buf)) {
    BSP_UART_Transmit((uint8_t *)cmd_buf, (uint8_t)len);
  }
}

/// @brief 获取设置界面某行的Y坐标
static uint16_t Display_GetSettingsRowY(uint8_t row) {
  switch (row) {
    case 1: return SETTINGS_ROW_Y1;
    case 2: return SETTINGS_ROW_Y2;
    case 3: return SETTINGS_ROW_Y3;
    default: return SETTINGS_ROW_Y1;
  }
}

/// @brief 获取设置界面某行的标签Y坐标
static uint16_t Display_GetSettingsLabelY(uint8_t row) {
  switch (row) {
    case 1: return SETTINGS_LABEL_Y1;
    case 2: return SETTINGS_LABEL_Y2;
    case 3: return SETTINGS_LABEL_Y3;
    default: return SETTINGS_LABEL_Y1;
  }
}

/// @brief 构建单行预设数据命令（追加到cmd_buf，不发送）
/// @param buf 目标缓冲区
/// @param row_y 行Y坐标
/// @param bg_color 背景色（SBC参数）
/// @param digit_color 数字颜色
/// @param selected_col 当前选中位序号（1~8，0=未选择任何位）
/// @param data 预设数据
/// @return 写入的字符数（不含结尾null）
static int Display_BuildPresetRow(char *buf,
                                   uint16_t row_y, uint8_t bg_color, uint8_t digit_color,
                                   uint8_t selected_col,
                                   const Display_PresetData_t *data) {
  uint8_t c1 = (selected_col == 1) ? 1 : digit_color;
  uint8_t c2 = (selected_col == 2) ? 1 : digit_color;
  uint8_t c3 = (selected_col == 3) ? 1 : digit_color;
  uint8_t c4 = (selected_col == 4) ? 1 : digit_color;
  uint8_t c5 = (selected_col == 5) ? 1 : digit_color;
  uint8_t c6 = (selected_col == 6) ? 1 : digit_color;
  uint8_t c7 = (selected_col == 7) ? 1 : digit_color;
  uint8_t c8 = (selected_col == 8) ? 1 : digit_color;
  return sprintf(buf,
    "SBC(%d);"
    "DCV16(%d,%d,'%d',%d);DCV16(%d,%d,'%d',%d);DCV16(%d,%d,'%d',%d);"
    "DCV16(%d,%d,'%d',%d);DCV16(%d,%d,'%d',%d);DCV16(%d,%d,'%d',%d);"
    "DCV16(%d,%d,'%d',%d);DCV16(%d,%d,'%d',%d)",
    bg_color,
    SETTINGS_V100_X, row_y, data->voltage / 100,         c1,
    SETTINGS_V10_X,  row_y, (data->voltage / 10) % 10,   c2,
    SETTINGS_V1_X,   row_y, data->voltage % 10,          c3,
    SETTINGS_F10_X,  row_y, data->frequency / 100,       c4,
    SETTINGS_F1_X,   row_y, (data->frequency / 10) % 10, c5,
    SETTINGS_F01_X,  row_y, data->frequency % 10,        c6,
    SETTINGS_D10_X,  row_y, data->duty_cycle / 10,       c7,
    SETTINGS_D1_X,   row_y, data->duty_cycle % 10,       c8
  );
}

/// @brief 刷新主界面顶部全部设定值（与参考程序 shuaxinxianshi(1) 一致）
static void Display_RefreshMainTopValues(const Display_MainData_t *data) {
  int len = sprintf(cmd_buf,
    "SBC(13);"
    "DCV16(54,10,'%d',16);DCV16(62,10,'%d',16);DCV16(70,10,'%d',16);"
    "DCV16(50,151,'%d',16);DCV16(58,151,'%d',16);DCV16(74,151,'%d',16);"
    "DCV16(140,151,'%d',16);DCV16(148,151,'%d',16);\r\n",
    data->set_voltage / 100, (data->set_voltage / 10) % 10, data->set_voltage % 10,
    data->set_frequency / 100, (data->set_frequency / 10) % 10, data->set_frequency % 10,
    data->set_duty_cycle / 10, data->set_duty_cycle % 10
  );
  if (len > 0) BSP_UART_Transmit((uint8_t *)cmd_buf, (uint8_t)len);
}

/// @brief 刷新主界面实测数据（与参考程序 DigDisplay 一致）
static void Display_RefreshMainMeasured(const Display_MainData_t *data) {
  uint16_t v = (uint16_t)data->measured_voltage;
  uint16_t c = (uint16_t)data->measured_current;
  uint16_t p_int = (uint16_t)data->measured_power;
  uint8_t  p_dec = (uint8_t)((data->measured_power - (float)p_int) * 10.0f + 0.5f);
  int len = sprintf(cmd_buf,
    "SBC(7);"
    "DCV24(88,41,'%d',1);DCV24(100,41,'%d',1);DCV24(112,41,'%d',1);"
    "DCV24(88,76,'%d',10);DCV24(100,76,'%d',10);DCV24(112,76,'%d',10);"
    "DCV24(82,111,'%d',3);DCV24(94,111,'%d',3);DCV24(118,111,'%d',3);\r\n",
    v / 100, (v / 10) % 10, v % 10,
    c / 100, (c / 10) % 10, c % 10,
    p_int / 10, p_int % 10, p_dec
  );
  if (len > 0) BSP_UART_Transmit((uint8_t *)cmd_buf, (uint8_t)len);
}

// ============================================================
// API实现
// ============================================================

void Display_Init(void) {
  // 创建互斥锁
  osMutexAttr_t mutex_attr = {
    .name = "display_mutex",
    .attr_bits = osMutexRecursive,
  };
  display_mutex = osMutexNew(&mutex_attr);

  // 初始化缓存数据
  memset(&cached_main_data, 0, sizeof(cached_main_data));
  memset(&cached_settings_data, 0, sizeof(cached_settings_data));
}

// ---- 页面切换 ----

void Display_ShowMain(void) {
  osMutexAcquire(display_mutex, osWaitForever);

  s_display_state = DISPLAY_STATE_MAIN;
  s_display_main_cursor = DISPLAY_CURSOR_NONE;

  // 第一帧：背景+占位值（与参考程序完全一致）
  int len = sprintf(cmd_buf,
    "CLR(0);DIR(1);"
    "FSIMG(%d,0,0,220,176,0);"
    "SBC(13);"
    "DCV16(54,10,'000',16);DCV16(50,151,'00.0',16);"
    "DCV16(132,10,'00.0',16);DCV16(140,151,'50',16);"
    "SBC(7);"
    "DCV24(88,41,'000',1);DCV24(88,76,'000',10);"
    "DCV24(82,111,'00.0',3);\r\n",
    IMG_MAIN_BG
  );
  if (len > 0) BSP_UART_Transmit((uint8_t *)cmd_buf, (uint8_t)len);

  // 第二帧：实际设定值
  Display_RefreshMainTopValues(&cached_main_data);

  // 输出图标
  Display_SetOutputIcon(cached_main_data.output_enabled);

  osMutexRelease(display_mutex);
}

void Display_ShowSettings(void) {
  osMutexAcquire(display_mutex, osWaitForever);

  s_display_state = DISPLAY_STATE_SETTINGS;
  s_display_settings_row  = DISPLAY_SETTINGS_ROW_CURRENT;
  s_display_settings_col  = DISPLAY_SETTINGS_COL_NONE;

  // 与参考程序完全一致：SBC(15)背景+标签+SBC(13)顶部占位
  int len = sprintf(cmd_buf,
    "SBC(15);"
    "FSIMG(%d,0,0,220,176,0);"
    "DCV16(15,38,'NowSet',0);"
    "DCV16(15,72,'Preset1',0);"
    "DCV16(15,108,'Preset2',0);"
    "SBC(13);"
    "DCV16(54,10,'000',16);DCV16(50,151,'00.0',16);"
    "DCV16(132,10,'00.0',16);DCV16(140,151,'50',16);\r\n",
    IMG_SETTINGS_BG
  );
  if (len > 0) BSP_UART_Transmit((uint8_t *)cmd_buf, (uint8_t)len);

  // 实际设定值
  Display_RefreshMainTopValues(&cached_main_data);

  // 显示3行预设（第1行高亮）
  Display_RefreshSettingsRow(1, 1);
  Display_RefreshSettingsRow(2, 0);
  Display_RefreshSettingsRow(3, 0);

  osMutexRelease(display_mutex);
}

Display_State_t Display_GetState(void) {
  return s_display_state;
}

// ---- 数据更新 ----

void Display_UpdateMainData(const Display_MainData_t *data) {
  osMutexAcquire(display_mutex, osWaitForever);
  memcpy(&cached_main_data, data, sizeof(Display_MainData_t));
  if (s_display_state != DISPLAY_STATE_MAIN) { osMutexRelease(display_mutex); return; }
  Display_RefreshMainTopValues(data);
  Display_RefreshMainMeasured(data);
  Display_RefreshInputFrequency(data->input_frequency);
  Display_SetOutputIcon(data->output_enabled);
  osMutexRelease(display_mutex);
}

void Display_UpdateSettingsData(const Display_SettingsData_t *data) {
  osMutexAcquire(display_mutex, osWaitForever);

  // 缓存数据
  memcpy(&cached_settings_data, data, sizeof(Display_SettingsData_t));

  if (s_display_state != DISPLAY_STATE_SETTINGS) {
    osMutexRelease(display_mutex);
    return;
  }

  // 刷新全部3行
  uint8_t hl = (s_display_settings_row == DISPLAY_SETTINGS_ROW_CURRENT) ? 1 : 0;
  Display_RefreshSettingsRow(1, hl);
  hl = (s_display_settings_row == DISPLAY_SETTINGS_ROW_PRESET1) ? 1 : 0;
  Display_RefreshSettingsRow(2, hl);
  hl = (s_display_settings_row == DISPLAY_SETTINGS_ROW_PRESET2) ? 1 : 0;
  Display_RefreshSettingsRow(3, hl);

  osMutexRelease(display_mutex);
}

// ---- 光标控制 ----

void Display_SetMainCursor(Display_MainCursor_t pos) {
  osMutexAcquire(display_mutex, osWaitForever);
  Display_MainCursor_t old = s_display_main_cursor;
  s_display_main_cursor = pos;

  // 恢复旧光标为正常色(SBC(13), 色16)
  if (old >= DISPLAY_CURSOR_V_HUND && old <= DISPLAY_CURSOR_V_ONES) {
    uint8_t idx = old - DISPLAY_CURSOR_V_HUND;
    uint8_t x = 54 + idx * 8, d;
    if (idx == 0) d = cached_main_data.set_voltage / 100;
    else if (idx == 1) d = (cached_main_data.set_voltage / 10) % 10;
    else d = cached_main_data.set_voltage % 10;
    Display_SendCmdF("SBC(13);DCV16(%d,10,'%d',16);\r\n", x, d);
  } else if (old >= DISPLAY_CURSOR_F_TENS && old <= DISPLAY_CURSOR_F_DEC) {
    uint8_t idx = old - DISPLAY_CURSOR_F_TENS;
    uint8_t x = (idx == 2) ? 74 : (50 + idx * 8), d;
    if (idx == 0) d = cached_main_data.set_frequency / 100;
    else if (idx == 1) d = (cached_main_data.set_frequency / 10) % 10;
    else d = cached_main_data.set_frequency % 10;
    Display_SendCmdF("SBC(13);DCV16(%d,151,'%d',16);\r\n", x, d);
  } else if (old >= DISPLAY_CURSOR_D_TENS && old <= DISPLAY_CURSOR_D_ONES) {
    uint8_t idx = old - DISPLAY_CURSOR_D_TENS;
    uint8_t d = (idx == 0) ? cached_main_data.set_duty_cycle / 10
                           : cached_main_data.set_duty_cycle % 10;
    Display_SendCmdF("SBC(13);DCV16(%d,151,'%d',16);\r\n", 140 + idx * 8, d);
  }

  // 新光标设为闪烁色(SBC(13), 色30)
  if (pos >= DISPLAY_CURSOR_V_HUND && pos <= DISPLAY_CURSOR_V_ONES) {
    uint8_t idx = pos - DISPLAY_CURSOR_V_HUND;
    uint8_t x = 54 + idx * 8, d;
    if (idx == 0) d = cached_main_data.set_voltage / 100;
    else if (idx == 1) d = (cached_main_data.set_voltage / 10) % 10;
    else d = cached_main_data.set_voltage % 10;
    Display_SendCmdF("SBC(13);DCV16(%d,10,'%d',30);\r\n", x, d);
  } else if (pos >= DISPLAY_CURSOR_F_TENS && pos <= DISPLAY_CURSOR_F_DEC) {
    uint8_t idx = pos - DISPLAY_CURSOR_F_TENS;
    uint8_t x = (idx == 2) ? 74 : (50 + idx * 8), d;
    if (idx == 0) d = cached_main_data.set_frequency / 100;
    else if (idx == 1) d = (cached_main_data.set_frequency / 10) % 10;
    else d = cached_main_data.set_frequency % 10;
    Display_SendCmdF("SBC(13);DCV16(%d,151,'%d',30);\r\n", x, d);
  } else if (pos >= DISPLAY_CURSOR_D_TENS && pos <= DISPLAY_CURSOR_D_ONES) {
    uint8_t idx = pos - DISPLAY_CURSOR_D_TENS;
    uint8_t d = (idx == 0) ? cached_main_data.set_duty_cycle / 10
                           : cached_main_data.set_duty_cycle % 10;
    Display_SendCmdF("SBC(13);DCV16(%d,151,'%d',30);\r\n", 140 + idx * 8, d);
  }
  osMutexRelease(display_mutex);
}

Display_MainCursor_t Display_GetMainCursor(void) {
  return s_display_main_cursor;
}

// ---- 设置界面选择控制 ----

void Display_SetSettingsRow(Display_SettingsRow_t row) {
  osMutexAcquire(display_mutex, osWaitForever);
  if (s_display_settings_row != row) {
    Display_RefreshSettingsRow(s_display_settings_row, 0);
    s_display_settings_row = row;
    Display_RefreshSettingsRow(s_display_settings_row, 1);
  }
  osMutexRelease(display_mutex);
}

void Display_SetSettingsColumn(Display_SettingsCol_t col) {
  osMutexAcquire(display_mutex, osWaitForever);
  s_display_settings_col = col;
  Display_RefreshSettingsRow(s_display_settings_row, 1);
  osMutexRelease(display_mutex);
}

Display_SettingsRow_t Display_GetSettingsRow(void) { return s_display_settings_row; }
uint8_t Display_GetSettingsColumn(void) { return s_display_settings_col; }

// ---- 局部刷新 ----

void Display_RefreshSetVoltage(uint16_t voltage) {
  osMutexAcquire(display_mutex, osWaitForever);
  cached_main_data.set_voltage = voltage;
  if (s_display_state == DISPLAY_STATE_MAIN) Display_RefreshMainTopValues(&cached_main_data);
  osMutexRelease(display_mutex);
}

void Display_RefreshSetFrequency(uint16_t frequency) {
  osMutexAcquire(display_mutex, osWaitForever);
  cached_main_data.set_frequency = frequency;
  if (s_display_state == DISPLAY_STATE_MAIN) Display_RefreshMainTopValues(&cached_main_data);
  osMutexRelease(display_mutex);
}

void Display_RefreshSetDuty(uint8_t duty) {
  osMutexAcquire(display_mutex, osWaitForever);
  cached_main_data.set_duty_cycle = duty;
  if (s_display_state == DISPLAY_STATE_MAIN) Display_RefreshMainTopValues(&cached_main_data);
  osMutexRelease(display_mutex);
}

void Display_RefreshMeasuredData(float voltage, float current, float power) {
  osMutexAcquire(display_mutex, osWaitForever);
  cached_main_data.measured_voltage = voltage;
  cached_main_data.measured_current = current;
  cached_main_data.measured_power = power;
  if (s_display_state == DISPLAY_STATE_MAIN) Display_RefreshMainMeasured(&cached_main_data);
  osMutexRelease(display_mutex);
}

void Display_RefreshInputFrequency(uint16_t frequency) {
  osMutexAcquire(display_mutex, osWaitForever);
  cached_main_data.input_frequency = frequency;
  if (s_display_state != DISPLAY_STATE_MAIN) { osMutexRelease(display_mutex); return; }
  uint16_t f = frequency / 10;
  Display_SendCmdF("SBC(13);DCV16(132,10,'%d',16);DCV16(140,10,'%d',16);DCV16(156,10,'%d',16);\r\n",
                   f / 10, f % 10, frequency % 10);
  osMutexRelease(display_mutex);
}

void Display_SetOutputIcon(uint8_t enabled) {
  osMutexAcquire(display_mutex, osWaitForever);
  cached_main_data.output_enabled = enabled;
  if (s_display_state != DISPLAY_STATE_MAIN) { osMutexRelease(display_mutex); return; }
  Display_SendCmdF("FSIMG(%d,170,76,26,27,0);\r\n", enabled ? IMG_OUTPUT_ON : IMG_OUTPUT_OFF);
  osMutexRelease(display_mutex);
}

void Display_RefreshSettingsRow(uint8_t row, uint8_t highlighted) {
  osMutexAcquire(display_mutex, osWaitForever);
  if (s_display_state != DISPLAY_STATE_SETTINGS) { osMutexRelease(display_mutex); return; }

  const Display_PresetData_t *data = NULL;
  switch (row) {
    case 1: data = &cached_settings_data.default_preset; break;
    case 2: data = &cached_settings_data.preset1; break;
    case 3: data = &cached_settings_data.preset2; break;
    default: osMutexRelease(display_mutex); return;
  }

  uint16_t row_y = Display_GetSettingsRowY(row);
  uint8_t bg_color = highlighted ? 7 : 15;
  uint8_t digit_color = 3;
  uint8_t selected_col = highlighted ? s_display_settings_col : 0;

  int len = Display_BuildPresetRow(cmd_buf,
                                    row_y, bg_color, digit_color, selected_col, data);
  if (len > 0 && len < (int)sizeof(cmd_buf) - 3) {
    cmd_buf[len] = '\r';
    cmd_buf[len + 1] = '\n';
    cmd_buf[len + 2] = '\0';
    BSP_UART_Transmit((uint8_t *)cmd_buf, (uint8_t)(len + 2));
  }
  osMutexRelease(display_mutex);
}

void Display_RefreshSettingsDigit(uint8_t row, uint8_t col, uint8_t value) {
  osMutexAcquire(display_mutex, osWaitForever);

  if (s_display_state != DISPLAY_STATE_SETTINGS || value > 9) {
    osMutexRelease(display_mutex);
    return;
  }

  uint16_t row_y = Display_GetSettingsRowY(row);
  uint16_t x = 0;
  switch (col) {
    case 1: x = SETTINGS_V100_X; break;
    case 2: x = SETTINGS_V10_X;  break;
    case 3: x = SETTINGS_V1_X;   break;
    case 4: x = SETTINGS_F10_X;  break;
    case 5: x = SETTINGS_F1_X;   break;
    case 6: x = SETTINGS_F01_X;  break;
    case 7: x = SETTINGS_D10_X;  break;
    case 8: x = SETTINGS_D1_X;   break;
    default: osMutexRelease(display_mutex); return;
  }

  Display_SendCmdF("SBC(7);DCV16(%d,%d,'%d',1);\r\n", x, row_y, value);

  osMutexRelease(display_mutex);
}

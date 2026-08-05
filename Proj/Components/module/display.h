/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-08-04 09:34:20
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-08-05 18:56:46
 * @FilePath: \code\Proj\Components\module\display.h
 * @Description: MF022 UART串口屏显示模块
 *  屏幕分辨率: 220x176
 *  通过UART1发送AT指令控制显示
 *  本项目只用于控制THz专用激励源的控制，切勿用于其他用途，否则西安理工大学及开发者不承担任何责任。
 * Copyright (c) 2026 by Jiang Tianhang 1919524828@qq.com, All Rights Reserved. 
 */
#ifndef DISPLAY_H
#define DISPLAY_H

#include "main.h"
#include <stdint.h>

// ============================================================
// 显示状态枚举
// ============================================================

/// @brief 显示页面状态
typedef enum {
  DISPLAY_STATE_MAIN     = 1,  // 主界面
  DISPLAY_STATE_SETTINGS = 2,  // 设置界面
} Display_State_t;

/// @brief 设置界面行选择（预设组）
typedef enum {
  DISPLAY_SETTINGS_ROW_CURRENT = 1,  // 当前设定
  DISPLAY_SETTINGS_ROW_PRESET1 = 2,  // 预设1
  DISPLAY_SETTINGS_ROW_PRESET2 = 3,  // 预设2
} Display_SettingsRow_t;

/// @brief 主界面光标位置
typedef enum {
  DISPLAY_CURSOR_NONE    = 0,  // 无光标，未选中
  DISPLAY_CURSOR_V_HUND  = 1,  // 电压-百位
  DISPLAY_CURSOR_V_TENS  = 2,  // 电压-十位
  DISPLAY_CURSOR_V_ONES  = 3,  // 电压-个位
  DISPLAY_CURSOR_F_TENS  = 4,  // 频率-十位
  DISPLAY_CURSOR_F_ONES  = 5,  // 频率-个位 
  DISPLAY_CURSOR_F_DEC   = 6,  // 频率-小数位
  DISPLAY_CURSOR_D_TENS  = 7,  // 占空比-十位
  DISPLAY_CURSOR_D_ONES  = 8,  // 占空比-个位
} Display_MainCursor_t;

/// @brief 设置界面列选择
typedef enum {
  DISPLAY_SETTINGS_COL_NONE     = 0,  // 未选择任何位
  DISPLAY_SETTINGS_COL_VOLTAGE  = 1,  // 电压列（3位）
  DISPLAY_SETTINGS_COL_FREQ     = 4,  // 频率列（xx.x）
  DISPLAY_SETTINGS_COL_DUTY     = 7,  // 占空比列（2位）
} Display_SettingsCol_t;

// ============================================================
// 显示数据结构
// ============================================================

/// @brief 主界面显示数据
typedef struct {
  uint16_t set_voltage;         // 设定电压 3位 0~400 V
  uint16_t set_frequency;       // 设定频率 3位 0~500 (单位：0.1kHz)
  uint8_t  set_duty_cycle;      // 设定占空比 2位 0~99 %

  uint16_t measured_voltage;    // 实测电压 3位 0~400 V
  uint16_t measured_current;    // 实测电流 3位 0~25 mA (单位：0.1mA)
  uint16_t measured_power;      // 实测功率 3位 0~xx.x W (单位：0.1W)

  uint16_t input_frequency;  // 输入TTL频率 3位 (单位：0.1kHz)

  uint8_t  output_enabled;      // 输出状态: 0=关闭, 1=开启
} Display_MainData_t;

/// @brief 单组预设数据
typedef struct {
  uint16_t voltage;     // 电压 0~400 V
  uint16_t frequency;   // 频率 0~500 (x0.1kHz)
  uint8_t  duty_cycle;  // 占空比 0~99 %
} Display_PresetData_t;

/// @brief 设置界面数据（3组预设）
typedef struct {
  Display_PresetData_t default_preset;   // 默认开机设定
  Display_PresetData_t preset1;   // 预设1
  Display_PresetData_t preset2;   // 预设2
} Display_SettingsData_t;

/// @brief 初始化显示模块（创建互斥锁）
void Display_Init(void);

// ---- 页面切换 ----

/// @brief 切换到主界面
void Display_ShowMain(void);

/// @brief 切换到设置界面
void Display_ShowSettings(void);

// ---- 主界面数据更新 ----
void Display_ShowMain_Bg(void);
void Display_ShowMain_SetValues(void);
void Display_ShowMain_Measured(void);

#endif // DISPLAY_H

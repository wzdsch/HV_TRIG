/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-08-04 09:34:20
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-08-07 21:32:22
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
} Display_State_e;

/// @brief 输出的状态
typedef enum {
  DISPLAY_OUTSTATE_OFF = 0, // 输出开启
  DISPLAY_OUTSTATE_ON = 1   // 输出关闭
} Display_OutState_e;

/// @brief 设置界面行选择（预设组）
typedef enum {
  DISPLAY_SETTINGS_ROW_DEFAULT = 1,  // 默认预设
  DISPLAY_SETTINGS_ROW_PRESET1 = 2,  // 预设1
  DISPLAY_SETTINGS_ROW_PRESET2 = 3,  // 预设2
} Display_SettingsRow_e;

/// @brief 主界面光标位置
typedef enum {
  DISPLAY_SELBIT_NONE    = 0,  // 无光标，未选中
  DISPLAY_SELBIT_VOL100  = 1,  // 电压-百位
  DISPLAY_SELBIT_VOL10   = 2,  // 电压-十位
  DISPLAY_SELBIT_VOL1    = 3,  // 电压-个位
  DISPLAY_SELBIT_FREQ100 = 4,  // 频率-十位
  DISPLAY_SELBIT_FREQ10  = 5,  // 频率-个位 
  DISPLAY_SELBIT_FREQ1   = 6,  // 频率-小数位
  DISPLAY_SELBIT_DUTY10  = 7,  // 占空比-十位
  DISPLAY_SELBIT_DUTY1   = 8,  // 占空比-个位
} Display_SelBit_e;

// ============================================================
// 显示数据结构
// ============================================================

/// @brief 主界面显示数据
typedef struct {
  uint16_t set_voltage;         // 设定电压 3位 0~400 V
  uint16_t set_freq;       // 设定频率 3位 0~500 (单位：0.1kHz)
  uint8_t  set_duty;      // 设定占空比 2位 0~99 %

  uint16_t measured_voltage;    // 实测电压 3位 0~400 V
  uint16_t measured_current;    // 实测电流 3位 0~25 mA (单位：0.1mA)
  uint16_t measured_power;      // 实测功率 3位 0~xx.x W (单位：0.1W)

  uint16_t input_freq;  // 输入TTL频率 3位 (单位：0.1kHz)
  Display_OutState_e  output_enabled;      // 输出状态
} Display_MainData_t;

/// @brief 单组预设数据
typedef struct {
  uint16_t voltage;     // 电压 0~400 V
  uint16_t freq;   // 频率 0~500 (x0.1kHz)
  uint8_t  duty;  // 占空比 0~99 %
} Display_PresetData_t;

/// @brief 设置界面数据（3组预设）
typedef struct {
  Display_PresetData_t default_preset;   // 默认开机设定
  Display_PresetData_t preset1;   // 预设1
  Display_PresetData_t preset2;   // 预设2
} Display_SettingsData_t;

/// @brief 初始化显示模块, 创建互斥锁, 给定数据指针
void Display_Init(  const Display_State_e* gp_display_state,              \
                    const Display_OutState_e* gp_display_out_state,       \
                    const Display_SelBit_e* gp_display_SELBIT,            \
                    const Display_MainData_t* gp_display_main_data,       \
                    const Display_SettingsRow_e* gp_display_settings_row, \
                    const Display_SettingsData_t* gp_display_settings_data
                  );
#endif

/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-08-04 09:34:20
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-08-04 15:51:30
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
  uint16_t set_voltage;       // 设定电压 0~400 V
  uint16_t set_frequency;     // 设定频率 0~500 (x0.1kHz)
  uint8_t  set_duty_cycle;    // 设定占空比 0~99 %

  float    measured_voltage;  // 实测电压 0~400.0 V
  float    measured_current;  // 实测电流 0~25.0 mA
  float    measured_power;    // 实测功率 0~xx.x W

  uint16_t input_frequency;   // 输入TTL频率 (x0.1kHz)

  uint8_t  output_enabled;    // 输出状态: 0=关闭, 1=开启
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

// ============================================================
// API函数声明
// ============================================================

/// @brief 初始化显示模块（创建互斥锁）
void Display_Init(void);

// ---- 页面切换 ----

/// @brief 切换到主界面
void Display_ShowMain(void);

/// @brief 切换到设置界面
void Display_ShowSettings(void);

/// @brief 获取当前显示状态
/// @return 当前页面状态
Display_State_t Display_GetState(void);

// ---- 数据更新 ----

/// @brief 更新主界面全部数据
/// @param data 主界面显示数据指针
void Display_UpdateMainData(const Display_MainData_t *data);

/// @brief 更新设置界面全部预设数据
/// @param data 设置界面数据指针
void Display_UpdateSettingsData(const Display_SettingsData_t *data);

// ---- 光标控制（主界面） ----

/// @brief 设置主界面编辑光标位置
/// @param pos 光标位置 (DISPLAY_MAIN_CURSOR_NONE 或 1~8)
void Display_SetMainCursor(Display_MainCursor_t pos);

/// @brief 获取主界面当前光标位置
/// @return 光标位置
Display_MainCursor_t Display_GetMainCursor(void);

// ---- 设置界面选择控制 ----

/// @brief 设置设置界面当前选中行
/// @param row 行号
void Display_SetSettingsRow(Display_SettingsRow_t row);

/// @brief 设置设置界面当前选中列
/// @param col 列号（DISPLAY_SETTINGS_COL_NONE=0 表示未选择任何位）
void Display_SetSettingsColumn(Display_SettingsCol_t col);

/// @brief 获取设置界面当前选中行
/// @return 行号
Display_SettingsRow_t Display_GetSettingsRow(void);

/// @brief 获取设置界面当前选中列
/// @return 列号（0=未选择任何位）
uint8_t Display_GetSettingsColumn(void);

// ---- 局部刷新 ----

/// @brief 刷新主界面设定电压区域
void Display_RefreshSetVoltage(uint16_t voltage);

/// @brief 刷新主界面设定频率区域
void Display_RefreshSetFrequency(uint16_t frequency);

/// @brief 刷新主界面设定占空比区域
void Display_RefreshSetDuty(uint8_t duty);

/// @brief 刷新主界面实测数据（电压/电流/功率）
void Display_RefreshMeasuredData(float voltage, float current, float power);

/// @brief 刷新主界面输入频率显示
void Display_RefreshInputFrequency(uint16_t frequency);

/// @brief 刷新主界面输出开关图标
/// @param enabled 0=关, 1=开
void Display_SetOutputIcon(uint8_t enabled);

/// @brief 刷新设置界面某一行（带高亮/非高亮状态）
/// @param row 行号 1=当前设定, 2=预设1, 3=预设2
/// @param highlighted 1=高亮选中, 0=非高亮
void Display_RefreshSettingsRow(uint8_t row, uint8_t highlighted);

/// @brief 刷新设置界面某一行的单个数字位
/// @param row 行号
/// @param col 位序号(1~8: V百/V十/V个/F十/F个/F小/D十/D个)
/// @param value 数字值(0~9)
void Display_RefreshSettingsDigit(uint8_t row, uint8_t col, uint8_t value);

#endif // DISPLAY_H

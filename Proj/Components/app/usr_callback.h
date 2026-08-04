/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-08-04 15:24:51
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-08-04 15:25:12
 * @FilePath: \code\Proj\Components\app\usr_callback.h
 * @Description: 按键和编码器的回调函数声明与注册接口
 * 本项目只用于控制THz专用激励源的控制，切勿用于其他用途，否则西安理工大学及开发者不承担任何责任。
 * Copyright (c) 2026 by Jiang Tianhang 1919524828@qq.com, All Rights Reserved. 
 */
#ifndef USR_CALLBACK_H
#define USR_CALLBACK_H

#include "stdint.h"

/// @brief 注册所有按键和编码器的回调函数（在 Key_Init 和 Encoder_Init 之后调用）
/// @param 无
/// @retval 无
void Usr_Callback_Register(void);

// ===== 按键按下回调（供注册，也可直接在模块内部使用）=====
void Key0_Down_Handler(void);   // K0: Enter 按下
void Key1_Down_Handler(void);   // K1: T/Down 按下
void Key2_Down_Handler(void);   // K2: V/UP 按下
void Key3_Down_Handler(void);   // K3: Menu 按下
void Key4_Down_Handler(void);   // K4: ADJ/SET 按下
void Key5_Down_Handler(void);   // K5: ON/OFF 按下

// ===== 按键释放回调 =====
void Key0_Up_Handler(void);     // K0: Enter 释放
void Key1_Up_Handler(void);     // K1: T/Down 释放
void Key2_Up_Handler(void);     // K2: V/UP 释放
void Key3_Up_Handler(void);     // K3: Menu 释放
void Key4_Up_Handler(void);     // K4: ADJ/SET 释放
void Key5_Up_Handler(void);     // K5: ON/OFF 释放

// ===== 编码器回调 =====
/// @brief 编码器每次轮询周期（10ms）的回调
/// @param delta 本次增量（正数=正向旋转，负数=反向旋转）
void Encoder_Delta_Handler(int32_t delta);

#endif
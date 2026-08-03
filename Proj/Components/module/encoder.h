/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-08-03 09:52:09
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-08-03 10:13:56
 * @FilePath: \code\Proj\Components\module\encoder.h
 * @Description: 
 * 本项目只用于控制THz专用激励源的控制，切勿用于其他用途，否则西安理工大学及开发者不承担任何责任。
 * Copyright (c) 2026 by Jiang Tianhang 1919524828@qq.com, All Rights Reserved. 
 */
#ifndef ENCODER_H
#define ENCODER_H

#include "main.h"

/// @brief Initialize encoder module, start encoder peripheral and create RTOS task
void Encoder_Init(void);

/// @brief Start encoder counting (enable encoder timer)
void Encoder_Start(void);

/// @brief Stop encoder counting (disable encoder timer)
void Encoder_Stop(void);

/// @brief Get encoder running state
/// @return running: 1, stop: 0
uint8_t Encoder_GetState(void);

/// @brief Get accumulated encoder position count (32-bit, handles 16-bit overflow)
/// @return accumulated encoder count (positive: forward, negative: reverse)
int32_t Encoder_GetCount(void);

/// @brief Reset accumulated encoder count to zero
void Encoder_ResetCount(void);

/// @brief Set callback for encoder data update
/// @param callback function pointer, called each polling cycle with delta count
void Encoder_SetCallback(void (*callback)(int32_t delta));

#endif

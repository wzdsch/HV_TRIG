/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-08-01 11:11:38
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-08-03 09:17:58
 * @FilePath: \code\Proj\Components\moudle\dac_ctrl.h
 * @Description: 
 * 本项目只用于控制THz专用激励源的控制，切勿用于其他用途，否则西安理工大学及开发者不承担任何责任。
 * Copyright (c) 2026 by Jiang Tianhang 1919524828@qq.com, All Rights Reserved. 
 */
#ifndef DAC_CTRL_H
#define DAC_CTRL_H

#include "main.h"

/// @brief Start DAC converter using DMA
void DAC_Ctrl_Start(void);

/// @brief Stop DAC converter and DMA
void DAC_Ctrl_Stop(void);

/// @brief Get the state of the DAC converter
/// @return running: 1, stop: 0
uint8_t DAC_Ctrl_GetState(void);

/// @brief Set the digital value of the DAC converter
/// @param value original digital value, 0~4000
void DAC_Ctrl_SetDigitalValue(uint16_t value);

/// @brief Set the analog voltage of the DAC converter
/// @param voltage processed analog output voltage, 0~200V
void DAC_Ctrl_SetAnalogVoltage(float voltage);

#endif

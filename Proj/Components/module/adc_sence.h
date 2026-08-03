/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-08-01 11:03:02
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-08-01 15:28:13
 * @FilePath: \code\Proj\Components\moudle\adc_sence.h
 * @Description: 
 * 本项目只用于控制THz专用激励源的控制，切勿用于其他用途，否则西安理工大学及开发者不承担任何责任。
 * Copyright (c) 2026 by Jiang Tianhang 1919524828@qq.com, All Rights Reserved. 
 */
#ifndef ADC_SENCE_H
#define ADC_SENCE_H

#include "main.h"


/// @brief Start ADC sence
void ADC_Sence_Start(void);

/// @brief Stop ADC sence
void ADC_Sence_Stop(void);

/// @brief Get weather ADC sence is running
/// @return running : 1, stop : 0
uint8_t ADC_Sence_GetState(void);

/// @brief Get the digital voltage value sensed from the HV module output.
/// @return Original digital voltage value
uint16_t ADC_Sence_GetDigitalVoltage(void);

/// @brief Get the analog voltage value sensed from the HV module output.
/// @return Processed output voltage value
float ADC_Sence_GetAnalogVoltage(void);

/// @brief Get the digital current value sensed from the HV module output.
/// @return Original digital current value
uint16_t ADC_Sence_GetDigitalCurrent(void);

/// @brief Get the analog current value sensed from the HV module output.
/// @return processed output current value
float ADC_Sence_GetAnalogCurrent(void);

#endif

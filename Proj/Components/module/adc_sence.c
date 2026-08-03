/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-08-01 11:02:58
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-08-03 10:41:57
 * @FilePath: \code\Proj\Components\module\adc_sence.c
 * @Description: 
 * 本项目只用于控制THz专用激励源的控制，切勿用于其他用途，否则西安理工大学及开发者不承担任何责任。
 * Copyright (c) 2026 by Jiang Tianhang 1919524828@qq.com, All Rights Reserved. 
 */
#include "adc_sence.h"
#include "adc.h"

#define GAIN_U_D2A 0.05037f
#define GAIN_I_D2A 0.01685f

static uint8_t adc_sence_flg = 0;

volatile static uint16_t adc1_values[2];

void ADC_Sence_Start() {
  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc1_values, 2);
  adc_sence_flg = 1;
}

void ADC_Sence_Stop() {
  HAL_ADC_Stop_DMA(&hadc1);
  adc_sence_flg = 0;
}

uint8_t ADC_Sence_GetState() {
  return adc_sence_flg;
}

uint16_t ADC_Sence_GetDigitalVoltage() {
  return adc1_values[0];
}

float ADC_Sence_GetAnalogVoltage() {
  return adc1_values[0] * GAIN_U_D2A;
}

uint16_t ADC_Sence_GetDigitalCurrent() {
  return adc1_values[1];
}

float ADC_Sence_GetAnalogCurrent() {
  return adc1_values[1] * GAIN_I_D2A;
}

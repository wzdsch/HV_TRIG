/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-08-01 11:11:38
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-08-01 15:56:22
 * @FilePath: \code\Proj\Components\moudle\dac_ctrl.c
 * @Description: 
 * 本项目只用于控制THz专用激励源的控制，切勿用于其他用途，否则西安理工大学及开发者不承担任何责任。
 * Copyright (c) 2026 by Jiang Tianhang 1919524828@qq.com, All Rights Reserved. 
 */
#include "dac_ctrl.h"
#include "dac.h"

#define GAIN_U_A2D 19.853f

#define DAC_MAX_VALUE 4000

static uint8_t dac_flg = 0;

static uint16_t dac_value = 0;

void DAC_Ctrl_Start() {
  HAL_DAC_Start_DMA(&hdac, DAC1_CHANNEL_1, (uint32_t*)&dac_value, 1, DAC_ALIGN_12B_R);
  dac_flg = 1;
}

void DAC_Ctrl_Stop() {
  HAL_DAC_Stop_DMA(&hdac, DAC1_CHANNEL_1);
  dac_flg = 0;
}

uint8_t DAC_Ctrl_GetState() {
  return dac_flg;
}

void DAC_Ctrl_SetDigitalValue(uint16_t value) {
  if (value > DAC_MAX_VALUE) {
    dac_value = DAC_MAX_VALUE;
  }
  else {
    dac_value = value;
  }
}

void DAC_Ctrl_SetAnalogVoltage(float voltage) {
  if (voltage < 0.f) {
    voltage = 0.f;
  }
  DAC_Ctrl_SetDigitalValue((uint16_t)(voltage * GAIN_U_A2D));
}

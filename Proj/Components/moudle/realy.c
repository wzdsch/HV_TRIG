/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-07-31 16:57:55
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-07-31 17:03:35
 * @FilePath: \code\Proj\Components\moudle\realy.c
 * @Description: 
 * 本项目只用于控制THz专用激励源的控制，切勿用于其他用途，否则西安理工大学及开发者不承担任何责任。
 * Copyright (c) 2026 by Jiang Tianhang 1919524828@qq.com, All Rights Reserved. 
 */
#include "realy.h"

static uint8_t realy_state = 0;

void Relay_On() {
  HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, GPIO_PIN_SET);
  realy_state = 1;
}

void Relay_Off() {
  HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, GPIO_PIN_RESET);
  realy_state = 0;
}

uint8_t Relay_GetState() {
  return realy_state;
}

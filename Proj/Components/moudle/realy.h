/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-07-31 16:57:55
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-07-31 17:00:37
 * @FilePath: \code\Proj\Components\moudle\realy.h
 * @Description: 
 * 本项目只用于控制THz专用激励源的控制，切勿用于其他用途，否则西安理工大学及开发者不承担任何责任。
 * Copyright (c) 2026 by Jiang Tianhang 1919524828@qq.com, All Rights Reserved. 
 */
#ifndef RELAY_H
#define RELAY_H

#include "main.h"

/// @brief turn on relay
void Relay_On(void);

/// @brief turn off relay
void Relay_Off(void);

/// @brief get relay state
/// @param  
/// @return on : 1, off : 0
uint8_t Relay_GetState(void);

#endif

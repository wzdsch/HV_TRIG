/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-07-30 21:49:10
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-08-05 14:07:29
 * @FilePath: \code\Proj\Components\bsp\bsp_uart.h
 * @Description: 
 * 本项目只用于控制THz专用激励源的控制，切勿用于其他用途，否则西安理工大学及开发者不承担任何责任。
 * Copyright (c) 2026 by Jiang Tianhang 1919524828@qq.com, All Rights Reserved. 
 */
#ifndef BSP_UART_H
#define BSP_UART_H

#include "main.h"

/// @brief Create the UART task and message queue.
/// @param  
void BSP_UART_Init(void);

/// @brief Transmit data by queue (never used in interrupt)
/// @param p_data pointer to the data
/// @param size size of the data
/// @return success: 0, error: 1
uint8_t BSP_UART_Transmit(uint8_t* p_data, uint16_t size);

#endif

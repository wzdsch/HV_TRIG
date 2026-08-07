/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-08-04 16:40:00
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-08-07 16:00:00
 * @FilePath: \code\Proj\Components\module\EEPROM.h
 * @Description:
 *  AT24C02 EEPROM 读写模块（消息队列+DMA架构）
 *  外部接口：EEPROM_Init / EEPROM_SaveSettings / EEPROM_LoadSettings
 *  Save/Load 均为异步非阻塞，入队即返回
 *  本项目只用于控制THz专用激励源的控制，切勿用于其他用途，否则西安理工大学及开发者不承担任何责任。
 * Copyright (c) 2026 by Jiang Tianhang 1919524828@qq.com, All Rights Reserved.
 */
#ifndef EEPROM_H
#define EEPROM_H

#include "main.h"
#include <stdint.h>
#include "display.h"

// ============================================================
// AT24C02 数据布局（地址 0~15，共 16 字节）
// ============================================================
//  0-1  : 默认预设电压   (u16，低字节在前)
//  2-3  : 默认预设频率   (u16，低字节在前)
//  4    : 默认预设占空比 (u8)
//  5-6  : 预设1电压      (u16，低字节在前)
//  7-8  : 预设1频率      (u16，低字节在前)
//  9    : 预设1占空比    (u8)
//  10-11: 预设2电压      (u16，低字节在前)
//  12-13: 预设2频率      (u16，低字节在前)
//  14   : 预设2占空比    (u8)
//  15   : 初始化标志     (0x55 = 已初始化)

/// @brief 初始化 EEPROM 模块（创建消息队列和后台任务）
void EEPROM_Init(void);

/// @brief 保存全部预设数据（异步非阻塞，内部复制数据，入队即返回）
/// @param data 预设数据指针（调用后可立即释放）
/// @return 0=已入队, 1=入队失败
uint8_t EEPROM_SaveSettings(const Display_SettingsData_t *data);

/// @brief 读取全部预设数据（异步非阻塞，入队即返回，数据稍后写入 data）
/// @param data 预设数据指针（调用者须保证在操作完成前有效，含范围保护）
/// @return 0=已入队, 1=入队失败
uint8_t EEPROM_LoadSettings(Display_SettingsData_t *data);

#endif // EEPROM_H

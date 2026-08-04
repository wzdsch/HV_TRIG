/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-08-04 16:40:00
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-08-04 17:01:27
 * @FilePath: \code\Proj\Components\module\EEPROM.h
 * @Description:
 *  AT24C02 EEPROM 读写模块，使用硬件 I2C1（PB6=SCL，PB7=SDA）
 *  实现电压/频率/占空比预设数据的保存与读取
 *  本项目只用于控制THz专用激励源的控制，切勿用于其他用途，否则西安理工大学及开发者不承担任何责任。
 * Copyright (c) 2026 by Jiang Tianhang 1919524828@qq.com, All Rights Reserved.
 */
#ifndef EEPROM_H
#define EEPROM_H

#include "main.h"
#include <stdint.h>
#include "display.h"

// ============================================================
// AT24C02 数据布局（地址 0~255）
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

/// @brief 读取单个字节
/// @param addr 地址 0~255
/// @return 读取到的数据
uint8_t EEPROM_ReadByte(uint16_t addr);

/// @brief 写入单个字节
/// @param addr 地址 0~255
/// @param data 要写入的数据
/// @return 0=成功, 1=失败
uint8_t EEPROM_WriteByte(uint16_t addr, uint8_t data);

/// @brief 连续读取多个字节
/// @param addr 起始地址 0~255
/// @param buf  数据缓冲区
/// @param len  读取长度
/// @return 0=成功, 1=失败
uint8_t EEPROM_Read(uint16_t addr, uint8_t *buf, uint16_t len);

/// @brief 连续写入多个字节
/// @param addr 起始地址 0~255
/// @param buf  数据缓冲区
/// @param len  写入长度
/// @return 0=成功, 1=失败
uint8_t EEPROM_Write(uint16_t addr, uint8_t *buf, uint16_t len);

/// @brief 保存全部预设数据到 EEPROM
/// @param data 预设数据指针
/// @return 0=成功, 1=失败
uint8_t EEPROM_SaveSettings(const Display_SettingsData_t *data);

/// @brief 从 EEPROM 读取全部预设数据（带范围保护）
/// @param data 预设数据指针
/// @return 0=成功, 1=失败
uint8_t EEPROM_LoadSettings(Display_SettingsData_t *data);

#endif // EEPROM_H

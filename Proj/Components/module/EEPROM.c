/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-08-04 16:40:00
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-08-06 17:23:59
 * @FilePath: \code\Proj\Components\module\EEPROM.c
 * @Description:
 *  AT24C02 EEPROM 读写模块，使用硬件 I2C1（PB6=SCL，PB7=SDA）
 *  实现参考程序(2023.3.15)中 AT24C02 的电压/频率/占空比预设存储功能
 *  本项目只用于控制THz专用激励源的控制，切勿用于其他用途，否则西安理工大学及开发者不承担任何责任。
 * Copyright (c) 2026 by Jiang Tianhang 1919524828@qq.com, All Rights Reserved.
 */
#include "EEPROM.h"
#include "i2c.h"
#include "cmsis_os2.h"

#define EEPROM_DEV_ADDR  0xA0    // AT24C02 8位从机地址 (7位地址0x50<<1, HAL自动处理bit0: 写=0xA0, 读=0xA1)
#define EEPROM_TIMEOUT   100     // I2C 超时时间 ms

#define EEPROM_ADDR_DEFAULT  0   // 默认预设起始地址
#define EEPROM_ADDR_PRESET1  5   // 预设1起始地址
#define EEPROM_ADDR_PRESET2  10  // 预设2起始地址
#define EEPROM_PRESET_SIZE   5   // 单组预设字节数 (2电压+2频率+1占空比)

// 数据范围保护
#define EEPROM_VOLTAGE_MAX  400
#define EEPROM_FREQ_MAX     500
#define EEPROM_DUTY_MAX     99

/// @brief 等待 EEPROM 内部写周期完成
static void EEPROM_WriteDelay(void) {
  osDelay(5);  // AT24C02 内部写周期典型值 2~5ms
}

uint8_t EEPROM_ReadByte(uint16_t addr) {
  uint8_t data = 0;
  EEPROM_Read(addr, &data, 1);
  return data;
}

uint8_t EEPROM_WriteByte(uint16_t addr, uint8_t data) {
  if (HAL_I2C_Mem_Write(&hi2c1, EEPROM_DEV_ADDR, addr, I2C_MEMADD_SIZE_8BIT, &data, 1, EEPROM_TIMEOUT) != HAL_OK) {
    return 1;
  }
  EEPROM_WriteDelay();
  return 0;
}

uint8_t EEPROM_Read(uint16_t addr, uint8_t *buf, uint16_t len) {
  if (HAL_I2C_Mem_Read(&hi2c1, EEPROM_DEV_ADDR, addr, I2C_MEMADD_SIZE_8BIT, buf, len, EEPROM_TIMEOUT) != HAL_OK) {
    return 1;
  }
  return 0;
}

uint8_t EEPROM_Write(uint16_t addr, uint8_t *buf, uint16_t len) {
  uint16_t i;
  for (i = 0; i < len; i++) {
    if (EEPROM_WriteByte(addr + i, buf[i]) != 0) {
      return 1;
    }
  }
  return 0;
}

/// @brief 保存单组预设数据
/// @param addr   起始地址
/// @param preset 预设数据指针
/// @return 0=成功, 1=失败
static uint8_t EEPROM_SavePreset(uint16_t addr, const Display_PresetData_t *preset) {
  if (EEPROM_WriteByte(addr, (uint8_t)(preset->voltage & 0xFF)) != 0) return 1;
  if (EEPROM_WriteByte(addr + 1, (uint8_t)(preset->voltage >> 8)) != 0) return 1;
  if (EEPROM_WriteByte(addr + 2, (uint8_t)(preset->freq & 0xFF)) != 0) return 1;
  if (EEPROM_WriteByte(addr + 3, (uint8_t)(preset->freq >> 8)) != 0) return 1;
  if (EEPROM_WriteByte(addr + 4, preset->duty) != 0) return 1;
  return 0;
}

/// @brief 读取单组预设数据（带范围保护）
/// @param addr   起始地址
/// @param preset 预设数据指针
/// @return 0=成功, 1=失败
static uint8_t EEPROM_LoadPreset(uint16_t addr, Display_PresetData_t *preset) {
  uint8_t buf[EEPROM_PRESET_SIZE];
  if (EEPROM_Read(addr, buf, EEPROM_PRESET_SIZE) != 0) {
    return 1;
  }
  preset->voltage    = buf[0] | (buf[1] << 8);
  preset->freq  = buf[2] | (buf[3] << 8);
  preset->duty = buf[4];

  // 防止 EEPROM 内容异常导致越界
  if (preset->voltage > EEPROM_VOLTAGE_MAX) preset->voltage = EEPROM_VOLTAGE_MAX;
  if (preset->freq > EEPROM_FREQ_MAX) preset->freq = EEPROM_FREQ_MAX;
  if (preset->duty > EEPROM_DUTY_MAX) preset->duty = EEPROM_DUTY_MAX;
  return 0;
}

uint8_t EEPROM_SaveSettings(const Display_SettingsData_t *data) {
  if (EEPROM_SavePreset(EEPROM_ADDR_DEFAULT, &data->default_preset) != 0) return 1;
  if (EEPROM_SavePreset(EEPROM_ADDR_PRESET1, &data->preset1) != 0) return 1;
  if (EEPROM_SavePreset(EEPROM_ADDR_PRESET2, &data->preset2) != 0) return 1;
  return 0;
}

uint8_t EEPROM_LoadSettings(Display_SettingsData_t *data) {
  if (EEPROM_LoadPreset(EEPROM_ADDR_DEFAULT, &data->default_preset) != 0) return 1;
  if (EEPROM_LoadPreset(EEPROM_ADDR_PRESET1, &data->preset1) != 0) return 1;
  if (EEPROM_LoadPreset(EEPROM_ADDR_PRESET2, &data->preset2) != 0) return 1;
  return 0;
}

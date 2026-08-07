/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-08-04 16:40:00
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-08-07 16:00:00
 * @FilePath: \code\Proj\Components\module\EEPROM.c
 * @Description:
 *  AT24C02 EEPROM 读写模块（消息队列 + I2C DMA 架构）
 *  外部接口 EEPROM_SaveSettings / EEPROM_LoadSettings 为异步非阻塞（入队即返回）
 *  内部任务逐字节 DMA 完成预设数据的物理读写
 *  本项目只用于控制THz专用激励源的控制，切勿用于其他用途，否则西安理工大学及开发者不承担任何责任。
 * Copyright (c) 2026 by Jiang Tianhang 1919524828@qq.com, All Rights Reserved.
 */
#include "EEPROM.h"
#include "i2c.h"
#include "dma.h"
#include "cmsis_os2.h"
#include "freertos.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <string.h>

// ============================================================
// 硬件参数
// ============================================================
#define EEPROM_DEV_ADDR  0xA0    // AT24C02 8位从机地址 (7位0x50<<1)
#define EEPROM_TIMEOUT   100     // I2C DMA 超时 ms

// ============================================================
// 数据布局
// ============================================================
#define EEPROM_ADDR_DEFAULT  0
#define EEPROM_ADDR_PRESET1  5
#define EEPROM_ADDR_PRESET2  10
#define EEPROM_PRESET_SIZE   5
#define EEPROM_TOTAL_SIZE    15    // 3组 × 5字节

#define EEPROM_VOLTAGE_MAX  400
#define EEPROM_FREQ_MAX     500
#define EEPROM_DUTY_MAX     99

// ============================================================
// 内部消息
// ============================================================
#define EEPROM_OP_SAVE  0
#define EEPROM_OP_LOAD  1

typedef struct {
  uint8_t   op;                // EEPROM_OP_SAVE / EEPROM_OP_LOAD
  Display_SettingsData_t *data; // save: 内部副本; load: 调用者缓冲区
  uint8_t   owns_data;         // 1=任务完成后 vPortFree(data)
  uint8_t   result;            // 0=成功, 1=失败（仅同步模式有意义）
  SemaphoreHandle_t done_sem;  // NULL=异步
} EEPROM_Msg_t;

// ============================================================
// 任务 & 队列
// ============================================================
static osThreadId_t   s_eeprom_task;
static TaskHandle_t   s_eeprom_task_handle;

static const osThreadAttr_t s_eeprom_task_attr = {
  .name       = "EEPROM_Task",
  .stack_size = 512,
  .priority   = osPriorityNormal
};

static osMessageQueueId_t s_eeprom_queue;

static void EEPROM_Task(void *arg);

// ============================================================
// 内部：ACK 轮询写周期等待
// ============================================================
static void EEPROM_WriteDelay(void) {
  HAL_I2C_IsDeviceReady(&hi2c1, EEPROM_DEV_ADDR, 2, 10);
}

// ============================================================
// 内部：单字节 DMA 读取
// ============================================================
static uint8_t EEPROM_ReadByteDMA(uint16_t addr, uint8_t *val) {
  if (HAL_I2C_Mem_Read_DMA(&hi2c1, EEPROM_DEV_ADDR, addr,
        I2C_MEMADD_SIZE_8BIT, val, 1) != HAL_OK) {
    return 1;
  }
  if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(EEPROM_TIMEOUT)) == 0) {
    return 1;
  }
  return (hi2c1.ErrorCode == HAL_I2C_ERROR_NONE) ? 0 : 1;
}

// ============================================================
// 内部：单字节 DMA 写入（含写周期等待）
// ============================================================
static uint8_t EEPROM_WriteByteDMA(uint16_t addr, uint8_t val) {
  if (HAL_I2C_Mem_Write_DMA(&hi2c1, EEPROM_DEV_ADDR, addr,
        I2C_MEMADD_SIZE_8BIT, &val, 1) != HAL_OK) {
    return 1;
  }
  if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(EEPROM_TIMEOUT)) == 0) {
    return 1;
  }
  if (hi2c1.ErrorCode != HAL_I2C_ERROR_NONE) {
    return 1;
  }
  EEPROM_WriteDelay();
  return 0;
}

// ============================================================
// 内部：将预设数据打包写入 5 字节
// ============================================================
static uint8_t EEPROM_WritePresetDMA(uint16_t addr, const Display_PresetData_t *p) {
  uint8_t buf[EEPROM_PRESET_SIZE];
  buf[0] = (uint8_t)(p->voltage & 0xFF);
  buf[1] = (uint8_t)(p->voltage >> 8);
  buf[2] = (uint8_t)(p->freq & 0xFF);
  buf[3] = (uint8_t)(p->freq >> 8);
  buf[4] = p->duty;
  for (uint8_t i = 0; i < EEPROM_PRESET_SIZE; i++) {
    if (EEPROM_WriteByteDMA(addr + i, buf[i]) != 0) return 1;
  }
  return 0;
}

// ============================================================
// 内部：读取 5 字节并解析为预设数据
// ============================================================
static uint8_t EEPROM_ReadPresetDMA(uint16_t addr, Display_PresetData_t *p) {
  uint8_t buf[EEPROM_PRESET_SIZE];
  for (uint8_t i = 0; i < EEPROM_PRESET_SIZE; i++) {
    if (EEPROM_ReadByteDMA(addr + i, &buf[i]) != 0) return 1;
  }
  p->voltage = buf[0] | (buf[1] << 8);
  p->freq    = buf[2] | (buf[3] << 8);
  p->duty    = buf[4];
  if (p->voltage > EEPROM_VOLTAGE_MAX) p->voltage = EEPROM_VOLTAGE_MAX;
  if (p->freq    > EEPROM_FREQ_MAX)    p->freq    = EEPROM_FREQ_MAX;
  if (p->duty    > EEPROM_DUTY_MAX)    p->duty    = EEPROM_DUTY_MAX;
  return 0;
}

// ============================================================
// 外部 API（异步非阻塞）
// ============================================================

void EEPROM_Init(void) {
  s_eeprom_queue = osMessageQueueNew(4, sizeof(EEPROM_Msg_t*), NULL);
  s_eeprom_task  = osThreadNew(EEPROM_Task, NULL, &s_eeprom_task_attr);
  s_eeprom_task_handle = (TaskHandle_t)s_eeprom_task;
  xTaskNotifyStateClear(s_eeprom_task_handle);
  ulTaskNotifyValueClear(s_eeprom_task_handle, 0xFFFFFFFF);
}

uint8_t EEPROM_SaveSettings(const Display_SettingsData_t *data) {
  EEPROM_Msg_t *msg = pvPortMalloc(sizeof(EEPROM_Msg_t));
  if (msg == NULL) return 1;

  msg->data = pvPortMalloc(sizeof(Display_SettingsData_t));
  if (msg->data == NULL) {
    vPortFree(msg);
    return 1;
  }
  memcpy(msg->data, data, sizeof(Display_SettingsData_t));

  msg->op       = EEPROM_OP_SAVE;
  msg->owns_data = 1;
  msg->result   = 0;
  msg->done_sem = NULL;

  if (osMessageQueuePut(s_eeprom_queue, &msg, 0, 0) != osOK) {
    vPortFree(msg->data);
    vPortFree(msg);
    return 1;
  }
  return 0;
}

uint8_t EEPROM_LoadSettings(Display_SettingsData_t *data) {
  EEPROM_Msg_t *msg = pvPortMalloc(sizeof(EEPROM_Msg_t));
  if (msg == NULL) return 1;

  msg->op       = EEPROM_OP_LOAD;
  msg->data     = data;
  msg->owns_data = 0;
  msg->result   = 0;
  msg->done_sem = NULL;

  if (osMessageQueuePut(s_eeprom_queue, &msg, 0, 0) != osOK) {
    vPortFree(msg);
    return 1;
  }
  return 0;
}

// ============================================================
// 后台任务
// ============================================================
static void EEPROM_Task(void *arg) {
  (void)arg;
  for(;;) {
    EEPROM_Msg_t *msg = NULL;
    osMessageQueueGet(s_eeprom_queue, &msg, NULL, osWaitForever);

    if (msg->op == EEPROM_OP_SAVE) {
      msg->result = 0;
      if (EEPROM_WritePresetDMA(EEPROM_ADDR_DEFAULT, &msg->data->default_preset) != 0) msg->result = 1;
      if (msg->result == 0 && EEPROM_WritePresetDMA(EEPROM_ADDR_PRESET1, &msg->data->preset1) != 0) msg->result = 1;
      if (msg->result == 0 && EEPROM_WritePresetDMA(EEPROM_ADDR_PRESET2, &msg->data->preset2) != 0) msg->result = 1;

    } else if (msg->op == EEPROM_OP_LOAD) {
      msg->result = 0;
      if (EEPROM_ReadPresetDMA(EEPROM_ADDR_DEFAULT, &msg->data->default_preset) != 0) msg->result = 1;
      if (msg->result == 0 && EEPROM_ReadPresetDMA(EEPROM_ADDR_PRESET1, &msg->data->preset1) != 0) msg->result = 1;
      if (msg->result == 0 && EEPROM_ReadPresetDMA(EEPROM_ADDR_PRESET2, &msg->data->preset2) != 0) msg->result = 1;
    }

    // 清理
    if (msg->owns_data) {
      vPortFree(msg->data);
    }
    if (msg->done_sem != NULL) {
      xSemaphoreGive(msg->done_sem);
    } else {
      vPortFree(msg);
    }
  }
}

// ============================================================
// I2C DMA 回调 → 通知 EEPROM 任务
// ============================================================
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c) {
  if (hi2c->Instance == I2C1) {
    BaseType_t yield = pdFALSE;
    vTaskNotifyGiveFromISR(s_eeprom_task_handle, &yield);
    if (yield == pdTRUE) portYIELD_FROM_ISR(yield);
  }
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
  if (hi2c->Instance == I2C1) {
    BaseType_t yield = pdFALSE;
    vTaskNotifyGiveFromISR(s_eeprom_task_handle, &yield);
    if (yield == pdTRUE) portYIELD_FROM_ISR(yield);
  }
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
  if (hi2c->Instance == I2C1) {
    BaseType_t yield = pdFALSE;
    vTaskNotifyGiveFromISR(s_eeprom_task_handle, &yield);
    if (yield == pdTRUE) portYIELD_FROM_ISR(yield);
  }
}

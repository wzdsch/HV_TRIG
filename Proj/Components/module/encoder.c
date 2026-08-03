/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-08-03 09:52:41
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-08-03 10:30:07
 * @FilePath: \code\Proj\Components\module\encoder.c
 * @Description: 
 * 本项目只用于控制THz专用激励源的控制，切勿用于其他用途，否则西安理工大学及开发者不承担任何责任。
 * Copyright (c) 2026 by Jiang Tianhang 1919524828@qq.com, All Rights Reserved. 
 */
#include "encoder.h"
#include "tim.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"

// Encoder timer is TIM2, 16-bit, ARR = 65535
#define ENCODER_TIM        (&htim2)
#define ENCODER_CNT_MAX    65535U
#define ENCODER_HALF_CNT   32768U

// Task polling interval (ms)
#define ENCODER_SAMPLE_MS  10U

// Encoder running state
static uint8_t encoder_flg = 0;

// Accumulated 32-bit position count
static int32_t encoder_count = 0;

// Last read CNT value for overflow detection
static uint16_t encoder_last_cnt = 0;

// User callback
static void (*Encoder_Callback)(int32_t delta) = NULL;

// task attributes
static const osThreadAttr_t encoder_task_attr = {
  .name = "encoder_task",
  .stack_size = 256,
  .priority = osPriorityNormal
};

// Forward declaration of task function
static void Encoder_Task(void *arguments);

void Encoder_Init(void) {
  osThreadNew(Encoder_Task, NULL, &encoder_task_attr);
}

void Encoder_Start(void) {
  encoder_last_cnt = (uint16_t)__HAL_TIM_GET_COUNTER(ENCODER_TIM);
  encoder_count = 0;
  encoder_flg = 1;
}

void Encoder_Stop(void) {
  encoder_flg = 0;
}

uint8_t Encoder_GetState(void) {
  return encoder_flg;
}

int32_t Encoder_GetCount(void) {
  return encoder_count;
}

void Encoder_ResetCount(void) {
  encoder_count = 0;
  encoder_last_cnt = (uint16_t)__HAL_TIM_GET_COUNTER(ENCODER_TIM);
}

void Encoder_SetCallback(void (*callback)(int32_t delta)) {
  Encoder_Callback = callback;
}

static void Encoder_Task(void *arguments) {
  uint16_t crt_cnt;
  int16_t  delta;

  for (;;) {
    if (encoder_flg) {
      crt_cnt = (uint16_t)__HAL_TIM_GET_COUNTER(ENCODER_TIM);

      // 16-bit delta auto-handles overflow via signed cast
      delta = (int16_t)(crt_cnt - encoder_last_cnt);
      encoder_count += (int32_t)delta;
      encoder_last_cnt = crt_cnt;

      if (Encoder_Callback != NULL) {
        Encoder_Callback((int32_t)delta);
      }
    }

    osDelay(10);
  }
}

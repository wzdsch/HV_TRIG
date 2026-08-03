/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-07-31 14:53:37
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-08-03 10:22:37
 * @FilePath: \code\Proj\Components\module\key_scan.c
 * @Description: 
 * 本项目只用于控制THz专用激励源的控制，切勿用于其他用途，否则西安理工大学及开发者不承担任何责任。
 * Copyright (c) 2026 by Jiang Tianhang 1919524828@qq.com, All Rights Reserved. 
 */
#include "key_scan.h"
#include "stdlib.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"

// key pin is pull-up
// electrical level is high when key is not pressed

// key pin electrical level
uint8_t key0_last_state = 1;
uint8_t key1_last_state = 1;
uint8_t key2_last_state = 1;
uint8_t key3_last_state = 1;
uint8_t key4_last_state = 1;
uint8_t key5_last_state = 1;

uint8_t key0_crt_state = 1;
uint8_t key1_crt_state = 1;
uint8_t key2_crt_state = 1;
uint8_t key3_crt_state = 1;
uint8_t key4_crt_state = 1;
uint8_t key5_crt_state = 1;

// key action callback function, regist by extern
// key pressed callback
static void (*Key0Down_Callback)(void) = NULL;
static void (*Key1Down_Callback)(void) = NULL;
static void (*Key2Down_Callback)(void) = NULL;
static void (*Key3Down_Callback)(void) = NULL;
static void (*Key4Down_Callback)(void) = NULL;
static void (*Key5Down_Callback)(void) = NULL;

// key released callback
static void (*Key0Up_Callback)(void) = NULL;
static void (*Key1Up_Callback)(void) = NULL;
static void (*Key2Up_Callback)(void) = NULL;
static void (*Key3Up_Callback)(void) = NULL;
static void (*Key4Up_Callback)(void) = NULL;
static void (*Key5Up_Callback)(void) = NULL;

// weather or not should run callback falg
// key pressed flag
uint8_t key0_down_flg = 0;
uint8_t key1_down_flg = 0;
uint8_t key2_down_flg = 0;
uint8_t key3_down_flg = 0;
uint8_t key4_down_flg = 0;
uint8_t key5_down_flg = 0;

// key released flag
uint8_t key0_up_flg = 0;
uint8_t key1_up_flg = 0;
uint8_t key2_up_flg = 0;
uint8_t key3_up_flg = 0;
uint8_t key4_up_flg = 0;
uint8_t key5_up_flg = 0;

// tasks
osThreadId_t key_scan_task;
osThreadAttr_t key_scan_task_attr = {
  .name = "key_scan_task",
  .stack_size = 256,
  .priority = osPriorityNormal
};

osThreadId_t key_callback_task;
osThreadAttr_t key_callback_task_attr = {
  .name = "key_callback_task",
  .stack_size = 1024,
  .priority = osPriorityNormal
};


void Key_Scan_Task(void *arguments);
void Key_Callback_Task(void *arguments);

void Key_Init(void) {
  key_scan_task = osThreadNew(Key_Scan_Task, NULL, &key_scan_task_attr);
  key_callback_task = osThreadNew(Key_Callback_Task, NULL, &key_callback_task_attr);
}

void Key_Scan_Task(void *arguments) {
  for(;;) {
    // scan key pin, get current state
    key0_crt_state = HAL_GPIO_ReadPin(KEY0_GPIO_Port, KEY0_Pin);
    key1_crt_state = HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin);
    key2_crt_state = HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin);
    key3_crt_state = HAL_GPIO_ReadPin(KEY3_GPIO_Port, KEY3_Pin);
    key4_crt_state = HAL_GPIO_ReadPin(KEY4_GPIO_Port, KEY4_Pin);
    key5_crt_state = HAL_GPIO_ReadPin(KEY5_GPIO_Port, KEY5_Pin);

    // judge key action, notify callback task
    if (key0_last_state == 1 && key0_crt_state == 0) {
      key0_down_flg = 1;
      xTaskNotifyGive((TaskHandle_t)key_callback_task);
    }
    if (key1_last_state == 1 && key1_crt_state == 0) {
      key1_down_flg = 1;
      xTaskNotifyGive((TaskHandle_t)key_callback_task);
    }
    if (key2_last_state == 1 && key2_crt_state == 0) {
      key2_down_flg = 1;
      xTaskNotifyGive((TaskHandle_t)key_callback_task);
    }
    if (key3_last_state == 1 && key3_crt_state == 0) {
      key3_down_flg = 1;
      xTaskNotifyGive((TaskHandle_t)key_callback_task);
    }
    if (key4_last_state == 1 && key4_crt_state == 0) {
      key4_down_flg = 1;
      xTaskNotifyGive((TaskHandle_t)key_callback_task);
    }
    if (key5_last_state == 1 && key5_crt_state == 0) {
      key5_down_flg = 1;
      xTaskNotifyGive((TaskHandle_t)key_callback_task);
    }

    if (key0_last_state == 0 && key0_crt_state == 1) {
      key0_up_flg = 1;
      xTaskNotifyGive((TaskHandle_t)key_callback_task);
    }
    if (key1_last_state == 0 && key1_crt_state == 1) {
      key1_up_flg = 1;
      xTaskNotifyGive((TaskHandle_t)key_callback_task);
    }
    if (key2_last_state == 0 && key2_crt_state == 1) {
      key2_up_flg = 1;
      xTaskNotifyGive((TaskHandle_t)key_callback_task);
    }
    if (key3_last_state == 0 && key3_crt_state == 1) {
      key3_up_flg = 1;
      xTaskNotifyGive((TaskHandle_t)key_callback_task);
    }
    if (key4_last_state == 0 && key4_crt_state == 1) {
      key4_up_flg = 1;
      xTaskNotifyGive((TaskHandle_t)key_callback_task);
    }
    if (key5_last_state == 0 && key5_crt_state == 1) {
      key5_up_flg = 1;
      xTaskNotifyGive((TaskHandle_t)key_callback_task);
    }

    // update key last state
    key0_last_state = key0_crt_state;
    key1_last_state = key1_crt_state;
    key2_last_state = key2_crt_state;
    key3_last_state = key3_crt_state;
    key4_last_state = key4_crt_state;
    key5_last_state = key5_crt_state;

    // scan every 10ms
    osDelay(10);
  }
}

void Key_Callback_Task(void *arguments) {
  for(;;) {
    if(key0_down_flg == 1) {
      if (Key0Down_Callback != NULL) {
        Key0Down_Callback();
      }
      key0_down_flg = 0;
    }
    if(key1_down_flg == 1) {
      if (Key1Down_Callback != NULL) {
        Key1Down_Callback();
      }
      key1_down_flg = 0;
    }
    if(key2_down_flg == 1) {
      if (Key2Down_Callback != NULL) {
        Key2Down_Callback();
      }
      key2_down_flg = 0;
    }
    if(key3_down_flg == 1) {
      if (Key3Down_Callback != NULL) {
        Key3Down_Callback();
      }
      key3_down_flg = 0;
    }
    if(key4_down_flg == 1) {
      if (Key4Down_Callback != NULL) {
        Key4Down_Callback();
      }
      key4_down_flg = 0;
    }
    if(key5_down_flg == 1) {
      if (Key5Down_Callback != NULL) {
        Key5Down_Callback();
      }
      key5_down_flg = 0;
    }
    if(key0_up_flg == 1) {
      if (Key0Up_Callback != NULL) {
        Key0Up_Callback();
      }
      key0_up_flg = 0;
    }
    if(key1_up_flg == 1) {
      if (Key1Up_Callback != NULL) {
        Key1Up_Callback();
      }
      key1_up_flg = 0;
    }
    if(key2_up_flg == 1) {
      if (Key2Up_Callback != NULL) {
        Key2Up_Callback();
      }
      key2_up_flg = 0;
    }
    if(key3_up_flg == 1) {
      if (Key3Up_Callback != NULL) {
        Key3Up_Callback();
      }
      key3_up_flg = 0;
    }
    if(key4_up_flg == 1) {
      if (Key4Up_Callback != NULL) {
        Key4Up_Callback();
      }
      key4_up_flg = 0;
    }
    if(key5_up_flg == 1) {
      if (Key5Up_Callback != NULL) {
        Key5Up_Callback();
      }
      key5_up_flg = 0;
    }
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  }
}

void Key_SetKey0DownCallback(void (*callback)(void)) {
  Key0Down_Callback = callback;
}

void Key_SetKey1DownCallback(void (*callback)(void)) {
  Key1Down_Callback = callback;
}

void Key_SetKey2DownCallback(void (*callback)(void)) {
  Key2Down_Callback = callback;
}

void Key_SetKey3DownCallback(void (*callback)(void)) {
  Key3Down_Callback = callback;
}

void Key_SetKey4DownCallback(void (*callback)(void)) {
  Key4Down_Callback = callback;
}

void Key_SetKey5DownCallback(void (*callback)(void)) {
  Key5Down_Callback = callback;
}

void Key_SetKey0UpCallback(void (*callback)(void)) {
  Key0Up_Callback = callback;
}

void Key_SetKey1UpCallback(void (*callback)(void)) {
  Key1Up_Callback = callback;
}

void Key_SetKey2UpCallback(void (*callback)(void)) {
  Key2Up_Callback = callback;
}

void Key_SetKey3UpCallback(void (*callback)(void)) {
  Key3Up_Callback = callback;
}

void Key_SetKey4UpCallback(void (*callback)(void)) {
  Key4Up_Callback = callback;
}

void Key_SetKey5UpCallback(void (*callback)(void)) {
  Key5Up_Callback = callback;
}

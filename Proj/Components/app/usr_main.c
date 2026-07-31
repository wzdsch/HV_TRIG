/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-07-30 17:21:02
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-07-30 21:31:24
 * @FilePath: \code\Proj\Components\app\usr_main.c
 * @Description: 
 * 本项目只用于控制THz专用激励源的控制，切勿用于其他用途，否则西安理工大学及开发者不承担任何责任。
 * Copyright (c) 2026 by Jiang Tianhang 1919524828@qq.com, All Rights Reserved. 
 */
#include "usr_main.h"
#include "main.h"

void Usr_Main_Init(void) {
  HAL_Delay(1000); // 等待屏幕从机先上电工作
}

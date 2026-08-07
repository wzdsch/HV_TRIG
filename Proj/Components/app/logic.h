/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-08-04 15:27:14
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-08-07 11:40:02
 * @FilePath: \code\Proj\Components\app\logic.h
 * @Description: 
 * 本项目只用于控制THz专用激励源的控制，切勿用于其他用途，否则西安理工大学及开发者不承担任何责任。
 * Copyright (c) 2026 by Jiang Tianhang 1919524828@qq.com, All Rights Reserved. 
 */
#ifndef LOGIC_H
#define LOGIC_H

#include "stdint.h"
#include "display.h"

void Key0_Down_Logic(void);
void Key1_Down_Logic(void);
void Key2_Down_Logic(void);
void Key3_Down_Logic(void);
void Key4_Down_Logic(void);
void Key5_Down_Logic(void);
void Encoder_Logic(int32_t delta);

#endif

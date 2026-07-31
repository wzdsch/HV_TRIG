/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-07-31 14:53:48
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-07-31 16:27:47
 * @FilePath: \code\Proj\Components\moudle\key_scan.h
 * @Description: 
 * 本项目只用于控制THz专用激励源的控制，切勿用于其他用途，否则西安理工大学及开发者不承担任何责任。
 * Copyright (c) 2026 by Jiang Tianhang 1919524828@qq.com, All Rights Reserved. 
 */
#ifndef KEY_SCAN_H
#define KEY_SCAN_H

// K0: enter
// K1: T/Down
// K2: V/UP
// K3: Menu
// K4: ADJ/SET
// K5: ON/OFF

#include "stdint.h"
#include "main.h"

/// @brief create key task and callback task
/// @param  
void Key_Init(void);

// set key action callback
// key pressed callback
void Key_SetKey0DownCallback(void (*callback)(void));
void Key_SetKey1DownCallback(void (*callback)(void));
void Key_SetKey2DownCallback(void (*callback)(void));
void Key_SetKey3DownCallback(void (*callback)(void));
void Key_SetKey4DownCallback(void (*callback)(void));
void Key_SetKey5DownCallback(void (*callback)(void));

// key released callback
void Key_SetKey0UpCallback(void (*callback)(void));
void Key_SetKey1UpCallback(void (*callback)(void));
void Key_SetKey2UpCallback(void (*callback)(void));
void Key_SetKey3UpCallback(void (*callback)(void));
void Key_SetKey4UpCallback(void (*callback)(void));
void Key_SetKey5UpCallback(void (*callback)(void));


#endif

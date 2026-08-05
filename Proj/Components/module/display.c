/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-08-04 09:34:09
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-08-05 23:01:10
 * @FilePath: \code\Proj\Components\module\display.c
 * @Description: MF022 UART串口屏显示模块实现
 *  通过BSP_UART队列发送DGUS指令控制MF022串口屏（220x176）
 *  指令格式与参考程序 PINMUSHUAXIN.c / main.c 完全一致
 *  坐标和颜色值直接内联在指令字符串中，不使用宏定义
 *  本项目只用于控制THz专用激励源的控制，切勿用于其他用途，否则西安理工大学及开发者不承担任何责任。
 * Copyright (c) 2026 by Jiang Tianhang 1919524828@qq.com, All Rights Reserved.
 */
#include "display.h"
#include "bsp_uart.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/* 图片资源ID（烧录在屏幕Flash中） */
#define IMG_MAIN_BG     2099960
#define IMG_SETTINGS_BG 2177400
#define IMG_OUTPUT_ON   2098556
#define IMG_OUTPUT_OFF  2097152

#define MAIN_SETVOL_BG_COLOR 13           // 主界面设定电压背景颜色（淡蓝色）
#define MAIN_SETVOL_FONT_SIZE 16          // 主界面设定电压字体大小
#define MAIN_SETVOL_FONT_COLOR 16         // 主界面设定电压字体颜色（白色）
#define MAIN_SETVOL100_POS_X 54           // 主界面设定电压百位位置X
#define MAIN_SETVOL100_POS_Y 10           // 主界面设定电压百位位置Y
#define MAIN_SETVOL10_POS_X 62            // 主界面设定电压十位位置X
#define MAIN_SETVOL10_POS_Y 10            // 主界面设定电压十位位置Y
#define MAIN_SETVOL1_POS_X 70             // 主界面设定电压个位位置X
#define MAIN_SETVOL1_POS_Y 10             // 主界面设定电压个位位置Y

#define MAIN_INPUTFREQ_BG_COLOR 13        // 主界面输入频率背景颜色（淡蓝色）
#define MAIN_INPUTFREQ_FONT_SIZE 16       // 主界面输入频率字体大小
#define MAIN_INPUTFREQ_FONT_COLOR 16      // 主界面输入频率字体颜色（白色） 注意频率值单位为0.1kHz
#define MAIN_INPUTFREQ100_POS_X 132       // 主界面输入频率十位位置X
#define MAIN_INPUTFREQ100_POS_Y 10        // 主界面输入频率十位位置Y
#define MAIN_INPUTFREQ10_POS_X 140        // 主界面输入频率个位位置X
#define MAIN_INPUTFREQ10_POS_Y 10         // 主界面输入频率个位位置Y
#define MAIN_INPUTFREQ_POINT_POS_X 148    // 主界面输入频率小数点位置X
#define MAIN_INPUTFREQ_POINT_POS_Y 10     // 主界面输入频率小数点位置Y
#define MAIN_INPUTFREQ1_POS_X 156         // 主界面输入频率小数位位置X
#define MAIN_INPUTFREQ1_POS_Y 10          // 主界面输入频率小数位位置Y


#define MAIN_SETFREQ_BG_COLOR 13          // 主界面设定频率背景颜色（淡蓝色）
#define MAIN_SETFREQ_FONT_SIZE 16         // 主界面设定频率字体大小
#define MAIN_SETFREQ_FONT_COLOR 16        // 主界面设定频率字体颜色（白色） 注意频率值单位为0.1kHz
#define MAIN_SETFREQ100_POS_X 50          // 主界面设定频率十位位置X
#define MAIN_SETFREQ100_POS_Y 151         // 主界面设定频率十位位置Y
#define MAIN_SETFREQ10_POS_X 58           // 主界面设定频率个位位置X
#define MAIN_SETFREQ10_POS_Y 151          // 主界面设定频率个位位置Y
#define MAIN_SETFREQ_POINT_POS_X 66       // 主界面设定频率小数点位置X
#define MAIN_SETFREQ_POINT_POS_Y 151      // 主界面设定频率小数点位置Y
#define MAIN_SETFREQ1_POS_X 74            // 主界面设定频率小数位位置X
#define MAIN_SETFREQ1_POS_Y 151           // 主界面设定频率小数位位置Y

#define MAIN_SETDUTY_BG_COLOR 13          // 主界面设定占空比背景颜色（淡蓝色）
#define MAIN_SETDUTY_FONT_SIZE 16         // 主界面设定占空比字体大小
#define MAIN_SETDUTY_FONT_COLOR 16        // 主界面设定占空比字体颜色（白色）
#define MAIN_SETDUTY10_POS_X 140          // 主界面设定占空比十位位置X
#define MAIN_SETDUTY10_POS_Y 151          // 主界面设定占空比十位位置Y
#define MAIN_SETDUTY1_POS_X 148           // 主界面设定占空比个位位置X
#define MAIN_SETDUTY1_POS_Y 151           // 主界面设定占空比个位位置Y

#define MAIN_MEASUREDVOL_BG_COLOR 13      // 主界面实测电压背景颜色（淡蓝色）
#define MAIN_MEASUREDVOL_FONT_SIZE 16     // 主界面实测电压字体大小
#define MAIN_MEASUREDVOL_FONT_COLOR 16    // 主界面实测电压字体颜色（白色）
#define MAIN_MEASUREDVOL100_POS_X 54      // 主界面实测电压百位位置X
#define MAIN_MEASUREDVOL100_POS_Y 72      // 主界面实测电压百位位置Y
#define MAIN_MEASUREDVOL10_POS_X 62       // 主界面实测电压十位位置X
#define MAIN_MEASUREDVOL10_POS_Y 72       // 主界面实测电压十位位置Y
#define MAIN_MEASUREDVOL1_POS_X 70        // 主界面实测电压个位位置X
#define MAIN_MEASUREDVOL1_POS_Y 72        // 主界面实测电压个位位置Y

#define MAIN_MEASUREDCUR_BG_COLOR 13      // 主界面实测电流背景颜色（淡蓝色）
#define MAIN_MEASUREDCUR_FONT_SIZE 16     // 主界面实测电流字体大小
#define MAIN_MEASUREDCUR_FONT_COLOR 16    // 主界面实测电流字体颜色（白色） 注意电流值单位为0.1mA
#define MAIN_MEASUREDCUR100_POS_X 132     // 主界面实测电流十位位置X
#define MAIN_MEASUREDCUR100_POS_Y 72      // 主界面实测电流十位位置Y
#define MAIN_MEASUREDCUR10_POS_X 140      // 主界面实测电流个位位置X
#define MAIN_MEASUREDCUR10_POS_Y 72       // 主界面实测电流个位位置Y
#define MAIN_MEASUREDCUR_POINT_POS_X 148  // 主界面实测电流小数点位置X
#define MAIN_MEASUREDCUR_POINT_POS_Y 72   // 主界面实测电流小数点位置Y
#define MAIN_MEASUREDCUR1_POS_X 156       // 主界面实测电流小数位位置X
#define MAIN_MEASUREDCUR1_POS_Y 72        // 主界面实测电流小数位位置Y

#define MAIN_MEASUREDPOW_BG_COLOR 13      // 主界面实测功率背景颜色（淡蓝色）
#define MAIN_MEASUREDPOW_FONT_SIZE 16     // 主界面实测功率字体大小
#define MAIN_MEASUREDPOW_FONT_COLOR 16    // 主界面实测功率字体颜色（白色） 注意功率值单位为0.1W
#define MAIN_MEASUREDPOW100_POS_X 50      // 主界面实测功率十位位置X
#define MAIN_MEASUREDPOW100_POS_Y 120     // 主界面实测功率十位位置Y
#define MAIN_MEASUREDPOW10_POS_X 58       // 主界面实测功率个位位置X
#define MAIN_MEASUREDPOW10_POS_Y 120      // 主界面实测功率个位位置Y
#define MAIN_MEASUREDPOW_POINT_POS_X 66   // 主界面实测功率小数点位置X
#define MAIN_MEASUREDPOW_POINT_POS_Y 120  // 主界面实测功率小数点位置Y
#define MAIN_MEASUREDPOW1_POS_X 74        // 主界面实测功率小数位位置X
#define MAIN_MEASUREDPOW1_POS_Y 120       // 主界面实测功率小数位位置Y


/* ---- 内部状态 ---- */
static Display_State_t        s_state        = DISPLAY_STATE_MAIN;
static Display_MainCursor_t   s_cursor       = DISPLAY_CURSOR_NONE;
static Display_SettingsRow_t  s_settings_row = DISPLAY_SETTINGS_ROW_CURRENT;
static uint8_t                s_settings_col = DISPLAY_SETTINGS_COL_NONE;

static Display_MainData_t     s_main = {
  .set_voltage          = 123,
  .set_frequency        = 234,
  .set_duty_cycle       = 34,
  .measured_voltage     = 456,
  .measured_current     = 567,
  .measured_power       = 678,
  .input_frequency      = 789,
  .output_enabled       = 0,
};
static Display_SettingsData_t s_settings;
static osMutexId_t            s_mutex;
static char                   s_buf[1024];

/* ---- 内部发送（自动追加\r\n，与参考程序 printf 行为一致） ---- */
static void Display_Send(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int len = vsnprintf(s_buf, sizeof(s_buf) - 3, fmt, args);
  va_end(args);
  if (len > 0 && len < (int)(sizeof(s_buf) - 3)) {
    s_buf[len]     = '\r';
    s_buf[len + 1] = '\n';
    BSP_UART_Transmit((uint8_t *)s_buf, (uint16_t)(len + 2));
  }
}

void Display_Init(void) {
  s_mutex = osMutexNew(NULL);
}

/* ---- 主界面三帧（内部函数，调用前需持有 s_mutex） ---- */

void Display_ShowMain_Bg(void) {
  Display_Send("CLR(0);DIR(1);FSIMG(%d,0,0,220,176,0)", IMG_MAIN_BG);
}

void Display_ShowMain_SetValues(void) {
  Display_Send(
    "SBC(%d);"
    "DCV%d(%d,%d,'%d',%d);"
    "DCV%d(%d,%d,'%d',%d);"
    "DCV%d(%d,%d,'%d',%d);"

    "SBC(%d);"
    "DCV%d(%d,%d,'%d',%d);"
    "DCV%d(%d,%d,'%d',%d);"
    "DCV%d(%d,%d,'.',%d);"
    "DCV%d(%d,%d,'%d',%d);"

    "SBC(%d);"
    "DCV%d(%d,%d,'%d',%d);"
    "DCV%d(%d,%d,'%d',%d)"
    ,

    MAIN_SETVOL_BG_COLOR,
    MAIN_SETVOL_FONT_SIZE, MAIN_SETVOL100_POS_X, MAIN_SETVOL100_POS_Y, s_main.set_voltage / 100, MAIN_SETVOL_FONT_COLOR,
    MAIN_SETVOL_FONT_SIZE, MAIN_SETVOL10_POS_X, MAIN_SETVOL10_POS_Y, s_main.set_voltage / 10 % 10, MAIN_SETVOL_FONT_COLOR,
    MAIN_SETVOL_FONT_SIZE, MAIN_SETVOL1_POS_X, MAIN_SETVOL1_POS_Y, s_main.set_voltage % 10, MAIN_SETVOL_FONT_COLOR,

    MAIN_SETFREQ_BG_COLOR,
    MAIN_SETFREQ_FONT_SIZE, MAIN_SETFREQ100_POS_X, MAIN_SETFREQ100_POS_Y, s_main.set_frequency / 100, MAIN_SETFREQ_FONT_COLOR,
    MAIN_SETFREQ_FONT_SIZE, MAIN_SETFREQ10_POS_X, MAIN_SETFREQ10_POS_Y, s_main.set_frequency / 10 % 10, MAIN_SETFREQ_FONT_COLOR,
    MAIN_SETFREQ_FONT_SIZE, MAIN_SETFREQ_POINT_POS_X, MAIN_SETFREQ_POINT_POS_Y, MAIN_SETFREQ_FONT_COLOR,
    MAIN_SETFREQ_FONT_SIZE, MAIN_SETFREQ1_POS_X, MAIN_SETFREQ1_POS_Y, s_main.set_frequency % 10, MAIN_SETFREQ_FONT_COLOR,

    MAIN_SETDUTY_BG_COLOR,
    MAIN_SETDUTY_FONT_SIZE, MAIN_SETDUTY10_POS_X, MAIN_SETDUTY10_POS_Y, s_main.set_duty_cycle / 10, MAIN_SETDUTY_FONT_COLOR,
    MAIN_SETDUTY_FONT_SIZE, MAIN_SETDUTY1_POS_X, MAIN_SETDUTY1_POS_Y, s_main.set_duty_cycle % 10, MAIN_SETDUTY_FONT_COLOR
  );
}

void Display_ShowMain_Measured(void) {
  Display_Send(
    "SBC(%d);"
    "DCV%d(%d,%d,'%d',%d);"
    "DCV%d(%d,%d,'%d',%d);"
    "DCV%d(%d,%d,'%d',%d);"

    "SBC(%d);"
    "DCV%d(%d,%d,'%d',%d);"
    "DCV%d(%d,%d,'%d',%d);"
    "DCV%d(%d,%d,'.',%d);"
    "DCV%d(%d,%d,'%d',%d);"

    "SBC(%d);"
    "DCV%d(%d,%d,'%d',%d);"
    "DCV%d(%d,%d,'%d',%d);"
    "DCV%d(%d,%d,'.',%d);"
    "DCV%d(%d,%d,'%d',%d)"

    "SBC(%d);"
    "DCV%d(%d,%d,'%d',%d);"
    "DCV%d(%d,%d,'%d',%d);"
    "DCV%d(%d,%d,'.',%d);"
    "DCV%d(%d,%d,'%d',%d);"
    ,

    MAIN_MEASUREDVOL_BG_COLOR,
    MAIN_MEASUREDVOL_FONT_SIZE, MAIN_MEASUREDVOL100_POS_X, MAIN_MEASUREDVOL100_POS_Y, s_main.measured_voltage / 100, MAIN_MEASUREDVOL_FONT_COLOR,
    MAIN_MEASUREDVOL_FONT_SIZE, MAIN_MEASUREDVOL10_POS_X, MAIN_MEASUREDVOL10_POS_Y, s_main.measured_voltage / 10 % 10, MAIN_MEASUREDVOL_FONT_COLOR,
    MAIN_MEASUREDVOL_FONT_SIZE, MAIN_MEASUREDVOL1_POS_X, MAIN_MEASUREDVOL1_POS_Y, s_main.measured_voltage % 10, MAIN_MEASUREDVOL_FONT_COLOR,

    MAIN_MEASUREDCUR_BG_COLOR,
    MAIN_MEASUREDCUR_FONT_SIZE, MAIN_MEASUREDCUR100_POS_X, MAIN_MEASUREDCUR100_POS_Y, s_main.measured_current / 100, MAIN_MEASUREDCUR_FONT_COLOR,
    MAIN_MEASUREDCUR_FONT_SIZE, MAIN_MEASUREDCUR10_POS_X, MAIN_MEASUREDCUR10_POS_Y, s_main.measured_current / 10 % 10, MAIN_MEASUREDCUR_FONT_COLOR,
    MAIN_MEASUREDCUR_FONT_SIZE, MAIN_MEASUREDCUR_POINT_POS_X, MAIN_MEASUREDCUR_POINT_POS_Y, MAIN_MEASUREDCUR_FONT_COLOR,
    MAIN_MEASUREDCUR_FONT_SIZE, MAIN_MEASUREDCUR1_POS_X, MAIN_MEASUREDCUR1_POS_Y, s_main.measured_current % 10, MAIN_MEASUREDCUR_FONT_COLOR,

    MAIN_MEASUREDPOW_BG_COLOR,
    MAIN_MEASUREDPOW_FONT_SIZE, MAIN_MEASUREDPOW100_POS_X, MAIN_MEASUREDPOW100_POS_Y, s_main.measured_power / 100, MAIN_MEASUREDPOW_FONT_COLOR,
    MAIN_MEASUREDPOW_FONT_SIZE, MAIN_MEASUREDPOW10_POS_X, MAIN_MEASUREDPOW10_POS_Y, s_main.measured_power / 10 % 10, MAIN_MEASUREDPOW_FONT_COLOR,
    MAIN_MEASUREDPOW_FONT_SIZE, MAIN_MEASUREDPOW_POINT_POS_X, MAIN_MEASUREDPOW_POINT_POS_Y, MAIN_MEASUREDPOW_FONT_COLOR,
    MAIN_MEASUREDPOW_FONT_SIZE, MAIN_MEASUREDPOW1_POS_X, MAIN_MEASUREDPOW1_POS_Y, s_main.measured_power % 10, MAIN_MEASUREDPOW_FONT_COLOR,

    MAIN_INPUTFREQ_BG_COLOR,
    MAIN_INPUTFREQ_FONT_SIZE, MAIN_INPUTFREQ100_POS_X, MAIN_INPUTFREQ100_POS_Y, s_main.input_frequency / 100, MAIN_INPUTFREQ_FONT_COLOR,
    MAIN_INPUTFREQ_FONT_SIZE, MAIN_INPUTFREQ10_POS_X, MAIN_INPUTFREQ10_POS_Y, s_main.input_frequency / 10 % 10, MAIN_INPUTFREQ_FONT_COLOR,
    MAIN_INPUTFREQ_FONT_SIZE, MAIN_INPUTFREQ_POINT_POS_X, MAIN_INPUTFREQ_POINT_POS_Y, MAIN_INPUTFREQ_FONT_COLOR,
    MAIN_INPUTFREQ_FONT_SIZE, MAIN_INPUTFREQ1_POS_X, MAIN_INPUTFREQ1_POS_Y, s_main.input_frequency % 10, MAIN_INPUTFREQ_FONT_COLOR
  );
}

void Display_ShowMain(void) {
  osMutexAcquire(s_mutex, osWaitForever);
  s_state = DISPLAY_STATE_MAIN;
  Display_ShowMain_Bg();
  Display_ShowMain_SetValues();
  Display_ShowMain_Measured();
  osMutexRelease(s_mutex);
}

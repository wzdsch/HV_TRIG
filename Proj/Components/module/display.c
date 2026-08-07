/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-08-04 09:34:09
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-08-07 11:49:02
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

#define SETVALUE_FONT_HIGHLIGHT_COLOR 1     // 四角数据字体高亮颜色（红色）

#define SETVOL_BG_COLOR 13                // 设定电压背景颜色（淡蓝色）
#define SETVOL_FONT_SIZE 16               // 设定电压字体大小
#define SETVOL_FONT_COLOR 16              // 设定电压字体颜色（白色）
#define SETVOL100_POS_X 54                // 设定电压百位位置X
#define SETVOL100_POS_Y 10                // 设定电压百位位置Y
#define SETVOL10_POS_X 62                 // 设定电压十位位置X
#define SETVOL10_POS_Y 10                 // 设定电压十位位置Y
#define SETVOL1_POS_X 70                  // 设定电压个位位置X
#define SETVOL1_POS_Y 10                  // 设定电压个位位置Y

#define INPUTFREQ_BG_COLOR 13        // 输入频率背景颜色（淡蓝色）
#define INPUTFREQ_FONT_SIZE 16       // 输入频率字体大小
#define INPUTFREQ_FONT_COLOR 16      // 输入频率字体颜色（白色） 注意频率值单位为0.1kHz
#define INPUTFREQ100_POS_X 132       // 输入频率十位位置X
#define INPUTFREQ100_POS_Y 10        // 输入频率十位位置Y
#define INPUTFREQ10_POS_X 140        // 输入频率个位位置X
#define INPUTFREQ10_POS_Y 10         // 输入频率个位位置Y
#define INPUTFREQ_POINT_POS_X 148    // 输入频率小数点位置X
#define INPUTFREQ_POINT_POS_Y 10     // 输入频率小数点位置Y
#define INPUTFREQ1_POS_X 156         // 输入频率小数位位置X
#define INPUTFREQ1_POS_Y 10          // 输入频率小数位位置Y

#define SETFREQ_BG_COLOR 13          // 设定频率背景颜色（淡蓝色）
#define SETFREQ_FONT_SIZE 16         // 设定频率字体大小
#define SETFREQ_FONT_COLOR 16        // 设定频率字体颜色（白色） 注意频率值单位为0.1kHz
#define SETFREQ100_POS_X 50          // 设定频率十位位置X
#define SETFREQ100_POS_Y 151         // 设定频率十位位置Y
#define SETFREQ10_POS_X 58           // 设定频率个位位置X
#define SETFREQ10_POS_Y 151          // 设定频率个位位置Y
#define SETFREQ_POINT_POS_X 66       // 设定频率小数点位置X
#define SETFREQ_POINT_POS_Y 151      // 设定频率小数点位置Y
#define SETFREQ1_POS_X 74            // 设定频率小数位位置X
#define SETFREQ1_POS_Y 151           // 设定频率小数位位置Y

#define SETDUTY_BG_COLOR 13          // 设定占空比背景颜色（淡蓝色）
#define SETDUTY_FONT_SIZE 16         // 设定占空比字体大小
#define SETDUTY_FONT_COLOR 16        // 设定占空比字体颜色（白色）
#define SETDUTY10_POS_X 140          // 设定占空比十位位置X
#define SETDUTY10_POS_Y 151          // 设定占空比十位位置Y
#define SETDUTY1_POS_X 148           // 设定占空比个位位置X
#define SETDUTY1_POS_Y 151           // 设定占空比个位位置Y

#define MAIN_MEASUREDVOL_BG_COLOR 7       // 主界面实测电压背景颜色（灰色）
#define MAIN_MEASUREDVOL_FONT_SIZE 24     // 主界面实测电压字体大小
#define MAIN_MEASUREDVOL_FONT_COLOR 1     // 主界面实测电压字体颜色（红色）
#define MAIN_MEASUREDVOL100_POS_X 88      // 主界面实测电压百位位置X
#define MAIN_MEASUREDVOL100_POS_Y 41      // 主界面实测电压百位位置Y
#define MAIN_MEASUREDVOL10_POS_X 100      // 主界面实测电压十位位置X
#define MAIN_MEASUREDVOL10_POS_Y 41       // 主界面实测电压十位位置Y
#define MAIN_MEASUREDVOL1_POS_X 112       // 主界面实测电压个位位置X
#define MAIN_MEASUREDVOL1_POS_Y 41        // 主界面实测电压个位位置Y

#define MAIN_MEASUREDCUR_BG_COLOR 7       // 主界面实测电流背景颜色（灰色）
#define MAIN_MEASUREDCUR_FONT_SIZE 24     // 主界面实测电流字体大小
#define MAIN_MEASUREDCUR_FONT_COLOR 10    // 主界面实测电流字体颜色（绿色） 注意电流值单位为0.1mA
#define MAIN_MEASUREDCUR100_POS_X 82      // 主界面实测电流十位位置X
#define MAIN_MEASUREDCUR100_POS_Y 76      // 主界面实测电流十位位置Y
#define MAIN_MEASUREDCUR10_POS_X 94       // 主界面实测电流个位位置X
#define MAIN_MEASUREDCUR10_POS_Y 76       // 主界面实测电流个位位置Y
#define MAIN_MEASUREDCUR_POINT_POS_X 106  // 主界面实测电流小数点位置X
#define MAIN_MEASUREDCUR_POINT_POS_Y 76   // 主界面实测电流小数点位置Y
#define MAIN_MEASUREDCUR1_POS_X 118       // 主界面实测电流小数位位置X
#define MAIN_MEASUREDCUR1_POS_Y 76        // 主界面实测电流小数位位置Y

#define MAIN_MEASUREDPOW_BG_COLOR 7       // 主界面实测功率背景颜色（淡蓝色）
#define MAIN_MEASUREDPOW_FONT_SIZE 24     // 主界面实测功率字体大小
#define MAIN_MEASUREDPOW_FONT_COLOR 3     // 主界面实测功率字体颜色（蓝色） 注意功率值单位为0.1W
#define MAIN_MEASUREDPOW100_POS_X 82      // 主界面实测功率十位位置X
#define MAIN_MEASUREDPOW100_POS_Y 112     // 主界面实测功率十位位置Y
#define MAIN_MEASUREDPOW10_POS_X 94       // 主界面实测功率个位位置X
#define MAIN_MEASUREDPOW10_POS_Y 112      // 主界面实测功率个位位置Y
#define MAIN_MEASUREDPOW_POINT_POS_X 106  // 主界面实测功率小数点位置X
#define MAIN_MEASUREDPOW_POINT_POS_Y 112  // 主界面实测功率小数点位置Y
#define MAIN_MEASUREDPOW1_POS_X 118       // 主界面实测功率小数位位置X
#define MAIN_MEASUREDPOW1_POS_Y 112       // 主界面实测功率小数位位置Y

#define SETTINGS_ROW_HIGHLIGHT_BG_COLOR 7 // 设置界面字体背景高亮颜色（灰色）
#define SETTINGS_ROW_NORMAL_BG_COLOR 15   // 设置界字体背景正常颜色（白色）

#define SETTINGS_FONT_HIGHLIGHT_COLOR 1   // 设置界面字体高亮颜色（红色）
#define SETTINGS_FONT_NORMAL_COLOR 0      // 设置界面字体正常颜色（黑色）
#define SETTINGS_FONT_SIZE 16             // 设置界面字体大小

#define SETTINGS_DEFAULT_ROW_Y 54         // 设置界面默认预设行位置Y
#define SETTINGS_PRESET1_ROW_Y 88         // 设置界面预设1行位置Y
#define SETTINGS_PRESET2_ROW_Y 124        // 设置界面预设2行位置Y

#define SETTINGS_SETVOL100_X 15           // 设置界面设定电压百位位置X
#define SETTINGS_SETVOL10_X 23            // 设置界面设定电压十位位置X
#define SETTINGS_SETVOL1_X 31             // 设置界面设定电压个位位置X
#define SETTINGS_SETVOLV_X 39             // 设置界面设定电压'V'位置X

#define SETTINGS_SPACE_VF_X 47            // 设置界面电压频率空格字符位置X

#define SETTINGS_SETFREQ100_X 55          // 设置界面设定频率百位位置X
#define SETTINGS_SETFREQ10_X 63           // 设置界面设定频率十位位置X
#define SETTINGS_SETFREQ_POINT_X 71       // 设置界面设定频率小数点位置X
#define SETTINGS_SETFREQ1_X 79            // 设置界面设定频率小数位位置X
#define SETTINGS_SETFREQK_X 87            // 设置界面设定频率'k'位置X

#define SETTINGS_SPACE_FD_X 95            // 设置界面频率占空比空格字符位置X

#define SETTINGS_SETDUTY10_X 103          // 设置界面设定占空比十位位置X
#define SETTINGS_SETDUTY1_X 111           // 设置界面设定占空比个位位置X
#define SETTINGS_SETDUTY_PERCENT_X 119    // 设置界面设定占空比'%'位置X

osThreadAttr_t display_task_attr = {
  .name = "DisplayTask",
  .stack_size = 1024,
  .priority = osPriorityNormal
};

// 屏幕显示数据，由指针指向的变量确定
static const Display_State_e        *sp_state           = NULL;
static const Display_OutState_e     *sp_out_state       = NULL;
static const Display_SelBit_e       *sp_cursor          = NULL;
static const Display_SettingsRow_e  *sp_settings_row    = NULL;
static const Display_MainData_t     *sp_main_data       = NULL;
static const Display_SettingsData_t *sp_settings_data   = NULL;

static Display_State_e last_state = DISPLAY_STATE_MAIN;
static Display_OutState_e last_out_state = DISPLAY_OUTSTATE_OFF;
static Display_SelBit_e last_cursor = DISPLAY_SELBIT_NONE;
static Display_SettingsRow_e last_settings_row = DISPLAY_SETTINGS_ROW_DEFAULT;

// 显示互斥量，避免重入覆盖缓存
static osMutexId_t            s_mutex;

// 命令缓存，
static char                   s_buf[512];

static void Display_Task(void *args);

/*********************************************************************
 ******************************* API *********************************
 *********************************************************************/


void Display_Init(  const Display_State_e* gp_display_state,              \
                    const Display_OutState_e* gp_display_out_state,       \
                    const Display_SelBit_e* gp_display_SELBIT,            \
                    const Display_MainData_t* gp_display_main_data,       \
                    const Display_SettingsRow_e* gp_display_settings_row, \
                    const Display_SettingsData_t* gp_display_settings_data
                  ) {
  if (
        gp_display_state          == NULL || \
        gp_display_out_state      == NULL || \
        gp_display_SELBIT         == NULL || \
        gp_display_main_data      == NULL || \
        gp_display_settings_row   == NULL || \
        gp_display_settings_data  == NULL
     ) {
    return;
  }

  sp_state = gp_display_state;
  sp_out_state = gp_display_out_state;
  sp_cursor = gp_display_SELBIT;
  sp_main_data = gp_display_main_data;
  sp_settings_row = gp_display_settings_row;
  sp_settings_data = gp_display_settings_data;
  s_mutex = osMutexNew(NULL);
  osThreadNew(Display_Task, NULL, &display_task_attr);
}

/*********************************************************************
 **************************** INTERNAL *******************************
 *********************************************************************/

/// @brief 内部发送函数
/// @param fmt 
/// @param  
static void Display_Send(const char *fmt, ...) {
  osMutexAcquire(s_mutex, osWaitForever);
  va_list args;
  va_start(args, fmt);
  int len = vsnprintf(s_buf, sizeof(s_buf) - 3, fmt, args);
  va_end(args);
  if (len > 0 && len < (int)(sizeof(s_buf) - 3)) {
    s_buf[len]     = '\r';
    s_buf[len + 1] = '\n';
    BSP_UART_Transmit((uint8_t *)s_buf, (uint16_t)(len + 2));
  }
  osMutexRelease(s_mutex);
}

static void Display_ShowMain_Bg(void) {
  Display_Send("DIR(1);FSIMG(%d,0,0,220,176,0)", IMG_MAIN_BG);
}

static void Display_Show_SetValuesBit(Display_SelBit_e bit) {
  uint8_t bit_bg_color   = SETVOL_BG_COLOR;
  uint8_t bit_font_size  = SETVOL_FONT_SIZE;
  uint8_t bit_font_color = (*sp_cursor == bit) ? SETVALUE_FONT_HIGHLIGHT_COLOR : SETVOL_FONT_COLOR;
  uint8_t bit_pos_x      = SETVOL100_POS_X;
  uint8_t bit_pos_y      = SETVOL100_POS_Y;
  uint8_t bit_value      = sp_main_data->set_voltage / 100;

  switch (bit) {
    case DISPLAY_SELBIT_VOL100:
      bit_bg_color   = SETVOL_BG_COLOR;
      bit_font_size  = SETVOL_FONT_SIZE;
      bit_font_color = (*sp_cursor == bit) ? SETVALUE_FONT_HIGHLIGHT_COLOR : SETVOL_FONT_COLOR;
      bit_pos_x      = SETVOL100_POS_X;
      bit_pos_y      = SETVOL100_POS_Y;
      bit_value      = sp_main_data->set_voltage / 100;
      break;
    case DISPLAY_SELBIT_VOL10:
      bit_bg_color   = SETVOL_BG_COLOR;
      bit_font_size  = SETVOL_FONT_SIZE;
      bit_font_color = (*sp_cursor == bit) ? SETVALUE_FONT_HIGHLIGHT_COLOR : SETVOL_FONT_COLOR;
      bit_pos_x      = SETVOL10_POS_X;
      bit_pos_y      = SETVOL10_POS_Y;
      bit_value      = sp_main_data->set_voltage / 10 % 10;
      break;
    case DISPLAY_SELBIT_VOL1:
      bit_bg_color   = SETVOL_BG_COLOR;
      bit_font_size  = SETVOL_FONT_SIZE;
      bit_font_color = (*sp_cursor == bit) ? SETVALUE_FONT_HIGHLIGHT_COLOR : SETVOL_FONT_COLOR;
      bit_pos_x      = SETVOL1_POS_X;
      bit_pos_y      = SETVOL1_POS_Y;
      bit_value      = sp_main_data->set_voltage % 10;
      break;
    case DISPLAY_SELBIT_FREQ100:
      bit_bg_color   = SETFREQ_BG_COLOR;
      bit_font_size  = SETFREQ_FONT_SIZE;
      bit_font_color = (*sp_cursor == bit) ? SETVALUE_FONT_HIGHLIGHT_COLOR : SETFREQ_FONT_COLOR;
      bit_pos_x      = SETFREQ100_POS_X;
      bit_pos_y      = SETFREQ100_POS_Y;
      bit_value      = sp_main_data->set_freq / 100;
      break;
    case DISPLAY_SELBIT_FREQ10:
      bit_bg_color   = SETFREQ_BG_COLOR;
      bit_font_size  = SETFREQ_FONT_SIZE;
      bit_font_color = (*sp_cursor == bit) ? SETVALUE_FONT_HIGHLIGHT_COLOR : SETFREQ_FONT_COLOR;
      bit_pos_x      = SETFREQ10_POS_X;
      bit_pos_y      = SETFREQ10_POS_Y;
      bit_value      = sp_main_data->set_freq / 10 % 10;
      break;
    case DISPLAY_SELBIT_FREQ1:
      bit_bg_color   = SETFREQ_BG_COLOR;
      bit_font_size  = SETFREQ_FONT_SIZE;
      bit_font_color = (*sp_cursor == bit) ? SETVALUE_FONT_HIGHLIGHT_COLOR : SETFREQ_FONT_COLOR;
      bit_pos_x      = SETFREQ1_POS_X;
      bit_pos_y      = SETFREQ1_POS_Y;
      bit_value      = sp_main_data->set_freq % 10;
      break;
    case DISPLAY_SELBIT_DUTY10:
      bit_bg_color   = SETDUTY_BG_COLOR;
      bit_font_size  = SETDUTY_FONT_SIZE;
      bit_font_color = (*sp_cursor == bit) ? SETVALUE_FONT_HIGHLIGHT_COLOR : SETDUTY_FONT_COLOR;
      bit_pos_x      = SETDUTY10_POS_X;
      bit_pos_y      = SETDUTY10_POS_Y;
      bit_value      = sp_main_data->set_duty / 10 % 10;
      break;
    case DISPLAY_SELBIT_DUTY1:
      bit_bg_color   = SETDUTY_BG_COLOR;
      bit_font_size  = SETDUTY_FONT_SIZE;
      bit_font_color = (*sp_cursor == bit) ? SETVALUE_FONT_HIGHLIGHT_COLOR : SETDUTY_FONT_COLOR;
      bit_pos_x      = SETDUTY1_POS_X;
      bit_pos_y      = SETDUTY1_POS_Y;
      bit_value      = sp_main_data->set_duty % 10;
      break;
    default:
      return;
  }

  Display_Send("SBC(%d);DCV%d(%d,%d,'%d',%d);",
    bit_bg_color,
    bit_font_size, bit_pos_x, bit_pos_y, bit_value, bit_font_color);
}

static void Display_Show_SetValues(void) {
  // 根据当前界面和sp_cursor选中位，计算各数字位的显示颜色
  // 主界面：选中位高亮，否则默认颜色
  // 设置界面：全部使用默认颜色
  uint8_t vol100_color  = SETVOL_FONT_COLOR;
  uint8_t vol10_color   = SETVOL_FONT_COLOR;
  uint8_t vol1_color    = SETVOL_FONT_COLOR;
  uint8_t freq100_color = SETFREQ_FONT_COLOR;
  uint8_t freq10_color  = SETFREQ_FONT_COLOR;
  uint8_t freq1_color   = SETFREQ_FONT_COLOR;
  uint8_t duty10_color  = SETDUTY_FONT_COLOR;
  uint8_t duty1_color   = SETDUTY_FONT_COLOR;

  if (*sp_state == DISPLAY_STATE_MAIN) {
    vol100_color  = (*sp_cursor == DISPLAY_SELBIT_VOL100)  ? SETVALUE_FONT_HIGHLIGHT_COLOR : SETVOL_FONT_COLOR;
    vol10_color   = (*sp_cursor == DISPLAY_SELBIT_VOL10)   ? SETVALUE_FONT_HIGHLIGHT_COLOR : SETVOL_FONT_COLOR;
    vol1_color    = (*sp_cursor == DISPLAY_SELBIT_VOL1)    ? SETVALUE_FONT_HIGHLIGHT_COLOR : SETVOL_FONT_COLOR;

    freq100_color = (*sp_cursor == DISPLAY_SELBIT_FREQ100) ? SETVALUE_FONT_HIGHLIGHT_COLOR : SETFREQ_FONT_COLOR;
    freq10_color  = (*sp_cursor == DISPLAY_SELBIT_FREQ10)  ? SETVALUE_FONT_HIGHLIGHT_COLOR : SETFREQ_FONT_COLOR;
    freq1_color   = (*sp_cursor == DISPLAY_SELBIT_FREQ1)   ? SETVALUE_FONT_HIGHLIGHT_COLOR : SETFREQ_FONT_COLOR;

    duty10_color  = (*sp_cursor == DISPLAY_SELBIT_DUTY10)  ? SETVALUE_FONT_HIGHLIGHT_COLOR : SETDUTY_FONT_COLOR;
    duty1_color   = (*sp_cursor == DISPLAY_SELBIT_DUTY1)   ? SETVALUE_FONT_HIGHLIGHT_COLOR : SETDUTY_FONT_COLOR;
  }

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
    ,

    SETVOL_BG_COLOR,
    SETVOL_FONT_SIZE, SETVOL100_POS_X, SETVOL100_POS_Y, sp_main_data->set_voltage / 100, vol100_color,
    SETVOL_FONT_SIZE, SETVOL10_POS_X, SETVOL10_POS_Y, sp_main_data->set_voltage / 10 % 10, vol10_color,
    SETVOL_FONT_SIZE, SETVOL1_POS_X, SETVOL1_POS_Y, sp_main_data->set_voltage % 10, vol1_color,

    SETFREQ_BG_COLOR,
    SETFREQ_FONT_SIZE, SETFREQ100_POS_X, SETFREQ100_POS_Y, sp_main_data->set_freq / 100, freq100_color,
    SETFREQ_FONT_SIZE, SETFREQ10_POS_X, SETFREQ10_POS_Y, sp_main_data->set_freq / 10 % 10, freq10_color,
    SETFREQ_FONT_SIZE, SETFREQ_POINT_POS_X, SETFREQ_POINT_POS_Y, SETFREQ_FONT_COLOR,
    SETFREQ_FONT_SIZE, SETFREQ1_POS_X, SETFREQ1_POS_Y, sp_main_data->set_freq % 10, freq1_color,

    SETDUTY_BG_COLOR,
    SETDUTY_FONT_SIZE, SETDUTY10_POS_X, SETDUTY10_POS_Y, sp_main_data->set_duty / 10, duty10_color,
    SETDUTY_FONT_SIZE, SETDUTY1_POS_X, SETDUTY1_POS_Y, sp_main_data->set_duty % 10, duty1_color
  );
}

static void Display_Show_InputFreq(void) {
  Display_Send(
    "SBC(%d);"
    "DCV%d(%d,%d,'%d',%d);"
    "DCV%d(%d,%d,'%d',%d);"
    "DCV%d(%d,%d,'.',%d);"
    "DCV%d(%d,%d,'%d',%d);"
    ,
    INPUTFREQ_BG_COLOR,
    INPUTFREQ_FONT_SIZE, INPUTFREQ100_POS_X, INPUTFREQ100_POS_Y, sp_main_data->input_freq / 100, INPUTFREQ_FONT_COLOR,
    INPUTFREQ_FONT_SIZE, INPUTFREQ10_POS_X, INPUTFREQ10_POS_Y, sp_main_data->input_freq / 10 % 10, INPUTFREQ_FONT_COLOR,
    INPUTFREQ_FONT_SIZE, INPUTFREQ_POINT_POS_X, INPUTFREQ_POINT_POS_Y, INPUTFREQ_FONT_COLOR,
    INPUTFREQ_FONT_SIZE, INPUTFREQ1_POS_X, INPUTFREQ1_POS_Y, sp_main_data->input_freq % 10, INPUTFREQ_FONT_COLOR
  );
}

static void Display_ShowMain_Measured(void) {
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
    "DCV%d(%d,%d,'%d',%d);"
    ,

    MAIN_MEASUREDVOL_BG_COLOR,
    MAIN_MEASUREDVOL_FONT_SIZE, MAIN_MEASUREDVOL100_POS_X, MAIN_MEASUREDVOL100_POS_Y, sp_main_data->measured_voltage / 100, MAIN_MEASUREDVOL_FONT_COLOR,
    MAIN_MEASUREDVOL_FONT_SIZE, MAIN_MEASUREDVOL10_POS_X, MAIN_MEASUREDVOL10_POS_Y, sp_main_data->measured_voltage / 10 % 10, MAIN_MEASUREDVOL_FONT_COLOR,
    MAIN_MEASUREDVOL_FONT_SIZE, MAIN_MEASUREDVOL1_POS_X, MAIN_MEASUREDVOL1_POS_Y, sp_main_data->measured_voltage % 10, MAIN_MEASUREDVOL_FONT_COLOR,

    MAIN_MEASUREDCUR_BG_COLOR,
    MAIN_MEASUREDCUR_FONT_SIZE, MAIN_MEASUREDCUR100_POS_X, MAIN_MEASUREDCUR100_POS_Y, sp_main_data->measured_current / 100, MAIN_MEASUREDCUR_FONT_COLOR,
    MAIN_MEASUREDCUR_FONT_SIZE, MAIN_MEASUREDCUR10_POS_X, MAIN_MEASUREDCUR10_POS_Y, sp_main_data->measured_current / 10 % 10, MAIN_MEASUREDCUR_FONT_COLOR,
    MAIN_MEASUREDCUR_FONT_SIZE, MAIN_MEASUREDCUR_POINT_POS_X, MAIN_MEASUREDCUR_POINT_POS_Y, MAIN_MEASUREDCUR_FONT_COLOR,
    MAIN_MEASUREDCUR_FONT_SIZE, MAIN_MEASUREDCUR1_POS_X, MAIN_MEASUREDCUR1_POS_Y, sp_main_data->measured_current % 10, MAIN_MEASUREDCUR_FONT_COLOR,

    MAIN_MEASUREDPOW_BG_COLOR,
    MAIN_MEASUREDPOW_FONT_SIZE, MAIN_MEASUREDPOW100_POS_X, MAIN_MEASUREDPOW100_POS_Y, sp_main_data->measured_power / 100, MAIN_MEASUREDPOW_FONT_COLOR,
    MAIN_MEASUREDPOW_FONT_SIZE, MAIN_MEASUREDPOW10_POS_X, MAIN_MEASUREDPOW10_POS_Y, sp_main_data->measured_power / 10 % 10, MAIN_MEASUREDPOW_FONT_COLOR,
    MAIN_MEASUREDPOW_FONT_SIZE, MAIN_MEASUREDPOW_POINT_POS_X, MAIN_MEASUREDPOW_POINT_POS_Y, MAIN_MEASUREDPOW_FONT_COLOR,
    MAIN_MEASUREDPOW_FONT_SIZE, MAIN_MEASUREDPOW1_POS_X, MAIN_MEASUREDPOW1_POS_Y, sp_main_data->measured_power % 10, MAIN_MEASUREDPOW_FONT_COLOR
  );
}

static void Display_ShowMain_OutState(void) {
  if (*sp_out_state == DISPLAY_OUTSTATE_ON) {
    Display_Send("FSIMG(%d,170,76,26,27,0)", IMG_OUTPUT_ON);
  } else {
    Display_Send("FSIMG(%d,170,76,26,27,0)", IMG_OUTPUT_OFF);
  }
}

static void Display_ShowMain(void) {
  Display_ShowMain_Bg();
  Display_Show_SetValues();
  Display_Show_InputFreq();
  Display_ShowMain_Measured();
  Display_ShowMain_OutState();
}

static void Display_ShowSettings_Bg(void) {
  Display_Send("DIR(1);FSIMG(%d,0,0,220,176,0)", IMG_SETTINGS_BG);
}

static void Display_ShowSettings_PresetGroup(Display_SettingsRow_e row) {
  uint8_t font_bg_color = SETTINGS_ROW_NORMAL_BG_COLOR;

  uint8_t vol100_color = SETTINGS_FONT_NORMAL_COLOR;
  uint8_t vol10_color = SETTINGS_FONT_NORMAL_COLOR;
  uint8_t vol1_color = SETTINGS_FONT_NORMAL_COLOR;
  uint8_t freq100_color = SETTINGS_FONT_NORMAL_COLOR;
  uint8_t freq10_color = SETTINGS_FONT_NORMAL_COLOR;
  uint8_t freq1_color = SETTINGS_FONT_NORMAL_COLOR;
  uint8_t duty10_color = SETTINGS_FONT_NORMAL_COLOR;
  uint8_t duty1_color = SETTINGS_FONT_NORMAL_COLOR;

  uint8_t pos_y = SETTINGS_DEFAULT_ROW_Y;

  const Display_PresetData_t* p_data = &sp_settings_data->default_preset; // 默认预设


  // 当前行被选中，执行高亮操作
  if (*sp_settings_row == row) {
    font_bg_color = SETTINGS_ROW_HIGHLIGHT_BG_COLOR;
    switch (*sp_cursor) {
      case DISPLAY_SELBIT_VOL100:
        vol100_color = SETTINGS_FONT_HIGHLIGHT_COLOR;
        break;
      case DISPLAY_SELBIT_VOL10:
        vol10_color = SETTINGS_FONT_HIGHLIGHT_COLOR;
        break;
      case DISPLAY_SELBIT_VOL1:
        vol1_color = SETTINGS_FONT_HIGHLIGHT_COLOR;
        break;
      case DISPLAY_SELBIT_FREQ100:
        freq100_color = SETTINGS_FONT_HIGHLIGHT_COLOR;
        break;
      case DISPLAY_SELBIT_FREQ10:
        freq10_color = SETTINGS_FONT_HIGHLIGHT_COLOR;
        break;
      case DISPLAY_SELBIT_FREQ1:
        freq1_color = SETTINGS_FONT_HIGHLIGHT_COLOR;
        break;
      case DISPLAY_SELBIT_DUTY10:
        duty10_color = SETTINGS_FONT_HIGHLIGHT_COLOR;
        break;
      case DISPLAY_SELBIT_DUTY1:
        duty1_color = SETTINGS_FONT_HIGHLIGHT_COLOR;
        break;
      default:
        break;
    }
  }

  switch(row) {
    case DISPLAY_SETTINGS_ROW_DEFAULT:
      pos_y = SETTINGS_DEFAULT_ROW_Y;
      p_data = &sp_settings_data->default_preset;
      break;
    case DISPLAY_SETTINGS_ROW_PRESET1:
      pos_y = SETTINGS_PRESET1_ROW_Y;
      p_data = &sp_settings_data->preset1;
      break;
    case DISPLAY_SETTINGS_ROW_PRESET2:
      pos_y = SETTINGS_PRESET2_ROW_Y;
      p_data = &sp_settings_data->preset2;
      break;
    default:
      break;
  }

  Display_Send(
    "SBC(%d);"

    // voltage
    "DCV%d(%d,%d,'%d',%d);"
    "DCV%d(%d,%d,'%d',%d);"
    "DCV%d(%d,%d,'%d',%d);"
    "DCV%d(%d,%d,'V',%d);"

    "DCV%d(%d,%d,' ',%d);"

    // frequency
    "DCV%d(%d,%d,'%d',%d);"
    "DCV%d(%d,%d,'%d',%d);"
    "DCV%d(%d,%d,'.',%d);"
    "DCV%d(%d,%d,'%d',%d);"
    "DCV%d(%d,%d,'k',%d);"

    "DCV%d(%d,%d,' ',%d);"

    // duty
    "DCV%d(%d,%d,'%d',%d);"
    "DCV%d(%d,%d,'%d',%d);"
    "DCV%d(%d,%d,'%%',%d);"
    ,
    font_bg_color,

    // voltage
    SETTINGS_FONT_SIZE, SETTINGS_SETVOL100_X, pos_y, p_data->voltage / 100, vol100_color,
    SETTINGS_FONT_SIZE, SETTINGS_SETVOL10_X, pos_y, p_data->voltage / 10 % 10, vol10_color,
    SETTINGS_FONT_SIZE, SETTINGS_SETVOL1_X, pos_y, p_data->voltage % 10, vol1_color,
    SETTINGS_FONT_SIZE, SETTINGS_SETVOLV_X, pos_y, SETTINGS_FONT_NORMAL_COLOR,

    SETTINGS_FONT_SIZE, SETTINGS_SPACE_VF_X, pos_y, SETTINGS_FONT_NORMAL_COLOR,

    // frequency
    SETTINGS_FONT_SIZE, SETTINGS_SETFREQ100_X, pos_y, p_data->freq / 100, freq100_color,
    SETTINGS_FONT_SIZE, SETTINGS_SETFREQ10_X, pos_y, p_data->freq / 10 % 10, freq10_color,
    SETTINGS_FONT_SIZE, SETTINGS_SETFREQ_POINT_X, pos_y, SETTINGS_FONT_NORMAL_COLOR,
    SETTINGS_FONT_SIZE, SETTINGS_SETFREQ1_X, pos_y, p_data->freq % 10, freq1_color,
    SETTINGS_FONT_SIZE, SETTINGS_SETFREQK_X, pos_y, SETTINGS_FONT_NORMAL_COLOR,

    SETTINGS_FONT_SIZE, SETTINGS_SPACE_FD_X, pos_y, SETTINGS_FONT_NORMAL_COLOR,

    // duty
    SETTINGS_FONT_SIZE, SETTINGS_SETDUTY10_X, pos_y, p_data->duty / 10, duty10_color,
    SETTINGS_FONT_SIZE, SETTINGS_SETDUTY1_X, pos_y, p_data->duty % 10, duty1_color,
    SETTINGS_FONT_SIZE, SETTINGS_SETDUTY_PERCENT_X, pos_y, SETTINGS_FONT_NORMAL_COLOR
  );
}

/// @brief 刷新设置界面预设行中的单个数位
/// @param row 预设行
/// @param bit 数位
static void Display_ShowSettings_PresetBit(Display_SettingsRow_e row, Display_SelBit_e bit) {
  uint8_t pos_x = 0;
  uint8_t pos_y = SETTINGS_DEFAULT_ROW_Y;
  uint8_t value = 0;
  uint8_t font_bg_color = SETTINGS_ROW_NORMAL_BG_COLOR;
  uint8_t font_color = SETTINGS_FONT_NORMAL_COLOR;
  const Display_PresetData_t* p_data = &sp_settings_data->default_preset;

  // 确定行Y位置和数据指针
  switch (row) {
    case DISPLAY_SETTINGS_ROW_DEFAULT:
      pos_y = SETTINGS_DEFAULT_ROW_Y;
      p_data = &sp_settings_data->default_preset;
      break;
    case DISPLAY_SETTINGS_ROW_PRESET1:
      pos_y = SETTINGS_PRESET1_ROW_Y;
      p_data = &sp_settings_data->preset1;
      break;
    case DISPLAY_SETTINGS_ROW_PRESET2:
      pos_y = SETTINGS_PRESET2_ROW_Y;
      p_data = &sp_settings_data->preset2;
      break;
    default:
      return;
  }

  // 确定位X位置和数值
  switch (bit) {
    case DISPLAY_SELBIT_VOL100:
      pos_x = SETTINGS_SETVOL100_X;
      value = p_data->voltage / 100;
      break;
    case DISPLAY_SELBIT_VOL10:
      pos_x = SETTINGS_SETVOL10_X;
      value = p_data->voltage / 10 % 10;
      break;
    case DISPLAY_SELBIT_VOL1:
      pos_x = SETTINGS_SETVOL1_X;
      value = p_data->voltage % 10;
      break;
    case DISPLAY_SELBIT_FREQ100:
      pos_x = SETTINGS_SETFREQ100_X;
      value = p_data->freq / 100;
      break;
    case DISPLAY_SELBIT_FREQ10:
      pos_x = SETTINGS_SETFREQ10_X;
      value = p_data->freq / 10 % 10;
      break;
    case DISPLAY_SELBIT_FREQ1:
      pos_x = SETTINGS_SETFREQ1_X;
      value = p_data->freq % 10;
      break;
    case DISPLAY_SELBIT_DUTY10:
      pos_x = SETTINGS_SETDUTY10_X;
      value = p_data->duty / 10;
      break;
    case DISPLAY_SELBIT_DUTY1:
      pos_x = SETTINGS_SETDUTY1_X;
      value = p_data->duty % 10;
      break;
    default:
      return;
  }

  // 此行被选中：背景高亮，且若此位被选中则字体高亮
  if (*sp_settings_row == row) {
    font_bg_color = SETTINGS_ROW_HIGHLIGHT_BG_COLOR;
    if (*sp_cursor == bit) {
      font_color = SETTINGS_FONT_HIGHLIGHT_COLOR;
    }
  }

  Display_Send("SBC(%d);DCV%d(%d,%d,'%d',%d);",
    font_bg_color,
    SETTINGS_FONT_SIZE, pos_x, pos_y, value, font_color);
}

static void Display_ShowSettings() {
  Display_ShowSettings_Bg();
  Display_Show_SetValues();
  Display_Show_InputFreq();
  Display_ShowSettings_PresetGroup(DISPLAY_SETTINGS_ROW_DEFAULT);
  Display_ShowSettings_PresetGroup(DISPLAY_SETTINGS_ROW_PRESET1);
  Display_ShowSettings_PresetGroup(DISPLAY_SETTINGS_ROW_PRESET2);
}

/// @brief 显示任务：100ms周期，按需增量刷新
static void Display_Task(void* args) {
  Display_ShowMain();
  osDelay(200);

  // ---- 数据变化检测：上一次已知值 ----
  uint16_t last_set_voltage      = sp_main_data->set_voltage;
  uint16_t last_set_freq         = sp_main_data->set_freq;
  uint8_t  last_set_duty         = sp_main_data->set_duty;
  uint16_t last_input_freq       = sp_main_data->input_freq;
  uint16_t last_measured_voltage = sp_main_data->measured_voltage;
  uint16_t last_measured_current = sp_main_data->measured_current;
  uint16_t last_measured_power   = sp_main_data->measured_power;
  Display_SettingsData_t last_settings_data = *sp_settings_data;

  // ---- 各数据独立刷新计数器（100ms/周期，5周期=500ms） ----
  uint8_t measured_cycle_cnt   = 0;  // 实测电压/电流/功率
  uint8_t input_freq_cycle_cnt = 0;  // 输入频率
  uint8_t set_values_cycle_cnt = 0;  // 设定电压/频率/占空比

  for(;;) {
    uint32_t start_tick = osKernelGetTickCount();

    // ---- 界面切换检测 ----
    if (*sp_state != last_state) {
      if (*sp_state == DISPLAY_STATE_MAIN) {
        Display_ShowMain();
      } else if (*sp_state == DISPLAY_STATE_SETTINGS) {
        Display_ShowSettings();
      }
      // 更新所有last状态
      last_state         = *sp_state;
      last_out_state     = *sp_out_state;
      last_cursor        = *sp_cursor;
      last_settings_row  = *sp_settings_row;
      // 更新所有数据副本
      last_set_voltage      = sp_main_data->set_voltage;
      last_set_freq         = sp_main_data->set_freq;
      last_set_duty         = sp_main_data->set_duty;
      last_input_freq       = sp_main_data->input_freq;
      last_measured_voltage = sp_main_data->measured_voltage;
      last_measured_current = sp_main_data->measured_current;
      last_measured_power   = sp_main_data->measured_power;
      last_settings_data    = *sp_settings_data;
      // 重置所有计数器
      measured_cycle_cnt   = 0;
      input_freq_cycle_cnt = 0;
      set_values_cycle_cnt = 0;
      // 界面切换后延时200ms再进行下一轮
      osDelayUntil(start_tick + 200);
      continue;
    }

    // ---- 计数器累加 ----
    measured_cycle_cnt++;
    input_freq_cycle_cnt++;
    set_values_cycle_cnt++;

    // ---- 500ms：设定值（电压/频率/占空比）变化检测（两个界面通用）----
    if (set_values_cycle_cnt >= 5) {
      set_values_cycle_cnt = 0;
      if (last_set_voltage != sp_main_data->set_voltage ||
          last_set_freq    != sp_main_data->set_freq    ||
          last_set_duty    != sp_main_data->set_duty) {
        Display_Show_SetValues();
        last_set_voltage = sp_main_data->set_voltage;
        last_set_freq    = sp_main_data->set_freq;
        last_set_duty    = sp_main_data->set_duty;
      }
    }

    // ---- 500ms：输入频率变化检测（两个界面通用）----
    if (input_freq_cycle_cnt >= 5) {
      input_freq_cycle_cnt = 0;
      if (last_input_freq != sp_main_data->input_freq) {
        Display_Show_InputFreq();
        last_input_freq = sp_main_data->input_freq;
      }
    }

    // ---- 500ms：实测数据变化检测（仅主界面）----
    if (measured_cycle_cnt >= 5) {
      measured_cycle_cnt = 0;
      if (*sp_state == DISPLAY_STATE_MAIN) {
        if (last_measured_voltage != sp_main_data->measured_voltage ||
            last_measured_current != sp_main_data->measured_current ||
            last_measured_power   != sp_main_data->measured_power) {
          Display_ShowMain_Measured();
          last_measured_voltage = sp_main_data->measured_voltage;
          last_measured_current = sp_main_data->measured_current;
          last_measured_power   = sp_main_data->measured_power;
        }
      }
    }

    // ---- 无界面切换，按状态增量刷新 ----
    switch (*sp_state) {
      case DISPLAY_STATE_MAIN: {
        // 输出状态变化
        if (*sp_out_state != last_out_state) {
          Display_ShowMain_OutState();
          last_out_state = *sp_out_state;
        }

        // 检测设定值是否跨位变化（进位/借位导致非光标位也变了）
        if (last_set_voltage != sp_main_data->set_voltage ||
            last_set_freq    != sp_main_data->set_freq    ||
            last_set_duty    != sp_main_data->set_duty) {
          Display_Show_SetValues();
          last_set_voltage = sp_main_data->set_voltage;
          last_set_freq    = sp_main_data->set_freq;
          last_set_duty    = sp_main_data->set_duty;
          last_cursor = *sp_cursor;
        } else if (*sp_cursor != last_cursor) {
          // 光标切换：刷新切换前后的位
          if (last_cursor != DISPLAY_SELBIT_NONE) {
            Display_Show_SetValuesBit(last_cursor);
          }
          if (*sp_cursor != DISPLAY_SELBIT_NONE) {
            Display_Show_SetValuesBit(*sp_cursor);
          }
          last_cursor = *sp_cursor;
        } else if (*sp_cursor != DISPLAY_SELBIT_NONE) {
          // 无光标切换，只刷新当前光标位
          Display_Show_SetValuesBit(*sp_cursor);
        }
        break;
      }
      case DISPLAY_STATE_SETTINGS: {
        // 预设行切换：刷新切换前后的整行（整行刷新已包含数据变化，无需再逐位检测）
        if (*sp_settings_row != last_settings_row) {
          Display_ShowSettings_PresetGroup(last_settings_row);
          Display_ShowSettings_PresetGroup(*sp_settings_row);
          last_settings_row  = *sp_settings_row;
          last_cursor        = *sp_cursor;
          last_settings_data = *sp_settings_data;
        } else {
          // ---- 100ms：设置界面预设数据变化检测（仅在无行切换时）----
          // 默认预设
          if (last_settings_data.default_preset.voltage != sp_settings_data->default_preset.voltage) {
            Display_ShowSettings_PresetBit(DISPLAY_SETTINGS_ROW_DEFAULT, DISPLAY_SELBIT_VOL100);
            Display_ShowSettings_PresetBit(DISPLAY_SETTINGS_ROW_DEFAULT, DISPLAY_SELBIT_VOL10);
            Display_ShowSettings_PresetBit(DISPLAY_SETTINGS_ROW_DEFAULT, DISPLAY_SELBIT_VOL1);
          }
          if (last_settings_data.default_preset.freq != sp_settings_data->default_preset.freq) {
            Display_ShowSettings_PresetBit(DISPLAY_SETTINGS_ROW_DEFAULT, DISPLAY_SELBIT_FREQ100);
            Display_ShowSettings_PresetBit(DISPLAY_SETTINGS_ROW_DEFAULT, DISPLAY_SELBIT_FREQ10);
            Display_ShowSettings_PresetBit(DISPLAY_SETTINGS_ROW_DEFAULT, DISPLAY_SELBIT_FREQ1);
          }
          if (last_settings_data.default_preset.duty != sp_settings_data->default_preset.duty) {
            Display_ShowSettings_PresetBit(DISPLAY_SETTINGS_ROW_DEFAULT, DISPLAY_SELBIT_DUTY10);
            Display_ShowSettings_PresetBit(DISPLAY_SETTINGS_ROW_DEFAULT, DISPLAY_SELBIT_DUTY1);
          }
          // 预设1
          if (last_settings_data.preset1.voltage != sp_settings_data->preset1.voltage) {
            Display_ShowSettings_PresetBit(DISPLAY_SETTINGS_ROW_PRESET1, DISPLAY_SELBIT_VOL100);
            Display_ShowSettings_PresetBit(DISPLAY_SETTINGS_ROW_PRESET1, DISPLAY_SELBIT_VOL10);
            Display_ShowSettings_PresetBit(DISPLAY_SETTINGS_ROW_PRESET1, DISPLAY_SELBIT_VOL1);
          }
          if (last_settings_data.preset1.freq != sp_settings_data->preset1.freq) {
            Display_ShowSettings_PresetBit(DISPLAY_SETTINGS_ROW_PRESET1, DISPLAY_SELBIT_FREQ100);
            Display_ShowSettings_PresetBit(DISPLAY_SETTINGS_ROW_PRESET1, DISPLAY_SELBIT_FREQ10);
            Display_ShowSettings_PresetBit(DISPLAY_SETTINGS_ROW_PRESET1, DISPLAY_SELBIT_FREQ1);
          }
          if (last_settings_data.preset1.duty != sp_settings_data->preset1.duty) {
            Display_ShowSettings_PresetBit(DISPLAY_SETTINGS_ROW_PRESET1, DISPLAY_SELBIT_DUTY10);
            Display_ShowSettings_PresetBit(DISPLAY_SETTINGS_ROW_PRESET1, DISPLAY_SELBIT_DUTY1);
          }
          // 预设2
          if (last_settings_data.preset2.voltage != sp_settings_data->preset2.voltage) {
            Display_ShowSettings_PresetBit(DISPLAY_SETTINGS_ROW_PRESET2, DISPLAY_SELBIT_VOL100);
            Display_ShowSettings_PresetBit(DISPLAY_SETTINGS_ROW_PRESET2, DISPLAY_SELBIT_VOL10);
            Display_ShowSettings_PresetBit(DISPLAY_SETTINGS_ROW_PRESET2, DISPLAY_SELBIT_VOL1);
          }
          if (last_settings_data.preset2.freq != sp_settings_data->preset2.freq) {
            Display_ShowSettings_PresetBit(DISPLAY_SETTINGS_ROW_PRESET2, DISPLAY_SELBIT_FREQ100);
            Display_ShowSettings_PresetBit(DISPLAY_SETTINGS_ROW_PRESET2, DISPLAY_SELBIT_FREQ10);
            Display_ShowSettings_PresetBit(DISPLAY_SETTINGS_ROW_PRESET2, DISPLAY_SELBIT_FREQ1);
          }
          if (last_settings_data.preset2.duty != sp_settings_data->preset2.duty) {
            Display_ShowSettings_PresetBit(DISPLAY_SETTINGS_ROW_PRESET2, DISPLAY_SELBIT_DUTY10);
            Display_ShowSettings_PresetBit(DISPLAY_SETTINGS_ROW_PRESET2, DISPLAY_SELBIT_DUTY1);
          }
          // 更新设置数据副本
          last_settings_data = *sp_settings_data;

          // 光标切换：刷新切换前后的位
          if (*sp_cursor != last_cursor) {
            if (last_cursor != DISPLAY_SELBIT_NONE) {
              Display_ShowSettings_PresetBit(*sp_settings_row, last_cursor);
            }
            if (*sp_cursor != DISPLAY_SELBIT_NONE) {
              Display_ShowSettings_PresetBit(*sp_settings_row, *sp_cursor);
            }
            last_cursor = *sp_cursor;
          }
        }
        break;
      }
      default:
        break;
    }

    osDelayUntil(start_tick + 100);
  }
}

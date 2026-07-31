/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-07-30 17:19:16
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-07-30 18:13:25
 * @FilePath: \code\Proj\Components\tools\pid.h
 * @Description: 
 * 本项目只用于控制THz专用激励源的控制，切勿用于其他用途，否则西安理工大学及开发者不承担任何责任。
 * Copyright (c) 2026 by ${git_name_email}, All Rights Reserved. 
 */
#include "main.h"

/// @brief PID mode enum
typedef enum {
  PID_MODE_POSITION = 0,
  PID_MODE_DELTA
} PIDMode_e;


/// @brief pid struct
typedef struct {
  PIDMode_e mode;

  float kp;
  float ki;
  float kd;
  float integral_limit_max;
  float integral_limit_min;
  float out_limit_max;
  float out_limit_min;

  float set;
  float fdb;

  float d_buf[3];
  float err_buf[3];

  float kp_out;
  float ki_out;
  float kd_out;
  float out;
} PID_t;

/// @brief Initialize PID struct
/// @param p_pid pid pointer
/// @param mode pid mode
/// @param kp proportional gain
/// @param ki integral gain
/// @param kd derivative gain
/// @param out_limit_max maximum output limit
/// @param out_limit_min minimum output limit
/// @param integral_limit_max maximum integral limit
/// @param integral_limit_min minimum integral limit
void PID_Init(PID_t *p_pid, PIDMode_e mode, float kp, float ki, float kd, \
              float out_limit_max, float out_limit_min, float integral_limit_max, float integral_limit_min);

/// @brief Calculate PID output
/// @param p_pid pid pointer
/// @param set setpoint
/// @param feedback feedback value
/// @return PID output
float PID_Calc(PID_t *p_pid, float set, float feedback);

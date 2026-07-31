/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-07-30 17:19:16
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-07-30 18:13:15
 * @FilePath: \code\Proj\Components\tools\pid.c
 * @Description: 
 * 本项目只用于控制THz专用激励源的控制，切勿用于其他用途，否则西安理工大学及开发者不承担任何责任。
 * Copyright (c) 2026 by ${git_name_email}, All Rights Reserved. 
 */
#include "pid.h"

void PID_Init(PID_t *p_pid, PIDMode_e mode, float kp, float ki, float kd, \
              float out_limit_max, float out_limit_min, float integral_limit_max, float integral_limit_min) {
  if (p_pid == NULL) {
    return;
  }

  p_pid->mode = mode;
  p_pid->kp = kp;
  p_pid->ki = ki;
  p_pid->kd = kd;
  p_pid->out_limit_max = out_limit_max;
  p_pid->out_limit_min = out_limit_min;
  p_pid->integral_limit_max = integral_limit_max;
  p_pid->integral_limit_min = integral_limit_min;

  p_pid->set = 0.f;
  p_pid->fdb = 0.f;

  p_pid->err_buf[0] = 0.f;
  p_pid->err_buf[1] = 0.f;
  p_pid->kp_out = 0.f;
  p_pid->ki_out = 0.f;
  p_pid->kd_out = 0.f;
  p_pid->out = 0.f;
}

float PID_Calc(PID_t *p_pid, float set, float feedback) {
  float tmp_out = 0;

  // error update and calculate
  p_pid->err_buf[2] = p_pid->err_buf[1];
  p_pid->err_buf[1] = p_pid->err_buf[0];
  p_pid->err_buf[0] = set - feedback;

  // error diffrential update
  p_pid->d_buf[2] = p_pid->d_buf[1];
  p_pid->d_buf[1] = p_pid->d_buf[0];

  if (p_pid->mode == PID_MODE_POSITION) {
    // error diffrential calculation
    p_pid->d_buf[0] = p_pid->err_buf[0] - p_pid->err_buf[1];

    // P D out calculation
    p_pid->kp_out = p_pid->kp * p_pid->err_buf[0];
    p_pid->kd_out = p_pid->kd * p_pid->d_buf[0];

    // integral calculation and limit
    float tmp_ki_out = p_pid->ki_out + p_pid->ki * p_pid->err_buf[0];
    if (tmp_ki_out > p_pid->integral_limit_max) {
      tmp_ki_out = p_pid->integral_limit_max;
    } else if (tmp_ki_out < p_pid->integral_limit_min) {
      tmp_ki_out = p_pid->integral_limit_min;
    }
    p_pid->ki_out = tmp_ki_out;

    // output calculation
    tmp_out = p_pid->kp_out + p_pid->ki_out + p_pid->kd_out;
  }
  else if (p_pid->mode == PID_MODE_DELTA) {
    // P I out calculation
    p_pid->kp_out = p_pid->kp * (p_pid->err_buf[0] - p_pid->err_buf[1]);
    p_pid->ki_out = p_pid->ki * p_pid->err_buf[0]; // delta PID does not need integral limit

    // error diffrential and D out calculation
    p_pid->d_buf[0] = (p_pid->err_buf[0] - 2.0f * p_pid->err_buf[1] + p_pid->err_buf[2]);
    p_pid->kd_out = p_pid->kd * p_pid->d_buf[0];

    // output calculation
    tmp_out = p_pid->out + p_pid->kp_out + p_pid->ki_out + p_pid->kd_out;
  }

  if (tmp_out > p_pid->out_limit_max) {
    tmp_out = p_pid->out_limit_max;
  } else if (tmp_out < p_pid->out_limit_min) {
    tmp_out = p_pid->out_limit_min;
  }
  p_pid->out = tmp_out;

  return p_pid->out;
}

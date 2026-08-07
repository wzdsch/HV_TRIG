/*
 * @Author: Jiang Tianhang 1919524828@qq.com
 * @Date: 2026-08-04 15:26:52
 * @LastEditors: Jiang Tianhang 1919524828@qq.com
 * @LastEditTime: 2026-08-07 22:14:05
 * @FilePath: \code\Proj\Components\app\logic.c
 * @Description: 
 * 本项目只用于控制THz专用激励源的控制，切勿用于其他用途，否则西安理工大学及开发者不承担任何责任。
 * Copyright (c) 2026 by Jiang Tianhang 1919524828@qq.com, All Rights Reserved. 
 */
#include "logic.h"
#include "usr_tasks.h"
#include "EEPROM.h"

#define MIN_SET_VOLTAGE 0   // 单位：V
#define MAX_SET_VOLTAGE 200 // 单位：V

#define MIN_SET_FREQ 100  // 单位：0.1kHz
#define MAX_SET_FREQ 300  // 单位：0.1kHz

#define MIN_SET_DUTY 1   // 单位：%
#define MAX_SET_DUTY 99 // 单位：%

extern Display_State_e g_display_state;
extern Display_OutState_e g_display_out_state;
extern Display_SelBit_e g_cursor;
extern Display_SettingsRow_e g_settings_row;
extern Display_MainData_t g_main_data;
extern Display_SettingsData_t g_settings_data;

// ENTER键：在SETTING界面选择预设并保存预设
void Key2_Down_Logic(void) {
  if (g_display_state == DISPLAY_STATE_SETTINGS) {
    EEPROM_SaveSettings(&g_settings_data);
    Display_PresetData_t *p_preset = &g_settings_data.default_preset;
    switch (g_settings_row) {
    case DISPLAY_SETTINGS_ROW_DEFAULT:
      p_preset = &g_settings_data.default_preset;
      break;
    case DISPLAY_SETTINGS_ROW_PRESET1:
      p_preset = &g_settings_data.preset1;
      break;
    case DISPLAY_SETTINGS_ROW_PRESET2:
      p_preset = &g_settings_data.preset2;
      break;
    }
    g_main_data.set_voltage = p_preset->voltage;
    g_main_data.set_freq = p_preset->freq;
    g_main_data.set_duty = p_preset->duty;
    g_display_state = DISPLAY_STATE_MAIN;
    g_cursor = DISPLAY_SELBIT_NONE;
    g_settings_row = DISPLAY_SETTINGS_ROW_DEFAULT;
  }
}

// T/DOWN
void Key0_Down_Logic(void) {
  // 主界面中，选择频率/占空比
  if (g_display_state == DISPLAY_STATE_MAIN) {
    if (g_cursor == DISPLAY_SELBIT_DUTY10 || \
        g_cursor == DISPLAY_SELBIT_DUTY1) {
      g_cursor = DISPLAY_SELBIT_NONE;
    }
    else if (g_cursor != DISPLAY_SELBIT_FREQ100 && \
        g_cursor != DISPLAY_SELBIT_FREQ10 && \
        g_cursor != DISPLAY_SELBIT_FREQ1) {
      g_cursor = DISPLAY_SELBIT_FREQ1;
    }
    else {
      g_cursor = DISPLAY_SELBIT_DUTY1;
    }
  }

  // 设置界面中，选择下一行
  if (g_display_state == DISPLAY_STATE_SETTINGS) {
    if (g_settings_row == DISPLAY_SETTINGS_ROW_DEFAULT) {
      g_settings_row = DISPLAY_SETTINGS_ROW_PRESET1;
    }
    else if (g_settings_row == DISPLAY_SETTINGS_ROW_PRESET1) {
      g_settings_row = DISPLAY_SETTINGS_ROW_PRESET2;
    }
    else if (g_settings_row == DISPLAY_SETTINGS_ROW_PRESET2) {
      g_settings_row = DISPLAY_SETTINGS_ROW_DEFAULT;
    }
    g_cursor = DISPLAY_SELBIT_NONE;
  }
}

// V/UP
void Key1_Down_Logic(void) {
  // 在主界面中，选择电压
  if (g_display_state == DISPLAY_STATE_MAIN) {
    if (g_cursor != DISPLAY_SELBIT_VOL100 && \
        g_cursor != DISPLAY_SELBIT_VOL10 && \
        g_cursor != DISPLAY_SELBIT_VOL1) {
      g_cursor = DISPLAY_SELBIT_VOL1;
    }
    else {
      g_cursor = DISPLAY_SELBIT_NONE;
    }
  }

  // 在设置界面中，选择上一行
  if (g_display_state == DISPLAY_STATE_SETTINGS) {
    if (g_settings_row == DISPLAY_SETTINGS_ROW_DEFAULT) {
      g_settings_row = DISPLAY_SETTINGS_ROW_PRESET2;
    }
    else if (g_settings_row == DISPLAY_SETTINGS_ROW_PRESET1) {
      g_settings_row = DISPLAY_SETTINGS_ROW_DEFAULT;
    }
    else if (g_settings_row == DISPLAY_SETTINGS_ROW_PRESET2) {
      g_settings_row = DISPLAY_SETTINGS_ROW_PRESET1;
    }
    g_cursor = DISPLAY_SELBIT_NONE;
  }
}

// Menu: 切换主界面/设置界面
void Key4_Down_Logic(void) {
  if (g_display_state == DISPLAY_STATE_MAIN) {
    g_display_state = DISPLAY_STATE_SETTINGS;
    g_cursor = DISPLAY_SELBIT_NONE;
    g_settings_row = DISPLAY_SETTINGS_ROW_DEFAULT;
  }
  else if (g_display_state == DISPLAY_STATE_SETTINGS) {
    g_display_state = DISPLAY_STATE_MAIN;
    g_cursor = DISPLAY_SELBIT_NONE;
    g_settings_row = DISPLAY_SETTINGS_ROW_DEFAULT;
    EEPROM_SaveSettings(&g_settings_data);
  }
}

// ADJ/SET
void Key3_Down_Logic(void) {
  // 在主界面中，在数据范围内向高位移动光标
  if (g_display_state == DISPLAY_STATE_MAIN) {
    if (g_cursor == DISPLAY_SELBIT_VOL1) {
      g_cursor = DISPLAY_SELBIT_VOL10;
    }
    else if (g_cursor == DISPLAY_SELBIT_VOL10) {
      g_cursor = DISPLAY_SELBIT_VOL100;
    }
    else if (g_cursor == DISPLAY_SELBIT_VOL100) {
      g_cursor = DISPLAY_SELBIT_VOL1;
    }
    else if (g_cursor == DISPLAY_SELBIT_FREQ1) {
      g_cursor = DISPLAY_SELBIT_FREQ10;
    }
    else if (g_cursor == DISPLAY_SELBIT_FREQ10) {
      g_cursor = DISPLAY_SELBIT_FREQ100;
    }
    else if (g_cursor == DISPLAY_SELBIT_FREQ100) {
      g_cursor = DISPLAY_SELBIT_FREQ1;
    }
    else if (g_cursor == DISPLAY_SELBIT_DUTY1) {
      g_cursor = DISPLAY_SELBIT_DUTY10;
    }
    else if (g_cursor == DISPLAY_SELBIT_DUTY10) {
      g_cursor = DISPLAY_SELBIT_DUTY1;
    }
  }

  // 在设置界面中，在选择预设的数据组内，由左向右移动光标
  if (g_display_state == DISPLAY_STATE_SETTINGS) {
    if (g_cursor == DISPLAY_SELBIT_NONE) {
      g_cursor = DISPLAY_SELBIT_VOL100;
    }
    else if (g_cursor == DISPLAY_SELBIT_VOL100) {
      g_cursor = DISPLAY_SELBIT_VOL10;
    }
    else if (g_cursor == DISPLAY_SELBIT_VOL10) {
      g_cursor = DISPLAY_SELBIT_VOL1;
    }
    else if (g_cursor == DISPLAY_SELBIT_VOL1) {
      g_cursor = DISPLAY_SELBIT_FREQ100;
    }
    else if (g_cursor == DISPLAY_SELBIT_FREQ100) {
      g_cursor = DISPLAY_SELBIT_FREQ10;
    }
    else if (g_cursor == DISPLAY_SELBIT_FREQ10) {
      g_cursor = DISPLAY_SELBIT_FREQ1;
    }
    else if (g_cursor == DISPLAY_SELBIT_FREQ1) {
      g_cursor = DISPLAY_SELBIT_DUTY10;
    }
    else if (g_cursor == DISPLAY_SELBIT_DUTY10) {
      g_cursor = DISPLAY_SELBIT_DUTY1;
    }
    else if (g_cursor == DISPLAY_SELBIT_DUTY1) {
      g_cursor = DISPLAY_SELBIT_NONE;
    }
  }
}

// ON/OFF : 在主界面中切换输出状态
void Key5_Down_Logic(void) {
  if (g_display_state == DISPLAY_STATE_MAIN) {
    if (g_display_out_state == DISPLAY_OUTSTATE_ON) {
      g_display_out_state = DISPLAY_OUTSTATE_OFF;
    }
    else if (DISPLAY_OUTSTATE_ON) {
      g_display_out_state = DISPLAY_OUTSTATE_ON;
    }
  }
}

// 编码器，加减光标位
void Encoder_Logic(int32_t delta) {
  if (g_cursor == DISPLAY_SELBIT_NONE) {
    return;
  }
  if (g_display_state == DISPLAY_STATE_MAIN) {
    uint16_t temp_set_vol = g_main_data.set_voltage;
    uint16_t temp_set_freq = g_main_data.set_freq;

    // 这里不使用uint8_t，因为编码值变化量过大，会引起超范围，后面几处也一样
    uint16_t temp_set_duty = g_main_data.set_duty;

    switch (g_cursor) {
      case DISPLAY_SELBIT_VOL1:
        temp_set_vol += delta;
        break;
      case DISPLAY_SELBIT_VOL10:
        temp_set_vol += delta * 10;
        break;
      case DISPLAY_SELBIT_VOL100:
        temp_set_vol += delta * 100;
        break;
      case DISPLAY_SELBIT_FREQ1:
        temp_set_freq += delta;
        break;
      case DISPLAY_SELBIT_FREQ10:
        temp_set_freq += delta * 10;
        break;
      case DISPLAY_SELBIT_FREQ100:
        temp_set_freq += delta * 100;
        break;
      case DISPLAY_SELBIT_DUTY1:
        temp_set_duty += delta;
        break;
      case DISPLAY_SELBIT_DUTY10:
        temp_set_duty += delta * 10;
        break;
      default:
        break;
    }
    if (temp_set_vol > MAX_SET_VOLTAGE) {
      temp_set_vol = MAX_SET_VOLTAGE;
    }
    if (temp_set_vol < MIN_SET_VOLTAGE) {
      temp_set_vol = MIN_SET_VOLTAGE;
    }
    if (temp_set_freq > MAX_SET_FREQ) {
      temp_set_freq = MAX_SET_FREQ;
    }
    if (temp_set_freq < MIN_SET_FREQ) {
      temp_set_freq = MIN_SET_FREQ;
    }
    if (temp_set_duty > MAX_SET_DUTY) {
      temp_set_duty = MAX_SET_DUTY;
    }
    if (temp_set_duty < MIN_SET_DUTY) {
      temp_set_duty = MIN_SET_DUTY;
    }
    g_main_data.set_voltage = temp_set_vol;
    g_main_data.set_freq = temp_set_freq;
    g_main_data.set_duty = temp_set_duty;
  }
  if (g_display_state == DISPLAY_STATE_SETTINGS) {
    if (g_settings_row == DISPLAY_SETTINGS_ROW_DEFAULT) {
      uint16_t temp_set_vol = g_settings_data.default_preset.voltage;
      uint16_t temp_set_freq = g_settings_data.default_preset.freq;
      uint16_t temp_set_duty = g_settings_data.default_preset.duty;
      switch (g_cursor) {
        case DISPLAY_SELBIT_VOL1:
          temp_set_vol += delta;
          break;
        case DISPLAY_SELBIT_VOL10:
          temp_set_vol += delta * 10;
          break;
        case DISPLAY_SELBIT_VOL100:
          temp_set_vol += delta * 100;
          break;
        case DISPLAY_SELBIT_FREQ1:
          temp_set_freq += delta;
          break;
        case DISPLAY_SELBIT_FREQ10:
          temp_set_freq += delta * 10;
          break;
        case DISPLAY_SELBIT_FREQ100:
          temp_set_freq += delta * 100;
          break;
        case DISPLAY_SELBIT_DUTY1:
          temp_set_duty += delta;
          break;
        case DISPLAY_SELBIT_DUTY10:
          temp_set_duty += delta * 10;
          break;
        default:
          break;
      }
      if (temp_set_vol > MAX_SET_VOLTAGE) {
        temp_set_vol = MAX_SET_VOLTAGE;
      }
      if (temp_set_vol < MIN_SET_VOLTAGE) {
        temp_set_vol = MIN_SET_VOLTAGE;
      }
      if (temp_set_freq > MAX_SET_FREQ) {
        temp_set_freq = MAX_SET_FREQ;
      }
      if (temp_set_freq < MIN_SET_FREQ) {
        temp_set_freq = MIN_SET_FREQ;
      }
      if (temp_set_duty > MAX_SET_DUTY) {
        temp_set_duty = MAX_SET_DUTY;
      }
      if (temp_set_duty < MIN_SET_DUTY) {
        temp_set_duty = MIN_SET_DUTY;
      }
      g_settings_data.default_preset.voltage = temp_set_vol;
      g_settings_data.default_preset.freq = temp_set_freq;
      g_settings_data.default_preset.duty = temp_set_duty;
    }
    else if (g_settings_row == DISPLAY_SETTINGS_ROW_PRESET1) {
      uint16_t temp_set_vol = g_settings_data.preset1.voltage;
      uint16_t temp_set_freq = g_settings_data.preset1.freq;
      uint16_t temp_set_duty = g_settings_data.preset1.duty;
      switch (g_cursor) {
        case DISPLAY_SELBIT_VOL1:
          temp_set_vol += delta;
          break;
        case DISPLAY_SELBIT_VOL10:
          temp_set_vol += delta * 10;
          break;
        case DISPLAY_SELBIT_VOL100:
          temp_set_vol += delta * 100;
          break;
        case DISPLAY_SELBIT_FREQ1:
          temp_set_freq += delta;
          break;
        case DISPLAY_SELBIT_FREQ10:
          temp_set_freq += delta * 10;
          break;
        case DISPLAY_SELBIT_FREQ100:
          temp_set_freq += delta * 100;
          break;
        case DISPLAY_SELBIT_DUTY1:
          temp_set_duty += delta;
          break;
        case DISPLAY_SELBIT_DUTY10:
          temp_set_duty += delta * 10;
          break;
        default:
          break;
      }
      if (temp_set_vol > MAX_SET_VOLTAGE) {
        temp_set_vol = MAX_SET_VOLTAGE;
      }
      if (temp_set_vol < MIN_SET_VOLTAGE) {
        temp_set_vol = MIN_SET_VOLTAGE;
      }
      if (temp_set_freq > MAX_SET_FREQ) {
        temp_set_freq = MAX_SET_FREQ;
      }
      if (temp_set_freq < MIN_SET_FREQ) {
        temp_set_freq = MIN_SET_FREQ;
      }
      if (temp_set_duty > MAX_SET_DUTY) {
        temp_set_duty = MAX_SET_DUTY;
      }
      if (temp_set_duty < MIN_SET_DUTY) {
        temp_set_duty = MIN_SET_DUTY;
      }
      g_settings_data.preset1.voltage = temp_set_vol;
      g_settings_data.preset1.freq = temp_set_freq;
      g_settings_data.preset1.duty = temp_set_duty;
    }
    else if (g_settings_row == DISPLAY_SETTINGS_ROW_PRESET2) {
      uint16_t temp_set_vol = g_settings_data.preset2.voltage;
      uint16_t temp_set_freq = g_settings_data.preset2.freq;
      uint16_t temp_set_duty = g_settings_data.preset2.duty;
      switch (g_cursor) {
        case DISPLAY_SELBIT_VOL1:
          temp_set_vol += delta;
          break;
        case DISPLAY_SELBIT_VOL10:
          temp_set_vol += delta * 10;
          break;
        case DISPLAY_SELBIT_VOL100:
          temp_set_vol += delta * 100;
          break;
        case DISPLAY_SELBIT_FREQ1:
          temp_set_freq += delta;
          break;
        case DISPLAY_SELBIT_FREQ10:
          temp_set_freq += delta * 10;
          break;
        case DISPLAY_SELBIT_FREQ100:
          temp_set_freq += delta * 100;
          break;
        case DISPLAY_SELBIT_DUTY1:
          temp_set_duty += delta;
          break;
        case DISPLAY_SELBIT_DUTY10:
          temp_set_duty += delta * 10;
          break;
        default:
          break;
      }
      if (temp_set_vol > MAX_SET_VOLTAGE) {
        temp_set_vol = MAX_SET_VOLTAGE;
      }
      if (temp_set_vol < MIN_SET_VOLTAGE) {
        temp_set_vol = MIN_SET_VOLTAGE;
      }
      if (temp_set_freq > MAX_SET_FREQ) {
        temp_set_freq = MAX_SET_FREQ;
      }
      if (temp_set_freq < MIN_SET_FREQ) {
        temp_set_freq = MIN_SET_FREQ;
      }
      if (temp_set_duty > MAX_SET_DUTY) {
        temp_set_duty = MAX_SET_DUTY;
      }
      if (temp_set_duty < MIN_SET_DUTY) {
        temp_set_duty = MIN_SET_DUTY;
      }
      g_settings_data.preset2.voltage = temp_set_vol;
      g_settings_data.preset2.freq = temp_set_freq;
      g_settings_data.preset2.duty = temp_set_duty;
    }
  }
}

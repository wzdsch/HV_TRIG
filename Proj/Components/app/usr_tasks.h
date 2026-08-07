#ifndef USR_TASKS_H
#define USR_TASKS_H

#include "display.h"

extern Display_State_e g_display_state;
extern Display_OutState_e g_display_out_state;
extern Display_SelBit_e g_cursor;
extern Display_SettingsRow_e g_settings_row;
extern Display_MainData_t g_main_data;
extern Display_SettingsData_t g_settings_data;

void UartTask(void *argument);

#endif

#ifndef BSP_UART_H
#define BSP_UART_H

#include "main.h"

/// @brief Create the UART task and message queue.
/// @param  
void BSP_UART_Init(void);

/// @brief Transmit data by queue (never used in interrupt)
/// @param p_data pointer to the data
/// @param size size of the data
/// @return success: 0, error: 1
uint8_t BSP_UART_Transmit(uint8_t* p_data, uint8_t size);

#endif

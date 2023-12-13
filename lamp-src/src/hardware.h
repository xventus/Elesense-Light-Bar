//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   hardware.h
/// @author Petr Vanek

#pragma once

#include "driver/gpio.h"
#include "driver/uart.h"

/**
 * @brief Hardware definition, pin assignment, communication interfaces settings
 * 
 */


// LEDs
#define  HEART_BEAT_LED  GPIO_NUM_0
#define  DATA_LED        GPIO_NUM_2

// Buttons
#define  FLASH_BUTTON    GPIO_NUM_0
#define  A_BUTTON        GPIO_NUM_2
#define  B_BUTTON        GPIO_NUM_4

// Serial line to LSC12C
#define LSC_TX_PIN  GPIO_NUM_25 // connect to RX on LSC12C
#define LSC_RX_PIN  GPIO_NUM_26 // connect to TX pin on LSC12C
#define LSC_SET     GPIO_NUM_19 // set mode 
#define LSC_CS      GPIO_NUM_18 // chip select

// USED UART
#define LCS_UART    UART_NUM_1

// Aux free pins - not used 
#define I2C_SDA GPIO_NUM_21
#define I2C_SCL GPIO_NUM_22
#define UA_PIN  GPIO_NUM_27
#define UB_PIN  GPIO_NUM_33
#define UC_PIN  GPIO_NUM_32


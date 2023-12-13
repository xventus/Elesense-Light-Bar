//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   button.h
/// @author Petr Vanek

#pragma once

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/// @brief  Encapsulate button 
class Button {
public:

    /// @brief CTOR
    /// @param gpioPin button pin 
    Button(gpio_num_t gpioPin) : _gpioPin(gpioPin), _lastState(false), _buttonPressed(false) {}

    /// @brief initialize
    void init() {
        gpio_config_t io_conf{};
        // disable interrupt
        io_conf.intr_type = GPIO_INTR_DISABLE;
        // set as input mode
        io_conf.mode = GPIO_MODE_INPUT;
        // bit mask of the pins
        io_conf.pin_bit_mask = (1ULL << _gpioPin);
        // enable pull-up mode
        io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        gpio_config(&io_conf);
    }

    /// @brief Check if button pressed
    /// @return 
    bool isPressed() {
        if (gpio_get_level(_gpioPin) == 0) {
            // Wait for debouncing
            vTaskDelay(_debounceTime / portTICK_PERIOD_MS);
            // Check if the button is still pressed
            return gpio_get_level(_gpioPin) == 0;
        }
        return false;
    }

    /// @brief Check if button clicked
    /// @return 
    bool click() {
        bool currentState = isPressed();
        if (currentState && !_lastState) {
            _lastState = true;
            return true;
        }

        if (!currentState) {
            _lastState = false;
        }

        return false;
    }

    /// @brief Check if button released
    /// @return 
    bool isReleased() {
        bool currentState = isPressed();
        if (!_buttonPressed && currentState) {
            _buttonPressed = true;
        } else if (_buttonPressed && !currentState) {
            _buttonPressed = false;
            return true; 
        }
        return false;
    }

    /// @brief Sets new debounce time
    /// @param debounceTime 
    void setDebounceTime(uint32_t debounceTime) {
        _debounceTime = debounceTime;
    }

private:
    gpio_num_t _gpioPin;         ///< GPIO pin
    uint32_t _debounceTime = 20; ///< Default debounce time in milliseconds
    bool _lastState;             ///< Last state 
    bool _buttonPressed;         ///< If button pressed
};
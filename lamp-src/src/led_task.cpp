
//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   led_task.cpp
/// @author Petr Vanek

#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include "led_task.h"



LedTask::LedTask(gpio_num_t pin) : _pin(pin) { 
	_queue = xQueueCreate( 5 , sizeof(BlinkMode));
}

LedTask::~LedTask() {
	done();
	if (_queue) vQueueDelete(_queue);
}


    void LedTask::loop() {

  gpio_set_direction(_pin, GPIO_MODE_OUTPUT);

    BlinkMode mode = BlinkMode::NONE; 

    while (true) {
        BlinkMode req;
        if (xQueueReceive(_queue, &req, 0) == pdTRUE) {
            mode = req;
        }

        switch (mode) {
			case BlinkMode::NONE:
 				gpio_set_level(_pin, 0);
                vTaskDelay(100 / portTICK_PERIOD_MS);
			break;
            case BlinkMode::AP_MODE:
                // AP mode .. .. .. 
                for (int i = 0; i < 2; i++) {
                    gpio_set_level(_pin, 1);
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                    gpio_set_level(_pin, 0);
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                }
                vTaskDelay(500 / portTICK_PERIOD_MS);
                break;

            case BlinkMode::CLIENT:
                // CLIENT - - - - 
                gpio_set_level(_pin, 1);
                vTaskDelay(500 / portTICK_PERIOD_MS);
                gpio_set_level(_pin, 0);
                vTaskDelay(500 / portTICK_PERIOD_MS);
                break;

            case BlinkMode::ERROR:
                // ERROR  . . . . . 
                gpio_set_level(_pin, 1);
                vTaskDelay(100 / portTICK_PERIOD_MS);
                gpio_set_level(_pin, 0);
                vTaskDelay(100 / portTICK_PERIOD_MS);
                break;

			case BlinkMode::LEARN:
                // LEARN    . -  . -
                gpio_set_level(_pin, 1);
                vTaskDelay(100 / portTICK_PERIOD_MS); 
                gpio_set_level(_pin, 0);
                vTaskDelay(100 / portTICK_PERIOD_MS); 

                gpio_set_level(_pin, 1);
                vTaskDelay(500 / portTICK_PERIOD_MS); 
                gpio_set_level(_pin, 0);
                vTaskDelay(500 / portTICK_PERIOD_MS); 
                break;
        }
    }
}



 void LedTask::mode(BlinkMode mode) {

	if (_queue) {
        xQueueSendToBack(_queue, &mode, 0); 
    }
 }
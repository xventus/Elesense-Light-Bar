
//
// vim: ts=4 et
// Copyright (c) 2024 Petr Vanek, petr@fotoventus.cz
//
/// @file   button_task.cpp
/// @author Petr Vanek

#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include "button_task.h"
#include "application.h"

ButtonTask::ButtonTask(gpio_num_t buttonA, gpio_num_t buttonB, gpio_num_t buttonX) : _ba(buttonA), _bb(buttonB), _b(buttonX)
{
}

ButtonTask::~ButtonTask()
{
    done();
}

void ButtonTask::loop()
{
    uint8_t intensity = 0;  // TODO from configuration & receiver
    while (true)
    {

        if (_b.click()) {
           // X-BOOT button ON / OFF
		   Application::getInstance()->getLcsTask()->command(LC12STask::Command::toggle);
        } else {
            if (_ba.isPressed()) {
            if (intensity < 0x17) intensity++;
            Application::getInstance()->getLcsTask()->intensity(intensity);
            }
            
            if (_bb.isPressed()) {
            if (intensity > 0x00) intensity--;
            Application::getInstance()->getLcsTask()->intensity(intensity);
            }
        }

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}


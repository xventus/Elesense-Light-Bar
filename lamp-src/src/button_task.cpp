
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
    while (true)
    {

        if (_b.click()) {
           // X-BOOT button ON / OFF
		   Application::getInstance()->getLcsTask()->command(LC12STask::Command::toggle);
        } else {
            if (_ba.isPressed()) {
                Application::getInstance()->getLcsTask()->command(LC12STask::Command::incIntensity);
                vTaskDelay(200 / portTICK_PERIOD_MS);
            }
            
            if (_bb.isPressed()) {
                Application::getInstance()->getLcsTask()->command(LC12STask::Command::decIntensity);
                vTaskDelay(200 / portTICK_PERIOD_MS);
            }
        }

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}


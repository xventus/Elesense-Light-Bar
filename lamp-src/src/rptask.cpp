//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   rptask.cpp
/// @author Petr Vanek

#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include "rptask.h"

RPTask::RPTask() {
    ;
}

RPTask::~RPTask()
{
    done();
}

void RPTask::done()
{
    if (_handle != NULL)
    {
        vTaskDelete(_handle);
        _handle = NULL;
    }
}

TaskHandle_t RPTask::task()
{
    return _handle;
}

bool RPTask::init(const char * name,
                  UBaseType_t priority,
                  const configSTACK_DEPTH_TYPE stackDepth)
{

    auto res = xTaskCreate(
        RPTask::handler,
       name,
        stackDepth,
        (void *)this,
        priority,
        &_handle);
    return (res == pdPASS);
}

void RPTask::handler(void *pvParameters)
{
    RPTask *task = (RPTask *)pvParameters;
    if (task != NULL)
    {
        task->loop();
    }
}
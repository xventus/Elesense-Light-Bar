//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   rptimer.cpp
/// @author Petr Vanek

#include "rptimer.h"

RPTimer::RPTimer() {
    ;
}

RPTimer::~RPTimer()
{
    done();
}

void RPTimer::done()
{
    if (_handle != NULL)
    {
        xTimerDelete(_handle, portMAX_DELAY);
        _handle = NULL;
    }
}


bool RPTimer::init(const char * name,
                  TickType_t tick,
                  bool periodic)
{

    _handle = xTimerCreate( name,
                            tick,
                            periodic ? pdTRUE : pdFALSE,
                            this,
                            handler);

    return (_handle != NULL);
}




void RPTimer::handler(TimerHandle_t xTimer)
{
    RPTimer *timer = static_cast<RPTimer *>(pvTimerGetTimerID(xTimer));
    timer->loop();
}


bool RPTimer::isActive()
{
    return xTimerIsTimerActive(_handle) == pdFALSE ? false : true;
}


bool RPTimer::start(TickType_t timout)
{
    return xTimerStart(_handle, timout) == pdFALSE ? false : true;
}


bool RPTimer::stop(TickType_t timout)
{
    return xTimerStop(_handle, timout) == pdFALSE ? false : true;
}


bool RPTimer::reset(TickType_t timout)
{
    return xTimerReset(_handle, timout) == pdFALSE ? false : true;
}


bool RPTimer::changePeriod(TickType_t period, TickType_t timout)
{
    return xTimerChangePeriod(_handle, period, timout) == pdFALSE
            ? false : true;
}


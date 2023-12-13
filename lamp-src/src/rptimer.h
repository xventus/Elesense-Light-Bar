//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   rptimer.h
/// @author Petr Vanek

#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"


class RPTimer {

  
    public:
  
        RPTimer();

        virtual ~RPTimer();

        virtual bool init(const char * name, TickType_t tick, bool periodic);

        void done();

               bool isActive();

        bool start(TickType_t timout);

        bool stop(TickType_t timout);

        bool reset(TickType_t timout);

        bool changePeriod(TickType_t period, TickType_t timout);      

    private:
                
        static void handler(TimerHandle_t xTimer);
	    virtual void loop() = 0;

        TimerHandle_t _handle = NULL;
	   
};


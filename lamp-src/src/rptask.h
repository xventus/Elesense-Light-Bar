//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   rptask.h
/// @author Petr Vanek

#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

class RPTask
{
public:
	RPTask();
	virtual ~RPTask();
	virtual bool init(const char * name, UBaseType_t priority = tskIDLE_PRIORITY, const configSTACK_DEPTH_TYPE stackDepth = configMINIMAL_STACK_SIZE);
	virtual void done();
	virtual TaskHandle_t task();

protected:
	static void handler(void *pvParameters);
	virtual void loop() = 0;

private:
	TaskHandle_t _handle = NULL;
};

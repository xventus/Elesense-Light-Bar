//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   led_task.h
/// @author Petr Vanek

#pragma once

#include "hardware.h"
#include "rptask.h"

enum class BlinkMode {
    AP_MODE,
    CLIENT,
    ERROR,
	LEARN, 
	NONE
};

class LedTask : public RPTask
{
public:
	LedTask(gpio_num_t pin = HEART_BEAT_LED);
	virtual ~LedTask();
	void mode(BlinkMode mode);

protected:
	void loop() override;

private:
	const uint32_t  _defaultTick{1000};
	gpio_num_t 		_pin{GPIO_NUM_0};
	QueueHandle_t 	_queue;
};

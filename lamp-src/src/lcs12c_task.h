//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   lcs12c_task.h
/// @author Petr Vanek

#pragma once

#include "hardware.h"
#include "rptask.h"

class LC12STask : public RPTask
{
public:
	 enum class Command {
		learn,
	    on,     
        off, 	
        hueintensity,
		toggle,
		incIntensity,
		decIntensity,
		};

	LC12STask();
	virtual ~LC12STask();
	void  command(LC12STask::Command cmd);
	void  hue(uint8_t hue);
	void  intensity(uint8_t intensity);

protected:
	void loop() override;

private:
	const uint32_t  _defaultTick{1000};
	gpio_num_t 		_pin{GPIO_NUM_0};
	QueueHandle_t 	_queue;
};

//
// vim: ts=4 et
// Copyright (c) 2024 Petr Vanek, petr@fotoventus.cz
//
/// @file   button_task.h
/// @author Petr Vanek

#pragma once

#include "hardware.h"
#include "rptask.h"
#include "button.h"

class ButtonTask : public RPTask
{
public:
	ButtonTask(gpio_num_t buttonA, gpio_num_t buttonB, gpio_num_t buttonX);
	virtual ~ButtonTask();
	
protected:
	void loop() override;

private:
	Button _ba;  ///< A - button
	Button _bb;  ///< B - button
	Button _b;	 ///< X-BOOT button
};

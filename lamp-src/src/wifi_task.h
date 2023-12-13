//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   web_task.h
/// @author Petr Vanek

#pragma once

#include "hardware.h"
#include "rptask.h"
#include "access_point.h"
#include "wifi_client.h"
#include "literals.h"


class WifiTask : public RPTask
{
public:
 enum class Mode {
	    AP,     // Access Point 
        Client, // STA
        Stop    // Stopped
    };

	WifiTask();
	virtual ~WifiTask();
	void switchMode(Mode mode);

protected:
	void loop() override;

private:
	Mode            _mode {Mode::Stop};
	QueueHandle_t 	_queue;
};

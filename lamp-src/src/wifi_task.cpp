
//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   wifi_task.cpp
/// @author Petr Vanek

#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>

#include "wifi_task.h"
#include "key_val.h"
#include "literals.h"
#include "application.h"
#include "wifi_scanner.h"

WifiTask::WifiTask()
{
	_queue = xQueueCreate(2, sizeof(int));
}

WifiTask::~WifiTask()
{
	done();
	if (_queue)
		vQueueDelete(_queue);
}

void WifiTask::loop()
{

	WiFiAccessPoint wftt;
	WiFiClient wfcli;
	esp_netif_ip_info_t staticip{0};
	bool processit = true;
	int receivedMode = static_cast<int>(Mode::Stop);

	while (true)
	{ // Loop forever

		auto res = xQueueReceive(_queue, (void *)&receivedMode, 0);
		if (res == pdTRUE)
		{
			processit = true;
		}

		if (processit)
		{
			processit = false;
			Mode mode = static_cast<Mode>(receivedMode);

			// Stop mode & check initial configuration
			if (mode == Mode::Stop)
			{

				// stop all previous wifi modes
				wftt.stop();
				if (wfcli.isConnected())
				{
					wfcli.disconnect();
				}

				// valid configuration?
				KeyVal &kv = KeyVal::getInstance();

				const auto &apname = kv.readString(literals::kv_ssid);
				if (apname.empty())
				{
					// no valid configuration & switch to AP
					receivedMode = static_cast<int>(Mode::AP);
					processit = true;
					Application::getInstance()->getLEDTask()->mode(BlinkMode::AP_MODE);
				}
				else
				{
					// switch to Client
					receivedMode = static_cast<int>(Mode::Client);
					processit = true;
					Application::getInstance()->getLEDTask()->mode(BlinkMode::CLIENT);
				}
			}
			else if (mode == Mode::Client)
			{

				wftt.stop();
				if (wfcli.isConnected())
				{
					wfcli.disconnect();
				}

				wfcli.init(false);

				KeyVal &kv = KeyVal::getInstance();
				esp_netif_str_to_ip4(kv.readString(literals::kv_ip).c_str(), &staticip.ip);
				esp_netif_str_to_ip4(kv.readString(literals::kv_mask).c_str(), &staticip.netmask);
				esp_netif_str_to_ip4(kv.readString(literals::kv_gtw).c_str(), &staticip.gw);

				bool cntok = false;
				if (staticip.ip.addr == 0 || staticip.netmask.addr == 0)
				{
					// DHCP mode
					cntok = wfcli.connect(kv.readString(literals::kv_ssid), kv.readString(literals::kv_passwd), true, nullptr);
				}
				else
				{
					// static IP mode
					cntok = wfcli.connect(kv.readString(literals::kv_ssid), kv.readString(literals::kv_passwd), false, &staticip);
				}

				if (!cntok)
					Application::getInstance()->getLEDTask()->mode(BlinkMode::ERROR);

				// startup web server for client
				Application::getInstance()->getWebTask()->command(WebTask::Mode::Control);
			}
			else if (mode == Mode::AP)
			{
				if (wfcli.isConnected())
				{
					wfcli.disconnect();
				}
				// scan APs for info
				WiFiScanner scanner;
				scanner.init(false);
				scanner.scan();

				// clear web task AP info
				Application::getInstance()->getWebTask()->command(WebTask::Mode::ClearAPInfo);
				ScanResultCallback callback = [](const APInfo &info)
				{
					// send each AP into WebTask
					Application::getInstance()->getWebTask()->apInfo(info);
				};
				scanner.processResults(callback);
				scanner.down();

				// AP start
				wftt.start(literals::ap_name, literals::ap_passwd);
				Application::getInstance()->getWebTask()->command(WebTask::Mode::Setting);
			}
		}

		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}

void WifiTask::switchMode(Mode mode)
{
	if (_queue)
	{
		int modeToSend = static_cast<int>(mode);
		xQueueSendToBack(_queue, (void *)&modeToSend, 0);
	}
}
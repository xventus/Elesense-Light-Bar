
//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   web_task.cpp
/// @author Petr Vanek

#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>

#include "application.h"
#include "web_task.h"
#include "http_server.h"
#include "key_val.h"
#include "content_file.h"
#include "http_request.h"
#include "packet.h"
#include <cJSON.h>

WebTask::WebTask()
{
	_queue = xQueueCreate(2, sizeof(int));
	_queueAP = xQueueCreate(2, sizeof(APInfo));
	_queueLcs = xQueueCreate(2, sizeof(LCSInfo));
}

WebTask::~WebTask()
{
	done();
	if (_queue)
		vQueueDelete(_queue);
	if (_queueAP)
		vQueueDelete(_queueAP);
	if (_queueLcs) 
	   vQueueDelete(_queueLcs);
}

void WebTask::loop()
{
	APInfo apinf{0};
	HttpServer server;
	std::string apinfo;

	LCSInfo lcs {
		.hue = 0,
		.intensity = 0,
		.command = static_cast<uint8_t>(lamp::Packet::Command::Unknown),
		.id = {'\0'} 
	};


	while (true)
	{ // Loop forever
		
		// AP info update - used only in registration
		auto resap = xQueueReceive(_queueAP, (void *)&apinf, 0);
		if (resap == pdTRUE)
		{  
			apinfo += "<li><pre>";
			apinfo += apinf.ap_name;
			apinfo += "  RSSI: ";
			apinfo += std::to_string(apinf.rssi);
			apinfo += "</pre></li>";
		}
		
		// form LCS update
		resap = xQueueReceive(_queueLcs, (void *)&lcs, 0);
		if (resap == pdTRUE) 
		{
           //todo...
		}

		int receivedMode;
		auto res = xQueueReceive(_queue, (void *)&receivedMode, 0);
		if (res == pdTRUE)
		{
			Mode mode = static_cast<Mode>(receivedMode);
			if (mode == Mode::Unknown)
			{
				server.stop();
			}
			else if (mode == Mode::Control)
			{
				server.stop();
				server.start();
				server.registerUriHandler("/", HTTP_GET, [&apinfo](httpd_req_t *req) -> esp_err_t
										  {
					ConentFile ap1cntb(literals::kv_fl_index);
				
    				auto cnt =  ap1cntb.readContnet();
 					httpd_resp_send(req, cnt.c_str(), cnt.length());
                    return ESP_OK; });

				// get info - slieders & status - repeated query about slider position
				server.registerUriHandler("/values", HTTP_GET, [&apinfo, &lcs](httpd_req_t *req) -> esp_err_t
					 {

						cJSON *root = cJSON_CreateObject();
						if (root) 
						{
							if (lcs.command == static_cast<uint8_t>(lamp::Packet::Command::Unknown)) {
								// unknown values
								cJSON_AddNumberToObject(root, "brightness", 0); 
								cJSON_AddNumberToObject(root, "hue", 0);
								cJSON_AddStringToObject(root, "id", "???");  
							} else if (lcs.command == static_cast<uint8_t>(lamp::Packet::Command::Startup)) {
								// TODO: known ID but unknown intensity & hue
								cJSON_AddNumberToObject(root, "brightness", 0); 
								cJSON_AddNumberToObject(root, "hue", 0);  
								cJSON_AddStringToObject(root, "id", lcs.id);   
							} else {
								cJSON_AddNumberToObject(root, "brightness", lcs.intensity); 
								cJSON_AddNumberToObject(root, "hue", lcs.hue);  
								cJSON_AddStringToObject(root, "id", lcs.id);  
							}
							
							httpd_resp_set_type(req, "application/json");

							char *json_string = cJSON_Print(root);
							if (json_string != nullptr) {
							httpd_resp_send(req, json_string, strlen(json_string));
							}

							cJSON_Delete(root);
							if (json_string) free(json_string);
						}
								
                    return ESP_OK; });

				server.registerUriHandler("/style.css", HTTP_GET, [](httpd_req_t *req) -> esp_err_t
										  {
					 httpd_resp_set_type(req, "text/css");
					 ConentFile ap1cnt(literals::kv_fl_style);
    				 auto contnet = ap1cnt.readContnet();
                    httpd_resp_send(req, contnet.c_str(), contnet.length());
                    return ESP_OK; });

				// Slider movement - send to LCS
				server.registerUriHandler("/slider", HTTP_POST, [&lcs](httpd_req_t *req) -> esp_err_t {
					char content[300] = {0}; 
					
					int received = httpd_req_recv(req, content, sizeof(content) - 1);
					if (received <= 0) { 
						if (received == HTTPD_SOCK_ERR_TIMEOUT) {
							httpd_resp_send_408(req);
						}
						return ESP_FAIL;
					}

					// processign JSON from page
					cJSON *json = cJSON_Parse(content);

					if (json == nullptr) {
						// invalid JSON 
						// TODO
					} else {
						//values
						// {slider: "brightness", value: "28"}
						// {slider: "hue", value: "28"}
						cJSON *slider = cJSON_GetObjectItem(json, "slider");
						cJSON *valueItem = cJSON_GetObjectItem(json, "value");
						if (slider && valueItem && cJSON_IsString(slider)&& cJSON_IsString(valueItem)) {
							uint8_t value = (uint8_t)atoi(valueItem->valuestring);

							if (strcmp(slider->valuestring, "brightness") == 0 && cJSON_IsString(valueItem)) {	
								lcs.intensity = value;
								Application::getInstance()->getLcsTask()->intensity(value);
							} else if (strcmp(slider->valuestring, "hue") == 0) {
								lcs.hue = value;
								Application::getInstance()->getLcsTask()->hue(value);
							}
						}
						
						cJSON_Delete(json);
					}

					httpd_resp_send(req, "", 0);

					return ESP_OK; 
				});

				// commands - send to LCS
				server.registerUriHandler("/command", HTTP_POST, [&lcs](httpd_req_t *req) -> esp_err_t {
					char content[300] = {0}; 
					int received = httpd_req_recv(req, content, sizeof(content) - 1);
					if (received <= 0) { 
						if (received == HTTPD_SOCK_ERR_TIMEOUT) {
							httpd_resp_send_408(req);
						}
						return ESP_FAIL;
					}

					// processign JSON from page
					cJSON *json = cJSON_Parse(content);

					if (json == nullptr) {
						// invalid JSON 
						// TODO
					} else {
						//values
						// {command: "ON", brightness: "20", hue: "50"}
						// {command: "OFF", brightness: "20", hue: "50"}
						// {command: "RECONFIG", brightness: "20", hue: "50"}
						cJSON *command = cJSON_GetObjectItem(json, "command");
						if (command) {
							if (strcmp(command->valuestring, "ON") == 0) {
								Application::getInstance()->getLcsTask()->command(LC12STask::Command::on);
							} else if (strcmp(command->valuestring, "OFF") == 0) {
								Application::getInstance()->getLcsTask()->command(LC12STask::Command::off);
							} if (strcmp(command->valuestring, "RECONFIG") == 0) {
								Application::getInstance()->getLcsTask()->command(LC12STask::Command::learn);
								lcs.command = static_cast<uint8_t>(lamp::Packet::Command::Unknown);
							}
						}
						cJSON_Delete(json);
					}

					httpd_resp_send(req, "", 0);
					return ESP_OK; 
				});

				
			}
			else if (mode == Mode::Setting)
			{
				server.stop();
				server.start();

				// AP main page
				server.registerUriHandler("/", HTTP_GET, [&apinfo](httpd_req_t *req) -> esp_err_t
										  {
					ConentFile ap1cntb(literals::kv_fl_apb);
					ConentFile ap1cnte(literals::kv_fl_ape);
    				 auto contnetb = ap1cntb.readContnet();
                    contnetb += apinfo;
					contnetb += ap1cnte.readContnet();
 					httpd_resp_send(req, contnetb.c_str(), contnetb.length());
                    return ESP_OK; });

				server.registerUriHandler("/style.css", HTTP_GET, [](httpd_req_t *req) -> esp_err_t
										  {
					 httpd_resp_set_type(req, "text/css");
					 ConentFile ap1cnt(literals::kv_fl_style);
    				 auto contnet = ap1cnt.readContnet();
                    httpd_resp_send(req, contnet.c_str(), contnet.length());
                   
                    return ESP_OK; });

				// AP setting answer
				server.registerUriHandler("/", HTTP_POST, [](httpd_req_t *req) -> esp_err_t {
					char content[300] = {0}; 
					int received = httpd_req_recv(req, content, sizeof(content) - 1);
					if (received <= 0) { 
						if (received == HTTPD_SOCK_ERR_TIMEOUT) {
							httpd_resp_send_408(req);
						}
						return ESP_FAIL;
					}

					// parse response & store configuration
					std::map<std::string, std::string> formData = HttpReqest::parseFormData(std::string(content));
					KeyVal& kv = KeyVal::getInstance();
					
					kv.writeString(literals::kv_ssid, HttpReqest::getValue(formData, literals::kv_ssid).c_str());
					kv.writeString(literals::kv_passwd, HttpReqest::getValue(formData, literals::kv_passwd).c_str());
					kv.writeString(literals::kv_ip, HttpReqest::getValue(formData, literals::kv_ip).c_str());
					kv.writeString(literals::kv_gtw, HttpReqest::getValue(formData, literals::kv_gtw).c_str());
					kv.writeString(literals::kv_mask, HttpReqest::getValue(formData, literals::kv_mask).c_str());
					
					
					// switch to Stop mode & check configuration
					Application::getInstance()->getWifiTask()->switchMode(WifiTask::Mode::Stop);

					// Response & swith mode 
					ConentFile respContnetDialog(literals::kv_fl_finish);
    				auto finish = respContnetDialog.readContnet();
					httpd_resp_send(req, finish.c_str(), finish.length());

					return ESP_OK; 
				});
			}
		}

		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}

void WebTask::apInfo(const APInfo &ap)
{
	if (_queueAP)
	{
		xQueueSendToBack(_queueAP, (void *)&ap, 0);
	}
}

void WebTask::command(Mode mode)
{
	if (_queue)
	{
		int modeToSend = static_cast<int>(mode);
		xQueueSendToBack(_queue, (void *)&modeToSend, 0);
	}
}


void  WebTask::lcsUpdate(const LCSInfo& lcs) 
{
	if (_queueLcs)
	{
		xQueueSendToBack(_queueLcs, (void *)&lcs, 0);
	}
}
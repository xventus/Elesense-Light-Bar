
//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   lcs12c_task.cpp
/// @author Petr Vanek

#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include "lcs12c_task.h"
#include "driver/uart.h"
#include "parser.h"
#include "hardware.h"
#include "application.h"
#include "lcs_info.h"
#include "key_val.h"



LC12STask::LC12STask() { 
	_queue = xQueueCreate( 10 , sizeof(LCSInfo));
}

LC12STask::~LC12STask() {
	done();
	if (_queue) vQueueDelete(_queue);
}


void LC12STask::loop(){
	
	lamp::PacketParser prs; ///< lamp packet sniffer
	uint8_t data[100];		///< serial buffer for LCS
	int length = 0; 	   /// TODO
	int readcnt = 0; 	  ///< bytes from UART
	bool learn = false;   ///< lamp ID must be learned
	lamp::Packet mylamp;  ///< all LCS operation over this lamp
	
	// minimal content
	mylamp.prepare();
	
	// check if valid ID exists
	KeyVal& kv = KeyVal::getInstance();
	const auto& strId = kv.readString(literals::kv_lampid);

	if (!strId.empty()) {
		mylamp.setIdentification(strId);
		LCSInfo lcs {
		.hue = 0,
		.intensity = 0,
		.command = static_cast<uint8_t>(lamp::Packet::Command::Startup),
		.id = {'\0'} };
		strcpy(lcs.id, strId.c_str());
		Application::getInstance()->getWebTask()->lcsUpdate(lcs);
	} else {
		learn = true;
		vTaskDelay(500 / portTICK_PERIOD_MS); 
		Application::getInstance()->getLEDTask()->mode(BlinkMode::LEARN);
	}
	

	while (true) {

		if (uart_get_buffered_data_len(LCS_UART, (size_t*)&length) == ESP_OK) {
			// TODO !!!
			readcnt = uart_read_bytes(LCS_UART, data, length, 20 / portTICK_PERIOD_MS);			
			for (int i = 0; i < readcnt; ++i) {
				if (prs.parseByte(data[i])) {
					const auto& packet = prs.getPacket();
					if (packet.validateChecksum() && !packet.canIgnoreMagic()) {
						LCSInfo lcs;	
						const auto& viewID =  packet.getIdentification();
						auto const &idstr = lamp::Packet::arrayToString(viewID);
						strcpy(lcs.id, idstr.c_str());
						lcs.hue = packet.getYellow2White();
						lcs.intensity = packet.getIntensity();
						lcs.command = static_cast<int>(packet.getCommnad());
						
						// learn mode stores my lamp ID into KV
						if (learn) {
							mylamp.setIdentification(viewID);
							KeyVal& kv = KeyVal::getInstance();
							kv.writeString(literals::kv_lampid, idstr);
							learn = false;
							Application::getInstance()->getLEDTask()->mode(BlinkMode::CLIENT);
						}
						
						// values from remote controller for my lamp & update web pages & local copy udate
						if (mylamp.getIdentification() == packet.getIdentification())
						{   
						    // update web interface
						    Application::getInstance()->getWebTask()->lcsUpdate(lcs);
							// sync hue & intensity
							mylamp.setIntensity(packet.getIntensity());
							mylamp.setYellow2White(packet.getYellow2White());
						}
												 
					} else {
						// error of simultaneous transmission of several transmitters or insufficient receive buffer
					}
				}
			}
		}
		
		
		// LCS info & command from web interface
		LCSInfo req;
		auto res = xQueueReceive(_queue, (void *)&req, 0);
		if (res == pdTRUE) { 
			Command cmd = static_cast<Command>(req.command);
			if (cmd == LC12STask::Command::off) {
				mylamp.setCommand(lamp::Packet::Command::Off);
			} else if (cmd == LC12STask::Command::on) {
				mylamp.setCommand(lamp::Packet::Command::On);
			} else if (cmd == LC12STask::Command::learn) {
				learn = true;
				kv.writeString(literals::kv_lampid, "");
				Application::getInstance()->getLEDTask()->mode(BlinkMode::LEARN);
			} else if (cmd == LC12STask::Command::hueintensity) {
				if (req.hue != 255) {
					mylamp.setYellow2White(req.hue);
				}

				if (req.intensity != 255) {
					 mylamp.setIntensity(req.intensity);
				}

				
				if (mylamp.getCommnad() !=  lamp::Packet::Command::On) {
					// switch on if in OFF mode or automatic
					mylamp.setCommand(lamp::Packet::Command::On);
				}
			} 
		
			if (!learn) {
				// send to LCS lamp
				mylamp.computeChecksum();				
				uart_write_bytes(UART_NUM_1, reinterpret_cast<const char*>(mylamp.getContnet().data()), mylamp.getContnet().size());
			} 
				
			
		
		}

		vTaskDelay(1 / portTICK_PERIOD_MS); 
	}
	
 }


void  LC12STask::command(LC12STask::Command cmd)
{
	if (_queue)
	{
		LCSInfo l;
		l.hue = 0;
		l.intensity = 0;
		l.command = static_cast<int>(cmd);
		xQueueSendToBack(_queue, (void *)&l, 0);
	}
}

void  LC12STask::hue(uint8_t hue)
{
	if (_queue)
	{
		LCSInfo l;
		l.hue = hue;
		l.intensity = 255;
		l.command = static_cast<int>(LC12STask::Command::hueintensity);
		xQueueSendToBack(_queue, (void *)&l, 0);
	}
}

void  LC12STask::intensity(uint8_t intensity)
{
	if (_queue)
	{
		LCSInfo l;
		l.hue = 255;
		l.intensity = intensity;
		l.command = static_cast<int>(LC12STask::Command::hueintensity);
		xQueueSendToBack(_queue, (void *)&l, 0);
	}
}


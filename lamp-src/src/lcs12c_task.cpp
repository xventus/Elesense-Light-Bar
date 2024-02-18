
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
	int length = 0; 	   	/// TODO
	int readcnt = 0; 	  	///< bytes from UART
	bool learn = false;   	///< lamp ID must be learned
	lamp::Packet mylamp;  	///< all LCS operation over this lamp
	bool lampIsOn = false;  ///< for toggle switch
	uint8_t hue = 0;		///< hue value
	uint8_t intensity = 0;	///< intensity value
	std::string strId;		///< LAMP ID

	const uint8_t maxIntensity = 0x17;
	const uint8_t minIntensity = 0x00;
	const uint8_t defHue = 0x00;
	
	auto updateWeb = [](const std::string& idstr, uint8_t hue, uint8_t intensity, uint8_t command ) {
		LCSInfo lcs;                        
		strcpy(lcs.id, idstr.c_str()); // Ensure lcs.id is large enough to hold idstr
		lcs.hue = hue;
		lcs.intensity = intensity;
		lcs.command = command;
		Application::getInstance()->getWebTask()->lcsUpdate(lcs);
	};

	// minimal content
	mylamp.prepare();
	
	// check if valid ID exists
	KeyVal& kv = KeyVal::getInstance();
	strId = kv.readString(literals::kv_lampid);
	
	if (!strId.empty()) {
		// update web interface with last known value
		mylamp.setIdentification(strId);
		
		// last known value of hue & intensity
		hue = static_cast<uint8_t>(kv.readUint32(literals::kv_lamhue, defHue));
		intensity = static_cast<uint8_t>(kv.readUint32(literals::kv_lampintnesity, minIntensity));
		mylamp.setIntensity(intensity);
		mylamp.setYellow2White(hue);

		updateWeb(strId, hue, intensity, static_cast<uint8_t>(lamp::Packet::Command::Startup));
		
	} else {
		// switch to learn mode
		learn = true;
		vTaskDelay(500 / portTICK_PERIOD_MS); 
		Application::getInstance()->getLEDTask()->mode(BlinkMode::LEARN);
	}


	// LCS loop
	while (true) {
		// read packet from some LCS UART
		if (uart_get_buffered_data_len(LCS_UART, (size_t*)&length) == ESP_OK) {
			readcnt = uart_read_bytes(LCS_UART, data, length, 20 / portTICK_PERIOD_MS);			
			for (int i = 0; i < readcnt; ++i) {
				if (prs.parseByte(data[i])) {
					const auto& packet = prs.getPacket();
					if (packet.validateChecksum() && !packet.canIgnoreMagic()) {
						
						const auto& viewID =  packet.getIdentification();
						strId = lamp::Packet::arrayToString(viewID);

						// update local copy from remote
						hue = packet.getYellow2White();
						intensity = packet.getIntensity();
						
						// learn mode stores my lamp ID into KV
						if (learn) {
							mylamp.setIdentification(viewID);
							kv.writeString(literals::kv_lampid, strId);
							learn = false;
							Application::getInstance()->getLEDTask()->mode(BlinkMode::CLIENT);
						}
						
						// values from remote controller for my lamp & update web pages & local copy udate
						if (mylamp.getIdentification() == packet.getIdentification())
						{   
							if (packet.getCommnad() == lamp::Packet::Command::On) lampIsOn = true;
							if (packet.getCommnad() == lamp::Packet::Command::Off) lampIsOn = false;

						    updateWeb(strId, hue, intensity,static_cast<int>(packet.getCommnad()));

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
		
		
		// LCS info & command from web interface & hw button control 
		LCSInfo req;
		auto res = xQueueReceive(_queue, (void *)&req, 0);
		if (res == pdTRUE) { 
			
			Command cmd = static_cast<Command>(req.command);

 			if (cmd == LC12STask::Command::toggle) {
				// hardware button
				lampIsOn = !lampIsOn;
				printf("#####1  %d  %d  on=%d\n", intensity, hue, lampIsOn?1:0);

				if (lampIsOn) {
					mylamp.setIntensity(intensity);
					mylamp.setYellow2White(hue);
					mylamp.setCommand(lamp::Packet::Command::On);
					updateWeb(strId, hue, intensity,static_cast<int>(lamp::Packet::Command::On));
				} else {
					mylamp.setCommand(lamp::Packet::Command::Off);
					
					// Store last known config
					kv.writeUint32(literals::kv_lampintnesity, intensity);
					kv.writeUint32(literals::kv_lamhue, hue);
				}
			}
			if (cmd == LC12STask::Command::incIntensity) {
				// hardware button
				if (!lampIsOn) {
					lampIsOn = true;
					mylamp.setIntensity(intensity);
					mylamp.setYellow2White(hue);
					mylamp.setCommand(lamp::Packet::Command::On);
				}
				
				if (intensity < maxIntensity) intensity++;
				mylamp.setIntensity(intensity); 
				updateWeb(strId, hue, intensity,static_cast<int>(lamp::Packet::Command::On));
			}
			if (cmd == LC12STask::Command::decIntensity) {
				// hardware button
				if (!lampIsOn) {
					lampIsOn = true;
					mylamp.setIntensity(intensity);
					mylamp.setYellow2White(hue);
					mylamp.setCommand(lamp::Packet::Command::On);
				}

				if (intensity > minIntensity) intensity--;
				mylamp.setIntensity(intensity);
				updateWeb(strId, hue, intensity,static_cast<int>(lamp::Packet::Command::On));
			}
			else if (cmd == LC12STask::Command::off) {
				mylamp.setCommand(lamp::Packet::Command::Off);
				lampIsOn = false;
			} else if (cmd == LC12STask::Command::on) {
				mylamp.setCommand(lamp::Packet::Command::On);
				mylamp.setIntensity(intensity);
				mylamp.setYellow2White(hue);
				lampIsOn = true;
			} else if (cmd == LC12STask::Command::learn) {
				learn = true;
				kv.writeString(literals::kv_lampid, "");
				Application::getInstance()->getLEDTask()->mode(BlinkMode::LEARN);
			} else if (cmd == LC12STask::Command::hueintensity) {
				
				if (req.hue != 255) {
					mylamp.setYellow2White(req.hue);
					hue = req.hue;
					lampIsOn = true;
				}

				if (req.intensity != 255) {
					 mylamp.setIntensity(req.intensity);
					 intensity = req.intensity;
				}
			
				if (mylamp.getCommnad() !=  lamp::Packet::Command::On) {
					// switch ON if in OFF mode or automatic
					mylamp.setCommand(lamp::Packet::Command::On);
					mylamp.setIntensity(intensity);
					mylamp.setYellow2White(hue);
					lampIsOn = true;
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


//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   application.h
/// @author Petr Vanek

#pragma once

#include "freertos/FreeRTOS.h"
#include <memory>
#include <atomic>
#include <stdio.h>
#include "led_task.h"
#include "web_task.h"
#include "wifi_task.h"
#include "lcs12c_task.h"
#include "button_task.h"
#include "literals.h"

/**
 * @brief application class - singleton
 *
 */
class Application
{

public:
    
   
    /**
     * @brief Destroy the Application object
     *
     */
    virtual ~Application();

    /**
     * @brief application initialization
     *
     */
    void init();

    void run();

    /**
     * @brief application deinitialization, an error occurred
     *
     */
    void done();

    
    /**
     * @brief Get heart beat task access
     *
     * @return LedTask*
     */
    LedTask *getLEDTask() { return &_heartBeat; }
    WebTask *getWebTask() { return &_webTask;}
    WifiTask *getWifiTask() { return &_wifiTask;}
    LC12STask *getLcsTask() { return &_lcs12cTask;}
    
    /**
     * Singleton
    */
    Application *operator->();
    Application* operator->() const;
    static Application *getInstance();

private:
   
    /**
     * @brief Constructor
     *
     */
    Application();
    Application(const Application &) = delete;
    Application &operator=(const Application &) = delete;

    void checkRenewAP();


    LedTask     _heartBeat;         ///< led task instance
    WebTask     _webTask;          ///< web interface
    WifiTask    _wifiTask;         ///< wifi AP / client  
    LC12STask   _lcs12cTask;       ///< 2.4 GHz link 
    ButtonTask  _btnTask;          ///< button task X, A, B 
   
};

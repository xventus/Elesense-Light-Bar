
//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   main.cpp
/// @author Petr Vanek

// NOTE: !!!!  Edit  sdkconfig.esp32dev  end set new value --> CONFIG_HTTPD_MAX_REQ_HDR_LEN=1024  !!!!


#include <stdio.h>
#include "application.h"

extern "C" {
    void app_main(void);
}

void app_main() {

    Application::getInstance()->init();
    Application::getInstance()->run();

    return;   
}
//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   literals.h
/// @author Petr Vanek

#pragma once

#include <stdio.h>
#include <string_view>


class literals
{
public:

    // internal - do not modify
    
    // internal task name
    static constexpr const char *tsk_led{"LEDTSK"};
    static constexpr const char *tsk_web{"WEBTSK"};
    static constexpr const char *tsk_wifi{"WIFITSK"};
    static constexpr const char *tsk_lcs{"LCSTSK"};
    static constexpr const char *btn_lcs{"BTNTSK"};

    // AP definition
    static constexpr const char *ap_name{"LAMP AP"};
    static constexpr const char *ap_passwd{""};

    // KV & files
    static constexpr const char *kv_namespace{"lamp"};
    static constexpr const char *kv_ssid{"ssid"};
    static constexpr const char *kv_passwd{"pass"};
    static constexpr const char *kv_ip{"ip"};
    static constexpr const char *kv_gtw{"gw"};
    static constexpr const char *kv_mask{"mask"};
    static constexpr const char *kv_lampid{"lampid"};
    static constexpr const char *kv_lampintnesity{"lamintensity"};
    static constexpr const char *kv_lamhue{"lamphue"};

    // spiffs filenames
    static constexpr const char *kv_fl_index{"/spiffs/index.html"}; 
    static constexpr const char *kv_fl_apb{"/spiffs/ap_beg.html"};
    static constexpr const char *kv_fl_ape{"/spiffs/ap_end.html"}; 
    static constexpr const char *kv_fl_style{"/spiffs/style.css"}; 
    static constexpr const char *kv_fl_finish{"/spiffs/finish.html"};
    


};
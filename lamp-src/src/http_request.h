//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   http_request.h
/// @author Petr Vanek

#pragma once


#include "esp_http_server.h"
#include <string>
#include <cstring>

#include <string>
#include <map>
#include <sstream>
#include <iostream>


class HttpReqest
{
public:

static std::map<std::string, std::string> parseFormData(const std::string& data) {
    std::map<std::string, std::string> form_data;
    std::istringstream data_stream(data);
    std::string pair;

    while (std::getline(data_stream, pair, '&')) {
        std::string::size_type pos = pair.find('=');

        if (pos != std::string::npos) {
            std::string key = pair.substr(0, pos);
            std::string value = pair.substr(pos + 1);

            // Případné URL dekódování hodnoty zde
            // ...

            form_data[key] = value;
        }
    }

    return form_data;
}

std::string static getValue(const std::map<std::string, std::string>& map, const std::string& key) {
    auto it = map.find(key);
    if (it != map.end()) {
        return it->second; 
    } else {
        return ""; 
    }
}

};
//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   http_server.h
/// @author Petr Vanek
#pragma once

#include <esp_http_server.h>
#include <string>
#include <functional>
#include <memory>
#include <list>

class HttpServer {
public:
    HttpServer() : _server(nullptr) {
        _config = HTTPD_DEFAULT_CONFIG();
    }

    ~HttpServer() {
        stop();
    }

    using HttpHandlerFunc = std::function<esp_err_t(httpd_req_t *req)>;

    bool start() {
        if (_server != nullptr) {
            return false;
        }

        return (httpd_start(&_server, &_config) == ESP_OK);
    }

 bool registerUriHandler(const std::string& uri, httpd_method_t method, HttpHandlerFunc handler) {
        if (!_server) return false;

        _handlerList.push_back(std::make_shared<HttpHandlerFunc>(handler));
        auto& handlerWrapper = _handlerList.back();

        httpd_uri_t httpdUri = {
            .uri = uri.c_str(),
            .method = method,
            .handler = [](httpd_req_t *req) -> esp_err_t {
                auto& handlerFunction = *static_cast<std::shared_ptr<HttpHandlerFunc>*>(req->user_ctx);
                return handlerFunction->operator()(req);
            },
            .user_ctx = &handlerWrapper
        };

        esp_err_t result = httpd_register_uri_handler(_server, &httpdUri);
        if (result != ESP_OK) {
            _handlerList.pop_back();
            return false;
        }
        return true;
    }

    void stop() {
        if (_server != nullptr) {
            httpd_stop(_server);
            _server = nullptr;
            _handlerList.clear(); 
        }
    }


private:
    httpd_handle_t _server;
    httpd_config_t _config;
    std::list<std::shared_ptr<HttpHandlerFunc>> _handlerList; 
};

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

    void stop() {
        if (_server != nullptr) {
            httpd_stop(_server);
            _server = nullptr;
        }
    }

    bool registerUriHandler(const std::string& uri, httpd_method_t method, HttpHandlerFunc handler) {
        if (!_server) return false;

        auto handlerWrapper = std::make_shared<HttpHandlerFunc>(handler);

        httpd_uri_t httpdUri = {
            .uri = uri.c_str(),
            .method = method,
            .handler = [](httpd_req_t *req) -> esp_err_t {
                auto handler = *static_cast<std::shared_ptr<HttpHandlerFunc>*>(req->user_ctx);
                return (*handler)(req);
            },
            .user_ctx = new std::shared_ptr<HttpHandlerFunc>(handlerWrapper)
        };
        
        esp_err_t result = httpd_register_uri_handler(_server, &httpdUri);
        if (result != ESP_OK) {
            delete static_cast<std::shared_ptr<HttpHandlerFunc>*>(httpdUri.user_ctx);
            return false;
        }
        return true;
    }



private:
    httpd_handle_t _server;
    httpd_config_t _config;
};

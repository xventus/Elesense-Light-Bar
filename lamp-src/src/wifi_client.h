//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   web_client.h
/// @author Petr Vanek


#pragma once

#include <functional>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sys.h"

using WiFiConnectedCallback = std::function<void(const ip_event_got_ip_t &)>;
using WiFiDisconnectedCallback = std::function<void()>;

class WiFiClient
{
public:
    WiFiClient() : _isConnected(false), _espNetif(nullptr) {}

    /// @brief Initialize Wifi client
    /// @param coreInit true if  WiFiClient used as standalone and needs to initialize NVS, Netinf, EventLoop
    /// @return true if success
    bool init(bool coreInit = false)
    {
        esp_err_t ret;

        // initialize NVS
        if (coreInit)
        {
            ret = nvs_flash_init();
            if (ret != ESP_OK)
            {
                ESP_LOGE("WiFiClient", "nvs_flash_init failed: %s", esp_err_to_name(ret));
                return false;
            }

            // Inicializace ESP Netif
            esp_netif_init();

            // ESP event loop
            ret = esp_event_loop_create_default();
            if (ret != ESP_OK)
            {
                ESP_LOGE("WiFiClient", "esp_event_loop_create_default failed: %s", esp_err_to_name(ret));
                return false;
            }
        }

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ret = esp_wifi_init(&cfg);
        if (ret != ESP_OK)
        {
            ESP_LOGE("WiFiClient", "esp_wifi_init failed: %s", esp_err_to_name(ret));
            return false;
        }

        _espNetif = esp_netif_create_default_wifi_sta();
        if (!_espNetif)
        {
            ESP_LOGE("WiFiClient", "Failed to create default wifi STA");
            return false;
        }

        // callback
        esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &WiFiClient::eventHandler, this);
        esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &WiFiClient::eventHandler, this);

        return true;
    }

    bool connect(const std::string& ssid, const std::string& pass, bool use_dhcp = true, esp_netif_ip_info_t *static_ip = nullptr) {
        return connect(ssid.c_str(), pass.c_str(), use_dhcp, static_ip);
    }

    bool connect(const char *ssid, const char *pass, bool use_dhcp = true, esp_netif_ip_info_t *static_ip = nullptr)
    {
        // station mode
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

        // SSID nad password
        wifi_config_t wifi_config = {};
        strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
        strncpy((char *)wifi_config.sta.password, pass, sizeof(wifi_config.sta.password));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

        // sets DHCP or static IP
        if (!use_dhcp && static_ip != nullptr)
        {
            esp_netif_dhcpc_stop(_espNetif);
            esp_netif_set_ip_info(_espNetif, static_ip);
        }
        else
        {
            esp_netif_dhcpc_start(_espNetif);
        }

        // connect to AP
        esp_err_t ret = esp_wifi_start();
        if (ret != ESP_OK)
        {
            ESP_LOGE("WiFiClient", "esp_wifi_start failed: %s", esp_err_to_name(ret));
            return false;
        }

        ret = esp_wifi_connect();
        if (ret != ESP_OK)
        {
            ESP_LOGE("WiFiClient", "esp_wifi_connect failed: %s", esp_err_to_name(ret));
            return false;
        }

        _isConnected = true;
        return true;
    }

    bool disconnect()
    {
        if (!_isConnected)
        {
            return false;
        }

        esp_err_t ret = esp_wifi_disconnect();
        if (ret != ESP_OK)
        {
            ESP_LOGE("WiFiClient", "esp_wifi_disconnect failed: %s", esp_err_to_name(ret));
            return false;
        }

        ret = esp_wifi_stop();
        if (ret != ESP_OK)
        {
            ESP_LOGE("WiFiClient", "esp_wifi_stop failed: %s", esp_err_to_name(ret));
            return false;
        }

        _isConnected = false;
        return true;
    }

    bool isConnected() const
    {
        return _isConnected;
    }

    void onDisconnect()
    {
        _isConnected = false;
        ESP_LOGI("WiFiClient", "Wi-Fi disconnected");
        if (_disconnectedCallback)
        {
            _disconnectedCallback();
        }
    }

    void onGotIP(const ip_event_got_ip_t &event)
    {
        _isConnected = true;
        char ipStr[16];
        esp_ip4addr_ntoa(&event.ip_info.ip, ipStr, sizeof(ipStr));
        ESP_LOGI("WiFiClient", "Got IP: %s", ipStr);

        if (_connectedCallback)
        {
            _connectedCallback(event);
        }
    }

    void registerConnectedCallback(WiFiConnectedCallback callback)
    {
        _connectedCallback = callback;
    }

    void registerDisconnectedCallback(WiFiDisconnectedCallback callback)
    {
        _disconnectedCallback = callback;
    }

    static void eventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
    {
        WiFiClient *client = static_cast<WiFiClient *>(arg);
        if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
        {
            client->onDisconnect();
        }
        else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
        {
            client->onGotIP(*static_cast<ip_event_got_ip_t *>(event_data));
        }
    }

private:
    bool _isConnected;                                ///< connection state
    esp_netif_t *_espNetif;                           ///<
    WiFiConnectedCallback _connectedCallback{};       ///< connect callback
    WiFiDisconnectedCallback _disconnectedCallback{}; ///< disconnect callback
};

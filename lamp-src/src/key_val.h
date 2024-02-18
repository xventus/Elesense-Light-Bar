//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   key_val.h
/// @author Petr Vanek

#pragma once

#include <string>
#include "nvs_flash.h"
#include "nvs.h"
#include <mutex>


/// @brief Key Value NVS storage
class KeyVal {
public:
    static KeyVal& getInstance() {
        static KeyVal instance;
        return instance;
    }

    KeyVal(KeyVal const&) = delete;
    void operator=(KeyVal const&) = delete;

    bool init(const std::string &namespaceName, bool needInit = false, bool readOnly = false) {
        std::lock_guard<std::mutex> lock(_mutex);

        if (_isInitialized) {
            return true;
        }

        if (needInit) {
            esp_err_t ret = nvs_flash_init();
            if (ret != ESP_OK && ret != ESP_ERR_NVS_NO_FREE_PAGES) {
                return false;
            }
        }

        esp_err_t ret = nvs_open(namespaceName.c_str(), readOnly ? NVS_READONLY : NVS_READWRITE, &_nvsHandle);
        if (ret != ESP_OK) {
            return false;
        }

        _isInitialized = true;
        return true;
    }

    void done() {
        std::lock_guard<std::mutex> lock(_mutex);

        if (_isInitialized) {
            nvs_close(_nvsHandle);
            _isInitialized = false;
        }
    }

    /// @brief Write number aka uint32_t
    /// @param key Key name
    /// @param value content
    /// @return true - success
    bool writeUint32(const std::string &key, uint32_t value)
    { 
        std::lock_guard<std::mutex> lock(_mutex);
        esp_err_t err = nvs_set_u32(this->_nvsHandle, key.c_str(), value);
        if (err != ESP_OK) {
            return false;
        }
        err = nvs_commit(this->_nvsHandle); 
        return err == ESP_OK;
    }

    /// @brief Write string
    /// @param key Key name
    /// @param value contnet
    /// @return true - success
    bool writeString(const std::string &key, const std::string &value)
    {
        return writeString(key.c_str(), value.c_str());
    }

    bool writeString(const char* key, const char* value)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        esp_err_t err = nvs_set_str(this->_nvsHandle, key, value);
        if (err != ESP_OK) {
            return false;
        }
        err = nvs_commit(this->_nvsHandle); 
        return err == ESP_OK;
    }

    /// @brief Read uint32_t from NVS
    /// @param key Key name
    /// @param defvalue Default value if key does not exists or failed
    /// @return true - success
    uint32_t readUint32(const std::string &key, const uint32_t defvalue = 0) const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        uint32_t value = 0;
        if (nvs_get_u32(this->_nvsHandle, key.c_str(), &value) != ESP_OK)
            value = defvalue;
        return value;
    }

    /// @brief Read string from NVS
    /// @param key  Key name
    /// @param defvalue Default value if key does not exists or failed
    /// @return true - success
    std::string readString(const std::string &key, const std::string &defvalue = "") const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        std::string rc;
        size_t requiredSize = 0;
        if (nvs_get_str(this->_nvsHandle, key.c_str(), nullptr, &requiredSize) != ESP_OK)
        {
            rc = defvalue;
            return rc;
        }
        rc.resize(requiredSize);
        auto err = nvs_get_str(this->_nvsHandle, key.c_str(), &rc[0], &requiredSize);
        if (err != ESP_OK)
        {
            rc = defvalue;
        }
        if (rc.size()==1 && rc.at(0)=='\0') rc.clear();
        return rc;
    }

private:
    
    KeyVal() : _isInitialized(false) {}
    nvs_handle_t _nvsHandle;
    bool _isInitialized;
    mutable std::mutex _mutex;
};

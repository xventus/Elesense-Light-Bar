//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   contnet_file.h
/// @author Petr Vanek

#include <string>
#include "cJSON.h"
#include "esp_spiffs.h"
#include "esp_log.h"
#include <fstream>
#include <sstream>
#include "freertos/FreeRTOS.h"

/// @brief Store & read configuration data & preloaded content
/// based on SPIFFS system
class ConentFile
{
public:

    /// @brief CTOR
    /// @param filename name of config file or preloaded file 
    ConentFile(const std::string &filename) : _configFilename(filename) {}

    /// @brief Initialize file system SPIFFS
    /// @return true if success
    static bool initFS()
    {
        esp_vfs_spiffs_conf_t conf = {
            .base_path = "/spiffs",
            .partition_label = nullptr,
            .max_files = 5,
            .format_if_mount_failed = true};

        esp_err_t ret = esp_vfs_spiffs_register(&conf);
        if (ret != ESP_OK)
        {
            if (ret == ESP_FAIL)
            {
                ESP_LOGE("ConentFile", "Failed to mount or format filesystem");
            }
            else if (ret == ESP_ERR_NOT_FOUND)
            {
                ESP_LOGE("ConentFile", "Failed to find SPIFFS partition");
            }
            else
            {
                ESP_LOGE("ConentFile", "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
            }
            return false;
        }

        return true;
    }

    /// @brief save configuration key - value pair
    /// @param key - key name
    /// @param value -value content
    /// @return 
    bool saveJsonKeyVal(const std::string &key, const std::string &value)
    {
        std::ifstream fileIn(_configFilename);
        std::string data;
        if (fileIn)
        {
            std::stringstream buffer;
            buffer << fileIn.rdbuf();
            data = buffer.str();
            fileIn.close();
        }

        cJSON *root = data.empty() ? cJSON_CreateObject() : cJSON_Parse(data.c_str());
        if (!root)
        {
            ESP_LOGE("ConfigFile", "Failed to create or parse JSON object");
            return false;
        }

        cJSON_AddStringToObject(root, key.c_str(), value.c_str());

        char *rendered = cJSON_Print(root);
        if (!rendered)
        {
            ESP_LOGE("ConfigFile", "Failed to render JSON");
            cJSON_Delete(root);
            return false;
        }

        std::ofstream fileOut(_configFilename);
        if (!fileOut)
        {
            ESP_LOGE("ConfigFile", "Failed to open file for writing");
            free(rendered);
            cJSON_Delete(root);
            return false;
        }

        fileOut << rendered;
        fileOut.close();

        free(rendered);
        cJSON_Delete(root);
        return true;
    }

    /// @brief Gets configuration item 
    /// @param key - name of key
    /// @param value - value content
    /// @return true if success (item and key is exists)
    bool loadJsonKeyVal(const std::string &key, std::string &value)
    {
        std::ifstream file(_configFilename);
        if (!file)
        {
            ESP_LOGE("ConfigFile", "Failed to open file for reading");
            return false;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string data = buffer.str();
        file.close();

        cJSON *root = cJSON_Parse(data.c_str());
        if (root == NULL)
        {
            ESP_LOGE("ConfigFile", "Failed to parse JSON");
            return false;
        }

        cJSON *json_value = cJSON_GetObjectItem(root, key.c_str());
        if (json_value == NULL)
        {
            ESP_LOGE("ConfigFile", "Key not found in JSON");
            cJSON_Delete(root);
            return false;
        }

        value = json_value->valuestring;

        cJSON_Delete(root);
        return true;
    }

    /// @brief Delete config file or preloaded file
    /// @return 
    bool deleteContnet()
    {
        if (std::remove(_configFilename.c_str()) != 0)
        {
            ESP_LOGE("ConfigFile", "Failed to delete file");
            return false;
        }
        else
        {
            ESP_LOGI("ConfigFile", "File successfully deleted");
            return true;
        }
    }

    /// @brief Reads file content
    /// @return content or empty is file not found
    std::string readContnet() {
        std::ifstream file(_configFilename);
        if (!file) {
            ESP_LOGE("ConfigFile", "Failed to open file for reading: %s", _configFilename.c_str());
            return "";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        return buffer.str();
    }

    /// @brief Check if file exists
    /// @return true if exists
    bool fileExists() {
        std::ifstream file(_configFilename);
        return file.good();
    }

     /// @brief Store content
     /// @param content payload
     /// @return 
     bool saveContent(const std::string& content) {
        std::ofstream file(_configFilename);
        if (!file) {
            ESP_LOGE("ConfigFile", "Failed to open file for writing: %s", _configFilename.c_str());
            return false;
        }

        file << content;
        if (!file.good()) {
            ESP_LOGE("ConfigFile", "Failed write to file: %s", _configFilename.c_str());
            return false;
        }

        file.close();
        return true;
    }

private:
    std::string _configFilename; ///>  configuration filename
};

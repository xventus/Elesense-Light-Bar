/*
 * @file packet.h
 * @author Petr Vanek (petr@fotoventus.cz)
 * @brief Lamp packet control
 * @version 0.1
 * @date 2023-11-30
 *
 * @copyright Copyright (c) 2023 Petr Vanek
 */

#pragma once
#include <cstdint>
#include <array>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <queue>

namespace lamp {

/**
 * @brief Encapsulates the lamp communication packet
 *
 * Packet description byte position:
 * example:   53 C2 1C 00 9D 1B 00 0E 01 17 0E 26 00   ON
 *            53 C2 1C 00 9D 1B 00 0E 10 10 10 30 00   OFF
 * byte:
 * 0 -     HEAD 53
 * 1 - 6   IDentification  eg. XX XX 00 XX XX 00 XX
 * 7 -     YY - unknown, if  0xA4 occured the next octets are zeros 
 * 8       Data:  Command:  0x01 - ON,  0x10 - OFF, 0x02 - Automatic dimming
 * 9       Data:  0x00 - 0x17 intensity (0x00 - min intensity, 0x17 - max intensity), note: when off 0x10
 * 10      Data:  0x00 - 0x17 intensity (0x00 - Yellow MAX, 0x17 - Yellow Min), note: when off 0x10
 * 11      Checksum  = Sum of [8] + [9] + [10]
 * 12 -    END 00
 *
 */
class Packet
{

public:


    enum class Command : uint8_t
    {
        On = 0x01,        ///< on
        Off = 0x10,       ///< off
        Automatic = 0x02, ///< automatic
        Startup = 0xE0, ///< Startup
        Unknown = 0xFF    ///< unknown or default value
    };

    static constexpr uint8_t _header = 0x53;    ///< header 

    /// @brief CTOR
    Packet()
    {
        clear();
    }

    /// @brief required contnet
    void prepare() 
    {
        clear();
        setHead();
        setEnd();
    }

    /// @brief Clear contnet
    void clear()
    {
        std::fill(_data.begin(), _data.end(), 0);
    }

    /// @brief Set head 
    /// @param head identification
    void setHead(uint8_t head = _header)
    {
        _data[0] = head;
    }

    /// @brief Lamp identification (included magic)
    /// @param id - 7 bytes ID
    void setIdentification(const std::array<uint8_t, 7> &id)
    {
        std::copy(id.begin(), id.end(), _data.begin() + 1);
    }

    /// @brief Lamp identification from string
    /// @param hexId 
    void setIdentification(const std::string& hexId)
    {   
        const auto & hid = stringToID(hexId);
        std::copy(hid.begin(), hid.end(), _data.begin() + 1);
    }


    /// @brief Gets identification  
    /// @return IDentification  eg. XX XX 00 XX XX 00 XX 
    const std::array<uint8_t, 7> getIdentification() const
    {
        std::array<uint8_t, 7> subArray;
        std::copy(_data.begin() + 1, _data.begin() + 8, subArray.begin()); 
        return subArray;
    }

    /// @brief unknown byte at position 7
    /// @return 
    uint8_t getMagic() const 
    {
       return _data[7];     
    }

    bool canIgnoreMagic() const
    {
      return (_data[7] == 0xA4);
    }
    
    /// @brief Lamp identification
    /// @param queue ID eat from queue
    /// @return true if length is ok
    bool setIdentification(std::queue<uint8_t> &queue)
    {
        if (queue.size() < 7)
        {
            return false;
        }

        for (int i = 1; i <= 7; ++i)
        {
            _data[i] = queue.front();
            queue.pop();
        }

        return true;
    }

    /// @brief Sets data content eg. Command & intensity
    /// @param queue 3 bytes from queue
    /// @return true if length is ok
    bool setData(std::queue<uint8_t> &queue)
    {
        if (queue.size() < 3)
        {
            return false;
        }

        for (int i = 1; i <= 3; ++i)
        {
            _data[i + 7] = queue.front();
            queue.pop();
        }

        return true;
    }

    /// @brief Gets packet content
    /// @return 
    const std::array<uint8_t, 13> &getContnet() const
    {
        return _data;
    }

    /// @brief Set command 
    /// @param cmd command
    void setCommand(Command cmd)
    {
        _data[8] = static_cast<uint8_t>(cmd);
        if (cmd != Command::On) {
            setIntensity(0x10);
            setYellow2White(0x10);
        }
    }

    Command getCommnad() const 
    {
        switch (_data[8]) {
            case static_cast<uint8_t>(Command::On):
                return Command::On;
            case static_cast<uint8_t>(Command::Off):
                return Command::Off;
            case static_cast<uint8_t>(Command::Automatic):
                return Command::Automatic;
            default:
                return Command::Unknown;
        }
    }

    //9       Data:  0x00 - 0x17 intensity (0x00 - min intensity, 0x17 - max intensity)
    //10      Data:  0x00 - 0x17 intensity (0x00 - Yellow MAX, 0x17 - Yellow Min)

    /// @brief Sets intensity lamp
    /// @param intensity 0x00 - 0x17
    void setIntensity(uint8_t intensity)
    {
        _data[9] = intensity;
    }

    /// @brief Gets setting intensity
    /// @return value 
    uint8_t getIntensity() const
    {
        return _data[9];
    }

    /// @brief Set white against yeallow
    /// @param intensity 
    void setYellow2White(uint8_t intensity) 
    {
        _data[10] = intensity;
    }

    /// @brief Gets setting white 2 yellow
    /// @return value 
    uint8_t getYellow2White() const
    {
        return _data[10];
    }

    /// @brief Compute Checksum
    void updateChecksum()
    {
        uint8_t sum = 0;
        for (int i = 8; i < 11; ++i)
        {
            sum += _data[i];
        }
        _data[10] = sum;
    }

    /// @brief Sets end byte
    /// @param end 
    void setEnd(uint8_t end = 0x00)
    {
        _data[12] = end;
    }

    /// @brief Sets Checksum
    /// @param sum 
    void setChSum(uint8_t sum)
    {
        _data[11] = sum;
    }

    /// @brief Gets whole packet
    /// @return byte array
    const std::array<uint8_t, 13> &getPacket() const
    {
        return _data;
    }

    /// @brief Gets data contnet
    /// @return 
    const std::array<uint8_t, 3> getData() const
    {
        std::array<uint8_t, 3> subArray;
        std::copy(_data.begin() + 8, _data.begin() + 11, subArray.begin());
        return subArray;
    }

    /// @brief Check if packet contains valid checksum
    /// @return true if packet is OK
    bool validateChecksum() const
    {
        uint8_t sum = 0;
        for (int i = 8; i < 11; ++i)
        {
            sum += _data[i];
        }
        return _data[11] == sum;
    }

    // compute checksum 
    void computeChecksum()
    {
        uint8_t sum = 0;
        for (int i = 8; i < 11; ++i)
        {
            sum += _data[i];
        }
        _data[11] = sum;
    }

    /// @brief Gets end packet
    /// @return 
    uint8_t getEnd() const
    {
        return _data[12];
    }

    template <size_t N>
    static std::string arrayToString(const std::array<uint8_t, N>& arr) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (auto byte : arr) {
            ss << std::setw(2) << static_cast<unsigned>(byte);
        }
        return ss.str();
    }


    static std::array<uint8_t, 7> stringToID(const std::string& str) {
        std::array<uint8_t, 7> arr;
        size_t count = std::min(str.size() / 2, arr.size());

        for (size_t i = 0; i < count; ++i) {
            std::string byteString = str.substr(i * 2, 2);
            arr[i] = static_cast<uint8_t>(std::stoul(byteString, nullptr, 16));
        }

        return arr;
    }

private:
    std::array<uint8_t, 13> _data;  ///< content 
};

} // namespace lamp 
/*
 * @file parser.h
 * @author Petr Vanek (petr@fotoventus.cz)
 * @brief Lamp protocol parser
 * @version 0.1
 * @date 2023-12-01
 *
 * @copyright Copyright (c) 2023 Petr Vanek
 */

#pragma once
#include <cstdint>
#include <queue>
#include "packet.h"

namespace lamp {

/// @brief Packet parser
class PacketParser
{
public:
    
    /// @brief Ctor
    PacketParser() : _state(ParserState::WaitingForHead) {}

    /// @brief Parse input data from serial line
    /// @param b - one byte
    /// @return true - if packet completly received
    bool parseByte(uint8_t b)
    {
        switch (_state)
        {
        case ParserState::WaitingForHead:
            if (b == Packet::_header)
            {
                _currentPacket.setHead(b);
                _byteCount = 1;
                _state = ParserState::ReceivingId;
            }
            break;

        case ParserState::ReceivingId:

            _receiveBuffer.push(b);
            _byteCount++;
            if (_byteCount >= 8)
            {
                _currentPacket.setIdentification(_receiveBuffer);
                _state = ParserState::ReceivingData;
                _receiveBuffer = std::queue<uint8_t>();
            }
            break;

        case ParserState::ReceivingData:
            _receiveBuffer.push(b);
            _byteCount++;
            if (_byteCount >= 11)
            {
                _currentPacket.setData(_receiveBuffer);
                _state = ParserState::ReceivingSum;
                _receiveBuffer = std::queue<uint8_t>();
            }
            break;

        case ParserState::ReceivingSum:
            _currentPacket.setChSum(b);
            _byteCount++;
            _state = ParserState::ReceivingEnd;
            break;

        case ParserState::ReceivingEnd:
            _currentPacket.setEnd(b);
            if (_currentPacket.validateChecksum() && b == 0)
            {
                _state = ParserState::WaitingForHead;
                return true;
            }
            else
            {
                clear();
            }
            break;
        }
        return false;
    }

    /// @brief Get whole packet
    /// @return packet
    const Packet &getPacket() const
    {
        return _currentPacket;
    }

    /// @brief Go to initial state
    void clear()
    {
        std::queue<uint8_t>().swap(_receiveBuffer);
        _state = ParserState::WaitingForHead;
        _byteCount = 0;
        _currentPacket.clear();
    }

private:

    /// @brief state of parsing
    enum class ParserState
    {
        WaitingForHead,
        ReceivingId,
        ReceivingData,
        ReceivingSum,
        ReceivingEnd
    };


    ParserState _state{ParserState::WaitingForHead};    ///< parse state
    Packet _currentPacket;                              ///< packet content
    std::queue<uint8_t> _receiveBuffer;                 ///< aux buffer
    size_t _byteCount{0};                               ///< counter of receiced bytes
};

} // namespace lamp 
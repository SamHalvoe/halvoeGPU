#pragma once

#include <PicoDVI.h>
#include <elapsedMillis.h>
#include <array>

#include "SerialGFXInterface.hpp"

namespace halvoeGPU
{
  namespace atGPU
  {
    class SerialGFXInterface
    {
      private:
        enum class ReceiveState : uint8_t
        {
          null = 0,
          receiveCommand,
          receiveParameters
        };

      private:
        SerialUART& m_serial;
        DVIGFX8& m_dviGFX;

        SerialGFXCommandCode m_receivedCommandCode = SerialGFXCommandCode::noCommand;
        uint16_t m_parameterCount = 0;
        std::array<bool, 8> m_boolBuffer;
        std::array<int16_t, 16> m_intBuffer;
        std::array<uint16_t, 16> m_uintBuffer;
        std::array<String, 8> m_stringBuffer;
        ReceiveState m_receiveState = ReceiveState::receiveCommand;

      private:
        void dviGFX_swap()
        {
          m_dviGFX.swap();
        }

        void dviGFX_fillScreen()
        {
          dviGFX.fillScreen(0);
        }

      public:
        SerialGFXInterface(SerialUART& io_serial, DVIGFX8& io_dviGFX) :
          m_serial(io_serial), m_dviGFX(io_dviGFX)
        {}

        bool begin(unsigned long in_baud = 9600)
        {
          if (not m_dviGFX.begin()) // false if insufficient RAM
          {
            return false;
          }

          m_serial.begin(in_baud);
          elapsedMillis timeSinceBegin;

          while (not m_serial && timeSinceBegin < 10000)
          {}

          return m_serial;
        }
        
        int32_t receiveCommand()
        {
          if (m_receivedCommandCode != SerialGFXCommandCode::noCommand) { return -1; }

          static std::array<char, 8> buffer;

          if (m_serial.available() > 0)
          {
            buffer.fill(0);

            while (m_serial.available() > 0)
            {
              switch (m_receiveState)
              {
                case ReceiveState::receiveCommand:
                  size_t bytesReceived = m_serial.readBytes(buffer.data(), 2);

                  if (bytesReceived == 2)
                  {
                    uint16_t cmdElement = (static_cast<uint16_t>(buffer[0]) << 8) + static_cast<uint16_t>(buffer[1]);
                    
                    if (cmdElement == SerialGFXCommandElement::begin)
                    {
                      size_t bytesReceived = m_serial.readBytes(buffer.data(), 2);

                      if (bytesReceived == 2)
                      {
                        m_receivedCommandCode = toSerialGFXCommandCode((static_cast<uint16_t>(buffer[0]) << 8) + static_cast<uint16_t>(buffer[1]));

                        if (m_receivedCommandCode == SerialGFXCommandCode::invalid || m_receivedCommandCode == SerialGFXCommandCode::noCommand)
                        {
                          // handle error
                        }
                        else
                        {
                          size_t bytesReceived = m_serial.readBytes(buffer.data(), 2);

                          if (bytesReceived == 2)
                          {
                            m_parameterCount = (static_cast<uint16_t>(buffer[0]) << 8) + static_cast<uint16_t>(buffer[1]);
                            m_receiveState = ReceiveState::receiveParameters;
                          }
                        }
                      }
                    }
                  }
                break;

                case ReceiveState::receiveParameters:
                break;
              }
            }
          }
        }

        int32_t runCommand()
        {
          switch (m_receivedCommandCode)
          {
            case SerialGFXCommandCode::swap: dviGFX_swap(); break;
            case SerialGFXCommandCode::fillScreen: dviGFX_fillScreen(); break;
          }

          m_receivedCommandCode = SerialGFXCommandCode::null;
          return 0;
        }
    };
  }
}

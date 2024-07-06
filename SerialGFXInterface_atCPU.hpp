#pragma once

#include <elapsedMillis.h>
#include <array>
#include <cstring>

#include "SerialGFXInterface.hpp"

namespace halvoeGPU
{
  namespace atCPU
  {
    class SerialGFXInterface
    {
      private:
        HALVOE_SERIAL_TYPE& m_serial;
        uint16_t m_parameterBufferLength = 0;
        std::array<char, 2> m_commandBuffer;
        std::array<char, g_maxParameterBufferLength> m_parameterBuffer;

      private:
        bool sendCommand(SerialGFXCommandCode in_commandCode)
        {
          //m_serial.availableForWrite();
          *reinterpret_cast<uint16_t*>(m_commandBuffer.data()) = fromSerialGFXCommandCode(in_commandCode);
          size_t bytesWritten = m_serial.write(m_commandBuffer.data(), 2);
          Serial.print("m_serial.write 0: ");
          Serial.println(bytesWritten);
          if (bytesWritten != 2) { return false; }
          *reinterpret_cast<uint16_t*>(m_commandBuffer.data()) = m_parameterBufferLength;
          bytesWritten = m_serial.write(m_commandBuffer.data(), 2);
          Serial.print("m_serial.write 1: ");
          Serial.println(bytesWritten);
          if (bytesWritten != 2) { return false; }

          if (m_parameterBufferLength > 0)
          {
            bytesWritten = m_serial.write(m_parameterBuffer.data(), m_parameterBufferLength);
            Serial.print("m_serial.write 2: ");
            Serial.println(bytesWritten);
            if (bytesWritten != m_parameterBufferLength) { return false; }
          }

          return true;
        }

        void clearBuffer()
        {
          m_parameterBufferLength = 0;
        }

        bool addBoolToBuffer(bool in_value)
        {
          if (m_parameterBufferLength + 1 > m_parameterBuffer.size()) { return false; }

          std::memcpy(m_parameterBuffer.data() + m_parameterBufferLength, &in_value, 1);
          m_parameterBufferLength = m_parameterBufferLength + 1;

          return true;
        }

        bool addInt8ToBuffer(int8_t in_value)
        {
          if (m_parameterBufferLength + 1 > m_parameterBuffer.size()) { return false; }

          std::memcpy(m_parameterBuffer.data() + m_parameterBufferLength, &in_value, 1);
          m_parameterBufferLength = m_parameterBufferLength + 1;

          return true;
        }

        bool addUInt8ToBuffer(uint8_t in_value)
        {
          if (m_parameterBufferLength + 1 > m_parameterBuffer.size()) { return false; }

          std::memcpy(m_parameterBuffer.data() + m_parameterBufferLength, &in_value, 1);
          m_parameterBufferLength = m_parameterBufferLength + 1;

          return true;
        }

        bool addInt16ToBuffer(int16_t in_value)
        {
          if (m_parameterBufferLength + 2 > m_parameterBuffer.size()) { return false; }
          
          std::memcpy(m_parameterBuffer.data() + m_parameterBufferLength, &in_value, 2);
          m_parameterBufferLength = m_parameterBufferLength + 2;

          return true;
        }

        bool addUInt16ToBuffer(uint16_t in_value)
        {
          if (m_parameterBufferLength + 2 > m_parameterBuffer.size()) { return false; }
          
          std::memcpy(m_parameterBuffer.data() + m_parameterBufferLength, &in_value, 2);
          m_parameterBufferLength = m_parameterBufferLength + 2;

          return true;
        }

        bool addStringToBuffer(const String& in_string)
        {
          uint16_t stringLength = in_string.length();

          if (m_parameterBufferLength + 2 + stringLength > m_parameterBuffer.size()) { return false; }

          std::memcpy(m_parameterBuffer.data() + m_parameterBufferLength, &stringLength, 2);
          m_parameterBufferLength = m_parameterBufferLength + 2;
          in_string.toCharArray(m_parameterBuffer.data() + m_parameterBufferLength, stringLength);
          m_parameterBufferLength = m_parameterBufferLength + stringLength;

          return true;
        }

      public:
        SerialGFXInterface(HALVOE_SERIAL_TYPE& io_serial) :
          m_serial(io_serial)
        {}

        bool begin(unsigned long in_baud = 9600)
        {
          m_serial.begin(in_baud);
          elapsedMillis timeSinceBegin;

          while (not m_serial && timeSinceBegin < 10000)
          {}

          return m_serial;
        }

        bool sendSwap()
        {
          clearBuffer();

          return sendCommand(SerialGFXCommandCode::swap);
        }

        bool sendFillScreen()
        {
          clearBuffer();

          return sendCommand(SerialGFXCommandCode::fillScreen);
        }

        bool sendFillRect(int16_t in_x, int16_t in_y, int16_t in_width, int16_t in_height, uint16_t in_color)
        {
          clearBuffer();
          addInt16ToBuffer(in_x);
          addInt16ToBuffer(in_y);
          addInt16ToBuffer(in_width);
          addInt16ToBuffer(in_height);
          addUInt16ToBuffer(in_color);

          return sendCommand(SerialGFXCommandCode::fillRect);
        }
    };
  }
}

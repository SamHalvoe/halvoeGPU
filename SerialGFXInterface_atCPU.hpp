#pragma once

#include <elapsedMillis.h>
#include <array>

#include "SerialGFXInterface.hpp"

namespace halvoeGPU
{
  namespace atCPU
  {
    const uint8_t READY_PIN = 40;

    class SerialGFXInterface
    {
      private:
        HALVOE_SERIAL_TYPE& m_serial;
        uint16_t m_parameterBufferLength = 0;
        std::array<char, 2> m_commandBuffer;
        std::array<char, g_maxParameterBufferLength> m_parameterBuffer;
        uint16_t m_gpuCode;

      private:
        bool sendCommand(SerialGFXCommandCode in_commandCode)
        {
          *reinterpret_cast<uint16_t*>(m_commandBuffer.data()) = fromSerialGFXCommandCode(in_commandCode);
          size_t bytesWritten = m_serial.write(m_commandBuffer.data(), 2);
          if (bytesWritten != 2) { return false; }
          *reinterpret_cast<uint16_t*>(m_commandBuffer.data()) = m_parameterBufferLength;
          bytesWritten = m_serial.write(m_commandBuffer.data(), 2);
          if (bytesWritten != 2) { return false; }

          if (m_parameterBufferLength > 0)
          {
            bytesWritten = m_serial.write(m_parameterBuffer.data(), m_parameterBufferLength);
            if (bytesWritten != m_parameterBufferLength) { return false; }
          }

          return true;
        }

        void clearBuffer()
        {
          m_parameterBufferLength = 0;
        }

        template<typename ValueType>
        bool setValueInBufferAt(ValueType in_value, uint16_t in_position)
        {
          if (m_parameterBufferLength + sizeof(ValueType) > m_parameterBuffer.size()) { return false; }
          *reinterpret_cast<ValueType*>(m_parameterBuffer.data() + in_position) = in_value;
          return true;
        }

        bool addBoolToBuffer(bool in_value)
        {
          if (not setValueInBufferAt<bool>(in_value, m_parameterBufferLength)) { return false; }
          m_parameterBufferLength = m_parameterBufferLength + sizeof(bool);
          return true;
        }

        bool addInt8ToBuffer(int8_t in_value)
        {
          if (not setValueInBufferAt<int8_t>(in_value, m_parameterBufferLength)) { return false; }
          m_parameterBufferLength = m_parameterBufferLength + sizeof(int8_t);
          return true;
        }

        bool addUInt8ToBuffer(uint8_t in_value)
        {
          if (not setValueInBufferAt<uint8_t>(in_value, m_parameterBufferLength)) { return false; }
          m_parameterBufferLength = m_parameterBufferLength + sizeof(uint8_t);
          return true;
        }

        bool addInt16ToBuffer(int16_t in_value)
        {
          if (not setValueInBufferAt<int16_t>(in_value, m_parameterBufferLength)) { return false; }
          m_parameterBufferLength = m_parameterBufferLength + sizeof(int16_t);
          return true;
        }

        bool addUInt16ToBuffer(uint16_t in_value)
        {
          if (not setValueInBufferAt<uint16_t>(in_value, m_parameterBufferLength)) { return false; }
          m_parameterBufferLength = m_parameterBufferLength + sizeof(uint16_t);
          return true;
        }

        bool addStringToBuffer(const String& in_string)
        {
          uint16_t stringLength = in_string.length();
          if (m_parameterBufferLength + sizeof(uint16_t) + stringLength > m_parameterBuffer.size()) { return false; }

          if (not addUInt16ToBuffer(stringLength)) { return false; }
          in_string.toCharArray(m_parameterBuffer.data() + m_parameterBufferLength, stringLength);
          m_parameterBufferLength = m_parameterBufferLength + stringLength;

          return true;
        }

      public:
        SerialGFXInterface(HALVOE_SERIAL_TYPE& io_serial) :
          m_serial(io_serial)
        {}

        bool begin(SerialGFXBaud in_baud = SerialGFXBaud::Default)
        {
          pinMode(READY_PIN, INPUT);
          m_serial.begin(fromSerialGFXBaud(in_baud));
          elapsedMillis timeSinceBegin;
          while (not m_serial && timeSinceBegin < 10000) {}
          return m_serial;
        }

        bool isGPUReady()
        {
          return digitalRead(READY_PIN) == HIGH;
        }

        bool sendSwap()
        {
          clearBuffer();
          return sendCommand(SerialGFXCommandCode::swap);
        }

        bool sendFillScreen(uint16_t in_color)
        {
          clearBuffer();
          addUInt16ToBuffer(in_color);
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

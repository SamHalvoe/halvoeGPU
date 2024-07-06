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
        HALVOE_SERIAL_TYPE& m_serial;
        DVIGFX8& m_dviGFX;

        SerialGFXCommandCode m_receivedCommandCode = SerialGFXCommandCode::noCommand;
        uint16_t m_parameterBufferLength = 0;
        std::array<char, 2> m_commandBuffer;
        std::array<char, g_maxParameterBufferLength> m_parameterBuffer;

      private:
        void dviGFX_swap()
        {
          m_dviGFX.swap();
        }

        void dviGFX_fillScreen()
        {
          m_dviGFX.fillScreen(0);
        }

        void dviGFX_fillRect()
        {
          if (m_parameterBufferLength < 10) { return; }

          int16_t x      = *reinterpret_cast<int16_t*>(m_parameterBuffer.data());
          int16_t y      = *reinterpret_cast<int16_t*>(m_parameterBuffer.data() + 2);
          int16_t width  = *reinterpret_cast<int16_t*>(m_parameterBuffer.data() + 4);
          int16_t height = *reinterpret_cast<int16_t*>(m_parameterBuffer.data() + 6);
          uint16_t color = *reinterpret_cast<uint16_t*>(m_parameterBuffer.data() + 8);

          m_dviGFX.fillRect(x, y, width, height, color);
        }

      public:
        SerialGFXInterface(HALVOE_SERIAL_TYPE& io_serial, DVIGFX8& io_dviGFX) :
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
        
        bool receiveCommand()
        {
          if (m_receivedCommandCode != SerialGFXCommandCode::noCommand) { return false; }

          if (m_serial.available() >= 4)
          {
            size_t receivedBytesCount = m_serial.readBytes(m_commandBuffer.data(), 2);
            if (receivedBytesCount != 2) { return false; }
            m_receivedCommandCode = toSerialGFXCommandCode(*reinterpret_cast<uint16_t*>(m_commandBuffer.data()));

            receivedBytesCount = m_serial.readBytes(m_commandBuffer.data(), 2);
            if (receivedBytesCount != 2) { return false; }
            m_parameterBufferLength = *reinterpret_cast<uint16_t*>(m_commandBuffer.data());

            receivedBytesCount = m_serial.readBytes(m_parameterBuffer.data(), m_parameterBufferLength);
            if (receivedBytesCount != m_parameterBufferLength) { return false; }

            return true;
          }

          return false;
        }

        bool runCommand()
        {
          Serial.print("runCommand: ");
          Serial.println(fromSerialGFXCommandCode(m_receivedCommandCode));

          switch (m_receivedCommandCode)
          {
            case SerialGFXCommandCode::swap: dviGFX_swap(); break;
            case SerialGFXCommandCode::fillScreen: dviGFX_fillScreen(); break;
            case SerialGFXCommandCode::fillRect: dviGFX_fillRect(); break;
          }

          m_receivedCommandCode = SerialGFXCommandCode::noCommand;
          return true;
        }
    };
  }
}

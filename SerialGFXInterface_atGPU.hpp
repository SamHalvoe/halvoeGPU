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
        elapsedMillis m_timeSinceLastFrame;
        unsigned long m_maxFrameTime = 1000;
        bool m_isPrintFPSEnabled = false;

        SerialGFXCommandCode m_receivedCommandCode = SerialGFXCommandCode::noCommand;
        uint16_t m_parameterBufferLength = 0;
        std::array<char, 2> m_commandBuffer;
        std::array<char, g_maxParameterBufferLength> m_parameterBuffer;

      private:
        void dviGFX_swap()
        {
          if (m_timeSinceLastFrame < 5) { return; }
          if (m_isPrintFPSEnabled) { printFrameTime(); }
          m_dviGFX.swap();
          m_timeSinceLastFrame = 0;
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

        void printFPS()
        {
          m_dviGFX.setTextColor(255);
          m_dviGFX.setCursor(320 - 50, 5);
          m_dviGFX.print(1000 / getFrameTime());
          m_dviGFX.print(" FPS");
        }

        void printFrameTime()
        {
          m_dviGFX.setTextColor(255);
          m_dviGFX.setCursor(320 - 50, 5);
          if (getFrameTime() < 10) { m_dviGFX.print(" "); }
          m_dviGFX.print(getFrameTime());
          m_dviGFX.print(" ms");
        }

      public:
        SerialGFXInterface(HALVOE_SERIAL_TYPE& io_serial, DVIGFX8& io_dviGFX) :
          m_serial(io_serial), m_dviGFX(io_dviGFX)
        {}

        bool begin(SerialGFXBaud in_baud = SerialGFXBaud::DEFAULT)
        {
          if (not m_dviGFX.begin()) // false if insufficient RAM
          {
            return false;
          }

          m_serial.setFIFOSize(128);
          m_serial.begin(fromSerialGFXBaud(in_baud));
          elapsedMillis timeSinceBegin;

          while (not m_serial && timeSinceBegin < 10000)
          {}

          return m_serial;
        }
        
        bool sendCode(SerialGFXCode in_code)
        {
          uint16_t code = fromSerialGFXCode(in_code);
          return m_serial.write(reinterpret_cast<char*>(&code), 2) != 2;
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

        unsigned long getFrameTime() const
        {
          unsigned long frameTime = m_timeSinceLastFrame;
          return min(frameTime, m_maxFrameTime);
        }

        void enablePrintFPS()
        {
          m_isPrintFPSEnabled = true;
        }

        void disablePrintFPS()
        {
          m_isPrintFPSEnabled = false;
        }
    };
  }
}

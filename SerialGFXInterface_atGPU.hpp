#pragma once

#include <PicoDVI.h>
#include <elapsedMillis.h>
#include <array>

#include "SerialGFXInterface.hpp"
#include "halvoeVersion.hpp"

namespace halvoeGPU
{
  namespace atGPU
  {
    const pin_size_t READY_PIN = 24;

    class SerialGFXInterface
    {
      private:
        HALVOE_SERIAL_TYPE& m_serial;
        DVIGFX8& m_dviGFX;
        elapsedMillis m_timeSinceLastFrame;
        unsigned long m_maxFrameTimeMs = 1000;
        bool m_isPrintFrameTimeEnabled = false;
        bool m_isPrintFPSEnabled = false;

        SerialGFXCommandCode m_receivedCommandCode = SerialGFXCommandCode::noCommand;
        uint16_t m_parameterBufferLength = 0;
        std::array<char, 2> m_commandBuffer;
        std::array<char, g_maxParameterBufferLength> m_parameterBuffer;

      private:
        void dviGFX_swap()
        {
          if (m_timeSinceLastFrame < 5) { return; }
          if (m_isPrintFrameTimeEnabled) { printFrameTime(); }
          if (m_isPrintFPSEnabled) { printFPS(); }
          m_dviGFX.swap();
          m_timeSinceLastFrame = 0;
        }

        void dviGFX_fillScreen()
        {
          if (m_parameterBufferLength < 2) { return; }
          uint16_t color = *reinterpret_cast<uint16_t*>(m_parameterBuffer.data());
          m_dviGFX.fillScreen(color);
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
          String fps(1000 / getFrameTime());
          fps.concat(" FPS");
          uint16_t width = 0;
          m_dviGFX.getTextBounds(fps, 0, 0, nullptr, nullptr, &width, nullptr);
          m_dviGFX.setCursor(320 - 5 - width, 5);
          m_dviGFX.print(fps);
        }

        void printFrameTime()
        {
          m_dviGFX.setTextColor(255);
          m_dviGFX.setCursor(320 - 45, m_isPrintFPSEnabled ? 15 : 5);
          String frameTime(getFrameTime());
          frameTime.concat(" ms");
          uint16_t width = 0;
          m_dviGFX.getTextBounds(frameTime, 0, 0, nullptr, nullptr, &width, nullptr);
          m_dviGFX.setCursor(320 - 5 - width, 5);
          m_dviGFX.print(frameTime);
        }

      public:
        SerialGFXInterface(HALVOE_SERIAL_TYPE& io_serial, DVIGFX8& io_dviGFX) :
          m_serial(io_serial), m_dviGFX(io_dviGFX)
        {}

        bool begin(SerialGFXBaud in_baud = SerialGFXBaud::Default)
        {
          pinMode(READY_PIN, OUTPUT);
          writeReady(false);

          if (not m_dviGFX.begin()) { return false; } // false if (probably) insufficient RAM
          setupDefaultPalette();

          m_serial.setFIFOSize(128);
          m_serial.begin(fromSerialGFXBaud(in_baud));
          elapsedMillis timeSinceBegin;
          while (not m_serial && timeSinceBegin < 10000) {}
          return m_serial;
        }

        void setupDefaultPalette()
        {
          for (uint16_t index = 0; index < g_colorCount; ++index)
          {
            m_dviGFX.setColor(index, index, index, index);
          }

          m_dviGFX.swap(false, true); // Duplicate same palette into front & back buffers
        }

        void writeReady(bool in_isReady)
        {
          digitalWrite(READY_PIN, in_isReady ? HIGH : LOW);
        }

        void printVersion()
        {
          m_dviGFX.setCursor(5, 5);
          m_dviGFX.print("halvoeGPU");
          m_dviGFX.setCursor(5, 15);
          m_dviGFX.print("Version: ");
          m_dviGFX.print(buildVersion);
          m_dviGFX.setCursor(5, 25);
          m_dviGFX.print("Build: ");
          m_dviGFX.print(buildTimestamp);
          #ifdef HALVOE_GPU_DEBUG
            m_dviGFX.setCursor(5, 35);
            m_dviGFX.print("HALVOE_GPU_DEBUG is enabled!");
          #endif // HALVOE_GPU_DEBUG
          m_dviGFX.swap();
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
          #ifdef HALVOE_GPU_DEBUG
            Serial.print("runCommand: ");
            Serial.println(fromSerialGFXCommandCode(m_receivedCommandCode));
          #endif // HALVOE_GPU_DEBUG

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
          unsigned long frameTimeMs = m_timeSinceLastFrame;
          return min(frameTimeMs, m_maxFrameTimeMs);
        }

        void enablePrintFrameTime()
        {
          m_isPrintFrameTimeEnabled = true;
        }

        void disablePrintFrameTime()
        {
          m_isPrintFrameTimeEnabled = false;
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

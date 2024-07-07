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
    const size_t GPU_SERIAL_RECEIVE_FIFO_SIZE = 256;

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
        template<typename ParameterType>
        ParameterType getParameterFromBuffer(size_t in_bufferOffset)
        {
          return *reinterpret_cast<ParameterType*>(m_parameterBuffer.data() + in_bufferOffset);
        }

        const char* getCStringFromBuffer(size_t in_bufferOffset, uint16_t* out_stringLenght)
        {
          if (out_stringLenght != nullptr)
          {
            *out_stringLenght = getParameterFromBuffer<uint16_t>(in_bufferOffset);
          }
          
          return reinterpret_cast<const char*>(m_parameterBuffer.data() + in_bufferOffset + sizeof(uint16_t));
        }

        void cmd_swap()
        {
          if (m_timeSinceLastFrame < 4) { return; }
          if (m_isPrintFrameTimeEnabled) { printFrameTime(); }
          if (m_isPrintFPSEnabled) { printFPS(); }
          m_dviGFX.swap();
          m_timeSinceLastFrame = 0;
        }

        void cmd_fillScreen()
        {
          if (m_parameterBufferLength < 2) { return; }
          uint16_t color = getParameterFromBuffer<uint16_t>(0);
          m_dviGFX.fillScreen(color);
        }

        void cmd_fillRect()
        {
          if (m_parameterBufferLength < 10) { return; }
          int16_t x      = getParameterFromBuffer<int16_t>(0);
          int16_t y      = getParameterFromBuffer<int16_t>(2);
          int16_t width  = getParameterFromBuffer<int16_t>(4);
          int16_t height = getParameterFromBuffer<int16_t>(6);
          uint16_t color = getParameterFromBuffer<uint16_t>(8);
          m_dviGFX.fillRect(x, y, width, height, color);
        }

        void cmd_drawRect()
        {
          if (m_parameterBufferLength < 10) { return; }
          int16_t x      = getParameterFromBuffer<int16_t>(0);
          int16_t y      = getParameterFromBuffer<int16_t>(2);
          int16_t width  = getParameterFromBuffer<int16_t>(4);
          int16_t height = getParameterFromBuffer<int16_t>(6);
          uint16_t color = getParameterFromBuffer<uint16_t>(8);
          m_dviGFX.drawRect(x, y, width, height, color);
        }

        void cmd_setFont()
        {
          if (m_parameterBufferLength < 1) { return; }
          uint8_t font = getParameterFromBuffer<uint8_t>(0);
          
          if (GFXfont* fontPointer = nullptr; getFontPointer(static_cast<SerialGFXFont>(font), fontPointer))
          {
            m_dviGFX.setFont(fontPointer);
          }
        }

        void cmd_setTextSize()
        {
          if (m_parameterBufferLength < 1) { return; }
          uint8_t size = getParameterFromBuffer<uint8_t>(0);
          m_dviGFX.setTextSize(size);
        }

        void cmd_setTextColor()
        {
          if (m_parameterBufferLength < 2) { return; }
          uint16_t color = getParameterFromBuffer<uint16_t>(0);
          m_dviGFX.setTextSize(color);
        }

        void cmd_setCursor()
        {
          if (m_parameterBufferLength < 4) { return; }
          int16_t x = getParameterFromBuffer<int16_t>(0);
          int16_t y = getParameterFromBuffer<int16_t>(2);
          m_dviGFX.setCursor(x, y);
        }

        void cmd_print()
        {
          m_dviGFX.print(getCStringFromBuffer(0, nullptr));
        }

        void cmd_println()
        {
          m_dviGFX.println(getCStringFromBuffer(0, nullptr));
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
          m_dviGFX.cp437(true);
          setupDefaultPalette();

          #ifdef HALVOE_GPU_DEBUG
            if (not m_serial.setFIFOSize(GPU_SERIAL_RECEIVE_FIFO_SIZE))
            { Serial.println("setFIFOSize(" + String(GPU_SERIAL_RECEIVE_FIFO_SIZE) + ") failed!"); }
          #else
            m_serial.setFIFOSize(GPU_SERIAL_RECEIVE_FIFO_SIZE);
          #endif // HALVOE_GPU_DEBUG

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
            case SerialGFXCommandCode::swap:        cmd_swap(); break;
            case SerialGFXCommandCode::fillScreen:  cmd_fillScreen(); break;
            case SerialGFXCommandCode::fillRect:    cmd_fillRect(); break;
            case SerialGFXCommandCode::drawRect:    cmd_drawRect(); break;
            case SerialGFXCommandCode::setFont:     cmd_setFont(); break;
            case SerialGFXCommandCode::setTextSize: cmd_setTextSize(); break;
            case SerialGFXCommandCode::setCursor:   cmd_setCursor(); break;
            case SerialGFXCommandCode::print:       cmd_print(); break;
            case SerialGFXCommandCode::println:     cmd_println(); break;
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

#pragma once

#include <Adafruit_GFX.h>
#include <elapsedMillis.h>
#include <array>

#include "SerialGFXInterface.hpp"

namespace halvoeGPU
{
  namespace atCPU
  {
    const uint8_t READY_PIN = 40;

    // This class exists, because we need some methods from Adafruit_GFX (in particular getTextBounds()),
    // but we do not need a working Adafruit_GFX like GFXcanvas8 or DVIGFX8.
    class HelperGFX : public Adafruit_GFX
    {
      public:
        HelperGFX(uint16_t w, uint16_t h) : Adafruit_GFX(w, h)
        {}

      private:
        void drawPixel(int16_t x, int16_t y, uint16_t color)
        {
          // This method does nothing. (see comment of class)
        }
    };

    class SerialGFXInterface
    {
      private:
        HALVOE_SERIAL_TYPE& m_serial;
        uint16_t m_parameterBufferLength = 0;
        std::array<char, 2> m_commandBuffer;
        std::array<char, g_maxParameterBufferLength> m_parameterBuffer;
        HelperGFX m_helperGFX;

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

        bool addCharToBuffer(char in_value)
        {
          if (not setValueInBufferAt<char>(in_value, m_parameterBufferLength)) { return false; }
          m_parameterBufferLength = m_parameterBufferLength + sizeof(char);
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
          size_t stringLength = in_string.length();
          if (m_parameterBufferLength + stringLength + g_zeroTerminatorLength > m_parameterBuffer.size()) { return false; }

          in_string.toCharArray(m_parameterBuffer.data() + m_parameterBufferLength, stringLength + g_zeroTerminatorLength);
          #ifdef HALVOE_GPU_DEBUG
            Serial.println(m_parameterBuffer.data() + m_parameterBufferLength);
          #endif // HALVOE_GPU_DEBUG
          m_parameterBufferLength = m_parameterBufferLength + stringLength + g_zeroTerminatorLength;
                   
          return true;
        }

      public:
        SerialGFXInterface(HALVOE_SERIAL_TYPE& io_serial) :
          m_serial(io_serial), m_helperGFX(g_screenWidth, g_screenHeight)
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

        void getTextBounds(const char* in_string, int16_t in_x, int16_t in_y,
                           int16_t* out_x, int16_t* out_y, uint16_t* out_width, uint16_t* out_height)
        {
          m_helperGFX.getTextBounds(in_string, in_x, in_y, out_x, out_y, out_width, out_height);
        }

        void getTextBounds(const String& in_string, int16_t in_x, int16_t in_y,
                           int16_t* out_x, int16_t* out_y, uint16_t* out_width, uint16_t* out_height)
        {
          m_helperGFX.getTextBounds(in_string, in_x, in_y, out_x, out_y, out_width, out_height);
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

        bool sendDrawRect(int16_t in_x, int16_t in_y, int16_t in_width, int16_t in_height, uint16_t in_color)
        {
          clearBuffer();
          addInt16ToBuffer(in_x);
          addInt16ToBuffer(in_y);
          addInt16ToBuffer(in_width);
          addInt16ToBuffer(in_height);
          addUInt16ToBuffer(in_color);
          return sendCommand(SerialGFXCommandCode::drawRect);
        }

        bool sendSetFont(SerialGFXFont in_font)
        {
          GFXfont* fontPointer = nullptr;
          if (not getFontPointer(in_font, fontPointer)) { return false; }
          m_helperGFX.setFont(fontPointer); // set for getTextBounds() atCPU

          clearBuffer();
          addUInt8ToBuffer(fromSerialGFXFont(in_font));
          return sendCommand(SerialGFXCommandCode::setFont);
        }

        bool sendSetTextSize(uint8_t in_size)
        {
          m_helperGFX.setTextSize(in_size); // set for getTextBounds() atCPU

          clearBuffer();
          addUInt8ToBuffer(in_size);
          return sendCommand(SerialGFXCommandCode::setTextSize);
        }

        bool sendSetTextColor(uint16_t in_color)
        {
          clearBuffer();
          addUInt16ToBuffer(in_color);
          return sendCommand(SerialGFXCommandCode::setTextColor);
        }

        bool sendSetCursor(int16_t in_x, int16_t in_y)
        {
          m_helperGFX.setCursor(in_x, in_y); // set for getTextBounds() atCPU

          clearBuffer();
          addInt16ToBuffer(in_x);
          addInt16ToBuffer(in_y);
          return sendCommand(SerialGFXCommandCode::setCursor);
        }

        bool sendPrint(const String& in_string)
        {
          clearBuffer();
          addStringToBuffer(in_string);
          return sendCommand(SerialGFXCommandCode::print);
        }

        bool sendPrintln(const String& in_string)
        {
          clearBuffer();
          addStringToBuffer(in_string);
          return sendCommand(SerialGFXCommandCode::println);
        }
    };
  }
}

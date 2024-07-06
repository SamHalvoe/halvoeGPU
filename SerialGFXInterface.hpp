#pragma once

#ifdef ARDUINO_ARCH_RP2040
  #define HALVOE_SERIAL_TYPE SerialUART
#elif __IMXRT1062__
  #define HALVOE_SERIAL_TYPE HardwareSerialIMXRT
#else
  #error Invalid MCU!
#endif // ARDUINO_ARCH_RP2040 / 

namespace halvoeGPU
{
  const size_t g_maxParameterBufferLength = 16384;

  enum class SerialGFXCommandCode : uint16_t
  {
    invalid = 0,
    noCommand,
    swap,
    fillScreen,
    fillRect
  };

  SerialGFXCommandCode toSerialGFXCommandCode(uint16_t in_value)
  {
    switch (static_cast<SerialGFXCommandCode>(in_value))
    {
      case SerialGFXCommandCode::noCommand:
      case SerialGFXCommandCode::swap:
      case SerialGFXCommandCode::fillScreen:
      case SerialGFXCommandCode::fillRect:
        return static_cast<SerialGFXCommandCode>(in_value);
    }

    return SerialGFXCommandCode::invalid;
  }

  uint16_t fromSerialGFXCommandCode(SerialGFXCommandCode in_code)
  {
    return static_cast<uint16_t>(in_code);
  }
}

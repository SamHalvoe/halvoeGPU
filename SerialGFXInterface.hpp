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

  enum class SerialGFXBaud : unsigned long
  {
    FALLBACK = 9600,
    MIN = 115200,
    QUARTER = 250000,
    HALF = 576000,
    DEFAULT = 1000000,
    DOUBLE = 2000000,
    QUAD = 4000000,
    MAX = 6000000
  };

  enum class SerialGFXCode : uint16_t
  {
    invalid = 0,
    gpuIsReady
  };

  enum class SerialGFXCommandCode : uint16_t
  {
    invalid = 0,
    noCommand,
    swap,
    fillScreen,
    fillRect
  };

  unsigned long fromSerialGFXBaud(SerialGFXBaud in_baud)
  {
    return static_cast<unsigned long>(in_baud);
  }

  SerialGFXCode toSerialGFXCode(uint16_t in_value)
  {
    switch (static_cast<SerialGFXCode>(in_value))
    {
      case SerialGFXCode::gpuIsReady:
        return static_cast<SerialGFXCode>(in_value);
    }

    return SerialGFXCode::invalid;
  }

  uint16_t fromSerialGFXCode(SerialGFXCode in_code)
  {
    return static_cast<uint16_t>(in_code);
  }

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

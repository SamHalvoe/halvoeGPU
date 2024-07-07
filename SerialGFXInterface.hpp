#pragma once

#ifdef ARDUINO_ARCH_RP2040
  #define HALVOE_SERIAL_TYPE SerialUART
#elif __IMXRT1062__
  #define HALVOE_SERIAL_TYPE HardwareSerialIMXRT
#else
  #error Invalid MCU!
#endif // ARDUINO_ARCH_RP2040 / 

//#define HALVOE_GPU_DEBUG

namespace halvoeGPU
{
  const uint16_t g_colorCount = 256;
  const size_t g_maxParameterBufferLength = 8192;

  enum class SerialGFXBaud : unsigned long
  {
    Fallback = 9600,
    Min = 115200,
    Quarter = 250000,
    Half = 576000,
    Default = 1000000,
    Double = 2000000,
    Quad = 4000000,
    Max = 6000000
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

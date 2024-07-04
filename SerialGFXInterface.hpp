#pragma once

namespace halvoeGPU
{
  enum class SerialGFXCommandElement : uint16_t
  {
    beginCommand = 0x00FF,
    endCommand = 0xFF00,
    commandCode = 0x00FF,
    parameterCount,
    parameterOfTypeBool,
    parameterOfTypeInt,
    parameterOfTypeUInt,
    parameterOfTypeString
  };

  enum class SerialGFXCommandCode : uint16_t
  {
    invalid = 0,
    noCommand,
    swap,
    fillScreen
  };

  SerialGFXCommandCode toSerialGFXCommandCode(uint16_t in_value)
  {
    switch()
    {
      case SerialGFXCommandCode::noCommand:
      case SerialGFXCommandCode::swap:
      case SerialGFXCommandCode::fillScreen:
        return static_cast<SerialGFXCommandCode>(in_value);
    }

    return SerialGFXCommandCode::invalid;
  }
}

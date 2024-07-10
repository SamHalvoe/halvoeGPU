#pragma once

namespace halvoeCString
{
  size_t getLength(const char* in_string, size_t in_maxLength)
  {
    if (in_string == nullptr) { return 0; }
    
    for (size_t index = 0; index < in_maxLength; ++index)
    {
      if (in_string[index] == '\0') { return index; }
    }

    return in_maxLength;
  }

  bool copy(const char* in_string, char* out_buffer, size_t in_stringLength)
  {
    if (in_string == nullptr || out_buffer == nullptr) { return false; }

    size_t index = 0;
    for (; index < in_stringLength; ++index)
    {
      out_buffer[index] = in_string[index];
    }

    out_buffer[index] = '\0';

    return true;
  }
}

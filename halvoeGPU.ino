// Double-buffered 8-bit Adafruit_GFX-compatible framebuffer for PicoDVI.
// Animates without redraw flicker. Requires Adafruit_GFX >= 1.11.4

#include <PicoDVI.h>
#include "SerialGFXInterface_atGPU.hpp"

// Here's how a 320x240 8-bit (color-paletted) framebuffer is declared.
// Second argument ('true' here) enables double-buffering for flicker-free
// animation. Third argument is a hardware configuration -- examples are
// written for Adafruit Feather RP2040 DVI, but that's easily switched out
// for boards like the Pimoroni Pico DV (use 'pimoroni_demo_hdmi_cfg') or
// Pico DVI Sock ('pico_sock_cfg').
DVIGFX8 dviGFX(DVI_RES_320x240p60, true, adafruit_feather_dvi_cfg);
halvoeGPU::atGPU::SerialGFXInterface serialGFXInterface(Serial1, dviGFX);

const uint16_t g_colorCount = 256;
String g_inText;

void setup()
{
  if (not serialGFXInterface.begin())
  {
    pinMode(LED_BUILTIN, OUTPUT);
    for (;;) digitalWrite(LED_BUILTIN, (millis() / 500) & 1);
  }

  for (uint16_t index = 0; index < g_colorCount; ++index)
  {
    dviGFX.setColor(index, index, index, index);
  }

  dviGFX.swap(false, true); // Duplicate same palette into front & back buffers
}

void loop()
{
  dviGFX.fillScreen(0);
  
  dviGFX.fillRect(5, 5, 25, 25, 255);
  dviGFX.fillRect(25, 25, 25, 25, 127);

  dviGFX.setTextColor(255);
  dviGFX.setCursor(35, 10);
  dviGFX.print("Hello Arduino DVI!");

  /*if (Serial1.available() > 0)
  {
    g_inText = Serial1.readStringUntil('\0');
  }*/

  dviGFX.setCursor(35, 55);
  dviGFX.print(g_inText);

  dviGFX.swap();
}

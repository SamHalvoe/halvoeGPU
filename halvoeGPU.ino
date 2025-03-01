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

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);

  #ifdef HALVOE_GPU_DEBUG
    Serial.begin(115200);
    while (not Serial) { delay(1000); }
    Serial.println("gpu serial to usb ready");
  #endif // HALVOE_GPU_DEBUG

  if (not serialGFXInterface.begin())
  {
    while (true) { digitalWrite(LED_BUILTIN, HIGH); delay(250); digitalWrite(LED_BUILTIN, LOW); }
  }

  serialGFXInterface.enablePrintFrameTime();
  serialGFXInterface.enablePrintFPS();
  serialGFXInterface.printVersion();
  serialGFXInterface.writeReady(true);
}

void loop()
{
  if (serialGFXInterface.receiveCommand())
  {
    serialGFXInterface.runCommand();
  }
}

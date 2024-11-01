
// TTGO ESP32 S3 T-Display

  sendCmd(ST7789_SLPOUT);
  delay(120);
  sendCmd(ST7789_NORON);
  sendCmdByte(TFT_MADCTL, TFT_MAD_RGB);
 // sendCmdData(ST7789_RAMCTRL,  // writedata(0x00); // writedata(0xE0); // 5 to 6-bit conversion: r0 = r5, b0 = b5
  sendCmdData(TFT_PIXFMT, 0x55);
  delay(10);
  sendCmdData(ST7789_PORCTRL, "\x0b\x0b\x00\x33\x33", 5);
  sendCmdByte(ST7789_GCTRL, 0x75);  // Voltages: VGH / VGL0x75
  sendCmdByte(ST7789_VCOMS, 0x28);  // JLX240 display datasheet
  sendCmdByte(ST7789_LCMCTRL, 0x2C);
  sendCmdByte(ST7789_VDVVRHEN, 0x01);
  sendCmdByte(ST7789_VRHS, 0x1F);       // voltage VRHS
  sendCmdByte(ST7789_FRCTR2, 0x13);
  sendCmdByte(ST7789_PWCTRL1, 0xa7);
  sendCmdData(ST7789_PWCTRL1, "\xa4\xa1", 2);
  sendCmdByte(0xD6, 0xa1);
  sendCmdData(ST7789_PVGAMCTRL, "\xf0\x05\x0a\x06\x06\x03\x2b\x32\x43\x36\x11\x10\x2b\x32", 14);
  sendCmdData(ST7789_NVGAMCTRL, "\xf0\x08\x0c\x0b\x09\x24\x2b\x22\x43\x38\x15\x16\x2f\x37", 14);
  delay(120);
  sendCmd(ST7789_DISPON);     //Display on
  delay(120);

#ifdef TFT_BL
  // Turn on the back-light LED
  digitalWrite(TFT_BL, HIGH);
  pinMode(TFT_BL, OUTPUT);
#endif


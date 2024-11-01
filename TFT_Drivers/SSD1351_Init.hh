



  sendCmdByte(0xFD, 0x12); // COMMANDLOCK
  sendCmdByte(0xFD, 0xB1); // COMMANDLOCK
  sendCmd(0xAE);           // DISPLAYOFF
  sendCmdByte(0xB3, 0xF1); // CLOCKDIV
  sendCmdByte(0xCA, 0x7F); // MUXRATIO
  sendCmdByte(0xA2, 0x00); // DISPLAYOFFSET
  sendCmdByte(0xB5, 0x00); // SETGPIO
  sendCmdByte(0xAB, 0x01); // FUNCTIONSELECT
  sendCmdByte(0xB1, 0x32); // PRECHARGE
  sendCmdByte(0xBE, 0x05); // VCOMH
  sendCmd(0xA6);           // NORMALDISPLAY
  sendCmdData(0xC1, (uint8_t*)"\xC8\x80\xC8", 3); // CONTRASTABC
  sendCmdByte(0xC7, 0x0F); // CONTRASTMASTER
  sendCmdData(0xB4, (uint8_t*)"\xA0\xB5\x55", 3); // SETVSL
  sendCmdByte(0xB6, 0x01); // PRECHARGE2
  sendCmd(0xAF);           // DISPLAYON


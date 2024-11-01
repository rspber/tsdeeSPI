
  rotation = m % 4; // Limit the range of values to 0-3

  madctl = 0x64;

  switch (rotation) {
    case 0:
      madctl |= 0x10;
      break;
    case 1:
      madctl |= 0x13;
      break;
    case 2:
      madctl |= 0x02;
      break;
    case 3:
      madctl |= 0x01;
      break;
  }

  sendCmdByte(0xA0, madctl); // SETREMAP
  sendCmdByte(0xA1, rotation < 2 ? TFT_HEIGHT : 0); // STARTLINE

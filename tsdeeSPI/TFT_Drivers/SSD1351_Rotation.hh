
  rotation = m % 4; // Limit the range of values to 0-3

  _madctl = 0x64;

  switch (rotation) {
    case 0:
      _madctl |= 0x10;
      break;
    case 1:
      _madctl |= 0x13;
      break;
    case 2:
      _madctl |= 0x02;
      break;
    case 3:
      _madctl |= 0x01;
      break;
  }

  sendCmdByte(0xA0, _madctl); // SETREMAP
  sendCmdByte(0xA1, rotation < 2 ? TFT_HEIGHT : 0); // STARTLINE

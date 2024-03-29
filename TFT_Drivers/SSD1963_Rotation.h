
// This is the command sequence that rotates the SSD1963 driver coordinate frame

  rotation = m % 4; // Limit the range of values to 0-3

  writecommand(TFT_MADCTL);
  switch (rotation) {
    case 0:
      writedata(0x21 | TFT_MAD_COLOR_ORDER);
      break;
    case 1:
      writedata(0x00 | TFT_MAD_COLOR_ORDER);
      break;
    case 2:
      writedata(0x22 | TFT_MAD_COLOR_ORDER);
      break;
    case 3:
      writedata(0x03 | TFT_MAD_COLOR_ORDER);
      break;

  }

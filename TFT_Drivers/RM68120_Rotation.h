
// This is the command sequence that rotates the RM68120 driver coordinate frame

  rotation = m % 4; // Limit the range of values to 0-3

  writecommand(TFT_MADCTL);
  switch (rotation) {
    case 0:
      writedata(TFT_MAD_COLOR_ORDER);
      break;
    case 1:
      writedata(TFT_MAD_MV | TFT_MAD_MX | TFT_MAD_COLOR_ORDER);
      break;
    case 2:
      writedata(TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_COLOR_ORDER);
      break;
    case 3:
      writedata(TFT_MAD_MV | TFT_MAD_MY | TFT_MAD_COLOR_ORDER);
      break;

  }

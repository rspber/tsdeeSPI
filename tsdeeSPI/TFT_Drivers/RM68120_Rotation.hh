
// This is the command sequence that rotates the RM68120 driver coordinate frame

  rotation = m % 4; // Limit the range of values to 0-3
  uint8_t reg = 0;

  switch (rotation) {
    case 0:
      reg = TFT_MAD_RGB;
      break;
    case 1:
      reg = TFT_MAD_YX | TFT_MAD_MX | TFT_MAD_RGB;
      break;
    case 2:
      reg = TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_RGB;
      break;
    case 3:
      reg = TFT_MAD_YX | TFT_MAD_MY | TFT_MAD_RGB;
      break;
  }
  writeRegister16(TFT_MADCTL, reg);

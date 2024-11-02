
// This is the command sequence that rotates the SSD1963 driver coordinate frame

  rotation = m % 4; // Limit the range of values to 0-3

  switch (rotation) {
    case 0:
      setMADCTL((0x21 ^ REV) | TFT_MAD_RGB);
      break;
    case 1:
      setMADCTL((0x00 ^ REV) | TFT_MAD_RGB);
      break;
    case 2:
      setMADCTL((0x22 ^ REV) | TFT_MAD_RGB);
      break;
    case 3:
      setMADCTL((0x03 ^ REV) | TFT_MAD_RGB);
      break;
  }

/*
  RM68140 rotation
*/

  uint8_t T[] {0x00, 0x02, 0x3B};
  rotation = m % 4;
  switch (rotation) {
    case 0: // Portrait
      setMDCTL((0x00 ^ REV) | TFT_MAD_BGR);
      T[1] |= ((MAD_SS ^ REV) << 4);
    break;
    case 1: // Landscape (Portrait + 90)
      setMADCTL((MAD_YX ^ REV) | TFT_MAD_BGR);
      T[1] |= 0x00;
    break;
    case 2: // Inverter portrait
      setMADCTL((0x00 ^ REV) | TFT_MAD_BGR);
      T[1] |= ((MAD_GS ^ REV) << 6);
    break;
    case 3: // Inverted landscape
      setMADCTL((MAD_YX ^ REV) | TFT_MAD_BGR);
      T[1] |= ((MAD_SS ^ REV) << 4) | ((MAD_GS ^ REV) << 6);
    break;
  }
  sendCmdData(0xB6, T, 3);

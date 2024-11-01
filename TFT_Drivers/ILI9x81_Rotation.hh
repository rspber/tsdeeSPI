/*
  ILI9481 rotation
*/
  rotation = m % 4;

  switch (rotation) {
    case 0: // Portrait
      setMADCTL(((TFT_MAD_SS) ^ REV) | TFT_MAD_BGR);
      break;
    case 1: // Landscape (Portrait + 90)
      setMADCTL(((TFT_MAD_YX) ^ REV) | TFT_MAD_BGR);
      break;
    case 2: // Inverter portrait
      setMADCTL(((TFT_MAD_GS) ^ REV) | TFT_MAD_BGR);
      break;
    case 3: // Inverted landscape
      setMADCTL(((TFT_MAD_SS | TFT_MAD_GS | TFT_MAD_YX) ^ REV) | TFT_MAD_BGR);
      break;
  }

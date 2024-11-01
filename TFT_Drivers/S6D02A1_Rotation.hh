
// This is the command sequence that rotates the S6D02A1 driver coordinate frame

  rotation = m % 4;

  switch (rotation) {
    case 0:
      setMADCTL(((TFT_MAD_MX | TFT_MAD_MY) ^ REV) | TFT_MAD_BGR);
      break;
    case 1:
      setMADCTL(((TFT_MAD_YX | TFT_MAD_MY) ^ REV) | TFT_MAD_BGR);
      break;
    case 2:
      setMADCTL(((0x00) ^ REV) | TFT_MAD_BGR);
      break;
    case 3:
      setMADCTL(((TFT_MAD_MX | TFT_MAD_YX) ^ REV) | TFT_MAD_BGR);
      break;
  }

/*
  SSD196x rotation
*/
  rotation = m % 4;

  switch (rotation) {
    case 0:
      setMADCTL(((TFT_MAD_YX | TFT_MAD_GS) ^ REV) | TFT_MAD_BGR);
    break;
    case 1:
      setMADCTL(((0x00) ^ REV) | TFT_MAD_BGR);
    break;
    case 2:
      setMADCTL(((TFT_MAD_YX | TFT_MAD_SS) ^ REV) | TFT_MAD_BGR);
    break;
    case 3:
      setMADCTL(((TFT_MAD_SS | TFT_MAD_GS) ^ REV) | TFT_MAD_BGR);
    break;
  }


  rotation = m % 4;

  switch (rotation) {
    case 0:
      setMADCTL(((TFT_MAD_MX | TFT_MAD_YX) ^ REV) | TFT_MAD_RGB);
	  #ifdef CGRAM_OFFSET
        colstart = 0;
        rowstart = 0;
	  #endif
      break;
    case 1:
      setMADCTL(((TFT_MAD_YX | TFT_MAD_MY) ^ REV) | TFT_MAD_RGB);
	  #ifdef CGRAM_OFFSET
        colstart = 0;
        rowstart = 0;
	  #endif
      break;
    case 2:
      setMADCTL(((0x00) ^ REV) | TFT_MAD_BGR);
	  #ifdef CGRAM_OFFSET
        colstart = 0;
        rowstart = 32;
	  #endif
      break;
    case 3:
      setMADCTL(((TFT_MAD_MX | TFT_MAD_YX) ^ REV) | TFT_MAD_RGB);
	  #ifdef CGRAM_OFFSET
        colstart = 32;
        rowstart = 0;
	  #endif
      break;
  }

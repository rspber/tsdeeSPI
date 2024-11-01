
  rotation = m % 8; // Limit the range of values to 0-7

  switch (rotation) {
    case 0:
      setMADCTL(((TFT_MAD_MX)^ REV) | TFT_MAD_RGB);
#ifdef CGRAM_OFFSET
      if (_init_width == 222)
      {
        colstart = 49;
        rowstart = 0;
      }
#endif
      break;
    case 1:
      setMADCTL(((TFT_MAD_YX)^ REV) | TFT_MAD_RGB);
#ifdef CGRAM_OFFSET
      if (_init_width == 222)
      {
        colstart = 0;
        rowstart = 49;
      }
#endif
      break;
    case 2:
      setMADCTL(((TFT_MAD_MY)^ REV) | TFT_MAD_RGB);
#ifdef CGRAM_OFFSET
      if (_init_width == 222)
      {
        colstart = 49;
        rowstart = 0;
      }
#endif
      break;
    case 3:
      setMADCTL(((TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_YX)^ REV) | TFT_MAD_RGB);
#ifdef CGRAM_OFFSET
      if (_init_width == 222)
      {
        colstart = 0;
        rowstart = 49;
      }
#endif
      break;
  // These next rotations are for bottom up BMP drawing
    case 4:
      setMADCTL(((TFT_MAD_MX | TFT_MAD_MY)^ REV) | TFT_MAD_RGB);
      break;
    case 5:
      setMADCTL(((TFT_MAD_YX | TFT_MAD_MX)^ REV) | TFT_MAD_RGB);
      break;
    case 6:
      setMADCTL(((0x00)^ REV) | TFT_MAD_RGB);
      break;
    case 7:
      setMADCTL(((TFT_MAD_MY | TFT_MAD_YX)^ REV) | TFT_MAD_RGB);
      break;

  }

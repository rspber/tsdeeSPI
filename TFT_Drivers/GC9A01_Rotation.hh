
// This is the command sequence that rotates the GC9A01 driver coordinate frame

  rotation = m % 4;

  switch (rotation) {
    case 0: // Portrait
      setMADCTL(((0x00) ^ REV) | TFT_MAD_BGR);
#ifdef CGRAM_OFFSET
      if (_init_width == 128)
      {
        colstart = 2;
        rowstart = 1;
      }
#endif    
      break;
    case 1: // Landscape (Portrait + 90)
      writedata(((TFT_MAD_MX | TFT_MAD_YX) ^ REV) | TFT_MAD_BGR);
#ifdef CGRAM_OFFSET
      if (_init_width == 128)
      {
        colstart = 1;
        rowstart = 2;
      }
#endif
      break;
    case 2: // Inverter portrait
      writedata(((TFT_MAD_MX | TFT_MAD_MY) ^ REV) | TFT_MAD_BGR);
#ifdef CGRAM_OFFSET
      if (_init_width == 128)
      {
        colstart = 2;
        rowstart = 1;
      }
#endif
      break;
    case 3: // Inverted landscape
      writedata(((TFT_MAD_YX | TFT_MAD_MY) * REV) | TFT_MAD_BGR);
#ifdef CGRAM_OFFSET
      if (_init_width == 128)
      {
        colstart = 1;
        rowstart = 2;
      }
#endif
      break;
  }

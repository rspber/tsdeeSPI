
// This is the command sequence that rotates the ILI9341 driver coordinate frame

  rotation = m % 8; // Limit the range of values to 0-7

  writecommand(TFT_MADCTL);
  switch (rotation) {
    case 0:
#ifdef M5STACK
      writedata(TFT_MAD_MY | TFT_MAD_YX | TFT_MAD_BGR);
#else
      writedata(TFT_MAD_MX | TFT_MAD_BGR);
#endif
      break;
    case 1:
#ifdef M5STACK
      writedata(TFT_MAD_BGR);
#else
      writedata(TFT_MAD_YX | TFT_MAD_BGR);
#endif
      break;
    case 2:
#ifdef M5STACK
      writedata(TFT_MAD_YX | TFT_MAD_MX | TFT_MAD_BGR);
#else
      writedata(TFT_MAD_MY | TFT_MAD_BGR);
#endif
      break;
    case 3:
#ifdef M5STACK
      writedata(TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_BGR);
#else
      writedata(TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_YX | TFT_MAD_BGR);
#endif
      break;
  // These next rotations are for bottom up BMP drawing
    case 4:
#ifdef M5STACK
      writedata(TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_YX | TFT_MAD_BGR);
#else
      writedata(TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_BGR);
#endif
      break;
    case 5:
#ifdef M5STACK
      writedata(TFT_MAD_MY | TFT_MAD_BGR);
#else
      writedata(TFT_MAD_YX | TFT_MAD_MX | TFT_MAD_BGR);
#endif
      break;
    case 6:
#ifdef M5STACK
      writedata(TFT_MAD_YX | TFT_MAD_BGR);
#else
      writedata(TFT_MAD_BGR);
#endif
      break;
    case 7:
#ifdef M5STACK
      writedata(TFT_MAD_MX | TFT_MAD_BGR);
#else
      writedata(TFT_MAD_MY | TFT_MAD_YX | TFT_MAD_BGR);
#endif
      break;

  }

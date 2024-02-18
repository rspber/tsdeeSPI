  // This is the command sequence that rotates the HX8357D driver coordinate frame

  writecommand(TFT_MADCTL);
  rotation = m % 4;
  switch (rotation) {
   case 0: // Portrait
     writedata(TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_COLOR_ORDER);
     break;
   case 1: // Landscape (Portrait + 90)
     writedata(TFT_MAD_MV | TFT_MAD_MY | TFT_MAD_COLOR_ORDER);
     break;
   case 2: // Inverter portrait
     writedata(TFT_MAD_COLOR_ORDER);
     break;
   case 3: // Inverted landscape
     writedata(TFT_MAD_MX | TFT_MAD_MV | TFT_MAD_COLOR_ORDER);
     break;
  }

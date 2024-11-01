  // This is the command sequence that rotates the HX8357D driver coordinate frame

  writecommand(TFT_MADCTL);
  rotation = m % 4;
  switch (rotation) {
   case 0: // Portrait
     writedata(TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_RGB);
     break;
   case 1: // Landscape (Portrait + 90)
     writedata(TFT_MAD_YX | TFT_MAD_MY | TFT_MAD_RGB);
     break;
   case 2: // Inverter portrait
     writedata(TFT_MAD_RGB);
     break;
   case 3: // Inverted landscape
     writedata(TFT_MAD_MX | TFT_MAD_YX | TFT_MAD_RGB);
     break;
  }

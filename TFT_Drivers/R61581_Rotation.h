  // This is the command sequence that rotates the R61581 driver coordinate frame

  writecommand(TFT_MADCTL);
  rotation = m % 4;
  switch (rotation) {
   case 0: // Portrait
     writedata(TFT_MAD_BGR | TFT_MAD_MX);
     break;
   case 1: // Landscape (Portrait + 90)
     writedata(TFT_MAD_MV | TFT_MAD_BGR);
     break;
   case 2: // Inverter portrait
     writedata(TFT_MAD_BGR | TFT_MAD_GS);
     break;
   case 3: // Inverted landscape
     writedata(TFT_MAD_MV | TFT_MAD_BGR | TFT_MAD_MX | TFT_MAD_GS);
     break;
  }
   

  // This is the command sequence that rotates the HX8357C driver coordinate frame

  writecommand(TFT_MADCTL);
  rotation = m % 8;
  switch (rotation) {
   case 0: // Portrait
     writedata(TFT_MAD_COLOR_ORDER | TFT_MAD_MX);
     break;
   case 1: // Landscape (Portrait + 90)
     writedata(TFT_MAD_COLOR_ORDER | TFT_MAD_MV);
     break;
   case 2: // Inverter portrait
     writedata( TFT_MAD_COLOR_ORDER | TFT_MAD_MY);
    break;
   case 3: // Inverted landscape
     writedata(TFT_MAD_COLOR_ORDER | TFT_MAD_MV | TFT_MAD_MX | TFT_MAD_MY);
     break;
   case 4: // Portrait
     writedata(TFT_MAD_COLOR_ORDER | TFT_MAD_MX | TFT_MAD_MY);
     break;
   case 5: // Landscape (Portrait + 90)
     writedata(TFT_MAD_COLOR_ORDER | TFT_MAD_MV | TFT_MAD_MX);
     break;
   case 6: // Inverter portrait
     writedata( TFT_MAD_COLOR_ORDER);
     break;
   case 7: // Inverted landscape
     writedata(TFT_MAD_COLOR_ORDER | TFT_MAD_MV | TFT_MAD_MY);
     break;
  }
  

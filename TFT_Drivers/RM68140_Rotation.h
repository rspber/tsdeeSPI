  // This is the command sequence that rotates the RM68140 driver coordinate frame


  writecommand(TFT_MADCTL);
  rotation = m % 4;
  switch (rotation) {
   case 0: // Portrait
     writedata(TFT_MAD_BGR);
     writecommand(0xB6);
     writedata(0);
     writedata(0x22);
     writedata(0x3B);
     break;
   case 1: // Landscape (Portrait + 90)
     writedata(TFT_MAD_MV | TFT_MAD_BGR);
     writecommand(0xB6);
     writedata(0);
     writedata(0x02);
     writedata(0x3B);
     break;
   case 2: // Inverter portrait
     writedata(TFT_MAD_BGR);
     writecommand(0xB6);
     writedata(0);
     writedata(0x42);
     writedata(0x3B);
     break;
   case 3: // Inverted landscape
     writedata(TFT_MAD_MV | TFT_MAD_BGR);
     writecommand(0xB6);
     writedata(0);
     writedata(0x62);
     writedata(0x3B);
     break;
  }
   

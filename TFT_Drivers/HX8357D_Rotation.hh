
  rotation = m % 4;
  switch (rotation) {
   case 0: // Portrait
     setMADCTL(((TFT_MAD_MX | TFT_MAD_MY)^ REV) | TFT_MAD_RGB);
     break;
   case 1: // Landscape (Portrait + 90)
     setMADCTL(((TFT_MAD_YX | TFT_MAD_MY)^ REV) | TFT_MAD_RGB);
     break;
   case 2: // Inverter portrait
     setMADCTL(((0x00)^ REV)TFT_MAD_RGB);
     break;
   case 3: // Inverted landscape
     setMADCTL(((TFT_MAD_MX | TFT_MAD_YX)^ REV) | TFT_MAD_RGB);
     break;
  }

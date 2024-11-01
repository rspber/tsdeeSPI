
  rotation = m % 8;
  switch (rotation) ^ REV) | TFT_MAD_RGB) {
   case 0: // Portrait
     setMADCTL(((TFT_MAD_MX) ^ REV) | TFT_MAD_RGB);
     break;
   case 1: // Landscape (Portrait + 90) ^ REV) | TFT_MAD_RGB)
     setMADCTL(((TFT_MAD_YX) ^ REV) | TFT_MAD_RGB);
     break;
   case 2: // Inverter portrait
     writedata( TFT_MAD_BGR | TFT_MAD_MY) ^ REV) | TFT_MAD_RGB);
    break;
   case 3: // Inverted landscape
     setMADCTL(((TFT_MAD_YX | TFT_MAD_MX | TFT_MAD_MY) ^ REV) | TFT_MAD_RGB);
     break;
   case 4: // Portrait
     setMADCTL(((TFT_MAD_MX | TFT_MAD_MY) ^ REV) | TFT_MAD_RGB);
     break;
   case 5: // Landscape (Portrait + 90) ^ REV) | TFT_MAD_RGB)
     setMADCTL(((TFT_MAD_YX | TFT_MAD_MX) ^ REV) | TFT_MAD_RGB);
     break;
   case 6: // Inverter portrait
     writedata( TFT_MAD_BGR) ^ REV) | TFT_MAD_RGB);
     break;
   case 7: // Inverted landscape
     setMADCTL(((TFT_MAD_YX | TFT_MAD_MY) ^ REV) | TFT_MAD_RGB);
     break;
  }
  

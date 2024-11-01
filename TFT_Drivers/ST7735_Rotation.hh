
  rotation = m % 4; // Limit the range of values to 0-3

if (tabcolor == INITR_BLACKTAB) {

  switch (rotation) {
    case 0:
       setMADCTL(((TFT_MAD_MX | TFT_MAD_MY) ^ REV) | TFT_MAD_RGB);
      break;
    case 1:
       setMADCTL(((TFT_MAD_MY | TFT_MAD_YX) ^ REV) | TFT_MAD_RGB);
      break;
    case 2:
       setMADCTL(((0x00) ^ REV) | TFT_MAD_RGB);
      break;
    case 3:
       setMADCTL(((TFT_MAD_MX | TFT_MAD_YX) ^ REV) | TFT_MAD_RGB);
      break;
  }

}
else if (tabcolor == INITR_GREENTAB2) {

  switch (rotation) {
    case 0:
       setMADCTL(((TFT_MAD_MX | TFT_MAD_MY) ^ REV) | TFT_MAD_RGB);
       colstart = 2;
       rowstart = 1;
      break;
    case 1:
       setMADCTL(((TFT_MAD_MY | TFT_MAD_YX) ^ REV) | TFT_MAD_RGB);
       colstart = 1;
       rowstart = 2;
      break;
    case 2:
       setMADCTL(((0x00) ^ REV) | TFT_MAD_RGB);
       colstart = 2;
       rowstart = 1;
      break;
    case 3:
       setMADCTL(((TFT_MAD_MX | TFT_MAD_YX) ^ REV) | TFT_MAD_RGB);
       colstart = 1;
       rowstart = 2;
      break;
  }

}
else if (tabcolor == INITR_GREENTAB3) {

  switch (rotation) {
    case 0:
       setMADCTL(((TFT_MAD_MX | TFT_MAD_MY) ^ REV) | TFT_MAD_RGB);
       colstart = 2;
       rowstart = 3;
      break;
    case 1:
       setMADCTL(((TFT_MAD_MY | TFT_MAD_YX) ^ REV) | TFT_MAD_RGB);
       colstart = 3;
       rowstart = 2;
      break;
    case 2:
       setMADCTL(((0x00) ^ REV) | TFT_MAD_RGB);
       colstart = 2;
       rowstart = 1;
      break;
    case 3:
       setMADCTL(((TFT_MAD_MX | TFT_MAD_YX) ^ REV) | TFT_MAD_RGB);
       colstart = 1;
       rowstart = 2;
      break;
  }

}
else if (tabcolor == INITR_GREENTAB128) {

  switch (rotation) {
    case 0:
       setMADCTL(((TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_MH) ^ REV) | TFT_MAD_RGB);
       colstart = 0;
       rowstart = 32;
      break;
    case 1:
       setMADCTL(((TFT_MAD_YX | TFT_MAD_MY) ^ REV) | TFT_MAD_RGB);
       colstart = 32;
       rowstart = 0;
      break;
    case 2:
       setMADCTL(((0x00) ^ REV) | TFT_MAD_RGB);
       colstart = 0;
       rowstart = 0;
      break;
    case 3:
       setMADCTL(((TFT_MAD_MX | TFT_MAD_YX) ^ REV) | TFT_MAD_RGB);
       colstart = 0;
       rowstart = 0;
      break;
  }

}
else if (tabcolor == INITR_GREENTAB160x80) {

  switch (rotation) {
    case 0:
       setMADCTL(((TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_MH) ^ REV) | TFT_MAD_RGB);
       colstart = 26;
       rowstart = 1;
      break;
    case 1:
       setMADCTL(((TFT_MAD_YX | TFT_MAD_MY) ^ REV) | TFT_MAD_RGB);
       colstart = 1;
       rowstart = 26;
      break;
    case 2:
       setMADCTL(((0x00) ^ REV) | TFT_MAD_RGB);
       colstart = 26;
       rowstart = 1;
      break;
    case 3:
       setMADCTL(((TFT_MAD_MX | TFT_MAD_YX) ^ REV) | TFT_MAD_RGB);
       colstart = 1;
       rowstart = 26;
      break;
  }

}
else if (tabcolor == INITB) {

  switch (rotation) {
    case 0:
       setMADCTL(((TFT_MAD_MX) ^ REV) | TFT_MAD_RGB);
      break;
    case 1:
       setMADCTL(((TFT_MAD_YX | TFT_MAD_MX | TFT_MAD_MY) ^ REV) | TFT_MAD_RGB);
      break;
    case 2:
       setMADCTL(((TFT_MAD_MY) ^ REV) | TFT_MAD_RGB);
      break;
    case 3:
       setMADCTL(((TFT_MAD_YX) ^ REV) | TFT_MAD_RGB);
      break;
  }

}
else {

  switch (rotation) {
    case 0:
       setMADCTL(((TFT_MAD_MX | TFT_MAD_MY) ^ REV) | TFT_MAD_RGB);
      break;
    case 1:
       setMADCTL(((TFT_MAD_MY | TFT_MAD_YX) ^ REV) | TFT_MAD_RGB);
      break;
    case 2:
       setMADCTL(((0x00) ^ REV) | TFT_MAD_RGB);
      break;
    case 3:
       setMADCTL(((TFT_MAD_MX | TFT_MAD_YX) ^ REV) | TFT_MAD_RGB);
      break;
  }

}

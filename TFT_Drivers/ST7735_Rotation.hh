
// This is the command sequence that rotates the ST7735 driver coordinate frame

  rotation = m % 4; // Limit the range of values to 0-3

// tabcolor == INITR_BLACKTAB

  writecommand(TFT_MADCTL);
  switch (rotation) {
    case 0:
       writedata(TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_RGB);
      break;
    case 1:
       writedata(TFT_MAD_MY | TFT_MAD_YX | TFT_MAD_RGB);
      break;
    case 2:
       writedata(TFT_MAD_RGB);
      break;
    case 3:
       writedata(TFT_MAD_MX | TFT_MAD_YX | TFT_MAD_RGB);
      break;
  }


// tabcolor == INITR_GREENTAB2

  writecommand(TFT_MADCTL);
  switch (rotation) {
    case 0:
       writedata(TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_RGB);
       colstart = 2;
       rowstart = 1;
      break;
    case 1:
       writedata(TFT_MAD_MY | TFT_MAD_YX | TFT_MAD_RGB);
       colstart = 1;
       rowstart = 2;
      break;
    case 2:
       writedata(TFT_MAD_RGB);
       colstart = 2;
       rowstart = 1;
      break;
    case 3:
       writedata(TFT_MAD_MX | TFT_MAD_YX | TFT_MAD_RGB);
       colstart = 1;
       rowstart = 2;
      break;
  }

// tabcolor == INITR_GREENTAB3

    writecommand(TFT_MADCTL);
  switch (rotation) {
    case 0:
       writedata(TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_RGB);
       colstart = 2;
       rowstart = 3;
      break;
    case 1:
       writedata(TFT_MAD_MY | TFT_MAD_YX | TFT_MAD_RGB);
       colstart = 3;
       rowstart = 2;
      break;
    case 2:
       writedata(TFT_MAD_RGB);
       colstart = 2;
       rowstart = 1;
      break;
    case 3:
       writedata(TFT_MAD_MX | TFT_MAD_YX | TFT_MAD_RGB);
       colstart = 1;
       rowstart = 2;
      break;
  }


// tabcolor == INITR_GREENTAB128

    writecommand(TFT_MADCTL);
  switch (rotation) {
    case 0:
       writedata(TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_MH | TFT_MAD_RGB);
       colstart = 0;
       rowstart = 32;
      break;
    case 1:
       writedata(TFT_MAD_YX | TFT_MAD_MY | TFT_MAD_RGB);
       colstart = 32;
       rowstart = 0;
      break;
    case 2:
       writedata(TFT_MAD_RGB);
       colstart = 0;
       rowstart = 0;
      break;
    case 3:
       writedata(TFT_MAD_MX | TFT_MAD_YX | TFT_MAD_RGB);
       colstart = 0;
       rowstart = 0;
      break;
  }


// tabcolor == INITR_GREENTAB160x80

    writecommand(TFT_MADCTL);
  switch (rotation) {
    case 0:
       writedata(TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_MH | TFT_MAD_RGB);
       colstart = 26;
       rowstart = 1;
      break;
    case 1:
       writedata(TFT_MAD_YX | TFT_MAD_MY | TFT_MAD_RGB);
       colstart = 1;
       rowstart = 26;
      break;
    case 2:
       writedata(TFT_MAD_RGB);
       colstart = 26;
       rowstart = 1;
      break;
    case 3:
       writedata(TFT_MAD_MX | TFT_MAD_YX | TFT_MAD_RGB);
       colstart = 1;
       rowstart = 26;
      break;
  }



// tabcolor == INITB

    writecommand(TFT_MADCTL);
  switch (rotation) {
    case 0:
       writedata(TFT_MAD_MX | TFT_MAD_RGB);
      break;
    case 1:
       writedata(TFT_MAD_YX | TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_RGB);
      break;
    case 2:
       writedata(TFT_MAD_MY | TFT_MAD_RGB);
      break;
    case 3:
       writedata(TFT_MAD_YX | TFT_MAD_RGB);
      break;
  }


// else

    writecommand(TFT_MADCTL);
  switch (rotation) {
    case 0:
       writedata(TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_RGB);
      break;
    case 1:
       writedata(TFT_MAD_MY | TFT_MAD_YX | TFT_MAD_RGB);
      break;
    case 2:
       writedata(TFT_MAD_RGB);
      break;
    case 3:
       writedata(TFT_MAD_MX | TFT_MAD_YX | TFT_MAD_RGB);
      break;
  }


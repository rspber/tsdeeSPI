
     if (tabcolor == INITB)
     // Bcmd   Initialization commands for ST7735 screens
      {
          sendCmd(TFT_SWRESET);
          delay(50);
          sendCmd(TFT_SLPOUT);
          delay(500);
          sendCmdByte(TFT_PIXFMT, 0x05);   //     16-bit color
          delay(10);
          sendCmdData(ST7735_FRMCTR1, (uint8_t*)"\x00\x06\x03", 3);
          //  0x00,                   //     fastest refresh
          //  0x06,                   //     6 lines front porch
          //  0x03,                   //     3 lines back porch
          delay(10);
          sendCmdByte(TFT_MADCTL , 0x40 | TFT_MAD_RGB);
          sendCmdData(ST7735_DISSET5, (uint8_t*)"\x15\x02", 2);
          //  0x15,                   //     1 clk cycle nonoverlap, 2 cycle gate
                                    //     rise, 3 cycle osc equalize
          //  0x02,                   //     Fix on VTL
          sendCmdByte(ST7735_INVCTR , 0x0);    //     Line inversion
          sendCmdData(ST7735_PWCTR1 , (uint8_t*)"\x02\x70", 2);
          //  0x02,                   //     GVDD = 4.7V
          //  0x70,                   //     1.0uA
          delay(10);
          sendCmdByte(ST7735_PWCTR2, 0x05);  //     VGH = 14.7V, VGL = -7.35V
          sendCmdData(ST7735_PWCTR3, (uint8_t*)"\x01\x02", 2);
          //  0x01,                   //     Opamp current small
          //  0x02,                   //     Boost frequency
          sendCmdData(ST7735_VMCTR1, (uint8_t*)"\x3C\x38", 2);
          //  0x3C,                   //     VCOMH = 4V
          //  0x38,                   //     VCOML = -1.1V
          delay(10);
          sendCmdData(ST7735_PWCTR6, (uint8_t*)"\x11\x15", 2);
          //  0x11,
          //  0x15,
          sendCmdData(ST7735_GMCTRP1, (uint8_t*)"\x09\x16\x09\x20\x21\x1B\x13\x19\x17\x15\x1E\x2B\x04\x05\x02\x0E", 16);
          sendCmdData(ST7735_GMCTRN1, (uint8_t*)"\x0B\x14\x08\x1E\x22\x1D\x18\x1E\x1B\x1A\x24\x2B\x06\x06\x02\x0F", 16);
          delay(10);
          sendCmdData(TFT_CASET, (uint8_t*)"\x00\x02\x00\x81", 4);
          //  0x00, 0x02,             //     XSTART = 2
          //  0x00, 0x81,             //     XEND = 129
          sendCmdData(TFT_PASET, (uint8_t*)"\x00\x02\x00\x81", 4);
          //  0x00, 0x02,             //     XSTART = 1
          //  0x00, 0x81,             //     XEND = 160
          sendCmd(TFT_NORON);
          delay(10);
          sendCmd(TFT_DISPON);
          delay(500);
      }
     else
     {
     // Rcmd1   Init for 7735R, part 1 (red or green tab)

        sendCmd(TFT_SWRESET);
        delay(150);
        sendCmd(TFT_SLPOUT);
        delay(500);
        sendCmdData(ST7735_FRMCTR1, (uint8_t*)"\x01\x2C\x2D", 3);  //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
        sendCmdData(ST7735_FRMCTR2, (uint8_t*)"\x01\x2C\x2D", 3);  //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
        sendCmdData(ST7735_FRMCTR3, (uint8_t*)"\x01\x2C\x2D\x01\x2C\x2D", 6);
        //  0x01, 0x2C, 0x2D,       //     Dot inversion mode
        //  0x01, 0x2C, 0x2D,       //     Line inversion mode
        sendCmdByte(ST7735_INVCTR, 0x07);   //     No inversion
        sendCmdData(ST7735_PWCTR1, (uint8_t*)"\xA2\x02\x84", 3);
        //  0xA2,
        //  0x02,                   //     -4.6V
        //  0x84,                   //     AUTO mode
        sendCmdByte(ST7735_PWCTR2, 0xC5);  //     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
        sendCmdData(ST7735_PWCTR3, (uint8_t*)"\x0A\x00", 2);
        //  0x0A,                   //     Opamp current small
        //  0x00,                   //     Boost frequency
        sendCmdData(ST7735_PWCTR4, (uint8_t*)"\x8A\x2A", 2);  // 10: Power control, 2 args, no delay:
        //  0x8A,                   //     BCLK/2, Opamp current small & Medium low
        //  0x2A,
        sendCmdData(ST7735_PWCTR5, (uint8_t*)"\x8A\xEE", 2);
        sendCmdByte(ST7735_VMCTR1, 0x0E);
        sendCmd(TFT_INVOFF);        // 13: Don't invert display, no args, no delay
        sendCmdByte(TFT_MADCTL, 0xC0 | TFT_MAD_RGB); //     row addr/col addr, bottom to top refresh
        sendCmdByte(TFT_PIXFMT, 0x05);  //     16-bit color


       if (tabcolor == INITR_GREENTAB ||
           tabcolor == INITR_GREENTAB2 ||
           tabcolor == INITR_GREENTAB3 ||
           tabcolor == INITR_GREENTAB128 ||
           tabcolor == INITR_GREENTAB160x80 ||
           tabcolor == INITR_ROBOTLCD ||
           tabcolor == INITR_REDTAB160x80
       ) {
       // Rcmd2green     Init for 7735R, part 2 (green tab only)
          sendCmdData(TFT_CASET, (uint8_t*)"\x00\x02\x00\x81", 4);
          //  0x00, 0x02,             //     XSTART = 0
          //  0x00, 0x7F+0x02,        //     XEND = 127
          sendCmdData(TFT_PASET, (uint8_t*)"\x00\x01\x00\xA0", 4);
          //  0x00, 0x01,             //     XSTART = 0
          //  0x00, 0x9F+0x01,        //     XEND = 159
       }

       if (tabcolor == INITR_GREENTAB)
       {
         colstart = 2;
         rowstart = 1;
       }
       else if (tabcolor == INITR_GREENTAB2)
       {
         sendCmdByte(TFT_MADCTL, 0xC0 | TFT_MAD_RGB);
         colstart = 2;
         rowstart = 1;
       }
       else if (tabcolor == INITR_GREENTAB3)
       {
         colstart = 2;
         rowstart = 3;
       }
       else if (tabcolor == INITR_GREENTAB128)
       {
         colstart = 0;
         rowstart = 32;
       }
       else if (tabcolor == INITR_GREENTAB160x80)
       {
         sendCmd(TFT_INVON);
         colstart = 26;
         rowstart = 1;
       }
       else if (tabcolor == INITR_ROBOTLCD)
       {
           // Frame control init for RobotLCD, taken from https://github.com/arduino-libraries/TFT, Adafruit_ST7735.cpp l. 263, commit 61b8a7e
        // Rcmd3RobotLCD
        sendCmdData(ST7735_FRMCTR1, (uint8_t*)"\x0B\x14", 2);
        sendCmdData(ST7735_FRMCTR2, (uint8_t*)"\x0B\x14", 2);
        sendCmdData(ST7735_FRMCTR3, (uint8_t*)"\x0B\x14\x0B\x14", 4);

      }
       else if (tabcolor == INITR_REDTAB160x80)
       {
         colstart = 24;
         rowstart = 0;
       }
       else if (tabcolor == INITR_REDTAB)
       {
      // Rcmd2red Init for 7735R, part 2 (red tab only)
          sendCmdData(TFT_CASET, (uint8_t*)"\x00\x00\x00\x7F", 4);
          //  0x00, 0x00,             //     XSTART = 0
          //  0x00, 0x7F,             //     XEND = 127
          sendCmdData(TFT_PASET, (uint8_t*)"\x00\x00\x00\x9F", 4);
          //  0x00, 0x00,             //     XSTART = 0
          //  0x00, 0x9F,             //     XEND = 159
       }
       else if (tabcolor == INITR_BLACKTAB)
       {
         sendCmdByte(TFT_MADCTL, 0xC0 | TFT_MAD_RGB);
       }

      // Rcmd3  Init for 7735R, part 3 (red or green tab)
      sendCmdData(ST7735_GMCTRP1, (uint8_t*)"\x02\x1c\x07\x12\x37\x32\x29\x2d\x29\x25\x2B\x39\x00\x01\x03\x10", 16);
      sendCmdData(ST7735_GMCTRN1, (uint8_t*)"\x03\x1d\x07\x06\x2E\x2C\x29\x2D\x2E\x2E\x37\x3F\x00\x00\x02\x10", 16);
      sendCmd(TFT_NORON);
      delay(10);
      sendCmd(TFT_DISPON);
      delay(100);
    }








  sendCmdData(0xEF, 0x030x800x02
  sendCmdData(0xCF, 0x00
  writedata(0XC1);
  writedata(0X30);
  sendCmdData(0xED, 0x640x03
  writedata(0X12);
  writedata(0X81);
  sendCmdData(0xE8, 0x850x000x78
  sendCmdData(0xCB, 0x390x2C0x000x340x02
  sendCmdData(0xF7, 0x20
  sendCmdData(0xEA, 0x000x00
  sendCmdData(ILI9341_PWCTR1,     //Power control0x23   //VRH[5:0]
  sendCmdData(ILI9341_PWCTR2,     //Power control0x10   //SAP[2:0];BT[3:0]
  sendCmdData(ILI9341_VMCTR1,     //VCM control0x3e0x28
  sendCmdData(ILI9341_VMCTR2,     //VCM control20x86  //--
  sendCmdData(ILI9341_MADCTL,     // Memory Access Control
#ifdef M5STACK
  writedata(TFT_MAD_MY | TFT_MAD_MV | TFT_MAD_RGB); // Rotation 0 (portrait mode)
#else
  writedata(TFT_MAD_MX | TFT_MAD_RGB); // Rotation 0 (portrait mode)
#endif
  sendCmdData(ILI9341_PIXFMT, 0x55
  sendCmdData(ILI9341_FRMCTR1, 0x000x13 // 0x18 79Hz, 0x1B default 70Hz, 0x13 100Hz
  sendCmdData(ILI9341_DFUNCTR,     // Display Function Control0x080x820x27
  sendCmdData(0xF2,     // 3Gamma Function Disable0x00
  sendCmdData(ILI9341_GAMMASET,     //Gamma curve selected0x01
  sendCmdData(ILI9341_GMCTRP1,     //Set Gamma0x0F0x310x2B0x0C0x0E0x080x4E0xF10x370x070x100x030x0E0x090x00
  sendCmdData(ILI9341_GMCTRN1,     //Set Gamma0x000x0E0x140x030x110x070x310xC10x480x080x0F0x0C0x310x360x0F
  sendCmdData(ILI9341_SLPOUT,     //Exit Sleep
  delay(120);
  sendCmdData(ILI9341_DISPON,     //Display on




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
sendCmdData(ILI9341_PWCTR1,  //Power control0x10 //VRH[5:0]
sendCmdData(ILI9341_PWCTR2,  //Power control0x00 //SAP[2:0];BT[3:0]
sendCmdData(ILI9341_VMCTR1,  //VCM control0x300x30
sendCmdData(ILI9341_VMCTR2,  //VCM control20xB7 //--
sendCmdData(ILI9341_PIXFMT, 0x55
sendCmdData(0x36,  // Memory Access Control0x08 // Rotation 0 (portrait mode)
sendCmdData(ILI9341_FRMCTR1, 0x000x1A
sendCmdData(ILI9341_DFUNCTR,  // Display Function Control0x080x820x27
sendCmdData(0xF2,  // 3Gamma Function Disable0x00
sendCmdData(0x26,  //Gamma curve selected0x01
sendCmdData(0xE0,  //Set Gamma0x0F0x2A0x280x080x0E0x080x540xA90x430x0A0x0F0x000x000x000x00
sendCmdData(0XE1,  //Set Gamma0x000x150x170x070x110x060x2B0x560x3C0x050x100x0F0x3F0x3F0x0F
sendCmdData(0x2B, 0x000x000x010x3f
sendCmdData(0x2A, 0x000x000x000xef
sendCmdData(ILI9341_SLPOUT,  //Exit Sleep
delay(120);
sendCmdData(ILI9341_DISPON,  //Display on

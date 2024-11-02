
  rotation = m % 4; // Limit the range of values to 0-3

  switch (rotation) {
    case 0:
    {
      sendCmdData(ILI9225_DRIVER_OUTPUT_CTRL, (uint8_t*)"\x01\x1C", 2);
      uint8_t buf[] = {TFT_MAD_RGB, 0x30};
      sendCmdData(ILI9225_ENTRY_MODE, buf, 2);
      break;
    }
    case 1:
    {
      sendCmdData(ILI9225_DRIVER_OUTPUT_CTRL, (uint8_t*)"\x00\x1C", 2);
      uint8_t buf[] = {TFT_MAD_RGB, 0x38};
      sendCmdData(ILI9225_ENTRY_MODE, buf, 2);
      break;
    }
    case 2:
    {
      sendCmdData(ILI9225_DRIVER_OUTPUT_CTRL, (uint8_t*)"\x02\x1C", 2);
      uint8_t buf[] = {TFT_MAD_RGB, 0x30};
      sendCmdData(ILI9225_ENTRY_MODE, buf, 2);
      break;
    }
    case 3:
    {
      sendCmdData(ILI9225_DRIVER_OUTPUT_CTRL, (uint8_t*)"\x03\x1C", 2);
      uint8_t buf[] = {TFT_MAD_RGB, 0x38};
      sendCmdData(ILI9225_ENTRY_MODE, buf, 2);
      break;
    }
  }

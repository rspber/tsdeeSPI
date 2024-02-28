# tsdeeSPI

Change of concept.

## Notes

I've already done so much damage to the software that this one more repository shouldn't surprise or bother anyone.
I decided to deal with the TFT_eSPI library again, but this time I will use the opposite variant than the one in TFT_eSPI_light.
Namely, there the support for processors and protocols was rejected and only graphics remained, here it will be the other way around.

And why TFT_eSPI, someone may ask?
Because it is the only library I know of on the network that supports typical microprocessors in communication with TFT in such a comprehensive and optimal/fast way.
So it won't be a waste of time to work with her a bit.

First, processors and protocols support should be separated from graphics.
For this reason, the TFT_eSPI library was divided into two classes, TFT_eSPI with the interface as it was, i.e. serving everything that was
and subclass TFT_eeSPI, which contains the very essence, i.e. support for processors and protocols.

#### Mega graphics code reorganization

Now tsdeeSPI consists of:

* TFT_eSPI  :: top level class, entire TFT_eSPI interface is preserved
* TFT_CHAR  :: character drawing class
* TFT_GFX   :: basic graphics, rectangles, circles, triangles and similar
* TFT_eeSPI :: low level procedures: processors and protocols

#### Sprites finally work

* ESP32 SPI ILI9341 240x320 40Mhz

https://youtube.com/shorts/JJ1OKqgW7Co TFT_eSPI

https://youtube.com/shorts/dqPDFML9xew tsdeeSPI

* rp2040 8-bit parallel ILI9488 320x480

https://youtube.com/shorts/jzjth5kYvA4 TFT_eSPI

https://youtube.com/shorts/YY17d5BoMTI tsdeeSPI

* example https://www.youtube.com/watch?v=U4jOFLFNZBI works, but there is not enough memory in ESP32/RP2040 to maintain 320x240 background sprite.

Example https://youtube.com/shorts/d-qKtZR7m2o arrow shows it.

And what have we learned so far about sprites. It seems that the sprite feels best on a buffered screen, i.e. background. All drawings are created on the background stored in memory (with transparent effect too), and then the entire background is displayed and the flickering effect does not appear.

Although in some situations, e.g. image rotation (and _bpp = 16), the readPixel function in sprite refers directly to the parent screen, especially the physical screen, so other effects implemented in the sprite should be expected.

And, after thought, I have an idea I must check:

If we assume that the sprite moves only one pixel in any direction each time, there is no need to repaint the entire background each time. Just create a background sprite 1 pixel wider than the painted image on it, and each time recreate the fragmentary background in sprite memory and then the image on it at position (1,1, width-2, height-2) enouch, but not (0,0) and move not the image but this background on the screen.

I hear that some people are laughing because already the Yetis use this technique to disappear into the snow, but whatever, I'll write it.

And I found that there is already an example that uses this technique:
    Sprite_draw:
    Line 98:

    // Draw a blue rectangle in sprite so when we move it 1 pixel it does not leave a trail
    // on the blue screen background
    spr.drawRect(0, 0, WIDTH, HEIGHT, TFT_BLUE);

#### Sprite and DMA tested, I think these tests are enough.

The future plans was changed.

However, I decided to return to the previous strategy, namely expanding tsdesktop with additional functionalities / processors / protocols, because from my point of view it will be more useful. Adding the missing processors and protocols will take less time than processing TFT_eSPI and will lead to the final result faster. Therefore, we will leave the TFT_eSPI library alone for now.

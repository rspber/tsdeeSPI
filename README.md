# tsdeeSPI

tsdesktop on TFT_eeSPI

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

The next step will be to connect TFT_eeSPI (and maybe more) with tsdesktop by adopting all settings from tsdesktop.
The work will be done until everything stops working and is no longer useful, forks are welcome.

## Further Relations

#### The first step has been done.

Unfortunately, from a performance standpoint, this step was the most damaging, causing an estimated 1% to 5% performance penalty. Unfortunately, this cannot be avoided if you want to keep the appearance of the software in a civilized form.

This is due to the weaknesses of processors combined with the imperfections of compilers, which cannot generate optimal code for procedures located in different C++ classes. More precisely, it is about the inline clause, which has no effect in such an arrangement.

And it turns out that the processors we are dealing with here show a significant slowdown in code execution when, instead of executing simple instructions, a procedure is called (even one, and the efficiency of the entire process drops).

But there is no other option, we have to come to terms with it to go farther. The next steps will not be so drastic.

#### Mega graphics code reorganization

Now tsdeeSPI consists of:

* TFT_eSPI  :: top level class, entire TFT_eSPI interface is preserved
* TFT_CHAR  :: character drawing class
* TFT_GFX   :: basic graphics, rectangles, circles, triangles and similar
* TFT_eeSPI :: low level procedures: processors and protocols

Before the next step, all tests will be done, mainly sprites and dma.

#### Tests show that the last step has been achieved

Tests show that sprites don't work, but smooth fonts, for example, work.
Sprites don't work, but that's maybe for the better, at least I'll find out how they work when I run them.
Since it will take some time, I decided to upgrade to the current version first.
Because if it stays in this state, it may happen that I won't be able to finish it before it is automatically archived and deleted by github.

#### Sprites finally work

* ESP32 SPI ILI9341 240x320 40Mhz

https://youtube.com/shorts/JJ1OKqgW7Co TFT_eSPI

https://youtube.com/shorts/dqPDFML9xew tsdeeSPI

* rp2040 8-bit parallel ILI9488 320x480

https://youtube.com/shorts/jzjth5kYvA4 TFT_eSPI

https://youtube.com/shorts/YY17d5BoMTI tsdeeSPI

* example https://www.youtube.com/watch?v=U4jOFLFNZBI works, but there is not enough memory in ESP32/RP2040 to maintain 320x240 background sprite.

Example https://youtube.com/shorts/vQo6OsArblg arrow shows it.

Some bug in tsdeeSPI occured with background sprite width, it's with rotation I suppose. And I see it depends on _vpDatum value. Thank God it's not a Schrodingers cat type variable, which has no value until it's read.

And what have we learned so far about sprites. It seems that the sprite feels best on a buffered screen, i.e. background. All drawings are created on the background stored in memory (with transparent effect too), and then the entire background is displayed and the flickering effect does not appear.

But,...but,....but, in tsdesktop buffered screen is maintaned by GFXButton object (example canvas3) and what I did wrong is that I forgot to name it sprite. It's a joke for now. Sprite has some effects that are not implemented yet in GTXButton, but who knows in the future.

Although in some situations, e.g. image rotation (and _bpp = 16), the readPixel function in sprite refers directly to the parent screen, especially the physical screen, so other effects implemented in the sprite should be expected.

#### Sprite and DMA tested, I think these tests are enough.

Let's try to draw up a short plan, what to do next:

1. Successively reducing redundant code that was only used to achieve better library performance.

  Tests show that this has little impact and the readability of the code decreases.

2. Switching to RGB color, adding the ability to display 256K colors.

3. Share drivers definition (init and rotation) here and in tsdesktop.

  Of the classes defining rotation in tsdesktop, only the instructions will remain, as in TFT_eSPI, there will be less code and it will be included by #include. However, the code defining init and rotation will be taken from tsdesktop, because it is more concise and readable there. The following instructions will be added to TFT_eeSPI: sendCmd, sendCmdByte and sendCmdData to understand this.
  Also init for ILI9341 and ST7789 will be taken from tsdesktop because there is correct.

4. Share TFT_GFX and TSD_GFX

  drawEllipse(because there is correct) and draw...Gradient...percentage will be taken from tsdesktop.

5. Adopting setup definition from tsdesktop.

  - Adopting Touch definitions/rotation from tsdesktop.

6. Adding tsdesktop/tsdesktop support and tsdesktop examples,

  - Adding tsdesktop chars drawing

7. Adding tsdesktop protocols to support lacking rp2040 processors enchacements (reading by pio and touch by pio)

Other notes:

1. All changes will be successively applied in tsdesktop (and TFT_eSPI_light to maintain compilation in pico-sdk mainly).

2. Projects tsdesktop and TFT_eSPI_light will keep up with changes to get various aspects implemented in them, including educational ones.

3. In order to maintain 100% compatibility with the TFT_eSPI library, a branch or a separate project will be created in which only those changes that do not conflict with the currently operating TFT_eSPI library will be implemented.

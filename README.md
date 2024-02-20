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
* TFT_CHAR  :: character drawing class, and strings
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

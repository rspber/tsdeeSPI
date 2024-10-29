# tsdeeSPI

Change of concept.

## Notes

#### Mega graphics code reorganization

Now tsdeeSPI consists of:

* TFT_eSPI  :: top level class, entire TFT_eSPI interface is preserved
* TFT_CHAR  :: character drawing class
* TFT_GFX   :: basic graphics, rectangles, circles, triangles and similar
* TFT_eeSPI :: low level procedures: processors and protocols

#### Sprites and DMA tested, I think these tests are enough.

## Current situation

After multiple attempts to port all protocol support from TFT_eSPI to tsdesktop,
I have decided to continue with this repository in order to obtain a solution
that will allow to run intact examples from TFT_eSPI and tsdesktop simultaneously.<br/></br>

The examples with TFT_eSPI worked here from the beginning,
now after thorough testing it can be said that there are only problems
with the correct display of colors and some little errors with colordepth in Sprite handling.<br/></br>

Examples from tsdesktop with appropriate support will be added successively<br/></br>.

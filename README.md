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

The TFT_eSPI examples, with little breaks, worked here from the beginning.<br/></br>

Examples from tsdesktop with appropriate support will be added successively<br/></br>.

Main steps to do:
- change setups to this from tsdesktop: with touch and i2c configuration,
- color swap by use of TFT_MADCTL function instead of software color swapping, (if it's possible)
- enable color 666, for now it's rgb_t type use only,


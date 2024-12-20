# tsdeeSPI

Both: TFT_eSPI and tsdesktop examples work simultaneously, without no corrections in examples and environment.<br/><br/>


This was achieved by the following library configuration:<br/><br/>

                             -> TFT_GFX  -> TFT_CHAR  -> TFT_Print  -> TFT_eSPI

  TFT_eeSPI (processor's and protocol's handling) -> |<br/><br/>

                             -> TSD_GFX  -> TSD_CHAR  -> TSD_SCREEN  -> TFT_SCREEN


## Mega TFT_eSPI code reorganization

Now tsdeeSPI consists of:

* TFT_eSPI  :: top level class, entire TFT_eSPI interface is preserved
* TFT_Print :: string printing class
* TFT_CHAR  :: character drawing class
* TFT_GFX   :: basic graphics, rectangles, circles, triangles and similar
* TFT_eeSPI :: low level procedures: processors and protocols

## Current situation

After multiple attempts to port all protocol support from TFT_eSPI to tsdesktop,
I have decided to continue with this repository in order to obtain a solution
that will allow to run intact examples from TFT_eSPI and tsdesktop simultaneously.<br/></br/>

Main steps to do:
- change setups to this from tsdesktop: with touch and i2c configuration,
- chunks in PIO from tsdesktop,
- reading by PIO from tsdesktop,
- touch by PIO from tsdesktop,
- enable color 666, for now it's rgb_t type use only,


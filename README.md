# tsdeeSPI

tsdesktop on TFT_eeSPI

## Notes

I've already done so much damage to the software that this one more repository shouldn't surprise or bother anyone.
I decided to deal with the TFT_eSPI library again, but this time I will use the opposite variant than the one in TFT_eSPI_light.
Namely, there the support for processors and protocols was rejected and only graphics remained, here it will be the other way around.

And why TFT_eSPI, someone may ask?
Because it is the only library I know of on the network that supports typical microprocessors in communication with TFT in such a comprehensive and optimal/fast way.
So it won't be a waste of time to work with her a bit.

First, processor and protocol support should be separated from graphics.
For this reason, the TFT_eSPI library was divided into two classes, TFT_eSPI with the interface as it was, i.e. serving everything that was
and subclass TFT_eeSPI, which contains the very essence, i.e. support for processors and protocols.

The next step will be to connect TFT_eeSPI with tsdesktop graphics by adopting all settings from tsdesktop.
The work will be done until everything stops working and is no longer useful, forks are welcome.

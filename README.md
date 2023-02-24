# AMCC_KernelDriver

If you have made it here- THANK YOU!  This is the place that I am asking (begging) for help on porting a 32 bit kernel module that drives a PCI card talking to an infrared detector, to 64 bit and modern OS.  

This repo is kind of a mess, but it includes a few things:

### amcc_natasha.c (and the corresponding header file)
This is the original driver that was modeled off of an existing driver at the time (info on that is located in the LEGACY_amcc_info folder if you are curious or if it would help).  It was functional for over a decade on an old computer, and behaved appropriately.

### amcc_orig64.c (and the corresponding header file)
This is the result from an effort in 2015 (read:before my time in the lab) that was an unsuccesful implementation into a 64 bit system.  It compiles and loads into the kernel without issue, but the behavior is incorrect, and does not read or behave appropriately with the software, called pydsp.

## Folders

### 2015
This is all of the information from the kernel driver from the failed attempt in 2015.  I am not sure if it will be useful or not.  The amcc.c and amcc.h in this folder should be identical to the amcc_orig64.c *.h in the parent.

### Legacy_amcc_info
This is a driver compiled from a once popular chip,the AMCC S5933, similar to the one we are using, which is the AMCC S5920.  

### py3dsp
This is a python program that we use to acquire the images for our project.  The operations that interact with the character file generated from the kernel module is located in amcc.py, which (theoretically) loads a clocking program in the sload.py.  Honestly, this entire code is an absolute mess, and I would rewrite it all if I knew what I was doing.  

For those interested, the original name for this driver was ociw (that company has long since disappeared).  The (nonfunctional) driver is located in the ociwpci-64bit folder, and is named amcc.c with the amcc.h header!  

# What I am hoping for

Someone to help me identify things in the amcc_natasha.c file that are not 64 bit safe, and maybe kernel functions that have changed (like the IRQ_DISABLED function, for example, which I think can just be replaced with either a 0 or IRQF_NO_THREAD?)

ANYTHING helps.  I super appreciate any input!  
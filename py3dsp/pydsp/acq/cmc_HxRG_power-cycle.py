#
# Version:                                                              
#      $Id: hawaii.bias,v 1.3 2003/12/01 22:48:52 drew Exp $
#                                                                           
# Revisions:                                         
#      $Log: hawaii.bias,v $
#      Revision 1.3  2003/12/01 22:48:52  drew
#      updated voltages
#
#      Revision 1.2  2003/07/16 00:14:24  dsp
#      got all the names, and the voltages too.
#
# 
# this file sets up the dacs to the number of millivolts specified here.
# it does it in the order that the voltages are found in the file.
# upon reset, all of the voltages are zero.

# Turn off all voltages.  Recall that any clock which has a "bar" is off
# in the high state.

from run import rd

rd.vdd      =0 
rd.vdda     =0
rd.celdrain =0
rd.drain    =0
rd.bias5    =0
rd.heater   =0
rd.bias7    =0
rd.vbpower  =0
rd.vbgate   =0
rd.vload    =0
rd.voffset  =0 
rd._vreset  =0
rd.dsub     =0
rd.bias14   =0
rd.bias15   =0

# clock rails..
rd.fsyncblo =3300 
rd.fsyncbhi =3300
rd.csb_lo   =3300
rd.csb_hi   =3300
rd.heaterlo =0
rd.heaterhi =0
rd.clock4lo =0
rd.clock4hi =0
rd.lsyncblo =3300
rd.lsyncbhi =3300
rd.hclk_lo  =0
rd.hclk_hi  =0
rd.mainrslo =3300
rd.mainrshi =3300
rd.clock12l =0
rd.clock12h =0
rd.vclk_lo  =0
rd.vclk_hi  =0
rd.resetnlo =0
rd.resetnhi =0
rd.readenlo =0
rd.readenhi =0
rd.clock20l =0
rd.clock20h =0
rd.clock21l =0
rd.clock21h =0
rd.clock22l =0
rd.clock22h =0
rd.clock23l =0
rd.clock23h =0
rd.clock24l =0
rd.clock24h =0



rd.vdd      =3300 
rd.vdda     =3300
rd.celdrain =000
rd.drain    =000
rd.bias5    =0
rd.heater   =0
rd.bias7    =0
rd.vbpower  =3300
rd.vbgate   =2400
rd.vload    =3300
rd.voffset  =-2200 
rd._vreset  =100
rd.dsub     =100
rd.bias14   =0
rd.bias15   =0

# clock rails..
rd.fsyncblo =0 
rd.fsyncbhi =3300
rd.csb_lo   =0
rd.csb_hi   =3300
rd.heaterlo =0
rd.heaterhi =0
rd.clock4lo =0
rd.clock4hi =0
rd.lsyncblo =0
rd.lsyncbhi =3300
rd.hclk_lo  =0
rd.hclk_hi  =3300
rd.mainrslo =0
rd.mainrshi =3300
rd.clock12l =0
rd.clock12h =0
rd.vclk_lo  =0
rd.vclk_hi  =3300
rd.resetnlo =0
rd.resetnhi =3300
rd.readenlo =0
rd.readenhi =3300
rd.clock20l =0
rd.clock20h =0
rd.clock21l =0
rd.clock21h =0
rd.clock22l =0
rd.clock22h =0
rd.clock23l =0
rd.clock23h =0
rd.clock24l =0
rd.clock24h =0


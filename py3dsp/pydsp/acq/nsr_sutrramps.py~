from run import rd
import time
import filterBase

rd.object = "SUTR200_40K_250mV_blackbody"

#filterBase.set(("cds"))
rd.nsamp = 1
rd.itime=11000

'''
print "Taking garbage data!"
for garbage in range(20):
	sscan()

rd.nsamp = 20
rd.itime = 11000

print "Taking SUTR Ramps w/ CVF-K!"
for color in range(1900, 2700+1, 100):
	print "Taking " + str(color) + " nm."
	filterBase.set(("cvfK", color))
	sutr()
'''

rd.nsamp = 200
rd.itime = 11000
filterBase.set(("m'"))
#pedrun() #just to put a file in between the ramps!
print "Taking SUTR Ramps w/ CVF-L!"
'''
for color in range(2400, 4500+1, 100):
	print "Taking " + str(color) + " nm."
	filterBase.set(("cvfL", color))
	sutr()
'''
sutr()

rd.itime=11000
rd.nsamp = 1
filterBase.set(("cds"))
crun()

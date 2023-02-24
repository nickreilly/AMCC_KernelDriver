from run import rd
import time
import filterBase

rd.object ='Signalperframe_57K_250mV'
filterBase.set("cds")
rd.itime=11000
rd.nrow=2048
rd.nrowskip=0
rd.nsamp=1
print('Sleeping for 10 minutes')
time.sleep(600)

for i in range(0,3):
	rd.itime=(2**i)*11000
	rd.nsamp=int(200/(2**i))	
	crun()
	time.sleep(600)
	burst()
	time.sleep(5)
	burst()
	time.sleep(5)
	for j in range(0,4):
		sutr()

rd.nsamp=1
rd.itime=11000
print('Done taking your data!')
crun()

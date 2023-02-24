"""
This script will record temp and pressure without running PyDSP.  
"""

#from run import rd
#from pydsp import cloop
#from numpy import *
#import xdir
#import pyfits
import time
import sys
sys.path.insert(1, '/home/dsp/dsp/pydsp/')
import kjl375
import ls332
import select


#print 'What is the detector?  (H2RG-20909 for example- Should be dir after /data/)'
#det_dir = raw_input()
det_dir = 'H2RG-21024'

#print 'What would you like the timeout to be?  3s is generally good'
#timeout = int(raw_input())
timeout = 3

filename = det_dir + 'TempPress.txt'
filename_string = '/data/'+det_dir+'/'+filename

print 'Recording Temperature and Pressure to '+filename_string


try: 
	if lakeshore332:
		print 'Temp Controller already Connected'
except:
	lakeshore332 = ls332.openTempSensor()

ls332.readTemp()

try: 
	if pgauge:
		print 'Pressure gauge already connected'
except:
	ports = kjl375.serial_ports()
	pgauge = kjl375.open_pgauge(ports[0])

time_first = time.time()
wfile = open(filename_string, "a")
wfile.write("Seconds,Pressure,Temperature\n")
wfile.close()

print "Recording Pressure and Temperature"
CurrTemp = ls332.readTemp()
time.sleep(1)
new_time = time.time()


while True:
    try:
	    new_time = time.time()
	    duration_post = new_time - time_first
	    current_temp = ls332.readTemp()
	    current_tempB = ls332.readTempB()
	    current_pressure = kjl375.read_pressure()
	    wfile = open(filename_string,"a")
	    wfile.write("%s, " % new_time)
	    wfile.write("%s, " % current_pressure)
	    wfile.write("%s," % current_temp)
	    wfile.write("%s" % current_tempB)
	    wfile.write("\n")
	    wfile.close()
	    print'It has been ' + str(int(duration_post)) + ' seconds since start. T='+str(current_temp) + 'K. Lab temp='+str(current_tempB) + 'K. P=' + str(current_pressure) + 'mTorr.  Press enter to stop'
	    #timeout = 5		
	    stop_var, _, _ = select.select([sys.stdin],[],[], timeout)
	    if stop_var:
		    break		
    except ValueError:
        print('Some Exception prevented this.  Continuing!')

print "Stopped recording pressure and temperatures."

wfile.close()

ls332.closeTempSensor()
kjl375.close_pgauge()


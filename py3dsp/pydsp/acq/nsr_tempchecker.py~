from run import rd
import time
import filterBase
import ls332
import kjl375
import serial 
import sys
import glob
import csv
import xdir


def ask_inputs():
	print 'What do you want to name the object?'
	object_name = raw_input()

	print 'What is the time resolution you would like at first?  (enter in s)'
	early_itime = int(raw_input())

	print 'How long would you like to read out at this itime?  (enter in s)'
	phase_change = int(raw_input())

	print 'What is the long term time resolution you would like?  (enter in s)'
	late_itime = int(raw_input())

	print 'Would you like to save one row images for the rest of the header info? (y or n)'
	save_im = raw_input()
	acceptable_inputs = ['y', 'n']
	while save_im not in acceptable_inputs:
		print 'Incorrect input- use lower case y or n'
		save_im = raw_input()

	print 'Going down or coming up in temperature? (type: "down" or "up")'
	acceptable_inputs = ['down', 'up']
	up_down = raw_input()
	while up_down not in acceptable_inputs:
		print 'Incorrect input- type "up" or "down'
		up_down = raw_input()

	print 'What temp do you want to stop at?'
	stop_temp = float(raw_input())
	
	return object_name, early_itime, phase_change, late_itime, save_im, up_down, stop_temp


object_name, early_itime, phase_change, late_itime, save_im, up_down, stop_temp = ask_inputs()

rd.object = object_name
filterBase.set("cds")

rd.nrow=1
rd.nrowskip=1024
rd.nsamp=1
dd.dsub= 100
rd.itime = 100

#Create the csv file for later analysis
wfile = open(xdir.get_objpath() + "/"+rd['object']+".txt", "a")
wfile.write("Seconds,Pressure,Temperature\n")

ports = kjl375.serial_ports()
kjl375.open_pgauge(ports[0])

tmps()
current_temp = ls332.readTemp()
time_start = time.time()

if up_down == 'down':
	while current_temp > stop_temp:
		tmps()
		new_time = round(time.time() - time_start, 3)
		if new_time < phase_change:
			#rd.itime = early_itime
			time.sleep(early_itime)
		else:
			#rd.itime = late_itime
			time.sleep(late_itime)
		print 'It has been ' + str(int(new_time)) + ' seconds'
		current_temp = ls332.readTemp()
	
		current_pressure = kjl375.read_pressure()
		if current_pressure == '':
			current_pressure = 'nan'
			print 'Could not get pressure'	
		print 'Current pressure is ' + str(current_pressure) + ' mTorr'
		wfile.write("%s, " % new_time)
		wfile.write("%s, " % current_pressure)
		wfile.write("%s" % current_temp)
		wfile.write("\n")

		if save_im == 'y':
			rd.gc = 'Current Pressure is:'
			rd.lc = current_pressure	
			srun()
if up_down == 'up':
	while current_temp < stop_temp:
		tmps()
		new_time = round(time.time() - time_start, 3)
		if new_time < phase_change:
			#rd.itime = early_itime
			time.sleep(early_itime)
		else:
			#rd.itime = late_itime
			time.sleep(late_itime)
		print 'It has been ' + str(int(new_time)) + ' seconds'
		current_temp = ls332.readTemp()
	
		current_pressure = kjl375.read_pressure()
		if current_pressure == '':
			current_pressure = 'nan'
			print 'Could not get pressure'	
		print 'Current pressure is ' + str(current_pressure) + ' mTorr'
		wfile.write("%s, " % new_time)
		wfile.write("%s, " % current_pressure)
		wfile.write("%s" % current_temp)
		wfile.write("\n")

		if save_im == 'y':
			rd.gc = 'Current Pressure is:'
			rd.lc = current_pressure	
			srun()



wfile.close()
kjl375.close_pgauge()


rd.nsamp=1
rd.itime=11000
dd.dsub = 100
nrow = 2048
nrowskip = 0
print('Done taking your data!')
#crun()

from run import rd
import numpy
import time
import xdir
import ls332


'''
The purpose of this code is to test the responce to a temperature modulation that was design to mimic the expected temperature modulation in NEOCam (+-50 mK).

# First we need to ask the user if the setpoint has been set or not
print 'Is the current temp at the desired setpoint? (y/n)'
issetpointset = raw_input()

if issetpointset == 'n':
    print 'What temperature do you want as the setpoint? (give as an integer)'
    setpt = input()
    ls332.setSetpointTemp(setpt)
    #finish later
elif issetpointset == 'y':
'''

print 'What temp do you want start at?'
desired_temp = float(raw_input()) 
#desired_temp = 57.0
newsetpt = desired_temp
setpt=desired_temp

print 'How long do you want to wait for temperature stability before starting?'
print '    (enter in seconds)'
waittime = int(raw_input()) 

print 'How many throw away images do you want to take?'
print 'Try ~50'
throwaway = int(raw_input())

print 'What do you want to add to your object?'
print 'Use "_afterfix3", Greg!'# (empty should be okay)'
objectadd = str(raw_input())

time.sleep(waittime)
burst()
time.sleep(5)
burst()
time.sleep(5)
burst()

'''
#This is the setpoint that will be used in method 1, and to reset the setpoint after methods
setpt = desired_temp
#This setpoint will continuously change during method 2, but we need to initialize it to the temperature about which the temp will modulate


'''
# Method 1 -
#    Increase the temperature to 50 mK above starting temperature, wait for a specific number of samples, then check the actual temperature. This will tell us if our sampling rate is too long or too short.
'''
rd.object = 'Temp_Modulation' + objectadd + '_Method1' 
rd.gc = 'Method 1.'
numcycles = 40
# Here we take a few images as the baseline before temperature starts change

for garbage in numpy.arange(throwaway):
    sscan()

for whatevs in numpy.arange(64):
    srun()

# We will now increase the temperature by 50 mK above the setpoint

# Open a file, using "a" = append mode
#m1file = open(xdir.get_objpath() + "/"+"Method_1.txt", "a")

#start = time.time()
ls332.setSetpointTemp(setpt + 0.05)
time.sleep(.2)
current_temp = ls332.readTemp()
#current_time = time.time() - start

print 'Raising temperature by 50mK'
while current_temp <= (setpt + 0.05):
    #m1file.write("%f\t%f \n" % (current_time, current_temp))
    srun()
    current_temp = ls332.readTemp()
    #current_time = time.time() - start



for cycle in numpy.arange(numcycles):
    print 'Cycle %s out of %s' % (str(cycle+1), str(numcycles))
    time.sleep(.2)
    ls332.setSetpointTemp(setpt - 0.05)
    time.sleep(.2)
    #current_temp = ls332.readTemp()
    #current_time = time.time() - start
    print 'Lowering temp by 100 mK'
    while current_temp >= (setpt - 0.05):
        #m1file.write("%f\t%f \n" % (current_time, current_temp))
        srun()
        current_temp = ls332.readTemp()
        #current_time = time.time() - start
    time.sleep(.2)
    print 'Raising temp by 100 mK'
    ls332.setSetpointTemp(setpt + 0.05)
    time.sleep(.2)
    while current_temp <= (setpt + 0.05):
        srun()
        current_temp = ls332.readTemp()

#m1file.close()
time.sleep(.2)
ls332.setSetpointTemp(setpt)
print 'Done taking Method 1'


#crun()
time.sleep(600)
#burst()
#time.sleep(5)
#burst()
#time.sleep(5)

#'''

# Method 2 -
#    Change the temperature by +-2 mK after taking 2 cds images until we reach the +-50 mK amplitudes from starting temperature. 
rd.object = 'Temp_Modulation' + objectadd + '_Method2'
rd.gc = 'Method 2.'

numcycles = 20

for garbage in numpy.arange(throwaway):
    sscan()

# Here we take a few images as the baseline before temperature starts change
for whatevs in numpy.arange(64):
    srun()


print 'Raising temperature by 2mK increments until we reach 50mK'
for whatevs in numpy.arange(25):
    time.sleep(.2)
    ls332.setSetpointTemp(newsetpt + 0.002)
    time.sleep(.2)
    srun()
    srun()
    newsetpt += 0.002
time.sleep(30)


for cycle in numpy.arange(numcycles):
    print 'Cycle %s out of %s' % (str(cycle+1), str(numcycles))
    print 'Lowering temperature'
    for decrease in numpy.arange(50):
        time.sleep(.2)
        ls332.setSetpointTemp(newsetpt - 0.002)
        time.sleep(.2)
        srun()
        srun()
        newsetpt -= 0.002
    time.sleep(30)
    print 'Raising temperature'
    for increase in numpy.arange(50):
        time.sleep(.2)
        ls332.setSetpointTemp(newsetpt + 0.002)
        time.sleep(.2)
        srun()
        srun()
        newsetpt += 0.002
    time.sleep(30)

time.sleep(.2)
ls332.setSetpointTemp(setpt)
#'''
print 'Finished taking your data'
crun()

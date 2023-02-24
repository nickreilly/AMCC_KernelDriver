# This program moves the filter wheel by use of the
# parallel port.

"""
IF YOU CAN'T ACCESS THE PARALLEL PORT (ERROR OUT):

First, type 'lsmod' to see if lp is loaded. 
This can be temporarily disabled using 'rmmod lp' and then 
reset the parallel port driver using 'modprobe ppdev'. 

Some permanent solutions:
1) Note: lp is built into the kernel, so you cannot just blacklist it.
A permanent solution is to remove it from the kernel, and recompile.
Since that would be too much work, try:

2) just add 'rmmod lp >& /dev/null' and 'modprobe ppdev >&/dev/null' to your .bashrc file. Doesn't work unless you are root!  Could add to startup 
script...

3) a) Edit /etc/modules and comment out lp and add ppdev
   b) Edit /etc/modprobe.d/blacklist and add "blacklist lp"
      Also create a file /etc/modprobe.d/blacklist-lp.conf with the line
      "blacklist lp"
      Not sure about doing both, but didn't work with just the first one.
   c) run: dpkg-reconfigure linux-image-$(uname -r)
   d) edit /etc/default/cups and set LOAD_LP_MODULE=no
   e) reboot
 


 Pin Locations:
 DB9     |     Parallel Port |   (Pin description and function for parallel)
 1                (nothing)         (starts at DB0(no it doesnt?!))
 2                3                 DB1
 3                4                 DB2
 4                5                 DB3  Need 4 bits total?
 5                14                autofeed

"""

import sys, time

try: 
    import parallel
except:
    print('No Parallel module loaded.  Please install PyParallel (python-parallel).')
    parallel=None

if parallel :
    try:
        p = parallel.Parallel()
    except :
        print("access violation")
        print("This might be because the lp driver is loaded. \
          \nFirst, type 'lsmod' to see if lp is loaded. \
          \nThis can be temporarily disabled using 'rmmod lp' and then \
          \nreset the parallel port driver using 'modprobe ppdev'. \
          \nSee file dsp/pydsp/filter_wheel/FilterMove.py for full solution.")

if (p):
        print("Parallel port opened: %s" %(p.device))
else:
        print("Problem opening  \n Be sure to set the proper permissions on the parallel port. \n chmod a+rw /dev/parport0 \n OR Put dsp in lp group")

increase_bit = 5
decrease_bit = 1
backlash = 50
pulse_length = 0.005
t_step = 0.005

#provide voltage to optoisolators
p.setAutoFeed(0)  #Default = 0
# NOTE: On some computers, this needs to be 1.  On others, 0.
# This seems to be yet another, non-standard thing for parallel ports.
# IF you are getting the stepper motor only going one direction, try changing!



def newLocation(a, b): # Determines direction of shortest path and calls incease() or decrease()
   a = 4000 + a # make all values positive to simplify math
   b = 4000 + b
   c = b - a
   if (c > 2000):
      print('Moving ccw')
      d = 4000 - c
      decrease(abs(d))
   if (c <= -2000):
      print('Moving cw')
      d = 4000 + c
      increase(abs(d))
   if (c <= 2000) & (c > 0):
      print('Moving cw')
      d = c
      increase(abs(d))
   if (c > -2000) & (c < 0):
      print('Moving ccw')
      d = c
      decrease(abs(d))
   if (c == 0):
      print('No Movement')
   return

def increase(a):  #, increase_bit=increase_bit): # Steps motor in direction of increasing FWP
   p.setData(0)   #make all low 000
   for i in range(0, a):
      p.setData(5)  # increase_bit) #  - make d0 and d2 high 101
      time.sleep(0.002)
      p.setData(0)
      time.sleep(0.002)
      #print '*'
   p.setData(0)
   return

def decrease(a):#, decrease_bit=decrease_bit, increase_bit=increase_bit, backlash=backlash): # Steps motor in direction of decreasing FWP
   p.setData(0)   #make all low 000
   for i in range(0, a + 50):
      p.setData(1)   #make d0 high 001 # was 1
      time.sleep(0.002)
      p.setData(0)
      time.sleep(0.002)
      #print '-'
   time.sleep(0.01)
   # Go back 50 fw steps for gear backlash
   for i in range(0, 50):
      p.setData(5)
      time.sleep(0.002)
      p.setData(0)
      time.sleep(0.002)
   #    # print '*'
   return

def filterLoc(a, b): # Converts FWP to steps then runs newLocation()
   c = int(4 * float(a))
   d = int(4 * float(b))
   newLocation(int(c), int(d))
   print(f"New Location is: {b}")
   return b

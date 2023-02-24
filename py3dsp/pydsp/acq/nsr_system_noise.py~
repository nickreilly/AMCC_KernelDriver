from run import rd
import time
import filterBase
import ls332

burst()
time.sleep(2)
burst()

# Set directory
rd.object = 'stability-CDS'

# Set integration time and image mode
rd.itime = 11000

for i_image in range(2000):
    # Get the room temperature
    # roomtemp = ls332.readTempB()
    # Update the local comment
    rd.lc = 'We have made it %f images' % i_image
    srun()
    # Take temp again - not used, but this should stay to mimic overhead time
    #temp = ls332.readTemp()

rd.lc = ''
rd.gc = ''

crun2()

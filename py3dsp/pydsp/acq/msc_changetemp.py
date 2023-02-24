from run import rd
import time


burst()
time.sleep(6)
burst()
time.sleep(6)
burst()
setSetpointTemp(34.75) # Tell controller to change temperature
crun()  # continuously read the array, for temp stability

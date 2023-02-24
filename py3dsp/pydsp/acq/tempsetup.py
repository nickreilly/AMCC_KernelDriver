#
# tempsetup.py
#

"""
"""

TEMPDEFAULT = 30.0
import time

def tempsetup(temp) :
    if tc.carrot == temp : return
    tc.time = time.time()
    tc.goal = temp
    rd['nsamp']=1
    rd['itime']=10000
    while tc.carrot != tc.goal :
        tc.do()
        tmps()
        sscan()
    start = time.time()
    print "slew done. settling."
    while time.time() - start < 600 :
        tc.do()
        tmps()
        sscan()

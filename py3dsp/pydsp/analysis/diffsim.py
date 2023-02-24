
import numarray

class cubesim :
    """ Simulate diffusion in three dimensions.

    The simulation is divided into cells, and
    each cell contains a charge density value.
    """
    D = property(lambda x : 1.0/6.0, doc= 
        """diffusion coefficient is 1/6 boxes squared per interation.
looking at the gaussian distribution, 2*L*L = 2*D*t = sigma squared.
(its e**(-x**2/(2 sigma squared), and the distribution of charge goes as
e**(-x^2/4Dt)
we know sigma squared is equal to one-third boxes squared per iteration. 
D must be half of this.""" )

    def __init__(self, size, pixsize ) :
        "initialize. size = (depth, width, length) ]"
        self.size = size
        self.boxes_per_micron = 1.0
        self.data = numarray.zeros(shape = size, type=numarray.Float64)
        self.lastdata = numarray.zeros(shape = size, type=numarray.Float64)
        self.collected = [] # list of collection amounts over time.
        self.pixsize = pixsize

    def topcharge(self) :
        "set a uniform charge on the top layer"
        self.data[0,:,:] = 1.0 
        
    def iterate(self) :
        self.diffuse() # move charges around
        self.collect() # collect what has been collected

    def diffuse(self) :
        # first, copy over a fraction of original amount
        D  = self.D
        self.lastdata[:,:,:] = self.data[:,:,:]*(1.0-6*D)

        # diffuse up down right left forwards and backwards
        # each iteration yields a variance of 1/3 in each direction.
        # ALL of the charge leave this voxel equally for the
        # neighboring positions.
        # (other possibility would go equally to 8 corner neighbors.
        # THAT would have a variance of 1 in each axis.
        self.lastdata[:,:,:-1] += self.data[:,:,1:]*D
        self.lastdata[:,:,1:] += self.data[:,:,:-1]*D
        self.lastdata[:,:-1,:] += self.data[:,1:,:]*D
        self.lastdata[:,1:,:] += self.data[:,:-1,:]*D
        self.lastdata[:-1,:,:] += self.data[1:,:,:]*D
        self.lastdata[1:,:,:] += self.data[:-1,:,:]*D

        # handle the boundary conditions.. 
        # mirrors at all sides! diffuse in from all 6 mirrors
        self.lastdata[:,:,0] += self.data[:,:,0]*D
        self.lastdata[:,:,-1] += self.data[:,:,-1]*D
        self.lastdata[:,0,:] += self.data[:,0,:]*D
        self.lastdata[:,-1,:] += self.data[:,-1,:]*D
        self.lastdata[0,:,:] += self.data[0,:,:]*D
        self.lastdata[-1,:,:] += self.data[-1,:,:]*D
        self.data, self.lastdata = self.lastdata, self.data

    def collect(self) :
        "Go thru whereever charge is accumulated and accumulate. "
        pixsize = self.pixsize
        sum = numarray.sum
        self.collected.append(sum(sum(self.data[-1,0:pixsize,0:pixsize])))
        self.data[-1,0:pixsize,0:pixsize] = 0

    def totalcollected(self, lifetime = None, L = None) :
        """Using a simulation that collected practically everything,
        and compute total captured.
        mean carrier lifetime (in iterations) may optionally be passed in."""
        if not lifetime and not L : 
            return numarray.sum(self.collected)
        if L != None : # if diffusion length specified, use that over lifetime..
            lifetime = self.lifetime(L)
        from math import e
        sum = 0.0
        lifetime = float(lifetime)
        for i in range(len(self.collected)) :
            sum += self.collected[i]*e**(-i/lifetime)
        return sum

    def diffusionlength(self, lifetime) :
        """Given the known rate of diffusion from the simulation process
        and the assumed carrier lifetime, return the standard deviation of a
        carrier for that diffusion time, single axis.
        sigma squared is 1/3 boxes squared per iteration.
        one iteration moves charge to a mean distance of 1 box away.
        should go as root N after that.
        """
        D = self.D
        t = lifetime
        L = (D*t)**0.5 # L in boxes.
        L /= self.boxes_per_micron  # convert from boxes to microns
        return L 

    def lifetime(self, L) :
        """compute lifetime in iterations that corresponds with a diffusion length L
        expressed in microns"""
        L *= self.boxes_per_micron # convert from microns to boxes. 
        t = L**2 / self.D # apply mathematics.
        return t # 

    def uncollected(self) :
        return numarray.sum(self.data.flat)

def pointdiff(depth, spacing, pixwidth) :
    """Simulate diffusion from a point source over a pixel.
    all units are in microns.
    depth  = thickness of detector.
    pixel to pixel spacing pixel center to center.
    pixwidth is the size of a (square) pixel in microns.
    Simulation uses half micron per side cubes."""
    "C(x,y,z,t) = U/[(4piDt)^(3/2)]exp(-(x^2+y^2^z^2)/4Dt)"
    c = cubesim((depth*2,spacing,spacing),pixwidth)
    c.boxes_per_micron = 2.0
    c.data[0,0,0] = 500.0
    c.diffuse()
    c.data[0,0,0] += 500.0
    return c
        
def pointcornerdiff(depth, spacing, pixwidth) :
    """Simulate diffusion from a point source over the
    4-way gap between pixels.
    all units are in microns.
    depth  = thickness of detector.
    pixel to pixel spacing pixel center to center.
    pixwidth is the size of a (square) pixel in microns.
    Simulation uses half micron per side cubes."""
    "C(x,y,z,t) = U/[(4piDt)^(3/2)]exp(-(x^2+y^2^z^2)/4Dt)"
    c = cubesim((depth*2,spacing,spacing),pixwidth)
    c.boxes_per_micron = 2.0
    c.data[0,-1,-1] = 500.0
    c.diffuse()
    c.data[0,-1,-1] += 500.0
    return c
        
def _test() :
    "10 by 10 by 10 cube, 4 by 4 square pixels." 
    import time
    start = time.time()
    c = cubesim((10,10,10),4) 
    c.topcharge()
    print("iterating 100x")
    for i in range(100) :
        c.iterate()
    print(c.totalcollected())
    print(c.uncollected())
    assert "%5.3f"%c.totalcollected() == '5.459'
    assert "%5.3f"%c.uncollected() == '94.541'
    print("iterating 900x")
    for i in range(900) :
        c.iterate()
    assert "%5.3f"%c.totalcollected() == '73.025'
    assert "%5.3f"%c.uncollected() == '26.975'
    print(c.totalcollected())
    print(c.uncollected())
    print("total time: %5.3f"%(time.time()- start))
    print(len(c.data.flat)*6*len(c.collected), "multiplies and adds")
    
if __name__ == "__main__" :
    _test()

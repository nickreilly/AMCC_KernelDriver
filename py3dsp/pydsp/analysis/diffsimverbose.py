
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

    
"""
sample output :

>>> from diffsim import *
>>> centered = pointdiff( depth = 10, spacing = 36, pixwidth=8)
>>> centered.uncollected()
1000.0
>>> while True :
...     for i in range(1000) :
...             centered.iterate()
...     print len(centered.collected), centered.uncollected()
...     if centered.uncollected() < 3.0 :
...             break
...
 
1000 773.418389453
2000 676.363519195
3000 616.092204661
4000 565.695504956
5000 520.330421653
6000 478.794394804
7000 440.614712302
8000 405.488220753
9000 373.163927965
10000 343.416828005
11000 316.041130234
12000 290.847722413
13000 267.662626077
14000 246.325744094
15000 226.689744253
16000 208.619039588
17000 191.988851653
18000 176.684348812
19000 162.599853306
20000 149.638111542
21000 137.709622552
22000 126.732019991
23000 116.629503396
24000 107.33231478
25000 98.7762569552
26000 90.9022502505
27000 83.6559245645
28000 76.9872439401
29000 70.8501610656
30000 65.2022993177
31000 60.00466015
32000 55.221353808
33000 50.8193515097
34000 46.768257382
35000 43.0400985759
36000 39.6091321149
37000 36.4516671385
38000 33.5459013169
39000 30.8717703058
40000 28.4108092017
41000 26.1460250416
42000 24.0617794666
43000 22.1436807383
44000 20.3784843645
45000 18.754001645
46000 17.2590155092
47000 15.8832030617
48000 14.6170643027
49000 13.4518565304
50000 12.3795339726
51000 11.3926922304
52000 10.484517151
53000 9.64873777556
54000 8.8795830386
55000 8.17174191831
56000 7.52032676413
57000 6.92083954739
58000 6.36914080238
59000 5.86142104333
60000 5.39417445982
61000 4.96417470915
62000 4.5684526384
63000 4.20427578241
64000 3.86912949605
65000 3.5606995906
66000 3.27685635424
67000 3.01563984635
68000 2.77524636414
>>>
>>> cornered = pointcornerdiff(depth=10,spacing=36,pixwidth=8)
>>> while True :
...     for i in range(1000) :
...             cornered.iterate()
...     print len(cornered.collected), cornered.uncollected()
...     if cornered.uncollected() < 3.0 :
...             break
...
>>> cent = pointdiff(10,36,8)
>>> for i in xrange(80000) :
...     cent.iterate()

>>>
>>> corn = pointcornerdiff(10,36,8)
>>> for i in xrange(80000) :
...     corn.iterate()
...
>>> for length in range(200) :
...     print cent.totalcollected(L=length), corn.totalcollected(L=length)
...
998.448484827 998.542173213
0.0936888505492 5.35832676763e-08
8.71243838329 0.00615228274214
37.6060604316 0.315292067557
76.8697013274 2.3210543875
117.562817683 7.82764622497
156.361473548 17.8281746904
192.659305889 32.4039853651
226.743075688 51.1013270282
259.059776895 73.2493970847
289.982543784 98.1365072828
319.765034477 125.090805389
348.556708861 153.511675997
376.431707505 182.878361317
403.415646148 212.748676368
429.50632385 242.753612134
454.688332756 272.590360517
478.942693057 302.014881676
502.252771047 330.834537821
524.607562785 358.901062446
546.003188734 386.104015596
566.443226909 412.364810835
585.938334462 437.631356295
604.505470505 461.87331968
622.166930705 485.078002206
638.949330112 507.246788243
654.882618276 528.392125489
669.999174734 548.53498375
684.333008827 567.702737989
697.919072165 585.927421953
710.792682351 603.244301429
722.989050872 619.69072022
734.542905044 635.30517661
745.488192667 650.12659294
755.857857892 664.193745716
765.683677332 677.544828161
774.996146349 690.217121233
783.824406538 702.246752849
792.196206535 713.668528249
800.137889397 724.515817336
807.674400812 734.820487186
814.829313342 744.612870041
821.624862713 753.921758839
828.081992883 762.7744238
834.220407241 771.19664482
840.058623796 779.212755459
845.614032667 786.845695131
850.902954547 794.117066812
855.940699094 801.047198153
860.741622474 807.655204339
865.319183474 813.959051408
869.685997746 819.97561907
873.8538899 825.72076228
877.833943242 831.209371043
881.636547056 836.455428059
885.271441368 841.472063963
888.747759199 846.271609999
892.07406634 850.865648031
895.258398708 855.265057888
898.308297369 859.480062039
901.23084132 863.520267662
904.032678132 867.394706179
906.720052567 871.111870351
909.298833278 874.679749026
911.7745377 878.105859672
914.152355243 881.397278785
916.437168896 884.56067031
918.63357534 887.602312174
920.745903668 890.528121049
922.778232816 893.343675464
924.734407772 896.05423735
926.618054673 898.664772142
928.432594845 901.17996752
930.181257876 903.604250883
931.867093786 905.94180564
933.492984353 908.196586411
935.061653662 910.372333189
936.575677935 912.472584562
938.037494683 914.500690041
939.449411236 916.459821565
940.813612705 918.352984242
942.132169391 920.183026369
943.40704371 921.952648802
944.64009665 923.664413691
945.833093801 925.320752663
946.987710981 926.923974451
948.105539502 928.476272043
949.188091085 929.979729364
950.236802457 931.436327521
951.253039649 932.847950665
952.238102027 934.216391466
953.193226055 935.543356248
954.119588825 936.830469803
955.018311364 938.079279903
955.890461733 939.291261529
956.737057936 940.467820844
957.559070644 941.610298919
958.357425757 942.719975233
959.133006807 943.798070964
959.886657217 944.845752075
960.61918242 945.864132223
961.331351856 946.85427549
962.02390085 947.817198956
962.697532376 948.75387512
963.352918717 949.665234178
963.990703036 950.552166176
964.611500845 951.415523025
965.215901402 952.256120421
965.804469012 953.074739635
966.377744273 953.872129219
966.936245238 954.649006608
967.480468517 955.406059631
968.010890317 956.143947946
968.527967426 956.863304388
969.03213814 957.564736252
969.523823145 958.248826497
970.003426344 958.916134891
970.471335643 959.567199096
970.927923698 960.202535691
971.373548618 960.822641143
971.808554632 961.427992724
972.233272721 962.019049388
972.64802122 962.596252595
973.053106385 963.160027093
973.44882293 963.710781665
973.835454545 964.248909835
974.213274373 964.774790534
974.582545481 965.288788743
974.943521292 965.791256093
975.296446 966.282531442
975.641554974 966.762941422
975.979075126 967.232800956
976.309225273 967.692413758
976.632216477 968.142072798
976.948252369 968.582060751
977.257529457 969.012650427
977.560237424 969.434105175
977.8565594 969.846679267
978.146672239 970.250618275
978.430746763 970.646159413
978.708948014 971.03353188
978.981435478 971.412957178
979.248363312 971.784649415
979.509880548 972.148815597
979.7661313 972.505655911
980.017254954 972.855363983
980.263386348 973.19812714
980.504655954 973.534126644
980.74119004 973.863537932
980.973110832 974.186530832
981.200536667 974.503269777
981.42358214 974.813914008
981.642358242 975.118617768
981.856972498 975.417530486
982.067529091 975.710796958
982.274128986 975.998557512
982.476870051 976.280948177
982.675847166 976.558100835
982.87115233 976.830143372
983.062874769 977.097199822
983.251101032 977.359390501
983.435915086 977.616832145
983.617398409 977.869638032
983.795630078 978.117918102
983.970686847 978.361779079
984.142643237 978.601324578
984.311571606 978.836655214
984.477542225 979.067868703
984.640623354 979.295059965
984.800881301 979.518321215
984.958380498 979.737742057
985.113183558 979.95340957
985.265351337 980.165408392
985.414942993 980.373820806
985.562016045 980.578726809
985.706626419 980.780204196
985.848828511 980.978328627
985.988675228 981.173173696
986.126218039 981.364811003
986.261507024 981.553310212
986.394590916 981.738739118
986.525517144 981.921163703
986.654331874 982.100648197
986.781080052 982.277255129
986.905805438 982.451045383
987.028550648 982.622078252
987.149357183 982.790411481
987.268265472 982.956101322
987.385314897 983.119202572
987.50054383 983.279768627
987.613989663 983.437851517
987.725688835 983.593501951
987.835676866 983.746769355
987.943988378 983.897701912
988.050657129 984.046346601
988.155716034 984.192749227
988.25919719 984.336954463
988.361131903 984.479005876
988.461550709 984.618945967
988.560483397 984.756816196
>>>

"""

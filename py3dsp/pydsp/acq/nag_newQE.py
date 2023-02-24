from run import rd
import numpy
import xdir
import pyfits
import filterBase
import time
import ls332
import dv





def get_new_mosaic_img():
  # put comment in header
  rd.gc = 'sub-array reads were mosaiced to make full image'
  rd.itime = subarrayitime
  rd.nrow = skip+10 # Set the number of rows to do a sub-array read.
  newrowstart=rowstart-skip/2
  toofar=0
  if newrowstart <0:
    newrowstart=0
  for rowskip in range(newrowstart, rowend - skip/2, skip/2):
    if rowskip + skip + 10> fullrowend:
      rd.nrow=fullrowend-rowskip
      toofar=1

    rd.nrowskip = rowskip

    sscan() # Take an image, but do not save
    sscan() # do two because we want a better reset than just once.
    # recreate file name (duplicate code from runrun)
    scanfilename = xdir.get_objpath() + "/" + rd.bufferflag + ".fits"
    scanfile = pyfits.open(scanfilename)
    # These are FITS files. Get both the data and header.
    scandata = scanfile[0].data
    imgheader = scanfile[0].header

    if rd.nrowskip == newrowstart:
        if newrowstart==0:
            newrowblock = scandata[:-10,:] # can't concat on empty.
        elif toofar==1:
            newrowblock=scandata[skip/2:,:]
        else:
            newrowblock=scandata[skip/2:-10,:]
    else:
        if toofar==1:
            oldrowblock = newrowblock
            newrowblock = numpy.concatenate((oldrowblock, scandata[skip/2:,:]), axis=0)            
        else:
            oldrowblock = newrowblock
            newrowblock = numpy.concatenate((oldrowblock, scandata[skip/2:-10,:]), axis=0)
    scanfile.close()
  image = newrowblock

  fitsobj = pyfits.HDUList() # new fitsfile object
  hdu = pyfits.PrimaryHDU()
  fitsobj.append(hdu) # and
  hdu.data = image.astype(int)
  fitsobj[0].header = imgheader
  outputfilename = xdir.get_nextobjfilename() + ".fits" # Get the filename
  rd.objnum = xdir.objnum + 1 # increase object number for next filename
  print 'Writing mosaic image to \n %s'%outputfilename
  fitsobj.writeto(outputfilename)
  #pyfits.writeto(outputfilename, newrowblock)
  # Clear comment
  rd.gc = ''
  dv.load_src(outputfilename)

  rd.itime = fullarrayitime
  rd.nrow = fullrowend
  rd.nrowskip = 0
  
  

def get_new_mosaic_img_rst():#skip=skip, subarrayitime=subarrayitime, rowstart=rowstart, rowend=rowend):
    # put comment in header
    orig_itime = rd.itime
    orig_nrow = rd.nrow

    #rowstart = 0
    #rowend = 2048
    fullrowend = 2048
    rd.gc = 'sub-array reads were mosaiced to make full image'
    rd.itime = subarrayitime
    rd.nrow = skip + 20 # Set the number of rows to do a sub-array read.
    toofar=0
    #newrowstart= rowstart #0
    new_array = numpy.zeros((int(rowend-rowstart), rd.ncol), dtype=int)
    #print(range(newrowstart, rowend, skip))
    for i_sub, rowskip in enumerate(numpy.arange(rowstart, rowend, skip)):
        if i_sub == 0:
            pix_start, pix_end = rowskip, skip + 10
            sub_start, sub_end = 0, skip + 10
            array_start = 0
        else:
            pix_start, pix_end = rowskip + 10, rowskip + skip + 10
            sub_start, sub_end = 10, skip + 10
            array_start = rowskip - 10
        if pix_end > fullrowend:
            pix_end = fullrowend
            sub_start = 20
            sub_end = skip + 10
        #print(pix_start, pix_end, str('Sub:'), sub_start, sub_end)
        if pix_start > fullrowend - 1:
            continue   
        
        temp_row, temp_skip = skip + 20, rd.nrowskip
        

        # Do quick reset to help border pixels
        fast_rows = temp_row + 20
        fast_skip = temp_skip - 10
        rd.ncol = 4
        min_itime = 75
        #print(min_itime)
        rd.itime = min_itime
        pedscan()
        pedscan()
        
        rd.nrow = skip + 20
        rd.itime = subarrayitime
        rd.ncol = 2048

        
        rd.nrowskip = array_start
        rd.nrow = temp_row
        sscan() # Take an image, but do not save
        sscan() # do two because we want a better reset than just once.
        # recreate file name (duplicate code from runrun)
        scanfilename = xdir.get_objpath() + "/" + rd.bufferflag + ".fits"
        scanfile = pyfits.open(scanfilename)
        # These are FITS files. Get both the data and header.
        scandata = scanfile[0].data.astype(int)
        imgheader = scanfile[0].header
        
        #print(new_array[pix_start:pix_end, :].shape, scandata[sub_start:sub_end, :].shape)
        
        new_array[pix_start:pix_end, :] = scandata[sub_start:sub_end, :]

        scanfile.close()
    
    image = new_array.astype(int)
    #print(image.shape)


    fitsobj = pyfits.HDUList() # new fitsfile object
    hdu = pyfits.PrimaryHDU()
    fitsobj.append(hdu) # and
    hdu.data = image
    imgheader['NAXIS1'] = 2048
    imgheader['NAXIS2'] = 2048
    fitsobj[0].header = imgheader
    outputfilename = xdir.get_nextobjfilename() + ".fits" # Get the filename
    rd.objnum = xdir.objnum + 1 # increase object number for next filename
    print 'Writing mosaic image to %s'%outputfilename
    fitsobj.writeto(outputfilename)
    
    # Clear comment
    rd.gc = ''

    rd.itime = orig_itime
    rd.nrow =  orig_nrow
    rd.nrowskip = 0

    dv.load_src(outputfilename)
    
def quick_reset():
    prev_rows = rd.nrow
    prev_rowskip = rd.nrowskip
    prev_itime = rd.itime
    prev_cols = rd.ncol
    if prev_cols != 2048:
        prev_cols = 2048
    if prev_rowskip != 0:
        rd.nrowskip = prev_rowskip - 10
    if prev_rows != 2048:
        rd.nrow = prev_rows + 20
    rd.ncol = 4
    rd.itime = 150
    pedscan()
    pedscan()
    rd.ncol = prev_cols
    rd.itime = prev_itime
    rd.nrow = prev_rows
    rd.nrowskip = prev_rowskip

print 'What bias do you want to use to measure the QE?'
detbias=int(raw_input())

print 'What are you looking at?'
print '1) Black Cloth'
print '2) Spectra'
print '3) Liquid Nitrogen'
print '4) Blackbody'
target=int(raw_input())

if target ==1:
	print 'What is the temperature of the cloth (C)?'
	clothtemp=float(raw_input())
	
	print 'What is the temperature of the room (F)?'
	roomtemp=float(raw_input())
	rd.lc = 'Cloth Temp = ' + str(clothtemp) + ' C. Room Temp = ' + str(roomtemp) + ' F'
	
if target == 2:
    print 'What filter are you using?  (nofilter, 4.8, 8.8, or window)'
    window_temp = str(raw_input())
    print('What temp is the blackbody? (enter in C)')
    bb_temp = str(raw_input())
    rd.lc = 'Blackbody Temp = ' + bb_temp + 'C'

if target == 4:
    print 'Is this a nofilter, polystyrene, or a filter?'
    window_temp = str(raw_input())
    print('What temp is the blackbody? (enter in C)')
    bb_temp = str(raw_input())
    rd.lc = 'Blackbody Temp = ' + bb_temp + 'C'
	
print 'What do you want to add on the end of the object name?'
print '(Use what CVF you are centering!)'
print '(Include underscore if you are using it!)'
object_add=str(raw_input())

print 'How long do you want to wait for temperature stability?'
waittime=int(raw_input())

time.sleep(waittime)
burst()
time.sleep(5)
burst()
time.sleep(5)
burst()


detsub = dd.vreset + detbias
dd.dsub = detsub

tmps()
CurrTemp = int(round(ls332.readTemp()))
if target==1:
	object_name = "QE_Cloth_%dK_%dmV"%(CurrTemp, detbias)
elif target==2:
    object_name = "QE_Spectra_%dK_%dmV_%sC_%s"%(CurrTemp, detbias, bb_temp, window_temp)
   
elif target==3:
	object_name = "QE_LN2_%dK_%dmV"%(CurrTemp, detbias)
	
elif target==4:
    object_name = "QE_Blackbody_%dK_%dmV_%sC_%s"%(CurrTemp, detbias, bb_temp, window_temp)

object_name_full = str(object_name) + str(object_add)
rd.object = object_name_full

rowstart=0
rowend = 2048

fullrowend=2048
fullarrayitime=11000
num_images = 2
itimes_used = []
skips_used = []

if target == 2:
    wave_step = 10
    
    
    
    #rd.nrow = 256
    #rd.nrowskip = 896
    #rd.itime = 1500
    for wavelen in range(5500, 4499, -wave_step):
	    filterBase.set(("cvfII", wavelen))
	    for i_frame in range(1):
	        srun()
	        #get_new_mosaic_img()
    
    '''
    #LHe setup
    wave_step = 10
    rd.nrow = 2048
    rd.nrowskip = 0
    rd.itime = 120000
    filterBase.set('cds')
    srun()
    for wavelen in range(2250, 1850 - 1, -wave_step):
	    filterBase.set(("cvfK", wavelen))
	    for i_frame in range(1):
	        srun()
    for wavelen in range(2250, 1150 - 1, -wave_step):
	    filterBase.set(("cvfJH", wavelen))
	    for i_frame in range(1):
	        srun()

    '''
    '''
    rowstart= 896
    rowend = 1152
    subarrayitime = 450
    skip= 64
    for wavelen in range(9500, 8049, -wave_step):itimes_used.append(subarray_itime)
	    filterBase.set(("cvfIII", wavelen))
	    for i_frame in range(1):
	        #get_new_mosaic_img_rst()
	        #get_new_mosaic_img()
	        srun()
    '''

elif target == 4:
    rd.nrowskip = 896
    rd.nrow = 256
    rd.itime = 1500
    '''
    #for wavelen in range(11000, 8500, -10):
    for wavelen in range(4500, 3200, -10):
	    filterBase.set(("cvfL", wavelen))
	    for i_frame in range(2):
	        srun()
    rd.itime = 5000
    for wavelen in range(3200, 2350, -10):
        filterBase.set(("cvfL", wavelen))
        for i_frame in range(2):
            srun()
    '''
    '''        
    rd.itime = 1500
    for wavelen in range(2750, 2500, -10):
	    filterBase.set(("cvfK", wavelen))
	    for i_frame in range(2):
	        srun()
    rd.itime = 5000
    for wavelen in range(2500, 2300, -10):
	    filterBase.set(("cvfK", wavelen)img)
	    for i_frame in range(2):
	        srun()	        
    rd.itime = 20000
    for wavelen in range(2300, 1900, -10):
	    filterBase.set(("cvfK", wavelen))
	    for i_frame in range(2):
	        srun()
    '''
    '''
    rd.itime = 4500
    for wavelen in range(2200, 1750, -10):
	    filterBase.set(("cvfJH", wavelen))
	    for i_frame in range(2):
	        srun()	
    rd.itime = 15000
    for wavelen in range(1750, 1600, -10):
	    filterBase.set(("cvfJH", wavelen))
	    for i_frame in range(2):
	        srun()
    rd.itime = 60000
    for wavelen in range(1600, 1450, -10):
	    filterBase.set(("cvfJH", wavelen))
	    for i_frame in range(2):
	        srun()    
    rd.itime = 120000
    for wavelen in range(1450, 1300, -10):
	    filterBase.set(("cvfJH", wavelen))
	    for i_frame in range(2):
	        srun()
	'''
else:
    for blah in range(10): 
	    sscan()

    #subarrayitime = 500
    subarrayitime = 6000
    skip= 1024
    
    itimes_used.append(subarrayitime)
    skips_used.append(skip)
    
    #for wavelen in range(14000, 7999, -100): #was 14000
    for wavelen in range(13000, 7999, -100):
	    filterBase.set(("cvfIII", wavelen))
	    for i_frame in range(num_images):
	        #quick_reset()
	        get_new_mosaic_img()
	        #get_new_mosaic_img_rst(skip=skip, subarrayitime=subarrayitime)
	    
	

    subarrayitime = 3000
    skip=512
    itimes_used.append(subarrayitime)
    skips_used.append(skip)
    #for wavelen in range(6300, 4150, -25): # NC1
    for wavelen in range(8000, 4150, -100):
	    filterBase.set(("cvfII", wavelen))
	    for i_frame in range(num_images):
	        #quick_reset()
	        srun()
	    
            #get_new_mosaic_img()
            #get_new_mosaic_img_rst(skip=skip, subarrayitime=subarrayitime)

    subarrayitime = 500
    skip=64
   
    filterBase.set("7.1")
    get_new_mosaic_img()
    #get_new_mosaic_img_rst(skip=skip, subarrayitime=subarrayitime)

    filterBase.set("8.8")
    get_new_mosaic_img()
    #get_new_mosaic_img_rst(skip=skip, subarrayitime=subarrayitime)

    filterBase.set("5.8")
    get_new_mosaic_img()
    #get_new_mosaic_img_rst(skip=skip, subarrayitime=subarrayitime)

    filterBase.set("8.6")
    get_new_mosaic_img()
    #get_new_mosaic_img_rst(skip=skip, subarrayitime=subarrayitime)

    filterBase.set("11.6")
    get_new_mosaic_img()
    #get_new_mosaic_img_rst(skip=skip, subarrayitime=subarrayitime)

    rd.itime=60000
    filterBase.set("3.3")
    srun()

    rd.itime=11000
    filterBase.set("l'")
    srun()

    
    filterBase.set("cds")
    for i in range(num_images):
        srun()
    rd.itime=60000
    filterBase.set("cds")
    for i in range(num_images):
        srun()
        
    for i in range(len(itimes_used)):
        subarrayitime = itimes_used[i]
        skip = skips_used[i]
        for i_num in range(num_images):
            get_new_mosaic_img()


rd.itime = 11000
rd.nrow = 2048
rd.nrowskip = 0
filterBase.set("cds")

rd.gc=''
rd.lc=''

rd.object = 'test'
print('All Done!')
crun2()




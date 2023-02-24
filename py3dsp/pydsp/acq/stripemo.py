def stripemo(skipval = 8, itime = 100, nsamp=20) :
    "take a mosaic image such that all pixels are read shortly after reset."
    skips = range(0,512,skipval)
    # Record what all these values were before so that we can put them back.
    itimereset = rd['itime']
    nsampreset = rd['nsamp']
    nrowreset = rd['nrow']
    nrowskipreset = rd['nrowskip']
    ncolreset = rd['ncol']
    ncolskipreset = rd['ncolskip']
    # Read in the values for the mosaic
    rd['ncol'] = 512
    rd['ncolskip'] = 0
    rd['itime'] = itime 
    rd['nrow'] = skipval
    for rs in skips :
        rd['nrowskip'] = rs
        rd['nsamp'] = 1
        sscan()
        sscan()
        rd['nsamp'] = nsamp
        sutr()
    # Put everything back and save it.
    rd['itime'] = itimereset
    rd['nsamp'] = nsampreset
    rd['nrow'] = nrowreset
    rd['nrowskip'] = nrowskipreset
    rd['ncol'] = ncolreset
    rd['ncolskip'] = ncolskipreset
    savesetup()

def stripevsbias() :
    for bias in testconfigbias :
        itime = 100
        rd['object']="sutrmo%dmV_%dms"%(bias,itime)
        biassetup(bias)
        stripemo()
    biassetup(BIASDEFAULT)
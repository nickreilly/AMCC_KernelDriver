def sutrmo() :
    skips = [0,128,256,384]
    for rs in skips :
        rd['nrowskip'] = rs
        rd['nsamp'] = 2
        sscan()
        sscan()
        rd['nsamp'] = 256
        for cs in skips :
            rd['ncolskip'] = cs
            sutr()
            sutr()

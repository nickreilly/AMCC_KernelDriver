import numpy as np


def closest(list_used, find_val):
    return min(enumerate(list_used), key=lambda i: abs(i[1] - find_val))[0]

def filter_list(feed_list, excluded_list):
    for temp_str in excluded_list:
        feed_list = [i for i in feed_list if temp_str.lower() not in i.lower()]
    return sorted(feed_list)


def info_from_string(temp_string, type_info):
    '''
    Possible type_infos: temp, tempbb, bias, digit, array_name, ls332_output

    '''
    import re
    temp_pattern = '\d*K'
    bb_pattern = '\d+C|(cloth)|(ln2)'
    bias_pattern = '\d+mV'
    digit_pattern = '\d+\.?\d*'
    array_pattern = '(H\dRG)-\d*'
    ls332_pattern = '[+-]?(\d*[.])?\d+'

    if type_info == 'temp':
        match_info = re.search(temp_pattern, temp_string)
    if type_info == 'tempbb':
        match_info = re.search(bb_pattern, temp_string, re.IGNORECASE)
    if type_info == 'bias':
        match_info = re.search(bias_pattern, temp_string)
    if type_info == 'digit':
        match_info = re.search(digit_pattern, temp_string, re.IGNORECASE)
    if type_info == 'array_name':
        match_info = re.search(array_pattern, temp_string, re.IGNORECASE)
    if type_info == 'ls332_output':
        match_info = re.search(ls332_pattern, temp_string, re.IGNORECASE)
    # else:
    #     print('Failed to get any good info!')
    #     return
    if match_info:
        # print('found')
        return match_info.group()


def get_filelist(directory, ignore_subdir=True, include_src=0, include_bkg=0, include_ped=0, include_txt=False):
    import glob
    """Will return a sorted list of files in directory.  This should handle left filled numbers, too.
    Updated: includes flags for src, bkg, and ped files, too.  If you want these, change the flags to 1.
    Args:
        directory ([str]): This is the directory of the data files you want returned!
    Returns:
        [list]: This will be a sorted list of all the files in your directory
    """
    if directory[-1] == '/':
        directory = directory[:-1]
    object_name = directory.split('/')[-1]
    excludes = []
    if include_src == 0:
        excludes.append('src.fits')
    if include_bkg == 0:
        excludes.append('/bkg')
    if include_ped == 0:
        excludes.append('ped.fits')
    if not include_txt:
        excludes.append('.txt')
    filelist = [i for i in glob.glob(f'{directory}/*')]
    for filt_string in excludes:
        filelist = list(filter(lambda k: filt_string not in k, filelist))
    if ignore_subdir:
        filelist = [i for i in filelist if not os.path.isdir(i)]
    filelist = sorted(filelist, key=len)
    first_few = filelist[:4 - len(excludes)]
    try:
        bulk_files = filelist[4 - len(excludes):]
        bulk_files = sorted(bulk_files, key=lambda x: (int(x.split(object_name)[-1].split('_')[-1].split('.')[0])))
        bulk_files = sorted(bulk_files, key=lambda x: len(x))
        bulk_files = sorted(bulk_files, key=lambda x: (int(x.split(object_name)[-1].split('_')[1].split('.')[0])))
        filelist = first_few + bulk_files
    except:
        # if len(filelist) == 0 :
        filelist = [i for i in glob.glob(f'{directory}/*')]
        for filt_string in excludes:
            filelist = sorted(list(filter(lambda k: filt_string not in k, filelist)))
    if len(filelist) == 0:
        print('Check your directory- filelist has nothing in it!')
    return filelist


def reference_pixel_correction(im, smoothing=True, ref_scale=0.75, sav_window=31, poly_order=3, fix_amp=False, nchan=4, fix_cols=False, col_flag=False):
    from scipy.signal import savgol_filter
    """Finally generated a good reference pixel correction scheme.
        This will smooth using a window size of [sav_window]
        and use a reference scale factor of [ref_scale]

    Args:
        im ([2D array of Floats]): [The image you want reference pixel corrected]
        Smoothing (int): Anything but 0 will smooth the reference pixel column
        ref_scale ([float]): [Typically between 0.7 and 0.8]

    Returns:
        im_ref [2D Float]: [reference pixel corrected image]
    """
    if smoothing == 1:
        smoothing = True
    fixed_anycols = 0
    im_ref = np.zeros_like(im)
    im_amp = np.zeros_like(im)
    reflist_rows = np.zeros(im.shape[0])
    reflist_cols = np.zeros(im.shape[1])
    ref_scale = float(ref_scale)
    for i_row in range(im.shape[0]):
        reflist_rows[i_row] = 0.5 * (np.mean(im[i_row, :4] + np.mean(im[i_row, -4:])))
    if smoothing:
        reflist_rows = savgol_filter(reflist_rows, sav_window, poly_order, mode='nearest')

    if fix_amp:
        if im.shape[0] % 1024 == 0: # only do this if we have a full array- will be 1024 or 2048, so % 1024 will give 0 
            fixed_anycols += 1
            refs_top = im[:4, :]
            refs_bot = im[-4:, :]
            chan_size = int(im.shape[1] / nchan)
            for i_chan in range(nchan):
                col_start = i_chan * chan_size
                col_stop = col_start + chan_size
                mean_amp_top = np.mean(refs_top[:, col_start:col_stop])
                mean_amp_bot = np.mean(refs_bot[:, col_start:col_stop])
                amp_average = np.mean((mean_amp_top, mean_amp_bot))
                # amp_average = np.mean(refs_bot[:, col_start:col_stop])
                im_amp[:, col_start:col_stop] =  amp_average

    if fix_cols:
        if im.shape[0] % 1024 == 0: # only do this if we have a full array- will be 1024 or 2048, so % 1024 will give 0 
            fixed_anycols += 1
            for i_col in range(im.shape[1]):
                reflist_cols[i_col] = 0.5 * (np.mean(im[:4, i_col] + np.mean(im[-4:, i_col])))
            if smoothing:
                reflist_cols = savgol_filter(reflist_cols, sav_window, poly_order, mode='nearest')

    if col_flag:
        if fix_cols or fix_amp:
            if fixed_anycols == 0:
                print('Did not fix any columns- is this a subarray?')
    # for i_row in range(0, im.shape[0]):
    #     for i_col in range(0, im.shape[1]):
    #         im_ref[i_row, i_col] = im[i_row, i_col] - ref_scale * (reflist_rows[i_row] + reflist_cols[i_col]) - im_amp[i_row, i_col]

    im_ref = im - (ref_scale * (reflist_rows.reshape(reflist_rows.shape[0], 1) + reflist_cols.reshape(1, reflist_cols.shape[0]))) - im_amp
    return im_ref


def gaussian_fit(smoothed, x_list, xind_max):
    """This will fit a Gaussian peak to a list.

    Args:
        smoothed ([list]): the data you want to fit
        xind_max ([float]): how far past the max of the data you want to fit (if histogram with tail, use ~1.25 or so)

    Returns:
        xvals, yvals, constants [lists]: This will be the best fit.
        Constants are in [Amplitude, mean ,deviation, intercept]
    """
    from scipy.optimize import leastsq

    def fitfunc(p, x):
        result = p[0] * np.exp((-0.5 * ((x - p[1]) / p[2])**2)) + p[3]
        return result

    def errfunc(p, x, y):
        result = (y - fitfunc(p, x))
        return result

    init = [1.0, 15, 10, 0]
    fit_max = xind_max
    out = leastsq(errfunc, init, args=(x_list[1:fit_max], smoothed[1:fit_max]))
    c = out[0]
    xvals = x_list
    yvals = fitfunc(c, xvals)
    return xvals, yvals, c


def fwhm(datas_y, datas_x):
    """Calculates the FWHM of an x,y data set

    Args:
        datas_y ([list]): the y values you want to fit
        datas_x ([list]): the x values you want to fit

    Returns:
        FWHM [float]: this is the FWHM of your dataset.
    """
    ind_low = next(x for x, val in enumerate(datas_y) if val > 0.5 * max(datas_y))
    ind_high = next(x for x, val in enumerate(datas_y) if val < 0.5 * max(datas_y) and x > ind_low)
    # print(ind_low, ind_high, datas_x[ind_low], datas_x[ind_high])
    return abs(datas_x[ind_high] - datas_x[ind_low])


def subtract_times(older_time=None, newer_time=None, dir=''):
    '''Find the time delta between all named files in directory
    Format should be yyyymmdd HH:MM:SS
    Return will be in seconds, hours
    '''
    import os
    if dir != '':
        files = get_filelist(dir)
        times = sorted([os.stat(file).st_mtime for i, file in enumerate(files)])
        elapsed_time = (times[-1] - times[0])

    else:
        from datetime import datetime
        if len(older_time.split(":")) > 1:
            older_format = datetime.strptime(older_time, "%Y%m%d %H:%M:%S")
            newer_format = datetime.strptime(newer_time, "%Y%m%d %H:%M:%S")
        else:
            older_format = datetime.strptime(older_time, "%b %d %H%M")
            newer_format = datetime.strptime(newer_time, "%b %d %H%M")
        elapsed_time = (newer_format - older_format).total_seconds()
    print(f'This time delta is {round(elapsed_time, 2)} s or {round(elapsed_time/3600, 2)} Hours')
    return


def lin_function_lsquares(variables, x, y):
    '''
    This a normal linear fit function for least squares fitting.
    variables[0] should be guess for slope
    variables [1] should be guess for intercept
    '''
    slope = variables[0]
    intercept = variables[1]
    fit_trial = slope * x + intercept
    return y - fit_trial


def quadratic_function(x, a, b, c):
    # a = variables[0]
    # b = variables[1]
    # c = variables[2]
    fit_trial = a * (x ** 2) + b * x + c
    return fit_trial


def exponential_function(x, a, b, c, x0):
    # a = variables[0]
    # b = variables[1]
    # c = variables[2]
    fit_trial = a * np.exp(-b * (x - x0)) + c
    return fit_trial


def sigmoid_fit(variables, x, y):
    a = variables[0]
    b = variables[1]
    c = variables[2]
    d = variables[3]
    diff = a / (1 + np.exp(-c * (x - d))) + b
    return y - diff


def sigmoid_function(x, a, b, c, d):
    # a = variables[0]
    # b = variables[1]
    # c = variables[2]
    # d = variables[3]
    y = a / (1 + np.exp(-c * (x - d))) + b
    return y
    

def sigmoid_function_norm(x, c, d):
    # a = variables[0]
    # b = variables[1]
    # c = variables[2]
    # d = variables[3]
    y = 1 / (1 + np.exp(-c * (x - d)))
    return y


def histogram_maxs(ys, bins, keep_plot=False):
    from scipy.signal import argrelextrema, savgol_filter
    import matplotlib.pyplot as plt

    a = plt.hist(ys, bins=bins, histtype='step')
    if len(bins) > 50:
        b = savgol_filter(a[0], 15, 2)
    else:
        b = savgol_filter(a[0], 5, 2)
    bin_vals = np.zeros(len(a[1]) - 1)
    for j in range(0, len(bin_vals)):
        bin_vals[j] = (a[1][j + 1] + a[1][j]) / 2
    maxs = [bin_vals[i] for i in argrelextrema(a[0], np.greater)[0] if a[0][i] > 2 * np.mean(a[0])]
    max_hist = (bin_vals[np.argmax(b)])
    med = np.nanmedian(ys)
    if not keep_plot:
        plt.close('all')
    return max_hist, maxs, med


def do_linear_fit(y_full, x_full=None):
    from scipy.optimize import leastsq
    if x_full is None:
        x_full = np.arange(0, len(y_full))
    # Filter out the nans!
    good_inds = [i for i, val in enumerate(y_full) if ~np.isnan(val)]
    y = [val for i, val in enumerate(y_full) if i in good_inds]
    x = [val for i, val in enumerate(x_full) if i in good_inds]

    variables_fit = []
    variables_fit.append((y[-1] - y[0]) / (x[-1] - x[0]))
    variables_fit.append(y[0])

    output = leastsq(lin_function_lsquares, variables_fit, args=(np.array(x), np.array(y)))
    gen_slope = output[0][0]
    gen_int = output[0][1]
    return gen_slope, gen_int


def do_quadratic_fit(y_full, x_full=None, deg=2):
    from scipy.optimize import curve_fit
    if x_full is None:
        x_full = np.arange(0, len(y_full))
    # Filter out the nans!
    good_inds = [i for i, val in enumerate(y_full) if ~np.isnan(val)]
    y = np.array([val for i, val in enumerate(y_full) if i in good_inds])
    x = np.array([val for i, val in enumerate(x_full) if i in good_inds])

    variables_fit = []
    variables_fit.append(1) 
    variables_fit.append(1)
    variables_fit.append(min(y))

    output = curve_fit(quadratic_function, x, y)
    gen_a = output[0][0]
    gen_b = output[0][1]
    gen_c = output[0][2]

    return gen_a, gen_b, gen_c


def do_exponential_fit(y_full, x_full=None):
    from scipy.optimize import curve_fit
    if x_full is None:
        x_full = np.arange(0, len(y_full))
    # Filter out the nans!
    good_inds = [i for i, val in enumerate(y_full) if ~np.isnan(val)]
    y = np.array([val for i, val in enumerate(y_full) if i in good_inds])
    x = np.array([val for i, val in enumerate(x_full) if i in good_inds])

    variables_fit = []
    variables_fit.append(1)  # (y[-1] - y[0]) / (x[-1] - x[0]))
    variables_fit.append(1)
    variables_fit.append(min(y))
    variables_fit.append(x_full[0])

    output = curve_fit(exponential_function, x_full, y_full, p0=variables_fit)#, args=(np.array(x), np.array(y)))
    gen_a = output[0][0]
    gen_b = output[0][1]
    gen_c = output[0][2]
    gen_x0 = output[0][3]

    return gen_a, gen_b, gen_c, gen_x0


def do_sigmoid_fit(y_full, x_full=None):
    from scipy.optimize import curve_fit, leastsq
    if x_full is None:
        x_full = np.arange(0, len(y_full))
    # Filter out the nans!
    good_inds = [i for i, val in enumerate(y_full) if ~np.isnan(val)]
    y = np.array([val for i, val in enumerate(y_full) if i in good_inds])
    x = np.array([val for i, val in enumerate(x_full) if i in good_inds])

    p0 = [1, 0, 1, len(y)/2]
    bounds = (0, np.inf)
    try:
        output, pcov = curve_fit(sigmoid_function, x, y, p0=p0, bounds=bounds)
        a = output[0]
        b = output[1]
        c = output[2]
        d = output[3]

        new_y = sigmoid_function(x, a, b, c, d)
        return output, new_y
    except:
        return [1, 0, 1, len(y) / 2],  y


def do_sigmoid_fit_norm(y_full, x_full=None):
    from scipy.optimize import curve_fit, leastsq
    if x_full is None:
        x_full = np.arange(0, len(y_full))
    # # Filter out the nans!
    # good_inds = [i for i, val in enumerate(y_full) if ~np.isnan(val)]
    # y = np.array([val for i, val in enumerate(y_full) if i in good_inds])
    # x = np.array([val for i, val in enumerate(x_full) if i in good_inds])
    try:
        p0 = [1, len(y_full)/2]
        bounds = (0, np.inf)
        output, pcov = curve_fit(sigmoid_function_norm, x_full, y_full, p0=p0, bounds=bounds)
        c = output[0]
        d = output[1]

        new_y = sigmoid_function_norm(x_full, c, d)
        return output, new_y
    except:
        return [1, len(y_full)/2], y_full


def calculate_SUTR_slopes(ramp_cube, times=False, itime=11, sutr_list=False, multi_sutr=False):
    ynums = ramp_cube.shape[-2]
    xnums = ramp_cube.shape[-1]
    
    if len(ramp_cube.shape) == 3:
        number_conditions = 1
        num_ramps = 1
        len_ramp = ramp_cube.shape[0]
    if len(ramp_cube.shape) == 4: #
        num_ramps = ramp_cube.shape[0]
        len_ramp = ramp_cube.shape[1]
        number_conditions = 1
    if len(ramp_cube.shape) == 5:
        number_conditions = ramp_cube.shape[0]
        num_ramps = ramp_cube.shape[1]
        len_ramp = ramp_cube.shape[2]

    sutr_list = [len_ramp]
    if multi_sutr:# sutr_list:
        sutr_list = [2, 5, 8, 18, 36, len_ramp]
    if not times:
        times = [i * itime for i in range(len_ramp)]
    data_shape = (number_conditions, num_ramps, len(sutr_list), ynums, xnums)
    slopes = np.zeros(data_shape)
    signal_term = np.zeros(data_shape)

    for i_cond in range(number_conditions):
        # for i_ramp in range(num_ramps):

        sum1 = np.zeros((len(sutr_list), ynums, xnums))
        sum2 = np.zeros((len(sutr_list), ynums, xnums))
        sum3 = np.zeros(len(sutr_list))
        sum4 = np.zeros(len(sutr_list))

        for i_sutr in range(len(sutr_list)):
            sum3[i_sutr] = sum(times[:sutr_list[i_sutr]])
            sum4[i_sutr] = sum([i**2 for i in times[:sutr_list[i_sutr]]])

        for i_ramp in range(num_ramps):
            # current_ramp = current_ramp + 1
            # ramp_num = str(i_ramp + 1).zfill(3)

            sum1_temp = np.zeros((len(sutr_list), ynums, xnums))
            sum2_temp = np.zeros((len(sutr_list), ynums, xnums))
            # signal_term_temp = np.zeros((len(sutr_list), ynums, xnums))

            for i_samp in range(len_ramp):
                im_cor = ramp_cube[i_cond, i_ramp, i_samp]
                for i_sutr in range(len(sutr_list)):
                    if i_samp <= sutr_list[i_sutr]:
                        sum1_temp[i_sutr] = sum1_temp[i_sutr] + im_cor
                        sum2_temp[i_sutr] = sum2_temp[i_sutr] + (times[i_samp] * im_cor)
                    if i_samp == sutr_list[i_sutr] - 1:
                        signal_term[i_cond, i_ramp, i_sutr, :, :] = signal_term[i_cond, i_ramp, i_sutr, :, :] + im_cor
            sum1[:] = sum1_temp[:]
            sum2[:] = sum2_temp[:]
            # signal_term[i_cond, i_ramp, i_sutr] = signal_term_temp[:]
            denom = np.zeros(len(sutr_list))
            for i_sutr in range(len(sutr_list)):
                denom[i_sutr] = (sutr_list[i_sutr]) * sum4[i_sutr] - sum3[i_sutr]**2
                slopes[i_cond, i_ramp, i_sutr, :, :] = (((sutr_list[i_sutr]) * sum2_temp[i_sutr]) - (sum3[i_sutr] * sum1_temp[i_sutr])) / denom[i_sutr]
                # signal_term[i_cond, i_ramp, i_sutr] = signal_term_temp[i_sutr]

    return slopes, signal_term, sutr_list

def save_info_to_file(rd, dd):
    save_data = []

    for key, value in dd.__dict__.items():
        save_data.append(['dd:  ', key, value])
    # for key, value in rd.__dict__.items():
    #     if key in dd.__dict__.keys():
    #         continue
    #     if key == 'args':
    #         save_data.append(['rd:  ', key, value])

    #     continue
    max_len = max(len(str(item)) for sublist in save_data for item in sublist)
    if max_len >= 25:
        max_len = 25
    # for temp_name in things_to_save:
    with open('/home/dsp/Desktop/LatestInfo.txt', 'w') as f:
        for row_info in save_data:
            line = f'{row_info[0]:<5}  {row_info[1]:<15}  {row_info[2]:>{max_len}}\n'
            # line = f"{' '.join([str(i) for i in row_info])}\n"
            f.write(line)



from dataclasses import dataclass
import xdir

@dataclass
class LastRunObj:
    detpath: str = ''
    clockpath: str = ''
    detname: str = ''
    nrow: int = 2048
    ncol: int = 2048
    nrowskip: int = 0
    ncolskip: int = 0
    nta: int = 0
    ncd: int = 0
    itime: int = 11000
    nsamp: int = 1
    ctstime: int = 0
    adctime: int = 0
    sampmode: int = 0
    runflag: int = 0
    bufferflag: str = ''
    fwp: int = 0
    wavelength: float = 0
    bandwidth: float = 0
    fwn: str = ''
    filter: str = ''
    telescope: int = ''
    observer: str = ''
    gc: str = ''
    lc: str = ''
    voffset: int = -2200
    heater: int = 0
    vreset: int = 100
    dsub: int = 100
    datapath: str = '/data'
    night: str = 'H2RG-20908'
    object: str = 'test'

    # lastrun_filename: str = f'{xdir.detpath}/lastrun.run'

    def __init__(self, emptylastrun=False):
        # if not emptylastrun:
        with open(f'{xdir.detpath}/lastrun.run', 'r') as f:
            for line in f.readlines():
                split_line = line.rstrip('\n').split(' ')
                if not emptylastrun:
                    if split_line[1] != '':
                        new_val = split_line[1]
                        # print(f'Recovered: {split_line[0]} to {new_val}')
                        if str.isnumeric(new_val):
                            setattr(self, split_line[0], int(new_val))
                        else:
                            setattr(self, split_line[0], new_val)
    
    def set_actual_lro(self, filename=f'{xdir.detpath}/lastrun.run'):
        with open(filename, 'r') as f:
            for line in f.readlines():
                key, val = line.rstrip('\n').split(' ')[0], line.rstrip('\n').split(' ')[1]
                if str.isnumeric(val):
                    setattr(self, key, int(val))
                else:
                    setattr(self, key, val)

                        
                










cmd_/home/dsp/py3dsp/ociw_old/MoreBasic/Module.symvers := sed 's/\.ko$$/\.o/' /home/dsp/py3dsp/ociw_old/MoreBasic/modules.order | scripts/mod/modpost -m -a  -o /home/dsp/py3dsp/ociw_old/MoreBasic/Module.symvers -e -i Module.symvers   -T -
cmd_/home/dsp/py3dsp/ociwpci-64bit/Module.symvers := sed 's/ko$$/o/' /home/dsp/py3dsp/ociwpci-64bit/modules.order | scripts/mod/modpost -m -a  -o /home/dsp/py3dsp/ociwpci-64bit/Module.symvers -e -i Module.symvers   -T -

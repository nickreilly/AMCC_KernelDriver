cmd_/home/dsp/py3dsp/ociwpci-64bit/amcc.mod := printf '%s\n'   amcc.o | awk '!x[$$0]++ { print("/home/dsp/py3dsp/ociwpci-64bit/"$$0) }' > /home/dsp/py3dsp/ociwpci-64bit/amcc.mod

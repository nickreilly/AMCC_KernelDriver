from distutils.core import setup, Extension

# submodules not really required here...

ociwmodule = Extension('ociw',
			sources = ['ociwpy.c', 'sload.c'])

setup (name = 'ociw',
       version = '1.0.1',
       description = 'A python module to interface with the ociwpci driver.',
       ext_modules = [ociwmodule])

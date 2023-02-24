
DV2 is a simple FITS Data Viewer. It was developed by the 
NASA Infrared Telescope Facility for use with their 
data acquistion systems.

Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php
Copyright (C) 2012 <NASA IRTF. http://irtfweb.ifa.hawaii.edu>

DV uses the gtk+-2.0 widgets.

DV can fit a linecut to a Gaussian curve. GSL is used for this
feature.  An optional compile variable (-DUSE_GSLFIT=1) enable this.
Having GSL is optional. GSL is available at http://sources.redhat.com/gsl/
or Redhat enterprise make sure use: yum install gsl gsl-devel
Don't have GSL, then set USE_GSLFIT=0 in the makefile.dv file.

We has been used on  CentOS 4/5/6 systems. 
(Now mostly centos 6)

1. Installation
 
 1.1 download the dv2.tar.gz file.

 1.2 untar and make

    > tar xvzf dv2.tar.gz
	 > cd dv
	 > make realclean; make

 1.3 install in /usr/local/bin

    > su root
    # make install

 1.4 When DV starts up, it need to know the location of some of the
     file distributed in the dv/src directory (*.cm, ps_proc). The
     enviroment variable DV_HOME give the location of the file. Set this
     variable before starting dv. For example, after you compiled dv, let say 
     you installed dv, *.cm and ps_proc to /usr/local/dv.  Set the enviroment variable
     DV_HOME to /usr/local/dv before starting up dv. I recommend you have this
     in you .cshrc (or equivalent):

        setenv DV_HOME     /usr/local/dv

 1.5 You can create a ~/.dv-init file with text command to be
     executed when DV start up. For example:

		 M.Path $HOME/macro/dv
		 M.FileMask *
		 Printer irsummitpr
		 DivbyCoadds on
		 UseFITSAngle&Scale on
		 imageShowGBox on
		 Path $HOME/data

If you have question/comments, please contact me. I would be happy to
help you.

Tony Denault
IRTF Programmer

/-----------------------------------------------------------------------\
| Tony Denault                     |   Email: denault at ifa.hawaii.edu |
| Institute for Astronomy          |              Phone: (808) 932-2378 |
| 640 North Aohoku Place           |                Fax: (808) 933-0737 |
| Hilo, Hawaii 96720               |                                    |
\-----------------------------------------------------------------------/



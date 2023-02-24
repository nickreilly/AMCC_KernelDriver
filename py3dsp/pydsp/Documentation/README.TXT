This directory holds the pydsp source.

It starts up by reading the last settings in lastrun.run.
the environment variable DETPATH should be set to the directory where
the detector files are stored.

dspdir.gif shows a listing of the .py files.

several files are symbolic links.
ln -s pyshowall/pyshowall.py
ln -s pyshowall/gshowall.glade
ln -s [proper temperature curve file] tmptr.py
ln -s [proper filter wheel file] filterwh.py

These could probably be eliminated...
Python can execute code dynamically, so the name of the proper module
can probably be determined at runtime from lastrun.run.

The run and det files are stored as text configuration files.
This is also of dubious benefit. Python has excellent persistence mechanisms
available, and those could be used instead.

'det' stuff : always loaded the same way at startup.
'run' stuff : loaded up whatever state the system was last shut down in.

The filter wheel is an interesting beast. It is the internal state of the
filter wheel class that should be persisted. We don't want to move the filter
wheel to where it was when we shut down, we want to tell it that it IS where it was.


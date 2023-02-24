
__version__="""$Id$"""
__URL__="""$URL$"""

import unittest

class dspTestCase(unittest.TestCase):
    def runTest(self):
        widget = Widget("The widget")
        assert widget.size() == (50,50), 'incorrect default size'
    

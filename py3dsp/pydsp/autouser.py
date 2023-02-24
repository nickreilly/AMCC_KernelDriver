"""
autouser.py - automatic user object.

Simulates a user for automatically testing code that requires user input.
It is also useful for scripting that same code for real operation. Check the source code in tests.py for an example.

AutoUser is a class that simulates a user at the keyboard.

Quick Start:
To use in pydsp, break to the command prompt, then do:

	>>> from autouser import AutoUser
	>>> my_user = AutoUser("itime 4000","sscan","srun")
	>>> cloop(my_user)

You can only use the autouser object once! It depletes its list of commands as it runs.

To write code that can use autouser objects just make sure the code that requires input is defined
with raw_input as a keyword parameter that is passed in:

	>>> def myfunc( self, raw_input=raw_input) :
	>>> X = raw_input("are you sure?")

then in your test code use a simulated user!

Sample operation:
 
	>>> autouser = AutoUser("Y","N","x")
	>>> raw_input
	<built-in function raw_input>
	>>> raw_input = autouser
	>>> answer = raw_input("format hard drive?")
	format hard drive? Y
	>>> answer
	'Y'
	>>> answer = raw_input("are you sure?")
	are you sure? N
	>>> answer
	'N'
	>>> answer = raw_input("hit x to continue")
	hit x to continue x
	>>> answer
	'x'
	>>> answer = raw_input("Raise an error at end of list")
	Traceback (most recent call last):
	  File "<stdin>", line 1, in ?
	  File "utils.py", line 14, in next
	    raise StopIteration
	StopIteration
	>>> del raw_input
"""

__version__ = """$Id: autouser.py 405 2006-09-15 18:49:11Z drew $ """

__author__ = "$Author: drew $"

__URL__ = "$URL: http://astro.pas.rochester.edu/svn/pydsp/trunk/pydsp/autouser.py $"


class AutoUser :
    "Create a virtual user, executing a list of commands automatically (like a macro)"
    def __init__(self, *responses) :
        self.responses = list(responses)
    #def __iter__(self):
    #    return self
    def __call__(self, prompt='') :
        import sys
        if self.responses :
            print(prompt, end=' ')
            sys.stdout.flush()
            resp = self.responses.pop(0)
            print(resp)
            return resp
        raise StopIteration

def _test():
    import doctest, autouser
    return doctest.testmod(autouser)

if __name__ == "__main__":
    _test()


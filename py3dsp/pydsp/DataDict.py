"""
DataDict.py

The DataDict overloads the regular dictionary (but does not inherit from it) by allowing a user to call a special function with special arguments whenever any of its values change.

This code was based from the UserDict code.  It could have inherited from UserDict; perhaps though it is easier to understand with it all right here.

NOTE:
The latest Python versions allow derivation from built-in types.
UserDict and DataDict are not needed anymore, and a rewrite should occur later on at some point.

The DataDict also allows attribute access to the dictionary.

Usage:
Instantiating an instance of the class:
    >>> mydict = DataDict()

Adding an item, simplest form:
    >>> mydict.additem("simple")
    >>> mydict["simple"] # default value is None
    >>> print mydict["simple"]
    None

    >>> mydict["simple"] = 42
    >>> mydict["simple"]
    42
    >>> mydict.simple
    42

Reading an item that does not exist prints a message.
    >>> mydict["notthere"]
    Traceback (most recent call last):
      File "<stdin>", line 1, in ?
      File "DataDict.py", line 156, in __getitem__
        raise KeyError
    KeyError: 'Key name notthere does not exist.'
    >>> mydict.additem("withval",42)
    >>> mydict["withval"]
    42

What happens when you write an item that does not exist? Unlike ordinary dictionaries, we want that to complain.
    >>> mydict["notthere"] = 42
    Traceback (most recent call last):
      File "<stdin>", line 1, in ?
      File "DataDict.py", line 160, in __setitem__
        raise KeyError
    KeyError: 'Key name notthere does not exist.'

Writing an unknown attribute should complain too; but it doesn't.
    >>> mydict.notthere = 42 
    >>> 

The action functions setfuncs and getfuncs hook assignment and retrieval into executable code.
Lambda functions are used in this example and  are handy for simple setfuncs and getfuncs.
    >>> mydict.additem("withget",42,getfunc = lambda : "spam")
    >>> mydict.withget
    'spam'
    >>> mydict.withget = "eggs" # try to write it, it seems to fail
    >>> mydict["withget"] # a read returns value from function
    'spam'
    >>> mydict.data["withget"] # internally, it still knows last written value.
    'eggs'

You wont use a write-only variable like this very often, but
several dictionary items can share the same setter function
using arguments.
    >>> def setter(value, arg) : # setter function, two args.
    ...     print value, arg
    ...
    >>> mydict.additem("withset",42,setfunc = setter, args = ("cheese",))
    >>> mydict.additem("withset2",42,setfunc = setter, args = ("grail",))
    >>> mydict["withset"]  = "baloney"
    baloney cheese
    >>> mydict.withset2  = "baloney"
    baloney grail

A GUI hook for the whole dictionary can be set as well.
    >>> mydict.set_widget = setter # we'll reuse setter for testing.
    
Now accesses to any item calls the set_widget function.  currently, "_txt" is appended to each name.
This reflects a pyshowall internal, and should probably be removed.
The pyshowall set_widget function hook would be more appropriate for adding the "_txt"
    >>> mydict["simple"] = "parrot"
    simple_txt parrot

A namemap dictionary can alias a dict name to a widget name:
    >>> mydict.namemap = {"simple":"complicated"}
    >>> mydict["simple"] = "bridgekeeper"
    complicated_txt bridgekeeper

"""

__version__ = """
     $Id: DataDict.py 405 2006-09-15 18:49:11Z drew $
"""

__author__ = "$Author: drew $"

__URL__ = "$URL: http://astro.pas.rochester.edu/svn/pydsp/trunk/pydsp/DataDict.py $"

class DataDict(object):
    """
    Smart dictionary. Very similiar to Python properties.
    """
    def __init__(self, dict=None):
        self.__dict__["dirty"]  = 0 # are we dirty or not?
        self.__dict__["data"] = {} # the values themselves.
        self.__dict__["setfunc"] = {} # a function to call while setting the value
        self.__dict__["getfunc"] = {} # a function to call when getting the value.
        self.__dict__["args"] = {} # tuple of args to pass. value is inserted as first positional arg
        self.__dict__["kwds"] = {} # keyword args to pass.
        self.__dict__["docstring"] = {} # a docstring for the word.
        if dict is not None: self.update(dict)
    def __repr__(self): return repr(self.data)
    def __cmp__(self, dict):
        if isinstance(dict, DataDict):
            return cmp(self.data, dict.data)
        else:
            return cmp(self.data, dict)
    def __len__(self): return len(self.data)
    def set_widget(self,*args) :
        pass
    namemap = {}
    def __setitem__(self,name,val) : # set item, write to dictionary.
        """
	Assign a value to the smart dictionary.

        If a setfunc exists, that is called first (with args if they exist).
        Successful setfunc execution is followed by
        remembering the value that was written, then by
        an attempt to update a GUI via set_widget (the default set_widget function does nothing.)
        The namemap can map dictionary names to widget names if they are different; exceptions in namemap or set_widget are ignored.
        Reasons for doing this: both the Motorolla DSP and the PYShowall GUI will not start up with all the parameters/values set properly (i.e. just reading inthe lastrun.run file does not actually set values), so just execute all the stuff in the dictionaries using the current value.
        """
        if name in self.data:  # needs to be in dictionary already.
          try :
            if self.setfunc[name] : # if a set function exists..
                self.setfunc[name](val,*self.args[name],**self.kwds[name]) # call it with args
            self.dirty = 1 # set flag that says we are dirty.
            self.data[name] = val # always set the value locally.
            try :
                # can map pydsp name to pyshowall name here.
                # if namemap dictionary entry not present,
                # just use pydsp name. with '_txt' appended.
                # all gshowall widgets end in _txt. 
                gname = self.namemap.get(name,name)
                self.set_widget(gname +"_txt", str(val))
            except :
                pass
          except ValueError :
            raise
        else :
          raise KeyError("Key name %s does not exist."%name)
    def __getitem__(self,name) : #
        """
	Recall a value from the smart dictionary.

        If the name has a getfunc, that function is called (with arguments if arguments are defined for the name;
        the arguments are the same for setfunc and getfunc)
        The return value of getfunc is returned to the user.
        If no getfunc exists, the internal value associated with
        that name is returned. 
        """
        if name in self.data:  # needs to be in dictionary already.
            if self.getfunc[name]  :# if a getfunc exists,
                return self.getfunc[name](*self.args[name],**self.kwds[name]) # call it.
            elif name in self.data: # no function, so just return value.
                return self.data[name] # 
        else :
            raise KeyError("Key name %s does not exist."%name)

    def __getattr__(self,name) :
        return self[name]

    def __setattr__(self,name,value) :
        # if attribute is in contained dict:
        if name in self.data :
            self.__setitem__(name,value) # delegate to set_item.
        else:
            self.__dict__[name] = value # otherwise modify dict.

    def __delitem__(self, key):
        del self.data[key]
        del self.setfunc[key]
        del self.getfunc[key]
        del self.args[key]
        del self.kwds[key]
        del self.docstring[key]
    def clear(self): self.data.clear()
    def copy(self):
        if self.__class__ is DataDict:
            return DataDict(self.data)
        import copy
        data = self.data
        try:
            self.data = {}
            c = copy.copy(self)
        finally:
            self.data = data
        c.update(self)
        return c
    def keys(self): return list(self.data.keys())
    def items(self): return list(self.data.items()) # XXX need to call getfuncs if they exist!!
    def iteritems(self): return iter(self.data.items()) # likewise.
    def iterkeys(self): return iter(self.data.keys())
    def itervalues(self): return iter(self.data.values()) # etc.
    def values(self): return list(self.data.values()) # and so on.
    def has_key(self, key): return key in self.data
    def update(self, dict):
        if isinstance(dict, DataDict):
            self.data.update(dict.data)
        elif isinstance(dict, type(self.data)):
            self.data.update(dict)
        else:
            for k, v in list(dict.items()):
                self[k] = v
    def get(self, key, failobj=None):
        if key not in self:
            return failobj
        return self[key]
    def setdefault(self, key, failobj=None):
        if key not in self:
            self[key] = failobj
        return self[key]
    def popitem(self):
        return self.data.popitem()
    def __contains__(self, key):
        return key in self.data
    def __iter__(self):
        return iter(self.data)
    def additem(self,name,val=None,setfunc=None,getfunc=None,args=(), kwds=None,docstring=None) :
        """
	Put a new entry in the smart dictionary.

        name = string used to identify this thing
        val = initial value of item
        setfunc: callable. If defined, it is called with new value on writes
        getfunc: callable. If defined, it is called on reads
        args: if defined, passed as last argument for setfunc and getfunc.
        docstring: help string for the item.
        """
        self.data[name] = val
        self.setfunc[name] = setfunc # a default setfunction
        self.getfunc[name] = getfunc
        self.args[name] = args 
        if kwds :
            self.kwds[name] = kwds
        else :
            self.kwds[name] = {}
        self.docstring[name] = docstring

def _test() :
    import doctest, DataDict
    return doctest.testmod(DataDict)

if __name__ == "__main__" :
    _test()
 

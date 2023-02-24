"""
Socket client interface to pydsp

Experimental.
Loops in two threads. Main thread gets data from the local user,
sends this input to pydsp socket.
printer thread gets the output coming back from pydsp
and displays it.
"""

if __name__ == '__main__' :

    import socket

    pydsp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    pydsp_socket.setsockopt(0,socket.SO_REUSEADDR,1)
    pydsp_socket.connect( ('localhost',9747) )

    def printer():
        while True:
            print(pydsp_socket.recv(8192))

    import threading
    t = threading.Thread(None, printer)
    t.setDaemon(True)
    t.start()

    import time
    while True :
        input = input("spy? ")
        if not input: 
            continue # don't send empty line over a socket.
        # unless you want to close the connection:
        if input.lower() == 'bye' : 
            input = ''
        pydsp_socket.send(input)
        if not input: 
            break # if we closed the connection, bail.
        time.sleep(0.5) # give time for the response

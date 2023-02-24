"""
A UDP server that connects to  (runs inside of) pydsp.
It allows commands from an external source.

See client.py

this is EXPERIMENTAL
but having a server that you never shut down would be very cool.
You could start up another gui on a remote machine.
"""
import socket

pydsp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
pydsp_socket.setsockopt(0,socket.SO_REUSEADDR,1)
pydsp_socket.bind( ('localhost',9747) )

import threading
import time

def activatecmd(cmd) :
    print("got", cmd)

connected = []

class sockobject:
    def write(self, response, excluded=None):
        """
        send response to ALL connected clients.
        except excluded one.
        """
        for addr in connected[:]: # copy of list. we modify original in loop.
            if addr == excluded:
                continue
            try:
                pydsp_socket.sendto(response, addr)
            except:
                connected.remove(addr)
    def input(self, prompt=""):
        """
        acts like input function
        """
        self.write(prompt) # send prompt to everybody
        data, addr = pydsp_socket.recvfrom(8192)
        self.write(data, excluded=addr) # echo command to almost everybody
        return data
        
sockfile = sockobject()
printer = sockfile.write

def sockserv():
  while True :
    try:
        data, addr = pydsp_socket.recvfrom(8192)
    except KeyboardInterrupt:
        break
    except:
        continue

    if not data: # this client is exiting.
        if addr in connected:
            print("client exiting - ", addr)
            connected.remove(addr)
            printer("bye")
        else:
            print("unconnected client exiting?", addr)
        continue

    if addr not in connected:
        printer("new connection")
        connected.append(addr)

    printer(data, excluded=addr) # echo command to all clients except sender.
    activatecmd(data)
  pydsp_socket.close()

if __name__ == '__main__':
    sockserv()
    print('bye')

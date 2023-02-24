"""
This program serves as a software simulator for the pydsp program.

It connects to other programs using UNIX sockets.
"""

import socket
import logging as log
import sys, os

sock_addr = "./sock_sim.sock"
log_addr = "./log_sim.log"

logger = log.getLogger("dspsim")
logger.setLevel(log.DEBUG)
fh = log.FileHandler(log_addr)
fh.setLevel(log.DEBUG)

sh = log.StreamHandler()
sh.setLevel(log.DEBUG)

formatter = log.Formatter('[%(levelname)s] %(asctime)s: %(name)s - %(message)s')
fh.setFormatter(formatter)
sh.setFormatter(formatter)

logger.addHandler(fh)
logger.addHandler(sh)

logger.info("Starting Simulated DSP Board Server")

if os.path.exists(sock_addr):
    logger.warning("Socket Address exists, deleting")
    os.remove(sock_addr)

logger.debug("Staring up socket")
s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
s.bind(sock_addr)

logger.debug("Waiting for connection")
s.listen(1)


while True:
    # set up an echo server, log messages
    connection, c_addr = s.accept()
    logger.info("New connection made with {}".format(c_addr))

    while True:
        logger.info("Waiting for message")
        msg = connection.recv(1024)
        if msg:
            logger.info("Recieved message: {}".format(msg))
        else:
            logger.info("null msg, closing down")
            connection.close()
            break

s.close()


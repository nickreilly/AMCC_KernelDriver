"""
In the original code there was a section for an experimental webserver,
rather than delete it, it was moved to it's own separate file.

No work is expected to be done, it's more a matter of cleaning up the
code.
"""


# --------------------------------------------
# Next stuff is experimental web interface.
# completely unnecessary. Delete or move somewhere else.

pagetop = """ 
<html>
<head>
<title>Pydsp on Itchy</title>
</head>
<body>
<h1>Pydsp Status</h1>
"""

pageend = """
</body>
</html>
"""

import http.server

class pydspWebServer(http.server.BaseHTTPRequestHandler):
    """
    The class that defines the internals for the pydsp web server.
    """

    server_version = "pydspHTTP/1.0"

    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-type","text/html")
        self.end_headers()
        try:
            self.wfile.write(pagetop)
            for v in list(rd.items()):
                self.wfile.write("<p> %s %s" % (v[0], str(v[1])))
            for v in list(dd.items()):
                self.wfile.write("<p> %s %s" % (v[0], str(v[1])))
            self.wfile.write(pageend)
        finally:
            pass

    def do_POST(self):
        self.send_response(200)
        self.send_header("Content-type","text/html")
        self.end_headers()
        try:
            self.wfile.write(pagetop)
            self.wfile.write(pageforms)
            self.wfile.write("<p>"+self.path)
            self.wfile.write("<p>"+self.raw_requestline)
            self.wfile.write(pageend)
        finally:
            pass

    def log_request(self, *args):
        pass

    log_message = log_request

    do_HEAD = do_GET

def startWebServer(): 
    """
    Start up a simple embedded web server in its own thread.
    """
    def webThread():
        webserver = http.server.HTTPServer(("",8070), pydspWebServer)
        webserver.serve_forever()

    global webthread
    import threading
    webthread = threading.Thread(None, webThread)
    webthread.setDaemon(True)
    webthread.start()

# end of WebServer stuff.



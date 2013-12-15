__author__ = 'Christopher Nelson'

import tornado.web

class MainHandler(tornado.web.RequestHandler):
    def get(self):
        self.write("Hello, world")

handlers = [
    (r"/", MainHandler)
]

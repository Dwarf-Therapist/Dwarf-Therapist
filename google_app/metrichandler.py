from google.appengine.ext import webapp
import pprint
import logging

class MetricHandler(webapp.RequestHandler):
    def get(self, *args):
        self.response.headers['Content-Type'] = 'text/html'
        info = ip_info("96.253.131.222")
        self.response.out.write("Metrics must be posted! %s<br/>%s" % (pprint.pformat(info), self.request.arguments()))
        
    def post(self, *args):
        self.response.headers['Content-Type'] = 'text/html'
        self.response.out.write("POST<br/>%s" % "<br/>".join(args))
        logging.info("UPDATE FROM IP %s USING VERSION %s", 
                     self.request.remote_addr, 3)

        
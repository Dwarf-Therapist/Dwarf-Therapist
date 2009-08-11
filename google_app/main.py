from google.appengine.ext import webapp
from google.appengine.api import memcache
from google.appengine.ext.webapp.util import run_wsgi_app

from versionhandler import VersionHandler
from metrichandler import MetricHandler

class MainPage(webapp.RequestHandler):
    def get(self):
        self.response.headers['Content-Type'] = 'text/plain'
        self.response.out.write('Hello, webapp World!')
        
mapping = [
    ('/', MainPage)
    ,('/version', VersionHandler)
]
application = webapp.WSGIApplication(mapping, debug=True)

def main():
    run_wsgi_app(application)

if __name__ == "__main__":
    main()

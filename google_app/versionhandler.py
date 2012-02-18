import logging
import pprint
import re
from google.appengine.ext import webapp
from metricentry import MetricEntry
from utils import ip_info

CURRENT_VERSION = "0.6.12"
RX_VERSION_CHUNKER = re.compile(r"\s*DwarfTherapist\s+(\d+)\.(\d+)\.(\d+)\s*")
class VersionHandler(webapp.RequestHandler):
    def get(self, *args):
        info = ip_info(self.request.remote_addr)
        
        entry = MetricEntry.get_or_insert("ip_%s" % self.request.remote_addr)
        entry.ip = self.request.remote_addr
        entry.city = info['city']
        entry.country = info['country']
        entry.region_name = info['region_name']
        entry.region_code = info['region_code']
        entry.dt_version_string = self.request.headers["User-Agent"]
        #logging.info("user-agent: %s" % self.request.headers["User-Agent"])
        m = re.match(RX_VERSION_CHUNKER, self.request.headers["User-Agent"])
        if m:
            print "MATCH GROUPS", m.groups()
            entry.dt_version_major = int(m.group(1))
            entry.dt_version_minor = int(m.group(2))
            entry.dt_version_patch = int(m.group(3))
        
        entry.put()
        
        self.response.headers['Content-Type'] = 'text/plain'
        self.response.out.write(CURRENT_VERSION);

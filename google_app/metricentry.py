import datetime
from google.appengine.ext import db

class MetricEntry(db.Model):
    ip = db.StringProperty()
    city = db.StringProperty()
    region_code = db.StringProperty()
    region_name = db.StringProperty()
    country = db.StringProperty()
    dt_version_major = db.IntegerProperty()
    dt_version_minor = db.IntegerProperty()
    dt_version_patch = db.IntegerProperty()
    dt_version_string = db.StringProperty()
    entry_time = db.DateTimeProperty(auto_now=True, 
                                     auto_now_add=True)
   

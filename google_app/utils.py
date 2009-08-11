import logging
import urllib
from xml.etree import ElementTree as ET

def ip_info(ip):
    """
<?xml version="1.0" encoding="UTF-8"?>
<Response>
        <Ip>1.2.3.4</Ip>
        <Status>OK</Status>
        <CountryCode>US</CountryCode>
        <CountryName>United States</CountryName>
        <RegionCode>22</RegionCode>
        <RegionName>Arkansas</RegionName>
        <City>Some City</City>
        <ZipPostalCode>12345</ZipPostalCode>
        <Latitude>40.0000</Latitude>
        <Longitude>-120.000</Longitude>
        <Gmtoffset>-5.0</Gmtoffset>
        <Dstoffset>-4.0</Dstoffset>
</Response>
    """
    info = {
        'country': 'unknown',
        'city': 'unknown',
        'ip': ip
    }
    try:
        gs = urllib.urlopen('http://blogama.org/ip_query.php?ip=%s&output=xml' % ip)
        txt = gs.read()
    except:
        logging.error('GeoIP servers not available')
        return info
    
    try:
        tree = ET.fromstring(txt.strip())
        country = tree.find("CountryCode")
        city = tree.find("City")
        if country is not None:
            info['country'] = country.text
        if city is not None:
            info['city'] = city.text
    except:
        logging.error("error parsing IP result %s", txt)
    return info
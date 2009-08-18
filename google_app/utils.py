import logging
import urllib
from xml.etree import ElementTree as ET
from pprint import pformat

xml_to_dict_map = {
    'CountryCode': 'country',
    'RegionName': 'region_name',
    'RegionCode': 'region_code',
    'City': 'city'
}

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
        'region_code': 'unknown',
        'region_name': 'unknown',
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
        for xml_val, dict_key in xml_to_dict_map.items():
            elem = tree.find(xml_val)
            if elem is not None:
                info[dict_key] = elem.text
    except:
        logging.error("error parsing IP result %s", txt)
    logging.info("got info for %s", ip)
    logging.info(pformat(info))
    return info

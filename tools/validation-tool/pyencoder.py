
from binascii import hexlify, unhexlify
import J2735
import json
import sys

import csv

def extract_values(obj, key):
    """Pull all values of specified key from nested JSON."""
    arr = []

    def extract(obj, arr, key):
        """Recursively search for values of key in JSON tree."""
        if isinstance(obj, dict):
            for k, v in obj.items():
                if isinstance(v, (dict, list)):
                    extract(v, arr, key)
                elif k == key:
                    arr.append(v)
        elif isinstance(obj, list):
            for item in obj:
                extract(item, arr, key)
        return arr

    results = extract(obj, arr, key)
    return results

## read from file and print time for only asked id 
##usage pydecoder filename  id outputfile

print("usage pyencoder")



fp = open("Maps/Dekar_map_all_high.json",'r')
MAP = fp.read()

jMAP = json.loads(MAP)

print(type(jMAP))
mapasnobj =  J2735.DSRC.MapData()
print(mapasnobj)
#mapasnobj.set_val(jMAP)
#print(mapasnobj.to_asn())

#print(mapasnobj.to_uper(jMAP))



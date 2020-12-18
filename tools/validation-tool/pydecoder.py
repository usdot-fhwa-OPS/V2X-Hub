
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

print("usage pydecoder filename id outputfile J2735msg")


# fp1=open(sys.argv[1],'r')
# id=int(sys.argv[2])
# fout=open(sys.argv[3],'w',0)
# msgid=""
# if(sys.argv[4] == "BSM"):
#     msgid="0014"
# elif(sys.argv[4]=="SPAT"):
#     msgid="0013"
# elif(sys.argv[4]=="MAP"):
#     msgid="0012"

# fp= csv.reader(fp1,delimiter=',')
# list1=list(fp)
msgid = "0014"
print("Looking for ",msgid)
list1="00147843d50c40000d27666e948d9ea6d8c088caff7f7ffff0032852fd8017d07f7fff80007a196a00c0c081000dbff9affe012a10062bfd3101806dc0ffe9bf1ff0b412ce10032bf1010dc13e810034bf15110814fefffec800114100ce38404220641903c39c0000000000067793e0824d03440000010f12fc002cd88eaf"
#for dt in list1:
dt=list1
print(dt)

if( dt[0:4]==msgid):
    msg = J2735.DSRC.MessageFrame

    msg.from_uper(unhexlify(dt))
    print(type(msg))
    




    print(msg())


    print(msg.to_uper())

    # dict2str=json.dumps(msg(),indent=4,sort_keys=True,ensure_ascii=False)
    # parsed = json.loads(dict2str.decode("utf-8","ignore"))



    # if (msgid == "0013"):
    #     res=extract_values(parsed,'id')
    #     if (res[0]==id):
    #         fout.write(dt)
    #         fout.write(',')
    #         fout.write(str(res[0]))
    #         fout.write('\n')
    # elif (msgid == "0014"): # if bsm , look for lat, long, speed along with time
    #     lat1=extract_values(parsed,'lat')
    #     long1 = extract_values(parsed,'long')
    #     #speed1=extract_values(parsed,'speed')

    #     fout.write(str(dt[0])+','+str(lat1[1]/10000000.0)+','+str(long1[0]/10000000.0)+'\n')






   



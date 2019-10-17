#curl -X POST http://127.0.0.1:9000/v1/pedestrian -H "Content-Type:application/json" -d '{ "messageId": "32","lat": "32.000","lon": "\-123", "speed": "20","heading": "100"}'



#curl -X POST http://127.0.0.1:9000/v1/pedestrian -H "Content-Type:application/json" -d '{"messageId": "32","value": {"PersonalSafetyMessage": {"basicType": {"aPEDESTRIAN": []},"secMark": "1","msgCnt": "1","id": "10","position": {"lat": "32.000","lon": "\-84.000"},"accuracy": {"semiMajor": "4","semiMinor": "4","orientation": "65535"},"speed": "20","heading": "100","pathPrediction": {"radiusOfCurve": "32767","confidence": "100" }}}}'


#curl -X POST   http://127.0.0.1:9000/v1/pedestrian   -H "Content-Type: application/json" -d "{ \"messageId\": 32,\"value\": {\"PersonalSafetyMessage\": {\"basicType\": {\"aPEDESTRIAN\": []},\"secMark\": 1,\"msgCnt\": 1,\"id\": 10,\"position\": {\"lat\": 32.000,\"long\": -84.000},\"accuracy\": {\"semiMajor\": \"4\",\"semiMinor\": \"4\",\"orientation\": \"65535\"},\"speed\": \"20\",\"heading\": \"100\",\"pathPrediction\": {\"radiusOfCurve\": \"32767\",\"confidence\": \"100\" }"



# curl -X POST http://127.0.0.1:9000/v1/pedestrian -H "Content-Type:application/json" -d '<?xml version="1.0" encoding="UTF-8"?>
# <PersonalSafetyMessage>
# <basicType>
# <aPEDESTRIAN/>
# </basicType>
# <secMark>0</secMark>
# <msgCnt>0</msgCnt>
# <id>87654321</id>
# <position>
# <lat>406680509</lat>
# <long>-738318466</long>
# <elevation>40</elevation>
# </position>
# <accuracy>
# <semiMajor>155</semiMajor>
# <semiMinor>155</semiMinor>
# <orientation>65535</orientation>
# </accuracy>
# <speed>100</speed>
# <heading>122</heading>
# <crossState>
# <true/>
# </crossState>
# <clusterSize>
# <medium/>
# </clusterSize>
# <clusterRadius>6</clusterRadius>
# </PersonalSafetyMessage>'
#00201d01c0020000021d950c854de25cbd3f47f97d1028ffffffff0258e58a0c


curl -X POST http://192.168.55.46:9000/v1/pedestrian -H "Content-Type:application/json" -d '<?xml version="1.0" encoding="UTF-8"?>
<PersonalSafetyMessage>
<basicType>
<aPEDESTRIAN/>
</basicType>
<secMark>0</secMark>
<msgCnt>0</msgCnt>
<id>87654321</id>
<position>
<lat>406680509</lat>
<long>-738318466</long>
<elevation>40</elevation>
</position>
<accuracy>
<semiMajor>255</semiMajor>
<semiMinor>255</semiMinor>
<orientation>65535</orientation>
</accuracy>
<speed>75</speed>
<heading>3672</heading>
<crossState>
<true/>
</crossState>
<clusterSize>
<medium/>
</clusterSize>
<clusterRadius>6</clusterRadius>
</PersonalSafetyMessage>'
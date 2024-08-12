import aiohttp
import asyncio

url = "http://127.0.0.1:9000"
data = '''
<PersonalSafetyMessage>
    <basicType><aPEDESTRIAN /></basicType>
    <secMark>0</secMark>
    <msgCnt>0</msgCnt>
    <id>87654321</id>
    <position><lat>400774115</lat><long>-1248282086</long><elevation>538</elevation></position>
    <accuracy><semiMajor>255</semiMajor><semiMinor>255</semiMinor><orientation>65535</orientation></accuracy>
    <speed>0</speed>
    <heading>270</heading>
    <crossState><true /></crossState>
    <clusterSize><medium /></clusterSize>
    <clusterRadius>6</clusterRadius>
</PersonalSafetyMessage>
'''

headers = {
    'Content-Type': 'application/xml',
}

async def send_request(session, i):
    async with session.post(url, data=data, headers=headers) as response:
        if response.status == 201:
            print(f"Request {i} succeeded")
        else:
            print(f"Request {i} failed with status code {response.status}")

async def main():
    async with aiohttp.ClientSession() as session:
        tasks = []
        for i in range(1000):
            tasks.append(asyncio.create_task(send_request(session, i)))
            await asyncio.sleep(0.1)  # maintain the interval between requests
        await asyncio.gather(*tasks)

if __name__ == "__main__":
    asyncio.run(main())


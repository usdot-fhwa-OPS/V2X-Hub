#!/usr/bin/python3
######## use this block to send message to V2X hub##################
####### The messages hit MessageReceiver Plugin first at port: 26789 ########

import socket
import binascii as ba
import time

udpip = "127.0.0.1"
udpport = 26789


sk = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Send sample MobilityOperation messages
staging_area_messages = [
    {
        "DOT-80550: Arrival at ENTER_STAGING_AREA location": "00f381093f3cbbb265e57f4e47e797764cbcafe9c98b9edbfa7258b0c183060c183060b583060c16b060c182d60c18305ac183060c183060c183060c5bb170c993866cdab068e5b3766cdcb9431e1e5b70afe1bf974bf93961f3873e531bda0458f6f6bfa722274811449f516b860d5ab044b1022d9bf1e1e9a77ee4481020408102040e907b408b661e9a7a75c99513a40899b85ce5ab568cdbb744b1022d9bf767d3d3ae4ca89d2044b5bb75cc5ab066d19314483eac408b265e7d34eec3d34efdc89d20f68116cc3d34f4eb932a240810204081020748113370b9cb56ac1833708962045b37eecfa7a75c99513a40896b76eb98b470c5cb8668907d588116fe197961e9a77ee44e902245f4"
    },
    {
        "DOT-80550: Arrive at PICKUP location": "00f381583f3cbbb265e57f4e47e797764cbcafe9c98b9edbfa7258b0c183060c183060b583060c16b060c182d60c18305ac183060c183060c183060c5bb170c993968d19b76ce183570dd9b1431e1e5b70afe1bf974bf93961f3873e5483da0458f6f6bfa722274811449f516b860d5ab044b1022c787967df7f4e444e902287069479f7e0a2588116cdf8f0f4d3bf722408102040810207483da045b30f4d3d3ae4ca89d2044cdc2e72d5ab466ddba2588116cdfbb3e9e9d726544e90225addbae62d583368c98a241f562045932f3e9a7761e9a77ee44e907b408b661e9a7a75c9951204081020408103a40899b85ce5ab560c19b844b1022d9bf767d3d3ae4ca89d2044b5bb75cc5a3862e5c334483eac408b7f0cbcb0f4d3bf7227481150930e5d5a08962045871f4d3bf75fd3911204081020408103a4089a62cb858e368d56e568c98ad62c72e25b85c63c6b5830618d939c3959b8739117d0"
    },
    {
        "DOT-80550: Arrive at EXIT_STAGING_AREA location": "00f381623f3cbbb265e57f4e47e797764cbcafe9c98b9edbfa7258b0c183060c183060b583060c16b060c182d60c18305ac183060c183060c183060c5bb170c99396ec1ab970e1a3364e58b1431e1e5b70afe1bf974bf93961f3873e54afda0458f6f6bfa722274811449f516b860d5ab044b1022c787967df7f4e444e902287069479f7e0a2588116cdf8f0f4d3bf722408102040810207483da045b30f4d3d3ae4ca89d2044cdc2e72d5ab466ddba2588116cdfbb3e9e9d726544e90225addbae62d583368c98a241f562045932f3e9a7761e9a77ee44e907b408b661e9a7a75c9951204081020408103a40899b85ce5ab560c19b844b1022d9bf767d3d3ae4ca89d2044b5bb75cc5a3862e5c334483eac408b7f0cbcb0f4d3bf7227481145b126a5fa7520c7933a3df834a2c144b1022c38fa69dfbafe9c88902040810204081d2044cd93364c31b8c2b72b464c96b1639712dc2e31e35ac1830c6c9ce1cacdc39c88be80"
    },
]

port_area_messages = [
    {
        "DOT-80550: Arrive at ENTER_PORT location": "00f3815b3f3cbbb265e57f4e47e797764cbcafe9c98b9edbfa7258b0c183060c183060b583060c16b060c182d60c18305ac183060c183060c183060c5bb170c99396ee59b070e18b860c1ab3431e1e5b70afe1bf974bf93961f3873e5493da0458f6f6bfa722274811449f516b860d5ab044b1022c787967df7f4e444e902287069479f7e0a2588116cdf8f0f4d3bf722408102040810207483da045b30f4d3d3ae4ca89d2044cdc2e72d5ab466ddba2588116cdfbb3e9e9d726544e90225addbae62d583368c98a241f562045932f3e9a7761e9a77ee44e907b408b661e9a7a75c9951204081020408103a40899b85ce5ab560c19b844b1022d9bf767d3d3ae4ca89d2044b5bb75cc5a3862e5c334483eac408b7f0cbcb0f4d3bf72274811459d522d2bf427d2a88962045871f4d3bf75fd3911204081020408103a4089a61c7959b9cad96e5cacdb2d62c72e25ae70b066b583268cb0e362cd83060cd17d"
    },
    {
        "DOT-80550: Arrive at DROPOFF location": "00f381593f3cbbb265e57f4e47e797764cbcafe9c98b9edbfa7258b0c183060c183060b583060c16b060c182d60c18305ac183060c183060c183060c5bb170c993972dd8386cc5bb968d98b9431e1e5b70afe1bf974bf93961f3873e5487da0458f6f6bfa722274811449f516b860d5ab044b1022c787967df7f4e444e902287069479f7e0a2588116cdf8f0f4d3bf722408102040810207483da045b30f4d3d3ae4ca89d2044cdc2e72d5ab466ddba2588116cdfbb3e9e9d726544e90225addbae62d583368c98a241f562045932f3e9a7761e9a77ee44e907b408b661e9a7a75c9951204081020408103a40899b85ce5ab560c19b844b1022d9bf767d3d3ae4ca89d2044b5bb75cc5a3862e5c334483eac408b7f0cbcb0f4d3bf7227481144a53e84f8d1912c408b0e3e9a77eebfa72224081020408102074811366f970e4c8cf0ad66e31345ac58e5c6b5cb361856b060c18b46ac1cb8cad3322fa0"
    },
    {
        "DOT-80550: DOT-80550: Arrive at PICKUP location": "00f381583f3cbbb265e57f4e47e797764cbcafe9c98b9edbfa7258b0c183060c183060b583060c16b060c182d60c18305ac183060c183060c183060c5bb170c99b060e583964d583870c9832431e1e5b70afe1bf974bf93961f3873e5483da0458f6f6bfa722274811449f516b860d5ab044b1022c787967df7f4e444e902287069479f7e122588116cdf8f0f4d3bf722408102040810207483da045b30f4d3d3ae4ca89d2044cdc2e72d5ab466ddba2588116cdfbb3e9e9d726544e90225addbae62d583368c98a241f562045932f3e9a7761e9a77ee44e907b408b661e9a7a75c9951204081020408103a40899b85ce5ab560c19b844b1022d9bf767d3d3ae4ca89d2044b5bb75cc5a3862e5c334483eac408b7f0cbcb0f4d3bf7227481150930e5d5a08962045871f4d3bf75fd3911204081020408103a4089862ccdf2e2c9856b37189aad62c72e35ae59b0c2b583060c5a3560e5c65699917d0"
    },
    {
        "DOT-80550: Arrive at PORT_CHECKPOINT location": "00f381603f3cbbb265e57f4e47e797764cbcafe9c98b9edbfa7258b0c183060c183060b583060c16b060c182d60c18305ac183060c183060c183060c5bb170c99b064d18306ec5a3368e19b3431e1e5b70afe1bf974bf93961f3873e54a7da0458f6f6bfa722274811449f516b860d5ab044b1022c787967df7f4e444e902287069479f7e122588116cdf8f0f4d3bf722408102040810207483da045b30f4d3d3ae4ca89d2044cdc2e72d5ab466ddba2588116cdfbb3e9e9d726544e90225addbae62d583368c98a241f562045932f3e9a7761e9a77ee44e907b408b661e9a7a75c9951204081020408103a40899b85ce5ab560c19b844b1022d9bf767d3d3ae4ca89d2044b5bb75cc5a3862e5c334483eac408b7f0cbcb0f4d3bf72274811509f4aa5f87222c397427c99d5112c408b0e3e9a77eebfa722240810204081020748113964cd83560d322d66e31355ac58e5c6b5cb361856b060c18b46ac1cb8cad3322fa0"
    },
    {
        "DOT-80550: Arrive at HOLDING_AREA location": "00f3815d3f3cbbb265e57f4e47e797764cbcafe9c98b9edbfa7258b0c183060c183060b583060c16b060c182d60c18305ac183060c183060c183060c5bb170c99b068d98b068e1b3472d9cb1431e1e5b70afe1bf974bf93961f3873e549bda0458f6f6bfa722274811449f516b860d5ab044b1022c787967df7f4e444e902287069479f7e122588116cdf8f0f4d3bf722408102040810207483da045b30f4d3d3ae4ca89d2044cdc2e72d5ab466ddba2588116cdfbb3e9e9d726544e90225addbae62d583368c98a241f562045932f3e9a7761e9a77ee44e907b408b661e9a7a75c9951204081020408103a40899b85ce5ab560c19b844b1022d9bf767d3d3ae4ca89d2044b5bb75cc5a3862e5c334483eac408b7f0cbcb0f4d3bf72274811489f322499d1efc1a5160a25881161c7d34efdd7f4e44481020408102040e9022cccdcb370d1c395ac9cb070b58b1cb996b968cb32d60c186364e583170d5b3045f4"
    },
    {
        "DOT-80550: Arrive at EXIT_PORT location": "00f3815b3f3cbbb265e57f4e47e797764cbcafe9c98b9edbfa7258b0c183060c183060b583060c16b060c182d60c18305ac183060c183060c183060c5bb170c99b06ae1bb768e193170e1c33431e1e5b70afe1bf974bf93961f3873e548fda0458f6f6bfa722274811449f516b860d5ab044b1022c787967df7f4e444e902287069479f7e122588116cdf8f0f4d3bf722408102040810207483da045b30f4d3d3ae4ca89d2044cdc2e72d5ab466ddba2588116cdfbb3e9e9d726544e90225addbae62d583368c98a241f562045932f3e9a7761e9a77ee44e907b408b661e9a7a75c9951204081020408103a40899b85ce5ab560c19b844b1022d9bf767d3d3ae4ca89d2044b5bb75cc5a3862e5c334483eac408b7f0cbcb0f4d3bf7227481145b126a5fa13e95444b1022c38fa69dfbafe9c88902040810204081d2044d58b1c39183564b59b8c4d96b163971ad72cd8615ac183062d1ab072e32b4cc8be80"
    },
    {
        "DOT-80550: Arrive at ENTER_STAGING_AREA location": "00f381623f3cbbb265e57f4e47e797764cbcafe9c98b9edbfa7258b0c183060c183060b583060c16b060c182d60c18305ac183060c183060c183060c5bb170c99b06ec1c376ac5bb66cc5cb3431e1e5b70afe1bf974bf93961f3873e54b3da0458f6f6bfa722274811449f516b860d5ab044b1022c787967df7f4e444e902287069479f7e122588116cdf8f0f4d3bf722408102040810207483da045b30f4d3d3ae4ca89d2044cdc2e72d5ab466ddba2588116cdfbb3e9e9d726544e90225addbae62d583368c98a241f562045932f3e9a7761e9a77ee44e907b408b661e9a7a75c9951204081020408103a40899b85ce5ab560c19b844b1022d9bf767d3d3ae4ca89d2044b5bb75cc5a3862e5c334483eac408b7f0cbcb0f4d3bf72274811459d522d2bf4ea418f26747bf06945828962045871f4d3bf75fd3911204081020408103a408b36362d723565856b3c6c31ad62c72e35b89830c8b583060c5a3560e5c65699917d"
    },
]


def send_messages(messages):
    for message in messages:
        title = list(message.keys())[0]
        message = list(message.values())[0]
        if "HOLDING" in title:
            hold = input("Move to holding location? (y/n): ")
            if hold != "y":
                continue
        unhexed = ba.unhexlify(message)
        sk.sendto(unhexed, (udpip, udpport))
        print(f"\n{title}. Sent: {message}")
        input("Press enter to send next arrival message.")


if __name__ == "__main__":
    while True:
        print("\nAvailable area options:\n\t1: staging\n\t2: port\n\t0: exit\n")
        area = input("Choose area: ")
        if area == "1":
            send_messages(staging_area_messages)
        elif area == "2":
            send_messages(port_area_messages)
        elif area == "0":
            break
        else:
            print("Invalid area. Choose 0 to exit")
#!/usr/bin/env python3
"""
Mock SNMP RSU Server (Updated for Python 3.12+)
Simulates an RSU device responding to SNMPv3 queries.
Supports NTCIP 1218 and RSU MIB 4.1.1
snmpget -v 3 -u authOnlyUser -l authPriv     -a SHA-512 -A dummy123 -x AES-256 -X dummy123     127.0.0.1:1610 1.3.6.1.4.1.1206.4.2.18.13.4.0
"""

# --- COMPATIBILITY SHIM FOR PYTHON 3.12+ ---
import sys
try:
    import asyncore
except ImportError:
    import pyasyncore as asyncore
    sys.modules['asyncore'] = asyncore
# -------------------------------------------

from pysnmp.entity import engine, config
from pysnmp.entity.rfc3413 import cmdrsp, context
from pysnmp.carrier.asyncio.dgram import udp
from pysnmp.proto.api import v2c
from pysnmp.proto import rfc1902
from pysnmp.smi import instrum
import asyncio

# RSU Configuration
RSU_CONFIG = {
    "ip": "0.0.0.0",
    "port": 1610,
    "user": "authOnlyUser",
    "authProtocol": "SHA-512", 
    "authPassPhrase": "dummy123",
    "privacyProtocol": "AES-256",
    "privacyPassPhrase": "dummy123",
    "securityLevel": "authPriv"
}

# NTCIP 1218 MIB OIDs
NTCIP1218_OIDS = {
    "1.3.6.1.4.1.1206.4.2.18.13.4.0": ("rsuID", "RSU-TEST-001"),
    "1.3.6.1.4.1.1206.4.2.18.13.1.0": ("rsuMibVersion", "NTCIP1218 v01.38"),
    "1.3.6.1.4.1.1206.4.2.18.13.2.0": ("rsuFirmwareVersion", "v2.5.1-mock"),
    "1.3.6.1.4.1.1206.4.2.18.1.2.1.2.0": ("rsuRadioDesc", "Cohda MK5 OBU"),
    "1.3.6.1.4.1.1206.4.2.18.1.2.1.4.0": ("rsuRadioType", "1"),
    "1.3.6.1.4.1.1206.4.2.18.6.5.0": ("rsuGnssOutputString", "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47"),
    "1.3.6.1.4.1.1206.4.2.18.16.1.0": ("rsuChanStatus", "0"),
    "1.3.6.1.4.1.1206.4.2.18.16.2.0": ("rsuMode", "3"),
    "1.3.6.1.4.1.1206.4.2.18.4.1.0": ("maxRsuIFMs", "10"),
    "1.3.6.1.4.1.1206.4.2.18.4.2.1.2": ("rsuIFMPsid", "32"),
}

# RSU MIB 4.1 OIDs
RSU41_OIDS = {
    "1.0.15628.4.1.17.4.0": ("rsuID", "RSU-4.1-TEST-001"),
    "1.0.15628.4.1.17.1.0": ("rsuMibVersion", "rsuMIB 4.1"),
    "1.0.15628.4.1.99.0": ("rsuMode", "3"),
}

class CustomMibInstrumController(instrum.MibInstrumController):
    """Custom logic to handle SNMP GET requests using our dictionary"""
    def set_oid_data(self, data):
        self.oid_dict = {}
        for oid_str, (name, value) in data.items():
            oid = tuple([int(x) for x in oid_str.split('.')])
            if value.isdigit():
                mib_value = rfc1902.Integer(int(value))
            else:
                mib_value = rfc1902.OctetString(value.encode('utf-8'))
            self.oid_dict[oid] = (name, mib_value)

    def readVars(self, varBinds, acInfo=(None, None)):
        rspVarBinds = []
        for oid, val in varBinds:
            if oid in self.oid_dict:
                name, mib_value = self.oid_dict[oid]
                rspVarBinds.append((oid, mib_value))
                print(f"✅ GET: {name} ({oid}) -> {mib_value.prettyPrint()}")
            else:
                rspVarBinds.append((oid, v2c.NoSuchObject('')))
                print(f"❌ GET: Unknown OID {oid}")
        return rspVarBinds

def setup_snmp_server(listen_ip, listen_port, mib_version):
    snmpEngine = engine.SnmpEngine()

    # Transport
    config.addTransport(
        snmpEngine,
        udp.domainName,
        udp.UdpTransport().openServerMode((listen_ip, listen_port))
    )

    # Protocol mapping
    auth_protocol_map = {
        "SHA": config.usmHMACSHAAuthProtocol,
        "SHA-224": config.usmHMAC128SHA224AuthProtocol,
        "SHA-256": config.usmHMAC192SHA256AuthProtocol,
        "SHA-384": config.usmHMAC256SHA384AuthProtocol,
        "SHA-512": config.usmHMAC384SHA512AuthProtocol,
    }
    priv_protocol_map = {
        "AES": config.usmAesCfb128Protocol,
        "AES-128": config.usmAesCfb128Protocol,
        "AES-192": config.usmAesCfb192Protocol,
        "AES-256": config.usmAesCfb256Protocol,
    }

    # USM User Setup
    config.addV3User(
        snmpEngine,
        userName=RSU_CONFIG['user'],
        authProtocol=auth_protocol_map.get(RSU_CONFIG['authProtocol'], config.usmHMACSHAAuthProtocol),
        authKey=RSU_CONFIG['authPassPhrase'],
        privProtocol=priv_protocol_map.get(RSU_CONFIG['privacyProtocol'], config.usmAesCfb128Protocol),
        privKey=RSU_CONFIG['privacyPassPhrase']
    )

    # SNMP Context
    snmpContext = context.SnmpContext(snmpEngine)
    contextName = v2c.OctetString('')

    # Instrumentation Controller (The Logic)
    mibInstrum = snmpContext.getMibInstrum()
    customController = CustomMibInstrumController(mibInstrum.mibBuilder)
    
    # Load selected MIB data
    oid_data = NTCIP1218_OIDS if mib_version == "NTCIP1218" else RSU41_OIDS
    customController.set_oid_data(oid_data)

    # FIX: Unregister default context before registering our custom one
    snmpContext.unregisterContextName(contextName)
    snmpContext.registerContextName(contextName, customController)

    # Register Command Responder
    cmdrsp.GetCommandResponder(snmpEngine, snmpContext)

    print(f"🚀 Mock RSU ({mib_version}) running on {listen_ip}:{listen_port}")
    print(f"👤 User: {RSU_CONFIG['user']} | Auth: {RSU_CONFIG['authProtocol']} | Priv: {RSU_CONFIG['privacyProtocol']}")
    
    snmpEngine.transportDispatcher.jobStarted(1)
    
    try:
        snmpEngine.openDispatcher()
        asyncio.get_event_loop().run_forever()
    except KeyboardInterrupt:
        print("\n👋 Shutting down...")
    finally:
        snmpEngine.closeDispatcher()

def main():
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('--ip', default='0.0.0.0')
    parser.add_argument('--port', type=int, default=1610)
    parser.add_argument('--mib', choices=['NTCIP1218', 'RSU41'], default='NTCIP1218')
    args = parser.parse_args()

    try:
        setup_snmp_server(args.ip, args.port, args.mib)
    except PermissionError:
        print("❌ Error: Use 'sudo' to bind to port 1610, or use --port 1610")
    except Exception as e:
        print(f"❌ Critical Error: {e}")

if __name__ == "__main__":
    main()
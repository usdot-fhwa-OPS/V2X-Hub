#!/usr/bin/env python3
import j2735_202409
from binascii import unhexlify
from json2xml import json2xml
from json2xml.utils import readfromstring

# Monkey-patch imports to control JSON encoding behavior used by to_jer()
import pycrate_asn1rt.asnobj
import pycrate_core.elt as _core_elt
import pycrate_asn1rt.codecs as _asn_codecs

MESSAGE_FRAME = j2735_202409.MessageFrame.MessageFrame

class J2735_decode:
    '''
    Class to decode J2735 UPER hex to XML or JSON.

    Input - UPER hex payload

    Output - XML and JSON decoded J2735 message
    '''
    def __init__(self, payload, save=False):
        # Ensure JER JSON preserves insertion order (disable alphabetical sorting)
        # i.e., pycrate defaults to JSONEncoder(sort_keys=True). Override it here.
        try:
            # Recreate the encoder with sort_keys disabled
            _core_elt.JsonEnc = _core_elt.JSONEncoder(sort_keys=False, indent=1)
            # Propagate to modules that cached the encoder
            _asn_codecs.JsonEnc = _core_elt.JsonEnc
            pycrate_asn1rt.asnobj.JsonEnc = _core_elt.JsonEnc
        except Exception:
            pass

        decode = MESSAGE_FRAME
        decode.from_uper(unhexlify(payload))
        jer = decode.to_jer()

        self.xml = self.dict2xml(jer, save)
        self.json = self.writeJson(jer, save)

    def dict2xml(self, jer: str, save=False) -> str:
        '''
        Convert J2735 JER to XML.

        Parameters
        ----------
        jer (str): The J2735 JER string to convert.
        save (bool): Whether to save the XML to a file.

        Returns
        -------
        str: The converted XML string.
        '''
        jer_dict = readfromstring(jer)
        xml_str = json2xml.Json2xml(jer_dict, attr_type=False, wrapper="MessageFrame").to_xml()

        if save:
            with open("j2735decode.xml","w") as f:
                f.write(str(xml_str))
        return str(xml_str)

    def writeJson(self, jer: str, save=False) -> str:
        """
        Write J2735 JER to JSON file.

        Parameters
        ----------
        jer (str): The J2735 JER string to write.
        save (bool): Whether to save the JSON to a file.

        Returns
        -------
        str: The J2735 JER string.
        """
        if save:
            with open("j2735decode.json", "w") as f:
                f.write(jer)
        return jer

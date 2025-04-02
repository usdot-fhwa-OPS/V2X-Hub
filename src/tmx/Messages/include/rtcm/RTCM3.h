/*
 * RTCM3.h
 *
 * Message definitions for RTCM 3.3
 *
 *  Created on: Jun 1, 2018
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_RTCM_RTCM3_H_
#define INCLUDE_RTCM_RTCM3_H_


#include "RtcmMessage.h"
#include "RtcmDataManager.h"

namespace tmx {
namespace messages {
namespace rtcm {

enum RTCM3_MESSAGE_TYPE {
	L1OnlyGPSRTKObservable = 1001,
	ExtendedL1OnlyGPSRTKObservable = 1002,
	L1L2GPSRTKObservable = 1003,
	ExtendedL1L2GPSRTKObservable = 1004,
	StationaryRTKReferenceStationARP = 1005,
	StationaryRTKReferenceStationARPwithAntennaHeight = 1006,
	AntennaDescriptor = 1007,
	AntennaDescriptorAndSerialNumber = 1008,
	L1OnlyGLONASSRTKObservable = 1009,
	ExtendedL1OnlyGLONASSRTKObservable = 1010,
	L1L2GLONASSRTKObservable = 1011,
	ExtendedL1L2GLONASSRTKObservable = 1012,
	SystemParameters = 1013,
	NetworkAuxiliaryStationData = 1014,
	GPSIonosphericCorrectionDifferences = 1015,
	GPSGeometricCorrectionDifferences = 1016,
	GPSCombinedGeometricAndIonosphericCorrectionDifferences = 1017,
	AlternativeIonosphericCorrectDifferences = 1018,
	GPSEphmerides = 1019,
	GLONASSEphemerides = 1020,
	HelmertAbridgedMolodenskiTransformationParameters = 1021,
	MolodenskiBadekasTransformationParameters = 1022,
	ResidualsEllipsoidalGridRepresentation = 1023,
	ResidualsPlanGridRepresentation = 1024,
	ProjectionParametersProjectionTypesOtherLCC2SP = 1025,
	ProjectionParametersLCC2SP = 1026,
	ProjectionParametersOM = 1027,
	GlobalToPlateFixedTransformation = 1028,
	UnicodeTextString = 1029,
	GPSNetworkRTKResidualMessage = 1030,
	GLONASSNetworkRTKResidualMessage = 1031,
	PhysicalReferenceStationPositionMessage = 1032,
	ReceiverAndAntennaDescription = 1033,
	GPSNetworkFKPGradient = 1034,
	GLONASSNetworkFKPGradient = 1035,
	GLONASSIonosphericCorrectionDifferences = 1037,
	GLONASSGeometricCorrectionDifferences = 1038,
	GLONASSCombinedGeometricAndIonosphericCorrectionDifferences = 1039,
	BDSSatelliteEphemerisData = 1042,
	QZSSEphemerides = 1044,
	GalileoFNAVSatelliteEphemerisData = 1045,
	GalileoINAVSatelliteEphemerisData = 1046,
	SSRGPSOribitCorrection = 1057,
	SSRGPSClockCorrection = 1058,
	SSRGPSCodeBias = 1059,
	SSRGPSCombinedOrbitAndClockCorrection = 1060,
	SSRGPSURA = 1061,
	SSRGPSHighRateClockCorrection = 1062,
	SSRGLONASSOribitCorrection = 1063,
	SSRGLONASSClockCorrection = 1064,
	SSRGLONASSCodeBias = 1065,
	SSRGLONASSCombinedOrbitAndClockCorrection = 1066,
	SSRGLONASSURA = 1067,
	SSRGLONASSHighRateClockCorrection = 1068,
	GPSMSM1 = 1071,
	GPSMSM2 = 1072,
	GPSMSM3 = 1073,
	GPSMSM4 = 1074,
	GPSMSM5 = 1075,
	GPSMSM6 = 1076,
	GPSMSM7 = 1077,
	GLONASSMSM1 = 1081,
	GLONASSMSM2 = 1082,
	GLONASSMSM3 = 1083,
	GLONASSMSM4 = 1084,
	GLONASSMSM5 = 1085,
	GLONASSMSM6 = 1086,
	GLONASSMSM7 = 1087,
	GalileoMSM1 = 1091,
	GalileoMSM2 = 1092,
	GalileoMSM3 = 1093,
	GalileoMSM4 = 1094,
	GalileoMSM5 = 1095,
	GalileoMSM6 = 1096,
	GalileoMSM7 = 1097,
	SBASMSM1 = 1101,
	SBASMSM2 = 1102,
	SBASMSM3 = 1103,
	SBASMSM4 = 1104,
	SBASMSM5 = 1105,
	SBASMSM6 = 1106,
	SBASMSM7 = 1107,
	QZSSMSM1 = 1111,
	QZSSMSM2 = 1112,
	QZSSMSM3 = 1113,
	QZSSMSM4 = 1114,
	QZSSMSM5 = 1115,
	QZSSMSM6 = 1116,
	QZSSMSM7 = 1117,
	BeiDouMSM1 = 1121,
	BeiDouMSM2 = 1122,
	BeiDouMSM3 = 1123,
	BeiDouMSM4 = 1124,
	BeiDouMSM5 = 1125,
	BeiDouMSM6 = 1126,
	BeiDouMSM7 = 1127,
	GLONASSL1L2CodePhaseBiases = 1230,
	RTCM3_MESSAGE_TYPE_EOF
};

}

/**
 * Generic RTCM 3 message with transport layer contents
 */
template <>
class RTCMMessage<rtcm::SC10403_3>: public TmxRtcmMessage {
public:
	RTCMMessage<rtcm::SC10403_3>(): TmxRtcmMessage() { this->get_Preamble(); }
	RTCMMessage<rtcm::SC10403_3>(const TmxRtcmMessage &other): TmxRtcmMessage(other) { this->get_Preamble(); }
	virtual ~RTCMMessage() { }

	std_rtcm_attribute(this->msg, tmx::messages::rtcm::uint8,  Preamble, 0xD3, );
	std_rtcm_attribute(this->msg, tmx::messages::rtcm::uint6,  Reserved, 0, );
	std_rtcm_attribute(this->msg, tmx::messages::rtcm::uint10, MessageLength, 0, );
	std_rtcm_attribute(this->msg, tmx::messages::rtcm::uint12, MessageNumber, 0, );
	std_rtcm_attribute(this->msg, tmx::messages::rtcm::uint12, ReferenceStationID, 0, );
	std_rtcm_attribute(this->msg, tmx::messages::rtcm::uint24,  CRC, 0, );

protected:
	typedef rtcm::RtcmDataManager<rtcm::uint8> datamgr_type;
	typedef typename datamgr_type::rtcm_word rtcm_word;
	typedef typename datamgr_type::data_type data_type;

	datamgr_type mgr { this->msg };

public:
	virtual rtcm::RTCM_VERSION get_Version() { return rtcm::SC10403_3; }
	virtual rtcm::msgtype_type get_MessageType() { return this->get_MessageNumber(); }
	void clear() { TmxRtcmMessage::clear(); mgr.clearData(); }

	using tmx::message::set_contents;

	virtual void set_contents(const tmx::byte_stream &in) {
		TmxRtcmMessage::set_contents(in);

		tmx::byte_stream &bytes = getBytes();

		bool valid = false;
		size_t consumed = 0;

		try {
			do {
				// Look for the first header word in the incoming bytes
				while(bytes.size() > 0 && bytes[0] != Preamble::default_value())
					bytes.erase(bytes.begin());

				if (headerBytes > bytes.size()) {
					this->invalidate();
					return;
				}

				// Start the the minimal header
				consumed = mgr.assignData(bytes,
						_Preamble,
						_Reserved,
						_MessageLength
						);

				// Make sure there are enough bytes left
				if (get_MessageLength() + consumed + crcBytes > bytes.size()) {
					this->invalidate();
					return;
				}

				// Next, read the CRC from the end
				auto start = bytes.begin() + consumed + get_MessageLength();
				auto end = start + crcBytes;
				tmx::byte_stream crc(start, end);
				mgr.assignData(crc, _CRC);

				valid = get_Preamble() == Preamble::default_value() &&
						check_Parity(bytes);

				if (!valid) {
					// Reset for next try
					bytes.insert(start, crc.begin(), crc.end());
					bytes.erase(bytes.begin());
				}
			} while (!valid);

			// Load the rest of the data bytes
			bytes.erase(bytes.begin(), bytes.begin() + headerBytes);
			mgr.loadData(bytes, get_MessageLength());

			mgr.assignData(_MessageNumber, _ReferenceStationID);
		} catch (TmxException &ex) {
			this->invalidate();
			return;
		}
	}

	virtual tmx::byte_stream get_contents() {
		tmx::byte_stream bytes = mgr.extractAllData(
				get_MessageLength() - attrBytes,
				_Preamble,
				_Reserved,
				_MessageLength,
				_MessageNumber,
				_ReferenceStationID);

		// Add the CRC
		tmx::byte_stream crc = mgr.extractData(_CRC);
		bytes.insert(bytes.end(), crc.begin(), crc.end());
		return bytes;
	}

	tmx::messages::RtcmMessage get_RtcmMessage() {
		tmx::byte_stream msgContents = get_contents();

#if SAEJ2735_SPEC < 2016
		RTCM_Corrections *rtcm = (RTCM_Corrections *)calloc(1, sizeof(RTCM_Corrections));
		memset(rtcm, 0, sizeof(RTCM_Corrections));

		rtcm->msgID = DSRCmsgID_rtcmCorrections;
		rtcm->rev = RTCM_Revision_rtcmRev3_0;

		// Set the header
		rtcm->rtcmHeader.buf = (uint8_t *)malloc(5*sizeof(uint8_t));
		rtcm->rtcmHeader.size = 5;
		rtcm->rtcmHeader.buf[0] = 0x0E;
		rtcm->rtcmHeader.buf[1] = 0x00;
		rtcm->rtcmHeader.buf[2] = 0x00;
		rtcm->rtcmHeader.buf[3] = 0x00;
		rtcm->rtcmHeader.buf[4] = 0x00;

		// Copy the message bytes
		RTCMmsg *rtcmMessage = (RTCMmsg*)calloc(1, sizeof(RTCMmsg));
		rtcmMessage->payload.size = msgContents.size();
		rtcmMessage->payload.buf = (byte_t *)calloc(rtcmMessage->payload.size, sizeof(byte_t));
		memcpy(rtcmMessage->payload.buf, msgContents.data(), rtcmMessage->payload.size);

		ASN_SET_ADD(&rtcm->rtcmSets.list, rtcmMessage);
#else
		RTCMcorrections *rtcm = (RTCMcorrections *)calloc(1, sizeof(RTCMcorrections));
		memset(rtcm, 0, sizeof(RTCMcorrections));

		rtcm->rev = RTCM_Revision_rtcmRev3;

		// Copy the message bytes
		RTCMmessage_t *rtcmMessage = (RTCMmessage_t *)calloc(1, sizeof(RTCMmessage_t));
		rtcmMessage->size = msgContents.size();
		rtcmMessage->buf = (byte_t *)calloc(rtcmMessage->size, sizeof(byte_t));
		memcpy(rtcmMessage->buf, msgContents.data(), rtcmMessage->size);

		ASN_SET_ADD(&rtcm->msgs.list, rtcmMessage);
#endif
		rtcm->msgCnt = 0;

		return tmx::messages::RtcmMessage(rtcm);
	}

	virtual bool is_Valid() {
		if (!TmxRtcmMessage::is_Valid()) return false;

		try {
			const tmx::byte_stream data = get_contents();
			return this->get_Preamble() == Preamble::default_value() && check_Parity(data);
		} catch (TmxException &ex) {
			return false;
		}
	}

	void Validate() {
		const tmx::byte_stream data = get_contents();
		this->set_CRC(get_Parity(data));
	}

	size_t size() {
		return headerBytes + crcBytes + get_MessageLength();
	}

protected:
	typedef typename CRC::data_type crc_type;

	crc_type get_Parity(const tmx::byte_stream &bytes) {
		return crc24q_hash(bytes.data(), size() <= bytes.size() ? size() - crcBytes : bytes.size());
	}

	bool check_Parity(const tmx::byte_stream &bytes) {
		if (size() > bytes.size())
			return false;
		else
			return this->get_CRC() == get_Parity(bytes);
	}

private:
	static constexpr size_t headerBytes { (Preamble::size + Reserved::size + MessageLength::size) / datamgr_type::byteSize() };
	static constexpr size_t crcBytes { CRC::size / datamgr_type::byteSize() };
	static constexpr size_t attrBytes { (MessageNumber::size + ReferenceStationID::size) / datamgr_type::byteSize() };

	// Take from gpsd/crc24q.c per the GPSD license.

#define CRCSEED	0		/* could be NZ to detect leading zeros */
#define CRCPOLY	0x1864CFB	/* encodes all info about the polynomial */

	static crc_type get_HashVal(size_t n) {
		static constexpr crc_type crc24q[256] = {
			0x00000000, 0x01864CFB, 0x028AD50D, 0x030C99F6,
			0x0493E6E1, 0x0515AA1A, 0x061933EC, 0x079F7F17,
			0x08A18139, 0x0927CDC2, 0x0A2B5434, 0x0BAD18CF,
			0x0C3267D8, 0x0DB42B23, 0x0EB8B2D5, 0x0F3EFE2E,
			0x10C54E89, 0x11430272, 0x124F9B84, 0x13C9D77F,
			0x1456A868, 0x15D0E493, 0x16DC7D65, 0x175A319E,
			0x1864CFB0, 0x19E2834B, 0x1AEE1ABD, 0x1B685646,
			0x1CF72951, 0x1D7165AA, 0x1E7DFC5C, 0x1FFBB0A7,
			0x200CD1E9, 0x218A9D12, 0x228604E4, 0x2300481F,
			0x249F3708, 0x25197BF3, 0x2615E205, 0x2793AEFE,
			0x28AD50D0, 0x292B1C2B, 0x2A2785DD, 0x2BA1C926,
			0x2C3EB631, 0x2DB8FACA, 0x2EB4633C, 0x2F322FC7,
			0x30C99F60, 0x314FD39B, 0x32434A6D, 0x33C50696,
			0x345A7981, 0x35DC357A, 0x36D0AC8C, 0x3756E077,
			0x38681E59, 0x39EE52A2, 0x3AE2CB54, 0x3B6487AF,
			0x3CFBF8B8, 0x3D7DB443, 0x3E712DB5, 0x3FF7614E,
			0x4019A3D2, 0x419FEF29, 0x429376DF, 0x43153A24,
			0x448A4533, 0x450C09C8, 0x4600903E, 0x4786DCC5,
			0x48B822EB, 0x493E6E10, 0x4A32F7E6, 0x4BB4BB1D,
			0x4C2BC40A, 0x4DAD88F1, 0x4EA11107, 0x4F275DFC,
			0x50DCED5B, 0x515AA1A0, 0x52563856, 0x53D074AD,
			0x544F0BBA, 0x55C94741, 0x56C5DEB7, 0x5743924C,
			0x587D6C62, 0x59FB2099, 0x5AF7B96F, 0x5B71F594,
			0x5CEE8A83, 0x5D68C678, 0x5E645F8E, 0x5FE21375,
			0x6015723B, 0x61933EC0, 0x629FA736, 0x6319EBCD,
			0x648694DA, 0x6500D821, 0x660C41D7, 0x678A0D2C,
			0x68B4F302, 0x6932BFF9, 0x6A3E260F, 0x6BB86AF4,
			0x6C2715E3, 0x6DA15918, 0x6EADC0EE, 0x6F2B8C15,
			0x70D03CB2, 0x71567049, 0x725AE9BF, 0x73DCA544,
			0x7443DA53, 0x75C596A8, 0x76C90F5E, 0x774F43A5,
			0x7871BD8B, 0x79F7F170, 0x7AFB6886, 0x7B7D247D,
			0x7CE25B6A, 0x7D641791, 0x7E688E67, 0x7FEEC29C,
			0x803347A4, 0x81B50B5F, 0x82B992A9, 0x833FDE52,
			0x84A0A145, 0x8526EDBE, 0x862A7448, 0x87AC38B3,
			0x8892C69D, 0x89148A66, 0x8A181390, 0x8B9E5F6B,
			0x8C01207C, 0x8D876C87, 0x8E8BF571, 0x8F0DB98A,
			0x90F6092D, 0x917045D6, 0x927CDC20, 0x93FA90DB,
			0x9465EFCC, 0x95E3A337, 0x96EF3AC1, 0x9769763A,
			0x98578814, 0x99D1C4EF, 0x9ADD5D19, 0x9B5B11E2,
			0x9CC46EF5, 0x9D42220E, 0x9E4EBBF8, 0x9FC8F703,
			0xA03F964D, 0xA1B9DAB6, 0xA2B54340, 0xA3330FBB,
			0xA4AC70AC, 0xA52A3C57, 0xA626A5A1, 0xA7A0E95A,
			0xA89E1774, 0xA9185B8F, 0xAA14C279, 0xAB928E82,
			0xAC0DF195, 0xAD8BBD6E, 0xAE872498, 0xAF016863,
			0xB0FAD8C4, 0xB17C943F, 0xB2700DC9, 0xB3F64132,
			0xB4693E25, 0xB5EF72DE, 0xB6E3EB28, 0xB765A7D3,
			0xB85B59FD, 0xB9DD1506, 0xBAD18CF0, 0xBB57C00B,
			0xBCC8BF1C, 0xBD4EF3E7, 0xBE426A11, 0xBFC426EA,
			0xC02AE476, 0xC1ACA88D, 0xC2A0317B, 0xC3267D80,
			0xC4B90297, 0xC53F4E6C, 0xC633D79A, 0xC7B59B61,
			0xC88B654F, 0xC90D29B4, 0xCA01B042, 0xCB87FCB9,
			0xCC1883AE, 0xCD9ECF55, 0xCE9256A3, 0xCF141A58,
			0xD0EFAAFF, 0xD169E604, 0xD2657FF2, 0xD3E33309,
			0xD47C4C1E, 0xD5FA00E5, 0xD6F69913, 0xD770D5E8,
			0xD84E2BC6, 0xD9C8673D, 0xDAC4FECB, 0xDB42B230,
			0xDCDDCD27, 0xDD5B81DC, 0xDE57182A, 0xDFD154D1,
			0xE026359F, 0xE1A07964, 0xE2ACE092, 0xE32AAC69,
			0xE4B5D37E, 0xE5339F85, 0xE63F0673, 0xE7B94A88,
			0xE887B4A6, 0xE901F85D, 0xEA0D61AB, 0xEB8B2D50,
			0xEC145247, 0xED921EBC, 0xEE9E874A, 0xEF18CBB1,
			0xF0E37B16, 0xF16537ED, 0xF269AE1B, 0xF3EFE2E0,
			0xF4709DF7, 0xF5F6D10C, 0xF6FA48FA, 0xF77C0401,
			0xF842FA2F, 0xF9C4B6D4, 0xFAC82F22, 0xFB4E63D9,
			0xFCD11CCE, 0xFD575035, 0xFE5BC9C3, 0xFFDD8538,
		};

		return crc24q[n];
	}

	static crc_type crc24q_hash(const unsigned char *data, int len)
	{
		int i;
		crc_type crc = 0;

		for (i = 0; i < len; i++) {
		crc = (crc << 8) ^ get_HashVal(data[i] ^ (unsigned char)(crc >> 16));
		}

		crc = (crc & 0x00ffffff);

		return crc;
	}
};

typedef RTCMMessage<rtcm::SC10403_3> RTCM3Message;
typedef RTCMMessageType<rtcm::SC10403_3, 0> RTCM3UnknownMessage;

template <>
class RTCMMessageType<rtcm::SC10403_3, 0>: public RTCM3Message {
public:
	RTCMMessageType<rtcm::SC10403_3, 0>(): RTCM3Message() { }
	RTCMMessageType<rtcm::SC10403_3, 0>(const TmxRtcmMessage &other): RTCM3Message(other) { }

	using tmx::message::set_contents;

	void set_contents(const tmx::byte_stream &in) {
		RTCM3Message::set_contents(in);

		// Add an array of the data bytes to the container
		message_tree_type children;
		for (size_t i = 0; i < mgr.numDataWords(); i++) {
			data_type val = mgr.getData(i);
			char buf[13];
			sprintf(buf, "0x%02x", val);
			children.push_back(std::make_pair("", message_tree_type(std::string(buf))));
		}

		if (!children.empty())
			this->as_tree().get().add_child("data", children);
	}
};

typedef RTCMMessageType<rtcm::SC10403_3, rtcm::RTCM3_MESSAGE_TYPE::AntennaDescriptor> RTCM3AntennaDescriptorMessage;

/*
 * Some pre-defined message types for RTCM 2.3 that may be useful
 */
template <>
class RTCMMessageType<rtcm::SC10403_3, rtcm::RTCM3_MESSAGE_TYPE::AntennaDescriptor>: public RTCM3Message {
public:
	RTCMMessageType<rtcm::SC10403_3, rtcm::RTCM3_MESSAGE_TYPE::AntennaDescriptor>(): RTCM3Message() { }
	RTCMMessageType<rtcm::SC10403_3, rtcm::RTCM3_MESSAGE_TYPE::AntennaDescriptor>(const TmxRtcmMessage &other): RTCM3Message(other) { }

	using tmx::message::set_contents;

	std_rtcm_attribute(this->msg, rtcm::uint8, DescriptorCounter, 0, );
	std_attribute(this->msg, std::string, AntennaDescriptor, "", );
	std_rtcm_attribute(this->msg, rtcm::uint8, SetupID, 0, );

	void set_contents(const tmx::byte_stream &in) {
		RTCM3Message::set_contents(in);

		mgr.assignData(_DescriptorCounter);

		std::string antDesc;
		for (size_t i = 0; i < get_DescriptorCounter(); i++)
			antDesc.push_back(mgr.getData(i));

		this->set_AntennaDescriptor(antDesc);

		mgr.eraseData(get_DescriptorCounter());

		mgr.assignData(_SetupID);
	}

	tmx::byte_stream get_contents() {
		// Rebuild the data buffer
		tmx::byte_stream bytes = mgr.extractData(_DescriptorCounter);
		mgr.loadData(bytes);

		bytes = mgr.extractData(_SetupID);

		std::string antDesc = this->get_AntennaDescriptor();
		bytes.insert(bytes.begin(), antDesc.begin(), antDesc.end());
		mgr.loadData(bytes, bytes.size());

		return RTCM3Message::get_contents();
	}
};

namespace rtcm {

/**
 * Available RTCM 3 message types
 */
template <>
struct RtcmMessageTypeBox<SC10403_3> {
	typedef std::tuple<
				tmx::messages::RTCM3UnknownMessage,
				tmx::messages::RTCM3AntennaDescriptorMessage
			> types;
};

} /* End namespace rtcm */

} /* End namespace messages */
} /* End namespace tmx */



#endif /* INCLUDE_RTCM_RTCM3_H_ */

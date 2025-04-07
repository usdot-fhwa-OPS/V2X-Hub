/*
 * RTCM2.h
 *
 * Message definitions for RTCM 2.3
 *
 *  Created on: Apr 26, 2018
 *      @author: gmb
 */

#ifndef INCLUDE_RTCM_RTCM2_H_
#define INCLUDE_RTCM_RTCM2_H_

#include "RtcmMessage.h"
#include "RtcmDataManager.h"

namespace tmx {
namespace messages {
namespace rtcm {

enum RTCM2_MESSAGE_TYPE {
	DifferentialGPSCorrections = 1,
	DeltaDifferentialGPSCorrections = 2,
	GPSReferenceStationParameters = 3,
	ReferenceStationDatum = 4,
	GPSConstellationHealth = 5,
	GPSNullFrame = 6,
	DGPSRadiobeaconAlmanac = 7,
	PseudoliteAlmanac = 8,
	GPSPartialCorrectionSet = 9,
	P_CodeDifferentialCorrections = 10,
	CA_CodeL1L2DeltaCorrections = 11,
	PseudoliteStationParameters = 12,
	GroundTransmitterParameters = 13,
	GPSTimeOfWeek = 14,
	IonosphericDelayMessage = 15,
	GPSSpecialMessage = 16,
	GPSEphemerides = 17,
	RTKUncorrectedCarrierPhases = 18,
	RTKUncorrectedPseudoranges = 19,
	RTKCarrierPhaseCorrections = 20,
	RTKHi_AccuracyPseudorangeCorrections = 21,
	ExtendedReferenceStationParameters = 22,
	AntennaTypeDefinitionRecord = 23,
	AntennaReferentPoint = 24,
	ExtendedRadiobeaconAlmanac = 27,
	DifferentialGLONASSCorrections = 31,
	DifferentialGLONASSReferenceStationParameters = 32,
	GLONASSConstellationHealth = 33,
	GLONASSPartialDifferentialCorrectionSet = 34,
	GLONASSRadiobeaconAlmanac = 35,
	GLONASSSpecialMessage = 36,
	GNSSSystemTimeOffset = 37,
	RTCM2_MESSAGE_TYPE_EOF
};

} /* End namespace rtcm */

typedef RTCMMessage<rtcm::RTCM_VERSION::SC10402_3> RTCM2Message;

/**
 * Generic RTCM 2 message with header contents
 */
template <>
class RTCMMessage<rtcm::RTCM_VERSION::SC10402_3>: public TmxRtcmMessage {
public:
	enum StationHealthValue {
		UDREScaleFactor1_0 = 0,
		UDREScaleFactor0_75,
		UDREScaleFactor0_5S,
		UDREScaleFactor0_3,
		UDREScaleFactor0_2,
		UDREScaleFactor0_1,
		NotMontitored,
		NotWorking
	};


	RTCMMessage<rtcm::RTCM_VERSION::SC10402_3>(): TmxRtcmMessage() { this->get_Preamble(); }
	RTCMMessage<rtcm::RTCM_VERSION::SC10402_3>(const TmxRtcmMessage &other): TmxRtcmMessage(other) { this->get_Preamble(); }
	virtual ~RTCMMessage<rtcm::RTCM_VERSION::SC10402_3>() { }

	std_rtcm_attribute(this->msg, tmx::messages::rtcm::uint8,  Preamble, 0x66, );
	std_rtcm_attribute(this->msg, tmx::messages::rtcm::uint6,  FrameID, 0, );
	std_rtcm_attribute(this->msg, tmx::messages::rtcm::uint10, StationID, 0, );
	std_rtcm_attribute(this->msg, tmx::messages::rtcm::uint6,  Parity1, 0, );
	std_rtcm_attribute(this->msg, tmx::messages::rtcm::uint13, ModifiedZCount, 0, );
	std_rtcm_attribute(this->msg, tmx::messages::rtcm::uint3,  SequenceNumber, 0, );
	std_rtcm_attribute(this->msg, tmx::messages::rtcm::uint5,  NumberDataWords, 0, );
	std_rtcm_attribute(this->msg, tmx::messages::rtcm::uint3,  StationHealth, 0, );
	std_rtcm_attribute(this->msg, tmx::messages::rtcm::uint6,  Parity2, 0, );

	double get_ZCount() {
		return 0.6 * get_ModifiedZCount();
	}

	void set_ZCount(double zCount) {
		set_ModifiedZCount((ModifiedZCount::data_type)(zCount / 0.6));
	}

protected:
	typedef rtcm::RtcmDataManager<rtcm::uint30, rtcm::uint6> datamgr_type;
	typedef typename datamgr_type::rtcm_word rtcm_word;
	typedef typename datamgr_type::data_type data_type;

	datamgr_type mgr { this->msg, 0x40 };
public:
	using tmx::message::set_contents;

	virtual rtcm::RTCM_VERSION get_Version() { return rtcm::SC10402_3; }
	virtual rtcm::msgtype_type get_MessageType() { return this->get_FrameID(); }

	void set_PreviousMessage (TmxRtcmMessage *ptr) {
		if (ptr && ptr->get_Version() == get_Version()) {
			primerWord = dynamic_cast<RTCM2Message *>(ptr)->get_LastWord();
		}
	}

	void clear() { TmxRtcmMessage::clear(); mgr.clearData(); }

	virtual void set_contents(const tmx::byte_stream &in) {
		static Preamble::data_type complPreamble = Preamble::default_value() ^ 0x3F;

		TmxRtcmMessage::set_contents(in);

		tmx::byte_stream &bytes = getBytes();
		size_t consumed = 0;

		try {
			bool valid = false;

			do
			{
				// Look for the first header word in the incoming bytes
				while(bytes.size() > 0 && bytes[0] != Preamble::default_value() && bytes[0] != complPreamble)
					bytes.erase(bytes.begin());

				if (2 * mgr.wordSize() > bytes.size()) {
					this->invalidate();
					return;
				}

				mgr.loadData(bytes, 2);

				// Complement the data bits if the previous word ends in 1
				if (primerWord) {
					if (primerWord & 1)
						mgr.replaceData(0, mgr.getData(0) ^ data_bitmask);
				} else {
					data_type tmp = mgr.getData(0);

					// Just use trial and error based on the parity
					for (data_type p : { 0x0, 0x1, 0x2, 0x3 }) {
						primerWord = p;
						if (p)
							tmp = tmp ^ data_bitmask;

						if ((tmp >> (rtcm_word::size - Preamble::size)) == Preamble::default_value() &&
								check_Parity(tmp, primerWord)) {
							// This is the correct primerWord
							mgr.replaceData(0, tmp);
							break;
						}
					}
				}

				if (mgr.getData(0) & 1)
					mgr.replaceData(1, mgr.getData(1) ^ data_bitmask);

				consumed = mgr.assignData(_Preamble,
					_FrameID,
					_StationID,
					_Parity1,
					_ModifiedZCount,
					_SequenceNumber,
					_NumberDataWords,
					_StationHealth,
					_Parity2);

				valid = this->is_Valid();

				if (!valid) {
					// Reset for next try
					bytes.erase(bytes.begin());
					primerWord = 0;
				}
			} while (!valid);

			// Load the data bytes
			bytes.erase(bytes.begin(), bytes.begin() + consumed);
			if (bytes.size() < mgr.wordSize() * get_NumberDataWords()) {
				this->invalidate();
				return;
			}

			mgr.loadData(bytes, get_NumberDataWords());

			// Recycle through and complement the data bytes when necessary
			data_type lastWord = get_Parity2();
			for (size_t i = 0; i < get_NumberDataWords(); i++) {
				if (lastWord & 1)
					mgr.replaceData(i, mgr.getData(i) ^ data_bitmask);
				lastWord = mgr.getData(i);
			}

		} catch (TmxException &ex) {
			this->invalidate();
			return;
		}
	}

	virtual tmx::byte_stream get_contents() {
		tmx::byte_stream bytes;
		data_type hdr1 = get_Header1();
		data_type hdr2 = get_Header2();

		if (primerWord & 1)
			hdr1 = hdr1 ^ data_bitmask;
		if (hdr1 & 1)
			hdr2 = hdr2 ^ data_bitmask;

		mgr.writeWord(hdr1, bytes);
		mgr.writeWord(hdr2, bytes);

		data_type prev = hdr2;
		for (size_t i = 0; i < mgr.numDataWords(); i++)
		{
			data_type w = mgr.getData(i);
			if (prev & 1)
				w = w ^ data_bitmask;

			mgr.writeWord(w, bytes);
			prev = w;
		}

		return bytes;
	}

	data_type get_Header1() {
		return get_Rtcm2Header()[0];
	}

	data_type get_Header2() {
		return get_Rtcm2Header()[1];
	}

	virtual bool is_Valid() {
		data_type h1 = get_Header1();
		data_type h2 = get_Header2();
		return TmxRtcmMessage::is_Valid() &&
				(h1 >> (rtcm_word::size - Preamble::size)) == this->get_Preamble() &&
				this->get_Preamble() == Preamble::default_value() &&
				this->check_Parity(h1, primerWord) &&
				this->check_Parity(h2, this->get_Parity1());
	}

	data_type add_Parity(data_type word, data_type prevWord) {
		typename rtcm_word::bitset w(word);

		tmx::byte_t D29 = rtcm::get_Bit<rtcm_word, 29>(prevWord);
		tmx::byte_t D30 = rtcm::get_Bit<rtcm_word, 30>(prevWord);

		w[5] = D29 ^
				rtcm::bit_manipulator<1, 2, 3, 5, 6, 10, 11, 12, 13, 14, 17, 18, 20, 23>::xor_bits<rtcm_word>(word);
		w[4] = D30 ^
				rtcm::bit_manipulator<2, 3, 4, 6, 7, 11, 12, 13, 14, 15, 18, 19, 21, 24>::xor_bits<rtcm_word>(word);
		w[3] = D29 ^
				rtcm::bit_manipulator<1, 3, 4, 5, 7, 8, 12, 13, 14, 15, 16, 19, 20, 22>::xor_bits<rtcm_word>(word);
		w[2] = D30 ^
				rtcm::bit_manipulator<2, 4, 5, 6, 8, 9, 13, 14, 15, 16, 17 ,20, 21, 23>::xor_bits<rtcm_word>(word);
		w[1] = D30 ^
				rtcm::bit_manipulator<1, 3, 5, 6, 7, 9, 10, 14, 15, 16, 17, 18, 21, 22, 24>::xor_bits<rtcm_word>(word);
		w[0] = D29 ^
				rtcm::bit_manipulator<3, 5, 6, 8, 9, 10, 11, 13, 15, 19, 22, 23, 24>::xor_bits<rtcm_word>(word);

		return (data_type)w.to_ulong();
	}

	void Validate() {
		this->set_Parity1(add_Parity(get_Header1(), 0) & Parity1::bitmask());
		this->set_Parity2(add_Parity(get_Header2(), this->get_Parity1()) & Parity2::bitmask());
	}

	size_t size() {
		return mgr.wordSize() * (2 + get_NumberDataWords());
	}

	virtual data_type get_LastWord() {
		if (mgr.numDataWords() > 0)
			return mgr.getData(mgr.numDataWords() - 1);

		return get_Parity2();
	}

	virtual TmxRtcmMessage *clone() { return new RTCMMessage<rtcm::SC10402_3>(); }

	tmx::messages::RtcmMessage get_RtcmMessage() {
		tmx::byte_stream msgContents = get_contents();

#if SAEJ2735_SPEC < 2016
		RTCM_Corrections *rtcm = (RTCM_Corrections *)calloc(1, sizeof(RTCM_Corrections));
		memset(rtcm, 0, sizeof(RTCM_Corrections));

		rtcm->msgID = DSRCmsgID_rtcmCorrections;
		rtcm->rev = RTCM_Revision_rtcmRev2_3;

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

		rtcm->rev = RTCM_Revision_rtcmRev2;

		// Copy the message bytes
		RTCMmessage_t *rtcmMessage = (RTCMmessage_t *)calloc(1, sizeof(RTCMmessage_t));
		rtcmMessage->size = msgContents.size();
		rtcmMessage->buf = (byte_t *)calloc(rtcmMessage->size, sizeof(byte_t));
		memcpy(rtcmMessage->buf, msgContents.data(), rtcmMessage->size);

		ASN_SET_ADD(&rtcm->msgs.list, rtcmMessage);
#endif
		rtcm->msgCnt = this->get_SequenceNumber();

		return tmx::messages::RtcmMessage(rtcm);
	}

protected:
	bool check_Parity(data_type word, data_type prevWord) {
		return (word == add_Parity(word, prevWord));
	}

	std::vector<data_type> &get_Rtcm2Header() {
		// Check to see if the container changed
		if (headerVersion != msg.get_storage_version()) {
			rtcm2Hdr = mgr.extractWords(_Preamble,
										_FrameID,
										_StationID,
										_Parity1,
										_ModifiedZCount,
										_SequenceNumber,
										_NumberDataWords,
										_StationHealth,
										_Parity2);
			headerVersion = msg.get_storage_version();
		}

		while (rtcm2Hdr.size() < 2)
			rtcm2Hdr.push_back(0);

		return rtcm2Hdr;
	}
private:
	data_type data_bitmask { rtcm_word::bitmask() ^ Parity1::bitmask() };
	std::vector<data_type> rtcm2Hdr;
	int headerVersion = msg.get_storage_version();
	data_type primerWord = 0;
};

typedef RTCMMessageType<rtcm::SC10402_3, 0> RTCM2UnknownMessage;

/*
 * Generic empty message type for RTCM 2.3.
 */
template<>
class RTCMMessageType<rtcm::SC10402_3, 0>: public RTCM2Message {
public:
	RTCMMessageType<rtcm::SC10402_3, 0>():
		RTCM2Message() { }
	RTCMMessageType<rtcm::SC10402_3, 0>(const TmxRtcmMessage &other):
		RTCM2Message(other) { }

	void set_contents(const tmx::byte_stream &in) {
		RTCM2Message::set_contents(in);

		// Add an array of the data bytes to the container
		message_tree_type children;
		for (size_t i = 0; i < mgr.numDataWords(); i++) {
			data_type val = mgr.getData(i);
			char buf[13];
			sprintf(buf, "0x%08x", val);
			children.push_back(std::make_pair("", message_tree_type(std::string(buf))));
		}

		if (!children.empty())
			this->as_tree().get().add_child("data", children);
	}
};

typedef RTCMMessageType<rtcm::SC10402_3, rtcm::RTCM2_MESSAGE_TYPE::GPSSpecialMessage> RTCM2GPSSpecialMessage;

/*
 * Some pre-defined message types for RTCM 2.3 that may be useful
 */
template <>
class RTCMMessageType<rtcm::SC10402_3, rtcm::RTCM2_MESSAGE_TYPE::GPSSpecialMessage>:
	public RTCM2Message {
	using super = RTCM2Message;
public:
	RTCMMessageType<rtcm::SC10402_3, rtcm::RTCM2_MESSAGE_TYPE::GPSSpecialMessage>():
		super() { this->set_FrameID(rtcm::RTCM2_MESSAGE_TYPE::GPSSpecialMessage); }
	RTCMMessageType<rtcm::SC10402_3, rtcm::RTCM2_MESSAGE_TYPE::GPSSpecialMessage>(const TmxRtcmMessage &other):
		super(other) { this->set_FrameID(rtcm::RTCM2_MESSAGE_TYPE::GPSSpecialMessage); }
	virtual ~RTCMMessageType<rtcm::SC10402_3, rtcm::RTCM2_MESSAGE_TYPE::GPSSpecialMessage>() { }

	std_attribute(this->msg, std::string, SpecialMessage, "", assignMessage(value); );

	void set_contents(const tmx::byte_stream &in) {
		typedef rtcm::uint8 charByte;

		super::set_contents(in);

		// Consume the words in the data buffer
		data_type prevWord = this->get_Parity2();
		std::string theMessage;
		for (size_t i = 0; i < mgr.numDataWords(); i++) {
			data_type word = mgr.getData(i);
			if (!this->check_Parity(word, prevWord)) {
				this->invalidate();
				return;
			}

			prevWord = word;

			word >>= Parity1::size;
			for (size_t n = 0; n < 3; n++) {
				char c = (char)((word >> (charByte::size * (2 - n))) & charByte::bitmask());
				if (n > 0 && c == 0x00) continue;
				theMessage.push_back(c);
			}
		}

		_SpecialMessage.set(this->msg, theMessage);
	}

private:
	void assignMessage(std::string message) {
		// Reset the data bytes
		mgr.clearData();
		this->set_NumberDataWords(message.length() / 3 + (message.length() %3 == 0 ? 0 : 1));
		this->Validate();

		data_type prevWord = get_Header2();

		for (size_t i = 0; i < message.length(); i+=3) {
			data_type word = (message[i] << 22);
			word |= (i+1 < message.length() ? (message[i+1] << 14) : 0);
			word |= (i+2 < message.length() ? (message[i+2] <<  6) : 0);

			data_type w = this->add_Parity(word, prevWord);

			prevWord = w;
			mgr.loadWord(w);
		}
	}
};

namespace rtcm {

/**
 * Available RTCM 2 message types
 */
template <>
struct RtcmMessageTypeBox<SC10402_3> {
	typedef std::tuple<
				tmx::messages::RTCM2UnknownMessage,
				tmx::messages::RTCM2GPSSpecialMessage
			> types;
};

} /* End namespace rtcm */
} /* End namespace messages */
} /* End namespace tmx */



#endif /* INCLUDE_RTCM_RTCM2_H_ */

/*
 * RtcmFrameHelper.h
 *
 * This header contains some helper functions for RTCM message frames.
 *
 *  Created on: May 15, 2018
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_RTCM_RTCMDATAMANAGER_H_
#define INCLUDE_RTCM_RTCMDATAMANAGER_H_

#include <tmx/TmxException.hpp>
#include <tmx/messages/message.hpp>
#include <vector>

#include "RtcmVersion.h"
#include "RtcmTypes.h"

namespace tmx {
namespace messages {
namespace rtcm {

/***
 * A template class that can manage RTCM data of various sizes.  The RtcmWord
 * is the number of bits that make up a word for this set of data, which
 * defaults to one byte.  The UsableBits helps identify the cases where the
 * word is encoded in less than a byte, and presuming that each byte is "rolled".
 */
template <typename RtcmWord = uint8, typename Byte = uint8>
class RtcmDataManager {

public:
	typedef RtcmWord rtcm_word;
	typedef typename rtcm_word::data_type data_type;
	typedef Byte byte_type;

	RtcmDataManager(tmx::message_container_type &container, tmx::byte_t rollMask = 0x00):
		msg(container), mask(rollMask) { }

	/**
	 * Return the size (in bits) of a useful byte in a data word.  Most of the time this
	 * would be 8, except if some bits in the byte are fixed as overhead.
	 */
	static constexpr size_t byteSize() {
		return Byte::size;
	}

	/**
	 * Return the number of bytes in this RTCM word.  By default, this would be
	 * 1 for RtcmWord as 8 bits and UsableBits as 8 bits.
	 *
	 * @return The number of bytes in this RTCM word.
	 */
	static constexpr size_t wordSize() {
		return rtcm_word::size / byteSize();
	}

	/**
	 * @return The number of words in the data buffer
	 */
	size_t numDataWords() {
		return _data.size();
	}

	/**
	 * @return The number of bytes in the data buffer
	 */
	size_t numDataBytes() {
		return wordSize() * numDataWords();
	}

	/**
	 * Clears the data buffer.
	 */
	void clearData() {
		_data.clear();
	}

	/**
	 * Erases a portion of the data buffer
	 */
	void eraseData(size_t num) {
		_data.erase(_data.begin(), _data.begin() + num);
	}

	template <typename... RtcmAttrs>
	static constexpr size_t countBits() {
		return size_checker<RtcmAttrs...>::size;
	}

	template <typename... RtcmAttrs>
	static constexpr size_t countWords() {
		return checkCount<countBits<RtcmAttrs...>()>();
	}

	/**
	 * Access a data word
	 */
	data_type getData(size_t word) {
		if (word < _data.size())
			return _data[word];
		else
			return 0;
	}

	void replaceData(size_t word, data_type value) {
		if (word < _data.size())
			_data[word] = value;
	}

	/**
	 * Writes the data word out as a byte stream.
	 *
	 * @param word The data word to write
	 * @param bytes The byte stream to write to
	 * @return The number of bytes written to the stream	 *
	 */
	size_t writeWord(data_type word, tmx::byte_stream &bytes) {
		size_t oldSize = bytes.size();

		for (size_t i = 0; i < wordSize(); i++)
			splitWord<Byte>(bytes, word, (wordSize() - i - 1));

		// Roll the bytes if necessary
		if (mask > 0) {
			for (size_t i = oldSize; i < bytes.size(); i++)
				bytes[i] = rollByte<Byte>(bytes[i]) | mask;
		}

		return bytes.size() - oldSize;
	}

	/**
	 * Pulls out words of data from the buffer as a stream of bytes up to the given number of words,
	 * which defaults to 1.  If there is not enough bytes in the data buffer to read in the
	 * given number of words, then an exception is thrown.
	 *
	 * @param numWords The number of words to retrieve
	 * @return The byte stream for the data words
	 */
	tmx::byte_stream retrieveData(size_t numWords = 1) {
		checkAvailableWords(numWords);

		tmx::byte_stream bytes;

		for (size_t w = 0; w < numWords; w++)
			writeWord(getData(w), bytes);

		return bytes;
	}

	/**
	 * Extracts the assigned data from the given attributes back to an array of data words.  This
	 * operation does not affect the data buffer.
	 *
	 * @param attrs... The set of RTCM attributes to extract
	 * @return The set of data words in order of the extracted attributes
	 */
	template <typename... RtcmAttrs>
	std::vector<data_type> extractWords(RtcmAttrs... attrs) {
		static constexpr size_t numWords = countWords<RtcmAttrs...>();

		uintmax_t value = 0;
		extractFrom(value, countBits<RtcmAttrs...>(), attrs...);

		std::vector<data_type> words;
		for (size_t w = 0; w < numWords; w++)
			splitWord<rtcm_word>(words, value, (numWords - w - 1));

		return words;
	}

	/**
	 * Extracts the assigned data from the given attributes and builds a stream of bytes out of those
	 * words.  This operation does not affect the data buffer.
	 *
	 * @aparam attrs... The set of RTCM attributes to extract
	 * @return The byte stream of attribute data
	 */
	template <typename... RtcmAttrs>
	tmx::byte_stream extractData(RtcmAttrs... attrs) {
		std::vector<data_type> words = extractWords(attrs...);

		tmx::byte_stream bytes;
		for (size_t i = 0; i < words.size(); i++)
			writeWord(words[i], bytes);

		return bytes;
	}

	/**
	 * Extracts the assigned data from the given attributes and builds a stream of bytes out of those
	 * words, then appends the words from the data buffer up to the given number of words.  If there is
	 * not enough bytes in the data buffer to read in the given number of words, then an exception is
	 * thrown.
	 *
	 * @param numWords The number of words to retrieve from the data buffer
	 * @param attrs... The set of RTCM attributes to extract
	 * @return The byte stream of data
	 */
	template <typename... RtcmAttrs>
	tmx::byte_stream extractAllData(size_t numWords, RtcmAttrs... attrs) {
		tmx::byte_stream attrData = extractData(attrs...);
		tmx::byte_stream wordData = retrieveData(numWords);

		// Append the data words.
		attrData.insert(attrData.end(), wordData.begin(), wordData.end());
		return attrData;
	}

	/**
	 * Loads a word into the data buffer.
	 */
	void loadWord(data_type word) {
		_data.push_back(word);
	}

	/**
	 * Read a word of data from the incoming byte stream, starting at the given position.
	 *
	 * @param bytes The incoming byte stream
	 * @param pos The byte to start from
	 * @return The word value
	 *
	 */
	data_type readWord(const tmx::byte_stream &bytes, size_t pos = 0) {
		data_type val = 0;

		for (size_t i = pos; i < bytes.size() && i < pos + wordSize(); i++) {
			auto byte = bytes[i];

			// See if a roll is needed
			if (mask > 0 && (byte & mask) == mask)
				byte = rollByte<Byte>(byte);

			val = combineWord<Byte>(byte, val);
		}

		return val;
	}

	/**
	 * Loads the data buffer from a stream of bytes, up to a given number of words, which defaults to 1.
	 * If the data buffer contains less than the number of bytes needed to read in any word, then the last
	 * word is truncated and no further words are read.  This may lead to an exception later if the number
	 * of words in the buffer is insufficient to read in the RTCM data.
	 *
	 * @param bytes The byte stream to load from
	 * @param numWords The number of words to read (from the start of the stream)
	 * @return The total number of bytes read into the buffer
	 */
	size_t loadData(const tmx::byte_stream &bytes, size_t numWords = 1) {
		size_t oldSize = numDataBytes();
		for (size_t i = 0; i < numWords; i++)
			loadWord(readWord(bytes, i * wordSize()));

		return numDataBytes() - oldSize;
	}

	/**
	 * Assign data to the given attributes by the current contents of the data buffer.  Note that
	 * the attributes must be declared as one of the RTCM types, which declares the number of
	 * bits used.
	 *
	 * @param attrs... The set of RTCM attributes to assign
	 * @return The total number of bytes consumed from the buffer
	 */
	template <typename... RtcmAttrs>
	size_t assignData(RtcmAttrs... attrs) {
		static constexpr size_t numWords = countWords<RtcmAttrs...>();

		checkAvailableWords(numWords);

		size_t oldSize = numDataBytes();

		uintmax_t value = 0;
		for (size_t w = 0; w < numWords; w++)
			value = combineWord<rtcm_word>(_data[w], value);

		assignFrom(value, countBits<RtcmAttrs...>(), attrs...);
		_data.erase(_data.begin(), _data.begin() + numWords);
		return oldSize - numDataBytes();
	}

	/**
	 * Assign data to the given attributes from the beginning of the input byte stream. Note that
	 * the attributes must be declared as one of the RTCM types, which declares the number of
	 * bits used.
	 *
	 * @param bytes The input bytes to load from
	 * @param attrs... The set of RTCM attributes to assign
	 * @return The total number of bytes consumed from the byte stream
	 */
	template <typename... RtcmAttrs>
	size_t assignData(const tmx::byte_stream &bytes, RtcmAttrs... attrs) {
		loadData(bytes, countWords<RtcmAttrs...>());
		return assignData(attrs...);
	}

private:
	tmx::message_container_type &msg;
	tmx::byte_t mask;
	std::vector<data_type> _data;

	template <typename... Empty>
	struct size_checker {
		static constexpr const size_t size = 0;
	};

	template <typename T, typename... RtcmAttrs>
	struct size_checker<T, RtcmAttrs...> {
		typedef typename T::traits_type::attr_type rtcm_type;

		static constexpr const size_t size = rtcm_type::size + size_checker<RtcmAttrs...>::size;
	};

	template <size_t ByteSize = byteSize()>
	static constexpr size_t checkByteSize() {
		static_assert(ByteSize <= 8, "A byte can be no more than 8 bits");
		return ByteSize;
	}

	template <size_t TotalBits, size_t WordSize = wordSize()>
	static constexpr size_t checkCount() {
		static_assert(TotalBits % (byteSize() * WordSize) == 0, "Input attributes do not create a complete RTCM word");
		return TotalBits / (checkByteSize() * WordSize);
	}

	bool checkAvailableWords(size_t numWords) {
		if (numWords > numDataWords()) {
			std::stringstream err;
			err << "Unable to assign "
				<< numWords << " words ("
				<< numWords * wordSize() << "bytes) when only "
				<< numDataWords() << " data words ("
				<< numDataWords() * wordSize() << "bytes) have been loaded.";

			tmx::TmxException ex(err.str().c_str());
			BOOST_THROW_EXCEPTION(ex);
			return false;
		}

		return true;
	}

	template <typename T>
	size_t extractFrom(uintmax_t &value, size_t bitCount, T t) {
		// Assume a standard attribute is being used for an RTCM type
		typedef typename T::traits_type::attr_type rtcm_type;

		// Adjust the parameter
		bitCount -= rtcm_type::size;
		value |= ((uintmax_t)addSign<rtcm_type>(t.get(msg)) << bitCount);
		return bitCount;
	}

	template <typename T, typename... RtcmAttrs>
	uintmax_t extractFrom(uintmax_t &value, size_t bitCount, T t, RtcmAttrs... attrs) {
		bitCount = extractFrom<T>(value, bitCount, t);
		return extractFrom<RtcmAttrs...>(value, bitCount, attrs...);
	}

	template <typename T>
	size_t assignFrom(uintmax_t value, size_t bitCount, T t) {
		// Assume a standard attribute is being used for an RTCM type
		typedef typename T::traits_type::attr_type rtcm_type;

		// Adjust the parameters
		bitCount -= rtcm_type::size;
		value >>= bitCount;

		t.set(msg, addSign<rtcm_type>((typename rtcm_type::data_type)(value & rtcm_type::bitmask())));

		// Maintain the bit count moving forward
		return bitCount;
	}

	template <typename T, typename... RtcmAttrs>
	size_t assignFrom(uintmax_t value, size_t bitCount, T t, RtcmAttrs... attrs) {
		bitCount = assignFrom<T>(value, bitCount, t);
		return assignFrom<RtcmAttrs...>(value, bitCount, attrs...);
	}

};


} /* End namespace rtcm */
} /* End namespace messages */
} /* End namespace tmx */


#endif /* INCLUDE_RTCM_RTCMDATAMANAGER_H_ */

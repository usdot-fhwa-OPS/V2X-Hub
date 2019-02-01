/*
 * BsmConverter.h
 *
 *  Created on: Jun 10, 2016
 *      Author: ivp
 */

#ifndef SRC_BSMCONVERTER_H_
#define SRC_BSMCONVERTER_H_

#include <tmx/j2735_messages/BasicSafetyMessage.hpp>
#include <DecodedBsmMessage.h>

namespace tmx {
namespace utils {

class BsmConverter {
public:
	/*
	 * Convert a J2735 BasicSafetyMessage_t structure into a DecodedBsmMessage.
	 *
	 * @param bsm The J2735 BSM structure containing the source data.
	 * @param decoded The destination DecodedBsmMessage.
	 */
	static void ToDecodedBsmMessage(BasicSafetyMessage_t &bsm, tmx::messages::DecodedBsmMessage &decoded);

	/*
	 * Convert a J2735 BasicSafetyMessage_t structure's blob buffer structure into a DecodedBsmMessage.
	 *
	 * @param buf The J2735 BSM structure's blob buffer.
	 * @param decoded The destination DecodedBsmMessage that is populated with data.
	 */
	static void ToDecodedBsmMessage(uint8_t *buf, tmx::messages::DecodedBsmMessage &decoded);

	/**
	 * Convert a DecodedBsmMessage into a J2735 BasicSafetyMessage_t structure.
	 * The validity flags must be properly set before calling this method.
	 *
	 * @param decoded The DecodedBsmMessage containing the source data.
	 * @param bsm The destination J2735 BSM structure that is populated with data.
	 */
	static void ToBasicSafetyMessage(tmx::messages::DecodedBsmMessage &decoded, BasicSafetyMessage_t &bsm);
private:
	//static int iMsgCount;
};

} /* namespace utils */
} /* namespace tmx */

#endif /* SRC_BSMCONVERTER_H_ */

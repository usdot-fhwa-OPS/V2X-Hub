/*
 * TmxError.hpp
 *
 *  Created on: Apr 26, 2016
 *      Author: ivp
 */

#ifndef TMX_TMXSIGNALCONTROLLERSTATUS_HPP_
#define TMX_TMXSIGNALCONTROLLERSTATUS_HPP_

#include <tmx/messages/routeable_message.hpp>
#include "IvpSignalControllerStatus.h"

namespace tmx {
namespace messages {

class TmxSignalControllerStatusMessage: public tmx::routeable_message
{
public:
	TmxSignalControllerStatusMessage(): routeable_message()
	{
		this->set_type(IVPMSG_TYPE_SIGCONT);
		this->set_subtype("ACT");
		this->set_encoding(IVP_ENCODING_STRING);
	}

	TmxSignalControllerStatusMessage(const int action): TmxSignalControllerStatusMessage()
	{
		this->set_action(action);
	}

	int get_action()
	{
		return this->get(ATTR_PAYLOAD, -1);
	}

	void set_action(int action)
	{
		this->set_payload(action);
	}
};

} /* End namespace messages */
} /* End namespace tmx */


#endif /* TMX_TMXSIGNALCONTROLLERSTATUS_HPP_ */

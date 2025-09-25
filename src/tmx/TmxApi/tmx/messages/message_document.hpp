/*
 * message_document.hpp
 *
 *  Created on: May 2, 2016
 *      Author: ivp
 */

#ifndef TMX_MESSAGES_MESSAGE_DOCUMENT_HPP_
#define TMX_MESSAGES_MESSAGE_DOCUMENT_HPP_

#include <stdexcept>

#include <tmx/messages/message.hpp>
#include <pugixml.hpp>

namespace tmx {

class message_document: public pugi::xml_document
{
public:
	message_document(tmx::xml_message &message): msg(message)
	{
		pugi::xml_parse_result result = this->load_string(message.to_string().c_str());
		if (!result)
		{
			std::string err(result.description());
			BOOST_THROW_EXCEPTION(std::runtime_error("Unable to parse " + message.to_string() + ": " + err));
		}
	}

	~message_document()
	{
		this->flush();
	}

	void flush()
	{
		std::stringstream ss;
		this->save(ss, "", 0);
		ss >> this->msg;
	}
private:
	tmx::xml_message &msg;
};


} /* End namespace */

#endif /* TMX_MESSAGES_MESSAGE_DOCUMENT_HPP_ */

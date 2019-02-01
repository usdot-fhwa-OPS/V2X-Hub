/*
 * TmxError.hpp
 *
 *  Created on: Apr 26, 2016
 *      Author: ivp
 */

#ifndef TMX_TMXNMEA_HPP_
#define TMX_TMXNMEA_HPP_

#include <boost/regex.hpp>
#include <cstring>
#include <tmx/messages/routeable_message.hpp>
#include "IvpNmea.h"

namespace tmx {
namespace messages {

class TmxNmeaMessage: public tmx::routeable_message
{
public:
	TmxNmeaMessage(): routeable_message()
	{
		this->set_type(IVPMSG_TYPE_NMEA);
		this->set_encoding(IVP_ENCODING_STRING);
	}

	TmxNmeaMessage(const std::string sentence): TmxNmeaMessage()
	{
		this->set_sentence(sentence);
	}

	std::string get_sentence()
	{
		return get_payload_str();
	}

	void set_sentence(const std::string sentence)
	{
		this->set_payload(sentence);

		if(boost::regex_match(sentence, ggaExpr))
			this->set_subtype("GGA");
		if(boost::regex_match(sentence, gsaExpr))
			this->set_subtype("GSA");
		if(boost::regex_match(sentence, rmcExpr))
			this->set_subtype("RMC");
		if(boost::regex_match(sentence, gsvExpr))
			this->set_subtype("GSV");
		if(boost::regex_match(sentence, vtgExpr))
			this->set_subtype("VTG");
		if(boost::regex_match(sentence, gllExpr))
			this->set_subtype("GLL");

	}
private:
	boost::regex ggaExpr {"\\$G[LNP]GGA,.*"};
	boost::regex gsaExpr {"\\$G[LNP]GSA,.*"};
	boost::regex rmcExpr {"\\$G[LNP]RMC,.*"};
	boost::regex gsvExpr {"\\$G[LNP]GSV,.*"};
	boost::regex vtgExpr {"\\$G[LNP]VTG,.*"};
	boost::regex gllExpr {"\\$G[LNP]GLL,.*"};
};

} /* End namespace messages */
} /* End namespace tmx */


#endif /* TMX_TMXNMEA_HPP_ */

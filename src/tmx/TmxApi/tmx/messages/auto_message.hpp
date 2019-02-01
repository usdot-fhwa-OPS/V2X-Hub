/*
 * auto_message.hpp
 *
 *  Created on: Apr 18, 2016
 *      Author: ivp
 */

#ifndef TMX_MESSAGES_AUTO_MESSAGE_HPP_
#define TMX_MESSAGES_AUTO_MESSAGE_HPP_

#define DEFAULT_ARRAY_NAME "contents"

#include <tmx/messages/message.hpp>

namespace tmx {

template <typename Format = TMX_DEFAULT_MESSAGE_FORMAT>
class tmx_auto_message: public tmx_message<Format> {
public:
	tmx_auto_message(): tmx_message<Format>() {}

	tmx_auto_message(const message_container_type &contents): tmx_message<Format>(contents) {}

	template <typename OtherFormat>
	tmx_auto_message(const struct tmx_message<OtherFormat> &other, message_converter *converter = 0):
		tmx_message<Format>(other, converter) {}

	tmx_auto_message(const std::string &contents): tmx_message<Format>(contents) {}

	virtual ~tmx_auto_message() {}

	message_container_type &get_container()
	{
		return this->msg;
	}

	template <typename BaseType, typename DataType>
	inline battelle::attributes::typed_attribute<DataType>
			auto_attribute(DataType &field, typename message_container_type::path_type path = "") {
		static std::string name_base = battelle::attributes::type_id_name<BaseType>();

		std::string name(name_base + "::");
		bool mkArray = false;
		if (path.empty())
		{
			name += "_" + battelle::attributes::attribute_lexical_cast<std::string>(next_param());
			path = DEFAULT_ARRAY_NAME;
			path /= "TMP_VALUE";
			mkArray = true;
		}
		else
		{
			name += path.dump();
		}

		battelle::attributes::typed_attribute<DataType> attr =
				battelle::attributes::typed_attribute<DataType>(name, path, field, this->msg);
		if (mkArray)
		{
			fix_xml_arrays fixer;
			static message_converter converter;
			converter.repairAlongTree(fixer, this->as_tree().get(), DEFAULT_ARRAY_NAME);
		}
		return attr;
	}
private:
	static int next_param() {
		static int param_count = 0;
		return ++param_count;
	}
};

typedef tmx_auto_message<JSON> json_auto_message;
typedef tmx_auto_message<XML> xml_auto_message;
typedef tmx_auto_message<TMX_DEFAULT_MESSAGE_FORMAT> auto_message;

} /* namespace tmx */

#endif /* TMX_MESSAGES_AUTO_MESSAGE_HPP_ */

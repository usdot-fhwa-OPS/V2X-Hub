/*
 * @file serialization_types.hpp
 *
 *  Created on: Jan 27, 2016
 *      Author: BAUMGARDNER
 */

#ifndef SRC_ATTRIBUTES_SERIALIZATION_TYPES_HPP_
#define SRC_ATTRIBUTES_SERIALIZATION_TYPES_HPP_

#include "type_basics.hpp"
#include "container.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#pragma GCC diagnostic pop

namespace battelle { namespace attributes {

/**
 * JSON serialization
 */
typedef struct {
	static std::string name() { return "JSON"; }

	template <class InputType> inline
	void read(CSTORE_TYPE &container, InputType &in) {
		read_json(in, container.get_tree());
	}

	template <class OutputType> inline
	void write(CSTORE_TYPE &container, OutputType &out) {
		write_json(out, container.get_tree(), false);
	}
} JSON;

/**
 * XML serialization
 */
typedef struct {
	static std::string name() { return "XML"; }

	template <class InputType> inline
	void read(CSTORE_TYPE &container, InputType &in) {
		// Make sure to get rid of excess white spaces, which cause problems in translations
		read_xml(in, container.get_tree(), boost::property_tree::xml_parser::trim_whitespace);
	}

	template <class OutputType> inline
	void write(CSTORE_TYPE &container, OutputType &out) {
		write_xml(out, container.get_tree());
	}
} XML;

/**
 * Windows .INI serialization, not typically used but supported
 */
typedef struct {
	static std::string name() { return "INI"; }

	template <class InputType> inline
	void read(CSTORE_TYPE &container, InputType &in) {
		read_ini(in, container.get_tree());
	}

	template <class OutputType> inline
	void write(CSTORE_TYPE &container, OutputType &out) {
		write_ini(out, container.get_tree());
	}
} INI;

/**
 * Boost::property_tree INFO type serialization, not typically used but supported.
 */
typedef struct {
	static std::string name() { return "INFO"; }

	template <class InputType> inline
	void read(CSTORE_TYPE &container, InputType &in) {
		read_info(in, container.get_tree());
	}

	template <class OutputType> inline
	void write(CSTORE_TYPE &container, OutputType &out) {
		write_info(out, container.get_tree());
	}
} INFO;

}} /* End namespace */

#endif /* SRC_ATTRIBUTES_SERIALIZATION_TYPES_HPP_ */

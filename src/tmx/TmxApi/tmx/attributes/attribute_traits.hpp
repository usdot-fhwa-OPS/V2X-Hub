/*
 * @file attribute_type_traits.hpp
 *
 *  Created on: Jan 28, 2016
 *      Author: BAUMGARDNER
 */

#ifndef SRC_ATTRIBUTES_ATTRIBUTE_TRAITS_HPP_
#define SRC_ATTRIBUTES_ATTRIBUTE_TRAITS_HPP_

#include "attribute_cast.hpp"
#include "attribute_macros.hpp"
#include "type_basics.hpp"
#include "container.hpp"
#include <type_traits>

namespace battelle { namespace attributes {

/**
 * All attribute names are strings
 */
typedef std::string name_type;

/**
 * A structure that defines the traits of the attribute, including
 * the path in the container, the actual C++ type for the attribute,
 * and the default value if none is set.
 */
template <typename BackendType = CSTORE_TYPE>
struct basic_attribute_traits {
	typedef BackendType storage_type;
	typedef typename storage_type::path_type path_type;
	typedef typename storage_type::value_type value_type;
	typedef typename storage_type::value_type data_type;

	basic_attribute_traits() {};

	basic_attribute_traits(name_type &attrName, path_type &attrPath, value_type &attrDefault):
		name(attrName), path(attrPath), default_value(attrDefault) {
	}

	basic_attribute_traits(const name_type attrName, const path_type attrPath, const value_type attrDefault):
		name(attrName), path(attrPath), default_value(attrDefault) {
	}

	template <typename TraitsType>
	basic_attribute_traits(const TraitsType &other): name(other.name), path(other.path) {
		default_value = attribute_lexical_cast<value_type>(other.default_value);
		//std::cout << "Constructed new traits from existing " << other.name << " (" << other.path.dump() << ")" << std::endl;
	}

	name_type name;
	path_type path;
	value_type default_value;
};

typedef struct basic_attribute_traits<CSTORE_TYPE> attribute_traits;

/**
 * The attribute traits that are determined from a given structure.  The type is
 * determined by the structure as well as the path.  The path key is the shortened
 * name of the type.
 */
template <typename Type,
		  typename BackendType = CSTORE_TYPE>
struct detected_traits {
	typedef Type attr_type;
	typedef BackendType storage_type;
	typedef typename storage_type::path_type path_type;
	typedef typename attr_type::data_type data_type;

	detected_traits() {
		name = type_id_name<attr_type>();
		path = type_name<attr_type>();
		default_value = attr_type::default_value();
	}

	detected_traits(name_type &attrName, path_type &attrPath, data_type &attrDefault):
		name(attrName), path(attrPath), default_value(attrDefault) {
	}

	name_type name;
	path_type path;
	data_type default_value;
};

#ifndef ATTRIBUTE_VALUE_FIELD
#define ATTRIBUTE_VALUE_FIELD value
#endif

template <typename PathType>
struct default_path_creator {
	typedef PathType path_type;

	path_type operator()(path_type attr_path)
	{
		path_type p(attr_path);
		p /= quoted_attribute_name(ATTRIBUTE_VALUE_FIELD);
		return p;
	}
};

/**
 * The attribute traits that are determined from a given structure.  The type is
 * determined by the structure as well as the path.  The path key is the shortened
 * name of the type, but the value is stored as a sub-key in the tree, whose default
 * is <i>value</i>
 */
template <typename Type,
		  typename BackendType = CSTORE_TYPE,
		  typename PathCreator = default_path_creator<typename BackendType::path_type> >
struct detected_2level_traits {
	typedef Type attr_type;
	typedef BackendType storage_type;
	typedef typename storage_type::path_type path_type;
	typedef typename attr_type::data_type data_type;

	detected_2level_traits() {
		name = type_id_name<attr_type>();
		PathCreator pc;
		path = pc(path_type(type_name<attr_type>()));
		default_value = attr_type::default_value();
	}

	detected_2level_traits(name_type &attrName, path_type &attrPath, data_type &attrDefault):
		name(attrName), path(attrPath), default_value(attrDefault) {
	}

	name_type name;
	path_type path;
	data_type default_value;
};

}} /* End namespace */

#endif /* SRC_ATTRIBUTES_ATTRIBUTE_TRAITS_HPP_ */

/*
 * @file attributes.hpp
 *
 *  Created on: Jan 15, 2016
 *      Author: BAUMGARDNER
 */

#ifndef SRC_ATTRIBUTES_HPP_
#define SRC_ATTRIBUTES_HPP_

#include <vector>

#include "attribute_macros.hpp"
#include "attribute_cast.hpp"
#include "container.hpp"
#include "type_basics.hpp"
#include "serialization_types.hpp"
#include "attribute_traits.hpp"
#include "attribute_type.hpp"
#include "registry.hpp"

/*
 * Macro for load and save definitions, to be done once.  Protection is determined
 * by where this macro is invoked
 */
#define load_attribute_builder(C, X, Y) \
	template <typename SerializationType> \
	void attr_func_name(load_attributes_, Y)(X &in) { \
		return battelle::attributes::load_attributes<SerializationType>(C, in); \
	}
#define save_attribute_builder(C, X, Y) \
	template <typename SerializationType> \
	void attr_func_name(save_attributes_, Y)(X &out) { \
		return battelle::attributes::save_attributes<SerializationType>(C, out); \
	}
#define declare_load_attributes(C) \
		load_attribute_builder(C, std::istream, stream) \
		load_attribute_builder(C, std::string, file)
#define declare_save_attributes(C) \
		save_attribute_builder(C, std::ostream, stream) \
		save_attribute_builder(C, std::string, file)


/*
 * These macros help define attributes getters and setters based on standards.
 *
 * C = Container
 * T = Attribute type
 * X = Underlying C-type for type-safe access
 * Y = Attribute name
 * G = getter prefix
 * S = setter prefix
 * D = Default value
 * L = Expression for input validation
 */
#define typesafe_getter_builder(C, X, Y, G) \
	X attr_func_name(G, Y)() { \
		return attr_field_name(Y).get(C); \
	}
#define typesafe_setter_builder(C, X, Y, S, L) \
	void attr_func_name(S, Y)(const X value) { \
		L { \
			attr_field_name(Y).set(C, value); \
		} \
	}
#define attribute_builder(T, X, Y, D) \
public: \
	struct Y { \
		typedef X data_type; \
		static data_type default_value() { return D; } \
	}; \
private: \
	T attr_field_name(Y);
#define ro_attribute(C, T, X, Y, G, D) \
	attribute_builder(T, X, Y, D) \
public: \
	typesafe_getter_builder(C, X, Y, G)
#define rw_attribute(C, T, X, Y, G, D, S, L) \
	ro_attribute(C, T, X, Y, G, D) \
	typesafe_setter_builder(C, X, Y, S, L)
#define std_attribute(C, X, Y, D, L) \
	rw_attribute(C, battelle::attributes::standard_attribute<Y>, X, Y, get_, D, set_, L)
#define std_2level_attribute(C, X, Y, D, L) \
	rw_attribute(C, battelle::attributes::standard_2level_attribute<Y>, X, Y, get_, D, set_, L)

namespace battelle { namespace attributes {

/*
 *
 * Some helper functions
 */

#define __reg_type attribute_registry<typename ContainerType::storage_type>

/**
 * Retrieve an unknown and untyped attribute from the container by name.
 */
template <typename ContainerType>
static basic_attribute<typename ContainerType::storage_type>
		get_unknown_attribute(ContainerType &container,	typename __reg_type::key_type name) {
	typedef __reg_type reg_type;
	typedef typename ContainerType::storage_type storage_type;
	typedef basic_attribute<storage_type> attr_type;

	reg_type &reg = reg_type::get_instance();
	attr_type attr = reg.template get_attribute<attr_type>(name);
	return attr;
}

/**
 * Retrieve the value from the container by path name, or the default_value if the container
 * does not include the specified path.
 */
template <typename ContainerType>
static typename ContainerType::value_type get_value_at(ContainerType &container,
		typename ContainerType::path_type path, typename ContainerType::value_type default_value) {
	return container.retrieve(path, default_value, false);
}

/**
 * Retrieve the names of all the known attributes in the container, optionally filtered by a name prefix.
 */
template <typename ContainerType>
static std::vector<typename __reg_type::key_type> get_names(ContainerType &container,
		typename __reg_type::key_type name_prefix = "") {
	typedef __reg_type reg_type;
	typedef typename reg_type::key_type key_type;
	typedef typename reg_type::it_type it_type;

	reg_type &reg = reg_type::get_instance();

	std::vector<key_type> v;
	for (it_type i = reg.begin(); i != reg.end(); i++) {
		key_type key = i->first;
		if (name_prefix.length() <= 0 || key.substr(0, name_prefix.length()) == name_prefix)
			v.push_back(i->first);
	}

	return v;
}

/**
 * Retrieve all the known attribute types in the container, optionally filtered by a name prefix
 */
template <typename ContainerType>
static std::vector<typename __reg_type::value_type> get_types(ContainerType &container,
		typename __reg_type::key_type name_prefix = "") {
	typedef __reg_type reg_type;
	typedef typename reg_type::key_type key_type;
	typedef typename reg_type::value_type traits_type;
	typedef typename reg_type::it_type it_type;

	reg_type &reg = reg_type::get_instance();

	std::vector<traits_type> v;
	for (it_type i = reg.begin(); i != reg.end(); i++) {
		key_type key = i->first;
		if (name_prefix.length() <= 0 || key.substr(0, name_prefix.length()) == name_prefix)
			v.push_back(i->second);
	}

	return v;
}

/**
 * Retrieve all the known attribute types in the container for a given type
 */
template <typename Self, typename ContainerType>
static std::vector<typename __reg_type::value_type> get_my_types(ContainerType &container) {
	return get_types<ContainerType>(container, type_id_name<Self>() + "::");
}

/**
 * Do a quick get on every known attribute in the container for the given type in order
 * to set the defaults into the container
 */
template <typename Self, typename ContainerType>
static inline void initialize_my_attributes(ContainerType &container) {
	typedef __reg_type reg_type;
	typedef typename reg_type::value_type traits_type;
	typedef typename reg_type::storage_type storage_type;

	std::vector<traits_type> my_traits = get_my_types<Self>(container);
	for (std::size_t i = 0; i < my_traits.size(); i++) {
		basic_attribute<storage_type> attr(my_traits[i]);
		attr.get_untyped(container);
	}
}

/**
 * Basic serialization/deserialization for a given container
 */

template <typename SerializationType, typename InputType, typename ContainerType>
static inline void load_attributes(ContainerType &container, InputType &in) {
	container.template load<SerializationType>(in);
}

template <typename SerializationType, typename OutputType, typename ContainerType>
static inline void save_attributes(ContainerType &container, OutputType &out) {
	container.template save<SerializationType>(out);
}

#undef __reg_type

}} /* end namespace */
#endif /* SRC_ATTRIBUTES_HPP_ */

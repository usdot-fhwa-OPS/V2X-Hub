/*
 * @file type_functions.hpp
 *
 *  Created on: Jan 27, 2016
 *      Author: BAUMGARDNER
 */

#ifndef SRC_ATTRIBUTES_TYPE_BASICS_HPP_
#define SRC_ATTRIBUTES_TYPE_BASICS_HPP_

#include <cxxabi.h>
#include <cstdlib>
#include <memory>
#include <typeinfo>
#include <sstream>

#include "container.hpp"

namespace battelle { namespace attributes {

/**
 * Function to demangle a C++ type name.
 *
 * @param mangled The mangled name as a C-style string, from typeid
 * @return The demangled name as a C++ string.
 */
static inline std::string demangle_type(const char *mangled) {
    int status = 0;
    char *realname;
    realname = abi::__cxa_demangle(mangled, 0, 0, &status);

    std::stringstream ss;

    if (!status) {
    	ss << realname;
    } else {
    	ss << "Unknown demangling error: " << status;
    }

    free(realname);

    return ss.str();
}

/**
 * Function to retrieve the full demangled name for the given type using typeid.
 *
 * @return The full name of the type.
 */
template <typename Type>
static inline std::string type_id_name() {
	typedef Type myType;
	const std::type_info &info = typeid(myType);
	return demangle_type(info.name());
}

static inline std::string short_type_name(const std::string &long_type_name) {
	size_t index = long_type_name.find_last_of("::");
	if (index <= 0 || index + 1 >= long_type_name.length())
		return long_type_name;
	else
		return long_type_name.substr(index + 1);
}

/**
 * Function to retrieve the shortened name of the given type.
 *
 * @return The short name for the type.
 */
template <typename Type>
static inline std::string type_name() {
	return short_type_name(type_id_name<Type>());
}

/**
 * Function to retrieve the full demangled name of the variable reference, from
 * typeid.
 * Warning: This function will NOT work with polymorphic references.
 *
 * @param type The variable reference to check
 * @return The full name of the type
 */
template<typename Type>
static inline std::string type_id_name(Type &type) {
	return type_id_name<Type>();
}

/**
 * Function to retrieve the shortened type name of the variable reference
 * Warning: This function will NOT work with polymorphic references
 *
 * @param type The variable reference to check
 * @return The full name of the type
 */
template<typename Type>
static inline std::string type_name(Type &type) {
	return type_name<Type>();
}


}} /* End namespace */

#endif /* SRC_ATTRIBUTES_TYPE_BASICS_HPP_ */

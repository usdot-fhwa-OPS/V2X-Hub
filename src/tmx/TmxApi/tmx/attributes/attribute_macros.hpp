/*
 * @file attribute_macros.hpp
 *
 *  Created on: Jan 31, 2016
 *      Author: BAUMGARDNER
 */

#ifndef SRC_ATTRIBUTES_ATTRIBUTE_MACROS_HPP_
#define SRC_ATTRIBUTES_ATTRIBUTE_MACROS_HPP_

#include <boost/regex.hpp>

/*
 * Some helpful macros
 */
#define attr_str(s) #s
#define attr_func_name(X, Y) X ## Y
#define attr_field_name(Y) _ ## Y

#define quoted_attribute_name(Y) attr_str(Y)

#define REGEX_CHECK(X) if(boost::regex_match(value, boost::regex(X)))
#define POSITIVE_CHECK(X) if(X > 0)
#define POSITIVE_VAL_CHECK() POSITIVE_CHECK(value)

#endif /* SRC_ATTRIBUTES_ATTRIBUTE_MACROS_HPP_ */

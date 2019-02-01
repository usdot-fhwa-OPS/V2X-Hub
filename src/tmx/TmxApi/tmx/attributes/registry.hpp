/*
 * @file attribute_registry.hpp
 *
 *  Created on: Jan 28, 2016
 *      Author: BAUMGARDNER
 */

#ifndef SRC_ATTRIBUTES_REGISTRY_HPP_
#define SRC_ATTRIBUTES_REGISTRY_HPP_

#include <map>

#include "attribute_type.hpp"
#include "attribute_traits.hpp"

namespace battelle { namespace attributes {

/**
 * A registry of defined attributes for a given storage type in order to
 * maintain which container owns the attribute.
 */
template <typename BackendType = CSTORE_TYPE>
class attribute_registry {
public:
	typedef BackendType storage_type;
	typedef name_type key_type;
	typedef struct basic_attribute_traits<BackendType> value_type;
	typedef typename std::map<key_type, value_type> reg_type;
	typedef typename reg_type::iterator it_type;

	static inline attribute_registry &get_instance() {
		static attribute_registry instance;
		return instance;
	}

	template <typename Traits>
	void register_type(Traits &traits) {
		value_type basic_traits(traits);
		registry.insert(std::pair<key_type, value_type>(traits.name, value_type(traits)));
	}

	template <typename AttributeType>
	AttributeType get_attribute(key_type name) {
		typedef typename AttributeType::traits_type traits_type;

		const value_type reg_type = get_type(name);
		traits_type traits(reg_type);
		return AttributeType(traits);
	}

	value_type & get_type(key_type name) {
		return registry[name];
	}

	it_type begin() {
		return registry.begin();
	}

	it_type end() {
		return registry.end();
	}

private:
	attribute_registry() {}
	attribute_registry(attribute_registry const&);
	void operator=(attribute_registry const&);

	reg_type registry;
};

}} /* End namespace */

#endif /* SRC_ATTRIBUTES_REGISTRY_HPP_ */

/*
 * @file attribute.hpp
 *
 *  Created on: Jan 27, 2016
 *      Author: BAUMGARDNER
 */

#ifndef SRC_ATTRIBUTES_ATTRIBUTE_TYPE_HPP_
#define SRC_ATTRIBUTES_ATTRIBUTE_TYPE_HPP_

#include <typeinfo>

#include "attribute_traits.hpp"
#include "container.hpp"
#include "registry.hpp"
#include "type_basics.hpp"
#include <boost/lexical_cast.hpp>
#include <type_traits>


namespace battelle { namespace attributes {

/**
 * A structure to hold a basic attribute for the storage.  The attribute value
 * is taken from the container in the default value type, which is a string.
 * In other words, the basic attribute is untyped.
 */
template <typename BackendType = CSTORE_TYPE,
		  typename TraitsType = basic_attribute_traits<CSTORE_TYPE> >
class basic_attribute {
public:
	typedef basic_attribute<BackendType, TraitsType> attr_type;
	typedef basic_attribute_container<BackendType> container_type;
	typedef TraitsType traits_type;
	typedef typename traits_type::storage_type storage_type;
	typedef typename storage_type::path_type path_type;
	typedef typename storage_type::value_type value_type;

	basic_attribute(name_type &name, path_type &path, value_type &default_value):
		t(name, path, default_value) {
	}

	basic_attribute(const TraitsType &traits): t(traits) {
		attribute_registry<storage_type> &reg = attribute_registry<storage_type>::get_instance();
		reg.register_type(t);
	}

	template <typename AttributeType>
	attr_type &operator=(AttributeType &other) {
		attr_type attr(other.traits);
		return attr;
	}

	std::string name() { return this->traits().name; }
	path_type path() { return this->traits().path; }
	value_type default_value() {
		return attribute_lexical_cast<value_type>(this->traits().default_value);
	}

	value_type get_untyped(container_type &container) {
		return container.retrieve(this->path(), this->default_value());
	}

	void set_untyped(container_type &container, const value_type &data) {
		container.store(this->path(), data);
	}

	traits_type &traits() {
		return t;
	}
private:
	traits_type t;
};

/**
 * A structure to hold a basic attribute for the given storage.  The value
 * is not only stored in the container as an untyped value, but also is cached
 * in a boost::any type, which is only updated if the container is updated.
 * There is an associated type-safe get and set to retrieve the cached value.
 */
template <typename DataType,
     	  typename BackendType = CSTORE_TYPE,
		  typename TraitsType = basic_attribute_traits<CSTORE_TYPE> >
class typed_attribute: public basic_attribute<BackendType, TraitsType> {
public:
	typedef DataType data_type;
	typedef typed_attribute<DataType, BackendType, TraitsType> attr_type;
	typedef basic_attribute_container<BackendType> container_type;
	typedef typename basic_attribute<BackendType, TraitsType>::attr_type basic_attr_type;
	typedef TraitsType traits_type;
	typedef typename traits_type::storage_type storage_type;
	typedef typename storage_type::path_type path_type;
	typedef typename storage_type::value_type value_type;

	typed_attribute(name_type &name, path_type &path, value_type &default_value):
		basic_attribute<BackendType, TraitsType>(name, path, default_value),
		_attribute(), _reference(0), storage_version(0) {
	}

	typed_attribute(const TraitsType &traits): basic_attribute<BackendType, TraitsType>(traits),
		_attribute(), _reference(0), storage_version() {
	}

	typed_attribute(name_type name, path_type path, DataType &var, container_type &container):
			basic_attribute<BackendType, TraitsType>(
					traits_type(name, path, attribute_lexical_cast<value_type>(var))),
			_attribute(var), _reference(&var), storage_version() {
		get(container);
	}

	template <typename AttributeType>
	typed_attribute<typename AttributeType::data_type,
					typename AttributeType::storage_type,
					typename AttributeType::traits_type>
			&operator=(AttributeType &other) {
		typed_attribute<typename AttributeType::data_type,
						typename AttributeType::storage_type,
						typename AttributeType::traits_type>
			attr(other.traits);
		return attr;
	}

	std::string data_type_name() {
		static std::string name = type_id_name<data_type>();
		return name;
	}

	data_type get(container_type &container) {
		data_type curVal = get_as_type<data_type>(container);
		if (_reference)
			(*_reference) = curVal;
		return curVal;
	}

	template <typename OtherDataType>
	OtherDataType get_as_type(container_type &container) {
		// Check storage version
		int container_version = container.get_storage_version();
		if (storage_version < container_version)
			invalidate();

		if (_attribute.empty()) {
			_attribute = attribute_lexical_cast<OtherDataType>(
							basic_attr_type::get_untyped(container));

			storage_version = container_version;
		}

		return boost::any_cast<OtherDataType>(_attribute);
	}

	void set(container_type &container, const data_type &data) {
		if (_reference)
			*_reference = data;

		set_as_type(container, data);
	}

	template <typename OtherDataType>
	void set_as_type(container_type &container, const OtherDataType &data) {
		_attribute = data;
		basic_attr_type::set_untyped(container, attribute_lexical_cast<value_type>(data));
		storage_version = container.get_storage_version();
	}

	void invalidate() {
		_attribute.clear();
	}
private:
	boost::any _attribute;
	data_type *_reference;
	int storage_version;
};

/**
 * The standard attribute for the given storage using a detected type trait
 * structure.
 */
template <typename Type,
		  typename BackendType = CSTORE_TYPE,
		  typename TraitsType = detected_traits<Type, BackendType> >
class standard_attribute:
		public typed_attribute<typename Type::data_type, BackendType, TraitsType> {
public:
	typedef standard_attribute<Type, BackendType, TraitsType> attr_type;
	typedef TraitsType traits_type;

	standard_attribute():
		typed_attribute<typename Type::data_type, BackendType, TraitsType>(traits_type()) {
	}
};

/**
 * The standard attribute for the given storage using a two-level detected type
 * trait structure.
 */
template <typename Type,
		  typename BackendType = CSTORE_TYPE,
		  typename PathCreator = default_path_creator<typename BackendType::path_type>,
		  typename TraitsType = detected_2level_traits<Type, BackendType, PathCreator> >
class standard_2level_attribute:
		public typed_attribute<typename Type::data_type, BackendType, TraitsType> {
public:
	typedef standard_2level_attribute<Type, BackendType, PathCreator, TraitsType> attr_type;
	typedef TraitsType traits_type;

	standard_2level_attribute():
		typed_attribute<typename Type::data_type, BackendType, TraitsType>(traits_type()) {
	}
};

}} /* End namespace */

#endif /* SRC_ATTRIBUTES_ATTRIBUTE_TYPE_HPP_ */

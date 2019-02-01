/*
 * @file container.hpp
 *
 *  Created on: Jan 27, 2016
 *      Author: BAUMGARDNER
 */

#ifndef SRC_ATTRIBUTES_CONTAINER_HPP_
#define SRC_ATTRIBUTES_CONTAINER_HPP_

#include <vector>
#include <iostream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#include <boost/property_tree/ptree.hpp>
#pragma GCC diagnostic pop

namespace battelle { namespace attributes {

/**
 * A basic attribute container for a given backend storage.
 */
template <typename BackendType>
class basic_attribute_container {
public:
	typedef BackendType storage_type;
	typedef typename storage_type::path_type path_type;
	typedef typename storage_type::value_type value_type;

	basic_attribute_container(): storage_version(1) {};

	void store(const path_type &path, const value_type &data) {
		_storage.store(path, data);
		storage_version++;
	}

	value_type retrieve(const path_type &path, const value_type &default_value, bool set_on_missing = true) {
		return _storage.retrieve(path, default_value, set_on_missing);
	}

	template <typename SerializationType, typename OutputType>
	void save(OutputType &out) {
		SerializationType ser;
		ser.template write<OutputType>(_storage, out);
	}

	template <typename SerializationType, typename InputType>
	void load(InputType &in) {
		SerializationType ser;
		ser.template read<InputType>(_storage, in);
		storage_version++;
	}

	inline int get_storage_version() const {
		return storage_version;
	}

	inline storage_type &get_storage()
	{
		return _storage;
	}
private:
	storage_type _storage;
	int storage_version;
};

/**
 * The default boost::property_tree::ptree backend storage for the path and value type.
 */
template <typename PtreePathType, typename PtreeValueType>
class ptree_backend {
public:
	typedef typename boost::property_tree::basic_ptree<PtreePathType, PtreeValueType> tree_type;
	typedef typename tree_type::path_type path_type;
	typedef typename tree_type::data_type value_type;

	static boost::property_tree::path get_path(const path_type &pathLoc) {
		path_type path;
		char sep = path.separator();
	#ifdef ATTRIBUTE_PATH_CHARACTER
		sep = ATTRIBUTE_PATH_CHARACTER;
	#endif

		return boost::property_tree::path(pathLoc.dump(), sep);
	}

	boost::optional<tree_type &> subtree(const path_type &pathLoc) {
		return _tree.get_child_optional(get_path(pathLoc));
	}

	void store(const path_type &pathLoc, const value_type &value) {
		//std::cout << "Storing " << value << " into the tree at path " << pathLoc.dump() << std::endl;
		boost::property_tree::path path = get_path(pathLoc);
		_tree.template put<PtreeValueType>(path, value);
	}

	value_type retrieve(const path_type &pathLoc, const value_type &default_value, bool set_on_missing) {
		//std::cout << "Getting value from the tree at path " << pathLoc.dump() << std::endl;

		boost::optional<tree_type &> subTree = subtree(pathLoc);

		if (!subTree) {
			//std::cout << "Path does not exist.  Using default value " << default_value << std::endl;
			if (set_on_missing) store(pathLoc, default_value);
			return default_value;
		} else {
			//std::cout << "Path exists with value " << subTree.get().get_value(default_value) << std::endl;
			return subTree.get().get_value(default_value);
		}
	}

	tree_type & get_tree() {
		return _tree;
	}
private:
	tree_type _tree;
};

typedef ptree_backend<std::string, std::string> standard_ptree_backend;
typedef basic_attribute_container<standard_ptree_backend> ptree_backed_attribute_container;

// Right now, this is the only option
typedef ptree_backed_attribute_container attribute_container;

#define CSTORE_TYPE battelle::attributes::attribute_container::storage_type

}} /* End namepace */


#endif /* SRC_ATTRIBUTES_CONTAINER_HPP_ */

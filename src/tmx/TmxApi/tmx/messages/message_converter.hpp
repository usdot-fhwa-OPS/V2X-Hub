/*
 * @file message_converter.hpp
 *
 *  Created on: Apr 7, 2016
 *      Author: ivp
 */

#ifndef TMX_MESSAGE_CONVERTER_HPP_
#define TMX_MESSAGE_CONVERTER_HPP_

#define HELPER_TREE_ROOT "TreeRepair."

#include <tmx/attributes/attributes.hpp>

namespace tmx
{

/**
 * Converter function to change XML arrays to JSON arrays, removing the extra key
 */
struct fix_xml_arrays {
	template<typename Tree>
	void operator()(Tree &pt) {
		Tree copy(pt);

		pt.clear();
		for (typename Tree::iterator i = copy.begin(); i != copy.end(); i++) {
			pt.push_back(
					std::pair<typename Tree::key_type, Tree>("", i->second));
		}
	}
};

/**
 * Converter function to take an entire sub-tree up one level, effectively deleting unnecessary keys
 */
struct del_unnecessary_nodes {
	template<typename Tree>
	void operator()(Tree &pt) {
		Tree copy(pt);

		pt.clear();
		for (typename Tree::iterator i = copy.begin(); i != copy.end(); i++) {
			for (typename Tree::iterator j = i->second.begin();
					j != i->second.end(); j++) {
				pt.push_back(
						std::pair<typename Tree::key_type, Tree>(j->first,
								j->second));
			}
		}

	}
};

/**
 * Converter function to take make the key of a sub-tree a new value in the parent level,
 * effectively deleting unnecessary sub-objects
 */
struct flatten_node {
	template<typename Tree>
	void operator()(Tree &pt) {
		Tree copy(pt);
		pt.clear();
		pt.put_value(copy.begin()->first);
	}
};

/**
 * Converter function
 */

class message_converter
{
	typedef typename battelle::attributes::attribute_container::storage_type storage_type;
	typedef typename storage_type::tree_type tree_type;
	typedef typename tree_type::iterator it_type;
	typedef typename tree_type::key_type key_type;
	typedef typename tree_type::value_type value_type;
	typedef typename tree_type::path_type path_type;
public:
	message_converter()
	{
		static bool reportedError = false;
		try
		{
			battelle::attributes::XML ser;
			std::string inFile("treerepair.xml");
			init_container<battelle::attributes::XML, std::string>(ser, inFile);
		}
		catch (boost::property_tree::xml_parser_error &ex)
		{
			if (!reportedError)
			{
				std::cerr << ex.what() << std::endl;
				reportedError = true;
			}
		}
	}

	template <typename SerializationType, typename InputType>
	message_converter(SerializationType ser, InputType &in)
	{
		init_container<SerializationType, InputType>(ser, in);
	}

	virtual ~message_converter() {}

	template<typename Functor>
	void repairAlongTree(Functor &fn, tree_type &pt, path_type path)
	{
		if (path.empty())
		{
			return;
		}

		if (path.single())
		{
			boost::optional<tree_type &> child = pt.get_child_optional(path);
			if (child)
				fn(child.get());
		}
		else
		{
			key_type head = path.reduce();
			for (it_type i = pt.begin(); i != pt.end(); i++)
			{
				if (i->first == head)
				{
					repairAlongTree<Functor>(fn, i->second, path);
				}
			}
		}
	}

	std::string get_value(tree_type &msgTree, std::string field, bool *found = 0)
	{
		value_type &root = msgTree.front();
		boost::optional<std::string> id = helper().get_optional<std::string>(
				path_type(HELPER_TREE_ROOT + root.first + "." + field));

		if (found)
			*found = (id ? true : false);

		if (id)
			return id.get();
		else
			return root.first;
	}

	template <typename AttrType>
	std::string get_value(tree_type &msgTree)
	{
		return get_value(msgTree, battelle::attributes::type_name<AttrType>());
	}

	template<typename Functor>
	void cleanupTree(tree_type &msgTree)
	{
		Functor fn;
		value_type &root = msgTree.front();
		it_type i;

		boost::optional<tree_type &> subTree = helper().get_child_optional(
				path_type(HELPER_TREE_ROOT + root.first + "." + battelle::attributes::type_name(fn)));
		if (subTree)
		{
			for (i = subTree.get().begin(); i != subTree.get().end(); i++)
				repairAlongTree(fn, msgTree,
						path_type(i->second.template get_value<std::string>()));
		}
	}

	virtual void cleanupTree(tree_type &msgTree)
	{
		cleanupTree<fix_xml_arrays>(msgTree);
		cleanupTree<del_unnecessary_nodes>(msgTree);
		cleanupTree<flatten_node>(msgTree);
	}

private:
	battelle::attributes::attribute_container _helper_container;

	tree_type &helper()
	{
		return _helper_container.get_storage().get_tree();
	}

	template <typename SerializationType, typename InputType>
	void init_container(SerializationType ser, InputType &in)
	{
		_helper_container.load<SerializationType>(in);
	}
};


} /* End namespace tmx */

#endif /* TMX_MESSAGE_CONVERTER_HPP_ */

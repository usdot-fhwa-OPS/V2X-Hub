/*
 * attribute_wrapped_type.hpp
 *
 * This header file defines a special implementation of an
 * attribute type that either allocates the underlying type of
 * attribute on the fly, or uses a given reference to that
 * type as the placeholder.  The allocator is specified
 * as a template parameter.  Additionally, there is a special
 * allocator for a string type that allows management of C-style
 * char * string types.  This facilitates efficient storage of
 * character strings in old C-style structures, while still allowing
 * the attribute to be set and retrieved and stored in the container
 * as a C++ string object.
 *
 *  Created on: Jul 25, 2017
 *      @author: gmb
 */

#ifndef TMX_ATTRIBUTES_ATTRIBUTE_WRAPPED_TYPE_HPP_
#define TMX_ATTRIBUTES_ATTRIBUTE_WRAPPED_TYPE_HPP_

#include "attribute_type.hpp"

namespace battelle { namespace attributes {

/**
 * An allocator to use for strings, which uses a character pointer allocator.
 *
 * @param T The character template to use
 */
template <typename T>
class BasicStringAllocator {
public:
	using str_type = std::basic_string<T>;

	typedef typename str_type::allocator_type str_alloc;
	typedef typename str_type::value_type char_type;
	typedef typename str_type::pointer char_ptr;
	typedef char_ptr value_type;
	typedef value_type& reference;
	typedef value_type* pointer;
	typedef reference const_reference;
	typedef pointer const_pointer;
	typedef typename str_type::size_type size_type;
	typedef typename str_type::difference_type difference_type;

	pointer address(reference x) const { return &x; }

	pointer allocate(size_type n) { return _allocator.allocate(n); }
	void deallocate(pointer p, size_type n) { _allocator.deallocate(p, n); }

	size_type max_size() const { return _allocator.max_size(); }

	void construct(pointer p, const_reference val) { _allocator.construct(p, val); }
	void construct(pointer p, const str_type &x) {
		// Must deallocate the old string
		if (_str) {
			free(_str);
			_str = NULL;
		}
		_str = strdup(x.c_str());
		construct(p, _str);
	}

	void destroy(pointer p) { _allocator.destroy(p); }
private:
	std::allocator<value_type> _allocator;
	str_alloc _str_allocator;
	char_ptr _str = 0;
};

// Basic string allocator type definitions
typedef std::string::value_type std_char;
typedef std::wstring::value_type std_wchar;
typedef BasicStringAllocator<std_char> StringAllocator;
typedef BasicStringAllocator<std_wchar> WStringAllocator;

/**
 * The container compliant attribute type that maintains a cached reference to a real
 * instance of the underlying type, either built internally automatically by the allocator,
 * or passed in using the bind() operation.  The attribute traits follows those of its
 * typed base class, which is specified by the template parameter AttrType.
 *
 * @param AttrType The base attribute class to use.  Must be of a typed_attribute class.
 * @param Allocator The allocator for the underlying data type, default is std::allocator.
 */
template <typename AttrType,
		  typename Allocator = std::allocator<typename AttrType::traits_type::data_type> >
class standard_wrapped_attribute:
		public AttrType {
public:
	typedef Allocator alloc_type;
	typedef typename alloc_type::pointer pointer;
	typedef typename alloc_type::reference reference;
	typedef standard_wrapped_attribute<AttrType, Allocator> attr_type;
	typedef typename AttrType::traits_type traits_type;
	typedef typename AttrType::container_type container_type;
	typedef typename traits_type::data_type data_type;

	standard_wrapped_attribute():
		AttrType(), _autoflush(true), _internal(NULL), _external(NULL), _active(&_internal) {
	}

	~standard_wrapped_attribute() { destroy(); }

	/**
	 * A parameter for automatically flushing the cached type to the container.
	 * The default is true.
	 *
	 * @return The auto flush parameter
	 */
	bool &autoflush() {
		return _autoflush;
	}

	/**
	 * Retrieve the value of the attribute.  This, by default, will
	 * return the value of the reference stored.  If that data is blank,
	 * then this will use the default value.  This function will auto flush
	 * the value to the container, if set, to keep consistency.
	 *
	 * @param container The attribute container to get from
	 * @return The current value of this attribute
	 */
	data_type get(container_type &container) {
		if (!*_active || !**_active) {
			data_type data = AttrType::get(container);
			set(container, data);
		}

		if (_autoflush)
			flush(container);
		return **_active;
	}

	/**
	 * Set the value of the attribute.  This, by default, will create a local
	 * data storage area for the value, if one does not exist, and construct
	 * a value object there.  Or, if bound, the value of the reference object is
	 * set.  This function will also auto flush the value to the container, if
	 * set, to keep consistency.
	 *
	 * @param container The attribute container to set to
	 * @param data The data value to set
	 */
	void set(container_type &container, const data_type &data) {
		destroy();

		if (!*_active)
			*_active = _allocator.allocate(1);

		_allocator.construct(*_active, data);
		if (_autoflush)
			flush(container);
	}

	/**
	 * Forces a flush of the current cached value to the given container.  Note that
	 * if auto flush is set to false, then this function must be called manually, or
	 * else the attribute will not show up correctly in the container.
	 */
	void flush(container_type &container) {
		AttrType::set(container, **_active);
	}

	/**
	 * Bind the given reference object to this attribute.  From this point on, unless
	 * unbind() is called, changes to this attribute will also be reflected in the object
	 * referred to.  Unless the attribute value was flushed to the container, any
	 * previously set values are lost.
	 *
	 * @param ref The reference object to use
	 */
	void bind(reference ref) {
		destroy();
		_external = _allocator.address(ref);
		_active = &_external;
	}

	/**
	 * Sever the link to any referenced value.  From this point on, unless bind() is called,
	 * changes to this attribute are kept internally.  Unless the attribute was flushed to
	 * the container, any previously set values are lost.
	 */
	void unbind() {
		_external = NULL;
		_active = &_internal;
	}
private:
	void destroy() {
		if (_internal)
		{
			_allocator.destroy(_internal);
			_allocator.deallocate(_internal, 1);
		}

		_internal = NULL;
	}

	bool _autoflush;

	alloc_type _allocator;
	pointer _internal;
	pointer _external;

	pointer *_active;
};

}} /* End namespace */

#endif /* TMX_ATTRIBUTES_ATTRIBUTE_WRAPPED_TYPE_HPP_ */

/*
 * RtcmMessageFactory.h
 *
 * A header for the RTCM message factory
 *
 *  Created on: May 31, 2018
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_RTCM_RTCMMESSAGEFACTORY_H_
#define INCLUDE_RTCM_RTCMMESSAGEFACTORY_H_

#include "RtcmVersion.h"
#include "RtcmMessage.h"

#include <array>
#include <vector>

namespace tmx {
namespace messages {
namespace rtcm {


class RtcmMessageFactory {
public:
	TmxRtcmMessage *create(RTCM_VERSION version = UNKNOWN, msgtype_type type = 0) {
		if (!_types[rtcm::UNKNOWN].size()) {
			registerTypes<RTCM_EOF>();
		}

		// This could be a stream of messages.  Go until there are no more bytes
		if (type < _types[version].size() && _types[version][type])
			return _types[version][type]->allocate();
		else
			return _types[version][0]->allocate();
	}

	TmxRtcmMessage *create(std::string version, msgtype_type type = 0) {
		return create(RtcmVersion(version), type);
	}

private:
	/**
	 * Base template for allocator
	 */
	struct msgtype_allocator {
		virtual TmxRtcmMessage *allocate() { return NULL; }
	};

	template <class MsgType>
	struct msgtype_allocatorImpl: public msgtype_allocator {
		TmxRtcmMessage *allocate() { return new MsgType(); }
	};

	std::array<std::vector<msgtype_allocator *>, RTCM_VERSION::RTCM_EOF> _types;

	/**
	 * Base template for the registrar structure
	 */
	template<class... T> struct msgtype_registrar;

	/**
	 * Template specialization for an RTCM message type
	 */
	template<template<RTCM_VERSION, msgtype_type> class MsgType, RTCM_VERSION V, msgtype_type T>
	struct msgtype_registrar<MsgType<V, T>> {
		static void registerType(decltype(_types) &types) {
			if (types[V].size() <= T) types[V].resize(T+1, NULL);
			types[V][T] = new msgtype_allocatorImpl<MsgType<V, T>>();
		}
	};

	/**
	 * Variadic template specialization
	 */
	template<class MsgType, class... Others>
	struct msgtype_registrar<MsgType, Others...> {
		static void registerType(decltype(_types) &types) {
			msgtype_registrar<MsgType>::registerType(types);
			msgtype_registrar<Others...>::registerType(types);
		}
	};

	static RtcmMessageFactory &get_Instance() {
		static RtcmMessageFactory factory;

		// Initialize the types, if not done already

		return factory;
	}

	template <template <class...> class T, class... Types>
	void registrarDispatch(T<Types...> &tuple) {
		msgtype_registrar<Types...>::registerType(_types);
	}

	template <RTCM_VERSION Version>
	void registrarDispatch() {
		static typename RtcmMessageTypeBox<Version>::types tuple;
		registrarDispatch(tuple);
	}

	template <RTCM_VERSION Version>
	void registerTypes() {
		if (Version < RTCM_EOF) {
			registrarDispatch<Version>();
		}

		registerTypes<(RTCM_VERSION)((int)Version - 1)>();
	}

};

template <>
void RtcmMessageFactory::registerTypes<UNKNOWN>() {
	registrarDispatch<UNKNOWN>();
}


} /* End namespace rtcm */
} /* End namespace messages */
} /* End namspace tmx */


#endif /* INCLUDE_RTCM_RTCMMESSAGEFACTORY_H_ */

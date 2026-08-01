#pragma once

/*
 * The SPDU decode/unwrap/build helpers that used to live here now ship with the
 * tmxmessages headers, next to RawSpdu.h, so that TenaV2XPlugin and any other
 * plugin handling secured V2X traffic share one implementation.
 *
 * They live in namespace tmx::messages; MessageReceiverPlugin.cpp already has
 * `using namespace tmx::messages`, so existing call sites are unaffected.
 */

#include <RawSpduUtils.h>

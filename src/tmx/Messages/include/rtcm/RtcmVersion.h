/*
 * RtcmVersion.h
 *
 * Definition and operations on available RTCM message versions.
 *
 *  Created on: May 11, 2018
 *      Author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_RTCM_RTCMVERSION_H_
#define INCLUDE_RTCM_RTCMVERSION_H_

#include <cmath>
#include <cstdlib>
#include <memory>
#include <string>
#include <tmx/TmxException.hpp>

namespace tmx {
namespace messages {
namespace rtcm {

/**
 * Enumerated value for possible RTCM message versions, as well
 * as the associated numeric value.
 */
#define _Q(X) #X
#define ALL_RTCM_VERSIONS SC(2,3) SC(3,3)
#define SC(X, Y) SC1040 ## X ## _ ## Y,
enum RTCM_VERSION {
	UNKNOWN = 0, ALL_RTCM_VERSIONS RTCM_EOF
};
#undef SC
#define SC(X, Y) _Q(X ## . ## Y),
	static constexpr const char * RTCM_VERSIONS[] = { "Unknown", ALL_RTCM_VERSIONS "Unknown"};
#undef SC

constexpr const char *RtcmVersionName(RTCM_VERSION version) {
	return RTCM_VERSIONS[version];
}

template <RTCM_VERSION Version>
constexpr const char *RtcmVersionName() {
	return RtcmVersionName(Version);
}

RTCM_VERSION RtcmVersion(std::string version) {
	for (size_t i = 0; i < RTCM_EOF; i++)
		if (strcmp(RTCM_VERSIONS[i], version.c_str()) == 0)
			return (RTCM_VERSION)i;

	return UNKNOWN;
}

} /* End namespace rtcm */
} /* End namespace messages */
} /* End namespace tmx */

#endif /* INCLUDE_RTCM_RTCMVERSION_H_ */

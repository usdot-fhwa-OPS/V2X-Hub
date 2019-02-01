/*
 * FrequencyRegulator.h
 *
 *  Created on: Jun 7, 2016
 *      Author: ivp
 */

#ifndef SRC_FREQUENCYTHROTTLE_H_
#define SRC_FREQUENCYTHROTTLE_H_

#include <map>
#ifdef USE_STD_CHRONO
#include <chrono>
#define FREQUENCY_THROTTLE_DURATION_CAST std::chrono::duration_cast
#define FREQUENCY_THROTTLE_DEFAULT_DURATION std::chrono::milliseconds
#define FREQUENCY_THROTTLE_DEFAULT_CLOCK std::chrono::system_clock
#else
#include <boost/chrono.hpp>
#define FREQUENCY_THROTTLE_DURATION_CAST boost::chrono::duration_cast
#define FREQUENCY_THROTTLE_DEFAULT_DURATION boost::chrono::milliseconds
#define FREQUENCY_THROTTLE_DEFAULT_CLOCK boost::chrono::system_clock
#endif


namespace tmx {
namespace utils {

/**
 * This class is used to monitor a data source so that it can be throttled.
 */
template <class KeyType, typename Duration = FREQUENCY_THROTTLE_DEFAULT_DURATION, typename Clock = FREQUENCY_THROTTLE_DEFAULT_CLOCK>
class FrequencyThrottle
{
public:
	FrequencyThrottle();
	/**
	 * @param frequency The frequency at which Monitor returns true.
	 */
	FrequencyThrottle(Duration frequency);
	virtual ~FrequencyThrottle();

	/**
	 * Update the timestamp for the key specified and return true if the maxFrequency
	 * specified in the constructor has elapsed since the last time this method returned true
	 * for the matching key.
	 * Monitor will always return true the first time it is called for each unique key.
	 *
	 * @param key The unique key to monitor.
	 * @returns true if the frequency has elapsed since this method last returned true for the key.
	 */
	bool Monitor(KeyType key);

	/**
	 * Remove the data associated with any stale keys.
	 * This method should be called periodically if it is expected for keys to no longer be relevant.
	 */
	void RemoveStaleKeys();

	/**
	 * Get the frequency at which Monitor returns true.
	 */
	Duration get_Frequency();

	/**
	 * Set the frequency at which Monitor returns true.
	 */
	void set_Frequency(Duration frequency);

	/**
	 * Update the timestamp for the key specified in order to reset the clock.  Does
	 * nothing if the key is not found.
	 */
	void Touch(KeyType key);
private:
	Duration _frequency;
	Duration _staleDuration;
	std::map<KeyType, typename Clock::time_point> _mapLastTime;
};

template <class KeyType, typename Duration, typename Clock>
FrequencyThrottle<KeyType, Duration, Clock>::FrequencyThrottle()
{
	set_Frequency(Duration(0));
}

template <class KeyType, typename Duration, typename Clock>
FrequencyThrottle<KeyType, Duration, Clock>::FrequencyThrottle(Duration frequency)
{
	set_Frequency(frequency);
}

template <class KeyType, typename Duration, typename Clock>
FrequencyThrottle<KeyType, Duration, Clock>::~FrequencyThrottle()
{
}

template <class KeyType, typename Duration, typename Clock>
Duration FrequencyThrottle<KeyType, Duration, Clock>::get_Frequency()
{
	return _frequency;
}

template <class KeyType, typename Duration, typename Clock>
void FrequencyThrottle<KeyType, Duration, Clock>::set_Frequency(Duration frequency)
{
	_frequency = frequency;

	// A key is determined to be stale if it has been this long since Monitor has been called.
	_staleDuration = _frequency + Duration(5000);

}

template <class KeyType, typename Duration, typename Clock>
bool FrequencyThrottle<KeyType, Duration, Clock>::Monitor(KeyType key)
{
	// Find the key in the map.
	typename std::map<KeyType, typename Clock::time_point>::iterator it = _mapLastTime.find(key);

	// If key not found, store the current time, then return true to indicate it is time to do any processing.
	if (it == _mapLastTime.end())
	{
		_mapLastTime.insert(std::pair<KeyType, typename Clock::time_point>(key, Clock::now()));
		return true;
	}

	// Determine duration since last time that true was returned.
	typename Clock::time_point now = Clock::now();
	Duration duration = FREQUENCY_THROTTLE_DURATION_CAST<Duration> (now - it->second);

	// If duration surpassed, store new time and return true.
	if (duration >= _frequency)
	{
		it->second = now;
		return true;
	}

	return false;
}

template <class KeyType, typename Duration, typename Clock>
void FrequencyThrottle<KeyType, Duration, Clock>::RemoveStaleKeys()
{
	typename Clock::time_point now = Clock::now();

	typename std::map<KeyType, typename Clock::time_point>::iterator it = _mapLastTime.begin();

	while (it != _mapLastTime.end())
	{
		Duration duration = FREQUENCY_THROTTLE_DURATION_CAST<Duration> (now - it->second);

		if (duration >= _staleDuration)
	    {
			typename std::map<KeyType, typename Clock::time_point>::iterator toErase = it;
			it++;
			//std::cout << "Erasing " << toErase->first << std::endl;
			_mapLastTime.erase(toErase);
	    }
	    else
	    {
	       it++;
	    }
	}
}

template <class KeyType, typename Duration, typename Clock>
void FrequencyThrottle<KeyType, Duration, Clock>::Touch(KeyType key)
{
	// Find the key in the map.
	typename std::map<KeyType, typename Clock::time_point>::iterator it = _mapLastTime.find(key);

	// If key not found, do nothing
	if (it == _mapLastTime.end())
		return;

	// Update timestamp
	typename Clock::time_point now = Clock::now();
	it->second = now;
}

}} // namespace tmx::utils

#endif /* SRC_FREQUENCYTHROTTLE_H_ */

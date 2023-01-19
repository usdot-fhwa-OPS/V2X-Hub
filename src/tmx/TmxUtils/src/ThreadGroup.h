/*
 * ThreadGroup.h
 *
 *  Created on: May 5, 2017
 *      Author: gmb
 */

#ifndef SRC_THREADGROUP_H_
#define SRC_THREADGROUP_H_

#include <atomic>
#include <boost/algorithm/string.hpp>
#include <cstdlib>
#include <deque>
#include <vector>
#include <random>
#include <algorithm>

namespace tmx {
namespace utils {

/**
 * A template class that defines how tasks are assigned to individual threads
 * The ThreadClass implementation is designed to be a lock-free thread implementation,
 * but in reality can be ay implementation that has a default constructor and the following
 * functions:
 * 		void start() - Starts the thread
 * 		void stop() - Stops the thread
 * 		size_t inQueueSize() - The size of the current in-coming queue
 *
 * 	Note that std::thread does not work for this, so a wrapper must be created to use
 * 	a standard thread.
 *
 * 	Task assignment can be done by
 *
 * @see LockFreeThread
 */
template <class ThreadClass, typename GroupT = uint8_t, typename IdentifierT = uint8_t>
class ThreadGroup {
public:
	using group_type = GroupT;
	using id_type = IdentifierT;
	static constexpr size_t max_groups = ::pow(2, 8 * sizeof(group_type));
	static constexpr size_t max_ids = ::pow(2, 8 * sizeof(id_type));

	ThreadGroup(): gen(std::random_device()()) {
		srand(time (NULL));

		// Initialize the queue assignments
		for (size_t i = 0; i < max_groups; i++) {
			for (size_t j = 0; j < max_ids; j++) {
				assignments[i][j].count = 0;
				assignments[i][j].threadId = -1;
			}
		}
	}

	/**
	 * @return The number of threads in the group
	 */
	size_t size() {
		return _threads.size();
	}

	/**
	 * Set the number of threads in the group.  This can only increase beyond the current
	 * size, and new threads added to te group will be automatically started.
	 */
	void set_size(size_t size) {
		for (size_t i = 0; _threads.size() < size; i++) {
			_threads.emplace_back();
			_threads[i].start();
		}
	}

	/**
	 * Assign an incoming item to the incoming queue of a thread.  If the group and id are set to
	 * any non-zero value and there already is an thread assignment for that group and id, then
	 * the existing thread assignment will be used.  If no thread assignment exists, or group and
	 * id are set to zero, indicating thread assignment should be ignored, then the item is assigned
	 * to a thread based on the specified assignment strategy, either round-robin (default), random,
	 * or shortest-queue.
	 *
	 * @param group The group identifier, or 0 for no group
	 * @param id The unique identifier in the group, or 0 for no identifier
	 * @see set_strategy(const std::string &)
	 */
	void assign(group_type group, id_type id, const typename ThreadClass::incoming_item &item) {
		static std::atomic<uint32_t> next {0};

		if (_threads.size() == 0)
			return;

		int tId = -1;

		// Need this here to ensure no pre-mature free up of the thread ID
		assignments[group][id].count++;

		tId = assignments[group][id].threadId;

		// If no group and no id, then any existing thread assignment should be ignored
		if (tId < 0 || (group == 0 && id == 0)) {
			// No thread assignment.  Assign using assignment strategy
			switch (_strategy) {
			case strategy_RoundRobin:
				tId = next;
				if (++next >= _threads.size())
					next = 0;
				break;
			case strategy_Random:
				{
					uniform_int_distribution<> dis(0, _threads.size() -1);
					tId = dis(gen);
				}
				break;
			case strategy_ShortestQueue:
				tId = 0;
				for (size_t i = 1; i < _threads.size(); i++) {
					if (_threads[i].inQueueSize() < _threads[tId].inQueueSize())
						tId = i;
				}
			}

			assignments[group][id].threadId = tId;
		}

		_threads[tId].push(item);
	}

	/**
	 * Remove a thread assignment for a specific group and id, assuming no more tasks
	 * exists for that thread to finish.  If this is never called, then the thread
	 * assignment for the group and id will last forever.
	 *
	 * @param group The group identifier
	 * @param id The unique identifier in the group
	 */
	void unassign(group_type group, id_type id) {
		if (!(--assignments[group][id].count))
			assignments[group][id].threadId = -1;
	}

	/**
	 * Stop the thread group
	 */
	void stop() {
		size_t total = _threads.size();
		for (size_t i = 0; i < total; i++) {
			_threads[0].stop();
			_threads.pop_front();
		}
	}

	/**
	 * @param Thread id in the group
	 * @returns A reference to the thread object at that id
	 */
	ThreadClass &operator[](size_t n) {
		return _threads[n];
	}

	/**
	 * @param Thread id in the group
	 * @returns A reference to the thread object at that id
	 */
	ThreadClass &operator[](size_t n) const {
		return _threads[n];
	}

	/**
	 * Sets an assignment strategy by name, which is one of:
	 * 		RoundRobin
	 * 		Random
	 * 		ShortestQueue
	 * The string compare is case-insensitive.
	 * @param strategy The new strategy
	 */
	void set_strategy(const std::string & strategy) {
		_strategy = get_strategy(strategy);
	}

	int this_thread() {
		size_t total = _threads.size();
		for (size_t i = 0; i < total; i++) {
			if (_threads[i].get_id() == std::this_thread::get_id())
				return i;
		}

		return -1;
	}
private:
	mt19937 gen; // Standard mersenne_twister_engine seeded with rd()
	std::deque<ThreadClass> _threads;

	struct source_info {
		std::atomic<uint64_t> count;
		std::atomic<int> threadId;
	};

	source_info assignments[max_groups][max_ids];

	enum ThreadGroupAssignmentStrategy {
		strategy_RoundRobin,
		strategy_Random,
		strategy_ShortestQueue,
		strategy_END
	};

	ThreadGroupAssignmentStrategy _strategy = strategy_RoundRobin;

	ThreadGroupAssignmentStrategy get_strategy(const std::string &str) {
		static std::vector<std::string> allStrategies(strategy_END);
		if (allStrategies.empty()) {
			// Initialize all the strategies
#define LOAD(X) allStrategies[strategy_ ## X] = #X
			LOAD(RoundRobin);
			LOAD(Random);
			LOAD(ShortestQueue);
#undef LOAD
		}

		for (size_t i = 0; i < allStrategies.size(); i++) {
			if (boost::iequals(allStrategies[i], str))
				return (ThreadGroupAssignmentStrategy)i;
		}

		// Return existing strategy
		return _strategy;
	}
};


}} // namespace tmx::utils

#endif /* SRC_THREADGROUP_H_ */

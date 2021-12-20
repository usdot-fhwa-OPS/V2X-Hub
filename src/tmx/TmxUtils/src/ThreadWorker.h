/*
 * ThreadWorker.h
 *
 *  Created on: Aug 19, 2016
 *      Author: ivp
 */

#ifndef SRC_THREADWORKER_H_
#define SRC_THREADWORKER_H_

#include <atomic>
#include <thread>

namespace tmx {
namespace utils {

/**
 * Abstract class that manages a thread for performing work.
 */
class ThreadWorker {
public:
	ThreadWorker();
	virtual ~ThreadWorker();

	/**
	 * Start the thread worker.
	 */
	void Start();

	/**
	 * Stop the thread worker.
	 */
	void Stop();

	/**
	 * @return True if the background worker is currently running.
	 */
	bool IsRunning();

protected:
	/**
	 * The parent class implements this method to do the work in the spawned thread.
	 * DoWork should exit when the _stopThread variable has a value of false.
	 * The example below can be used as a template.
	 *
	 * void ParentClass::DoWork()
	 * {
	 * 	 while (!_stopThread)
	 *     this_thread::sleep_for(chrono::milliseconds(50));
	 * }
	 *
	 */
	virtual void DoWork() = 0;

	/**
	 * When this value is set to false (by the StopWorker method), the DoWork method should exit.
	 */
	std::atomic<bool> _stopThread{false};

private:
	std::thread* _thread = nullptr;
};

} /* namespace utils */
} /* namespace tmx */

#endif /* SRC_THREADWORKER_H_ */

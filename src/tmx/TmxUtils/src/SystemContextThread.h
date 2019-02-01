/*
 * SystemContextThread.h
 *
 *  Created on: Feb 18, 2016
 *      Author: Svetlana Jacoby
 */

#ifndef SYSTEMCONTEXT_THREAD_H_
#define SYSTEMCONTEXT_THREAD_H_

#include <string>
#include "ThreadTimer.h"
//#include "database/SystemContext.h"


using namespace std;
using namespace tmx;

namespace tmx {
namespace utils {

//class SystemContext;

class SystemContextThread : public ThreadTimerClient
{
public:
//	SystemContextThread(tmx::utils::SystemContext &context, ThreadTimer& timer);
	SystemContextThread(ThreadTimer& timer);
//	void set_Context(tmx::utils::SystemContext &context);
	virtual ~SystemContextThread();

private:
//	static tmx::utils::SystemContext &_sysContext;
	void TimerTick();
};

}}

#endif /* SYSTEMCONTEXT_THREAD_H_ */

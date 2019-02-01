/*
 * SystemContextThread.cpp
 *
 *  Created on: Feb 18, 2016
 *      Author: Svetlana Jacoby
 */

#include <assert.h>
#include "PluginLog.h"
#include "database/SystemContext.h"
#include "SystemContextThread.h"

using namespace std;
using namespace std::chrono;
//using namespace tmx::messages;

namespace tmx {
namespace utils {

SystemContextThread::SystemContextThread(ThreadTimer& timer) :
	ThreadTimerClient(timer)
{}

//SystemContextThread::SystemContextThread(SystemContext &context, ThreadTimer& timer) :
//	ThreadTimerClient(timer)
//{_sysContext = context;}


SystemContextThread::~SystemContextThread()
{}

//void SystemContextThread::set_Context(tmx::utils::SystemContext &context)
//{
////	_sysContext = context;
//}

void SystemContextThread::TimerTick()
{
	try
	{
//		_sysContext.updateLatencyDb();
		tmx::utils::SystemContext::updateLatencyDb();
	}
	catch (std::exception &e)
	{
		FILE_LOG(logERROR) << "Unexpected exception in SystemContextThread TimerTick: " << e.what();
	}
}

}}


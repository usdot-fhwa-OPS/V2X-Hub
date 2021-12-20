/*
 * Signals.h
 *
 *  Created on: Oct 27, 2016
 *      Author: gmb
 */

#ifndef SRC_PLUGINEXEC_H_
#define SRC_PLUGINEXEC_H_

#include "PluginLog.h"
#include "SignalException.h"

#include <boost/program_options.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <cstdio>
#include <execinfo.h>
#include <ostream>
#include <signal.h>
#include <string>
#include <ucontext.h>
#include <unistd.h>
#include <vector>

#define USAGE "Usage"
#ifndef INPUT_FILES_PARAM
#define INPUT_FILES_PARAM "input-file"
#endif

namespace tmx
{
namespace utils
{

/* This structure mirrors the one found in /usr/include/asm/ucontext.h */
typedef struct _sig_ucontext {
 unsigned long     uc_flags;
 struct ucontext   *uc_link;
 stack_t           uc_stack;
 struct sigcontext uc_mcontext;
 sigset_t          uc_sigmask;
} sig_ucontext_t;

// Functions for handling the program options
boost::program_options::options_description_easy_init AddOptions();
boost::program_options::options_description &GetOptions();
bool ProcessDefaults(const boost::program_options::variables_map &);
void Cleanup();

// Functions for handling the signals
std::string ExceptionToString(std::exception &, std::string = "", bool abort = true);
void HandleSignal(int, siginfo_t *, void *);
void RegisterSignals(std::string);

// Functions for launching a program

/**
 * The TMX abstract runnable class, which includes an optional function to process program arguments,
 * and a Main function to execute.  Note that default options are always processed automatically.
 */
class Runnable
{
public:
	Runnable(const char *inputParamName = INPUT_FILES_PARAM, const char *inputParamDescr = "Optional input file");
	virtual ~Runnable() { Cleanup(); }
	virtual bool ProcessOptions(const boost::program_options::variables_map &);
	virtual int Main() = 0;
	const char *inFileParam = nullptr;
};

/**
 * A version of the program launcher for code that exists outside of a standard Plugin.
 * Note that there is no way to log events with this invocation.
 *
 * @param name The name of the command to run, defaults to the program name
 * @param argc The number of program arguments
 * @param argv The program arguments
 * @param runnable The runnable instance to execute
 * @param newThread Launch the function in a new thread
 */
int run(std::string name, int argc, char *argv[], Runnable &runnable, bool newThread = false);

/**
 * A version of the program launcher for Plugin code
 */
template <typename Plugin>
int run_plugin(std::string name, int argc, char *argv[])
{
	Plugin plugin(name);

	try
	{
		return run(plugin.GetName(), argc, argv, plugin);
	}
	catch (std::exception &ex)
	{
		plugin.HandleException(ex, true);
		return -1;
	}
}

} /* namespace utils */
} /* namespace tmx */


#endif /* SRC_PLUGINEXEC_H_ */

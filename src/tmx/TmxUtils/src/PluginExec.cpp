/*
 * @file PluginExec.cpp
 *
 *  Created on: Oct 27, 2016
 *      @author: gmb
 */

#include "PluginExec.h"
#include "PluginClient.h"

#include <algorithm>
#include <iostream>

#define MAX

using namespace std;
using namespace boost::program_options;

namespace tmx
{
namespace utils
{

static vector<int> _signals { SIGABRT, SIGFPE, SIGILL, SIGSEGV, SIGUSR1, SIGUSR2 };

static string reporter;
static bool aborting = false;

static options_description _option_desc;

options_description_easy_init AddOptions() { return _option_desc.add_options(); }
options_description &GetOptions() { return _option_desc; }

template <class Exception>
void add_exception_details(Exception &ex, char const *current_function, char const * file, int line)
{
	ex  << boost::throw_function(current_function)
		<< boost::throw_file(file)
		<< boost::throw_line(line);
}

void Cleanup()
{
	if (Output2FILE::Stream() != stdout)
		fclose(Output2FILE::Stream());
}

Runnable::Runnable(const char *inputParamName, const char *inputParamDescr): inFileParam(inputParamName)
{
	// Add default options
	AddOptions()
			("help,h", "This help screen")
			//("manifest,m", boost::program_options::value<std::string>()->default_value(IVPREGISTER_MANIFEST_FILE_NAME), "Plugin manifest file")
			("level,l", value<std::string>(), "Log level, i.e. ERROR, WARNING, INFO, DEBUG, DEBUGn where n=1-4")
			("output,o", value<std::string>()->default_value("-"), "Log output file.  Use - for standard output")
			(inputParamName, value< vector<string> >()->default_value(vector<std::string>(), ""), inputParamDescr);
}

bool Runnable::ProcessOptions(const variables_map &opts)
{
	boost::property_tree::ptree cfg;

	if (opts.count("level"))
	{
		std::string lvl = opts["level"].as<std::string>();
		std::transform(lvl.begin(), lvl.end(), lvl.begin(), ::toupper);
		FILELog::ReportingLevel() = FILELog::FromString(lvl);
	}

	if (opts.count("manifest"))
	{
		std::string mf = opts["manifest"].as<std::string>();

		try
		{
			boost::property_tree::read_json(mf, cfg);
		}
		catch (std::exception &ex)
		{
			std::cerr << "Unable to read " << mf << ": " << ex.what() << std::endl;
			return false;
		}
	}

	FILE *logF = NULL;
	if (opts.count("output"))
	{
		std::string fn = opts["output"].as<std::string>();

		if (fn != "-")
		{
			logF = fopen(fn.c_str(), "w");
			if (logF == NULL)
				std::cerr << "Could not open log file: " << strerror(errno) << ".  Logging to standard output." << std::endl;
		}

		if (logF == NULL)
			Output2FILE::Stream() = stdout;
		else
			Output2FILE::Stream() = logF;
	}

	return true;
}

int run(std::string name, int argc, char *argv[], Runnable &runnable, bool newThread)
{
	RegisterSignals(name);

	stringstream ss;
	copy(argv, argv+argc, ostream_iterator<char *>(ss, ","));

	std::string title = USAGE;
	if (argc > 0 || argv[1] != NULL)
	{
		// Add program name to the usage
		title += ": ";
		title += argv[0];

		if (name.empty())
			name = argv[0];
	}

	boost::program_options::options_description desc(title);
	boost::program_options::variables_map opts;

	desc.add(GetOptions());

	boost::program_options::positional_options_description p;
	p.add(runnable.inFileParam, -1);

	bool optInvalid = false;

	try {
		store(command_line_parser(argc, argv).options(desc).positional(p).run(), opts);
		notify(opts);

		optInvalid = opts.count("help") || !runnable.ProcessOptions(opts);
	}
	catch (exception &ex) {

		// Unable to process arguments
		std::cerr << ex.what() << std::endl << std::endl;
		optInvalid = true;
	}

	if (optInvalid)
	{
		std::cerr << desc << std::endl;
		exit(-1);
	}

	// Needs to be after the options are processed for log level
	PLUGIN_LOG(logDEBUG, name) << "Program invoked with " << argc << " argument" <<
			((argc > 1) ? "s: " : ": ") << ss.str();

	if (newThread)
	{
		PLUGIN_LOG(logDEBUG1, name) << "Launching Main in a new thread";
		std::thread execThread(&Runnable::Main, &runnable);
		execThread.join();

		return 0;
	}
	else
	{
		return runnable.Main();
	}
}

string ExceptionToString(exception &ex, std::string name, bool abort)
{
	std::stringstream ss;
	ss << (name.empty() ? "Program" : name);
	if (abort)
		ss << " terminating from ";
	else
		ss << " encountered ";
	ss << "unhandled exception: ";

	try
	{
		ss << dynamic_cast<TmxException &>(ex) << endl;
	}
	catch (bad_cast &bc)
	{
		PluginException pe(ex);
		add_exception_details(pe, __FUNCTION__, __FILE__, __LINE__);
		ss << "(Unknown source): " << pe << endl;
	}

	return ss.str();
}

void HandleSignal(int sig_num, siginfo_t * info, void * ucontext)
{
	if (aborting)
		return;

	FILE_LOG(logDEBUG2) << "Inside signal handler with signal " << sig_num;

	// Taken mostly from http://stackoverflow.com/questions/77005/how-to-generate-a-stacktrace-when-my-gcc-c-app-crashes
	void * caller_address;
	sig_ucontext_t * uc;

	uc = (sig_ucontext_t *) ucontext;

	/* Get the address at the time the signal was raised */
#if defined(__i386__) // gcc specific
	caller_address = (void *) uc->uc_mcontext.eip; // EIP: x86 specific
#elif defined(__x86_64__) // gcc specific
	caller_address = (void *) uc->uc_mcontext.rip; // RIP: x86_64 specific
#elif defined(__arm__)
			caller_address = (void *) uc->uc_mcontext.arm_pc; // ARM specific
#else
			caller_address = 0; // TODO: Add support for other arch.
#endif

	SignalException ex(sig_num);
	add_exception_details(ex, __FUNCTION__, __FILE__, __LINE__);

	if (caller_address)
		ex.SetBacktrace((void *) caller_address);

	// Try to find the right plugin by name
	PluginClient *plugin = PluginClient::FindPlugin(reporter);

	// User signals sent just to force a stack trace, so do not abort
	aborting = (sig_num != SIGUSR1 && sig_num != SIGUSR2);

	if (plugin)
	{
		plugin->HandleException(ex, aborting);
	}
	else
	{
		std::cerr << ExceptionToString(ex, reporter, aborting);
		if (aborting)
		{
			// Wait for message
			sleep(1);
			std::terminate();
		}
	}

	aborting = false;
}

void RegisterSignals(string pluginName)
{
	if (!reporter.empty())
	{
		// Not sure why this would ever happen, but log it if it does
		PLUGIN_LOG(logWARNING, reporter) <<
				"Signal reporting plugin replaced by " << pluginName;
	}

	reporter = pluginName;

	struct sigaction sigact;

	sigact.sa_sigaction = HandleSignal;
	sigact.sa_flags = SA_RESTART | SA_SIGINFO;

	// Set up all the signal handlers
	for (int i : _signals)
	{
		PLUGIN_LOG(logDEBUG, pluginName) << "Creating handler for signal " << i;

		if (sigaction(i, &sigact, (struct sigaction *)NULL) != 0)
			BOOST_THROW_EXCEPTION(PluginException(string("Unable to initialize handler for ") + SignalException(i).what()));
	}
}


} /* namespace utils */
} /* namespace tmx */

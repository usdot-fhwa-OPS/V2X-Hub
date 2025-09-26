/*
 * PluginInstall.cpp
 *
 *  Edited on: Nov 25, 2019
 *      Author: animeshb27
 */

#include "TmxControl.h"

#include <boost/filesystem.hpp>
#include <boost/throw_exception.hpp>
#include <libgen.h>
#include <System.h>
#include <sys/stat.h>

#ifndef PLUGINDIRECTORY_ENV
#define PLUGINDIRECTORY_ENV "TMX_PLUGIN_DIRECTORY"
#endif

#define PLUGIN_DELETE_STMT "\
	DELETE FROM IVP.plugin \
	WHERE name = ?"

using namespace std;
using namespace sql;
using namespace tmx;
using namespace tmx::utils;

namespace tmxctl {

bool TmxControl::plugin_install(pluginlist &plugins, ...) {
	if (!checkPerm())
		return false;
	return plugin_install();
}

bool TmxControl::plugin_install() {
	pluginlist plugins;
	plugins.push_back("%");

	struct stat tmpStat;
	boost::filesystem::path temp;

	boost::filesystem::path file((*_opts)["plugin-install"].as<string>());

	if (file.empty() || access(file.c_str(), R_OK) < 0) {
		PLOG(logERROR) << "Could not open file " << file << " for install";
		return false;
	}

	string cmd = "file -b ";
	cmd += file.string();
	cmd += " 2>/dev/null";

	string cmdResults = System::ExecCommand(cmd);
	string ext = file.extension().string();

	if (!cmdResults.empty())
		PLOG(logDEBUG1) << "File returns: " << cmdResults;

	PLOG(logDEBUG1) << "File extension is " << ext;

	// Determine the directory to extract to from the manifest option
	boost::filesystem::path cwd = boost::filesystem::current_path();
	boost::filesystem::path pluginDir((*_opts)["plugin-directory"].as<string>());

	PLOG(logDEBUG) << "Installing plugin to " << pluginDir;

#define tstr(X) strncmp(X, cmdResults.c_str(), strlen(X)) == 0

	string exCmd = "tar -x -C ";
	exCmd += pluginDir.string();
	exCmd += " -f ";

	string listCmd = "tar -t -f ";

	// Handle the Debian archive separately
	if (tstr("Debian") || ext == ".deb") {
		int exitCode = 1;

#ifndef NO_DEBIAN_PKG_MGR
		// Try the native package manager first
		string dpkgResults = System::ExecCommand("dpkg -l >/dev/null 2>&1", &exitCode);
		if (exitCode == 0) {
			cmd = "sudo dpkg --install ";
			cmd += file.string();

			PLOG(logDEBUG) << "Installing Debian package with " << cmd;

			exitCode = system(cmd.c_str());
			return (exitCode == 0);
		}
#endif

		temp = boost::filesystem::temp_directory_path();
		temp /= boost::filesystem::unique_path();

		mkdir(temp.c_str(), S_IRWXU);

		// A .deb is just an ar archive with the tar file inside it
		boost::filesystem::path canonicalPath = boost::filesystem::canonical(file, cwd);

		if (chdir(temp.c_str()) != 0)
		{
			BOOST_THROW_EXCEPTION(std::runtime_error(std::string("chdir(") + temp.c_str() + "): " + strerror(errno)));
		}
		PLOG(logDEBUG) << "Writing in " << temp;

		cmd = "ar -x ";
		cmd += canonicalPath.string();

		PLOG(logDEBUG) << "Extracting debian archive with " << cmd;

		cmdResults = System::ExecCommand(cmd, &exitCode);
		if (exitCode < 0) {
			rmdir(temp.c_str());
			return false;
		}

		bool found = false;
		for (boost::filesystem::directory_iterator iter = boost::filesystem::directory_iterator(temp);
				iter != boost::filesystem::directory_iterator(); iter++) {
			if (strncmp("data.tar", iter->path().filename().c_str(), 8) == 0) {
				file = iter->path();
				ext = file.extension().string();

				PLOG(logDEBUG1) << "Data file is " << file;
				found = true;
			} else {
				// Don't need it
				unlink(iter->path().c_str());
			}
		}

		if (!found) {
			PLOG(logERROR) << "Debian package appears corrupt";
			rmdir(temp.c_str());
			return false;
		}

		if (chdir("/") != 0)
		{
			BOOST_THROW_EXCEPTION(std::runtime_error(std::string("chdir(/): ") + strerror(errno)));
		}

		// Path in the Debian archive is relative from /.
		// Therefore, do not use the -C option
		exCmd = "tar -x -f ";
	}

	string compressOpt = "";

	if (tstr("gzip") || ext == ".gz" || ext == ".tgz") {
		compressOpt = " -z";
	} else if (tstr("Zip") || ext == ".zip") {
		listCmd = "unzip -l ";
		exCmd = "unzip -o -d ";
		exCmd += pluginDir.string();
		exCmd += " ";
	} else if (tstr("compress'd") || ext == ".Z" || ext == ".tZ") {
		compressOpt = " -Z";
	} else if (tstr("bzip2") || ext == ".bz2" || ext == ".tbz2") {
		compressOpt = " -j";
	} else if (tstr("XZ") || ext == ".xz" || ext == ".txz") {
		compressOpt = " -J";
	}

#undef tstr

	cmd = listCmd;
	cmd += file.string();
	cmd += compressOpt;
	cmd += " 2>/dev/null | grep \"manifest.json$\"";

	PLOG(logDEBUG) << "Listing archive with " << cmd;

	cmdResults = System::ExecCommand(cmd);

	PLOG(logDEBUG1) << "Command returned with " << cmdResults;

	// Make sure this is not a relative path
	if (cmdResults[0] == '.')
		cmdResults.erase(0, 1);

	// ZIP files contain some other stuff, fast forward to the last field
	size_t idx = cmdResults.find_last_of(' ');
	if (idx != string::npos)
		cmdResults = cmdResults.substr(idx+1);

	boost::filesystem::path mfest = boost::filesystem::absolute(cmdResults, pluginDir);

	cmd = exCmd;
	cmd += file.string();
	cmd += compressOpt;

	PLOG(logDEBUG) << "Installing files with " << cmd;
	System::ExecCommand(cmd.c_str());

	if (chdir(cwd.c_str()) != 0)
	{
		BOOST_THROW_EXCEPTION(std::runtime_error(std::string("chdir(") + cwd.c_str() + "): " + strerror(errno)));
	}

	if (!temp.empty()) {
		// Remove the temp file
		unlink(file.c_str());
		rmdir(temp.c_str());
	}

	if (stat(mfest.parent_path().c_str(), &tmpStat) != 0) {
		PLOG(logERROR) << "Could not extract archive.  Missing " << mfest.parent_path();
		return false;
	}

	// Try to set permissions on the file
	cmd = "chown -R www-data:www-data ";
	cmd += mfest.parent_path().string();
	cmd += " >/dev/null 2>&1";

	PLOG(logDEBUG) << "Changing ownership with " << cmd;
	System::ExecCommand(cmd);

	PLOG(logDEBUG) << "Loading manifest from " << mfest;

	boost::any m(mfest.string());
	(*_opts).insert(make_pair("load-manifest", boost::program_options::variable_value(m, false)));
	return this->load_manifest(plugins);
}

bool TmxControl::plugin_remove(pluginlist &, ...) {
	if (!checkPerm())
		return false;
	return plugin_remove();
}

bool TmxControl::plugin_remove() {
	struct stat tmpStat;

	pluginlist plugins(1, (*_opts)["plugin-remove"].as<string>());

	// Get the path for the plugin
	this->list(plugins);
	message_path_type basePath(plugins[0], ATTRIBUTE_PATH_CHARACTER);

	message_path_type pathPath(basePath);
	pathPath /= message_path_type("path", ATTRIBUTE_PATH_CHARACTER);

	boost::filesystem::path dirPath(_output.retrieve(pathPath, "/tmp/__?NO+PATH?__"));

	pathPath = basePath;
	pathPath /= message_path_type("exeName", ATTRIBUTE_PATH_CHARACTER);

	boost::filesystem::path exePath = dirPath;
	exePath += _output.retrieve(pathPath, "/__?NO+FILE?__");

	_output.get_storage().get_tree().clear();

	// If the active system has an installed package manager, invoke that first
	if (!exePath.empty() && stat(exePath.c_str(), &tmpStat) == 0) {
		PLOG(logDEBUG1) << "Plugin found under " << dirPath;


#ifndef NO_DEBIAN_PKG_MGR
		string cmd = "dpkg -S ";
		cmd += exePath.string();
		cmd += " 2>/dev/null";

		string dpkgResult = System::ExecCommand(cmd);

		if (!dpkgResult.empty()) {
			// There is a Debian package component installed for this plugin, so execute the removal process
			// First get the component name from the output
			size_t idx = dpkgResult.find_first_of(':');
			if (idx != string::npos) {
				cmd = "sudo dpkg --remove ";
				cmd += dpkgResult.substr(0, idx);

				PLOG(logDEBUG1) << "Removing Debian package with " << cmd;

				int exitVal = system(cmd.c_str());
				return exitVal == 0;
			}
		}
#endif
	}

	try
	{
		DbConnection conn = _pool.Connection();

		std::ostringstream dltinspl;
		dltinspl << "DELETE FROM IVP.installedPlugin WHERE path = " << dirPath;

		std::string query = dltinspl.str();

		// Delete from installedPlugin table
		PLOG(logDEBUG1) << "Executing query: " << query;

		unique_ptr<PreparedStatement> stmt(conn.Get()->prepareStatement(query));
		int rows = stmt->executeUpdate();

		// Delete from plugin table
		PLOG(logDEBUG1) << "Executing query (?1 = " << plugins[0] << "): " << PLUGIN_DELETE_STMT;

		stmt.reset(conn.Get()->prepareStatement(PLUGIN_DELETE_STMT));
		stmt->setString(1, plugins[0]);
		rows += stmt->executeUpdate();

		PLOG(logDEBUG1) << "Deleted " << rows << " rows.";

		// Remove the installed files
		if (!dirPath.empty() && stat(dirPath.c_str(), &tmpStat) == 0) {
			PLOG(logDEBUG1) << "Deleting file path " << dirPath;

			boost::filesystem::remove_all(dirPath);
		}

		return rows > 0;
	}
	catch (exception &ex)
	{
		PLOG(logERROR) << TmxException(ex);
		return false;
	}
}

}

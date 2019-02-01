/*
 * j2735dump.cpp
 *
 *  Created on: Dec 15, 2016
 *      @author: gmb
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/version.hpp>
#include <cstdio>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>
#include <tmx/messages/message_converter.hpp>
#include <PluginExec.h>
#include <BsmConverter.h>

#ifndef DEFAULT_TREEREPAIR
#define DEFAULT_TREEREPAIR treerepair.xml
#endif


using namespace std;
using namespace tmx;
using namespace tmx::messages;
using namespace tmx::utils;

namespace j2735dump
{

#if BOOST_VERSION < 105800
typedef typename std::string::value_type xml_settings_type;
#else
typedef std::string xml_settings_type;
#endif

class J2735Dump: public Runnable
{
public:
	J2735Dump(): Runnable(), msg("-")
	{
		AddOptions()
			("repair-file,r", boost::program_options::value<string>()->default_value(quoted_attribute_name(DEFAULT_TREEREPAIR)), "XML to JSON repair file.")
			("no-pretty-print", "Do not pretty print the output.  Default is false")
			("no-blobs", "Do not crack the BSM blobs.  Default is false")
			("xml,x", "Write output as XML.  Default is to convert to JSON");
	}

	inline int Main()
	{
		if (msg == "-")
		{
			char buf[4000];
			cin.getline(buf, sizeof(buf));
			msg = string(buf);
		}

		PLOG(logDEBUG) << "Decoding message: " << msg;

		// Figure out if this is a JSON message or just the bytes
		bool jsonMsg = false;

		int i;
		for (i = 0; i < (int)msg.size() && (msg[i] == '\t' || msg[i] == ' '); i++); // Skip white space
		if (msg[i] == '{') jsonMsg = true;

		byte_stream bytes;
		if (jsonMsg)
		{
			routeable_message routeableMsg;
			routeableMsg.set_contents(msg);

			PLOG(logDEBUG) << "Decoding routeable message: " << routeableMsg;

			bytes = routeableMsg.get_payload_bytes();
		}
		else
		{
			bytes = battelle::attributes::attribute_lexical_cast<byte_stream>(msg);
		}

		PLOG(logDEBUG) << "Decoding bytes: " << bytes;

		J2735MessageFactory factory;
		TmxJ2735EncodedMessageBase *tmxMsg = factory.NewMessage(bytes);
		if (!tmxMsg)
		{
			BOOST_THROW_EXCEPTION(factory.get_event());
			exit(-1);
		}

		// Get a copy of the property tree to use and manipulate
		PLOG(logDEBUG) << "Trying to copy message " << *tmxMsg;
		PLOG(logDEBUG) << "Payload: " << tmxMsg->get_payload();

		xml_message msg(tmxMsg->get_payload().get_container());

		// Obtain a copy of the ptree for manipulation
		message_tree_type tree = msg.get_container().get_storage().get_tree();
		PLOG(logDEBUG) << "Copy: " << msg;

		delete tmxMsg;

		// If this is a BSM, then replace the contents of the blob with the decoded information
		if (blobs && tree.count("BasicSafetyMessage"))
		{
			static DecodedBsmMessage crackedBsm;

			// Decode the BSM
			BsmMessage bsmMessage;
			bsmMessage.set_contents(tree);

			PLOG(logDEBUG) << "Cracking BSM " << bsmMessage;
			BsmConverter::ToDecodedBsmMessage(*(bsmMessage.get_j2735_data()), crackedBsm);

			// Reset the blob1 data in the message with the decoded information
			string old_val = tree.get<string>("BasicSafetyMessage.blob1", "?");
			tree.put_child("BasicSafetyMessage.blob1", crackedBsm.get_container().get_storage().get_tree());
			tree.put("BasicSafetyMessage.blob1.Bytes", old_val);
			msg.set_contents(tree);
		}

		if (xml)
		{
			boost::property_tree::xml_writer_settings<xml_settings_type> xmlSettings;
			if (pretty)
				xmlSettings = boost::property_tree::xml_writer_make_settings<xml_settings_type>('\t', 1);
			boost::property_tree::write_xml(cout, msg.get_container().get_storage().get_tree(), xmlSettings);
		}
		else
		{
			tmx::XML ser;
			message_converter *conv;

			try
			{
				PLOG(logDEBUG) << "Trying to open converter file " << converter;
				conv = new message_converter(ser, converter);
			}
			catch (exception &ex)
			{
				PLOG(logDEBUG2) << ex.what();
				conv = 0;
			}

			json_message json(msg, conv);
			boost::property_tree::write_json(cout, json.get_container().get_storage().get_tree(), pretty);
		}

		return (0);
	}

	inline bool ProcessOptions(const boost::program_options::variables_map &opts)
	{
		Runnable::ProcessOptions(opts);

		if (opts.count(INPUT_FILES_PARAM))
		{
			vector<string> files = opts[INPUT_FILES_PARAM].as< vector<string> >();
			if (files.size())
				msg = files[0];
		}

		xml = opts.count("xml");
		pretty = !(opts.count("no-pretty-print"));
		blobs = !(opts.count("no-blobs"));
		converter = opts["repair-file"].as<string>();

		return true;
	}

private:
	string msg;
	string converter;
	bool xml = false;
	bool pretty = true;
	bool blobs = true;
};

} /* End namespace */

int main(int argc, char *argv[])
{
	FILELog::ReportingLevel() = logERROR;

	try
	{
		j2735dump::J2735Dump myExec;
		run("", argc, argv, myExec);
	}
	catch (exception &ex)
	{
		cerr << ExceptionToString(ex) << endl;
		throw;
	}
}


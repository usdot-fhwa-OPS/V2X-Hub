/*
 * @file TmxMessageManager.cpp
 *
 *  Created on: Oct 31, 2017
 *      @author: Gregory M. Baumgardner
 */

#include "TmxMessageManager.h"

#include "Clock.h"
#include "LockFreeThread.h"
#include "ThreadGroup.h"

#include <condition_variable>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>

#define MAX_USEC_SLEEP 50000

namespace tmx {
namespace utils {

enum incomingMessageType {
	type_RawBytes = 0,
	type_IvpMessage
};

struct rawIncomingMessage {
	uint8_t groupId;
	uint8_t uniqId;
	uint64_t timestamp;
	TmxMessageManager *mgr;
	void *message;
	incomingMessageType type;
	char *encoding;
};

struct rawOutgoingMessage {
	uint8_t groupId;
	uint8_t uniqId;
	tmx::routeable_message *msg;
};

class RxThread: public LockFreeThread<rawIncomingMessage, rawOutgoingMessage> {
public:
	RxThread() {}
	~RxThread() {}
protected:
	void doWork(rawIncomingMessage &msg);
	void idle();
private:
	// Each thread keeps its own additive increasing, multiplicative decreasing sleep time
	double usecSleep = 0;
};

static std::atomic<uint16_t> overflow {DEFAULT_OVERFLOW_CAPACITY};
static constexpr uint8_t sleepInc = 10;
static constexpr double sleepDec = 1.0 / sleepInc;

// The workerThreads
typedef tmx::utils::ThreadGroup<RxThread> RxThreadGroup;
static RxThreadGroup workerThreads;

// The output thread
std::atomic<bool> outRun;
static std::thread *outThread;

std::mutex _threadLock;
std::mutex _waitLock;

// This static factory initializes the map data for future use
static tmx::messages::J2735MessageFactory factory;

static std::condition_variable cv;

bool IsByteHexEncoded(const char *encoding)
{
	if (!encoding) return false;

	// All must end in hexstring
	static const std::string hexstring("hexstring");

	std::string enc(encoding);
	return (enc.find(hexstring) == enc.length() - hexstring.size());
}


/**
 * The incoming message handler will take the incoming message, construct
 * a routeable message from it, if one does not already exist, then invoke
 * the appropriate handler for that message.  The handlers must have been
 * registed already by the plugin.
 */
void RxThread::doWork(rawIncomingMessage &msg) {
	static std::atomic<bool> warn {false};

	uint16_t currentOverflow = overflow;

	FILE_LOG(logDEBUG2) << "Current overflow value is " << currentOverflow;
	if (currentOverflow > 0 && this->inQueueSize() > currentOverflow)
	{
		if (!warn)
		{
			FILE_LOG(logWARNING) << "Dropping messages due to incoming queue size above overflow size of "
					<< currentOverflow;
			warn = true;
		}

		// We are dropping incoming messages from the front of the queue in order to get to more relevant ones
		return;
	}

	// Warn again the next time the queue gets too full
	warn = false;

	tmx::routeable_message *routeableMsg = nullptr;

	if (msg.message) {
		
		tmx::byte_stream *bytes = nullptr;
		
		IvpMessage *ivpMsg = nullptr;


		switch (msg.type)
		{
		case type_RawBytes:
			bytes = static_cast<tmx::byte_stream *>(msg.message);
			if (bytes) {
				if (IsByteHexEncoded(msg.encoding)) {
					// New factory needed to avoid race conditions
					tmx::messages::J2735MessageFactory myFactory;

					FILE_LOG(logDEBUG4) << this->get_id() << " Decoding from bytes " << *bytes;

					// Bytes are encoded.  First try to convert to a J2735 message
					routeableMsg = myFactory.NewMessage(*bytes);

					if (!routeableMsg) {
						FILE_LOG(logDEBUG4) << "Not a J2735 message: " << myFactory.get_event();

						// Set the bytes directly as unknown type
						routeableMsg = new routeable_message();
						routeableMsg->set_payload_bytes(*bytes);
						if (msg.encoding)
							routeableMsg->set_encoding(msg.encoding);
					}
				} else {
					// Just use a regular string
					std::string str((const char *)bytes->data(), bytes->size());
					routeableMsg = new tmx::routeable_message();

					string encoding(msg.encoding ? msg.encoding : tmx::messages::api::ENCODING_STRING_STRING);
					routeableMsg->set_encoding(encoding);
					if (strncmp("json", encoding.c_str(), 4) == 0) {
						tmx::message jsonMsg;
						jsonMsg.set_contents(str);
						routeableMsg->set_payload(jsonMsg);
					} else {
						routeableMsg->set_payload(str);
						routeableMsg->set_encoding(encoding);
					}
				}

				FILE_LOG(logDEBUG4) << routeableMsg->get_message(); 

				delete bytes;
				msg.message = nullptr;
			}
			break;
		case type_IvpMessage:
			ivpMsg = static_cast<IvpMessage *>(msg.message);
			if (ivpMsg) {
				routeableMsg = new tmx::routeable_message(ivpMsg);

				ivpMsg_destroy(ivpMsg);
				msg.message = nullptr;
			}
			break;
		}

		free(msg.encoding);
		msg.encoding = nullptr;
	}

	// Invoke the handler
	if (routeableMsg) {
		if (msg.timestamp > 0)
			routeableMsg->set_timestamp(msg.timestamp);

		if (msg.mgr)
			msg.mgr->OnMessageReceived(*routeableMsg);

		delete routeableMsg;
	}

	// After other messages are sent out, then push a message to clean up for this thread assignment
	rawOutgoingMessage out;
	out.groupId = msg.groupId;
	out.uniqId = msg.uniqId;
	out.msg = nullptr;
	this->push_out(out);
	cv.notify_one();

	// If items are queued already, use a multiplicative decrease
	// This should be thread-safe since doWork and idle are mutually exclusive
	if (this->inQueueSize() > 1 && usecSleep > 0)
		usecSleep *= sleepDec;

	this_thread::yield();
}

void RxThread::idle() {
	this_thread::yield();

	usleep((uint32_t)usecSleep);

	// If no items are queued, use an additive increase
	// This should be thread-safe since doWork and idle are mutually exclusive
	if (this->inQueueSize() < 1 && usecSleep < MAX_USEC_SLEEP)
		usecSleep += sleepInc;
}

bool available_messages() {
	for (size_t i = 0; i < workerThreads.size(); i++)
		if (workerThreads[i].outQueueSize()) return true;

	return false;
}

/**
 * The outgoing thread will pop the messages coming off of the outgoing
 * queues of each worker thread and broadcast any message that was created.
 */
void RunOutputThread(TmxMessageManager *mgr) {
	outRun = true;

	rawOutgoingMessage outMsg;

	while (run) {
		unique_lock<mutex> lock(_waitLock);
		cv.wait(lock, available_messages);

		// Loop over each thread in the thread group
		for (size_t i = 0; i < workerThreads.size(); i++) {
			if (workerThreads[i].pop(outMsg)) {
				if (outMsg.msg) {
					outMsg.msg->flush();
					if (mgr)
						mgr->OutgoingMessage(*(outMsg.msg), true);
					delete outMsg.msg;
				}

				if (mgr)
					mgr->Cleanup(outMsg.groupId, outMsg.uniqId);
			}
		}
	}
}

TmxMessageManager::TmxMessageManager(std::string name):
		PluginClient(name) {
}

TmxMessageManager::~TmxMessageManager() {
	this->Stop();
}

void TmxMessageManager::Cleanup(tmx::byte_t groupId, tmx::byte_t uniqId) {
	PLOG(logDEBUG4) << "Unassigning " << (int)groupId << ":" << (int)uniqId;
	workerThreads.unassign(groupId, uniqId);
}

void TmxMessageManager::IncomingMessage(const IvpMessage *msg, byte_t groupId, byte_t uniqId, uint64_t timestamp) {
	if (!msg) return;

	// It was tempting to pass  the C++ object pointer to the thread,
	// but a big part of the purpose behind this class is throughput.
	// Constructing the C++ routeable_message object requires too much
	// overhead due to the attribute container.  Therefore, it is best
	// to pass the smaller C structure.

	// Need a copy of the message
	IvpMessage *copy = ivpMsg_copy(const_cast<IvpMessage *>(msg));

	rawIncomingMessage in;
	in.groupId = groupId;
	in.uniqId = uniqId;
	in.timestamp = timestamp;
	in.mgr = this;
	in.message = copy;
	in.type = type_IvpMessage;
	in.encoding = strdup(msg->encoding);

	PLOG(logDEBUG4) << "Assigning " << msg->type << "/" << msg->subtype <<
			" message from " << msg->source << " as " << (int)groupId << ":" << (int)uniqId;
	workerThreads.assign(groupId, uniqId, in);
}

void TmxMessageManager::IncomingMessage(const tmx::routeable_message &msg, byte_t groupId, byte_t uniqId, uint64_t timestamp) {
	// Only want the actual underlying IvpMessage
	this->IncomingMessage(msg.get_message(), groupId, uniqId, timestamp);
}

void TmxMessageManager::IncomingMessage(const tmx::byte_t *bytes, size_t size, const char *encoding, byte_t groupId, byte_t uniqId, uint64_t timestamp) {
	if (!bytes)
		return;

	// Need a copy of the bytes
	byte_stream *copy = new tmx::byte_stream(size);
	memcpy(copy->data(), bytes, size);

	rawIncomingMessage in;
	in.groupId = groupId;
	in.uniqId = uniqId;
	in.timestamp = timestamp;
	in.mgr = this;
	in.message = copy;
	in.type = type_RawBytes;
	in.encoding = encoding ? strdup(encoding) : NULL;

	PLOG(logDEBUG4) << "Assigning message bytes " << *copy << " as " << (int)groupId << ":" << (int)uniqId;
	workerThreads.assign(groupId, uniqId, in);
}

void TmxMessageManager::IncomingMessage(const tmx::byte_stream &bytes, const char *encoding, tmx::byte_t groupId, tmx::byte_t uniqId, uint64_t timestamp) {
	this->IncomingMessage(bytes.data(), bytes.size(), encoding, groupId, uniqId, timestamp);
}

void TmxMessageManager::IncomingMessage(std::string strBytes, const char *encoding, byte_t groupId, byte_t uniqId, uint64_t timestamp) {
	if (encoding) {

		// Extract the bytes as a hexstring
		std::stringstream ss(strBytes);
		byte_stream bytes;
		ss >> bytes;

		this->IncomingMessage(bytes, encoding, groupId, uniqId, timestamp);
	}
}

void TmxMessageManager::OutgoingMessage(const tmx::routeable_message &msg, bool immediate) {
	if (immediate) {
		this->BroadcastMessage(msg);
		return;
	}

	// At worst case, round robin the assignments
	static int lastId = 0;

	int id = workerThreads.this_thread();
	if (id < 0) {
		id = lastId++;

		if (lastId >= workerThreads.size())
			lastId = 0;
	}

	rawOutgoingMessage out;
	out.groupId = 0;
	out.uniqId = 0;
	out.msg = new tmx::routeable_message(msg);
	workerThreads[id].push_out(out);
	cv.notify_one();

	this_thread::yield();
}

void TmxMessageManager::OnMessageReceived(tmx::routeable_message &msg) {
	msg.flush();
	PLOG(logDEBUG4) << "Handling message " << msg;
	return PluginClient::OnMessageReceived(const_cast<IvpMessage *>(msg.get_message()));
}

void TmxMessageManager::OnMessageReceived(IvpMessage *msg) {
	PluginClient::OnMessageReceived(msg);

	if (msg)
		this->IncomingMessage(msg, 0, 0, msg->timestamp);
}

void TmxMessageManager::OnConfigChanged(const char *key, const char *value) {
	if (strcmp(NUMBER_WORKER_THREADS_CFG, key) == 0) {
		size_t n = strtoul(value, nullptr, 0);
		if (n > _numThreads && n < 512) {
			_numThreads = n;

			// Start new threads
			Start();
		}
	} else if (strcmp(ASSIGNMENT_STRATEGY_CFG, key) == 0) {
		workerThreads.set_strategy(value);
	} else if (strcmp(OVERFLOW_CAPACITY_CFG, key) == 0) {
		uint16_t n = strtoul(value, nullptr, 0);
		if (n != overflow)
			overflow = n;
	} else {
		// Not my key
		PluginClient::OnConfigChanged(key, value);
	}
}

void TmxMessageManager::OnStateChange(IvpPluginState state) {
	PluginClient::OnStateChange(state);

	if (state == IvpPluginState_registered) {
		// Initialize the manager config data
		_numThreads = DEFAULT_NUMBER_WORKER_THREADS;
		GetConfigValue(NUMBER_WORKER_THREADS_CFG, _numThreads);

		string s = DEFAULT_ASSIGNENT_STRATEGY;
		GetConfigValue(ASSIGNMENT_STRATEGY_CFG, s);
		workerThreads.set_strategy(s);

		GetConfigValue(OVERFLOW_CAPACITY_CFG, overflow);

		// Start the new threads
		Start();
	}
}

bool TmxMessageManager::IsRunning() {
	lock_guard<mutex> lock(_threadLock);

	for (size_t i = 0; i < workerThreads.size(); i++)
		if (!workerThreads[i].joinable()) return false;

	return run;
}

void TmxMessageManager::Start() {
	size_t n = _numThreads;

	PLOG(logDEBUG) << "Starting " << n << " message manager worker threads";

	lock_guard<mutex> lock(_threadLock);

	// Only support an increase in the number threads
	if (n > workerThreads.size())
		workerThreads.set_size(n);

	if (!outThread)
		outThread = new std::thread(&RunOutputThread, this);
}

void TmxMessageManager::Stop() {
	lock_guard<mutex> lock(_threadLock);

	workerThreads.stop();

	outRun = false;
	if (outThread && outThread->joinable())
		outThread->join();
}

}} // namespace tmx::utils

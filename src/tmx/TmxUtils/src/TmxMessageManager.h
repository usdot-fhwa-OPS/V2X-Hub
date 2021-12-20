/*
 * @file TmxMessageManager.h
 *
 *  Created on: Oct 31, 2017
 *      @author: Gregory M. Baumgardner
 */

#ifndef SRC_TMXMESSAGEMANAGER_H_
#define SRC_TMXMESSAGEMANAGER_H_

#include "PluginClient.h"

#include <atomic>
#include <tmx/messages/byte_stream.hpp>
#include <tmx/messages/routeable_message.hpp>

#define NUMBER_WORKER_THREADS_CFG "MessageManagerThreads"
#define ASSIGNMENT_STRATEGY_CFG "MessageManagerStrategy"
#define OVERFLOW_CAPACITY_CFG "MessageManagerQueueOverflow"

#define DEFAULT_NUMBER_WORKER_THREADS 3
#define DEFAULT_ASSIGNENT_STRATEGY "Random"
#define DEFAULT_OVERFLOW_CAPACITY 0

namespace tmx {
namespace utils {

/**
 * This class provides an optimized solution for injecting a TMX message
 * into the core, or processing incoming messages from the core.
 *
 * This class is not required, since processing the message can be handled by each plugin.
 * However, the intent of this class is to provide a ubiquitous solution on how to handle
 * and process incoming messages for a plugin, and force the processing into a separate thread
 * so overall throughput is optimized.  In other words, the messages can be handled roughly
 * in the order they are received, irrespective of the rate of entry.  Additionally, this class
 * helps any plugin that receives messages externally, say from a radio, be most efficient in
 * publishing that message without effecting the incoming rate.
 *
 * Note that this class heavily relies on threading, and the messages are tagged
 * to a specific thread by a 2-byte identifier.
 */
class TmxMessageManager: public PluginClient {
public:
	explicit TmxMessageManager(std::string name);
	virtual ~TmxMessageManager();

	// Thread control

	/**
	 * Starts the manager thread(s).  This should be called again if the worker thread size changes.
	 */
	void Start();

	/**
	 * Tops the manager thread(s).  This should be called during shutdown, and will automatically
	 * be called when the object is destroyed.
	 */
	void Stop();

	/**
	 * @returns True if all the threads are running.  False otherwise.
	 */
	bool IsRunning();

	// Incoming message handling

	/**
	 * Handle an incoming message as a byte stream.  The purpose of the identifiers is to guarantee that
	 * all active messages from the same source will be assigned to the same thread to ensure correct ordering.
	 * If that is not critical for the application, then assign both identifiers to 0.
	 *
	 * @param bytes - The bytes of the message.  These could be encoded or decoded bytes.
	 * @param size - The number of bytes passed in.
	 * @param encoding - The encoding of the bytes, or null for a non-encoded string
	 * @param groupId - A one-byte group identifier for the source
	 * @param uniqId - A one-byte unique identifier for the source in the group
	 * @param timestamp - The timestamp of the message, if not the current time.
	 */
	void IncomingMessage(const tmx::byte_t *bytes, size_t size, const char *encoding = tmx::messages::api::ENCODING_ASN1_UPER_STRING,
						 tmx::byte_t groupId = 0, tmx::byte_t uniqId = 0, uint64_t timestamp = 0);

	/**
	 * Handle an incoming message as a byte stream.  The purpose of the identifiers is to guarantee that
	 * all active messages from the same source will be assigned to the same thread to ensure correct ordering.
	 * If that is not critical for the application, then assign both identifiers to 0.
	 *
	 * @param bytes - The bytes of the message.  These could be encoded or decoded bytes.
	 * @param encoding - The encoding of the bytes, or null for a non-encoded string
	 * @param groupId - A one-byte group identifier for the source
	 * @param uniqId - A one-byte unique identifier for the source in the group
	 * @param timestamp - The timestamp of the message, if not the current time.
	 */
	void IncomingMessage(const tmx::byte_stream &bytes, const char *encoding = tmx::messages::api::ENCODING_ASN1_UPER_STRING,
						 tmx::byte_t groupId = 0, tmx::byte_t uniqId = 0, uint64_t timestamp = 0);

	/**
	 * Handle an incoming message as a string.  The purpose of the identifiers is to guarantee that
	 * all active messages from the same source will be assigned to the same thread to ensure correct ordering.
	 * If that is not critical for the application, then assign both identifiers to 0.
	 *
	 * @param strBytes - The bytes of the message.  These could be encoded or decoded bytes.
	 * @param encoding - The encoding of the bytes, or null for a non-encoded string
	 * @param groupId - A one-byte group identifier for the source
	 * @param uniqId - A one-byte unique identifier for the source in the group
	 * @param timestamp - The timestamp of the message, if not the current time.
	 */
	void IncomingMessage(std::string strBytes, const char *encoding = tmx::messages::api::ENCODING_ASN1_UPER_STRING,
						 tmx::byte_t groupId = 0, tmx::byte_t uniqId = 0, uint64_t timestamp = 0);

	/**
	 * Handle an incoming message as a IVP message.  The purpose of the identifiers is to guarantee that
	 * all active messages from the same source will be assigned to the same thread to ensure correct ordering.
	 * If that is not critical for the application, then assign both identifiers to 0.
	 *
	 * @param msg - The message to handle
	 * @param groupId - A one-byte group identifier for the source
	 * @param uniqId - A one-byte unique identifier for the source in the group
	 * @param timestamp - The timestamp of the message, if not the current time.
	 */
	void IncomingMessage(const IvpMessage *msg, tmx::byte_t groupId = 0, tmx::byte_t uniqId = 0, uint64_t timestamp = 0);

	/**
	 * Handle an incoming message as a TMX routeable message.  The purpose of the identifiers is to guarantee that
	 * all active messages from the same source will be assigned to the same thread to ensure correct ordering.
	 * If that is not critical for the application, then assign both identifiers to 0.
	 *
	 * @param msg - The message to handle
	 * @param groupId - A one-byte group identifier for the source
	 * @param uniqId - A one-byte unique identifier for the source in the group
	 * @param timestamp - The timestamp of the message, if not the current time.
	 */
	void IncomingMessage(const tmx::routeable_message &msg, tmx::byte_t groupId = 0, tmx::byte_t uniqId = 0, uint64_t timestamp = 0);

	/**
	 * Handle an outgoing message by broadcasting to the TMX core.  If immediate is set, then the broadcast is pushed out
	 * within the current thread.  If not, then the output message is queued.
	 * @param msg - The message to broadcast
	 * @param immediate - True to broadcast immediately, false to assign to the output queue
	 */
	virtual void OutgoingMessage(const tmx::routeable_message &msg, bool immediate = false);

	/**
	 * A duplicate call from the plugin used to invoke the correct handler.  This function is used internally by the worker
	 * threads, and should not directly be called.  However, this function may be overriden to handle any messages that
	 * were not specifically subscribed to or could not be directly decoded.
	 *
	 * @param msg The message to handle
	 */
	virtual void OnMessageReceived(tmx::routeable_message &msg);

	/**
	 * Clean up thread associated resources for this group and id.  By default, this unlinks the source information from the
	 * currently assigned thread.  If the identifiers are meant to be permanent, then this function needs to be overridden to do nothing.
	 * @param groupId - A one-byte group identifier for the source
	 * @param uniqId - A one-byte unique identifier for the source in the group
	 */
	virtual void Cleanup(tmx::byte_t groupId = 0, tmx::byte_t uniqId = 0);

protected:
	/**
	 * Default method for handling config changes.  Only worries about the thread manager configuration.
	 */
	virtual void OnConfigChanged(const char *key, const char *value);

	/**
	 * Default method for the plugin OnMessageReceived that will immediately throw a received message into a worker thread by the
	 * source id and type. This method may still be overridden in the actual plugin to further customize the thread assignments.
	 */
	virtual void OnMessageReceived(IvpMessage *msg);

	/**
	 * Default method for state change for the plugin.  Once the plugin is registered, the worker threads are started.
	 */
	virtual void OnStateChange(IvpPluginState state);

private:
	/**
	 * The number of manager threads for the plugin.
	 */
	std::atomic<size_t> _numThreads {0};
};

}} // namespace tmx::utils


#endif /* SRC_TMXMESSAGEMANAGER_H_ */

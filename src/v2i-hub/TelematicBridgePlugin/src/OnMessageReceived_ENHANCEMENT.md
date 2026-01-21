/**
 * TelematicBridgePlugin OnMessageReceived Enhancement
 * ===================================================
 * 
 * Location: /workspace/src/v2i-hub/TelematicBridgePlugin/src/TelematicBridgePlugin.cpp
 * 
 * Summary of Changes:
 * ===================
 * 
 * The OnMessageReceived method in TelematicBridgePlugin has been enhanced to:
 * 
 * 1. Detect incoming RSUStatusMessage instances
 * 2. Extract RSU health status information from the message payload
 * 3. Create RSUHealthStatusMessage objects from the extracted data
 * 4. Update the TRUHealthStatusTracker with the new RSU status information
 * 
 * Implementation Details:
 * ======================
 * 
 * New Include:
 * - Added: #include <RSUStatusMessage.h>
 *   Purpose: Access RSUStatusMessage type constants for message detection
 * 
 * RSU Status Message Detection:
 * 1. Check message type and subtype against RSUStatusMessage::MessageType and ::MessageSubType
 * 2. If match found, proceed with RSU status processing
 * 3. If not match, continue with existing message handling logic
 * 
 * RSU Status Extraction from JSON:
 * 1. Parse message payload as JSON
 * 2. Extract required fields:
 *    - rsuIpAddress: IP address of the RSU
 *    - rsuSnmpPort: SNMP port number
 *    - event: Optional event description
 * 3. Validate that required fields exist before processing
 * 
 * RSU ID Generation:
 * - Combine IP and port in format: "IP:port"
 * - Example: "192.168.1.100:161"
 * - Used as unique identifier for tracking
 * 
 * Health Status Determination:
 * The health status is determined based on available RSU status fields:
 * - "healthy": If rsuSystemUptime is present and > 0
 * - "operational": If rsuCpuLoad or rsuDiskUtilization fields are present
 * - "unknown": Default if neither condition is met
 * 
 * RSUHealthStatusMessage Creation:
 * - Instantiate ::TelematicBridgePlugin::RSUHealthStatusMessage with:
 *   * RSU IP address
 *   * RSU SNMP port
 *   * Determined health status
 *   * Event description (empty string if not provided)
 * 
 * TRUHealthStatusTracker Update:
 * - Call _truHealthStatusTracker.updateRsuStatus(rsuId, rsuHealthStatus)
 * - This method:
 *   * Thread-safely updates or adds the RSU status
 *   * Searches by rsu_id for existing entries
 *   * Adds new entry if rsu_id not found
 *   * Protected by mutex lock
 * 
 * Error Handling:
 * - JSON parsing errors: Log error message but continue processing
 * - Exception handling: Catch and log any std::exception during processing
 * - Missing required fields: Skip RSU status update if IP or port missing
 * 
 * Logging:
 * - INFO level: "Updated RSU health status for <rsuId> with status: <status>"
 * - ERROR level: JSON parsing errors or exceptions
 * 
 * Flow Diagram:
 * =============
 * 
 * OnMessageReceived(msg)
 *     |
 *     v
 * Is RSUStatusMessage? -----> NO ---> Continue with existing logic
 *     |
 *     YES
 *     v
 * Parse JSON payload
 *     |
 *     v
 * Extract rsuIpAddress & rsuSnmpPort -----> MISSING ---> Log error, skip
 *     |
 *     YES
 *     v
 * Generate rsuId = "IP:port"
 *     |
 *     v
 * Determine health status from fields
 *     |
 *     v
 * Create RSUHealthStatusMessage
 *     |
 *     v
 * _truHealthStatusTracker.updateRsuStatus(rsuId, status)
 *     |
 *     v
 * Log success message
 *     |
 *     v
 * Continue with existing message processing logic
 * 
 * Integration with TRUHealthStatusTracker:
 * ========================================
 * 
 * The updateRsuStatus() method in TRUHealthStatusTracker:
 * 
 * void updateRsuStatus(const std::string &rsuId, const RSUHealthStatusMessage &status)
 * {
 *     std::lock_guard<std::mutex> lock(_statusMutex);
 *     
 *     auto& rsuStatuses = _truHealthStatus.getRsuHealthStatus();
 *     
 *     // Try to find and update existing RSU status
 *     for (auto& rsuStatus : rsuStatuses)
 *     {
 *         if (rsuStatus.rsu_id == rsuId)
 *         {
 *             rsuStatus = status;  // Update found entry
 *             return;
 *         }
 *     }
 *     
 *     // If not found, add new RSU status
 *     _truHealthStatus.addRsuHealthStatus(status);
 * }
 * 
 * Key Features:
 * - Thread-safe with mutex protection
 * - Efficient upsert pattern (update or insert)
 * - Maintains complete TRU health state
 * 
 * JSON Payload Example:
 * =====================
 * 
 * {
 *   "rsuIpAddress": "192.168.1.100",
 *   "rsuSnmpPort": 161,
 *   "event": "operational status update",
 *   "rsuSystemUptime": 86400,
 *   "rsuCpuLoad": 45,
 *   "rsuDiskUtilization": 62,
 *   ...other RSU fields...
 * }
 * 
 * Message Flow:
 * =============
 * 
 * 1. RSUHealthMonitorPlugin collects RSU health data via SNMP
 * 2. Creates RSUStatusMessage with JSON payload containing RSU status
 * 3. Broadcasts RSUStatusMessage to TMX core
 * 4. TelematicBridgePlugin receives message in OnMessageReceived()
 * 5. Extracts RSU ID and health status
 * 6. Creates RSUHealthStatusMessage object
 * 7. Updates TRUHealthStatusTracker with thread-safe updateRsuStatus()
 * 8. TRUHealthStatusTracker maintains aggregated TRU health state
 * 9. Other components retrieve complete TRU status via getSnapshot()
 * 
 * Thread Safety:
 * ==============
 * 
 * Thread 1: OnMessageReceived (receives RSU status updates)
 *     |
 *     +---> updateRsuStatus() ---> acquires _statusMutex lock
 *
 * Thread 2: Other components retrieving TRU status
 *     |
 *     +---> getSnapshot() ---> acquires _statusMutex lock
 * 
 * Both threads are protected from race conditions by the mutex in TRUHealthStatusTracker
 * 
 * Backward Compatibility:
 * =======================
 * 
 * - Non-RSU messages are processed exactly as before
 * - Existing J2735 message handling unchanged
 * - Topic publishing logic unchanged
 * - Only RSUStatusMessage messages trigger the new functionality
 * 
 * Future Enhancements:
 * ====================
 * 
 * Potential improvements:
 * 1. Add timestamp tracking for last status update
 * 2. Implement status change notification mechanism
 * 3. Add health trend analysis (healthy -> degraded -> failed)
 * 4. Create RSU status validation rules
 * 5. Implement automatic failure recovery recommendations
 * 
 */

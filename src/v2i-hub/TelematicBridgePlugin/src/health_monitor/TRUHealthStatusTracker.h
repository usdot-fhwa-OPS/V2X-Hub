#pragma once

#include <mutex>
#include <memory>
#include <vector>
#include "TRUHealthStatusMessage.h"
#include "RSUHealthStatusMessage.h"
#include "UnitHealthStatusMessage.h"

namespace TelematicBridge
{
    /**
     * @class TRUHealthStatusTracker
     * @brief Manages and tracks TRU (Telematics RSU Unit) health status with thread-safe operations
     * 
     * This class provides thread-safe updates to the TRUHealthStatusMessage, including:
     * - Updating individual RSU health status messages
     * - Managing RSU status collections
     * - Updating unit health status
     * - Thread-safe retrieval of the complete TRU health status snapshot
     */
    class TRUHealthStatusTracker
    {
    private:
        mutable std::mutex _statusMutex;                    ///< Mutex for thread-safe access to TRU status
        TRUHealthStatusMessage _truHealthStatus;            ///< Current TRU health status message

    public:
        /**
         * @brief Default constructor
         */
        TRUHealthStatusTracker() = default;

        /**
         * @brief Constructor with initial TRU health status
         * @param initialStatus Initial TRUHealthStatusMessage
         */
        explicit TRUHealthStatusTracker(const TRUHealthStatusMessage &initialStatus)
            : _truHealthStatus(initialStatus) {}

        /**
         * @brief Destructor
         */
        ~TRUHealthStatusTracker() = default;

        /**
         * @brief Update an RSU health status by RSU ID (IP:port)
         * @param rsuId The RSU identifier in format "IP:port"
         * @param status The RSU health status message
         */
        void updateRsuStatus(const std::string &rsuId, const RSUHealthStatusMessage &status)
        {
            std::lock_guard<std::mutex> lock(_statusMutex);
            
            // Get current RSU statuses
            auto rsuStatuses = _truHealthStatus.getRsuHealthStatus();
            
            // Try to find and update existing RSU status
            bool found = false;
            for (auto& rsuStatus : rsuStatuses)
            {
                if (rsuStatus.rsuId == rsuId)
                {
                    rsuStatus = status;
                    found = true;
                    break;
                }
            }
            
            // If not found, add new RSU status
            if (!found)
            {
                rsuStatuses.push_back(status);
            }
            
            // Update the complete list
            _truHealthStatus.setRsuHealthStatus(rsuStatuses);
        }

        /**
         * @brief Update unit health status
         * @param unitStatus The unit health status message
         */
        void updateUnitStatus(const UnitHealthStatusMessage &unitStatus)
        {
            std::lock_guard<std::mutex> lock(_statusMutex);
            _truHealthStatus.setUnitHealthStatus(unitStatus);
        }

        /**
         * @brief Get a thread-safe snapshot of the complete TRU health status
         * @return A copy of the current TRUHealthStatusMessage
         */
        TRUHealthStatusMessage getSnapshot() const
        {
            std::lock_guard<std::mutex> lock(_statusMutex);
            return _truHealthStatus;
        }
    };

} // namespace TelematicBridgePlugin

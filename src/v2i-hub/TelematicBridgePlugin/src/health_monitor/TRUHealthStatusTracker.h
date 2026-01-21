#pragma once

#include <mutex>
#include <memory>
#include <vector>
#include "TRUHealthStatusMessage.h"
#include "RSUHeathStatusMessage.h"
#include "UnitHealthStatusMessage.h"

namespace TelematicBridgePlugin
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
            
            auto& rsuStatuses = _truHealthStatus.getRsuHealthStatus();
            
            // Try to find and update existing RSU status
            for (auto& rsuStatus : rsuStatuses)
            {
                if (rsuStatus.rsu_id == rsuId)
                {
                    rsuStatus = status;
                    return;
                }
            }
            
            // If not found, add new RSU status
            _truHealthStatus.addRsuHealthStatus(status);
        }

        /**
         * @brief Update multiple RSU health statuses
         * @param rsuStatuses Vector of RSU health status messages
         */
        void updateRsuStatuses(const std::vector<RSUHealthStatusMessage> &rsuStatuses)
        {
            std::lock_guard<std::mutex> lock(_statusMutex);
            _truHealthStatus.setRsuHealthStatus(rsuStatuses);
        }

        /**
         * @brief Remove an RSU health status by RSU ID
         * @param rsuId The RSU identifier in format "IP:port"
         * @return True if RSU status was found and removed, false otherwise
         */
        bool removeRsuStatus(const std::string &rsuId)
        {
            std::lock_guard<std::mutex> lock(_statusMutex);
            
            auto& rsuStatuses = _truHealthStatus.getRsuHealthStatus();
            
            for (auto it = rsuStatuses.begin(); it != rsuStatuses.end(); ++it)
            {
                if (it->rsu_id == rsuId)
                {
                    rsuStatuses.erase(it);
                    return true;
                }
            }
            
            return false;
        }

        /**
         * @brief Get an RSU health status by RSU ID
         * @param rsuId The RSU identifier in format "IP:port"
         * @param status Output parameter for the RSU health status
         * @return True if RSU status was found, false otherwise
         */
        bool getRsuStatus(const std::string &rsuId, RSUHealthStatusMessage &status) const
        {
            std::lock_guard<std::mutex> lock(_statusMutex);
            
            const auto& rsuStatuses = _truHealthStatus.getRsuHealthStatus();
            
            for (const auto& rsuStatus : rsuStatuses)
            {
                if (rsuStatus.rsu_id == rsuId)
                {
                    status = rsuStatus;
                    return true;
                }
            }
            
            return false;
        }

        /**
         * @brief Get all RSU health statuses
         * @return Vector of all RSU health status messages
         */
        std::vector<RSUHealthStatusMessage> getAllRsuStatuses() const
        {
            std::lock_guard<std::mutex> lock(_statusMutex);
            return _truHealthStatus.getRsuHealthStatus();
        }

        /**
         * @brief Clear all RSU health statuses
         */
        void clearRsuStatuses()
        {
            std::lock_guard<std::mutex> lock(_statusMutex);
            _truHealthStatus.clearRsuHealthStatus();
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
         * @brief Get the current unit health status
         * @return The unit health status message
         */
        UnitHealthStatusMessage getUnitStatus() const
        {
            std::lock_guard<std::mutex> lock(_statusMutex);
            return _truHealthStatus.getUnitHealthStatus();
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

        /**
         * @brief Replace the entire TRU health status
         * @param truStatus The new TRUHealthStatusMessage
         */
        void setTruStatus(const TRUHealthStatusMessage &truStatus)
        {
            std::lock_guard<std::mutex> lock(_statusMutex);
            _truHealthStatus = truStatus;
        }

        /**
         * @brief Get the number of RSU health statuses
         * @return Number of RSU health status messages being tracked
         */
        size_t getRsuCount() const
        {
            std::lock_guard<std::mutex> lock(_statusMutex);
            return _truHealthStatus.getRsuHealthStatusCount();
        }

        /**
         * @brief Check if any RSU statuses are being tracked
         * @return True if no RSU statuses are present
         */
        bool isEmpty() const
        {
            std::lock_guard<std::mutex> lock(_statusMutex);
            return _truHealthStatus.isRsuHealthStatusEmpty();
        }

        /**
         * @brief Get JSON representation of the current TRU health status
         * @return JSON::Value object representing the tracker state
         */
        Json::Value toJson() const
        {
            std::lock_guard<std::mutex> lock(_statusMutex);
            return _truHealthStatus.toJson();
        }

        /**
         * @brief Get string representation of the tracker state
         * @return String representation
         */
        std::string toString() const
        {
            std::lock_guard<std::mutex> lock(_statusMutex);
            return _truHealthStatus.toString();
        }
    };

} // namespace TelematicBridgePlugin

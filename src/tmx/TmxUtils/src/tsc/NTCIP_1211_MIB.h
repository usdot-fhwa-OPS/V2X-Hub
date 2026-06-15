#pragma once

#include <string>

namespace tmx::utils::tsc::mib::ntcip1211
{
    /**
     * @brief This header file contains a subset of NTCIP 1211 MIB OIDs from https://www.ntcip.org/file/2018/11/NTCIP1211v02A-SE03.docx
     */

    // prgPriorityRequestAbsolute (1211 v02A-SE03f PRS-MIB1 5.1.2.8)
    static const std::string NTCIP1211_PRIORITY_REQUEST_ABSOLUTE_OID = "1.3.6.1.4.1.1206.4.2.11.2.8.0";

    // prgPriorityUpdateAbsolute (1211 v02A-SE03f PRS-MIB1 5.1.2.9)
    static const std::string NTCIP1211_PRIORITY_UPDATE_ABSOLUTE_OID = "1.3.6.1.4.1.1206.4.2.11.2.9.0";

    // prgPriorityCancel (1211 v02A-SE03f PRS-MIB1 5.1.2.5)
    static const std::string NTCIP1211_PRIORITY_CANCEL_OID = "1.3.6.1.4.1.1206.4.2.11.2.5.0";

    // prgPriorityClear (1211 v02A-SE03f PRS-MIB1 5.1.2.6)
    static const std::string NTCIP1211_PRIORITY_CLEAR_OID = "1.3.6.1.4.1.1206.4.2.11.2.6.0";

    // prsServiceRequest (1211 v0224j CO-MIB1 5.2.2.1)
    static const std::string NTCIP1211_PRS_SERVICE_REQUEST_OID = "1.3.6.1.4.1.1206.4.2.11.4.1.0";
}

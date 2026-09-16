#pragma once
#include <vector>
#include <CTI4501ValidationMessage.h>
namespace IntersectionValidation{
 /** @brief Compare two vectors of missing data elements for equality.
     *  @param a First vector of missing data elements.
     *  @param b Second vector of missing data elements.
     *  @return True if the vectors are equal, false otherwise.
     */
    bool compareMissingDataElements(const std::vector<tmx::messages::MissingDataElement> &a,
                            const std::vector<tmx::messages::MissingDataElement> &b);
    /** @brief Compare two CTI 4501 revision count validation messages for and only returns true if they are equal
     * and revision count is 0 accross the board.
     *  @param a First CTI 4501 validation message.
     *  @param b Second CTI 4501 validation message.
     *  @return True if the messages are equal and revision count is 0, false otherwise.
     *  @note This is used to throttle duplicate CTI 4501 revision count validation events when revision count is never incremented.
     */
    bool compareRevisionValidationMessages(tmx::messages::CTI4501ValidationMessage &a,
                            tmx::messages::CTI4501ValidationMessage &b);
}
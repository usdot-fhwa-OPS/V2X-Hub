#include "IntersectionValidationUtils.h"

namespace IntersectionValidation{
     bool compareMissingDataElements( const std::vector<tmx::messages::MissingDataElement> &a,
                            const std::vector<tmx::messages::MissingDataElement> &b)
    {
        if (a.size() != b.size())
        {
            return false;
        }

        for (size_t i = 0; i < a.size(); ++i)
        {
            if (a[i].value != b[i].value)
            {
                return false;
            }
        }

        return true;
    }

    bool compareRevisionValidationMessages( tmx::messages::CTI4501ValidationMessage &a,
                             tmx::messages::CTI4501ValidationMessage &b)
    {
        if (a.get_messageCountA() != b.get_messageCountA())
        {
            return false;
        }

        if (a.get_messageCountB() != b.get_messageCountB())
        {
            return false;
        }
        if (a.get_messageCountA() == 0 && b.get_messageCountA() == 0 && a.get_messageCountB() == 0 && b.get_messageCountB() == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}
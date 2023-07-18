#ifndef INCLUDE_SIMULATED_COVARIANCE_H_
#define INCLUDE_SIMULATED_COVARIANCE_H_

#include <iostream>
#include <tmx/messages/message.hpp>

namespace tmx
{
    namespace messages
    {
        namespace simulation
        {
            // Row-major representation of the 6x6 covariance matrix
            //  # The orientation parameters use a fixed-axis representation.
            //  # In order, the parameters are:
            //  # (x, y, z, rotation about X axis, rotation about Y axis, rotation about Z axis)
            struct Covariance
            {
                double covariance = 0;

                Covariance() {}
                Covariance(double covariance) : covariance(covariance) {}

                static message_tree_type to_tree(Covariance element)
                {
                    message_tree_type treeElement;
                    treeElement.put("Covariance", element.covariance);
                    return treeElement;
                }

                static Covariance from_tree(message_tree_type &treeElement)
                {
                    Covariance element;
                    element.covariance = treeElement.get<double>("Covariance");
                    return element;
                }
            };
        }
    }
}
#endif
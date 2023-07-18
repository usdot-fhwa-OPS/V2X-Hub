#ifndef INCLUDE_SIMULATED_BSMID_H_
#define INCLUDE_SIMULATED_BSMID_H_

#include <iostream>
#include <tmx/messages/message.hpp>

namespace tmx
{
    namespace messages
    {
        namespace simulation
        {

            struct BSMID
            {
                uint8_t BsmId = 0;

                BSMID() {}
                BSMID(std::uint8_t bsmId) : BsmId(bsmId) {}

                static message_tree_type to_tree(BSMID element)
                {
                    message_tree_type treeElement;
                    treeElement.put("BsmId", element.BsmId);
                    return treeElement;
                }

                static BSMID from_tree(message_tree_type &treeElement)
                {
                    BSMID element;
                    element.BsmId = treeElement.get<std::uint8_t>("BsmId");
                    return element;
                }
            };
        }
    }
}
#endif
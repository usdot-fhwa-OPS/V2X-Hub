#pragma once
#include <vector>
#include<RSUConfig.h>

namespace tmx::messages{
    struct RSUConfigList
    {
        RSUConfigList() {}
        RSUConfigList(std::vector<RSUConfig>& rsuConfigs) : rsuConfigs(rsuConfigs) {}

        static message_tree_type to_tree(const RSUConfigList& rsuConfigList){
            message_tree_type tree;
            for (const auto& config: rsuConfigList.rsuConfigs){
                tree.push_back(std::make_pair("",RSUConfig::to_tree(config)));
            }

            return tree;
        }

        static RSUConfigList from_tree(const message_tree_type& tree){
            RSUConfigList rsuConfigList;
            for (const auto& item : tree){
                rsuConfigList.rsuConfigs.push_back(RSUConfig::from_tree(item.second));
            }
            return rsuConfigList;
        }

        std::vector<RSUConfig> rsuConfigs;
    };
}
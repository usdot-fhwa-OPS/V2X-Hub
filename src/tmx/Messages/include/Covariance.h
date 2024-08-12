#pragma once
#include <tmx/messages/message.hpp>
namespace tmx
{
    namespace messages
    {
        typedef struct Covariance{
            double value;             
            Covariance(){};
            Covariance(double value):value(value){};
            static message_tree_type to_tree(const Covariance& cov){
                message_tree_type tree;
                tree.put("",cov.value);
                return tree;
            }
            static Covariance from_tree(const message_tree_type& tree){
                Covariance cov;
                cov.value = tree.get<double>("");
                return cov;
            }
        } Covariance;   
    }
}
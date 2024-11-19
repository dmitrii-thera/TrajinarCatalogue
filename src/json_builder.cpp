// json_builder.cpp
#include "json_builder.h"
#include "json.h"

namespace json {

using namespace std::string_literals;

Builder::Builder() {
    nodes_stack_.emplace_back(&root_);
    
}

Builder::DictValueContext Builder::Key(std::string key) {
    bool cerr_on = true;
    if (cerr_on) { 
        std::cerr << __func__ << ": "s << key << "\n"; 
    }
    if(nodes_stack_.empty()){
         throw std::logic_error("Key is runned for completed object"s);
    }

    auto *node_ptr = nodes_stack_.back();
    if (!node_ptr->IsDict()) {
        throw std::logic_error("Key is outside a dictionary"s);
    }
    auto& map = std::get<Dict>(node_ptr->GetValue());
    std::cerr << "back_node_value Dict add map[key]\n"s;
    nodes_stack_.emplace_back(&map[key]);
    std::cerr << "back_node set to map[key]\n"s;
    std::cerr << "back_node_value nullptr\n"s;
    std::cerr << "nodes_stack_.size "s << nodes_stack_.size() << "\n"s;
    std::cerr << "-------------------------------------------------------------\n"s;

    return BaseContext(*this);
};

Builder::BaseContext Builder::Value(json::Node::Value value) {
    std::cerr << __func__ << ": \n"s;

     if(nodes_stack_.empty()){
         throw std::logic_error("Value is runned for completed object"s);
    }

    auto *node_ptr = nodes_stack_.back();
    if(node_ptr->IsArray()){
        auto& vec= std::get<Array>(node_ptr->GetValue());
        vec.emplace_back(value);
    } else {
      node_ptr->GetValue() = std::move(value);
      nodes_stack_.pop_back();
      std::cerr << "back_node pop_back\n"s;
    }
    std::cerr << "nodes_stack_.size "s << nodes_stack_.size() << "\n"s;
    std::cerr << "-------------------------------------------------------------\n"s;
    return BaseContext(*this);
};

Builder::DictItemContext Builder::StartDict() {
    std::cerr << __func__ << ": \n"s;
     if(nodes_stack_.empty()){
         throw std::logic_error("StartDict is runned for completed object"s);
    }
    auto *node_ptr = nodes_stack_.back();
    if(node_ptr->IsArray()){
        auto& vec= std::get<Array>(node_ptr->GetValue());
        vec.emplace_back(Dict());
        nodes_stack_.push_back(&vec.back());
    }
    else{
    node_ptr->GetValue() = Dict();
    std::cerr << "back_node_value Dict()\n"s;
    }
    std::cerr << "nodes_stack_.size "s << nodes_stack_.size() << "\n"s;
    std::cerr << "-------------------------------------------------------------\n"s;

    return BaseContext(*this);
};

Builder::ArrayItemContext Builder::StartArray() {
    std::cerr << __func__ << ": \n"s;
    if(nodes_stack_.empty()){
         throw std::logic_error("StartArray is runned for completed object"s);
    }
    auto *node_ptr = nodes_stack_.back();
    
    if (node_ptr->IsArray()){
        auto& vec= std::get<Array>(node_ptr->GetValue());
        vec.emplace_back(Array());
        nodes_stack_.push_back(&vec.back());
    } else if(node_ptr->IsNull()) {
        node_ptr->GetValue() = Array();
    }
    std::cerr << "back_node_value StartArray()\n"s;
    std::cerr << "nodes_stack_.size "s << nodes_stack_.size() << "\n"s;
    std::cerr << "-------------------------------------------------------------\n"s;
    return BaseContext(*this);
};

Builder::BaseContext Builder::EndDict() {
    std::cerr << __func__ << ": \n"s;
    if(nodes_stack_.empty()){
         throw std::logic_error("EndDict is runned for completed object"s);
    }
     auto *node_ptr = nodes_stack_.back();
    if (!node_ptr->IsDict()){
         throw std::logic_error("EndDict is runned not for Dict"s);
    }
    nodes_stack_.pop_back();
    
    std::cerr << "back_node pop_back\n"s;
    std::cerr << "nodes_stack_.size "s << nodes_stack_.size() << "\n"s;
    std::cerr << "-------------------------------------------------------------\n"s;
    return BaseContext(*this);
};

Builder::BaseContext Builder::EndArray() {
    std::cerr << __func__ << ": \n"s;
    if(nodes_stack_.empty()){
         throw std::logic_error("EndArray is runned for completed object"s);
    }
    auto *node_ptr = nodes_stack_.back();
    if (!node_ptr->IsArray()){
         throw std::logic_error("EndArray is runned not for Array"s);
    }
    nodes_stack_.pop_back();
    std::cerr << "back_node pop_back\n"s;
    std::cerr << "nodes_stack_.size "s << nodes_stack_.size() << "\n"s;
    std::cerr << "-------------------------------------------------------------\n"s;
    
   return BaseContext(*this);

};

json::Node Builder::Build() {
    if(!nodes_stack_.empty()){
         throw std::logic_error("JSON - object is not complete"s);
    }
    
    return root_;
};

Builder::BaseContext::BaseContext(Builder& builder) : builder_(builder) {}
Node Builder::BaseContext::Build() {
    return builder_.Build();
}

Builder::DictValueContext Builder::BaseContext::Key(std::string key) {
    return builder_.Key(key);
}

Builder::BaseContext Builder::BaseContext::Value(Node::Value value) {
    return builder_.Value(std::move(value));
}

Builder::DictItemContext Builder::BaseContext::StartDict() {
    return builder_.StartDict();
}

Builder::ArrayItemContext Builder::BaseContext::StartArray() {
    return builder_.StartArray();
}

Builder::BaseContext
Builder::BaseContext::EndDict() {
    return builder_.EndDict();
}

Builder::BaseContext
Builder::BaseContext::EndArray() {
    return builder_.EndArray();
}

Builder::DictItemContext Builder::DictValueContext::Value(Node::Value value) {
    return DictItemContext(BaseContext::builder_.Value(std::move(value)));
}

Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value) {
    return ArrayItemContext(BaseContext::builder_.Value(std::move(value)));
 }


}  // namespace json
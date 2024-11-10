#include "json_builder.h"

namespace json {
    
    using namespace std;
    
    KeyDelegate Builder::Key(std::string key) {
        if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict()) {
            auto& d = nodes_stack_.back()->MutableDict();
            d[key] = nullptr;
            nodes_stack_.push_back(&d.at(key));
            return KeyDelegate(*this);
        }
        throw logic_error("dictionary has not been started");
    }
    
    Builder& Builder::Value(Node::Value val) {
        if (nodes_stack_.empty()) {
            nodes_stack_.push_back(&root_);
        }
        if (nodes_stack_.back()->IsNull()) {
            nodes_stack_.back()->MutableValue() = val;
            nodes_stack_.pop_back();
        } else if (nodes_stack_.back()->IsArray()) {
            nodes_stack_.back()->MutableArray().emplace_back().MutableValue() = val;
        } else {
            throw logic_error("value has not been opened");
        }
        return *this;
    }
    
    DictDelegate Builder::StartDict() {
        if (nodes_stack_.empty()) {
            nodes_stack_.push_back(&root_);
        }
        if (nodes_stack_.back()->IsNull()) {
            nodes_stack_.back()->MutableValue() = Dict();
        } else if (nodes_stack_.back()->IsArray()) {
            nodes_stack_.back()->MutableArray().emplace_back(Dict());
            nodes_stack_.push_back(&nodes_stack_.back()->MutableArray().back());
        } else {
            throw logic_error("can't start dict on closed node");
        }
        return DictDelegate(*this);
    }
    
    ArrayDelegate Builder::StartArray() {
        if (nodes_stack_.empty()) {
            nodes_stack_.push_back(&root_);
        }
        if (nodes_stack_.back()->IsNull()) {
            nodes_stack_.back()->MutableValue() = Array();
        } else if (nodes_stack_.back()->IsArray()) {
            nodes_stack_.back()->MutableArray().emplace_back(Array());
            nodes_stack_.push_back(&nodes_stack_.back()->MutableArray().back());
        } else {
            throw logic_error("can't start array on closed node");
        }
        return ArrayDelegate(*this);
    }
    
    Builder& Builder::EndDict() {
        if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict()) {
            nodes_stack_.pop_back();
            return *this;
        }
        throw logic_error("dict is not opened");
    }
    
    Builder& Builder::EndArray() {
        if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
            nodes_stack_.pop_back();
            return *this;
        }
        throw logic_error("array is not opened");
    }
    
    Node Builder::Build() {
        if (nodes_stack_.empty() && !root_.IsNull()) {
            return root_;
        }
        throw logic_error("json is not finished");
    }
    
    KeyDelegate DictDelegate::Key(std::string key) {
        return KeyDelegate(GetBuilder().Key(key));
    }
    
    Builder& DictDelegate::EndDict() {
        return GetBuilder().EndDict();
    }
    
    DictDelegate KeyDelegate::Value(Node::Value val) {
        return DictDelegate(GetBuilder().Value(val));
    }
    
    DictDelegate KeyDelegate::StartDict() {
        return DictDelegate(GetBuilder().StartDict());
    }
    
    ArrayDelegate KeyDelegate::StartArray() {
        return ArrayDelegate(GetBuilder().StartArray());
    }
 
    ArrayDelegate ArrayDelegate::Value(Node::Value val) {
        return ArrayDelegate(GetBuilder().Value(val));
    }
    
    DictDelegate ArrayDelegate::StartDict() {
        return DictDelegate(GetBuilder().StartDict());
    }
    
    ArrayDelegate ArrayDelegate::StartArray() {
        return ArrayDelegate(GetBuilder().StartArray());
    }
    
    Builder& ArrayDelegate::EndArray() {
        return GetBuilder().EndArray();
    }
    
}

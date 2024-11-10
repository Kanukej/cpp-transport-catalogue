#pragma once

#include "json.h"

#include <stdexcept>

namespace json {
    
    class Builder;
    class DictDelegate;
    class ArrayDelegate;
    class KeyDelegate;
    
    class BaseDelegate {
    public:
        BaseDelegate(Builder& builder) : builder_(builder) {
            //
        }
    protected:
        Builder& GetBuilder() {
            return builder_;
        }
    private:
        Builder& builder_;
    };
    
    class DictDelegate : public BaseDelegate {
    public:
        KeyDelegate Key(std::string key);
        Builder& EndDict();
    };
    
    class KeyDelegate : public BaseDelegate {
    public:
        DictDelegate Value(Node::Value val);
        DictDelegate StartDict();
        ArrayDelegate StartArray() ;
    };
    
    class ArrayDelegate : public BaseDelegate {
    public:
        ArrayDelegate Value(Node::Value val);
        DictDelegate StartDict();
        ArrayDelegate StartArray();
        Builder& EndArray();
    };
    
    class Builder {
    public:
        KeyDelegate Key(std::string key);
        Builder& Value(Node::Value val);
        DictDelegate StartDict();
        ArrayDelegate StartArray();
        Builder& EndDict();
        Builder& EndArray();
        Node Build();
        
    private:
        Node root_;
        std::vector<Node*> nodes_stack_;
    };
    
}

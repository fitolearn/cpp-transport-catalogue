#include "json_builder.h"

using namespace std;
namespace json {

    BaseContext::BaseContext(Builder &builder) : builder_(builder) {}

    DictItemContext BaseContext::StartDict() {
        return builder_.StartDict();
    }

    ArrayItemContext BaseContext::StartArray() {
        return builder_.StartArray();
    }

    Builder &BaseContext::EndArray() {
        return builder_.EndArray();
    }

    Builder &BaseContext::EndDict() {
        return builder_.EndDict();
    }

    KeyItemContext BaseContext::Key(std::string key) {
        return builder_.Key(move(key));
    }

    Builder &BaseContext::Value(Node value) {
        return builder_.Value(move(value));
    }

    KeyItemContext::KeyItemContext(Builder &builder) : BaseContext(builder) {}

    KeyItemContext KeyItemContext::Value(Node value) {
        return BaseContext::Value(move(value));
    }

    DictItemContext::DictItemContext(Builder &builder) : BaseContext(builder) {}

    ArrayItemContext::ArrayItemContext(Builder &builder) : BaseContext(builder) {}

    ArrayItemContext ArrayItemContext::Value(Node value) {
        return BaseContext::Value(move(value));
    }

    Builder::Builder() {
        nodes_stack_.push_back(&root_);
    }

    DictItemContext Builder::StartDict() {
        if (nodes_stack_.empty() || (!nodes_stack_.back()->IsNull() && !nodes_stack_.back()->IsArray())) {
            throw std::logic_error("You start dict either in empty object or not in array");
        }
        if (nodes_stack_.back()->IsArray()) {
            const_cast<Array &>(nodes_stack_.back()->AsArray()).push_back(Dict());
            Node* node = &const_cast<Array &>(nodes_stack_.back()->AsArray()).back();
            nodes_stack_.push_back(node);
        } else {
            *nodes_stack_.back() = Dict();
        }
        return *this;
    }

    ArrayItemContext Builder::StartArray() {
        if (nodes_stack_.empty() || (!nodes_stack_.back()->IsNull() && !nodes_stack_.back()->IsArray())) {
            throw std::logic_error("You start dict either in empty object or not in array");
        }
        if (nodes_stack_.back()->IsArray()) {
            const_cast<Array &>(nodes_stack_.back()->AsArray()).push_back(Array());
            Node* node = &const_cast<Array &>(nodes_stack_.back()->AsArray()).back();
            nodes_stack_.push_back(node);
        } else {
            *nodes_stack_.back() = Array();
        }
        return *this;
    }

    Builder &Builder::EndDict() {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsMap()) {
            throw std::logic_error("You end dict either in empty object or not in dict");
        }
        nodes_stack_.erase(nodes_stack_.end() - 1);
        return *this;
    }

    Builder &Builder::EndArray() {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
            throw std::logic_error("You end array either in empty object or not in array");
        }
        nodes_stack_.erase(nodes_stack_.end() - 1);
        return *this;
    }

    KeyItemContext Builder::Key(std::string key) {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsMap()) {
            throw std::logic_error("You try to insert key either in ready object or not in dict");
        }
        nodes_stack_.emplace_back(&const_cast<Dict&>(nodes_stack_.back()->AsMap())[key]);
        return *this;
    }

    Builder &Builder::Value(const Node& value) {
        if (nodes_stack_.empty() || (!nodes_stack_.back()->IsNull() && !nodes_stack_.back()->IsArray())) {
            throw std::logic_error("You try to add value either in ready object or not in array");
        }
        if (nodes_stack_.back()->IsArray()) {
            const_cast<Array &>(nodes_stack_.back()->AsArray()).push_back(value);
        } else {
            *nodes_stack_.back() = value;
            nodes_stack_.erase(nodes_stack_.end() - 1);
        }
        return *this;
    }

    Node Builder::Build() {
        if (!nodes_stack_.empty()) {
            throw std::logic_error("You are trying to build an object before it is ready");
        }
        return root_;
    }
}

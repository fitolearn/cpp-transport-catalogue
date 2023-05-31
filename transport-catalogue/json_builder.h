#pragma once
#include <string>
#include <vector>
#include <memory> // shared_ptr

#include "json.h"

namespace json {

    class BuilderBase {
    public:
        BuilderBase();
        BuilderBase& Key(const std::string&);
        BuilderBase& Value(Node::Value);
        BuilderBase& StartDict();
        BuilderBase& StartArray();
        BuilderBase& EndDict();
        BuilderBase& EndArray();
        Node Build();
    private:
        Node root_;
        std::vector<Node*> context_stack_ {&root_};
        void ThrowIfReady(); // throw std::logic_error("Object already ready"), if stack size = 0
    };

    template <typename T>
    class DictBuilder;
    template <typename T>
    class ArrayBuilder;

    template <typename BuildClass>
    class DictValueBuilder {
    private:
        std::shared_ptr<BuilderBase> builder_;
    public:
        explicit DictValueBuilder(std::shared_ptr<BuilderBase> base): builder_{std::move(base)} {}

        BuildClass Value(Node::Value value) {
            builder_->Value(std::move(value));
            return BuildClass{ builder_ };
        }
        DictBuilder<BuildClass> StartDict() {
            builder_->StartDict();
            return DictBuilder<BuildClass>{ builder_ };
        }
        ArrayBuilder<BuildClass> StartArray() {
            builder_->StartArray();
            return ArrayBuilder<BuildClass>{ builder_ };
        }
    };

    template <typename DictClass>
    class DictBuilder {
    private:
        std::shared_ptr<BuilderBase> builder_;
    public:
        explicit DictBuilder(std::shared_ptr<BuilderBase> base): builder_{std::move(base)} {}

        DictValueBuilder<DictBuilder<DictClass>> Key(std::string key) {
            builder_->Key(std::move(key));
            return DictValueBuilder<DictBuilder<DictClass>>{builder_};
        }
        DictClass EndDict() {
            builder_->EndDict();
            return DictClass{builder_};
        }
    };

    template <typename ArrayClass>
    class ArrayBuilder {
    public:
        explicit ArrayBuilder(std::shared_ptr<BuilderBase> base): builder_{std::move(base)} {}

        ArrayBuilder<ArrayClass>& Value(Node::Value value) {
            builder_->Value(std::move(value));
            return *this;
        }
        DictBuilder<ArrayBuilder<ArrayClass>> StartDict() {
            builder_->StartDict();
            return DictBuilder<ArrayBuilder<ArrayClass>>{builder_};
        }
        ArrayBuilder<ArrayBuilder<ArrayClass>> StartArray() {
            builder_->StartArray();
            return ArrayBuilder<ArrayBuilder<ArrayClass>>{builder_};
        }
        ArrayClass EndArray() {
            builder_->EndArray();
            return ArrayClass{builder_};
        }
    private:
        std::shared_ptr<BuilderBase> builder_;
    };

    class BuilderComplete {
    public:
        BuilderComplete(std::shared_ptr<BuilderBase> base);
        Node Build();
    private:
        std::shared_ptr<BuilderBase> builder_;
    };

    class Builder {
    public:
        Builder();
        BuilderComplete Value(Node::Value);
        DictBuilder<BuilderComplete> StartDict();
        ArrayBuilder<BuilderComplete> StartArray();
    private:
        std::shared_ptr<BuilderBase> builder_;
    };
}//namespace json
#pragma once
#include <string>
#include <vector>
#include <memory> // shared_ptr

#include "json.h"

namespace json {

    class BuilderBase {
    public:
        BuilderBase();
        BuilderBase& Key(std::string);
        BuilderBase& Value(Node::Value);
        BuilderBase& StartDict();
        BuilderBase& StartArray();
        BuilderBase& EndDict();
        BuilderBase& EndArray();
        Node Build();
    private:
        Node root_;
        std::vector<Node*> context_stack_{&root_};
        void ThrowIfReady(); // throw std::logic_error("Object already ready"), if stack size = 0
    };


    template <typename EndReturnClass>
    class DictBuilder;
    template <typename EndReturnClass>
    class ArrayBuilder;

    template <typename ReturnClass>
    class DictValueBuilder {
    public:

        DictValueBuilder(std::shared_ptr<BuilderBase> base): builder_{std::move(base)} {

        }

        ReturnClass Value(Node::Value value) {
            builder_->Value(std::move(value));
            return ReturnClass{builder_};
        }
        DictBuilder<ReturnClass> StartDict() {
            builder_->StartDict();
            return DictBuilder<ReturnClass>{builder_};
        }
        ArrayBuilder<ReturnClass> StartArray() {
            builder_->StartArray();
            return ArrayBuilder<ReturnClass>{builder_};
        }

    private:
        std::shared_ptr<BuilderBase> builder_;
    };

    template <typename EndReturnClass>
    class DictBuilder {
    public:
        DictBuilder(std::shared_ptr<BuilderBase> base): builder_{std::move(base)} {

        }
        DictValueBuilder<DictBuilder<EndReturnClass>> Key(std::string key) {
            builder_->Key(std::move(key));
            return DictValueBuilder<DictBuilder<EndReturnClass>>{builder_};
        }
        EndReturnClass EndDict() {
            builder_->EndDict();
            return EndReturnClass{builder_};
        }
    private:
        std::shared_ptr<BuilderBase> builder_;
    };

    template <typename EndReturnClass>
    class ArrayBuilder {
    public:
        ArrayBuilder(std::shared_ptr<BuilderBase> base): builder_{std::move(base)} {

        }
        ArrayBuilder<EndReturnClass>& Value(Node::Value value) {
            builder_->Value(std::move(value));
            return *this;//ArrayBuilder<EndReturnClass>{std::move(builder_)};
        }
        DictBuilder<ArrayBuilder<EndReturnClass>> StartDict() {
            builder_->StartDict();
            return DictBuilder<ArrayBuilder<EndReturnClass>>{builder_};
        }
        ArrayBuilder<ArrayBuilder<EndReturnClass>> StartArray() {
            builder_->StartArray();
            return ArrayBuilder<ArrayBuilder<EndReturnClass>>{builder_};
        }
        EndReturnClass EndArray() {
            builder_->EndArray();
            return EndReturnClass{builder_};
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
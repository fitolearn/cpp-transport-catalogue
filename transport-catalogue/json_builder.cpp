#include <iostream>
#include "json_builder.h"
namespace json {
    BuilderBase::BuilderBase() {}

    void BuilderBase::ThrowIfReady() {
        if(context_stack_.size() == 0) {
            throw std::logic_error("Object already ready");
        }
    }

    BuilderBase& BuilderBase::Key(std::string key) {
        ThrowIfReady();

        if(context_stack_.back()->IsMap() == false) {
            throw std::logic_error("Call Key for not Dict");
        }
        context_stack_.push_back(&context_stack_.back()->AsMap()[key]);
        return *this;
    }
    BuilderBase& BuilderBase::Value(Node::Value value) {
        ThrowIfReady();

        if(context_stack_.back()->IsNull()) {
            *context_stack_.back() = std::move(value);
            context_stack_.pop_back();
            return *this;
        }
        if(context_stack_.back()->IsArray()) {
            Array& arr = context_stack_.back()->AsArray();
            arr.push_back(Node(std::move(value)));
            return *this;
        }
        throw std::logic_error("Set value for invalid Node");
    }
    BuilderBase& BuilderBase::StartDict() {
        ThrowIfReady();
        if(context_stack_.back()->IsNull()) {
            *context_stack_.back() = Dict{};
            return *this;
        }
        if(context_stack_.back()->IsArray()) {
            context_stack_.back()->AsArray().push_back(Dict{});
            context_stack_.push_back(&(context_stack_.back()->AsArray().back()));
            return *this;
        }
        throw std::logic_error("Start dict for invalid Node");
    }
    BuilderBase& BuilderBase::StartArray() {
        ThrowIfReady();
        if(context_stack_.back()->IsNull()) {
            *context_stack_.back() = Array{};
            return *this;
        }
        if(context_stack_.back()->IsArray()) {
            context_stack_.back()->AsArray().push_back(Array{});
            context_stack_.push_back(&(context_stack_.back()->AsArray().back()));
            return *this;
        }
        throw std::logic_error("Start array for invalid Node");
    }
    BuilderBase& BuilderBase::EndDict() {
        ThrowIfReady();
        if(context_stack_.back()->IsMap()) {
            context_stack_.pop_back();
            return *this;
        }
        throw std::logic_error("End dict for invalid Node");
    }
    BuilderBase& BuilderBase::EndArray() {
        ThrowIfReady();
        if(context_stack_.back()->IsArray()) {
            context_stack_.pop_back();
            return *this;
        }
        throw std::logic_error("End array for invalid Node");
    }
    Node BuilderBase::Build() {
        if(context_stack_.size() != 0) {
            throw std::logic_error("Build for not ready builder");
        }
        return std::move(root_);
    }

    BuilderComplete::BuilderComplete(std::shared_ptr<BuilderBase> base): builder_{std::move(base)} {}

    Node BuilderComplete::Build() {
        return builder_->Build();
    }

    Builder::Builder(): builder_{std::make_shared<BuilderBase>()} {}

    BuilderComplete Builder::Value(Node::Value value) {
        builder_->Value(std::move(value));
        return BuilderComplete{builder_};
    }
    DictBuilder<BuilderComplete> Builder::StartDict() {
        builder_->StartDict();
        return DictBuilder<BuilderComplete>{builder_};
    }
    ArrayBuilder<BuilderComplete> Builder::StartArray() {
        builder_->StartArray();
        return ArrayBuilder<BuilderComplete>{builder_};
    }
}//namespace json
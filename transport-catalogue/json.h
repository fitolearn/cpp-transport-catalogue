#pragma once

#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <variant>
#include "svg.h"

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    Node LoadNumber(std::istream& input);
    Node LoadString(std::istream& is);

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node : public std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict> {
    public:
        using variant::variant;
        using Value = variant;
        [[nodiscard]] const Value& GetValue() const {return *this;}

        [[nodiscard]] bool IsInt() const;
        [[nodiscard]] bool IsDouble() const;
        [[nodiscard]] bool IsPureDouble() const;
        [[nodiscard]] bool IsBool() const;
        [[nodiscard]] bool IsString() const;
        [[nodiscard]] bool IsNull() const;
        [[nodiscard]] bool IsArray() const;
        [[nodiscard]] bool IsMap() const;

        [[nodiscard]] int AsInt() const;
        [[nodiscard]] bool AsBool() const;
        [[nodiscard]] double AsDouble() const;
        [[nodiscard]] const std::string& AsString() const;
        [[nodiscard]] const Array& AsArray() const;
        [[nodiscard]] const Dict& AsMap() const;
        Array& AsArray();
    };

    class Document {
    public:
        explicit Document(Node root);
        const Node& GetRoot() const;
        bool operator==(const Document& other) const;
        bool operator!=(const Document& other) const;
    private:
        Node root_;
    };

    Document Load(std::istream& input);
    template <typename Value>
    void PrintValue(const Value& value, svg::RenderContext context) {
        std::ostream& out = context.out;
        out << value;
    }
    void PrintValue(const std::string& str, svg::RenderContext context);
    void PrintValue(std::nullptr_t, svg::RenderContext context);
    void PrintValue(bool value, svg::RenderContext context);
    void PrintValue(const Array& arr, svg::RenderContext context);
    void PrintValue(const Dict& dict, svg::RenderContext context);
    void PrintNode(const Node& node, svg::RenderContext context);
}  // namespace json
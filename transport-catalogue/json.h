#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <sstream>
#include <cassert>
#include <chrono>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    using json_variant = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;

    class Node: private json_variant {
    public:
        using Array = std::vector<Node>;
        using Dict = std::map<std::string, Node>;

        using json_variant::variant;
        using Value = variant;
        Node();
        Node(size_t size);
        Node(Value value);

        bool IsInt() const;
        bool IsDouble() const;// Возвращает true, если в Node хранится int либо double
        bool IsPureDouble() const;// Возвращает true, если в Node хранится double и только double
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        Array& AsArray();
        const Dict& AsMap() const;
        Dict& AsMap();

        bool operator==(const Node& other) const;
        bool operator!=(const Node& other) const;
        bool operator==(const std::string& str) const;

        std::ostream& Print(std::ostream& os) const;
        std::string Print() const;
    };

    class Document {
    public:
        explicit Document(Node root);
        [[nodiscard]] const Node& GetRoot() const;

        bool operator==(const Document& other) const;
        bool operator!=(const Document& other) const;
    private:
        Node root_;
    };

    Document Load(std::istream& input);
    void Print(const Document& doc, std::ostream& output);
    Document LoadJSON(const std::string& s);
    std::string Print(const Node& node);
}  // namespace json
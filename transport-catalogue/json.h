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
    using json_variant = std::variant<int,double,std::string,bool,Array,Dict,std::nullptr_t>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node: json_variant {
    public:
        using Array = std::vector<Node>;
        using Dict = std::map<std::string, Node>;
        using json_variant::variant;
        Node();
        Node(size_t size);

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
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
        const Dict& AsMap() const;

        bool operator==(const Node& other) const;
        bool operator!=(const Node& other) const;
        bool operator==(const std::string& str) const;

        std::ostream& Print(std::ostream& os) const;
        std::string Print() const;
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
    void Print(const Document& doc, std::ostream& output);
    Document LoadJSON(const std::string& s); // Unused
    std::string Print(const Node& node); // Unused
}  // namespace json
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
        explicit Node (size_t size);

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
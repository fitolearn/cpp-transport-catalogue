#include "json.h"
#include <exception>
#include <algorithm>

using namespace std;
using namespace std::literals;

namespace json {

    bool Node::IsInt() const {
        return std::holds_alternative<int>(*this);
    }

    bool Node::IsDouble() const {
        return IsPureDouble() || IsInt();
    }

    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(*this);
    }

    bool Node::IsBool() const {
        return std::holds_alternative<bool>(*this);
    }

    bool Node::IsString() const {
        return std::holds_alternative<std::string>(*this);
    }

    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(*this);
    }

    bool Node::IsArray() const {
        return std::holds_alternative<Array>(*this);
    }

    bool Node::IsMap() const {
        return std::holds_alternative<Dict>(*this);
    }

    const Array& Node::AsArray() const {
        if (! IsArray()) {
            throw std::logic_error("Node value is not array.");
        }
        return std::get<Array>(*this);
    }

    Array& Node::AsArray() {
        if (! IsArray()) {
            throw std::logic_error("Node value is not array.");
        }
        return std::get<Array>(*this);
    }

    const Dict& Node::AsMap() const {
        if (!IsMap()) {
            throw std::logic_error("Node value is not map.");
        }
        return std::get<Dict>(*this);
    }

    int Node::AsInt() const {
        if (! IsInt()) {
            throw std::logic_error("Node value is not int.");
        }
        return std::get<int>(*this);
    }

    const string& Node::AsString() const {
        if (! IsString()) {
            throw std::logic_error("Node value is not string.");
        }
        return std::get<std::string>(*this);
    }

    bool Node::AsBool() const {
        if (! IsBool()) {
            throw std::logic_error("Node value is not bool.");
        }
        return std::get<bool>(*this);
    }

    double Node::AsDouble() const {
        if (!IsDouble()) {
            throw std::logic_error("Node value is not double and not int.");
        }
        if ( IsInt() ) {
            int res = std::get<int>(*this);
            return static_cast<double>(res);
        } else {
            return std::get<double>(*this);
        }
    }

    Node LoadNode(istream& input);
    Node LoadArray(istream& input) {
        Array result;
        bool first = true;
        char c;
        for (; input >> c && c != ']';) {
            if(first) {
                if (c == ',') {
                    throw ParsingError("Array separator invalid: "s + c);
                }
                first = false;
                input.putback(c);
            } else if (c != ',') {
                throw ParsingError("Array separator invalid: "s + c);
            } result.push_back(LoadNode(input));
        }
        if(c != ']') {
            throw ParsingError("Array ] not found"s);
        } return Node(move(result));
    }

    Node LoadString(istream& input) {
        string line;
        char c;
        for(c = input.get(); input && c != '\"'; c = input.get()) {
            if(c == '\\') {
                c = input.get();
                switch(c) {
                    case 'n': line.push_back('\n'); break;
                    case 'r': line.push_back('\r'); break;
                    case 't': line.push_back('\t'); break;
                    default: line.push_back(c);
                }
            } else {
                line.push_back(c);
            }
        }
        if(c != '\"') {
            throw ParsingError("String is not closed"s);
        }
        return Node(std::move(line));
    }

    Node LoadDict(istream& input) {
        Dict result;
        bool first = true;
        char c;
        for (; input >> c && c != '}';) {
            if(first) {
                if (c == ',') {
                    throw ParsingError("Dict error format for first key \" != "s + c);
                }
                first = false;
            } else if (c != ',') {
                throw ParsingError("Dict error format , != "s + c);
            } else {
                input >> c;
            }
            if(c != '\"') {
                throw ParsingError("Dict error format \" != "s + c);
            }
            string key = LoadString(input).AsString();
            if(!(input >> c)) {
                throw ParsingError("Dict } not found"s);
            }
            if(c != ':') {
                throw ParsingError("Dict error format : != "s + c);
            }
            result.insert({std::move(key), LoadNode(input)});
        }
        if(c != '}') {
            throw ParsingError("Dict } not found"s);
        }
        return Node(std::move(result));
    }

    Node LoadNumber(istream& input) {
        std::string digit;
        char temp;
        temp = input.peek();
        if(temp != '-' && !isdigit(temp)) {
            throw ParsingError("Digit begin invalid: "s + temp);
        }
        digit.push_back(input.get());
        if(temp == '-') {
            temp = input.peek();
            if(!isdigit(temp)) {
                throw ParsingError("First char is not digit: "s + temp);
            }
            digit.push_back(input.get());
        }
        while(isdigit(input.peek())) {
            input >> temp;
            digit.push_back(temp);
        }
        temp = input.peek();
        if(temp != '.' && temp != 'e' && temp != 'E') {
            return Node(stoi(digit));
        }
        if(temp == '.') {
            digit.push_back(input.get());

            temp = input.peek();
            if(!isdigit(temp)) {
                throw ParsingError("Not digit after dot: "s + temp);
            }
            digit.push_back(input.get());

            while(isdigit(input.peek())) {
                input >> temp;
                digit.push_back(temp);
            }
        }
        temp = input.peek();
        if(temp != 'e' && temp != 'E') {
            return Node(stod(digit));
        }
        digit.push_back(input.get());
        temp = input.peek();
        if(temp != '+' && temp != '-' && !isdigit(temp)) {
            throw ParsingError("Not +/- after e: "s + temp);
        }
        digit.push_back(input.get());
        if(!isdigit(temp)) {
            temp = input.peek();
            if(!isdigit(temp)) {
                throw ParsingError("Not digit after e+/-: "s + temp);
            }
            digit.push_back(input.get());
        }
        while(isdigit(input.peek())) {
            input >> temp;
            digit.push_back(temp);
        }
        return Node(stod(digit));
    }

    Node LoadBool(istream& input) {
        std::string line;
        int c = input.get();
        if(c == 't' && input.get() == 'r' && input.get() == 'u' && input.get() == 'e') {
            return Node(true);
        } else if(c == 'f' && input.get() == 'a' && input.get() == 'l' && input.get() == 's' && input.get() == 'e') {
            return Node(false);
        }
        throw ParsingError("Invalid bool");
    }

    Node LoadNull(istream& input) {
        if(input.get() == 'u' && input.get() == 'l' && input.get() == 'l') {
            return Node(nullptr);
        } throw ParsingError("null invalid"s);
    }

    Node LoadNode(istream& input) {
        char c;
        while(input >> c && (iscntrl(c) || isspace(c)));
        if (c == '[') {
            return LoadArray(input);
        } else if (c == '{') {
            return LoadDict(input);
        } else if (c == '"') {
            return LoadString(input);
        } else if (c == 'n') {
            return LoadNull(input);
        } else if (c == 't' || c == 'f') {
            input.putback(c);
            return LoadBool(input);
        } else {
            input.putback(c);
            return LoadNumber(input);
        }
    }

    Document::Document(Node root)
            : root_(std::move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    bool Document::operator==(const Document& other) const {
        return root_ == other.root_;
    }

    bool Document::operator!=(const Document& other) const {
        return root_ != other.root_;
    }

    Document Load(istream& input) {
        return Document{LoadNode(input)};
    }

    void PrintValue(std::nullptr_t, svg::RenderContext context) {
        ostream& out = context.out;
        out << "null"sv;
    }

    void PrintValue(const string &str, svg::RenderContext context) {
        std::ostream& out = context.out;
        out << "\""sv;
        for (char c : str) {
            switch (c) {
                case '\"':
                    out << "\\\""sv;
                    break;
                case '\r':
                    out << "\\r"sv;
                    break;
                case '\n':
                    out << "\\n"sv;
                    break;
                case '\t':
                    out << "\t"sv;
                    break;
                case '\\':
                    out << "\\\\"sv;
                    break;
                default:
                    out << c;
            }
        } out << "\""sv;
    }

    void PrintValue(bool value, svg::RenderContext context) {
        std::ostream& out = context.out;
        if (value) {
            out << "true"sv;
        } else {
            out << "false"sv;
        }
    }

    void PrintValue(const Array &arr, svg::RenderContext context) {
        std::ostream& out = context.out;
        out << "["sv << endl;
        auto indent = context.Indented();
        for (auto iter = arr.begin(); iter != arr.end(); ++iter) {
            indent.RenderIndent();
            PrintNode(*iter, indent);
            if (std::next(iter) != arr.end()) {
                out << ","sv;
            }
            out << endl;
        }
        context.RenderIndent();
        out << "]"sv;
    }

    void PrintValue(const Dict &dict, svg::RenderContext context) {
        std::ostream& out = context.out;
        auto indent = context.Indented();
        out << "{"sv << endl;
        for (auto iter = dict.begin(); iter != dict.end(); ++iter) {
            indent.RenderIndent();
            out << "\""sv << iter->first << "\": "sv;
            PrintNode(iter->second, indent);
            if (std::next(iter) != dict.end() ) {
                out << ","sv;
            }
            out << endl;
        }
        context.RenderIndent();
        out << "}";
    }

    void PrintNode(const Node &node, svg::RenderContext context) {
        std::visit([&context](const auto& value){
            PrintValue(value, context);
        }, node.GetValue());
    }
}  // namespace json
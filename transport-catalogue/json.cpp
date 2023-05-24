#include "json.h"

using namespace std;

namespace json {

    bool Node::IsInt() const {
        return holds_alternative<int>(*this);
    }
    bool Node::IsDouble() const {
        return IsInt() || IsPureDouble();
    }
    bool Node::IsPureDouble() const {
        return holds_alternative<double>(*this);
    }
    bool Node::IsBool() const {
        return holds_alternative<bool>(*this);
    }
    bool Node::IsString() const {
        return holds_alternative<std::string>(*this);
    }
    bool Node::IsNull() const {
        return holds_alternative<std::nullptr_t>(*this);
    }
    bool Node::IsArray() const {
        return holds_alternative<Array>(*this);
    }
    bool Node::IsMap() const {
        return holds_alternative<Dict>(*this);
    }

    int Node::AsInt() const {
        if(IsInt()) {
            return std::get<int>(*this);
        }
        throw std::logic_error("Node is not int");
    }
    bool Node::AsBool() const {
        if(IsBool()) {
            return std::get<bool>(*this);
        }
        throw std::logic_error("Node is not bool");
    }
    double Node::AsDouble() const {
        if(IsPureDouble()) {
            return std::get<double>(*this);
        }
        if(IsInt()) {
            return std::get<int>(*this);
        }
        throw std::logic_error("Node is not double or int");
    }
    const std::string& Node::AsString() const {
        if(IsString()) {
            return std::get<std::string>(*this);
        }
        throw std::logic_error("Node is not string");
    }
    const Array& Node::AsArray() const {
        if(IsArray()) {
            return std::get<Array>(*this);
        }
        throw std::logic_error("Node is not Array");
    }
    const Dict& Node::AsMap() const {
        if(IsMap()) {
            return std::get<Dict>(*this);
        }
        throw std::logic_error("Node is not map");
    }

    bool Node::operator==(const Node& other) const {
        return this->AsString() == other.AsString();
    }

    bool Node::operator!=(const Node& other) const {
        return !(*this == other);
    }

    bool Node::operator==(const std::string& str) const {
        return this->AsString() == str;
    }

    Node::Node(): Node{nullptr} {
    }
    Node::Node(size_t size): Node(static_cast<int>(size)) {

    }

    Node LoadNode(istream& input);
    Node LoadArray(istream& input) {
        Array result;
        bool is_first = true;
        char c;
        for (; input >> c && c != ']';) {
            if(is_first) {
                if (c == ',') {
                    throw ParsingError("Array separator invalid: ,"s);
                }
                is_first = false;
                input.putback(c);
            } else if (c != ',') {
                throw ParsingError("Array separator invalid: "s + c);
            }
            result.push_back(LoadNode(input));
        }

        if(c != ']') {
            throw ParsingError("Array ] not found"s);
        }

        return Node{std::move(result)};
    }

    Node LoadDigit(istream& input) {
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

    Node LoadString(istream& input) {
        string line;
        char c;
        for(c = input.get(); input && c != '\"'; c = input.get()) {
            if(c == '\\') {
                c = input.get();
                switch(c) {
                    case 'n':
                        line.push_back('\n');
                        break;
                    case 'r':
                        line.push_back('\r');
                        break;
                    case 't':
                        line.push_back('\t');
                        break;
                    default:
                        line.push_back(c);
                }
            } else {
                line.push_back(c);
            }
        }
        if(c != '\"') {
            throw ParsingError("String is not closed"s);
        }
        return Node(move(line));
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
            result.insert({move(key), LoadNode(input)});
        }
        if(c != '}') {
            throw ParsingError("Dict } not found"s);
        }
        return Node(move(result));
    }

    Node LoadNull(istream& input) {

        if(input.get() == 'u' && input.get() == 'l' && input.get() == 'l') {
            return Node(nullptr);
        }

        throw ParsingError("null invalid"s);
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
            return LoadDigit(input);
        }
    }

    Document::Document(Node root)
            : root_(std::move(root)) {
    }
    bool Document::operator==(const Document& other) const {
        return root_ == other.root_;
    }

    bool Document::operator!=(const Document& other) const {
        return root_ != other.root_;
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{LoadNode(input)};
    }

    struct NodePrinter {
        std::ostream& os;

        void operator()(int value) {
            os << value;
        }
        void operator()(double value) {
            os << value;
        }
        void operator()(const std::string& str) {
            os << '\"';
            for(char c: str) {
                switch(c) {
                    case '\n': os << "\\n"; break;
                    case '\r': os << "\\r"; break;
                    case '\"': os << "\\\""; break;
                    case '\\': os << "\\\\"; break;
                    default: os << c; break;
                }
            } os << '\"';
        }
        void operator()(bool b) {
            os << (b ? "true" : "false");
        }
        void operator()(const Array& array) {
            os << "[";
            bool second = false;
            for(auto& node: array) {
                if(second) {
                    os << ",";
                } else {
                    second = true;
                }
                node.Print(os);
            } os << "]";
        }
        void operator()(const Dict& dict) {
            os << "{ ";
            bool first = true;
            for(auto& [key, node]: dict) {
                if(first) {
                    first = false;
                } else {
                    os << ", ";
                }
                Node(key).Print(os);
                os << ": ";
                node.Print(os);
            } os << " }";
        }
        void operator()(std::nullptr_t) {
            os << "null";
        }
    };

    std::ostream& Node::Print(std::ostream& os) const {
        visit(NodePrinter{os}, *dynamic_cast<const json_variant*>(this));
        return os;
    }

    std::string Node::Print() const {
        std::stringstream ret;
        Print(ret);
        return ret.str();
    }

    void Print(const Document& doc, std::ostream& output) {
        doc.GetRoot().Print(output);
    }

    Document LoadJSON(const std::string& s) {
        std::istringstream strm(s);
        return Load(strm);
    }

    std::string Print(const Node& node) {
        std::ostringstream out;
        Print(Document{ node }, out);
        return out.str();
    }
}  // namespace json
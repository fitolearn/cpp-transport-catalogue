#include <iostream>
#include <fstream>
#include "json_reader.h"
#include "transport_catalogue.h"
//#define JSON_TEST1
/*
#define JSON_TEST2
#define JSON_TEST3
#define JSON_TEST4
#define JSON_TEST5
#define JSON_TEST6
*/

using namespace std;
using namespace transport;
using namespace json;
using namespace json_reader;
using namespace std::literals;


void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    TransportCatalogue tc;
    JsonReader jr(tc);
    //jr.InputStatReader(std::cin);
    //jr.OutputStatReader(std::cout);

    if (argc != 2) {
        PrintUsage();
        return 1;
    }
    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        jr.InputStatReader(std::cin);
        SerializationSettings settings = jr.GetSerializationSettings();
        std::ofstream out(settings.file_name, std::ios::binary | std::ios::out);
        tc_serialize::TransportCatalogue ser_tc;

        tc.SaveToSerializeTransportCat(ser_tc);
        jr.SaveToReader(ser_tc);
        ser_tc.SerializeToOstream(&out);

    } else if (mode == "process_requests"sv) {
        jr.ReadJson(std::cin);
        tc_serialize::TransportCatalogue ser_tc;
        SerializationSettings settings = jr.GetSerializationSettings();
        std::ifstream input(settings.file_name, std::ios::binary | std::ios::in);
        ser_tc.ParseFromIstream(&input);

        tc.RestoreFromSerializedTransportCat(ser_tc);
        jr.RestoreFrom(ser_tc);
        jr.OutputStatReader(std::cout);
    } else {
        PrintUsage();
        return 1;
    }
}

/*    // json test
#ifdef JSON_TEST1
    json::Print(
            json::Document{
                    json::Builder{}
                            .StartDict()
                            .Key("key1"s).Value(123)
                            .Key("key2"s).Value("value2"s)
                            .Key("key3"s).StartArray()
                            .Value(456)
                            .StartDict().EndDict()
                            .StartDict()
                            .Key(""s)
                            .Value(nullptr)
                            .EndDict()
                            .Value(""s)
                            .EndArray()
                            .EndDict()
                            .Build()
            },
            cout
    );
    cout << endl;

    json::Print(
            json::Document{
                    json::Builder{}
                            .Value("just a string"s)
                            .Build()
            },
            cout
    );
    cout << endl;

#endif // JSON_TEST1

#ifdef JSON_TEST2
    json::Builder{}.StartDict().Key("1"s).Key(""s);  // правило 1
#endif // JSON_TEST2

#ifdef JSON_TEST3
    json::Builder{}.StartDict().Key("1"s).Value(1).Value(1);  // правило 2
#endif // JSON_TEST3

#ifdef JSON_TEST4
    json::Builder{}.StartDict().Build();  // правило 3
#endif // JSON_TEST4

#ifdef JSON_TEST5
    json::Builder{}.StartArray().Value(1).Value(2).EndDict();  // правило 5
#endif // JSON_TEST5

#ifdef JSON_TEST6
    json::Builder{}.StartArray().EndDict();  // правило 4
    json::Builder{}.StartArray().Key("1"s);  // правило 4
#endif // JSON_TEST6

}
*/
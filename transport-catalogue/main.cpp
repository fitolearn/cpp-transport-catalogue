#include <iostream>
#include <fstream>
#include "json_reader.h"
#include "json_builder.h"

#define JSON_TEST1
//#define JSON_TEST2
//#define JSON_TEST3
//#define JSON_TEST4
//#define JSON_TEST5
//#define JSON_TEST6


using namespace std;
using namespace transport;
using namespace json;

int main()
{
    // automatic test 11 sprint

    TransportCatalogue tc;
    fstream ist("s10_final_opentest_1.json");
    fstream out("output.txt");
    transport::reader::json::InputStatReader{}(ist, out, tc);

    // manual test 11 sprint

    //TransportCatalogue tc;
    //transport::reader::json::InputStatReader{}(cin, cout, tc);

    // json test
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

#endif // Json Test1

#ifdef JSON_TEST2
    json::Builder{}.StartDict().Key("1"s).Key(""s);  // правило 1
#endif // JsonTest2

#ifdef JSON_TEST3
    json::Builder{}.StartDict().Key("1"s).Value(1).Value(1);  // правило 2
#endif // JsonTest3

#ifdef JSON_TEST4
    json::Builder{}.StartDict().Build();  // правило 3
#endif // JsonTest4

#ifdef JSON_TEST5
    json::Builder{}.StartArray().Value(1).Value(2).EndDict();  // правило 5
#endif // JsonTest5

#ifdef JSON_TEST6
    json::Builder{}.StartArray().EndDict();  // правило 4
    json::Builder{}.StartArray().Key("1"s);  // правило 4
#endif // JsonTest6

}
#include <iostream>
#include <fstream>
#include "json_reader.h"
#include "json_builder.h"

using namespace std;
using namespace transport;

int main()
{
    // automatic test

    //TransportCatalogue tc;
    //fstream ist("s10_final_opentest_1.json");
    //fstream out("output.txt");
    //transport::reader::json::InputStatReader{}(ist, out, tc);

    // manual test

    //TransportCatalogue tc;
    //transport::reader::json::InputStatReader{}(cin, cout, tc);

    // json test
    /*json::Print(
            json::Document{
                    // Форматирование не имеет формального значения:
                    // это просто цепочка вызовов методов
                    json::Builder{}
                            .StartDict()
                            .Key("key1"s).Value(123)
                            .Key("key2"s).Value("value2"s)
                            .Key("key3"s).StartArray()
                            .Value(456)
                            .StartDict().EndDict()
                            .StartDict()
                            .Key(""s).Value(nullptr)
                            .EndDict()
                            .Value(""s)
                            .EndArray()
                            .EndDict()
                            .Build()
            },
            cout
    ); */
}
#include <iostream>
#include <fstream>

#include "json_reader.h"

using namespace std;
using namespace transport;

int main()
{

    TransportCatalogue tc;
    fstream ist("s10_final_opentest_1.json");
    fstream out("output.txt");
    transport::json::InputStatReader{}(ist, out, tc);
    //TransportCatalogue tc;
    //transport::json::InputStatReader{}(cin, cout, tc);
}
#pragma once
#include "transport_catalogue.h"
#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <sstream>

namespace transport_catalogue
{
    class InputReader
    {
    public:
        explicit InputReader(TransportCatalogue& transport_catalogue);

        void ReadRequests(std::istream& input_stream);

    private:
        TransportCatalogue& transport_catalogue_;
        std::vector<Stop> request_stops_;
        std::vector<std::vector<std::pair<std::string, int>>> distances_to_stops_;
        std::vector<std::tuple<std::string, bool, std::vector<std::string>>> request_buses_;

        void ReadStop(std::string& request_stop);
        void ReadBus(std::string& request_bus);

        std::vector<std::string> SplitBusStops(const std::string& str, const std::string& delimiter);
    };
}//namespace transport_catalogue

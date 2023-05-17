#include "input_reader.h"
#include "transport_catalogue.h"

using namespace std::literals;

namespace transport_catalogue
{
    InputReader::InputReader(TransportCatalogue& transport_catalogue)
        : transport_catalogue_(transport_catalogue)
    {
    }

    void InputReader::ReadRequests(std::istream& input_stream)
    {
        int num_request;
        input_stream >> num_request;
        input_stream.ignore();

        for (int i = 0; i < num_request; ++i)
        {
            std::string input;
            std::getline(input_stream, input);
            auto pos = input.find(' ');

            std::string request_type = input.substr(0, pos);
            std::string request_string = input.substr(pos + 1);

            if (request_type == "Stop"s)
            {
                ReadStop(request_string);
            }
            if (request_type == "Bus"s)
            {
                ReadBus(request_string);
            }
        }

        for (const Stop& stop : request_stops_)
        {
            transport_catalogue_.AddStop(stop);
        }

        int i = 0;
        for (const Stop & stop : request_stops_)
        {
            transport_catalogue_.SetDistance(stop.name, distances_to_stops_[i]);
            ++i;
        }

        for (const auto& bus : request_buses_)
        {
            transport_catalogue_.AddBus(bus);
        }
    }

    void InputReader::ReadStop(std::string& request_stop)
    {
        Stop stop;
        std::vector<std::pair<std::string, int>> distances_to_stops;

        std::size_t colon_pos = request_stop.find(": "s);
        stop.name = request_stop.substr(0, colon_pos);
        request_stop.erase(0, colon_pos + 2);

        std::size_t comma_pos = request_stop.find(", "s);
        stop.coordinates.latitude = std::stod(request_stop.substr(0, comma_pos));
        request_stop.erase(0, comma_pos + 2);

        comma_pos = request_stop.find(", ");
        stop.coordinates.longitude = std::stod(request_stop.substr(0, comma_pos));

        while (comma_pos != std::string::npos)
        {
            request_stop.erase(0, comma_pos + 2);
            std::size_t m_to_pos = request_stop.find("m to "s);
            int distance = std::stoi(request_stop.substr(0, m_to_pos));
            request_stop.erase(0, m_to_pos + 5);
            comma_pos = request_stop.find(", ");
            std::string stop_name = request_stop.substr(0, comma_pos);
            distances_to_stops.push_back(std::make_pair(stop_name, distance));
        }

        request_stops_.push_back(stop);
        distances_to_stops_.push_back(distances_to_stops);
    }

    void InputReader::ReadBus(std::string& request_bus)
    {
        std::string bus_name;
        bool is_circling;
        std::vector<std::string> bus_stops;
        std::string delimiter = ": "s;
        bus_name = request_bus.substr(0, request_bus.find(delimiter));
        request_bus.erase(0, request_bus.find(delimiter) + delimiter.length());
        auto roundtrip = request_bus.find(">"s);
        if (roundtrip != std::string::npos)
        {
            is_circling = true;
            bus_stops = SplitBusStops(request_bus, " > "s);
        }
        else
        {
            is_circling = false;
            bus_stops = SplitBusStops(request_bus, " - "s);
        }
        request_buses_.push_back(std::make_tuple(bus_name, is_circling, bus_stops));
    }

    std::vector<std::string> InputReader::SplitBusStops(const std::string& request, const std::string& delimiter)
    {
        std::vector<std::string> bus_stops;
        std::string stop;
        size_t start_pos = 0, end_pos;
        while ((end_pos = request.find(delimiter, start_pos)) != std::string::npos)
        {
            stop = request.substr(start_pos, end_pos - start_pos);
            start_pos = end_pos + delimiter.length();
            bus_stops.push_back(stop);
        }
        bus_stops.push_back(request.substr(start_pos));
        return bus_stops;
    }
}//namespace transport_catalogue

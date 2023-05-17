#include "stat_reader.h"

using namespace std::literals;

namespace transport_catalogue
{
	StatReader::StatReader(const TransportCatalogue& transport_catalogue)
		: transport_catalogue_(transport_catalogue)
	{
	}

    void StatReader::PrintRequests(std::vector<RequestData> requests_queue_, std::ostream& output_stream) const {
        for (const auto& request : requests_queue_)
        {
            if (request.type == "Bus"s)
            {
                auto bus_info = transport_catalogue_.GetBusInfo(request.query);
                if (bus_info.has_value())
                {
                    output_stream << "Bus "
                                  << bus_info.value().name << ": "s
                                  << bus_info.value().amount_stops << " stops on route, "s
                                  << bus_info.value().uniq_stops << " unique stops, "s
                                  << bus_info.value().route_length << " route length, "s
                                  << bus_info.value().curvature << " curvature"s << std::endl;
                }
                else
                {
                    output_stream << "Bus " << request.query << ": "s << "not found" << std::endl;
                }
            }

            if (request.type == "Stop"s)
            {
                const Stop* stop = transport_catalogue_.FindStop(request.query);
                if (stop == nullptr)
                {
                    output_stream << "Stop " << request.query << ": not found" << std::endl;
                }
                else
                {
                    auto stop_buses = transport_catalogue_.GetStopBuses(request.query);
                    if (stop_buses.has_value())
                    {
                        output_stream << "Stop " << request.query << ": "s << "buses ";
                        for (const std::string_view busname : stop_buses.value())
                        {
                            output_stream << std::string(busname) << " ";
                        }
                        output_stream << std::endl;
                    }
                    else
                    {
                        output_stream << "Stop " << request.query << ": no buses" << std::endl;
                    }
                }
            }
        }
    }
	void StatReader::OutputRequests(std::istream& input_stream, std::ostream& output_stream = std::cout)
	{
		int num_request;
		input_stream >> num_request;
		input_stream.ignore();

		for (int i = 0; i < num_request; ++i)
		{
			RequestData request;
			std::string input;
			std::getline(input_stream, input);
			auto pos = input.find(' ');

			request.type = input.substr(0, pos);
			request.query = input.substr(pos + 1);
			requests_queue_.push_back(request);
		}
        PrintRequests(requests_queue_, output_stream);
	}
}//namespace transport_catalogue

#pragma once
#include "transport_catalogue.h"
#include <iostream>
#include <set>
#include <deque>
#include <string>
#include <string_view>
#include <fstream>

namespace transport_catalogue
{
	struct RequestData
	{
		std::string type;
		std::string query;
	};

	class StatReader
	{
	public:
		explicit StatReader(const TransportCatalogue& transport_catalogue);

		void OutputRequests(std::istream& input_stream);

	private:
		const TransportCatalogue& transport_catalogue_;
		std::vector<RequestData> requests_queue_;
	};
}//namespace transport_catalogue

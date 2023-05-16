#pragma once
#include "geo.h"
#include <vector>
#include <deque>
#include <unordered_map>
#include <string>
#include <string_view>
#include <unordered_set>
#include <iostream>
#include <set>
#include <optional>

namespace transport_catalogue
{
	struct Stop
	{
		std::string name;
		geo::Coordinates coordinates;
	};

	struct Bus
	{
		std::string name;
		std::vector<std::string_view> stops;
		bool Is_Circling;
	};

	struct BusInfo
	{
		std::string name;
		int amount_stops;
		int uniq_stops;
		double route_length;
		double curvature;
	};

	namespace detail
	{
		struct PairStopHasher
		{
			std::size_t operator()(const std::pair<const  Stop*, const  Stop*>& pair_stop) const;
		};
	}// namespace detail

	class TransportCatalogue
	{
	public:
		void AddStop(Stop stop);

		const Stop* FindStop(const std::string_view stop) const;

		std::optional<std::set<std::string_view>> GetStopBuses(std::string_view stop) const;

		void SetDistance(const std::string& stop, std::vector<std::pair<std::string, int>>& distances_to_stops);

		int GetDistance(const Stop* stop_ptr, const Stop* anoter_stop_ptr) const;

		void AddBus(const std::tuple<std::string, bool, std::vector<std::string>>& bus_input);

		const Bus* FindBus(std::string_view bus) const;

		std::optional<BusInfo> GetBusInfo(std::string_view bus) const;

	private:
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, Bus*> busname_to_bus_;

		std::deque<Stop> stops_;
		std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
		std::unordered_map<std::string_view, std::set<std::string_view>> stopname_to_busnames_;

		std::unordered_map<std::pair<const Stop*, const Stop*>, int, detail::PairStopHasher> stops_distances_;
	};
}//namespace transport_catalogue

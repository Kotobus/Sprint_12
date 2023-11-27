#pragma once
#include "domain.h"
#include <deque>

#include <string>
#include <string_view>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <iostream>
#include <algorithm>
#include <set>
#include <utility>
#include <sstream>
#include <optional>
namespace tr_cat {
	

	class TransportCatalogue {
	public:

		//Constructors
		TransportCatalogue() = default;
		~TransportCatalogue() = default;

		//Methods

		void AddStop(const Stop& stop);
		Stop* FindStop(std::string_view stop) const;
		void AddBus(const Bus& bus);
		Bus* FindBus(std::string_view name) const;
		std::optional <BusInfo> GetBusInfo(std::string_view name) const;
		StopInfo GetStopInfo(std::string_view name) const;
		void SetDistance(std::string_view from, std::string_view to, uint32_t distance);
		//const void Test() const {};
		//Возвращает расстояние между остановками,если не находит A->B то возвращает B->A. 
		// Гарантировалось,что одно из расстояний точно есть.
		 uint32_t GetDistance(std::string_view from, std::string_view to) const ;
		 std::set<std::string_view> GetOrderedBusses() const;
		 std::set<std::string_view> GetOrderedStops() const;
		 

	private: //Fields
		std::deque<Stop> stops_;
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
		std::unordered_map<std::string_view, Bus*> busname_to_bus_;
		std::unordered_map<std::string_view, std::set<std::string_view>> stop_to_buses_;
		std::unordered_map<std::pair<Stop*, Stop*>, uint32_t, StopPointersHasher> distance_between_stops_;
		std::set<std::string_view> ordered_bus_list_;
		std::set<std::string_view> ordered_stop_list_;
	};
}
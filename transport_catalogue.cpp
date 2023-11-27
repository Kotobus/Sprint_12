#include "transport_catalogue.h"
#include "domain.h"
#include "geo.h"
#include <numeric>
#include <vector>
#include <iostream>
#include <iterator>
#include <optional>

namespace tr_cat {

	void TransportCatalogue::AddStop(const Stop& stop) {

		if (stopname_to_stop_.count(stop.name) > 0) return; //если такая есть выходим

		auto Iterator_to_stop_inside_deque = stops_.insert(stops_.begin(), stop);

		Stop* ptr_to_stop = Iterator_to_stop_inside_deque.operator->();

		//Прописыввем дистанции между остановками

		stopname_to_stop_[ptr_to_stop->name] = ptr_to_stop;
		ordered_stop_list_.insert(ptr_to_stop->name);
		if (!stop_to_buses_.count(ptr_to_stop->name)) {
			stop_to_buses_[ptr_to_stop->name] = {};
		}
	}

	Stop* TransportCatalogue::FindStop(std::string_view stop) const {
		auto it = stopname_to_stop_.find(stop);
		if (it != stopname_to_stop_.end()) {
			return stopname_to_stop_.at(stop);
		}
		else {
			return nullptr;
		}
	}

	void TransportCatalogue::AddBus(const Bus& bus) {
		auto Iterator_to_bus_inside_deque = buses_.insert(buses_.begin(), std::move(bus));
		Bus* ptr_to_bus = Iterator_to_bus_inside_deque.operator->();
		busname_to_bus_[ptr_to_bus->name] = ptr_to_bus;

		for_each(ptr_to_bus->stops.begin(), ptr_to_bus->stops.end(), [this, ptr_to_bus](Stop* stop) {
			stop_to_buses_[stop->name].insert(ptr_to_bus->name);
			});
		ordered_bus_list_.insert(ptr_to_bus->name);
	}

	Bus* TransportCatalogue::FindBus(std::string_view bus) const{
		
		if (busname_to_bus_.count(bus)>0) {
			return busname_to_bus_.at(bus);
		}
		else {
			return nullptr;
		}
	}

	 std::optional<BusInfo> TransportCatalogue::GetBusInfo(std::string_view name) const {
		//BusInfo bus_info;
		Bus* bus = FindBus(name);
		if (bus != nullptr) {

			size_t total_stops_on_route = !bus->is_roundtrip ? bus->stops.size() *2 - 1: bus->stops.size();
			double geographical_route_length = 0;
			int real_route_length = 0;
			auto bus_stops = bus->stops;
			std::set<Stop*>set;
			for (auto stop : bus->stops) {
				set.insert(stop);
			}

			double curvature;
			for (size_t i = 0; i < bus_stops.size() - 1; ++i) {
				geographical_route_length += geo::ComputeDistance({ bus_stops[i]->latitude, bus_stops[i]->longitude }, { bus_stops[i + 1]->latitude, bus_stops[i + 1]->longitude });
			}
			for (size_t i = 0; i < bus_stops.size() - 1; ++i) {
				real_route_length += GetDistance(bus_stops[i]->name, bus_stops[i + 1]->name);
			}
			
		
			if (!bus->is_roundtrip) {
				for (size_t i = bus_stops.size() - 1; i >0 ; --i) {
					geographical_route_length += geo::ComputeDistance({ bus_stops[i]->latitude, bus_stops[i]->longitude }, { bus_stops[i - 1]->latitude, bus_stops[i - 1]->longitude });
				}
				for (size_t i = bus_stops.size() - 1; i > 0; --i){
					real_route_length += GetDistance(bus_stops[i]->name, bus_stops[i -1]->name);
				}
			
			}
			
			curvature = real_route_length / geographical_route_length;
			
		const BusInfo bus_info = {
				bus->name,
				set.size(),
				total_stops_on_route,
				real_route_length,
				curvature
			};
			return bus_info;
		}
		else {
			return std::nullopt;
		}
	}

	 StopInfo TransportCatalogue::GetStopInfo(std::string_view name) const{
		StopInfo stop;
		stop.name = name;
		if (FindStop(name) == nullptr) {
			stop.found = false;
		}
		else {
			stop.found = true;
			stop.buses = stop_to_buses_.at(name);
		}
		return stop;
	}

	void TransportCatalogue::SetDistance(std::string_view from, std::string_view to, uint32_t distance)  {
		if (stopname_to_stop_.count(from) > 0 && stopname_to_stop_.count(to) > 0) {
			//std::pair<Stop*, Stop*> stops_pair = std::make_pair(stopname_to_stop_[from], stopname_to_stop_[to]);
			distance_between_stops_[{stopname_to_stop_[from], stopname_to_stop_[to]}] = distance;
		}
		else {
			throw std::invalid_argument("Distance add error: some of stops have not been found stops are:"+std::string(from) +" "+std::string(to));
		}
	}

	  uint32_t TransportCatalogue::GetDistance(std::string_view from, std::string_view to) const {
		 if (stopname_to_stop_.count(from) > 0 && stopname_to_stop_.count(to) > 0 && distance_between_stops_.count({ stopname_to_stop_.at(from), stopname_to_stop_.at(to) }) > 0) {

			 
			 return (distance_between_stops_.at({ stopname_to_stop_.at(from), stopname_to_stop_.at(to) }));
		 }
		 else {
			
			 return distance_between_stops_.at({ stopname_to_stop_.at(to), stopname_to_stop_.at(from) });
		 };
	}
	  std::set<std::string_view> TransportCatalogue::GetOrderedBusses() const
	  {
		  return ordered_bus_list_;
	  }
	  std::set<std::string_view> TransportCatalogue::GetOrderedStops() const
	  {
		  return ordered_stop_list_;
	  }
}// namespace transport_catalogue

#pragma once
#include "geo.h"
#include <string>
#include <string_view>
#include <vector>
#include <set>
#include <unordered_set>

struct BusInfo {
	std::string_view name = "";
	size_t unique_stops = 0;
	size_t total_stops = 0;
	int route_length = 0;
	double curvature = 0.0;

};

struct StopInfo {
	std::string_view name = "";
	std::set<std::string_view> buses{};
	bool found;
};

struct Stop {
	std::string name = "";
	double longitude = 0;
	double latitude = 0;
	geo::Coordinates GetCoordinates() const {
		return geo::Coordinates{ longitude,latitude };
	}
	size_t Hash() const {
		return size_t(longitude * 31 + latitude * 31 * 17);
	}
	bool operator==(const Stop& other) const {
		return name == other.name && latitude == other.latitude && longitude == other.longitude;
	}
};
struct Bus {
	std::string name = "";
	std::vector<Stop*> stops{};
	size_t unique_stops = 0;
	bool is_roundtrip;
};
struct StopPointersHasher {

	size_t operator ()(const std::pair<Stop*, Stop*>& stop_pointers) const {

		return size_t(stop_pointers.first) * 31 + size_t(stop_pointers.second) * 17 + 37;

	}
private:
	static const size_t secret_number{ 37 };
};

enum Type {
	ReqStop,
	ReqBus,
	ReqMap
};

class Stat {
public:
	Stat() = default;
	Stat(int id, Type type, std::string name) :id_(id), type_(type),name_(name)
	{
	
	}
	Stat(int id, Type type) :id_(id), type_(type)
	{

	}
	
	
	int GetId() {
		return id_;
	}
	Type GetType() {
		return type_;
	}
	std::string GetName() {
		return name_;
	}
private:
	int id_ = 0;
	Type type_;
	std::string name_ = "";
};

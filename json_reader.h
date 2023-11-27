#pragma once
#include "json.h"
#include "domain.h"
#include "transport_catalogue.h"
#include <memory>
#include "request_handler.h"
#include "map_renderer.h"
#include "json_builder.h"
//using namespace tr_cat;

namespace json {

	class JsonReader{

public:
	struct DistanceBetweenStops {
		std::string from = "";
		std::string to = "";
		int distance = 0;
	};

	JsonReader(std::istream& i_stream = std::cin, std::ostream& o_stream = std::cout) :is_(i_stream), os_(o_stream) 
	{
		HandleJson();
	};
	// Handlers
	std::optional<BusInfo> GetBusInfo(const std::string_view& bus_name) const;
	const std::optional<StopInfo> GetStopInfo(std::string_view name) const;
	std::optional<Dict> SingleBusHandler(std::string_view bus_name);
	Dict SingleBusHandlerBuilder(std::optional<BusInfo>& bus, int request_id);
	Dict SingleBusHandler(BusInfo bus);
	std::optional<Dict> SingleStopHandler(std::string_view stop_name);
	Dict SingleStopHandlerBuilder(std::optional<StopInfo>& stop, int request_id);
	Dict SingleStopHandler(StopInfo stop);
	void HandleRequests(render::Renderer renderer);
	//
	//Formers
	void FillTransportCatalogue(Array& arr);
	void StatsFormer(Array& arr);
	tr_cat::TransportCatalogue& GetCatalog();
	void AddStop(const json::Dict& stop, std::vector<DistanceBetweenStops>& dbs_vec);
	void AddBus(const json::Dict& bus);
	void HandleJson();
	std::unique_ptr<std::vector<Stat>> GetStats();
	//
	//Renders
	void SetRenderSettings(Dict render_settings_as_dict);
	std::string RenderJson(Dict render_settings_as_dict);
	svg::Color ColorPicker(Node color);
private:
	
	std::istream& is_;
	std::ostream& os_;
	std::vector<Stat> req_vec{}; 
	tr_cat::TransportCatalogue tr_cat_{}; 
	render::RendererSettings render_settings_{};

};
}


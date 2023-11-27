#include "json_reader.h"
#include "log_duration.h"
#include <cmath>
#include <iomanip>
#include <memory>
#include <sstream>

namespace json {
	using namespace std::literals;
	//Formers
	void JsonReader::AddStop(const json::Dict& stop, std::vector<DistanceBetweenStops>& dbs_vec)
	{
		
		Stop new_stop;
		new_stop.name = stop.at("name"s).AsString();
		//double lat_temp1 = stop.at("latitude"s).AsDouble();
		
		new_stop.latitude = stop.at("latitude"s).AsDouble();
		new_stop.longitude = stop.at("longitude"s).AsDouble();
		
		/*double lat_temp = round(stop.at("latitude"s).AsDouble() * 1000000) / 1000000;
		new_stop.latitude = lat_temp;
		double lon_temp = round(stop.at("longitude"s).AsDouble() * 1000000) / 1000000;
		new_stop.longitude = lon_temp;*/
		
		for (const auto& from_to : stop.at("road_distances"s).AsMap()) { //�������� ������ � ���������� ����� �����������
			DistanceBetweenStops dbs;
			dbs.from = new_stop.name;
			dbs.to = from_to.first;
			dbs.distance = from_to.second.AsInt();
			dbs_vec.push_back(dbs);
		}
		tr_cat_.AddStop(new_stop);
	}

	void JsonReader::AddBus(const json::Dict& bus)
	{
		Bus new_bus;
		new_bus.name = bus.at("name").AsString();
		new_bus.is_roundtrip = bus.at("is_roundtrip").AsBool();

		for (auto& stop : bus.at("stops").AsArray()) {
			auto stop_ptr = tr_cat_.FindStop(stop.AsString());
			if (stop_ptr) {
				new_bus.stops.push_back(stop_ptr);
			}
			else { throw std::invalid_argument("Json Parsing error:Bus add error there is no such stop in tr_cat database"); }
		}
		tr_cat_.AddBus(new_bus);
	}

	void JsonReader::FillTransportCatalogue(Array& arr)
	{
		std::vector<Bus> busses;
		std::vector<DistanceBetweenStops> dbs_vec;
		std::vector<json::Dict> bus_nodes;
		for (const auto& item : arr) {
			if (item.AsMap().at("type"s) == "Stop"s) {
				json::JsonReader::AddStop(item.AsMap(), dbs_vec);
			}
			else if (item.AsMap().at("type"s) == "Bus"s) { 
				bus_nodes.push_back(item.AsMap());
			}
		}
		
		std::for_each(dbs_vec.begin(), dbs_vec.end(), [this](DistanceBetweenStops& elem) {
			tr_cat_.SetDistance(elem.from, elem.to, elem.distance);
			});
		
		for (const auto& item : bus_nodes) {
			json::JsonReader::AddBus(item); //JsonReader Method not TransportCatalogue
		}
	}

	void JsonReader::StatsFormer(Array& arr)
	{
		req_vec.clear();
		req_vec.reserve(req_vec.size() + arr.size());
		for (const auto& item : arr) {
			int id = item.AsMap().at("id").AsInt();
			Type type;
			if (item.AsMap().at("type").AsString() == "Map"){
				type = ReqMap;
				Stat request(id, type);
				req_vec.push_back(request);
				continue;
			}
			type = item.AsMap().at("type").AsString() == "Stop" ? ReqStop  : ReqBus;
			std::string name = item.AsMap().at("name").AsString();
			if (name != "") {
				Stat request(id, type, name);
				req_vec.push_back(request);
			}
			else {
				throw std::invalid_argument("Json Parsing error: request add error, some of requied fields are empty");
			}

		}
	}

	tr_cat::TransportCatalogue& JsonReader::GetCatalog()
	{
		return tr_cat_;
	}

	std::unique_ptr<std::vector<Stat>> JsonReader::GetStats()
	{
		return std::make_unique <std::vector<Stat>>(req_vec);
	}
	//
	//Handlers
	void JsonReader::HandleJson() {

		Node document_json = Load(is_).GetRoot();
		Array base_context;
		Array request_context;
		Dict render_context;
	 
			base_context = document_json.AsMap().at("base_requests").AsArray();
			request_context = document_json.AsMap().at("stat_requests").AsArray();
			render_context = document_json.AsMap().at("render_settings").AsMap();
			JsonReader::FillTransportCatalogue(base_context);
			JsonReader::StatsFormer(request_context);
			JsonReader::SetRenderSettings(render_context); 
			render::Renderer renderer(tr_cat_, render_settings_);
			HandleRequests(renderer);
			}
	void JsonReader::HandleRequests(render::Renderer renderer)
	{
		Array result;
		for (auto request : req_vec) {
			Dict result_item;
			
			std::string str = request.GetName();
			if (request.GetType() == Type::ReqStop) {
				
					auto stop = GetStopInfo(str);
						
					if (stop.has_value()) {

						result_item = SingleStopHandlerBuilder(stop, request.GetId()); //Builder Method

						result_item["request_id"] = request.GetId();

					}
					else {
						Builder builder;
						result_item = builder.StartDict()
							.Key("error_message").Value(json::Node(std::string("not found")))
							.Key("request_id").Value(request.GetId())
							.EndDict()
							.Build()
							.AsMap();
					}
					result.emplace_back(result_item);
					
				
				
				continue;
			}

			if (request.GetType() == Type::ReqBus) {

				std:: optional<BusInfo> bus = GetBusInfo(str);
				
				if (bus.has_value()) {
					result_item = SingleBusHandlerBuilder(bus, request.GetId());  //Builder Method
					result_item["request_id"] = request.GetId();
				}
				else {
					Builder builder;
					
					result_item = builder.StartDict()
						.Key("error_message").Value(json::Node(std::string("not found")))
						.Key("request_id").Value(request.GetId())
						.EndDict()
						.Build()
						.AsMap();
					
				}
				result.emplace_back(result_item);
				continue;
			}
			if (request.GetType() == Type::ReqMap) {
				std::string str = renderer.Render();
				Builder builder;
				result_item = builder.StartDict().Key("map").Value(std::move(str)).Key("request_id").Value(request.GetId()).EndDict().Build().AsMap();//Builder Method
				result.emplace_back(result_item);
				continue;
			}

			throw std::invalid_argument("Handle Error : Unknown Type of request");

		}
		Document doc(std::move(result));
		Print(doc, os_); 
	}
	std::optional<BusInfo> JsonReader::GetBusInfo(const std::string_view& bus_name) const
	{
		auto bus_info = tr_cat_.GetBusInfo(bus_name);
		if (bus_info.has_value()) {
			return bus_info;
		}
		else
		{
			return std::nullopt;
		}
	}
	const std::optional<StopInfo> JsonReader::GetStopInfo(std::string_view name) const
	{
		const StopInfo stop = tr_cat_.GetStopInfo(name);
		if (stop.found) {
			return stop;
		}
		else {
			return std::nullopt;
		}
	}

	//BUilderMethods
	Dict JsonReader::SingleStopHandlerBuilder(std::optional<StopInfo>& stop, int request_id)
	{
			Builder builder;
			builder.StartDict().Key("buses");
			Array arr;
			for (auto stp : stop->buses) {
				arr.emplace_back(Node(std::string(std::move(stp.data()))));
			}
			builder.Value(std::move(arr));
			builder.Key("request_id").Value(request_id);
			builder.EndDict();
			return builder.Build().AsMap();
	}
	
	Dict JsonReader::SingleBusHandlerBuilder(std::optional<BusInfo>& bus, int request_id)
	{
		Builder builder;
		builder.StartDict();
		builder.Key("curvature").Value(Node(bus->curvature));
		builder.Key("route_length").Value(Node(bus->route_length));
		builder.Key("stop_count").Value(Node(static_cast<int>(bus->total_stops)));
		builder.Key("unique_stop_count").Value(Node(static_cast<int>(bus->unique_stops)));
		builder.Key("request_id").Value(request_id);
		builder.EndDict();
		return builder.Build().AsMap();
	}

	//Basic Handle Methods

	std::optional<json::Dict> JsonReader::SingleBusHandler(std::string_view bus_name)
	{
		auto bus = GetBusInfo(bus_name);
		if (bus.has_value()) {
			json::Dict result;
			result["curvature"] = Node(bus->curvature);
			result["route_length"] = Node(bus->route_length);
			result["stop_count"] = Node(static_cast<int>(bus->total_stops));
			result["unique_stop_count"] = Node(static_cast<int>(bus->unique_stops));
			return result;
		}
		else {
			return std::nullopt;
		}
	}
	
	json::Dict JsonReader::SingleBusHandler(BusInfo bus)
	{
		json::Dict result;
		result["curvature"] = Node(bus.curvature);
		result["route_length"] = Node(bus.route_length);
		result["stop_count"] = Node(static_cast<int>(bus.total_stops));
		result["unique_stop_count"] = Node(static_cast<int>(bus.unique_stops));
		return result;
	}
	std::optional<json::Dict> JsonReader::SingleStopHandler(std::string_view stop_name)
	{
		using namespace json;
		auto stop = GetStopInfo(stop_name);
		if (stop.has_value()) {
			json::Dict result;
			Array arr;
			for (auto stp : stop->buses) {
				arr.push_back(Node(std::string(stp.data())));
			}
			result["buses"] = arr;
			return result;
		}
		else {
			return std::nullopt;
		}
	}
	
	json::Dict JsonReader::SingleStopHandler(StopInfo stop)
	{
		json::Dict result;
		Array arr;
		for (auto stp : stop.buses) {
			arr.push_back(Node(std::string(stp.data())));
		}
		result["buses"] = arr;
		return result;
	}
	//
	//Renderers
	svg::Color JsonReader::ColorPicker(Node unhandled_color)
	{
		if (unhandled_color.IsString()) {
			return  unhandled_color.AsString();
		}
		auto& color_array = unhandled_color.AsArray();
		
			
			if (color_array.size() == 3) {
			
				return	 svg::Rgb{
					 static_cast<uint8_t>(color_array[0].AsInt()),
					 static_cast<uint8_t>(color_array[1].AsInt()),
					 static_cast<uint8_t>(color_array[2].AsInt()) 
				} ;
			}
			else if (color_array.size() == 4) {
				return
					svg::Rgba{ 
					 static_cast<uint8_t>(color_array[0].AsInt()),
					 static_cast<uint8_t>(color_array[1].AsInt()),
					 static_cast<uint8_t>(color_array[2].AsInt()),
					 color_array[3].AsDouble() };
			};

		
		return svg::Color{ svg::NoneColor };
	}
	void JsonReader::SetRenderSettings(Dict render_context)
	{
		
		render::RendererSettings render_settings;
		
		render_settings.SetScreen(
			{
				render_context.at("width").AsDouble(),
				render_context.at("height").AsDouble(),
				render_context.at("padding").AsDouble()

			});

		Array bus_offset = render_context.at("bus_label_offset").AsArray();

		render_settings.SetBusSettings(
			{ render_context.at("bus_label_font_size").AsInt() },
			{ bus_offset[0].AsDouble(),bus_offset[1].AsDouble() });
		Array stop_offset = render_context.at("stop_label_offset").AsArray();

		render_settings.SetStopSettings(
			{ render_context.at("stop_label_font_size").AsInt() },
			{ stop_offset[0].AsDouble(),stop_offset[1].AsDouble() },
			render_context.at("stop_radius").AsDouble()
		);
		
		render_settings.SetUnderLayer(
			{ColorPicker(render_context.at("underlayer_color")),
			render_context.at("underlayer_width").AsDouble()
			});
		
		render_settings.SetLineWidth(render_context.at("line_width").AsDouble());
		
		
		std::vector<svg::Color> colors;
		{
		
			for (Node color : render_context.at("color_palette").AsArray()) {
				colors.emplace_back(ColorPicker(color));
			}
			render_settings.SetColorPallete(colors);
		}
		render_settings_ = render_settings;
	}

	std::string JsonReader::RenderJson(Dict render_settings_as_dict)
	{
		SetRenderSettings(render_settings_as_dict);
		render::Renderer renderer(tr_cat_, render_settings_);
		return renderer.Render();
		
	}

} //namespace json
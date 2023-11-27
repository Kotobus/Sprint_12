#include "map_renderer.h"
#include <algorithm>

namespace render {

	void RendererSettings::SetScreen(Screen screen) {
		screen_ = screen;
	}

	void RendererSettings::SetBusSettings(int bus_label_font_size, svg::Point offset) {
		bus_settings_.font_size_ = bus_label_font_size;
		bus_settings_.offset_ = offset;
	}


	void RendererSettings::SetStopSettings(int stop_label_font_size, svg::Point offset, double stop_radius) {
		if (stop_label_font_size <= 0) {
			throw std::invalid_argument("Font size is inacceptable");
		}
		if (stop_radius <= 0) {
			throw std::invalid_argument("Stop radius is inacceptable");
		}
		stop_settings_.font_size_ = stop_label_font_size;
		stop_settings_.offset_ = offset;
		stop_settings_.stop_radius_ = stop_radius;
	}

	void RendererSettings::SetUnderLayer(UnderLayer under_layer) {
		underlayer_properties_ = under_layer;
	}
	void RendererSettings::SetLineWidth(double width) {
		line_width_ = width;
	}
	void RendererSettings::SetColorPallete(std::vector<svg::Color> colors) {
		for(auto &color : colors){
			colors_.emplace_back(color);
		}
		
	}
	const Screen RendererSettings::GetScreen() const {
		return screen_;
	}
	double RendererSettings::GetLineWidth() const {
		return line_width_;
	}
	std::vector<svg::Color> RendererSettings::GetColorPallete() const {
		return colors_;
	}
	UnderLayer RendererSettings::GetUnderLayerProperties() const {
		return underlayer_properties_;
	}
	BusLabel RendererSettings::GetBusSettings() const {
		return bus_settings_;
	}
	StopLabel RendererSettings::GetStopSettings() const
	{
		return stop_settings_;
	}
	

	Renderer::Renderer(const tr_cat::TransportCatalogue& catalogue, const RendererSettings& settings)
		:transport_catalogue_(catalogue),
		settings_(settings)
	{}


	void Renderer::RenderBusRouteLine(svg::Document& doc, const std::vector<svg::Point>& stops_points, size_t color_idx) const
	{

		svg::Polyline polyline;

		for (const auto& stop_point : stops_points) {

			polyline.AddPoint(stop_point)
				.SetStrokeWidth(settings_.GetLineWidth())
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
				.SetFillColor(svg::NoneColor)
				.SetStrokeColor(settings_.GetColorPallete()[color_idx]);
		}
		doc.Add(std::move(polyline));
	}
	void Renderer::RenderBusName(svg::Document& doc, const svg::Point& pos, const std::string& name, size_t color_idx) const {
		svg::Text text_underlayer;
		text_underlayer.SetFillColor(settings_.GetUnderLayerProperties().color_)
			.SetStrokeColor(settings_.GetUnderLayerProperties().color_)
			.SetStrokeWidth(settings_.GetUnderLayerProperties().width_)
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
			.SetPosition(pos)
			.SetOffset({ settings_.GetBusSettings().offset_.x, settings_.GetBusSettings().offset_.y })
			.SetFontSize(static_cast<uint32_t>(settings_.GetBusSettings().font_size_))
			.SetFontFamily("Verdana")
			.SetFontWeight("bold")
			.SetData(name);

		doc.Add(std::move(text_underlayer));

		svg::Text text_name;
		text_name.SetFillColor(settings_.GetColorPallete()[color_idx]) // the same as bus name !!! check
			.SetPosition(pos)
			.SetOffset({ settings_.GetBusSettings().offset_.x, settings_.GetBusSettings().offset_.y })
			.SetFontSize(static_cast<uint32_t>(settings_.GetBusSettings().font_size_))
			.SetFontFamily("Verdana")
			.SetFontWeight("bold")
			.SetData(name);

		doc.Add(std::move(text_name));
	}
	void Renderer::RenderStopCircles(svg::Document& doc, const std::vector<std::pair<std::string_view, svg::Point>> points) const
	{
		for (const auto& point : points) {
			svg::Circle circle;
			circle.SetCenter(point.second)
				.SetRadius(settings_.GetStopSettings().stop_radius_)
				.SetFillColor("white"s);
			doc.Add(std::move(circle));
		}


	}
	void Renderer::RenderStopNames(svg::Document& doc, const std::vector<std::pair<std::string_view, svg::Point>> points) const {
		svg::Text text_common;

		text_common.SetOffset({ settings_.GetStopSettings().offset_})
			.SetFontSize(static_cast<uint32_t>(settings_.GetStopSettings().font_size_))
			.SetFontFamily("Verdana");

		svg::Text bus_name_underlayer{ text_common };
		bus_name_underlayer.SetFillColor(settings_.GetUnderLayerProperties().color_)
			.SetStrokeColor(settings_.GetUnderLayerProperties().color_)
			.SetStrokeWidth(settings_.GetUnderLayerProperties().width_)
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

		svg::Text bus_name_title{ text_common };
		bus_name_title.SetFillColor("black"s);

		for (const auto& point : points) {
			bus_name_underlayer.SetData(std::string(point.first)).SetPosition(point.second);
			doc.Add(bus_name_underlayer);

			bus_name_title.SetData(std::string(point.first)).SetPosition(point.second);

			doc.Add(bus_name_title);
		}
	}
	size_t Renderer::GetColorPaletteSize() const {
		return settings_.GetColorPallete().size();
	}
	svg::Color Renderer::ColorPicker(int route_id) const
	{
		return settings_.GetColorPallete()[route_id];
	}
	void Renderer::RenderBusRoutelines(svg::Document& doc, SphereProjector& sp) {
		const size_t colors_in_palete_size = GetColorPaletteSize();

		size_t color_idx = 0;
		
		for (const std::string_view bus_name : transport_catalogue_.GetOrderedBusses()) {
			auto bus_ptr = transport_catalogue_.FindBus(bus_name);
			std::vector<svg::Point> stops_points;
			if (bus_ptr->stops.empty()) { continue; }
			for (auto stop : bus_ptr->stops) {

				double lat = transport_catalogue_.FindStop(stop->name)->latitude;
				double lng = transport_catalogue_.FindStop(stop->name)->longitude;
				stops_points.push_back(sp({ lat, lng }));
			}

			// for line bus reverse route
			if (!bus_ptr->is_roundtrip) {
				auto it = bus_ptr->stops.rbegin() + 1;

				while (it != bus_ptr->stops.rend()) {
					double lat = transport_catalogue_.FindStop((*(*it)).name)->latitude;
					double lng = transport_catalogue_.FindStop((*(*it)).name)->longitude;
					stops_points.push_back(sp({ lat, lng }));
					++it;
				}

			}

			RenderBusRouteLine(doc, stops_points, color_idx);
			++color_idx;

			if (color_idx == colors_in_palete_size) {
				color_idx = 0;
			}
		}
		

	}
	void Renderer::RenderBusNames(svg::Document& doc, SphereProjector& sp) {
		const size_t colors_in_palete_size = GetColorPaletteSize();

		size_t color_idx = 0;
		for (const std::string_view bus_name : transport_catalogue_.GetOrderedBusses()) {

			Bus* bus_ptr = transport_catalogue_.FindBus(bus_name);
			

			if (!bus_ptr->stops.empty()) {
				std::string_view first_stop = bus_ptr->stops.front()->name;

				double lat = transport_catalogue_.FindStop(first_stop)->latitude;
				double lng = transport_catalogue_.FindStop(first_stop)->longitude;

				RenderBusName(doc, sp({ lat, lng }), bus_ptr->name, color_idx);

				// Draw second text if bus is not round or line bus with same fin stops
				std::string_view last_stop = bus_ptr->stops.back()->name;
				bool is_same_first_last_stops = (first_stop == last_stop);

				if (!bus_ptr->is_roundtrip && !is_same_first_last_stops) {
					lat = transport_catalogue_.FindStop(last_stop)->latitude;
					lng = transport_catalogue_.FindStop(last_stop)->longitude;

					RenderBusName(doc, sp({ lat, lng }), bus_ptr->name, color_idx);
				}

				++color_idx;

				if (color_idx == colors_in_palete_size) {
					color_idx = 0;
				}
			}
		}
	}
	SphereProjector Renderer::ConstructSphereProjector() {
		std::deque<geo::Coordinates> geo_points;

		for (std::string_view stop_name : transport_catalogue_.GetOrderedStops()) {
			if (transport_catalogue_.GetStopInfo(stop_name).buses.empty()) {
				continue;
			}

			double lat = transport_catalogue_.FindStop(stop_name)->latitude;
			double lng = transport_catalogue_.FindStop(stop_name)->longitude;
			geo_points.push_back({ lat, lng });
		}

		SphereProjector sp(geo_points.begin(), geo_points.end(), settings_.GetScreen().width_, settings_.GetScreen().height_, settings_.GetScreen().padding_);
		return sp;
	}
	std::string Renderer::Render()
	{
		std::stringstream o_stream;
		svg::Document doc;
		std::set<std::string_view> sorted_stop_names;
		SphereProjector sp = ConstructSphereProjector();
		RenderBusRoutelines(doc, sp);
		RenderBusNames(doc, sp);
			// Add stops cycles excl stops without buses
		std::vector<std::pair<std::string_view, svg::Point >> stops_and_buses_cicles;

		for (std::string_view stop_name : transport_catalogue_.GetOrderedStops()) {
				if (transport_catalogue_.GetStopInfo(stop_name).buses.empty()) {
					continue;
				}
				double lat = transport_catalogue_.FindStop(stop_name)->latitude;
				double lng = transport_catalogue_.FindStop(stop_name)->longitude;
				stops_and_buses_cicles.emplace_back(stop_name, sp({ lat, lng }));
			}

		RenderStopCircles(doc, stops_and_buses_cicles);
		RenderStopNames(doc, stops_and_buses_cicles);

		doc.Render(o_stream);
		return o_stream.str();
		} //Render

	
	
}//namespace render
#pragma once
#include "geo.h"
#include "domain.h"
#include "svg.h"
#include "json.h"
#include "transport_catalogue.h"
namespace render {

	struct Screen {
		Screen() = default;
		Screen (double width, double height, double padding):
			width_(width), height_(height), padding_(padding){}
		 double width_{ 0. };
		 double height_{ 0. };
		 double padding_{ 0. };
	};


	struct UnderLayer {
		svg::Color color_ = "none";
		double width_;
	};

	struct BusLabel {
		int font_size_;
		svg::Point offset_;
	};

	struct StopLabel {
		int font_size_;
		svg::Point offset_;
		double stop_radius_;
	};
	inline const double EPSILON = 1e-6;
	
	
	class SphereProjector {
	public:
		bool IsZero(double value) {
			return std::abs(value) < EPSILON;
		}
		
		template <typename PointInputIt>
		SphereProjector(PointInputIt points_begin, PointInputIt points_end,
			double max_width, double max_height, double padding)
			: padding_(padding) //
		{
			// ���� ����� ����������� ����� �� ������, ��������� ������
			if (points_begin == points_end) {
				return;
			}

			// ������� ����� � ����������� � ������������ ��������
			const auto [left_it, right_it] = std::minmax_element(
				points_begin, points_end,
				[](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
			min_lon_ = left_it->lng;
			const double max_lon = right_it->lng;

			// ������� ����� � ����������� � ������������ �������
			const auto [bottom_it, top_it] = std::minmax_element(
				points_begin, points_end,
				[](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
			const double min_lat = bottom_it->lat;
			max_lat_ = top_it->lat;

			// ��������� ����������� ��������������� ����� ���������� x
			std::optional<double> width_zoom;
			if (!IsZero(max_lon - min_lon_)) {
				width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
			}

			// ��������� ����������� ��������������� ����� ���������� y
			std::optional<double> height_zoom;
			if (!IsZero(max_lat_ - min_lat)) {
				height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
			}

			if (width_zoom && height_zoom) {
				// ������������ ��������������� �� ������ � ������ ���������,
				// ���� ����������� �� ���
				zoom_coeff_ = std::min(*width_zoom, *height_zoom);
			}
			else if (width_zoom) {
				// ����������� ��������������� �� ������ ���������, ���������� ���
				zoom_coeff_ = *width_zoom;
			}
			else if (height_zoom) {
				// ����������� ��������������� �� ������ ���������, ���������� ���
				zoom_coeff_ = *height_zoom;
			}
		}

		
		svg::Point operator()(geo::Coordinates coords) const {
			return {
				(coords.lng - min_lon_) * zoom_coeff_ + padding_,
				(max_lat_ - coords.lat) * zoom_coeff_ + padding_
			};
		}

	private:
		double padding_;
		double min_lon_ = 0;
		double max_lat_ = 0;
		double zoom_coeff_ = 0;
	};
	class RendererSettings {
	public :
		RendererSettings() = default;
	
		void SetScreen(Screen screen);
		void SetBusSettings(int bus_label_font_size, svg::Point offset);
		void SetStopSettings(int stop_label_font_size, svg::Point offset, double stop_radius);
		void SetUnderLayer(UnderLayer under_layer);
		void SetLineWidth(double width);
		void SetColorPallete(std::vector<svg::Color> colors);
		const Screen GetScreen() const;
		double GetLineWidth() const;
		std::vector<svg::Color> GetColorPallete() const;
		UnderLayer GetUnderLayerProperties() const;
		BusLabel GetBusSettings() const;
		StopLabel GetStopSettings() const;

	protected:
		Screen screen_;
		BusLabel bus_settings_;
		StopLabel stop_settings_;
		UnderLayer underlayer_properties_;
		double line_width_ = 0.0;
		std::vector<svg::Color> colors_{};

	};

	class Renderer {
	
	public: 

		Renderer(const tr_cat::TransportCatalogue& catalogue, const RendererSettings& settings);

	public: 
		std::string Render();

	private:  
		void RenderBusRouteLine(svg::Document& doc, const std::vector<svg::Point>& stops_points, size_t color_idx) const;
		void RenderBusName(svg::Document& doc, const svg::Point& pos, const std::string& name, size_t color_idx) const;
		void RenderStopCircles(svg::Document& doc, const std::vector<std::pair<std::string_view, svg::Point>> points) const;
		void RenderStopNames(svg::Document& doc, const std::vector<std::pair<std::string_view, svg::Point>> points) const;
		void RenderBusRoutelines(svg::Document& doc,SphereProjector& sp);
		void RenderBusNames(svg::Document& doc, SphereProjector& sp);
		SphereProjector ConstructSphereProjector();
		size_t GetColorPaletteSize() const;
		svg::Color ColorPicker(int route_id) const;
		
	private: 
		
		const tr_cat::TransportCatalogue& transport_catalogue_;
		const RendererSettings& settings_;
		
		
		};
}


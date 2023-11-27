#define _USE_MATH_DEFINES
#include "svg.h"
#include<cmath>
#include <sstream>


namespace svg {

    using namespace std::literals;
    //------------------Color---------------------------//

    //------------------End of Color---------------------------//




    //------------------Object---------------------------//
    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }
    //------------------End_Of_Object--------------------//

    //------------------Circle---------------------------//


    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }
    //------------------End_of_Circle--------------------//


    //------------------Polyline-------------------------//


    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool isfirst = true;
        for (Point point : points_) {
            if (isfirst == true) {
                out << point.x << "," << point.y;
                isfirst = false;
                continue;
            }
            out << " " << point.x << "," << point.y;
        }

        std::stringstream stream;
        RenderAttrs(stream);

        if (!stream.str().empty()) {
            out << "\""sv;
            out << stream.str();
            out << "/>"sv;
        }
        else {
            out << "\"/>"sv;
        }
    }




    //------------------End_of_Polyline------------------//

    //------------------Text-----------------------------//

    std::string Text::TextTransformer(const std::string str_orig) {
        std::string str = str_orig;
        size_t pos = 0;

        while ((pos = str.find('&', pos)) != std::string::npos) {
            str.replace(pos, 1, "&amp;");
            pos++;
        }
        pos = 0;
        while ((pos = str.find('\"', pos)) != std::string::npos) {
            str.replace(pos, 1, "&quot;");
        }
        pos = 0;

        while ((pos = str.find('\'', pos)) != std::string::npos) {
            str.replace(pos, 1, "&apos;");
        }
        pos = 0;
        while ((pos = str.find('<', pos)) != std::string::npos) {
            str.replace(pos, 1, "&lt;");
        }
        pos = 0;
        while ((pos = str.find('>', pos)) != std::string::npos) {
            str.replace(pos, 1, "&gt;");
        }

        return str;
    }


    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {

        offset_ = std::move(offset);
        return *this;
    }

    // Задаёт размеры шрифта (атрибут font-size)
    Text& Text::SetFontSize(uint32_t size) {

        size_ = std::move(size);
        return *this;
    }

    // Задаёт название шрифта (атрибут font-family)
    Text& Text::SetFontFamily(std::string font_family)
    {
        font_family_ = std::move(font_family);
        return *this;
    }
    // Задаёт толщину шрифта (атрибут font-weight)
    Text& Text::SetFontWeight(std::string font_weight) {

        font_weight_ = std::move(font_weight);
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& Text::SetData(std::string data) {
        data_ = std::move(data);
        transformed_data_ = TextTransformer(data_);
        return *this;
    }


    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text"sv;
        std::stringstream stream;
        RenderAttrs(stream);

        if (!stream.str().empty()) {
            out << stream.str();
            out << " "sv;
        }
        else {
            out << " "sv;
        }
        out << "x=\"" << pos_.x << "\" ";
        out << "y=\"" << pos_.y << "\" ";
        out << "dx=\"" << offset_.x << "\" ";
        out << "dy=\"" << offset_.y << "\" ";
        out << "font-size=\"" << size_ << "\" ";
        if (font_family_ != "") {
            out << "font-family=\"" << font_family_ << "\"";
        }
        if (font_weight_ != "") {
            out << " font-weight=\"" << font_weight_ << "\"";
        }
        out << ">" << transformed_data_ << "</text>";
    }

    //------------------End_Of_Text----------------------//


    //------------------Document------------------------//

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        if (obj != nullptr) {
            objects_.emplace_back(move(obj));
        }
        else { throw std::invalid_argument("Null-Object injection"); }
    }

    void Document::Render(std::ostream& out) const {
        RenderContext ren(out, 1, 1);
        ren.out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        ren.out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        for (auto& obj : objects_) {
            obj->Render(ren);
        };

        ren.out << "</svg>";
    }
    //------------------End_Of_Document-----------------//

    //-------------------Operators---------------------//
    std::ostream& operator<<(std::ostream& os, const Rgb& rgb) {
        os << "rgb("s << std::to_string(rgb.red) << ","s << std::to_string(rgb.green) << ","s << std::to_string(rgb.blue) << ")"s;
        return os;
    }
    std::ostream& operator<<(std::ostream& os, const Rgba& rgba) {
        os << "rgba("s << std::to_string(rgba.red) << ","s << std::to_string(rgba.green) << ","s << std::to_string(rgba.blue) << ","s << rgba.opacity << ")"s;
        return os;
    }
   // std::ostream& operator<<(std::ostream& os, const Rgb& rgb) {
  //      std::ostringstream ss;
    //    ss << "rgb("s << static_cast<int>(rgb.red) << ","s << static_cast<int>(rgb.green) << ","s << static_cast<int>(rgb.blue) << ")"s;
    //    os << ss.str();
    //    return os;
    //}
   // std::ostream& operator<<(std::ostream& os, const Rgba& rgba) {
   //     std::ostringstream ss;
    //    ss << "rgba("s << static_cast<int>(rgba.red) << ","s << static_cast<int>(rgba.green) << ","s << static_cast<int>(rgba.blue) << ","s << rgba.opacity << ")"s;
   //     os << ss.str();
    //    return os;
   // }
    std::ostream& operator<<(std::ostream& os, const Color& color) {
        using namespace std::literals;
        std::visit([&os](Color color) {
            if (std::holds_alternative<std::monostate>(color)) {
                os << NoneColor;
            }
            else if (const auto* value2 = std::get_if<std::string>(&color)) {
                os << *value2;
            }
            else if (const auto* value3 = std::get_if<Rgb>(&color)) {
                os << *value3;

            }
            else if (const auto* value4 = std::get_if<Rgba>(&color)) {
                os << *value4;
            }

            }, color);
        return os;
    }
    //-------------------END OF Operators---------------------//
}  // namespace svg

namespace shapes {


    //------------------Triangle---------------------------//

    Triangle::Triangle(svg::Point p1, svg::Point p2, svg::Point p3)
        : p1_(p1)
        , p2_(p2)
        , p3_(p3) {
    }

    // Реализует метод Draw интерфейса svg::Drawable
    void  Triangle::Draw(svg::ObjectContainer& container) const {
        container.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
    }

    //----------------EndOfTriangle------------------------// 

     //-------------------Star-----------------------------//

    Star::Star(svg::Point center, double outer_rad, double inner_rad, int num_rays) :
        star_(std::move(Star::CreateStar(center, outer_rad, inner_rad, num_rays))) {}


    svg::Polyline Star::CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays) {
        svg::Polyline polyline;
        for (int i = 0; i <= num_rays; ++i) {

            double angle = 2 * M_PI * (i % num_rays) / num_rays;
            polyline.AddPoint({ center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle) });
            if (i == num_rays) {
                break;
            }
            angle += M_PI / num_rays;
            polyline.AddPoint({ center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle) });
            polyline.SetFillColor("red").SetStrokeColor("black");
        }
        return polyline;
    }


    void Star::Draw(svg::ObjectContainer& container) const {
        container.Add(star_);

    }
    //---------------EndOfStar----------------------------//

    //-------------------SnowMan--------------------------//
    Snowman::Snowman(svg::Point head_center, double radius) :head_center_(head_center), radius_(radius) {}

    void Snowman::Draw(svg::ObjectContainer& container) const {

        svg::Circle head;
        head.SetCenter(head_center_)
            .SetRadius(radius_)
            .SetFillColor("rgb(240,240,240)")
            .SetStrokeColor("black");

        svg::Circle body;
        body.SetCenter({ head_center_.x, head_center_.y + radius_ * 2 })
            .SetRadius(radius_ * 1.5)
            .SetFillColor("rgb(240,240,240)")
            .SetStrokeColor("black");
        svg::Circle bottom;
        bottom.SetCenter({ head_center_.x, head_center_.y + radius_ * 5 })
            .SetRadius(radius_ * 2)
            .SetFillColor("rgb(240,240,240)")
            .SetStrokeColor("black");

        container.Add(bottom);
        container.Add(body);
        container.Add(head);
    }



    //-------------------EndOfSnowMan---------------------//




} // namespace shapes 
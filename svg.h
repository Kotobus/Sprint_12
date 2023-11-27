#pragma once

#define _USE_MATH_DEFINESс
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <string_view>
#include <algorithm>
#include <optional>
#include <variant>

using namespace std::literals;

namespace svg {
    struct PrintColor {
        PrintColor() = default;
        PrintColor(std::ostream& str) :str_(str) {};
        std::ostream& str_;
    };
    /*inline std::ostream& operator <<(std::ostream& oc,Color& color) {
         std::visit(PrintColor(oc), color);
         return oc;
    }*/
    struct Rgb {

        Rgb() = default;
        explicit Rgb(uint8_t red, uint8_t green, uint8_t blue) :red(red), green(green), blue(blue)
        {}

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };
    struct Rgba {

        Rgba() = default;
        explicit Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity) :red(red), green(green), blue(blue), opacity(opacity)
        {}
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };
    using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;

    inline const Color NoneColor = ("none"s);
    std::ostream& operator<<(std::ostream& out, const Rgb& color);
    std::ostream& operator<<(std::ostream& out, const Rgba& color);
    std::ostream& operator<<(std::ostream& out, const Color& color);


    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };
    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };
    inline std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap_) {
        using namespace std::literals;
        switch (static_cast<int>(line_cap_)) {
        case 0:
            out << "butt"sv;
            break;
        case 1:
            out << "round"sv;
            break;
        case 2:
            out << "square"sv;
            break;
        }
        return out;
    }

    inline std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join_) {
        using namespace std::literals;
        switch (static_cast<int>(line_join_)) {
        case 0:
            out << "arcs"sv;
            break;
        case 1:
            out << "bevel"sv;
            break;
        case 2:
            out << "miter"sv;
            break;
        case 3:
            out << "miter-clip"sv;
            break;
        case 4:
            out << "round"sv;
            break;
        }
        return out;
    }

    /*
     * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
     * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
     */
    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    /*
     * Абстрактный базовый класс Object служит для унифицированного хранения
     * конкретных тегов SVG-документа
     * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
     */
    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };
    //-------------------------PathProps-------------------------//
    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeWidth(double width) {
            stroke_width_ = width;
            return AsOwner();

        };
        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            line_cap_ = line_cap;
            return AsOwner();

        };
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            line_join_ = line_join;
            return AsOwner();

        };

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) {
                out << " fill=\""sv << fill_color_.value() << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv << stroke_color_.value() << "\""sv;
            }
            if (stroke_width_) {
                out << " stroke-width=\""sv << stroke_width_.value() << "\""sv;
            }
            if (line_cap_) {
                out << " stroke-linecap=\""sv << line_cap_.value() << "\""sv;
            }
            if (line_join_) {
                out << " stroke-linejoin=\""sv << line_join_.value() << "\""sv;
            }
        }

    protected:
        Owner& AsOwner() {
            // static_cast безопасно преобразует *this к Owner&,
            // если класс Owner — наследник PathProps
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_ = std::nullopt;
        std::optional<Color> stroke_color_ = std::nullopt;
        std::optional<double> stroke_width_ = std::nullopt;
        std::optional<StrokeLineCap> line_cap_ = std::nullopt;
        std::optional<StrokeLineJoin> line_join_ = std::nullopt;
    };

    //-------------------------End_Of_PathProps------------------//

    //-------------------------Circle----------------------------//
    /*
     * Класс Circle моделирует элемент <circle> для отображения круга
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */

    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;


        Point center_;
        double radius_ = 1.0;
    };
    //-------------------------END OF Circle---------------------//

    //-------------------------Polyline--------------------------//
    /*
     * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline& AddPoint(Point point);

    private:
        void RenderObject(const RenderContext& context) const override;


        std::vector<Point> points_;

    };
    /*
     * Класс Text моделирует элемент <text> для отображения текста
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text final :public Object, public PathProps<Text> {
    public:
        // Задаёт координаты опорной точки (атрибуты x и y)
        std::string TextTransformer(std::string str_orig);

        Text& SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(std::string font_family);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(std::string font_weight);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(std::string data);

    private:
        void RenderObject(const RenderContext& context) const override;
        Point pos_{ 0.0, 0.0 };
        Point offset_{ 0.0, 0.0 };
        uint32_t size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_ = "";
        std::string transformed_data_ = "";
    };
    //-------------------------END OF Polyline--------------------------//


    class ObjectContainer {
    public:
        template <typename Obj>
        void Add(Obj obj) {
            objects_.emplace_back(std::make_unique<Obj>(std::move(obj)));
        }

        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

        virtual ~ObjectContainer() = default;

    protected:
        std::vector<std::unique_ptr<Object>> objects_;
    };

    class Document : public ObjectContainer {
    public:

        void AddPtr(std::unique_ptr<Object>&& obj) override;
        void Render(std::ostream& out) const;

    };

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;
        virtual ~Drawable() = default;
    };

}  // namespace svg

namespace shapes {

    class Triangle final : public svg::Drawable {
    public:
        Triangle(svg::Point p1, svg::Point p2, svg::Point p3);


        // Реализует метод Draw интерфейса svg::Drawable
        void Draw(svg::ObjectContainer& container) const override;

    private:
        svg::Point p1_, p2_, p3_;
    };

    class Star final : public svg::Drawable {
    public:
        Star(svg::Point center, double outer_rad, double inner_rad, int num_rays);

        void Draw(svg::ObjectContainer& container) const override;



    private:
        svg::Polyline star_;
        svg::Polyline CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays);

    };
    class Snowman : public svg::Drawable {
    public:
        Snowman(svg::Point head_center, double radius);
        void Draw(svg::ObjectContainer& container) const override;

    private:
        svg::Point head_center_;
        double radius_ = 0.0;

    };

} // namespace shapes 
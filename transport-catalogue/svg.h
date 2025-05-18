#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>

namespace svg {
    
using Color = std::string;
    
extern Color DefaultColor;

inline const Color NoneColor{"none"};
    
enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

std::ostream& operator << (std::ostream &os, const StrokeLineCap &cap);
    
    
enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};
    
std::ostream& operator << (std::ostream &os, const StrokeLineJoin &join);
         
        
class Object;

    
class ObjectContainer {
public:
    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    
    template <typename T>
    void Add(T obj) {
        AddPtr(std::make_unique<T>(std::move(obj)));
    }
    
    virtual ~ObjectContainer() {
    }
    
};
    
class Drawable {
public:
    virtual void Draw(ObjectContainer &container) const = 0;
    virtual ~Drawable() {
    }
};

    
struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};


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
        return {out, indent_step, indent + indent_step};
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

class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};


template<class T>
class PathProps {
    public:
        T& SetFillColor(Color color){
            fill_ = color;
            return *static_cast<T*>(this);
        }
        T& SetStrokeColor(Color color){
            stroke_ = color;
            return *static_cast<T*>(this);
        }
        T& SetStrokeWidth(double width){
            stroke_width_ = width;
            return *static_cast<T*>(this);
        }
        T& SetStrokeLineCap(StrokeLineCap line_cap){
            stroke_linecap_ = line_cap;
            return *static_cast<T*>(this);
        }
        T& SetStrokeLineJoin(StrokeLineJoin line_join) {
            stroke_linejoin_ = line_join;
            return *static_cast<T*>(this);
        }
        void WriteProps(std::ostream& context) const;
protected:
    
    std::optional<Color> fill_;
    std::optional<Color> stroke_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> stroke_linecap_;
    std::optional<StrokeLineJoin> stroke_linejoin_;
};
     
class Circle final : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};
        
        
class Polyline final : public Object, public PathProps<Polyline> {
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

    void RenderObject(const RenderContext& context) const override;
private:
    std::vector<Point> points_;
};
   
        
class Text final : public Object, public PathProps<Text> {
public:
    Text& SetPosition(Point pos);

    Text& SetOffset(Point offset);

    Text& SetFontSize(uint32_t size);

    Text& SetFontFamily(std::string font_family);

    Text& SetFontWeight(std::string font_weight);

    Text& SetData(std::string data);

    void RenderObject(const RenderContext& context) const override;
private:
    std::string data_;
    Point anchor_point_;
    Point offset_;
    uint32_t font_size_ = 1;
    std::string font_family_;
    std::string font_weight_;
};

       
class Document : public ObjectContainer {
public:

    void AddPtr(std::unique_ptr<Object>&& obj) override;

    void Render(std::ostream& out) const;

private:
    std::vector<std::unique_ptr<Object>> objects_;
};
    
namespace shapes {

class Triangle : public svg::Drawable {
public:
    Triangle(svg::Point p1, svg::Point p2, svg::Point p3)
        : p1_(p1)
        , p2_(p2)
        , p3_(p3) {
    }

    void Draw(svg::ObjectContainer& container) const override;

private:
    svg::Point p1_, p2_, p3_;
};

class Star : public svg::Drawable {
public:
    Star(svg::Point center, double outer_rad, double inner_rad, int num_rays) :
        center_(center), outer_rad_(outer_rad), inner_rad_(inner_rad), num_rays_(num_rays) {
    }
    
    void Draw(svg::ObjectContainer& container) const override;
    
private:
    svg::Point center_;
    double outer_rad_;
    double inner_rad_;
    int num_rays_;
};
class Snowman : public svg::Drawable {
public:
    Snowman(svg::Point center, double rad) : center_(center), rad_(rad) {
    }
    
    void Draw(svg::ObjectContainer& container) const override;
    
private:
    svg::Point center_;
    double rad_;
};

} // namespace shapes

}  // namespace

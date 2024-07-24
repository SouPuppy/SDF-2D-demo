#include <opencv2/opencv.hpp>
#include <iostream>
#include <algorithm>
#include <vector>
#include <memory>
#include <cmath>
#include "grid.h"
#include "vec.h"

#define THERMAL

struct Rotation {
    float angle;  // Angle in degrees
};

struct LevelSet {
    float threshold;
    Rotation rot;
    virtual float sdf_func(const Point& p) const = 0;
    virtual ~LevelSet() = default;
    
    Point rotate(const Point& p, float angle) const {
        float rad = angle * M_PI / 180.0;
        float cosA = std::cos(rad);
        float sinA = std::sin(rad);
        return Point(
            p.x * cosA - p.y * sinA,
            p.x * sinA + p.y * cosA
        );
    }

    Point rotate(const Point& anchor, const Point& p, float angle) const {
        float rad = angle * M_PI / 180.0;
        float cosA = std::cos(rad);
        float sinA = std::sin(rad);

        float relX = p.x - anchor.x;
        float relY = p.y - anchor.y;

        float newRelX = relX * cosA - relY * sinA;
        float newRelY = relX * sinA + relY * cosA;

        float newX = newRelX + anchor.x;
        float newY = newRelY + anchor.y;

        return Point(newX, newY);
    }

    void set_angle(float angle) {
        rot.angle = angle;
    }
    void set_threshold(float _threshold) {
        threshold = _threshold;
    }
};

struct Circle : LevelSet {
    Point center;
    float a, b;

    Circle(Point _center, float _a) : center(_center), a(_a), b(_a) {}
    Circle(Point _center, float _a, float _b) : center(_center), a(_a), b(_b) {}

    float sdf_func(const Point& p) const override {
        Point rotated_p = rotate(Point(p.x - center.x, p.y - center.y), rot.angle);
        float dx = rotated_p.x / a;
        float dy = rotated_p.y / b;
        return std::sqrt(dx * dx + dy * dy) - 1 + threshold;
    }
};

struct Segment : LevelSet {
    Point p1, p2; // Two points defining the seg

    Segment(Point _p1, Point _p2, float _threshold = 1) : p1(_p1), p2(_p2) {
        set_threshold(_threshold);
    }

    float sdf_func(const Point& p) const override {
        Vec seg_vec = rotate(p2 - p1, -rot.angle);
        float seg_len = seg_vec.len();
        Vec seg_dir = direction(seg_vec);
        Vec point_vec = p - p1;
        float projection = dot(point_vec, seg_dir);
        projection = std:: clamp(projection, 0.0f, seg_len);
        Point closest_point = p1 + (seg_dir * projection);
        float distance = dist(p, closest_point);
        return distance - threshold;
    }
};

struct Line : LevelSet {
    Point p1, p2; // C < 0, AC > 0 

    Line(Point _p1, Point _p2, float _threshold = 1) : p1(_p1), p2(_p2) {
        set_threshold(_threshold);
    }

    float sdf_func(const Point& p) const override {
        Point rp = rotate(p1, p, -rot.angle);
        return dist2line(rp, p1, p2) - threshold;
    }
};









struct Union : LevelSet {
    std::shared_ptr<LevelSet> a, b;

    Union(std::shared_ptr<LevelSet> _a, std::shared_ptr<LevelSet> _b) : a(_a), b(_b) {}

    float sdf_func(const Point& p) const override {
        return std:: min(a->sdf_func(p), b->sdf_func(p));
    }
};

struct Difference : LevelSet {
    std::shared_ptr<LevelSet> a, b;

    Difference(std::shared_ptr<LevelSet> _a, std::shared_ptr<LevelSet> _b) : a(_a), b(_b) {}

    float sdf_func(const Point& p) const override {
        return std:: max(- a->sdf_func(p), b->sdf_func(p));
    }
};

struct Intersection : LevelSet {
    std::shared_ptr<LevelSet> a, b;

    Intersection(std::shared_ptr<LevelSet> _a, std::shared_ptr<LevelSet> _b) : a(_a), b(_b) {}

    float sdf_func(const Point& p) const override {
        return std:: max(a->sdf_func(p), b->sdf_func(p));
    }
};



void render_sdf(Grid& grid, const std::vector<std::shared_ptr<LevelSet>>& sdfs) {
    for (int i = grid.xmin; i <= grid.xmax; i++) {
        for (int j = grid.ymin; j <= grid.ymax; j++) {
            float min_sdf_value = 1e5;
            for (const auto& sdf : sdfs) {
                min_sdf_value = std::min(min_sdf_value, sdf->sdf_func(Point(i, j)));
            }
#ifndef THERMAL
            grid.update(i, j, min_sdf_value < 0 ? Color::WHITE : Color::OPACITY);
#else
            grid.thermal(i, j, min_sdf_value);
#endif
        }
    }
}

#include "myshape.h"

int main() {
    Grid grid(-400, 400, -300, 300);

// Top
    auto circ0 = std::make_shared<Circle>(Point( 66, 0), 80);
    auto circ1 = std::make_shared<Circle>(Point(-66, 0), 80);
    
    
    auto top = std::make_shared<Union>(circ0, circ1);

// Triangle
    auto lin0 = std::make_shared<Line>(Point(-120, -58), Point(120, -58));
    auto lin1 = std::make_shared<Line>(Point(-120, -58), Point(120, -58));
    lin1->set_angle(135);
    auto lin2 = std::make_shared<Line>(Point(120, -58), Point(-120, -58));
    lin2->set_angle(45);
    
    auto tri = std::make_shared<Intersection>(lin0, std::make_shared<Intersection>(lin1, lin2));

    auto heart = std::make_shared<Union>(tri, top);

    std::vector<std::shared_ptr<LevelSet>> sdfs = {heart};

    render_sdf(grid, sdfs);

    grid.draw_axis();
    grid.show("SDF 2D demo");
    return 0;
}

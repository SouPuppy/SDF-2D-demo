typedef struct Point {
    Point(float _x, float _y) : x(_x), y(_y) {}
    float x, y;

    Point operator-(const Point& b) const {
        return Point(x - b.x, y - b.y);
    }
    Point operator+(const Point& b) const {
        return Point(x + b.x, y + b.y);
    }
    Point operator*(float scale) const {
        return Point(x * scale, y * scale);
    }
    Point operator/(float scale) const {
        return Point(x / scale, y / scale);
    }
    float len() {
        return sqrt(x*x + y*y);
    }
} Vec;

Vec direction(Vec a) {
    float len = sqrt(a.x * a.x + a.y * a.y);
    return a / len;
}

float dot(Vec a, Vec b) {
    return a.x * b.x + a.y * b.y;
}

float cross(const Vec& a, const Vec& b) {
    return a.x * b.y - a.y * b.x;
}

float dist(Point a, Point b) {
    return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

float dist2line(const Point& a, const Point& p1, const Point& p2) {
    Vec line_vec = p2 - p1;
    Vec point_vec = a - p1;

    float line_len = line_vec.len();
    if (line_len == 0) return 0;

    float projection_length = (dot(point_vec, line_vec) / line_len);

    Vec closest_point_on_line = line_vec * (projection_length / line_len);
    float distance = (a - (p1 + closest_point_on_line)).len();

    float cross_product = cross(point_vec, line_vec);

    return cross_product > 0 ? -distance : distance;
}

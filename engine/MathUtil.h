#ifndef MATHUTIL_H
#define MATHUTIL_H

#include <iostream>

namespace ECSEngine {

#include <cstdio>

struct Point2D {
    float x, y;
    Point2D(float x = 0, float y = 0) : x(x), y(y) {}
    Point2D(int x, int y) : x(x), y(y) {}
};

[[maybe_unused]] static std::ostream& operator<<(std::ostream& os, const Point2D& p) {
    os << "(" << p.x << ", " << p.y << ")";
    return os;
}

[[maybe_unused]] static Point2D operator*(float number, const Point2D& rhs) {
    return Point2D(number * rhs.x, number * rhs.y);
}

struct Line {
    Point2D p1, p2;

    Line(Point2D p1 = {0, 0}, Point2D p2 = {0, 0}) : p1(p1), p2(p2) {}
    Line(float x1, float y1, float x2, float y2) : p1(x1, y1), p2(x2, y2) {}
};

[[maybe_unused]] static std::ostream& operator<<(std::ostream& os, const Line& l) {
    os << "L{" << l.p1 << ", " << l.p2 << "}";
    return os;
}

struct Circle {
    Point2D center;
    float radius;

    Circle(Point2D c = {0, 0}, float r = 0) : center(c), radius(r) {}
    Circle(float x, float y, float r) : center(x, y), radius(r) {}
};

struct Rect {
    Point2D topLeft;
    float width, height;

    Rect &operator&=(const Rect &other) {
        // Make this rectangle the maximum sized rectangle that both rectangles overlap
        float left = std::max(topLeft.x, other.topLeft.x);
        float top = std::max(topLeft.y, other.topLeft.y);
        float right = std::min(topLeft.x + width, other.topLeft.x + other.width);
        float bottom = std::min(topLeft.y + height, other.topLeft.y + other.height);
        // NOTE: Originally had this function zero out the rectangle, it now just creates negative rectangles;
        // if wanting to change functionally, the check is (right < left || bottom < top)
        topLeft.x = left;
        topLeft.y = top;
        width = right - left;
        height = bottom - top;
        return *this;
    }
};

static bool RectIntersects(Rect a, const Rect &b) {
    a &= b;
    return a.width >= 0 && a.height >= 0;
}

[[maybe_unused]] static std::ostream& operator<<(std::ostream& os, const Rect& r) {
    os << "R{" << r.topLeft << ", " << r.width << "x" << r.height << "}";
    return os;
}

}  // namespace ECSEngine

#endif  // MATHUTIL_H

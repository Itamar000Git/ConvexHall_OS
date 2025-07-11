#ifndef CONVEXHALL_HPP
#define CONVEXHALL_HPP
#include <vector>

typedef struct Point{
    double x;
    double y;

    bool operator<(const Point& other) const {
        return x < other.x || (x == other.x && y < other.y);
    }
} Point;


typedef struct ConvexHull{
    std::vector<Point> points;
    int size;
    double area;
} ConvexHull;

void convexHull(std::vector<Point> points, ConvexHull& hull);
double cross(const Point& p1, const Point& p2, const Point& p3);
void convexHull(std::vector<Point> points);
double polygonArea(const std::vector<Point>& poly);
#endif
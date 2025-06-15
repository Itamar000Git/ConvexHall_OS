#include <iostream>
#include <vector>
#include <limits>
#include <algorithm>
#include <cmath>
#include "ConvexHall.hpp"

/**
 * @brief Culc the area of a polygon defined by the convex hull points.
 * @param poly The convex hull points.
 * @return The area of the polygon.
 */
double polygonArea(const ConvexHull& poly) {
    double area = 0;
    int n = poly.size;
    for (int i = 0; i < n; ++i) {
        const Point& p1 = poly.points[i];
        const Point& p2 = poly.points[(i + 1) % n];
        area += (p1.x * p2.y) - (p2.x * p1.y);
    }
  
    return std::abs(area) / 2.0;
}

/**
 * @brief Calculate the convex hull of a set of points using the Andrew's monotone chain algorithm.
 * @param points The input points.
 * @param hull The resulting convex hull.
 */
void convexHull(std::vector<Point> points, ConvexHull& hull) {
    if(points.empty()) {
        hull.size = 0;
        hull.area = 0.0;
        return; // No points to form a convex hull
    }
    int n = points.size();
    int k = 0;
    std::sort(points.begin(), points.end()); // Sort points (by x, then by y)
    hull.points.resize(2 * n); // Prepare space for the convex hull points
    hull.size = 2 * n;
      for (int i = 0; i < n; ++i) { // Build lower hull
        while (k >= 2 && cross(hull.points[k-2], hull.points[k-1], points[i]) <= 0) k--;
        hull.points[k++] = points[i];
    }
    for (int i = n-2, t = k+1; i >= 0; --i) { // Build upper hull
        while (k >= t && cross(hull.points[k-2], hull.points[k-1], points[i]) <= 0) k--;
        hull.points[k++] = points[i];
    }
    hull.points.resize(k - 1); // Remove the last point as it is the same as the first one
    hull.size = k - 1;
}

/**
 * @brief Calculate the cross product of vectors p1p2 and p1p3.
 * @param p1 The first point.
 * @param p2 The second point.
 * @param p3 The third point.
 * @return The cross product value.
 */
double cross(const Point& p1, const Point& p2, const Point& p3) {
    return (p2.x - p1.x)*(p3.y - p1.y) - (p2.y - p1.y)*(p3.x - p1.x);
}

/**
 * @brief Print the points of the convex hull.
 * @param hull The convex hull containing the points.
 */
void printConvexHull(const ConvexHull& hull) {
    std::cout << "Convex Hull Points:" << std::endl;
    for (const auto& point : hull.points) {
        std::cout << "(" << point.x << ", " << point.y << ")" << std::endl;
    }
}


int main() {
    ConvexHull graph;
    std::string cmd;
    ConvexHull hull;
    std::cout << "Convex Hull Algorithm Implementation" << std::endl;
    std::cout << "Available commands: Newgraph, CH, Newpoint, Removepoint, exit" << std::endl;
    while (std::cin >> cmd) {
        if (cmd == "Newgraph") {
            int n;
            std::cin >> n;
            graph.points.clear();
            hull.points.clear();
            hull.area = 0.0;
            graph.area = 0.0;
            graph.size = n;
            for (int i = 0; i < n; ++i) {
                std::cout << "Enter coordinates for point " << i + 1 << " (x y): ";
                char p;
                double x, y;
                std::cin >> x >>p >>y;
                if (std::cin.fail()|| p != ',') {
                    std::cin.clear(); // Clear the error state
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignore the rest of the line
                    std::cerr << "Invalid input. Please enter valid coordinates." << std::endl;
                    break;
                }
                graph.points.push_back(Point{x, y});
                graph.size = graph.points.size();
            }
            printConvexHull(graph); // Print the input points
        } else if (cmd == "CH") {
           
            convexHull(graph.points, hull);
            hull.area = polygonArea(hull); // Calculate the area of the convex hull
            std::cout << "Convex Hull Area: " << hull.area << std::endl;
            printConvexHull(hull);
        } else if (cmd == "Newpoint") {
            double x, y;
            char p;
            std::cin >> x >> p >> y;
            if (std::cin.fail() || p != ',') {
                std::cin.clear(); // Clear the error state
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignore the rest of the line
                std::cerr << "Invalid input. Please enter valid coordinates." << std::endl;
                break;
            }
            graph.points.push_back(Point{x, y});
            graph.size = graph.points.size();
            printConvexHull(graph); // Print the updated points
        } else if (cmd == "Removepoint") {
            double x, y;
            char p;
            bool validInput = false;
            std::cin >> x >> p >> y;
            if (std::cin.fail() || p != ',') {
                std::cin.clear(); // Clear the error state
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignore the rest of the line
                std::cerr << "Invalid input. Please enter valid coordinates." << std::endl;
                break;
            }
            auto it = std::remove_if(graph.points.begin(), graph.points.end(),
                [&](const Point& p) { return p.x == x && p.y == y; });
            if (it != graph.points.end()) {
                graph.points.erase(it, graph.points.end());
                graph.size = graph.points.size();
                validInput = true;
            }
            if (!validInput) {
                std::cerr << "Point (" << x << ", " << y << ") not found in the graph." << std::endl;
            }
            printConvexHull(graph); // Print the updated points
        }else if (cmd == "exit") {
            std::cout << "Exiting server." << std::endl;
            break;
        } else {
            std::cout << "Unknown command: " << cmd << std::endl;
            std::cout << "Available commands: Newgraph, CH, Newpoint, Removepoint, exit" << std::endl;
        }
    
    }
    return 0;
}
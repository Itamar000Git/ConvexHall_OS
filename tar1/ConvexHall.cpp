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

int main(int argc, char* argv[]) {
    ConvexHull points;
    ConvexHull hull;
    std::cout << "Convex Hall Algorithm Implementation" << std::endl;
    std::cout<< "Please enter the number of points: ";
    int n;
    std::cin >> n;
    if (std::cin.fail()) {
                std::cin.clear(); 
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); 
                return 1; // Exit if input is invalid
    }
    if (n <= 0) {
        std::cerr << "Number of points must be greater than zero." << std::endl;
        return 1;
    }
    
    points.points.resize(n);
    points.size = n;
    //std::cout << "Please enter the points (x y):" << std::endl;
    for (int i = 0; i < n; ++i) {
        
        points.points[i] = Point();
        points.points[i].x = 0.0;
        points.points[i].y = 0.0;
        char p;
    
        while(true) {
            std::cout << "Enter coordinates for point " << i + 1 << " (x y): ";
           
            std::cin >> points.points[i].x>> p >> points.points[i].y;

            if (std::cin.fail() || p != ',') {
                std::cin.clear(); 
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); 
                std::cout << "Invalid input. Please enter two float numbers (x y): ";
            } else {
                break;
            }
        }
    }
    printConvexHull(points); // Print the input points

    convexHull(points.points, hull); // Calculate the convex hull

    hull.area= polygonArea(hull); // Calculate the area of the convex hull
    
    std::cout << "Convex Hull Area: " << hull.area << std::endl;
    return 0;
}
#include <iostream>
#include <vector>
#include <limits>
#include <algorithm>
#include <cmath>
#include "ConvexHall.hpp"
#include <chrono>
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
void convexHull_Androw(std::vector<Point>& points, ConvexHull& hull) {
   
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
 * @brief Calculate the convex hull of a set of points using Graham's scan algorithm.
 * @param points The input points.
 * @param hull The resulting convex hull.
 */
void convexHull_Graham(std::vector<Point> &points,ConvexHull& hull){
    int index = 0;
    double minY = points[0].y;
    for (int i = 1; i < points.size(); i++) {
        if (points[i].y < minY || (points[i].y == minY && points[i].x < points[index].x)) {
            minY = points[i].y;
            index = i;
        }
    }
    std::swap(points[0], points[index]); // Move the point with the lowest y-coordinate to the front
    std::sort(points.begin() + 1, points.end(), [&](const Point& a, const Point& b) {
                double cp = cross(points[0], a, b);
                if (cp == 0) {
                    double da = (a.x - points[0].x)*(a.x - points[0].x) + (a.y - points[0].y)*(a.y - points[0].y);
                    double db = (b.x - points[0].x)*(b.x - points[0].x) + (b.y - points[0].y)*(b.y - points[0].y);
                    return da < db;
                }
                return cp > 0;
            });
    std::vector<Point> stack;
    stack.push_back(points[0]);
    stack.push_back(points[1]);
    stack.push_back(points[2]);
    for (size_t i = 3; i < points.size(); ++i) {
        while (stack.size() >= 2 && cross(stack[stack.size()-2], stack[stack.size()-1], points[i]) <= 0) {
            stack.pop_back();
        }
        stack.push_back(points[i]);
    }
    hull.points = stack;
    hull.size = stack.size();
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
    ConvexHull hull1;
    ConvexHull hull2;

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
    std::cout<<std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    convexHull_Androw(points.points, hull1); // Calculate the convex hull using Andrew's algorithm
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Convex Hull calculated using Andrew's algorithm in " << duration.count() << " seconds." << std::endl;
    std::cout<<std::endl;

    printConvexHull(hull1); // Print the convex hull points calculated by Andrew's algorithm
     std::cout<<std::endl;

    start = std::chrono::high_resolution_clock::now();
    convexHull_Graham(points.points, hull2); // Calculate the convex hull using Graham's algorithm
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "Convex Hull calculated using Graham's algorithm in " << duration.count() << " seconds." << std::endl;
     std::cout<<std::endl;

    printConvexHull(hull2); // Print the convex hull points calculated by Graham's algorithm
     std::cout<<std::endl;

    hull1.area= polygonArea(hull1); // Calculate the area of the convex hull
    hull2.area= polygonArea(hull2); // Calculate the area of the convex hull
    std::cout << "Convex Hull Area with androw: " << hull1.area << std::endl;
    std::cout << "Convex Hull Area with graham: " << hull2.area << std::endl;
    return 0;
}
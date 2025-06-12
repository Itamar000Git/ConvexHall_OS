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
void printConvexHull(const ConvexHull& hull, std::ostream& os = std::cout);
void convexHull(std::vector<Point> points, ConvexHull& hull);
double cross(const Point& p1, const Point& p2, const Point& p3);
double polygonArea(const std::vector<Point>& poly);
void readPoints(ConvexHull& graph, int n,std::istringstream& iss);
void addPoint(ConvexHull& graph, std::istringstream& iss);
void removePoint(ConvexHull& graph, std::istringstream& iss);
void handle_request(const std::string& request, int client_socket);
void* on_server_socket(int sk);
void* on_stdin(int fd);
void* on_client_socket(int client_fd);
#endif
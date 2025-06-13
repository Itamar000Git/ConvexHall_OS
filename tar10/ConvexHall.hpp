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
    bool treshhold;
} ConvexHull;

/**
 * @brief Print the points of the convex hull.
 * @param hull The convex hull containing the points.
 * @param os The output stream to print the points.
 */
void printConvexHull(const ConvexHull& hull, std::ostream& os = std::cout);

/**
 * @brief Calculate the convex hull of a set of points.
 * @param points The input points to form the convex hull.
 * @param hull The ConvexHull structure to store the result.
 */
void convexHull(std::vector<Point> points, ConvexHull& hull);

/**
 * @brief Calculate the cross product of three points.
 * @param p1 The first point.
 * @param p2 The second point.
 * @param p3 The third point.
 * @return The cross product value.
 */
double cross(const Point& p1, const Point& p2, const Point& p3);

/**
 * @brief Calculate the area of a polygon defined by the ConvexHull structure.
 * @param poly The ConvexHull structure containing the points of the polygon.
 * @return The area of the polygon.
 */
double polygonArea(const std::vector<Point>& poly);

/**
 * @brief Read points from an input stream and populate the ConvexHull structure.
 * @param graph The ConvexHull structure to populate.
 * @param n The number of points to read.
 * @param iss The input stream containing the points.
 */
void readPoints(ConvexHull& graph, int n,std::istringstream& iss);

/**
 * @brief Add a new point to the ConvexHull structure and print the updated convex hull.
 * @param graph The ConvexHull structure to update.
 * @param iss The input stream containing the new point.
 */
void addPoint(ConvexHull& graph, std::istringstream& iss);

/**
 * @brief Remove a point from the ConvexHull structure and print the updated convex hull.
 * @param graph The ConvexHull structure to update.
 * @param iss The input stream containing the point to remove.
 * @param os The output stream to print the result.
 */
void removePoint(ConvexHull& graph, std::istringstream& iss, std::ostream& os = std::cout);

/**
 * @brief Handle a request from a client or stdin.
 * @param request The request string.
 * @param client_socket The socket descriptor for the client (1 for stdin).
 * @param graph The ConvexHull structure to operate on.
 * @param hull The ConvexHull structure to store the convex hull result.
 */
void handle_request(const std::string& request, int client_socket);

/**
 * @brief Thread function to handle stdin input.
 * @param fd The file descriptor for stdin (1).
 */
void* on_stdin(int fd);

/**
 * @brief Thread function to handle client socket input.
 * @param client_fd The file descriptor for the client socket.
 * @return A pointer to the result (nullptr).
 */
void* on_client_socket(int client_fd);

void* wait_for_CH_area_change(void * tmp);
#endif
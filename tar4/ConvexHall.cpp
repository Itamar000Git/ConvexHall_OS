#include <iostream>
#include <vector>
#include <limits>
#include <algorithm>
#include <cmath>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <sstream>
#include "ConvexHall.hpp"
#define PORT 9034
#define MAX_CLIENTS 10
#define BUFSIZE 4096
bool runningServer = true;

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

    printConvexHull(hull, std::cout);
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
void printConvexHull(const ConvexHull& hull, std::ostream& os) {
    os << "Convex Hull Points:\n";
    for (const auto& point : hull.points) {
        os << "(" << point.x << ", " << point.y << ")\n";
    }
}

void readPoints(ConvexHull& graph, int n, std::istringstream& iss) {
    graph.points.clear();
    graph.size = n;
    for (int i = 0; i < n; ++i) {
    //std::cout << "Enter coordinates for point " << i + 1 << " (x y): ";
        double x, y;
        char comma;
        iss >> x >> comma >> y;
        graph.points.push_back(Point{x, y});
    }
    graph.size = graph.points.size();
    printConvexHull(graph, std::cout);
}

void addPoint(ConvexHull& graph, std::istringstream& iss) {
    double x, y;
    char comma;
    iss >> x >> comma >> y;
    graph.points.push_back(Point{x, y});
    graph.size = graph.points.size();
    printConvexHull(graph, std::cout);
}

void removePoint(ConvexHull& graph, std::istringstream& iss) {
    double x, y;
    char comma;
    bool validInput = false;
    iss >> x >> comma >> y;
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
    printConvexHull(graph, std::cout);
}
void handle_request(const std::string& request, int client_socket, ConvexHull& graph,ConvexHull& hull) {
    std::istringstream iss(request);
    std::ostringstream response;
    std::string cmd;
    iss >> cmd;
    if (cmd == "Newgraph") {
        int n;
        iss >> n;
        //ConvexHull graph;
        readPoints(graph, n, iss);
        printConvexHull(graph, response);
    } else if (cmd == "CH") {
        //ConvexHull hull;
        convexHull(graph.points, hull);
        hull.area = polygonArea(hull);
        response << "Convex Hull Area: " << hull.area << std::endl;
        printConvexHull(hull, response);
    } else if (cmd == "Newpoint") {
        addPoint(graph, iss);
        printConvexHull(graph, response);
    } else if (cmd   == "Removepoint") {
        removePoint(graph, iss);
        printConvexHull(graph, response); 
    }else if (cmd == "exit") {
        response << "Exiting." << std::endl;
        std::string out = response.str();
        send(client_socket, out.c_str(), out.size(), 0);
        if (client_socket == 1) { 
            runningServer = false; // Set the server to stop running
            std::cout << "Server is shutting down." << std::endl;
        }
        return; 
    }else {
        response << "Unknown command: " << request << std::endl;
        response << "Available commands: Newgraph, CH, Newpoint, Removepoint, exit" << std::endl;
    }
       std::string out = response.str();
    if (client_socket == 1) {
        std::cout << out;
    } else {
        send(client_socket, out.c_str(), out.size(), 0);
    }
}

int main() { 
    ConvexHull graph;
    ConvexHull hull;
    std::string cmd;
    runningServer = true;
    std::cout << "Convex Hull Algorithm Implementation" << std::endl;
    std::cout << "Available commands: Newgraph, CH, Newpoint, Removepoint , exit" << std::endl;
    int sk, newfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t addrlen;
    char buf[BUFSIZE];
    int fdmax;
    fd_set master, read_fds;

    sk= socket(AF_INET, SOCK_STREAM, 0);
    if (sk < 0) {
        std::cerr << "Error creating socket." << std::endl;
        return 1;
    }
    int yes = 1;
    setsockopt(sk, SOL_SOCKET, SO_REUSEADDR, &yes , sizeof(int));

    serv_addr.sin_family = AF_INET; // IPv4
    serv_addr.sin_addr.s_addr = INADDR_ANY; // Accept connections from any IP address
    serv_addr.sin_port = htons(PORT); // Port number

    memset(&(serv_addr.sin_zero), '\0', 8); // Zero out the rest of the struct
    if (bind(sk, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { // Bind the socket to the address and port
        std::cerr << "Error binding socket." << std::endl;
        return 1;
    }
    if( listen(sk, MAX_CLIENTS) < 0) { // Listen for incoming connections
        std::cerr << "Error listening on socket." << std::endl;
        return 1;
    }

    FD_ZERO(&master); // Initialize the master set
    FD_SET(sk, &master); // Add the listening socket to the master set
    FD_SET(0, &master);
    fdmax = std::max(sk,0); // Set the maximum file descriptor
    std::cout << "Server started on port " << PORT << std::endl;

    while (runningServer) {
        read_fds = master; // Copy the master set to read_fds
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) < 0) {
            std::cerr << "Error in select." << std::endl;
            return 1;
        }

        for (int i = 0; i <= fdmax; ++i) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == sk) {
                    // New client connection
                    addrlen = sizeof(cli_addr);
                    newfd = accept(sk, (struct sockaddr *)&cli_addr, &addrlen);
                    if (newfd < 0) {
                        std::cerr << "Error accepting connection." << std::endl;
                    } else {
                        FD_SET(newfd, &master);
                        if (newfd > fdmax) fdmax = newfd;
                        std::cout << "New connection from " << inet_ntoa(cli_addr.sin_addr)
                                << ":" << ntohs(cli_addr.sin_port) << std::endl;
                    }
                } else if (i == 0) {
                    // Terminal input 
                    std::string line;
                    if (std::getline(std::cin, line)) {
                        handle_request(line, 1, graph,hull); // 1 = stdout
                    } else {
                        std::cerr << "Error or EOF on stdin." << std::endl;
                    }
                } else {
                    // Data from client
                    int nbytes = recv(i, buf, sizeof(buf) - 1, 0);
                    if (nbytes <= 0) {
                        if (nbytes == 0) {
                            std::cout << "Connection closed by client." << std::endl;
                        } else {
                            std::cerr << "Error receiving data." << std::endl;
                        }
                        close(i);
                        FD_CLR(i, &master);
                    } else {
                        buf[nbytes] = '\0';
                        std::istringstream lines(buf);
                        std::string line;
                        while (std::getline(lines, line)) {
                            if (!line.empty() && line.back() == '\r') line.pop_back();
                            handle_request(line, i, graph,hull);
                        }
                    }
                }
            }
        }
    }
    close(sk); // Close the listening socket
    return 0;
}
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
#include "../tar5_8/ReactorProactor.hpp"
#define PORT 9034
#define MAX_CLIENTS 10
#define BUFSIZE 4096
Reactor* reactor_ptr = nullptr;
ConvexHull graph;
ConvexHull hull;

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
    graph.area = 0.0;
    hull.points.clear();
    hull.area = 0.0;
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
void handle_request(const std::string& request, int client_socket, ConvexHull& graph, ConvexHull& hull) {
    std::istringstream iss(request);
    std::ostringstream response;
    std::string cmd;
    iss >> cmd;
    if (cmd == "Newgraph") {
        int n;
        iss >> n;
        readPoints(graph, n, iss);
        printConvexHull(graph, response);
    } else if (cmd == "CH") {
        convexHull(graph.points, hull);
        hull.area = polygonArea(hull);
        response << "Convex Hull Area: " << hull.area << std::endl;
        printConvexHull(hull, response);
    } else if (cmd == "Newpoint") {
        addPoint(graph, iss);
        printConvexHull(graph, response);
    } else if (cmd == "Removepoint") {
        removePoint(graph, iss);
        printConvexHull(graph, response); 
    } else if (cmd == "exit") {
        response << "Exiting server." << std::endl;
        if (client_socket == 1) { // If the request is from stdin (client_socket == 1), stop the reactor
        std::cout << "Stopping reactor..." << std::endl;
        reactor_ptr->stopReactor(); 
        }else{ 
        reactor_ptr->pushFdToRemove(client_socket); // Mark the client socket for removal
        }
    } else {
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
 
/**
  * @brief Handle incoming connections on the server socket.
  * @param sk The server socket file descriptor.
  * @return A pointer to void (not used).
  */
void* on_server_socket(int sk) {
    struct sockaddr_in cli_addr;
    socklen_t addrlen = sizeof(cli_addr);
    int newfd = accept(sk, (struct sockaddr *)&cli_addr, &addrlen); // Accept a new connection
    if (newfd < 0) {
        std::cerr << "Error accepting connection." << std::endl;
    } else {
        std::cout << "New connection from " << inet_ntoa(cli_addr.sin_addr) 
                  << ":" << ntohs(cli_addr.sin_port) << std::endl;
        reactor_ptr->addFdToReactor(newfd, on_client_socket);// Add the new client socket to the reactor
    }
    return nullptr;
}
/**
 * @brief Handle input from stdin (terminal).
 * @param fd The file descriptor for stdin.
 * @return A pointer to void (not used).
 */
void* on_stdin(int fd) {
    std::string line;
    if (std::getline(std::cin, line)) {
        handle_request(line, 1, graph, hull); // 1 = stdout
    } else {
        std::cerr << "Error or EOF on stdin." << std::endl;
    }
    return nullptr;
}
/**
 * @brief Handle client socket events.
 * @param client_fd The file descriptor of the client socket.
 * @return A pointer to void (not used).
 */
void* on_client_socket(int client_fd) {
    char buf[BUFSIZE];
    int nbytes = recv(client_fd, buf, sizeof(buf) - 1, 0);
    if (nbytes <= 0) { // Check for errors or connection closure
        if (nbytes == 0) {
            std::cout << "Connection closed by client." << std::endl;
        } else {
            std::cerr << "Error receiving data." << std::endl;
        }
        close(client_fd);
        reactor_ptr->pushFdToRemove(client_fd); // Remove the client socket from the reactor
    } else {
        buf[nbytes] = '\0';
        std::istringstream lines(buf);
        std::string line;
        while (std::getline(lines, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            handle_request(line, client_fd, graph, hull);// Process the request from the client
        }
    }
    return nullptr;
}


int main() {
    Reactor reactor;
    reactor_ptr = &reactor;

    int sk = socket(AF_INET, SOCK_STREAM, 0);
    if (sk < 0) {
        std::cerr << "Error creating socket." << std::endl;
        return 1;
    }
    int yes = 1;
    setsockopt(sk, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);
    memset(&(serv_addr.sin_zero), '\0', 8);

    if (bind(sk, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Error binding socket." << std::endl;
        return 1;
    }
    if (listen(sk, MAX_CLIENTS) < 0) {
        std::cerr << "Error listening on socket." << std::endl;
        return 1;
    }

    reactor.addFdToReactor(sk, on_server_socket); // Add the server socket to the reactor
    reactor.addFdToReactor(0, on_stdin);         // Add stdin to the reactor for terminal input

    std::cout << "Server started on port " << PORT << std::endl;
    std::cout << "Convex Hull Algorithm Implementation" << std::endl;
    std::cout << "Available commands: Newgraph, CH, Newpoint, Removepoint , exit" << std::endl;
    reactor.startReactor(); // Start the reactor event loop

    close(sk);
    return 0;
}
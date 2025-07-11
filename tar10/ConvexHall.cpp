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
#include <condition_variable>
#include "ConvexHall.hpp"
#include "../tar5_8/ReactorProactor.hpp"
#include <set>

#define PORT 9034
#define MAX_CLIENTS 10
#define BUFSIZE 4096

pthread_cond_t cond = PTHREAD_COND_INITIALIZER; 
pthread_mutex_t area_mutex = PTHREAD_MUTEX_INITIALIZER;

ConvexHull graph;
ConvexHull hull;
std::mutex graph_mutex;
bool runningServer = true;
bool area_updated=false;
std::set<int> client_sockets;

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

double cross(const Point& p1, const Point& p2, const Point& p3) {
    return (p2.x - p1.x)*(p3.y - p1.y) - (p2.y - p1.y)*(p3.x - p1.x);
}

void printConvexHull(const ConvexHull& hull, std::ostream& os) {
    os << "Convex Hull Points:\n";
    for (const auto& point : hull.points) {
        os << "(" << point.x << ", " << point.y << ")\n";
    }
}

void readPoints(ConvexHull& graph, int n, std::istringstream& iss) {
    graph.points.clear();
    graph.area = 0.0;
    hull.treshhold=false;
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

void removePoint(ConvexHull& graph, std::istringstream& iss,std::ostream& os) {
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
        os << "Point (" << x << ", " << y << ") not found in the graph." << std::endl;
    }
    printConvexHull(graph, std::cout);
}

void handle_request(const std::string& request, int client_socket, ConvexHull& graph, ConvexHull& hull) {
    std::istringstream iss(request);
    std::ostringstream response;
    std::string cmd;
    iss >> cmd;
    if (cmd == "Newgraph") {
        std::lock_guard<std::mutex> lock(graph_mutex); // Lock the mutex to ensure thread safety
        int n;
        iss >> n;
        readPoints(graph, n, iss);
        printConvexHull(graph, response);
    } else if (cmd == "CH") {
        std::lock_guard<std::mutex> lock(graph_mutex);
        convexHull(graph.points, hull);
        hull.area = polygonArea(hull);
        response << "Convex Hull Area: " << hull.area << std::endl;
        printConvexHull(hull, response);


        pthread_mutex_lock(&area_mutex);
        area_updated=true;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&area_mutex);
        
    } else if (cmd == "Newpoint") {
        std::lock_guard<std::mutex> lock(graph_mutex);
        addPoint(graph, iss);
        printConvexHull(graph, response);
    } else if (cmd == "Removepoint") {
        std::lock_guard<std::mutex> lock(graph_mutex);
        removePoint(graph, iss, response);
        printConvexHull(graph, response); 
    } else if (cmd == "exit") {
        response << "Exiting server." << std::endl;
        if (client_socket == 1) { // If the request is from stdin (client_socket == 1), stop the reactor
            std::cout << "Stopping reactor..." << std::endl;
            runningServer= false; // Set the running flag to false
            close_all_clients(client_sockets);
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

void* on_stdin(void* fd) {
    std::string line;
    //client_sockets.insert(1); // Add the stdin socket to the set of client sockets
    while (runningServer) {
        if (std::getline(std::cin, line)) {// Read a line from stdin
            handle_request(line, 1, graph, hull); // Handle the request from stdin
            if (line == "exit") break; // If the command is "exit", break the loop
        } else {
            std::cerr << "Error or EOF on stdin." << std::endl;
        }
    }

    return nullptr;
}

void* on_client_socket(void* tmp) {
    int client_fd = (intptr_t)tmp;
    char buf[BUFSIZE];

    client_sockets.insert(client_fd); // Add the client socket to the set of client sockets
    while (runningServer) {
        int nbytes = recv(client_fd, buf, sizeof(buf) - 1, 0);// Receive data from the client
        if (nbytes <= 0) {
            if (nbytes == 0) {
                std::cout << "Connection closed by client." << std::endl;
            } else {
                std::cerr << "Error receiving data." << std::endl;
            }
            close(client_fd);
            break;
        } else {
            buf[nbytes] = '\0';
            std::istringstream lines(buf);
            std::string line;
            while (std::getline(lines, line)) {// Read lines from the received data
                if (!line.empty() && line.back() == '\r') line.pop_back();
                handle_request(line, client_fd, graph, hull);// Handle the request from the client
            }
        }
    }
    return nullptr;
}

void* wait_for_CH_area_change(void * tmp){
     while (runningServer) {
       pthread_mutex_lock(&area_mutex); // Lock the mutex to protect the area_updated flag
        while (!area_updated) { // Wait for the area to be updated
            pthread_cond_wait(&cond, &area_mutex); // Wait for the condition variable to be signaled
        }
        if(hull.treshhold==false && hull.area >=100 ){ 
            hull.treshhold=true;
            std::cout << "Convex Hull Area has reached the threshold of 100. Area: " << hull.area << std::endl;
            std::cout << "Convex Hull Points:\n";
           
        } else if(hull.treshhold==true && hull.area < 100){
            hull.treshhold=false;
            std::cout << "Convex Hull Area is below the threshold of 100. Area: " << hull.area << std::endl;
        }

          area_updated = false;
          pthread_mutex_unlock(&area_mutex);// Unlock the mutex after processing the area update
    }

  
    return nullptr;
}

void close_all_clients(std::set<int>& client_sockets) {
    for (int fd : client_sockets) { // Close all client sockets
        shutdown(fd, SHUT_RDWR); 
        close(fd);
    }
    client_sockets.clear();
}

int main() {
    int sk = socket(AF_INET, SOCK_STREAM, 0);// Create a socket
    if (sk < 0) {
        std::cerr << "Error creating socket." << std::endl;
        return 1;
    }
    int yes = 1;
    setsockopt(sk, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));// Set socket options to reuse the address

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);
    memset(&(serv_addr.sin_zero), '\0', 8);// Initialize the address structure

    if (bind(sk, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Error binding socket." << std::endl;
        return 1;
    }
    if (listen(sk, MAX_CLIENTS) < 0) {
        std::cerr << "Error listening on socket." << std::endl;
        return 1;
    }

    hull.treshhold=false;
    Proactor proactor;// Create a Proactor instance

    pthread_t cond_var_t;
    pthread_create(&cond_var_t, nullptr, wait_for_CH_area_change, nullptr);

    pthread_t server_thread = proactor.startProactor(sk, on_client_socket);// Start the Proactor with the server socket and the client handler function
    pthread_t stdin_thread;// Create a thread for handling stdin input
    pthread_create(&stdin_thread, nullptr, on_stdin, nullptr);// Start the stdin thread

    std::cout << "Server started on port " << PORT << std::endl;
    std::cout << "Convex Hull Algorithm Implementation" << std::endl;
    std::cout << "Available commands: Newgraph, CH, Newpoint, Removepoint , exit" << std::endl;

    while (runningServer) {// Main loop to keep the server running
        sleep(1);
    }
    pthread_mutex_lock(&area_mutex);
    pthread_cond_signal(&cond);// Signal the condition variable to wake up the waiting thread
    pthread_mutex_unlock(&area_mutex);


    proactor.stopProactor(server_thread);// Stop the Proactor and wait for the server thread to finish
    pthread_join(stdin_thread, nullptr);// Wait for the stdin thread to finish
    pthread_join(server_thread, nullptr);// Wait for the server thread to finish
   
    
    
    close(sk);// Close the server socket
    return 0;
}

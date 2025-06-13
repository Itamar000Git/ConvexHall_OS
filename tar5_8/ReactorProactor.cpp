#include "ReactorProactor.hpp"
#include <sys/select.h>
#include <unistd.h>
#include <iostream>
#include <atomic>
#include <netinet/in.h>
#include <string.h>


    Reactor::Reactor():running(false){
    }
    Reactor::~Reactor(){
        stopReactor();
    }

    //Stops the reactor and clears the file descriptor map.
    int Reactor::stopReactor(){
        std::lock_guard<std::mutex> lock(fd_mutex);
        if (running) {
            running = false; // Set the reactor to not running
            //fdMap.clear(); // Clear the file descriptor map
            std::cout << "Reactor stopped." << std::endl;
            return 0; // Return 0 on success
        } else {
            std::cout << "Reactor is already stopped." << std::endl;
            return -1; // Return -1 if the reactor was not running
        }
    }


   
// This function initializes the reactor and starts its event loop.
void Reactor::startReactor() {
    {
        std::lock_guard<std::mutex> lock(fd_mutex);// Lock the mutex to ensure thread safety
        if (!running) {
            running = true;
            std::cout << "Reactor started." << std::endl;
        }
    }
    while (true) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        int max_fd = 0;

        {
            std::lock_guard<std::mutex> lock(fd_mutex);// Lock the mutex to ensure thread safety
            for (const auto& pair : fdMap) {// Iterate over the file descriptor map
                FD_SET(pair.first, &read_fds);// Add the file descriptor to the set of file descriptors to monitor
                if (pair.first > max_fd) max_fd = pair.first;
            }
            if (!running) break;// If the reactor is not running, exit the loop
        }
        int activity = select(max_fd + 1, &read_fds, nullptr, nullptr, nullptr);
        if (activity < 0) {
            std::cerr << "Error in select." << std::endl;
            break;
        }
        std::vector<std::pair<int, reactorFunc>> ready_fds;// Vector to hold file descriptors that are ready for reading
        {
            std::lock_guard<std::mutex> lock(fd_mutex);
            for (const auto& pair : fdMap) {
                if (FD_ISSET(pair.first, &read_fds)) {
                    ready_fds.push_back(pair); // Store the ready file descriptors and their associated functions
                }
            }
        }
        for (const auto& pair : ready_fds) { // Iterate over the ready file descriptors
            pair.second(pair.first);// Call the associated function with the file descriptor
        }

        processRemovals();
    }
    std::lock_guard<std::mutex> lock(fd_mutex);
    fdMap.clear();
}

        // // adds fd to Reactor (for reading) ; returns 0 on success. 
    int  Reactor::addFdToReactor( int fd, reactorFunc func){
        std::lock_guard<std::mutex> lock(fd_mutex); // Lock the mutex to ensure thread safety
        fdMap[fd] = func; // Associate the fd with the function
        if (!running) {
            running = true; // Start the reactor if it wasn't running
            std::cout << "Reactor started." << std::endl;
        }
        std::cout << "Added fd " << fd << " to reactor." << std::endl;
        return 0; // Return 0 on success
    }

    void Reactor::processRemovals() {
        std::lock_guard<std::mutex> lock(fd_mutex);
        for (int fd : fds_to_remove) {
            fdMap.erase(fd); // מחיקה ישירה, בלי לקרוא לפונקציה שנועלת שוב!
            
            std::cout << "Removed fd " << fd << " from reactor." << std::endl;
        }
        fds_to_remove.clear();
    }
   



    Proactor::Proactor():args(nullptr){
    }
    Proactor::~Proactor(){

    }


pthread_t Proactor::startProactor (int sk, proactorFunc func){
   args = new ProactorArgs{sk, func, new std::atomic<bool>(true)};// Create a new ProactorArgs instance with the provided socket and function
    pthread_t tid;
    pthread_create(&tid, nullptr, proactor_main_loop, args);// Create a new thread that runs the proactor main loop
    return tid;
}   


int Proactor::stopProactor(pthread_t tid) {
   if (args && args->running) {// Check if args is not null and running is true
        *(args->running) = false;// Set running to false to signal the proactor thread to stop
        
        int sock = socket(AF_INET, SOCK_STREAM, 0);// Create a socket to wake up the proactor thread
        if (sock >= 0) {
            sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(9034);
            addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            memset(&(addr.sin_zero), '\0', 8);
            connect(sock, (struct sockaddr*)&addr, sizeof(addr));// Connect to the socket to wake up the proactor thread
            close(sock);
        }
    }
    pthread_join(tid, nullptr);// Wait for the proactor thread to finish
    args = nullptr;
    return 0;

}

void* proactor_main_loop(void* arg) {
    ProactorArgs* args = static_cast<ProactorArgs*>(arg);// Cast the argument to ProactorArgs
    int sk = args->sk;
    proactorFunc func = args->func;
    std::atomic<bool>* running = args->running;

    while (*running) {// Main loop of the proactor
        sockaddr_in cli_addr;
        socklen_t addrlen = sizeof(cli_addr);
        int newfd = accept(sk, (struct sockaddr*)&cli_addr, &addrlen);// Accept a new connection on the socket
        if (newfd < 0) {
            if (*running)
                perror("accept");
            continue;
        }
        std::thread([func, newfd]() {// Create a new thread to handle the accepted connection
            func((void*)(intptr_t)newfd);
            close(newfd);
        }).detach();
    }
    delete running;
    delete args;
    return nullptr;
}

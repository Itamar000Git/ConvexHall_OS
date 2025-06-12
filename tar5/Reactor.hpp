#ifndef REACTOR_HPP
#define REACTOR_HPP
#include <iostream>
#include <vector>
#include <map>
#include <mutex>

typedef void *(*reactorFunc) (int fd); // Define a function pointer type for the reactor functions that take an int fd and return void*

class Reactor{
    private:
        std::map<int, reactorFunc> fdMap;// Map to hold file descriptors and their associated functions
        bool running;
        std::vector<int> fds_to_remove;
        std::mutex fd_mutex;
    public:
        Reactor();
        ~Reactor();
        // This function initializes the reactor and starts its event loop.
        void startReactor ();  
        // adds fd to Reactor (for reading) ; returns 0 on success. 
        int  addFdToReactor( int fd, reactorFunc func);
        // removes fd from reactor 
       // int removeFdFromReactor( int fd);  
        // stops reactor 
        int stopReactor(); 
        // Adds a file descriptor to the removal list
        void pushFdToRemove(int fd) {
            std::lock_guard<std::mutex> lock(fd_mutex); // Lock the mutex to ensure thread safety
            fds_to_remove.push_back(fd); // Add the fd to the removal list
        }
        // Processes all fds in the removal list
        void processRemovals();
};



#endif // REACTOR_HPP
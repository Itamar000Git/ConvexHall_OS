#include "Reactor.hpp"
#include <sys/select.h>
#include <unistd.h>
#include <iostream>


    Reactor::Reactor():running(false){
    }
    Reactor::~Reactor(){
        stopReactor();
    }

    //Stops the reactor and clears the file descriptor map.
    int Reactor::stopReactor(){
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


        // // This function initializes the reactor and starts its event loop.
    void Reactor::startReactor (){
        if (!running) {
            running = true; // Set the reactor to running state
            std::cout << "Reactor started." << std::endl;
        } 
        // else {
        //     std::cout << "Reactor is already running." << std::endl;
        // }
        // Event loop (simplified for demonstration)
        while (running) {
            fd_set read_fds;
            FD_ZERO(&read_fds);
            // Add all fds to the read set
            for (const auto& pair : fdMap) { 
                FD_SET(pair.first, &read_fds);// Add the file descriptor to the set
            }
            int max_fd = 0;
            for (const auto& pair : fdMap) {
                if (pair.first > max_fd) {
                    max_fd = pair.first; // Find the maximum file descriptor
                }
            }
            // Wait for events
            int activity = select(max_fd + 1, &read_fds, nullptr, nullptr, nullptr); // Wait for activity on the file descriptors
            if (activity < 0) {
                std::cerr << "Error in select." << std::endl;
                break; // Exit on error
            }
            // Handle events
            for (const auto& pair : fdMap) {
                if (FD_ISSET(pair.first, &read_fds)) {
                    pair.second(pair.first); // Call the associated function
                }
            }

         processRemovals();// Process any removals after handling events
            
        }
         fdMap.clear(); // Clear the file descriptor map
        return; // Return when the reactor stops
        
    }  

        // // adds fd to Reactor (for reading) ; returns 0 on success. 
    int  Reactor::addFdToReactor( int fd, reactorFunc func){
        fdMap[fd] = func; // Associate the fd with the function
        if (!running) {
            running = true; // Start the reactor if it wasn't running
            std::cout << "Reactor started." << std::endl;
        }
        std::cout << "Added fd " << fd << " to reactor." << std::endl;
        return 0; // Return 0 on success
    }

        
        // // removes fd from reactor 
     int Reactor::removeFdFromReactor( int fd){
        auto it = fdMap.find(fd);
        if (it != fdMap.end()) {
            fdMap.erase(it); // Remove the fd from the map
            std::cout << "Removed fd " << fd << " from reactor." << std::endl;
            if (fdMap.empty()) {
                running = false; // Stop the reactor if no fds are left
                std::cout << "Reactor stopped." << std::endl;
            }
            return 0; // Return 0 on success
        } else {
            std::cerr << "Error: fd " << fd << " not found in reactor." << std::endl;
            return -1; // Return -1 if the fd was not found
        }

     }  
   


  
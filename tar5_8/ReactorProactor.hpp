#ifndef REACTOR_PROACTOR_HPP
#define REACTOR_PROACTOR_HPP
#include <iostream>
#include <vector>
#include <map>
#include <mutex>
#include <thread>
#include <atomic>

typedef void *(*reactorFunc) (int fd); // Define a function pointer type for the reactor functions that take an int fd and return void*
class Reactor {
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


// Proactor class definition   

typedef void * (* proactorFunc) (void* sockfd); // Define a function pointer type for the proactor functions that take an int sockfd and return void*

struct ProactorArgs {
    int sk;
    proactorFunc func;
    std::atomic<bool>* running;
};
class Proactor {
    private:
        ProactorArgs* args; // Pointer to ProactorArgs containing the socket and function pointer
    public:
        Proactor();
        ~Proactor();
        
        /**
         * @brief Starts the proactor with the given socket and function.
         * @param sk The socket file descriptor to be used by the proactor.
         * @param func The function to be executed by the proactor.
         * @return A pthread_t representing the thread running the proactor.
         * This function creates a new thread that runs the proactor main loop.
         * The thread will execute the function provided with the socket file descriptor.
         */
        pthread_t startProactor (int sk, proactorFunc func);  
        /**
         * @brief Stops the proactor by terminating the thread.
         * @param tid The thread ID of the proactor to be stopped.
         * @return An integer indicating success (0) or failure (non-zero).
         * This function signals the proactor thread to stop and waits for it to finish.
         */
        int stopProactor(pthread_t tid); 
};
/**
 * @brief The main loop of the proactor.
 * @param arg A pointer to the ProactorArgs structure containing the socket and function pointer.
 * @return A pointer to the result (nullptr).
 * This function runs in a separate thread and continuously checks for events on the socket.
 * It calls the associated function for each event and processes removals.
 */
void* proactor_main_loop(void* arg) ;

#endif // REACTOR_PROACTOR_HPP
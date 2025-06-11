#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define PORT 9034
#define BUFSIZE 4096
int main(){
    
    int sockfd;
    struct sockaddr_in serv_addr;
    char buf[BUFSIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error creating socket." << std::endl;
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Change if server is remote
    memset(&(serv_addr.sin_zero), '\0', 8);

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Error connecting to server." << std::endl;
        close(sockfd);
        return 1;
    }

    std::cout << "Connected to server on port " << PORT << std::endl;

    std::cout << "Available commands:" << std::endl;
    std::cout << "  Newgraph n x1,y1 x2,y2 ... xn,yn" << std::endl;
    std::cout << "  CH" << std::endl;
    std::cout << "  Newpoint x,y" << std::endl;
    std::cout << "  Removepoint x,y" << std::endl;
    std::cout << "  exit" << std::endl;

    while (true) {
        std::string cmd;
        std::cout << "> ";
        if (!std::getline(std::cin, cmd)) break;

        if (send(sockfd, cmd.c_str(), cmd.size(), 0) < 0) {
            std::cerr << "Error sending command." << std::endl;
            break;
        }
        if (cmd == "exit") break;
        
        int nbytes = recv(sockfd, buf, BUFSIZE - 1, 0);
        if (nbytes <= 0) {
            std::cerr << "Server closed connection or error." << std::endl;
            break;
        }
        buf[nbytes] = '\0';
        std::cout << buf;
    }

    close(sockfd);
    return 0;

}
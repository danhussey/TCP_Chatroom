//
//  tcpChatServer.cpp
//  maticianChallenge
//
//  Created by Daniel Hussey on 8/9/20.
//  Copyright © 2020 Daniel Husey. All rights reserved.
//

#include "tcpChatServer.hpp"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <sstream>
#include <map>
#include <vector>

#define PORT 1234
#define BUFFER_SZ 200
#define MAX_CONNECTIONS 3

int main (int argc, char const *argv[])
{
    int server_fd = 0, new_socket_fd = 0;
    const std::pair<std::string, std::string> unsetPair = std::make_pair("Unset", "Unset");
    std::map<int, std::pair<std::string, std::string>> nicknames;
    ssize_t read_length;
    struct sockaddr_in address; // Socket descriptor structure
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SZ];
    std::string welcomeMsg = "Connected to chat server.\n";
    
    // Create socket FD
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Error creating socket file descriptor.");
        exit(EXIT_FAILURE);
    }
    
//    // Attach the attach socket to the port
//    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
//    {
//        perror("Error setting socket options.\n");
//        exit(EXIT_FAILURE);
//
//    }
    
    // Configure TCP connection settings
    address.sin_family = AF_INET; // Internet domain
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT); // Get port number and address into corrct networking format
    
    // Bind socket to the port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Error binding socket to the port.\n");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 3) < 0)
    {
        perror("Error listening for conections.\n");
        exit(EXIT_FAILURE);
    }
    
    printf("Listening for connections.\n");
    
    // Master set, needed because SELECT only returns FDs of sockets that are currently interacting with the server
    fd_set socket_fds;
    int client_sockets[MAX_CONNECTIONS] = {0};
    
     // Flag for closing the server
     bool run = true;
     while (run) {
         // Clear socket set
         FD_ZERO(&socket_fds);
         
         // Add master listening socket to set
         FD_SET(server_fd, &socket_fds);
        int fdmax = server_fd;  // Needed for unix implementation of select.
         
         // Add client sockets to the fd_set
         for (int i = 0; i < MAX_CONNECTIONS; i++) {
             int sd = client_sockets[i];
             if (sd > 0) {
                 // Add a valid socket descriptor to the fd_set
                 FD_SET(sd, &socket_fds);
             }
             
             // Update max fd (needed for select plumbing)
             if (new_socket_fd > fdmax) fdmax = sd;
         }
         
         // Wait for activity on a socket
         if (int ready_sockets = select(fdmax + 1, &socket_fds, nullptr, nullptr, nullptr) < 0) {
             perror("Error performing select on socket file descriptors.\n");
             exit(EXIT_FAILURE);
         };
         
         // If the listening server fd is set, then we accept a new client who is trying to connect
         if (FD_ISSET(server_fd, &socket_fds))
         {
//             int new_socket_fd = 0;
             if ((new_socket_fd = accept(server_fd, (struct sockaddr *) &address, (socklen_t*)&addrlen)) <0)
             {
                 perror("Errror accepting new connection.\n");
                 exit(EXIT_FAILURE);
             }
             
             // Record socket number of new connection
             printf("New conn, socket fd is %d, ip is %s, port %d \n", new_socket_fd, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
             
             // Send a greeting message here
             //send new connection greeting message
             if( send(new_socket_fd, welcomeMsg.c_str(), strlen(welcomeMsg.c_str()), 0) != strlen(welcomeMsg.c_str()) )
             {
                 perror("Error sending welcome message to new client.\n");
             }
              
             // Add new socket to list of sockets
             for (int i = 0; i < MAX_CONNECTIONS; i++) {
                 // If fd is empty
                 if (client_sockets[i] == 0) {
                     client_sockets[i] = new_socket_fd;
                     printf("Added client to list of sockets at position %d.\n", i);
                     
                     // Add his fd to the nickname list but indicate that they haven't set one yet
                     nicknames[new_socket_fd] = std::make_pair("Unset", "Unset");
                     break;
                 }
             }
         }
             
         // Check the client list fd's if they're set in the f_set (ready for some IO)
         for (int i = 0; i < MAX_CONNECTIONS; i++) {
             int temp_fd = client_sockets[i];
             if (FD_ISSET(temp_fd, &socket_fds)) {
                 // Check if connection was closed
                 if ((read_length = read(temp_fd, buffer, BUFFER_SZ)) == 0) {
                     // Someone disconnected
                     getpeername(temp_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
                     printf("Host disconnected, ip %s, port %d, \n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                     
                     // Close socket and delete it from client fd list
                     close(temp_fd);
                     client_sockets[i] = 0;
                 }
                 
                 // Handle the connection
                else
                {
                    // Make sure buffer is null byte terminated and format into string
                    buffer[read_length] = '\0';
                    
                    // Check if it's from a user without a username
                    if (nicknames[temp_fd] ==  unsetPair) {
                        // The command must be a join command otherwise send error and disconnect
                        
                        // Sanitise command
                        std::stringstream ss;
                        ss << buffer;
                        std::string token;
                        std::vector<std::string> command;
                        
                        std::string endingStrippedCommand;
                        
                        while (std::getline(ss, token)) {
                            endingStrippedCommand = token;
                            
                        }
                        ss.clear();
                        ss << endingStrippedCommand;
                        
                        while (std::getline(ss, token, ' ')) {
                            command.push_back(token);
                        }
                        std::string finalWord = std::string(*(command.end()-1));
                        finalWord.erase(finalWord.length()-1);
                        *(command.end()-1) = finalWord;
                        
                        
                        
                        if (command.size() != 3) {
                            // Send error message
                            std::string error = "ERROR\n";
                            send(temp_fd, error.c_str(), error.size()+1, 0);
                            close(temp_fd);
                            client_sockets[i] = 0;
                        }
                        else {
                            
                            if (command[0] == "join") {
                                // Assign nickname and chatroom to the fd key of the client in the array
                                nicknames[temp_fd] = std::make_pair(command[1], command[2]);
                            }
                        }
                    }
                    
                    // If the client has an associated chatroom and nickname
                    else {
                        
                        // Construct message body ready to be sent
                        std::ostringstream oss;
                        
                        oss << nicknames[temp_fd].second << ": " << buffer << "\n";
                        // Probably check message body length here
                        std::string message_body = oss.str();
                        oss.clear();
                        
                        for (int j = 0; j < MAX_CONNECTIONS; j++) {
                            
                            int temp_fd_2 = client_sockets[j];
                            if (FD_ISSET(temp_fd_2, &socket_fds) || temp_fd_2 > 0 ) {
                                if (nicknames[temp_fd_2].first == nicknames[temp_fd].first) {
                                    // Send message to connected clients in the same chatroom
                                    send(temp_fd_2, message_body.c_str(), message_body.size() + 1 , 0 );
                                }
                            }
                        }
                        
                        // Null out the buffer
                        memset(buffer, '\0', strlen(buffer));
                    }
                   
                }
             }
         }
     
         
         
//
//
//
//        FD_ZERO(&socket_fds);
//
//        // Add listening socket to the FD set
//        FD_SET(server_fd, socket_fds);
//
//
//
//
//        // Make copy of socket fds
//        fd_set socket_fds_copy = socket_fds;
//
//        // Extract the first connection request on the queue of pending connection, create a new socket with the same properties of the parameter socket, and allocated a new file descriptor for the socket
//        // Blocks if the queue is empty
//        if ((new_socket = accept(server_fd, (struct sockaddr *) &address, (socklen_t*)&addrlen)) <0)
//        {
//            perror("Errror accepting connection.\n");
//            exit(EXIT_FAILURE);
//        }
//
//        // Update max file descriptor value for select functionality
//
//        if (new_socket == server_fd)
//
//        FD_SET(new_socket, &socket_fds_copy);
//
//
//
//        for (int i = 0; i < fdmax; i++) {
//
//            // Initialise file descriptor array for synchronous socket IO
//            int ready_sockets = select(fdmax + 1, &socket_fds_copy, nullptr, nullptr, nullptr);
//
//            // If we find a socket ready for IO
//            if (FD_ISSET(i, &socket_fds_copy) {
//                 if (new_socket == server_fd)
//
//            }
//            int nextSocket = socket_fds_copy;
//        }
//
//
//
//
//
//        read_length = read(new_socket, buffer, BUFFER_SZ);
//        printf("%s\n", buffer);
//        send(new_socket, welcomeMsg.c_str(), welcomeMsg.size() + 1, 0);
//        printf("Hello message sent.\n");
    }
    
    return 0;
}

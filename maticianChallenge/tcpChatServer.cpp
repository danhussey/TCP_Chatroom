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

#define BUFFER_SZ 20000
#define MAX_CONNECTIONS 100
#define BACKLOG 100

// Helper function to validate join commands
bool validateCommand (std::vector<std::string> command) {

    // Must be 3 parameters in join command
    if (command.size() != 3) return false;
    
    // Must have case insensitive 'join' command as first parameter
    if (strncasecmp(command[0].c_str(), "join", 4) != 0) return false;
    
    for (std::string parameter : command) {
        
        // Command can't be longer than 20 characters
        size_t parLength = parameter.size();
        if (parLength > 20) return false;
        
        // Test if all characters are ASCII
        const char * c = parameter.c_str();
        for (int i = 0; i < parLength; i++) {
            if (!isascii(c[i])) return false;
        }
    }
    return true;
}
//
//bool caseInsensitiveCompare(std::string s1, std::string s2) {
//
//}

int main (int argc, char const *argv[])
{
    
    // If port # argument is specified in the executable args, use it. Otherwise, default to port 1234.
    int PORT = 1234;
    if (argc == 1 && isnumber(atoi(argv[0]))) {
        PORT = atoi(argv[0]);
    }
    
    int server_fd = 0, new_socket_fd = 0;
    int opt = 1;
    const std::pair<std::string, std::string> unsetPair = std::make_pair("Unset", "Unset");
    std::map<int, std::pair<std::string, std::string>> nicknames;
    struct sockaddr_in address; // Socket descriptor structure
    int addrlen = sizeof(address);
    char buffer[BUFFER_SZ];
    memset(buffer, '\0', BUFFER_SZ);
    std::string welcomeMsg = "Connected to chat server.\n";
    
    // Create socket FD
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Error creating socket file descriptor.");
        exit(EXIT_FAILURE);
    }
    
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
        perror("Error, setting socket options failed.");
        exit(EXIT_FAILURE);
    }
    
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
    
    if (listen(server_fd, BACKLOG) < 0)
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
         
         // Add listening socket to set
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
             if (sd > fdmax) fdmax = sd;
         }
         
         // Set up timeout for select function
//         struct timeval tv;
//         tv.tv_sec = 1;
//         tv.tv_usec = 0;
//
         // Wait for activity on a socket
         int ready_sockets = select(fdmax + 1, &socket_fds, nullptr, nullptr, nullptr);
         if (ready_sockets < 0) {
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
             
         // Check the client list fd's to see if they're set in the fd_set (meaning ready for some IO)
         for (int i = 0; i < MAX_CONNECTIONS; i++) {
             
             int temp_fd = client_sockets[i];   // Store currently 'active' socket file descriptor for use
             
             // If the current file descriptor (temp_fd) has registered an event,
             if (FD_ISSET(temp_fd, &socket_fds)) {
                 
                 ssize_t read_length;   // Store socket read size here
                 
                 // If connection was closed (read length of 0 indicates an EOF was read)
                 if ((read_length = read(temp_fd, buffer, BUFFER_SZ-1)) == 0) {
                     
                     // Someone disconnected, so find out who it was and print the details to console
                     getpeername(temp_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
                     printf("Host disconnected, ip %s, port %d, \n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                     
                     // Make a copy of the socket file descriptor so we can announce the nickname of the client who left
                     int temp_fd_copy = temp_fd;
                     
                     // Close socket and delete it from client fd list
                     close(temp_fd);
                     client_sockets[i] = 0;
                     
                     // Announce to room they left by iterating over active sockets and writing to them
                     for (int j = 0; j < MAX_CONNECTIONS; j++) {
                         int temp_fd_2 = client_sockets[j];
                         if (temp_fd_2 > 0) {
                             if (nicknames[temp_fd_2].first == nicknames[temp_fd_copy].first) {
                                 // Send message to connected clients in the same chatroom
                                 std::stringstream leaveSs;
                                 leaveSs << nicknames[temp_fd_copy].second << " has left\n";
                                 send(temp_fd_2, leaveSs.str().c_str(), leaveSs.str().size() + 1 , 0 );
                             }
                         }
                     }
                     
                     // Erase client username and chatroom name from the nicknames map now that they've left
                     nicknames.erase(temp_fd);
                     
                 }
                 
                 // Read return value of -1 indicates an error reading the socket
                 else if (read_length == -1) {
                     perror("Error reading from socket");
                     exit(EXIT_FAILURE);
                 }
                 
                 // A read return of above 0 means that a number of bytes were read. Handle the incoming bytes now.
                 else
                 {
                    // Make sure buffer is null byte terminated and format into string
                     buffer[read_length] = '\0';
                     
                     // Trim the newline and / or carriage return characters from the buffer
                     buffer[strcspn(buffer, "\r\n")] = 0;
                    
                    // Check if it's from a user without a username
                    if (nicknames[temp_fd] ==  unsetPair) {
                        
                        // The command must be a correctly parsed join command, otherwise send error and disconnect
                        // Must be 'join [room] [name]', with join case insensitive, and the room and name strings must
                        // be less than 20 characters, and made of ASCII characters only.
                        
                        // Tokenise buffer delimited by spaces
                        std::stringstream ss;
                        ss << buffer;
                        std::string token;
                        std::vector<std::string> command;
                        while (std::getline(ss, token, ' ')) {
                            command.push_back(token);
                        }
                        
                        // If join command isn't in the correct 'join [room] [name] format
                        // Or if
                        if (!validateCommand(command)) {
                            // Send error message
                            std::string error = "ERROR\n";
                            send(temp_fd, error.c_str(), error.size()+1, 0);
                            close(temp_fd);
                            client_sockets[i] = 0;
                        }
                        else {
                            
//                            if (strncasecmp(command[0].c_str(), "join", 4) == 0) {
//
//
//                            }
                            // Assign nickname and chatroom to the fd key of the client in the array
                            nicknames[temp_fd] = std::make_pair(command[1], command[2]);
                            // Announce the join to the room
                            for (int j = 0; j < MAX_CONNECTIONS; j++) {
                                int temp_fd_2 = client_sockets[j];
                                if (temp_fd_2 > 0 ) {
                                    if (nicknames[temp_fd_2].first == nicknames[temp_fd].first) {
                                        // Send message to connected clients in the same chatroom
                                        std::stringstream leaveSs;
                                        leaveSs << nicknames[temp_fd].second << " has joined\n";
                                        send(temp_fd_2, leaveSs.str().c_str(), leaveSs.str().size() + 1 , 0 );
                                    }
                                }
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
                        
                        for (int j = 0; j < MAX_CONNECTIONS; j++) {
                            
                            int temp_fd_2 = client_sockets[j];
                            if (temp_fd_2 > 0 ) {
                                if (nicknames[temp_fd_2].first == nicknames[temp_fd].first) {
                                    // Send message to connected clients in the same chatroom
                                    send(temp_fd_2, message_body.c_str(), message_body.size() + 1 , 0 );
                                }
                            }
                        }
                    }
                 }
                 // Null out the buffer
                 memset(buffer, '\0', strlen(buffer));
             }
         }
     }
    
    return 0;
}

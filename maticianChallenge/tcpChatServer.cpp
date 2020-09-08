//
//  tcpChatServer.cpp
//  maticianChallenge
//
//  Created by Daniel Hussey on 8/9/20.
//  Copyright © 2020 Daniel Hussey. All rights reserved.
//

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

int main (int argc, char const *argv[])
{
    
    // If port # argument is specified in the executable args, use it. Otherwise, default to port 1234.
    int PORT = 1234;
    if (argc == 1 && isnumber(atoi(argv[0]))) {
        PORT = atoi(argv[0]);
    }
    
    // Socket file descriptor variables
    int server_fd = 0, new_socket_fd = 0;
    int opt = 1;    // For socket configuration
    
    // Sockets are the key to a nickname - chatroom string pair, in a map used to keep track of clients
    std::map<int, std::pair<std::string, std::string>> nicknames;
    // For easy initialisation and comparisons with unconfigured but connected clients (people in the 'lobby')
    const std::pair<std::string, std::string> unsetPair = std::make_pair("Unset", "Unset");
    
    // Server buffer memory reservation
    char buffer[BUFFER_SZ];
    memset(buffer, '\0', BUFFER_SZ);    // Setting buffer memory to null byte
    
    // Setting up listener and joiner socket
    struct sockaddr_in address; // Socket descriptor structure
    int addrlen = sizeof(address);
    
    // Create socket file description (fd)
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Error creating socket file descriptor.");
        exit(EXIT_FAILURE);
    }
    
    // Cofigure socket options
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
        perror("Error, setting socket options failed.");
        exit(EXIT_FAILURE);
    }
    
    // More configuration
    address.sin_family = AF_INET; // Internet domain
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT); // Get port number and address into corrct networking format
    
    // Bind socket to the port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Error binding socket to the port.\n");
        exit(EXIT_FAILURE);
    }
    
    // Listen for connections on given port, with a backlog of 100 possible waiting clients
    if (listen(server_fd, BACKLOG) < 0)
    {
        perror("Error listening for conections.\n");
        exit(EXIT_FAILURE);
    }
    
    printf("Listening for connections.\n");
    
    // This server uses SELECT(), which requires a set of file descriptors to be kept track of for updates
    fd_set socket_fds;  // Set of file descriptors (sockets)
    int client_sockets[MAX_CONNECTIONS] = {0};  // array to keep track of client fd's, easier than reading fd_set's bit field
    
     // Infinite loop
     while (1) {
         
         // Clear file descriptor set
         FD_ZERO(&socket_fds);
         
         // Add listening socket fd to tracked fd_set
         FD_SET(server_fd, &socket_fds);
         int fdmax = server_fd;  // Max FD in fd_set needed for unix implementation of select.
         
         // Add client sockets to the fd_set
         for (int i = 0; i < MAX_CONNECTIONS; i++) {
             int sd = client_sockets[i];    // Retrieve client socket fd's
             if (sd > 0) {                  // If fd > 0, assume valid
                 // Add a valid socket descriptor to the fd_set to watch
                 FD_SET(sd, &socket_fds);
             }
             
             // Update max fd
             if (sd > fdmax) fdmax = sd;
         }
         
         // Wait for activity on a socket
         int ready_sockets = select(fdmax + 1, &socket_fds, nullptr, nullptr, nullptr);
         if (ready_sockets < 0) {
             perror("Error performing select on socket file descriptors.\n");
             exit(EXIT_FAILURE);
         };
         
         // Now check which fd's require attention, indicated by their FD_ISSET bit being set in the fd_set.
         // If the listening server fd has activated, there's a new client waiting to connect to the chatroom
         if (FD_ISSET(server_fd, &socket_fds))
         {
             // Accept the new socket and get its fd
             if ((new_socket_fd = accept(server_fd, (struct sockaddr *) &address, (socklen_t*)&addrlen)) <0)
             {
                 perror("Errror accepting new connection.\n");
                 exit(EXIT_FAILURE);
             }
             
             // Record socket number of new connection
             printf("New connection. fd: %d, IP: %s, Port: %d\n", new_socket_fd, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

             // Add new socket to list of active client sockets
             for (int i = 0; i < MAX_CONNECTIONS; i++) {
                 if (client_sockets[i] == 0) {
                     client_sockets[i] = new_socket_fd;
                     printf("Added client to list of sockets at position %d.\n", i);
                     
                     // Add clients' fd to the nickname list but indicate that they haven't set one yet
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
                 
                 // Perform a read.
                 // If connection was closed, read length == 0 and EOF was read)
                 if ((read_length = read(temp_fd, buffer, BUFFER_SZ-1)) == 0) {
                     
                     // Someone disconnected, so find out who it was and print the details to console
                     getpeername(temp_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
                     printf("Client disconnected, IP: %s, Port: %d, \n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                     
                     // Close socket and delete it from client fd list
                     close(temp_fd);
                     client_sockets[i] = 0;
                     
                     // Announce to room they left by iterating over active sockets and writing to them
                     for (int j = 0; j < MAX_CONNECTIONS; j++) {
                         int temp_fd_2 = client_sockets[j];
                         if (temp_fd_2 > 0) {
                             if (nicknames[temp_fd_2].first == nicknames[temp_fd].first) {
                                 // Send leaving message to connected clients in the same chatroom
                                 std::stringstream leaveSs;
                                 leaveSs << nicknames[temp_fd].second << " has left\n";
                                 send(temp_fd_2, leaveSs.str().c_str(), leaveSs.str().size() + 1 , 0 );
                             }
                         }
                     }
                     
                     // Erase client username and chatroom name from the nicknames map now that they've left
                     nicknames.erase(temp_fd);
                     
                 }
                 
                 // Read return value of -1 indicates an error reading the socket
                 else if (read_length == -1) {
                     // TCP Connection failure. Close the port, clean up.
                     // Close socket and delete it from client fd list
                     close(temp_fd);
                     client_sockets[i] = 0;
                     // Erase client username and chatroom name from the nicknames map now that they've left
                     nicknames.erase(temp_fd);
                     perror("Error reading from socket");
                 }
                 
                 // A read return of above 0 means that a number of bytes were read. Handle the incoming bytes now.
                 else
                 {
                    // Make sure buffer is null byte terminated and format into string
                     buffer[read_length] = '\0';
                     
                     // Trim the newline and / or carriage return characters from the buffer
                     buffer[strcspn(buffer, "\r\n")] = 0;
                    
                    // Check if it's from a user without a username or chatroom (someone in the lobby)
                    if (nicknames[temp_fd] ==  unsetPair) {
                        
                        // The command must be a correctly parsed join command, otherwise send error and disconnect
                        // Must be 'join [room] [name]', with join case insensitive, and the room and name strings must
                        // be less than 20 characters, and made of ASCII characters only, with no spaces or newlines.
                        
                        // Tokenise buffer delimited by spaces
                        std::stringstream ss;
                        ss << buffer;
                        std::string token;
                        std::vector<std::string> command;
                        while (std::getline(ss, token, ' ')) {
                            command.push_back(token);
                        }
                        
                        // If join command isn't in the correct 'join [room] [name] format
                        if (!validateCommand(command)) {
                            
                            // Send error message, close connection, and delete from nickname and chatroom map
                            std::string error = "ERROR\n";
                            send(temp_fd, error.c_str(), error.size()+1, 0);
                            nicknames.erase(temp_fd);
                            close(temp_fd);
                            client_sockets[i] = 0;
                        }
                        
                        else {
                            
                            // Assign nickname and chatroom to the fd in nicknames map
                            nicknames[temp_fd] = std::make_pair(command[1], command[2]);
                            
                            // Announce the join to the room, and only to the room
                            for (int j = 0; j < MAX_CONNECTIONS; j++) {
                                int temp_fd_2 = client_sockets[j];
                                if (temp_fd_2 > 0 ) {
                                    if (nicknames[temp_fd_2].first == nicknames[temp_fd].first) {
                                        std::stringstream leaveSs;
                                        leaveSs << nicknames[temp_fd].second << " has joined\n";
                                        send(temp_fd_2, leaveSs.str().c_str(), leaveSs.str().size() + 1 , 0 );
                                    }
                                }
                            }
                        }
                    }
                    
                    // If the message originator already has an associated chatroom and nickname
                    else {
                        
                        // Construct message body ready to be sent to the other clients in the room
                        std::ostringstream oss;
                        // Make sure buffer is null byte terminated
                        buffer[read_length] = '\0';
                        oss << nicknames[temp_fd].second << ": " << buffer << "\n";
                        std::string message_body = oss.str();
                        
                        // Send message to each of the connected clients in that room
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
                 
                 // Null out the buffer for next read
                 memset(buffer, '\0', strlen(buffer));
             }
         }
     }
    
    return 0;
}

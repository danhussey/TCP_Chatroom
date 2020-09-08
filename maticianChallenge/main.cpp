////
////  main.cpp
////  maticianChallenge
////
////  Created by Daniel Hussey on 7/9/20.
////  Copyright © 2020 Daniel Husey. All rights reserved.
////
//
//  
//#include "boost/asio.hpp"
//#include <iostream>
//#include <vector>
//#include <mutex>
//#include <thread>
////
////using namespace std;
//using namespace boost::asio;
//using namespace boost::asio::ip;
//
//typedef boost::shared_ptr<tcp::socket> socket_ptr;
//
//std::map<std::string, std::vector<std::string>> chatrooms;
//std::vector<socket_ptr> clients;
//std::mutex mtx;
//
//  
//// Driver program for receiving data from buffer
//std::string getData(tcp::socket& socket)
//{
//    streambuf buf;
//    read_until(socket, buf, "\n");
//    std::string data = buffer_cast<const char*>(buf.data());
//    return data;
//}
//  
//// Driver program to send data
//void sendData(tcp::socket& socket, const std::string& message)
//{
//    write(socket,
//          buffer(message + "\n"));
//}
//
//// Check the join command to see if it's valid within the rules
//bool validateJoinCommand(std::string join_command) {
//    return true;
//}
//
//// Announce the message to the rest of the server
//// TODO: Change to only the chatroom
//void announce(std::string data) {
//    // TODO: Newlines might have to be stripped or appended here
//    mtx.lock();
//    for (socket_ptr client : clients) {
//        sendData(*client, data);
//    }
//    mtx.unlock();
//}
//
//// Function to handle a new client connection
//void handleNewClient(int id) {
//    std::cout << "Thread " << id << " has started.\n";
//    
//    mtx.lock();
//    socket_ptr client = clients[id];
//    mtx.unlock();
//    
//    while (true) {
//        std::string data = getData(*client);
////        if find(data.begin(), data.end(), "exit") != data.end() {
////            break;
////        }
//        
//        if (data.empty()) break;
//        
//        std::cout << "Data recieved on thread " << id << ": " << data << std::endl;
//        announce(data);
//        
//    }
//    // The data stream is empty, connection closed?
//    mtx.lock();
//    clients.erase(clients.begin()+id);
//    client->close();
//    std::cout << "Closed socket id: " << id << std::endl;
//    mtx.unlock();
//    
//    
//    
//}
//  
//int main(int argc, char* argv[])
//{
//    int count = 0;
//    
//    io_service io_service;
//  
//    // Listening for any new incomming connection
//    // at port 9999 with IPv4 protocol
//    tcp::acceptor acceptor_server(
//        io_service,
//        tcp::endpoint(tcp::v4(), 9999));
//    
//    while(true) {
//      
//        // Creating socket object
////        tcp::socket server_socket(io_service);
////        clients.push_back(new tcp::socket(io_service));
//      
//        // waiting for connection
//        acceptor_server.accept(server_socket);
//        std::cout << "Connection recieved.\n";
//        
//        mtx.lock();
//        // Add client to the list of clients
//        clients.emplace_back(server_socket);
//        std::cout << "Socket added to list of clients.";
//        mtx.unlock();
//        
//        // Handle the client in a separate thread from the listener
//        std::thread _newConnectionHandlerThread (handleNewClient, count++);
//        
//        
//      
////        // Read join command
////        std::string join_command = getData(server_socket);
////        if (!validateJoinCommand(join_command)) {
////            perror("ERROR: Join command invalid.");
////            exit(1);
////        }
////
////        // Get target chat room and username from first command sent by client
////        std::stringstream ss(join_command);
////        std::string chat_room, username, command;
////        std::getline(ss, command, ' ');
////        std::getline(ss, chat_room, ' ');
////        std::getline(ss, username, '\r');
////        
////        // If the chatroom already exists
////        std::map<std::string, vector<std::string>>::iterator it = chatrooms.find(chat_room);
////        if (it != chatrooms.end()) {
////            // Create the chatroom and add the username to the chatroom
////            chatrooms[chat_room].push_back(username);
////        }
////        else {
////            // Add the username to the existing chatroom
////            // TODO: Check for username double ups
////            chatrooms[chat_room].push_back(username);
////        }
////        
////      
////        // Announce user entry into the chatroom
////        
////        
////        std::string response, reply;
////        reply = username + "has joined\n";
////        cout << "Debug: Sending message to room" << chat_room << reply;
////        sendData(server_socket, reply);
////        
////        // Push the user onto a different port?
////        
//////    while (true) {
////  
////        // Fetching response
////        response = getData(server_socket);
////  
////        // Popping last character "\n"
////        response.pop_back();
////  
////        // Validating if the connection has to be closed
////        if (response == "exit") {
////            cout << username << " left!" << endl;
////            break;
////        }
////        cout << username << ": " << response << endl;
////  
////        // Reading new message from input stream
////        cout << "Debug: ";
////        getline(cin, reply);
////        sendData(server_socket, reply);
////  
////        if (reply == "exit")
////            break;
//    }
//    return 0;
//
//}

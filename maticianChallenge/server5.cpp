//
//#include "boost/asio.hpp"
//#include <iostream>
//
//using namespace std;
//using namespace boost::asio;
//using namespace boost::asio::ip;
//
//// Driver program for receiving data from buffer
//string getData(tcp::socket& socket)
//{
//    boost::asio::streambuf buf;
//    read_until(socket, buf, "\n");
//    string data = buffer_cast<const char*>(buf.data());
//    return data;
//}
//
//// Driver program to send data
//void sendData(tcp::socket& socket, const string& message)
//{
//    write(socket,
//          buffer(message + "\n"));
//}
//
//// Check the join command to see if it's valid within the rules
//bool validateJoinCommand(string join_command) {
//    return true;
//}
//  
//int main(int argc, char* argv[])
//{
//    io_service io_service;
//
//    // Listening for any new incomming connection
//    // at port 9999 with IPv4 protocol
//    tcp::acceptor acceptor_server(
//        io_service,
//        tcp::endpoint(tcp::v4(), 9999));
//
//    // Creating socket object
//    tcp::socket server_socket(io_service);
//
//    // waiting for connection
//    acceptor_server.accept(server_socket);
//
//    // Read join command
//    string join_command = getData(server_socket);
//    if (!validateJoinCommand(join_command)) {
//        perror("ERROR: Join command invalid.");
//        exit(1);
//    }
//
//    // Get target chat room and username from first command sent by client
//    std::stringstream ss(join_command);
//    string chat_room, username, command;
//    std::getline(ss, command, ' ');
//    std::getline(ss, chat_room, ' ');
//    std::getline(ss, username, '\n');
//
//    // Removing "\n" from the username
//    // username.pop_back();
//
//    // Replying with default mesage to initiate chat
//    std::string response, reply;
//    reply = "Hello " + username + "!";
//    cout << "Debug: " << reply << endl;
//    sendData(server_socket, reply);
//
//    while (true) {
//
//        // Fetching response
//        response = getData(server_socket);
//
//        // Popping last character "\n"
//        response.pop_back();
//
//        // Validating if the connection has to be closed
//        if (response == "exit") {
//            cout << username << " left!" << endl;
//            break;
//        }
//        cout << username << ": " << response << endl;
//
//        // Reading new message from input stream
//        cout << "Debug: ";
//        getline(cin, reply);
//        sendData(server_socket, reply);
//
//        if (reply == "exit")
//            break;
//    }
//    return 0;
//
//}

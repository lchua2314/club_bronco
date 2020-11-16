#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <errno.h>
#include <thread>
#include <unordered_map> // Still doesn't fix the error at line 83
#include <list>
#include <iterator>
#include <pthread.h>

#define SA struct sockaddr

using namespace std;

class Client {
    
private:
    char s_message[2048];
    char r_message[2048];
    int sock;
    bool open_for_sending = true;
    bool open_for_receiving = true;
    int port = 4310;
    std::thread thread_send, thread_receive;

public:

    Client(int p)
    {
        port = p;
        create_server_socket();
    }

    void receive_message() 
    {
        while (open_for_receiving)
        {
            recv(sock,r_message,2048,0);
            puts(r_message);
            memset(r_message,0,sizeof(r_message));
        }
    }

    void send_message() 
    {
        while (open_for_sending)
        {
            cout << "Send a message: ";
            cin >> s_message;
            send(sock,s_message,strlen(s_message), 0);
            memset(s_message,0,sizeof(s_message));
        }
    }

    int check_if_error(int returned_value, char *error_msg)
    {
        if (returned_value < 0)
        {
            perror(error_msg);
            exit(EXIT_FAILURE);
        }
        return returned_value;
    }

    void create_server_socket()
    {
        struct sockaddr_in server_address;

        int opt = 1;

        bzero(&server_address, sizeof(server_address));
        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = INADDR_ANY;
        server_address.sin_port = htons(port);
        
        //Create the socket
        sock = socket(AF_INET, SOCK_STREAM, 0);
        
        //Check for error with socket
        check_if_error(sock, "Error with socket");

        check_if_error(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)), "setsockopt");

        //Connect to server
        connect(sock, (SA*)&server_address,sizeof(server_address));

        cout << "Listening for requests on port " << port << endl;
    }

    /*
        Function to create a request to send to the server
        method: a string that is POST, GET, etc.
        headers: the fields that will help the server determine what to do.
        
        Note: headers is an unordered map which maps a certain field to a certain
        value.

    */
    // std::string build_request(std::string method, std::list<string> headers) // argument list for class template "std::unordered_map" is missing
    // { 
    //     std::list<std::string>::const_iterator it;

    //     std::string request = "";
    //     request += (method + " / HTTP 1.1\n");
    //     for (it = headers.begin(); it != headers.end; ++it) // identifier "header" is undefined
    //     {
    //         request += it->c_str();
    //         request += " : ";
    //         ++it;
    //         request += it->c_str();
    //     }
    //     return request;
    // }

    void send_request(std::string request) {
        //char c_request[2048];
        //c_request = request.c_str(); // Error: a value of type "const char *" cannot be assigned to an entity of type "char *"
        //send(sock,c_request,strlen(c_request), 0);
    }

    int run() {
        thread_send = std::thread(&Client::send_message, this);
        thread_receive = std::thread(&Client::receive_message, this);
        thread_send.join();
        thread_receive.join();
        while (1);
        return 0;
    }


};

int main() 
{
    Client myClient(4310);
    myClient.run();
}

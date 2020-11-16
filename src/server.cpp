#include <sys/socket.h>
#include <netinet/in.h> 
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>
#include <list>
#include <iterator>
#include "Parser.c"
#include <vector>

#define SA struct sockaddr

using namespace std;

struct Character
{
    std::string name; //Name of character;
    int xpos; //x position of the character;
    int ypos;
    bool inputting;
    bool dancing;
};

static vector<Character> characters;

static list<int> clients;

bool process_request();

int check_if_error(int returned_value, char *error_msg)
{
    if (returned_value < 0)
    {
        perror(error_msg);
        exit(EXIT_FAILURE);
    }
    return returned_value;
}

int create_server_socket(int port) {

    struct sockaddr_in server_address;

    int opt = 1;

    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);
    
    //Create the socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    //Check for error with socket
    check_if_error(sock, "Error with socket");

    // Binding newly created socket to given IP and verification 
    if ((bind(sock, (SA*)&server_address, sizeof(server_address))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    } 

    check_if_error(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)), "setsockopt");

    // Attempt to make the socket (fd) a listening type socket
    check_if_error(listen(sock, 10), "Could not make the socket a listening type socket");

    cout << "Listening for requests on port " << port << endl;

    return sock;
}

void* handle_client(void* client_ptr) {
    pthread_detach(pthread_self());

    cout << "Handling client" << endl;

    int client = *((int*) client_ptr);

    //Request from the client
    while (1) {
        char request[BUFSIZ + 1];
        bzero(request, sizeof(request));
        int bytes_read = recv(client, request, sizeof(request), 0);
        check_if_error(bytes_read, "Error reading from client");

        //char
        
        char response[BUFSIZ +1];
        bzero(response,sizeof(response));
        int bytes_written = 0;
        //TODO: craft a response based on the request
        //Write the response to all the clients
        for (const auto& c : clients)
        {
            //NOTE: Eventually this will be response, not request
            bytes_written = write( c, request, sizeof(request) );
        }
    }
    close(client);
    return 0;
}

std::string process_request(char* request) {
    char *req;
    char **headers;
    char *message;
    unsigned int numHeaders = 0;

}

int main() {
    int port = 4310;
    int socket;
    socket = create_server_socket(port);

    while (true) {
        int client = accept(socket, (struct sockaddr *)NULL, NULL);
        cout << "Connected to client!" << endl;
        clients.push_back(client);

        pthread_t tid;
        int flag = pthread_create(&tid, NULL, handle_client, &client);
        check_if_error(flag, "Problem creating thread");
    }

    return 0;
}
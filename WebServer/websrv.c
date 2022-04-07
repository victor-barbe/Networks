#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

//Creating macros
#define ISVALIDSOCKET(s) ((s) >= 0) // Check and return 1 if the socket is valid
#define CLOSESOCKET(s) close(s)     // Close a socket
#define SOCKET int                  // initialize the socket
#define GETSOCKETERRNO() (errno)    // Manage errors

int main(int argc, char **argv)
{
    printf("Configuring local address...\n");
    struct addrinfo hints;            // addrinfo structure
    memset(&hints, 0, sizeof(hints)); //put hint to 0 with memset

    hints.ai_family = AF_INET;       //AF_INET because we want IP4 address
    hints.ai_socktype = SOCK_STREAM; //To use TCP
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *bind_address;                 // A pointer holding the return information from getaddrinfo()
    getaddrinfo(0, "8080", &hints, &bind_address); //getaddrinfo() to give info to a structure addrinfo with the needed information, 8080 as a port number
    printf("Creating socket...\n");

    SOCKET socket_listen;                                                                                  //defining socket_listen as a SOCKET type and the MACRO definines it as int
    socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol); //check it is valid with ISVALIDSOCKET() macro

    if (!ISVALIDSOCKET(socket_listen)) // prints error message with GETSOCKETERRNO() macro to retrieve error number
    {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return 1; // to exit our programm with a message error
    }

    printf("Binding socket to local address...\n");
    //we call bind() to associate the socket with our address from getaddrinfo()

    if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen))
    {
        fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    freeaddrinfo(bind_address); //to release address memory

    //start listening for connections with listen()
    printf("Listening...\n");

    if (listen(socket_listen, 10) < 0) //10 is use so listen() knows how many connection are allowed to queue up
    {
        fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO()); //error when listen() returns a value
        return 1;
    }

    printf("Waiting for connection...\n");
    struct sockaddr_storage client_address;
    socklen_t client_len = sizeof(client_address);

    //We store the return value of accept() in socket_client.
    //new struct sockaddr_storage gets addres info for connecting client, client_len with the size of the address

    //Here we define new variables that are need for the loop and the opening of the HTML file
    int clifd;
    int file;
    char buf[BUFSIZ];
    int size;

    while (1)
    {
        SOCKET socket_client = accept(socket_listen, (struct sockaddr *)&client_address, &client_len);

        if (!ISVALIDSOCKET(socket_client)) //test for the socket acceptance
        {
            fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO());
            return 1;
        }

        size = read(clifd, buf, BUFSIZ);
        write(1, buf, size);

        if ((file = open("index.html", O_RDONLY)) == -1) //opening HTML file in ready only
            perror("open");                              //to handle errors

        size = sprintf(buf, "HTTP/1.1 200 OK\n\n"); //print HTML header response
        size += read(file, buf + size, BUFSIZ);     //readfile and store it in buffer
        write(1, buf, size);                        //write server response
        write(clifd, buf, size);                    //to the client socket
        close(clifd);                               //close client socket and html file
        close(file);
    }
}
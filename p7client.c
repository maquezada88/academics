/*
    CS 361 Project 7, Fall 2020
    p7client.c

    Team: 24
    Names: Martin Quezada
    Honor code statement: This work abides by the JMU Honor Code.
*/

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>

#include "network.h"

void sig_handler(int signum)
{
    printf("Client: request timed out\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    int server_udp_socket;          // UDP socket for sending requests and receiving results
    struct sockaddr_in server_addr; // socket struct for server address
    uint16_t server_port;           // server port to be read from the command line
    struct comp_request request;    // struct for request message to be sent to server
    struct comp_result result;      // struct for result messge to be received from server
    long int temp;                  // variable used to convert op1 and op2 command line arguments
    int32_t op1;                    // variable to hold op1 command line argument after conversion
    int32_t op2;                    // variable to hold op2 command line argument after conversion
    char *endptr;                   // used by strtol() for error checking


    // We require 5 command line parameters as specified in the usage
    if (argc != 6) {
        printf("Usage: %s server_ip_address server_port operator op1 op2\n", argv[0]);
        exit(0);
    }

    // see if the server port command line argument is a valid 2-byte integer
    server_port = (uint16_t) (strtol(argv[2], NULL, 0) & 0xffff);
    if (server_port == 0) {
        printf("Client: invalid server port: %s\n", argv[2]);
        exit(1);
    }

    // Make sure we only accept valid operator symbols
    if (strcmp(argv[3], "+") && strcmp(argv[3], "*") && strcmp(argv[3], "/")) {
      printf("Client: operator must be one of '+', '*', or '/'\n");
      exit(1);
    }

    // Try to convert the op1 command line parameter
    errno = 0;
    temp = strtol(argv[4], &endptr, 0);
    // Check if conversion succeeded. If not, invalid characters were part of the parameter
    if (errno != 0 || *endptr != '\0') {
      printf("Client: operand 1 is not a valid integer\n");
      exit(1);
    }
    // Check if it is a signed 4-byte integer
    if (temp > INT_MAX || temp < INT_MIN) {
      printf("Client: operand 1 is out of range\n");
      exit(1);
    }
    op1 = (int32_t)temp;

    // Try to convert the op2 command line parameter
    errno = 0;
    temp = strtol(argv[5], &endptr, 0);
    // Check if conversion succeeded. If not, invalid characters were part of the parameter
    if (errno != 0 || *endptr != '\0') {
      printf("Client: operand 2 is not a valid integer\n");
      exit(1);
    }
    // Check if it is a signed 4-byte integer
    if (temp > INT_MAX || temp < INT_MIN) {
      printf("Client: operand 2 is out of range\n");
      exit(1);
    }
    op2 = (int32_t)temp;

    /* At this point, all command line arguments except the IP address have been verified
       and properly converted. You need to implement the remaining client logic below. */

//logic to send to server

    server_udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, argv[1], &(server_addr.sin_addr));

    //initialize the struct to send to server
    request.magic = 0x31363343;

    //determing the operation
    if ( strcmp(argv[3], "+" ) == 0)
    {
        request.operation = 0;
    }
    else if ( strcmp(argv[3], "*") == 0)
    {
        request.operation = 1;
    }
    else if ( strcmp(argv[3], "/") == 0) 
    {
        request.operation = 2;
    }
    else
    {
        printf("Client: Invalid operation.");
    }

    request.op1 = op1;
    request.op2 = op2;

    //send comp to server, insert if statement
    if( sendto(server_udp_socket, &request, sizeof(struct comp_request), 0, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) < 0)
    {
        printf("Client: Error sending message to server: %s\n", strerror(errno));
    }


//logic receiving from server
    
    //receive comp/struct from server
    socklen_t rec_len = sizeof(server_addr);

    signal(SIGALRM, sig_handler);
    alarm(2);
    
    ssize_t bytes_received = recvfrom(server_udp_socket, (struct comp_result *) &result, sizeof(struct comp_result), 0, (struct sockaddr *) &server_addr, &rec_len) ;

    if(bytes_received < 0)
    {
        printf("Client: Error receiving from server.\n");
    }

    if(result.magic != 0x31363343)
    {
        printf("Client: Invalid magic value received from server.\n");
    }

    if(result.success == 1)
    {
        printf("Client: an error occurred during computation\n");
        exit(0);
    }
    else
    {
        printf("Client: the result is %d\n", result.value);
    }

    //close socket
    close(server_udp_socket);

    return 0;

}


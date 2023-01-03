/*
    CS 361 Project 7, Fall 2020
    p7server.c

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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <arpa/inet.h>

#include "network.h"

int main(int argc, char *argv[]) {

    uint16_t req_listen_port;           // port number to listen for requests
    int requests_socket;		// Socket used to receive UDP requests
    struct sockaddr_in requests_address;// Our listening address
    struct sockaddr_in requestor;	// Address of the socket we received a request from
    //socklen_t requestor_size;		// Used by recvfrom
    struct comp_request request_msg;    // Buffer to receive the request message
    struct comp_result result_msg;      // Buffer to send the result message
    char name_buffer[INET_ADDRSTRLEN];  // Buffer to hold IP address of requestor

    if (argc != 2) {
        printf("Usage: %s listen_port\n", argv[0]);
        exit(0);
    }

    /* Try to convert argv[1] to requests port number */
    req_listen_port = (uint16_t) (strtol(argv[1], NULL, 0) & 0xffff);
    if (req_listen_port == 0) {
        printf("Invalid requests port: %s\n", argv[1]);
        exit(1);
    }

    // Implement the server logic below

    
    requests_socket = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&requests_address, 0, sizeof(requests_address));
    requests_address.sin_family = AF_INET;
    requests_address.sin_port = htons(req_listen_port);
    requests_address.sin_addr.s_addr = INADDR_ANY;
    

    
   if( bind(requests_socket, (struct sockaddr *) &requests_address, sizeof(struct sockaddr_in )) < 0)
    {
        printf("Error binding to socket: %s\n:", strerror(errno));
    }

    //block until someone sends us a message
    while(1)
    {

        socklen_t rec_len = sizeof(requestor);
        ssize_t bytes_received = recvfrom(requests_socket, (struct comp_request *) &request_msg, sizeof(struct comp_request), 0, (struct sockaddr *) &requestor, &rec_len);

        if(bytes_received < 0)
        {
            printf("Server: Error receiving from client: %s\n", strerror(errno));
            exit(0);
        }

        //printf("%08x %d\n", requestor.sin_addr.s_addr, requestor.sin_port);
    
        //ignore if magic value is wrong.
        if(request_msg.magic != 0x31363343)
        {
            printf("Server: received invalid magic value 0x%08x from %s, port %d\n", request_msg.magic, inet_ntop(AF_INET, &(requestor.sin_addr), name_buffer, INET_ADDRSTRLEN), ntohs(requestor.sin_port));
        }
        else
        {

        //determine the binary opterator   
            switch(request_msg.operation)
            {

                case 0:

                    result_msg.value = request_msg.op1 + request_msg.op2;
                    result_msg.success = 0;
                    break;

                case 1:

                    result_msg.value = request_msg.op1 * request_msg.op2;
                    result_msg.success = 0;
                    break;

                case 2:  
       
                    if(request_msg.op2 != 0)
                    {
                        result_msg.success = 0;
                        result_msg.value = request_msg.op1 / request_msg.op2;
                    }
                    else
                    {
                        result_msg.success = 1;
                    }
                    
                    break;

                default:
                    
                    result_msg.success = 1;                                                      
                    printf("Server: received invalid operation request: %d from %s, port %d\n", request_msg.operation, inet_ntop(AF_INET, &(requestor.sin_addr), name_buffer, INET_ADDRSTRLEN), ntohs(requestor.sin_port));
                    break;
            }
        }   
     
         //send struct to client
        result_msg.magic = 0x31363343;
    
    
    
        if( sendto(requests_socket, &result_msg, sizeof(struct comp_result), 0, (struct sockaddr *) &requestor, sizeof(struct sockaddr_in)) < 0)
        {
            printf("Server: Error sending message to client %s", strerror(errno));
        }


    }
    

    close(requests_socket);

}

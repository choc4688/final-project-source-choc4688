
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define MAX_PACKET_SIZE 65536 //Buffer size for recv. Needs to be large enough to handle long-string.txt


int main(int argc, char *argv[]) {


    //Socket Setup

    //Open a stream socket bound to port 9000
    //Fail and return -1 if any socket connection steps fail
    int sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("Failed to create socket\n");
        syslog(LOG_ERR, "Failed to create socket\n");
        return -1;
    }

    //Setting REUSEADDR to avoid bind issues:
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
        syslog(LOG_ERR, "Failed to set socket options\n");
        return -1;
    }

    struct addrinfo hints;
    struct addrinfo *servinfo;

    //Setup for getaddrinfo()
    memset(&hints, 0, sizeof(hints)); //Ensures struct is empty
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_DGRAM; //SOCK_STREAM = TCP, SOCK_DGRAM = UDP*****************
    hints.ai_family = AF_UNSPEC; 

    //NULL for first param sets identity to the program name.
    int status = getaddrinfo(NULL, "9000", &hints, &servinfo); 
    if (status != 0) {
        // perror("Failed to getaddrinfo\n");
        syslog(LOG_ERR, "getaddrinfo() failed\n");
        // free(fileMutex);
        return -1;
    }


    //Assigns address of socket to sockfd basically
    status = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
    if (status == -1) {
        //  perror("Failed to bind\n");
         syslog(LOG_ERR, "Failed to bind\n");
         freeaddrinfo(servinfo); //Identified single missing free w/ valgrind and AI
        //  free(fileMutex);
         return -1;
    }

    // //Now listen for connections on the socket
    // status = listen(sockfd, 10); 
    // if (status == -1) {
    //      perror("Failed to listen\n");
    //      syslog(LOG_ERR, "Failed listen()\n");
    // }

    //Everything above is standard, now below is the actual handling of packets


    //--------------------------------------------------

    while(1) {

         size_t totalLen = 0;
    ssize_t numRecvBytes;
    
    //Additional setup part of the main loop in aesdsocket: ********************


        char* pbuff = malloc(MAX_PACKET_SIZE); //For incoming packets
        if (!pbuff) {
            perror("Failed to malloc pbuff\n");
            syslog(LOG_ERR, "Failed pbuff malloc\n");
            continue; //Try again on the next loop iteration in case the error is recoverable / temporary (Was suggested by Copilot AI for safe memory handling tips)
        }

        // char* outpbuff = malloc(MAX_PACKET_SIZE); //For outgoing packets
        // if (!outpbuff) {
        //     perror("Failed to malloc outpbuff\n"); 
        //     syslog(LOG_ERR, "Failed outpbuff malloc\n");
        //     continue;
        // }

        //The 2 lines below are AI generated. Was needed to fix my section of code trying to get the IP address.
        struct sockaddr_storage client_addr; 
        socklen_t addr_size = sizeof(client_addr);

        // int connfd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size);
        // if (connfd == -1) {
        //     perror("Failed to accept\n");
        //     syslog(LOG_ERR, "Failed accept()\n");
        //     // freeaddrinfo(servinfo);
        //     free(pbuff);
        //     free(outpbuff);
        //     return -1;
        // }

        //Log connection + get IP addr
        //Reference: https://stackoverflow.com/questions/3060950/how-to-get-ip-address-from-sock-structure-in-c
        struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&client_addr;
        struct in_addr ipAddr = pV4Addr->sin_addr;
        char ipv4str[INET_ADDRSTRLEN];
        const char* temp = inet_ntop( AF_INET, &ipAddr, ipv4str, INET_ADDRSTRLEN );
        if (!temp) {
            perror("Error with inet_ntop\n");
            syslog(LOG_ERR, "Failed inet_ntop()\n");
        }

        syslog(LOG_INFO, "Accepted connection from %s\n", ipv4str);

    //--------------------------------------------------------

    //Actual packet handling


    //Read a packet, then send it back out************************
    //Not waiting for a '\n' character


    numRecvBytes = recvfrom(sockfd, pbuff, MAX_PACKET_SIZE, 0, (struct sockaddr*) &client_addr, &addr_size);

    if (numRecvBytes == -1) {
        perror("Failed recvfrom()");
    }

    printf("Received packet: %s\n", pbuff)


    ssize_t status = sendto(sockfd, pbuff, numRecvBytes, 0, (struct sockaddr*) &client_addr, addr_size);

    if (status == -1) {
        perror("Failed sendto()");
    }

    free(pbuff);
    // free(outpbuff);

        
    }

   

}
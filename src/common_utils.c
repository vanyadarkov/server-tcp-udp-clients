#include "common_utils.h"

void fill_sockaddr(struct sockaddr_in * structure, int port_number){
    structure->sin_family = AF_INET;
	structure->sin_port = htons(port_number);
	structure->sin_addr.s_addr = INADDR_ANY;
}

int receive_message(int socket, char * buffer){
    memset(buffer, 0, BUFLEN);
    int received = 0;
    int size = 0;
    int ret = 0;
    while(received < sizeof(int)){
        ret = recv(socket, buffer + received, sizeof(int) - received, 0);
        DIE(ret < 0, "error recv len");
        if(ret == 0) return CLIENT_STOPPED;
        received += ret;
    }
    memcpy(&size, (int *)buffer, sizeof(int));
    memset(buffer, 0, BUFLEN);
    received = 0;
    while(received < size){
        ret = recv(socket, buffer + received, size - received, 0);
        DIE(ret < 0, "error recv message");
        received += ret;
    }
    return size;
}

void send_message(int socket, char * buffer){
    int ret;
    int len = strlen(buffer) + 1;
    ret = send(socket, &len, sizeof(int), 0);
    DIE(ret < 0, "Error sending len from subscriber");
    ret = send(socket, buffer, len, 0);
    DIE(ret < 0, "Error sending id from subscriber");
}
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#ifndef __UTILS__
#define __UTILS__

#define BUFLEN 2000
#define NOT_FOUND -1
#define ALLOCATION_ERROR -1
#define RUNNING 1
#define NOT_RUNNING 0
#define CLIENT_STOPPED 0
#define ONLINE 1
#define OFFLINE 0
#define SUCCESS 0
#define FD_START 4
#define MAX_CLIENTS_WAITING 5
#define MAX_ID_LEN 11
#define MAX_TOPIC_LEN 51
#define MAX_PAYLOAD_LEN 1501
#define VERIF_PTR(a) if(a == NULL) return ALLOCATION_ERROR
#define CHECK_ALLOC(a) if(a == NULL) return NULL

#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)

/**
 * @brief Completeaza structura sockaddr_in structure cu campurile necesare
 * 
 * @param structure structura ce trebuie completata
 * @param port_number portul
 */
void fill_sockaddr(struct sockaddr_in * structure, int port_number);

/**
 * @brief Structura pentru a primi un mesaj de la un client TCP, respectand
 * conventia ca mai intai primim lungimea mesajului si dupa mesajul in sine
 * 
 * @param socket socket-ul pe care urmeaza sa primim mesajul
 * @param buffer buferul unde urmeaza salvez mesajul
 * @return CLIENT_STOPPED daca s-a facut CTRL+C (recv va primi 0) sau
 * un numar > 0 semnificand cati octeti a primit
 */
int receive_message(int socket, char * buffer);


/**
 * @brief Trimite mesajul conform conventiei ca mai intai se trimite lungimea 
 * mesajului si dupa bufferul
 * 
 * @param socket unde trimitem mesajul
 * @param buffer mesajul ce trebuie trimis
 */
void send_message(int socket, char * buffer);


#endif
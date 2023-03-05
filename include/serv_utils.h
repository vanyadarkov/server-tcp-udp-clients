#include "common_utils.h"

#ifndef __SERV_UTILS__
#define __SERV_UTILS__

/**
 * @brief Structura care va memora topicurile unui client
 */
typedef struct __client_topic__{
	char topic[MAX_TOPIC_LEN];
	int sf_flag;
} client_topic;

/**
 * @brief Structura pentru a memora mesajul in asteptare pentru client
 */
typedef struct __waiting_message__{
	char message[BUFLEN];
} waiting_message;

/**
 * @brief Structura care reprezinta un client
 */
typedef struct __client__{
    int fd;
    char id[MAX_ID_LEN];
    int online_status;
    int nr_of_following_topics;
    client_topic * topics;
	int nr_of_waiting_messages;
	waiting_message * waiting_messages;
} client;

void server_usage(char *file);

/**
 * @brief Parsarea comenzii primita de la STDIN in server
 * 
 * @param buffer comanda primita
 * @param tcp_socket socketul pentru conexiunile tcp
 * @param udp_socket socketul pentru clientii udp
 * @return NOT_RUNNING daca s-a primit comanda exit si RUNNING in caz contrar
 */
int server_receive_command(char * buffer, int tcp_socket, int udp_socket);

/**
 * @brief Adaugarea unui nou client la clientii deja existenti
 * 
 * @param clients clientii deja existenti
 * @param new_client clientul de adaugat
 * @param nr_of_clients numarul de clienti existenti
 * @return SUCCESS/ALLOCATION_ERROR
 */
int add_client(client** clients, client * new_client, int* nr_of_clients);

/**
 * @brief Initializarea unui nou client
 * 
 * @param new_tcp_client socketul clientului nou
 * @param buffer id-ul clientului
 * @return client* clientul alocat
 */
client* init_new_client(int new_tcp_client, char * buffer);

/**
 * @brief Cautarea unui client dupa id
 * 
 * @param clients clientii existenti
 * @param nr_of_clients nr de clienti existenti
 * @param id id-ul
 * @return indexul clientului sau NOT_FOUND
 */
int search_client_by_id(client * clients, int nr_of_clients, char * id);

/**
 * @brief Cautarea unui client dupa fd
 * 
 * @param clients clientii existenti
 * @param nr_of_clients nr de clienti existenti
 * @param fd file descriptorul clientului de cautat
 * @return indexul clientului sau NOT_FOUND
 */
int search_client_by_fd(client * clients, int nr_of_clients, int fd);

/**
 * @brief Abonarea clientului la un topic
 * 
 * @param client clientul
 * @param topic topicul la care el se aboneaza
 * @param sf parametrul SF pentru acest topic
 * @return SUCCESS sau ALLOCATION_ERROR in caz de eroare la alocarea topicului
 */
int subscribe_client(client * client, char * topic, int sf);

/**
 * @brief Dezabonarea clientului de la un topic
 * 
 * @param client clientul pentru care dorim sa ne dezabonam
 * @param topic topicul de la care ne dezabonam
 * @return SUCCESS sau NOT_FOUND in caz ca nu gasim topicul
 */
int unsubscribe_client(client * client, char * topic);

/**
 * @brief Cautarea topicului in lista de topicuri a clientului
 * 
 * @param client clientul pentru care cautam
 * @param topic topicul cautat
 * @return indexul topicului pentru acest client sau NOT_FOUND
 */
int find_client_topic(client * client, char * topic);

/**
 * @brief Pregatirea unui mesaj primit de la UDP pentru trimitere catre TCP
 * 
 * @param buffer mesajul primit de la UDP
 * @param udp_info structura care contine informatii despre clientul UDP
 * @param topic string in care salvam topicul mesajului (se aloca in interiorul functiei,
 * se trimite ca referinta din exterior)
 * @return SUCCESS sau ALLOCATION_ERROR in caz de erori de alocare
 */
int prepare_message_for_tcp(char * buffer, struct sockaddr_in * udp_info, char ** topic);

/**
 * @brief Trimite mesajele care stau in asteptare pentru un client
 * 
 * @param client clientul pentru care se trimit mesajele in asteptare
 */
void send_waiting_messages(client * client);

/**
 * @brief Adaugarea unui mesaj in waiting_messages pentru un client
 * 
 * @param client clientul pentru care adaugam
 * @param message mesajul de adaugat
 * @return SUCCESS sau ALLOCATION_ERROR in caz de erori de alocare pentru
 * vectorul de mesaje in asteptare
 */
int add_to_waiting_messages(client * client, char * message);

/**
 * @brief Trimiterea mesajului catre toti clientii care sunt abonati la el. Se 
 * verifica daca clientul e online, iar daca nu se verifica parametrul SF setat
 * pentru topic, in caz ca e abonat la el. Pentru SF = 1 se va salva in 
 * waiting_messages
 * 
 * @param clients clientii
 * @param nr_of_clients nr de clienti
 * @param topic topicul mesajului
 * @param message mesajul propriu zis
 */
void send_messages_to_subscribers(client * clients, int nr_of_clients, 
                                        char * topic, char * message);

#endif
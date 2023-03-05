#include "serv_utils.h"

void server_usage(char *file)
{
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}

int server_receive_command(char * buffer, int tcp_socket, int udp_socket){
    int ret;
    fgets(buffer, BUFLEN, stdin);
    if(strcmp(buffer, "exit\n") == 0){
        // trebuiesc inchisi socketii clientilor activi
        ret = close(tcp_socket);
        DIE(ret < 0, "Closing tcp failed");
        ret = close(udp_socket);
        DIE(ret < 0, "Closing udp failed");
        ret = NOT_RUNNING;
    } else {
        printf("Command not found\n");
        ret = RUNNING;
    }
    return ret;
}

client* init_new_client(int new_tcp_client, char * buffer){
    client* new_client = calloc(1, sizeof(client));

    CHECK_ALLOC(new_client);

    new_client->fd = new_tcp_client;
    memcpy(new_client->id, buffer, strlen(buffer) + 1);
    new_client->nr_of_following_topics = 0;
    new_client->nr_of_waiting_messages = 0;
    new_client->online_status = ONLINE;
    new_client->topics = NULL;
    new_client->waiting_messages = NULL;
    return new_client;
}

int add_client(client** clients, client * new_client, int* nr_of_clients){
    *nr_of_clients = *nr_of_clients + 1;
    client * tmp;
    tmp = realloc(*clients, *nr_of_clients * sizeof(client));
    VERIF_PTR(tmp);
    *clients = tmp;
    memcpy(&((*clients)[*nr_of_clients - 1]), new_client, sizeof(client));
    return SUCCESS;
}

int search_client_by_id(client * clients, int nr_of_clients, char * id){
    for(int i = 0; i < nr_of_clients; i++) {
        if(strcmp(clients[i].id, id) == SUCCESS){
            return i;
        }
    }
    return NOT_FOUND;
}

int search_client_by_fd(client * clients, int nr_of_clients, int fd){
    for(int i = 0; i < nr_of_clients; i++) {
        if(clients[i].fd == fd){
            return i;
        }
    }
    return NOT_FOUND;
}

int subscribe_client(client * client, char * topic, int sf){
    int index = find_client_topic(client, topic);
    if(index != -1) return SUCCESS;
    client->nr_of_following_topics++;
    client_topic * tmp;
    tmp = realloc(client->topics, 
                        client->nr_of_following_topics * sizeof(client_topic));
    VERIF_PTR(tmp);
    client->topics = tmp;
    strcpy(client->topics[client->nr_of_following_topics - 1].topic, topic);
    client->topics[client->nr_of_following_topics - 1].sf_flag = sf;
    return SUCCESS;
}

int find_client_topic(client * client, char * topic){
    for(int i = 0; i < client->nr_of_following_topics; i++){
        if(strcmp(client->topics[i].topic, topic) == 0) {
            return i;
        }
    }
    return NOT_FOUND;
}

int unsubscribe_client(client * client, char * topic){
    int topic_index = find_client_topic(client, topic);
    if(topic_index != NOT_FOUND){
        memmove(client->topics + topic_index, 
                    client->topics + topic_index + 1, 
                    (client->nr_of_following_topics - topic_index - 1) * 
                                sizeof(*client->topics));
        client->nr_of_following_topics--;
        return SUCCESS;
    }
    return NOT_FOUND;
}

/**
 * @brief Formarea stringului de informatie in functie de tipul de date primit 
 * ca parametru
 * 
 * @param data_type tipul de date care urmeaza sa extragem din payload
 * @param payload payload de unde extragem informatia
 * @return string care contine toata informatia necesara
 */
char * get_type_string(uint8_t * data_type, char * payload){
    char * format = calloc(BUFLEN, sizeof(char)); 
    switch (*data_type)
    {
    case 0:
        {
            uint8_t * sign = (uint8_t *)payload;
            uint32_t * number = (uint32_t *)(payload + (sizeof(*sign)));
            int final = ntohl(*number);
            sprintf(format, "INT - %d", (*sign == 1) ? -1 * final : final);
            return format;
        }
    case 1:
        {
            uint16_t * short_real = (uint16_t *)payload;
            float number = ntohs((*short_real)) / 100.0;
            sprintf(format, "SHORT_REAL - %.2f", number);
            return format;
        }
    case 2:
        {
            uint8_t * sign = (uint8_t *)payload;
            uint32_t * number = (uint32_t *)(payload + (sizeof(*sign)));
            uint8_t * exp = (uint8_t *)(payload + (sizeof(*sign)) + sizeof(*number));
            int divide = 1;
            for(uint8_t i = 0; i < *exp; i++) divide *= 10;
            float res = ntohl((*number)) / (float) divide;
            sprintf(format,"FLOAT - %.4f", (*sign == 1) ? -1 * res : res);
            return format;
        }
    case 3:
        {
            sprintf(format, "STRING - %s", payload);
            return format;
        }
    default:
        {
            strcpy(format, "");
            return format;
        }
    }
}

int prepare_message_for_tcp(char * buffer, 
                            struct sockaddr_in * udp_info, 
                            char ** topic) {
                            
    *topic = calloc(MAX_TOPIC_LEN, sizeof(char));

    VERIF_PTR(*topic);

    uint8_t * data_type = calloc(1, sizeof(uint8_t));

    VERIF_PTR(data_type);

    char * payload = calloc(MAX_PAYLOAD_LEN, sizeof(char));

    VERIF_PTR(payload);
    
    memcpy(*topic, buffer, MAX_TOPIC_LEN - 1);
    memcpy(data_type, buffer + MAX_TOPIC_LEN - 1, sizeof(uint8_t));
    memcpy(payload, buffer + MAX_TOPIC_LEN, MAX_PAYLOAD_LEN);
    char * type_string = get_type_string(data_type, payload);
    sprintf(buffer, "%s:%hu - %s - %s", inet_ntoa(udp_info->sin_addr),
                                        ntohs(udp_info->sin_port), 
                                        *topic, type_string);
    free(type_string);
    free(data_type);
    free(payload);
    return SUCCESS;
}

void send_messages_to_subscribers(client * clients, int nr_of_clients, 
                                    char * topic, char * message) {
    int ret = 0;
    for(int i = 0; i < nr_of_clients; i++){
        for(int j = 0; j < clients[i].nr_of_following_topics; j++){
            if(strcmp(clients[i].topics[j].topic, topic) == 0) {
                if(clients[i].online_status == 1){
                    send_message(clients[i].fd, message);
                } else {
                    if(clients[i].topics[j].sf_flag == 1) {
                        ret = add_to_waiting_messages(&clients[i], message);
                        DIE(ret == ALLOCATION_ERROR, "allocation error");
                    }
                }
            }
        }
    }
}

int add_to_waiting_messages(client * client, char * message){
    client->nr_of_waiting_messages = client->nr_of_waiting_messages + 1;
    waiting_message * tmp;
    tmp = realloc(client->waiting_messages, 
                    client->nr_of_waiting_messages * sizeof(waiting_message));
    VERIF_PTR(tmp);
    client->waiting_messages = tmp;
    strcpy(client->waiting_messages[client->nr_of_waiting_messages - 1].message, 
            message);
    return SUCCESS;
}

void send_waiting_messages(client * client) {
    for(int i = 0; i < client->nr_of_waiting_messages; i++){
        send_message(client->fd, client->waiting_messages[i].message);
    }
    free(client->waiting_messages);
    client->waiting_messages = NULL;
    client->nr_of_waiting_messages = 0;
}

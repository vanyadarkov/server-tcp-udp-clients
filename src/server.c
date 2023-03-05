#include "serv_utils.h"

int main(int argc, char *argv[]){
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    if(argc < 2) {
        server_usage(argv[0]);
    }

    int ret;
    char buffer[BUFLEN];

    int tcp_socket;
    int udp_socket;
    uint16_t port_number;

    struct sockaddr_in tcp_client_addr, udp_client_addr;
    socklen_t tcp_client_len = sizeof(tcp_client_addr);
    socklen_t udp_client_len = sizeof(udp_client_addr);

    fd_set read_fds;
    fd_set tmp_fds;
    int fdmax;

    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);

    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    DIE(tcp_socket < 0, "TCP socket error");

    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    DIE(udp_socket < 0, "UDP socket error");

    port_number = atoi(argv[1]);
    DIE(port_number < 1024, "Port rezervat de sistem de operare");

    // Completeaza informatii server si clienti
    fill_sockaddr(&tcp_client_addr, port_number);
    fill_sockaddr(&udp_client_addr, port_number);

    // Bind pentru socket la un port si setare in multime de fd
    ret = bind(tcp_socket, (struct sockaddr *)&tcp_client_addr, 
                                            sizeof(struct sockaddr));
    DIE(ret < 0, "Bind TCP socket");
    FD_SET(tcp_socket, &read_fds);
    ret = bind(udp_socket, (struct sockaddr *)&udp_client_addr, 
                                            sizeof(struct sockaddr));
    DIE(ret < 0, "Bind UDP socket");
    FD_SET(udp_socket, &read_fds);

    // Actualizare fdmax
    if(udp_socket > tcp_socket) {
        fdmax = udp_socket;
    } else {
        fdmax = tcp_socket;
    }

    ret = listen(tcp_socket, MAX_CLIENTS_WAITING);
        DIE(ret < 0, "Error server listen");

    // Setarea pentru citire de la STDIN
    FD_SET(STDIN_FILENO, &read_fds);
    int running = RUNNING;

    client * clients = NULL;
    int nr_of_clients = 0;
    
    while(running){
        tmp_fds = read_fds;
        
        ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
        DIE(ret < 0, "Select error");
        
        for(int i = 0; i <= fdmax; i++) {
            if(FD_ISSET(i, &tmp_fds)){
                if(i == tcp_socket) {
                    // TCP Client nou
                    int new_tcp_client = accept(tcp_socket, 
                                        (struct sockaddr *)&tcp_client_addr,
                                        &tcp_client_len);
                    DIE(new_tcp_client < 0, "new tcp client error accept");

                    int turn_off = 1;
                    ret = setsockopt(new_tcp_client, IPPROTO_TCP, TCP_NODELAY, 
                                            (char *)&turn_off, sizeof(int));
                    DIE(ret < 0, "TCP_NODELAY failed server");
                    receive_message(new_tcp_client, buffer);
                    // Cautam clientul
                    int client_index = search_client_by_id(clients, 
                                                            nr_of_clients, 
                                                            buffer);
                    if(client_index != NOT_FOUND) {
                        if(clients[client_index].online_status == 1) {
                            printf("Client %s already connected.\n", buffer);
                            // Trebuie deconectat clientul care a incercat sa 
                            // se conecteze
                            strcpy(buffer, "exit");
                            send_message(new_tcp_client, buffer);
                            continue;
                        } else {
                            // clientul este deja in baza de date, il setam 
                            // online
                            clients[client_index].online_status = ONLINE;
                            clients[client_index].fd = new_tcp_client;
                            send_waiting_messages(&clients[client_index]);
                        }
                    } else {
                        client* new_client = init_new_client(new_tcp_client, 
                                                                    buffer);
                        DIE(new_client == NULL, "Error init client");
                        ret = add_client(&clients, new_client, &nr_of_clients);
                        DIE(ret == ALLOCATION_ERROR, 
                                    "Cannot add new client to the list");
                        free(new_client);
                    }
                    FD_SET(new_tcp_client, &read_fds);
                    if(new_tcp_client > fdmax) {
                        fdmax = new_tcp_client;
                    }
                    printf("New client %s connected from %s:%hu.\n", 
                                buffer, inet_ntoa(tcp_client_addr.sin_addr), 
                                tcp_client_addr.sin_port);
                }
                else if(i == udp_socket) {
                    // UDP message
                    memset(buffer, 0, BUFLEN);
                    ret = recvfrom(udp_socket, buffer, BUFLEN, 0, 
                                        (struct sockaddr *)&udp_client_addr, 
                                        &udp_client_len);
                    DIE(ret < 0, "error recv from udp");
                    char * topic = NULL;
                    ret = prepare_message_for_tcp(buffer, &udp_client_addr, 
                                                            &topic);
                    DIE(ret == ALLOCATION_ERROR, 
                                    "error preparing message for tcp");
                    send_messages_to_subscribers(clients, nr_of_clients, 
                                                        topic, 
                                                        buffer);
                    free(topic);
                }
                else if(i == STDIN_FILENO) {
                    // STDIN command
                    running = server_receive_command(buffer, 
                                                        tcp_socket, 
                                                        udp_socket);
                    if(running == NOT_RUNNING) {
                        strcpy(buffer, "exit");
                        for(int i = 0; i < nr_of_clients; i++) {
                            send_message(clients[i].fd, buffer);
                            free(clients[i].topics);
                            free(clients[i].waiting_messages);
                        }
                        free(clients);
                        continue;
                    }
                }
                else {
                    // Comanda de la client tcp
                    ret = receive_message(i, buffer);
                    int index = search_client_by_fd(clients, nr_of_clients, i);
                    if(index == NOT_FOUND) {
                        printf("Client not found\n");
                        continue;
                    }
                    if(ret == CLIENT_STOPPED || strcmp(buffer, "exit") == SUCCESS) {
                        printf("Client %s disconnected.\n", clients[index].id);
                        clients[index].online_status = OFFLINE;
                        FD_CLR(i, &read_fds);
                        int k = fdmax;
                        // Recalcularea fdmax
                        while(k > 2){
                            if(FD_ISSET(k, &read_fds)){
                                fdmax = k;
                                break;
                            }   
                            k--;
                        }
                    } else {
                        const char del[2] = " ";
                        char * token = strtok(buffer, del);
                        if(strcmp(token, "subscribe") == SUCCESS){
                            // subscribe client
                            char * topic = strtok(NULL, del);

                            int sf = atoi(strtok(NULL, del));

                            ret = subscribe_client(&clients[index], topic, sf);
                            
                            if(ret == ALLOCATION_ERROR) {
                                // Trimitere raspuns
                                strcpy(buffer, "error");
                                send_message(i, buffer);
                            }
                            DIE(ret == ALLOCATION_ERROR, 
                                    "Cannot add topic to client");
                            // Trimitere raspuns
                            strcpy(buffer, "success");
                            send_message(i, buffer);
                        } else if (strcmp(token, "unsubscribe") == SUCCESS){
                            // unsubscribe client
                            char * topic = strtok(NULL, del);
                            ret = unsubscribe_client(&clients[index], topic);
                            if(ret != SUCCESS) {
                                printf("Topic not found. Can't unsubscribe.\n");
                                strcpy(buffer, "not_found");
                                send_message(i, buffer);
                            } else {
                                strcpy(buffer, "success");
                                send_message(i, buffer);
                            }
                        } else {
                            printf("Unknown command from client\n");
                            continue;
                        }
                    }
                }
            }
        }
    }
    return SUCCESS;
}
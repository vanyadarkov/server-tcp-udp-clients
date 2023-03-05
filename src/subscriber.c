#include "subs_utils.h"

int main(int argc, char * argv[]){
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    if(argc != 4) {
        client_usage(argv[0]);
    }

    int ret;
    int running = RUNNING;
    char buffer[BUFLEN];

    int sub_sock;
    uint16_t port_number = atoi(argv[3]);
    DIE(port_number < 1024, "Port rezervat de sistem de operare");
    struct sockaddr_in client_addr;

    fd_set read_fds;
    fd_set tmp_fds;

    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);

    sub_sock = socket(AF_INET, SOCK_STREAM, 0);
    DIE(sub_sock < 0, "Error socket subscriber");

    fill_sockaddr(&client_addr, port_number);
    ret = inet_aton(argv[2], &client_addr.sin_addr);
    DIE(ret == 0, "Subscriber ip incorrect");

    ret = connect(sub_sock, 
                    (struct sockaddr *) &client_addr, 
                    sizeof(client_addr));
    FD_SET(sub_sock, &read_fds);
    FD_SET(STDIN_FILENO, &read_fds);
    
    int turn_off = 1;
    ret = setsockopt(sub_sock, IPPROTO_TCP, TCP_NODELAY, 
                        (char *)&turn_off, sizeof(int));
    DIE(ret < 0, "TCP_NODELAY failed subscriber");

    send_message(sub_sock, argv[1]);
    while(running) {
        tmp_fds = read_fds;
        memset(buffer, 0, BUFLEN);
        ret = select(sub_sock + 1, &tmp_fds, NULL, NULL, NULL);
        DIE(ret < 0, "Error select in subscriber");

        if(FD_ISSET(STDIN_FILENO, &tmp_fds)) {
            // mesaj stdin
            ret = parse_input(buffer, sub_sock);
            if(ret == INVALID_COMMAND) continue;
            else if (ret == STOP) running = 0;
        } else if (FD_ISSET(sub_sock, &tmp_fds)) {
            // mesaj de la server
            ret = receive_message(sub_sock, buffer);
            if(strcmp(buffer, "exit") == 0 || ret == 0){
                running = 0;
            } else {
                printf("%s\n", buffer);
            }
        }
    }
    ret = close(sub_sock);
    DIE(ret < 0, "Error stopping client");
    return 0;
}
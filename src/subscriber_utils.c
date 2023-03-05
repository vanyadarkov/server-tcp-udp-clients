#include "subs_utils.h"

void client_usage(char *file)
{
	fprintf(stderr, "Usage: %s client_id server_ip server_port\n", file);
	exit(0);
}

int parse_input(char * buffer, int sub_sock){
	fgets(buffer, BUFLEN, stdin);
	buffer[strlen(buffer) - 1] = 0;

	if(strcmp(buffer, "exit") == 0){
		send_message(sub_sock, buffer);
		return STOP;
	}

	const char del[2] = " ";
	char * token = strtok(buffer, del);

	if(token == NULL) {
		printf("Empty input!\n");
		return INVALID_COMMAND;
	}

	if(strcmp(token, "subscribe") == 0){
		char * topic = strtok(NULL, del);

		if(topic == NULL) {
			printf("No topic given\n");
			return INVALID_COMMAND;
		}

		token = strtok(NULL, del);

		if(token == NULL) {
			printf("Invalid sf\n");
			return INVALID_COMMAND;
		}

		int sf = atoi(token);

		if(sf < 0 || sf > 1) {
			printf("Invalid sf\n");
			return INVALID_COMMAND;
		}

		snprintf(buffer, BUFLEN, "%s %s %d", "subscribe", topic, sf);
		// Trimitere catre server
		send_message(sub_sock, buffer);
		// Primire raspuns server
		receive_message(sub_sock, buffer);
		if(strcmp(buffer, "success") == 0) {
			printf("Subscribed to topic.\n");
		} else if(strcmp(buffer, "error") == 0) {
			printf("Error from server\n");
		}
	} else if(strcmp(token, "unsubscribe") == 0) {
		char * topic = strtok(NULL, del);

		if(topic == NULL) {
			printf("No topic given!\n");
			return INVALID_COMMAND;
		}
		snprintf(buffer, BUFLEN, "%s %s", "unsubscribe", topic);
		// Trimitere catre server
		send_message(sub_sock, buffer);
		// Primire raspuns
		receive_message(sub_sock, buffer);
		if(strcmp(buffer, "success") == 0) {
			printf("Unsubscribed from topic.\n");
		} else if(strcmp(buffer, "not_found") == 0) {
			printf("Not subscribed to topic.\n");
		}
	} else {
		printf("Unknown command\n");
		return INVALID_COMMAND;
	}
	return SUCCESS;
}
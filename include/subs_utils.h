#include "common_utils.h"

#ifndef __SUBS_UTILS__
#define __SUBS_UTILS__

#define INVALID_COMMAND 1
#define STOP -1
#define NOT_SUBSCRIBED 2

void client_usage(char *file);

/**
 * @brief Parsarea unei comenzi de la STDIN si trimitere catre server
 * 
 * @param buffer comanda primita de la STDIN
 * @param sub_sock socketul pe care trimitem comanda
 * @return SUCCESS, STOP in caz ca am primit EXIT sau INVALID_COMMAND in caz de 
 * comanda invalida
 */
int parse_input(char * buffer, int sub_sock);

#endif
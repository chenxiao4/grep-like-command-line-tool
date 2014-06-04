#ifndef REMOTECLIENT_H
#define REMOTECLIENT_H


int IS_LOCAL(char *fname);
int remotefile(char *string, char *node, char *serv, char *fname);
void *client_agent(void *params);
void UpdateClientStat(NODE_t *node, INFO_t *statcs);
void client(char *serv, char *node, NODE_t *root, MESSAGE_t msg);
void StatToh(INFO_t *statcs);
#endif

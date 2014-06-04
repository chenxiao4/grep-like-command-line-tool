#ifndef REMOTESERVER_H
#define REMOTESERVER_H

void thread_init();
void *server_agent(void *params);
void listener(char *server_port, char *interface_name, int argc, char *argv[]);
void StatToNet(INFO_t *statcs);
void OptToh(MESSAGE_ONE_t *type1);
void OptToNet(MESSAGE_ONE_t *type1);

#endif

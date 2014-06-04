#ifndef SENDRECV_H
#define SENDRECV_H

int
our_send_message(int fd, HTYPE_t type, usint length,void *user_data);

int
our_recv_message(int fd, HTYPE_t *type, usint *length,void *user_data);

#endif

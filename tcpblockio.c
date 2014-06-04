/* tcpblockio.c  - a set of functions that define a block i/o interface to TCP.
 *		   These functions encapsulate all the steps needed to:
 *			- read and write blocks of a specified size
 *			- open clients and listeners
 *		must be linked with -lnsl and -lsocket on solaris
 */
#include "commonincludes.h"


/* Returns after all nbytes characters from buffer have been written to fd.
 * Will return sooner if an error is encountered.
 * Value returned is actual number of bytes written, or -errno if hit an error.
 */
int
writeblock(int fd, char *buffer, int nbytes)
{
	int n;			/* number of bytes actually sent in one write*/
	int offset;		/* total number of bytes sent so far */

	for (offset = 0; offset < nbytes; offset += n) {
		if ((n = send(fd, &buffer[offset], nbytes-offset, 0)) < 0) {
			if (errno == EINTR)
				n = 0;	/* interrupted by signal, keep going */
			else {
				if (offset == 0)
					offset = -errno;
				perror("send to socket");
				break;
			}
		}
	}
	return offset;		/* return total number of bytes actually sent*/
}	/* writeblock */

/* Returns after all nbytes characters have been read into buffer from fd.
 * Will return sooner if an end of file is encountered.
 * Value returned is actual number of bytes read, or -errno if hit an error.
 */
int readblock(int fd, char *buffer, int nbytes)
{
	int n;		/* number of bytes actually received in one read */
	int offset;	/* total number of bytes received so far */

	for (offset = 0; offset < nbytes; offset += n) {
		if ((n = recv(fd, &buffer[offset], nbytes-offset, 0)) < 0) {
			if (errno == EINTR)
				n = 0;	/* interrupted by signal, keep going */
			else {
				if (offset == 0)
					offset = -errno;
				perror("recv from socket");
				break;
			}
		} else if( n == 0 )
			break;		/* end of file on receive */
	}
	return offset;		/* return total number of bytes actually read*/
}	/* readblock */

/* Opens a new TCP socket to server on node server_node, port server_port.
 * Returns in server_addr the server's IPv4 address structure,
 * and in client_addr the client's IPv4 address structure.
 * Returns an fd that is connected to the server, or -1 on an error.
 */
int openclient(char *server_port, char *server_node,
		struct sockaddr *server_addr,
		struct sockaddr *client_addr)
{
	int fd, err;
	socklen_t len;
	char local_node[MAXHOSTNAMELEN];
	struct addrinfo *aptr, hints;
	/* get internet name of the local host node on which we are running */
	if (gethostname(local_node, MAXHOSTNAMELEN) < 0) {
		perror("openclient gethostname");
		return -1;
	}
	/* set up the name of the remote host node for the server */
	if (server_node == NULL)
	  server_node = local_node;	/* default to local node */

	/* get structure for remote host node on which server resides */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if  ((err = getaddrinfo(server_node, server_port, &hints, &aptr))) {
		fprintf(stderr, "%s port %s: %s\n", server_node, server_port,
			gai_strerror(err));
		return -1;
	}

	/* open an internet tcp socket for client to connect on */
	if ((fd = socket(aptr->ai_family, aptr->ai_socktype, 0)) < 0) {
		perror("openclient socket");
		freeaddrinfo(aptr);
		return -1;
	}

	/* connect this socket to the server's Internet address */
	if (connect(fd, aptr->ai_addr, aptr->ai_addrlen) < 0 ) {
		perror("openclient connect");
		close(fd);
		freeaddrinfo(aptr);
		return -1;
	}

	if (client_addr) {
		/* caller wants local port number assigned to this client */
		len = sizeof(struct sockaddr);
		if (getsockname(fd, client_addr, &len) < 0 ) {
			perror("client getsockname");
			close(fd);
			freeaddrinfo(aptr);
			return -1;
		}

	if (server_addr != NULL)
		/* caller wants server addess info returned */
		memcpy(server_addr, aptr->ai_addr, aptr->ai_addrlen);
	}

	/* we are now successfully connected to a remote server */
	freeaddrinfo(aptr);
	return fd;		/* return fd of connected socket */
}	/* openclient */


/* Opens a new TCP socket for the listener at interface listen_name,
 * port listen_port.  Returns in listen_addr the listener's internet
 * address structure.  The listener is NOT connected to a client on return.
 * Returns an fd that is a "listening post" on which to make connections.
 */
int
openlistener(char *listen_port, char *listen_name,
	    struct sockaddr *listen_address)
{
	int fd, err;
	socklen_t len;
	struct addrinfo *aptr, hints;
	char *host_name, *host_port;

	if (listen_name == NULL)
		host_name = "0.0.0.0";		/* all available interfaces */
	else
		host_name = listen_name;
	if (listen_port == NULL)
		host_port = "0";		/* for printing errors only */
	else
		host_port = listen_port;
	/* get structure for local interface listener is to use */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if  ((err = getaddrinfo(host_name, listen_port, &hints, &aptr))) {
		fprintf(stderr, "%s: %s port %s\n", host_name, host_port,
			gai_strerror(err));
		return -1;
	}
	/* open an internet tcp socket for listener to listen on */
	if ((fd = socket(aptr->ai_family, aptr->ai_socktype, 0)) < 0) {
		perror("openlistener socket");
		freeaddrinfo(aptr);
		return -1;
	}
	/* bind this socket to the listener's Internet address */
	if (bind(fd, aptr->ai_addr, aptr->ai_addrlen) < 0) {
		perror("openlistener bind");
		close(fd);
		freeaddrinfo(aptr);
		return -1;
	}
	/* set up listening backlog for connect requests from clients */
	if (listen(fd, LISTENING_DEPTH) < 0) {
		perror("openlistener listen");
		close(fd);
		freeaddrinfo(aptr);
		return -1;
	}
	if (listen_address) {
		/* caller wants local port number assigned to this listener */
		len = sizeof(struct sockaddr);
		if (getsockname(fd, listen_address, &len) < 0 ){
			perror("openlistener getsockname");
			close(fd);
			freeaddrinfo(aptr);
			return -1;
		}
	}

	/* we are now successfully established as a listener */
	freeaddrinfo(aptr);
	return fd;			/* return fd of listening socket */
}	/* openlistener */

/* vi: set autoindent tabstop=8 shiftwidth=8 : */

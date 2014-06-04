/* send_recv.c */

#include "commonincludes.h"

/* writes a fixed-size header followed by variable length data
 *
 * fills in header with values from type, length and info
 * writes length bytes of variable length data from user_data
 *
 * returns 0 if all ok, else -1 on any error
 */
int
our_send_message(int fd, HTYPE_t type, usint length,
		 void *user_data)
{
	HD_t	header;

	header.type = htonl(type);
	header.length = htonl(length);
	//	header.info = htonl(info);

	/* write fixed-size header */
	if (writeblock(fd, (void *)&header, sizeof(header)) != sizeof(header)) {
		return -1;
	}

	if (length > 0 ) {
		if (writeblock(fd, user_data, length) != (signed)length) {
			return -1;
		}
	}
	return 0;
}	/* our_send_message */

/* reads a fixed-size header followed by variable length data
 *
 * fills in type, length and info with values from the header
 * reads up to *length bytes of variable length data into user_data
 *
 * returns number of bytes of user_data read, or -1 on error
 */
int
our_recv_message(int fd, HTYPE_t *type, usint *length,
		 void *user_data)
{
	HD_t	header;
	usint	user_data_length;

	user_data_length = *length;
	if (readblock(fd, (void *)&header, sizeof(header)) != sizeof(header)) {
		return -1;
	}

	*type = ntohl(header.type);
	*length = ntohl(header.length);
	//	*info = ntohl(header.info);

	if (*length > user_data_length) {
		fprintf(stderr, "message length %u longer than available "
				"length %u\n", *length, user_data_length);
		return -1;
	}

	if (*length > 0)
		return readblock(fd, user_data, *length);

	return 0;
}	/* our_recv_message */

/* vi: set autoindent tabstop=8 shiftwidth=8 : */

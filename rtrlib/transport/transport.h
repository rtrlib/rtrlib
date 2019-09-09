/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

/**
 * @defgroup mod_transport_h Transport sockets
 * @brief The RTR transport sockets implement the communication channel
 * (e.g., SSH, TCP, TCP-AO) between an RTR server and client.
 * @details Before using the transport socket, a tr_socket must be
 * initialized based on a protocol-dependent init function (e.g.,
 * tr_tcp_init()).\n
 * The tr_* functions call the corresponding function pointers, which are
 * passed in the tr_socket struct, and forward the remaining arguments.
 *
 * @{
 */

#ifndef RTR_TRANSPORT_H
#define RTR_TRANSPORT_H

#include <time.h>

/**
 * @brief The return values for tr_ functions.
 */
enum tr_rtvals {
	/** @brief Operation was successful. */
	TR_SUCCESS = 0,

	/** Error occured. */
	TR_ERROR = -1,

	/** No data is available on the socket. */
	TR_WOULDBLOCK = -2,

	/** Call was interrupted from a signal */
	TR_INTR = -3,

	/** Connection closed */
	TR_CLOSED = -4
};

struct tr_socket;

/**
 * @brief A function pointer to a technology specific close function.
 * \sa tr_close
 */
typedef void (*tr_close_fp)(void *socket);

/**
 * @brief A function pointer to a technology specific open function.
 * \sa tr_open
 */
typedef int (*tr_open_fp)(void *socket);

/**
 * @brief A function pointer to a technology specific free function.
 * All memory associated with the tr_socket will be freed.
 * \sa tr_free
 */
typedef void (*tr_free_fp)(struct tr_socket *tr_sock);

/**
 * @brief A function pointer to a technology specific recv function.
 * \sa tr_recv
 */
typedef int (*tr_recv_fp)(const void *socket, void *pdu, const size_t len, const time_t timeout);

/**
 * @brief A function pointer to a technology specific send function.
 * \sa tr_send
 */
typedef int (*tr_send_fp)(const void *socket, const void *pdu, const size_t len, const time_t timeout);

/**
 * @brief A function pointer to a technology specific info function.
 * \sa tr_send
 */
typedef const char *(*tr_ident_fp)(void *socket);

/**
 * @brief A transport socket datastructure.
 *
 * @param socket A pointer to a technology specific socket.
 * @param open_fp Pointer to a function that establishes the socket connection.
 * @param close_fp Pointer to a function that closes the socket.
 * @param free_fp Pointer to a function that frees all memory allocated with this socket.
 * @param send_fp Pointer to a function that sends data through this socket.
 * @param recv_fp Pointer to a function that receives data from this socket.
 */
struct tr_socket {
	void *socket;
	tr_open_fp open_fp;
	tr_close_fp close_fp;
	tr_free_fp free_fp;
	tr_send_fp send_fp;
	tr_recv_fp recv_fp;
	tr_ident_fp ident_fp;
};

#endif
/** @} */

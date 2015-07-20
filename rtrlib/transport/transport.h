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
    /** @brief Operation was successfull. */
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
 * */
typedef void (*tr_close_fp)(void *socket);

/**
 * @brief A function pointer to a technology specific open function.
 * \sa tr_open
 * */
typedef int (*tr_open_fp)(void *socket);

/**
 * @brief A function pointer to a technology specific free function. All memory associated with the tr_socket will be freed.
 * \sa tr_free
 * */
typedef void (*tr_free_fp)(struct tr_socket *tr_sock);

/**
 * @brief A function pointer to a technology specific recv function.
 * \sa tr_recv
 * */
typedef int (*tr_recv_fp)(const void *socket, void *pdu, const size_t len, const time_t timeout);

/**
 * @brief A function pointer to a technology specific send function.
 * \sa tr_send
 * */
typedef int (*tr_send_fp)(const void *socket, const void *pdu, const size_t len, const time_t timeout);

/**
 * @brief A function pointer to a technology specific info function.
 * \sa tr_send
 * */
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


/**
 * @brief Establish the connection.
 * @param[in] socket Socket that will be used.
 * @return TR_SUCCESS On success.
 * @return TR_ERROR On error.
 */
int tr_open(struct tr_socket *socket);

/**
 * @brief Close the socket connection.
 * @param[in] socket Socket that will be closed.
 */
void tr_close(struct tr_socket *socket);

/**
 * @brief Deallocates all memory that the passed socket uses.
 * Socket have to be closed before.
 * @param[in] socket which will be freed.
 */
void tr_free(struct tr_socket *socket);

/**
 * @brief Receives <= len Bytes data from the socket.
 * @param[in] socket Socket that will be used.
 * @param[out] buf Received data, must be an allocated memory area of >=pdu_len bytes.
 * @param[in] len Size of pdu in Bytes.
 * @param[in] timeout Max. seconds the function will block till len data was received.
 * @return >0 Number of Bytes read.
 * @return TR_ERROR On error.
 * @return TR_WOULDBLOCK If no data was available at the socket before the timeout expired.
 */
int tr_recv(const struct tr_socket *socket, void *buf, const size_t len, const time_t timeout);

/**
 * @brief Send <= len Bytes data over the socket.
 * @param[in] socket Socket that will be used.
 * @param[out] pdu Data that will be be sent.
 * @param[in] len Size of pdu in Bytes.
 * @param[in] timeout Max. seconds the function should try to send the data till it returns.
 * @return >0 Number of Bytes sent.
 * @return TR_ERROR On error.
 */
int tr_send(const struct tr_socket *socket, const void *pdu, const size_t len, const time_t timeout);

/**
 * Repeatly calls tr_send(..) till len Bytes were sent, the timeout expired or an error occured.
 * @param[in] socket Socket that will be used.
 * @param[out] pdu Data that will be be sent.
 * @param[in] len Size of pdu in Bytes.
 * @param[in] timeout Max. seconds the functions should try to send pdu till it returns.
 * @return >0 Number of Bytes sent.
 * @return TR_ERROR On Error.
 * @return TR_WOULDBLOCK If send would block.
 */
int tr_send_all(const struct tr_socket *socket, const void *pdu, const size_t len, const time_t timeout);

/**
 * Repeatly calls tr_recv(..) till len Bytes were received, the timeout expired or an error occured.
 * @param[in] socket Socket that will be used.
 * @param[out] buf Received data, must be an allocated memory area of >=len bytes.
 * @param[in] len Size of pdu in Bytes.
 * @param[in] timeout Max. seconds the functions should try to receive len data till it returns.
 * @return >0 Number of Bytes received.
 * @return TR_ERROR On error.
 * @return TR_WOULDBLOCK If send would block.
 */
int tr_recv_all(const struct tr_socket *socket, const void *buf, const size_t len, const time_t timeout);

/**
 * Returns an identifier for the socket endpoint, eg host:port.
 * @param[in] socket
 * return Pointer to a \0 terminated String
 * return NULL on error
 */
const char *tr_ident(struct tr_socket *socket);

#endif
/* @} */

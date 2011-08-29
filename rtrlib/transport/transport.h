/*
 * This file is part of RTRlib.
 *
 * RTRlib is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * RTRlib is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with RTRlib; see the file COPYING.LESSER.
 *
 * written by Fabian Holler, in cooperation with:
 * INET group, Hamburg University of Applied Sciences,
 * CST group, Freie Universitaet Berlin
 * Webpage: https://rpki.realmv6.org/
 */

/**
 * @defgroup mod_transport_h Transport sockets
 * @brief Interface for RTR transports.
 * A RTR-transport implements the communication channel between a RTR server and client, e.g. SSH, TCP, TCP-AO. \n
 * A transport implemention have to implement an init function with initializes a tr_socket. \n
 *
 * @{
 */

#ifndef RTR_TRANSPORT_H
#define RTR_TRANSPORT_H

#include <time.h>

/**
 * @brief The return values for tr_ functions.
 */
enum tr_rtvals{
    /** @brief Operation was successfull. */
    TR_SUCCESS = 0,
    /** Error occured. */
    TR_ERROR = -1,
    /** No data is available on the socket. */
    TR_WOULDBLOCK = -2,
    /** Call was interrupted from a signal */
    TR_INTR = -3
};

struct tr_socket;

/** 
 * @brief A function pointer to a technology specific close function.
 * \sa tr_close
 * */
typedef void (*tr_close_fp)(void* socket);

/** 
 * @brief A function pointer to a technology specific open function.
 * \sa tr_open
 * */
typedef int (*tr_open_fp)(void* socket);

/** 
 * @brief A function pointer to a technology specific free function. All memory associated with tr_sock->socket will be freed.
 * \sa tr_free
 * */
typedef void (*tr_free_fp)(struct tr_socket* tr_sock);

/** 
 * @brief A function pointer to a technology specific recv function.
 * \sa tr_recv
 * */
typedef int (*tr_recv_fp)(const void* socket, void* pdu, const size_t len, const time_t timeout);

/** 
 * @brief A function pointer to a technology specific send function.
 * \sa tr_send
 * */
typedef int (*tr_send_fp)(const void* socket, const void* pdu, const size_t len, const time_t timeout);

/**
 * @brief A transport socket datastructure.
 *
 * @param techn_name The name of the protocol that this tr_socket uses.
 * @param socket A pointer to a technology specific socket.
 * @param open_fp Pointer to a function that establish the socket connection.
 * @param close_fp Pointer to a function that close the socket.
 * @param free_fp Pointer to a function that frees all memory allocated with this socket.
 * @param send_fp Pointer to a function which sends data through this socket.
 * @param recv_fp Pointer to a function which receives data from this socket.
 */
typedef struct tr_socket{
    const char* proto_name;
    void* socket;
    tr_open_fp open_fp;
    tr_close_fp close_fp;
    tr_free_fp free_fp;
    tr_send_fp send_fp;
    tr_recv_fp recv_fp;
} tr_socket;

/* @} */

/**
 * @defgroup mod_transport_conv Transport socket convenience functions
 * @ingroup mod_transport_h
 * @brief The functions call the corresponding function pointer in the tr_socket struct and forward the other function
 * arguments.
 * @{
 */

/**
 * @brief Establish the connection.
 * @param[in] socket Socket that will be used.
 * @return TR_SUCCESS
 * @return TR_ERROR
 */
int tr_open(tr_socket* socket);

/** 
 * @brief Close the socket connection.
 * @param[in] socket Socket that will be closed.
 */
void tr_close(tr_socket* socket);

/**
 * @brief Deallocate all memory that socket uses.
 * @param[in] tr_sock tr_sock->socket that will be freed.
 */
void tr_free(tr_socket* tr_sock);

/**
 * @brief Receive data from the socket (non-blocking).
 * Received <= len Bytes data.
 * @param[in] socket Socket that will be used.
 * @param[out] buf Received data, must be an allocated memory area of >=pdu_len bytes.
 * @param[in] len Size of pdu in Bytes.
 * @param[in] timeout Max. seconds the functions should try to recv len data till it returns.
 * @return >0 Number of Bytes read.
 * @return TR_ERROR
 * @return TR_WOULDBLOCK If no data was available on socket before the timeout expired.
 */
int tr_recv(const tr_socket* socket, void* buf, const size_t len, const time_t timeout);

/**
 * @brief Send data throught the socket (non-blocking).
 * <= len Bytes will be sent.
 * @param[in] socket Socket that will be used.
 * @param[out] pdu Data that will be be sent.
 * @param[in] len Size of pdu in Bytes.
 * @param[in] timeout Max. seconds the functions should try to send pdu till it returns.
 * @return >0 Number of Bytes sent.
 * @return TR_ERROR
 */
int tr_send(const tr_socket* socket, const void* pdu, const size_t len, const time_t timeout);

/**
 * Repeatly calls tr_send till len Bytes were sent or an error occured.
 * @param[in] socket Socket that will be used.
 * @param[out] pdu Data that will be be sent.
 * @param[in] len Size of pdu in Bytes.
 * @param[in] timeout Max. seconds the functions should try to send pdu till it returns.
 * @return >0 Number of Bytes sent.
 * @return TR_ERROR
 * @return TR_WOULDBLOCK If send would block.
 */
int tr_send_all(const tr_socket* socket, const void* pdu, const size_t len, const time_t timeout);

/**
 * Repeatly calls tr_send(socket, pdu, len, -1) till len Bytes were received or an Error occured.
 * @param[in] socket Socket that will be used.
 * @param[out] buf Received data, must be an allocated memory area of >=len bytes.
 * @param[in] len Size of pdu in Bytes.
 * @param[in] timeout Max. seconds the functions should try to recv len data till it returns.
 * @return >0 Number of Bytes received.
 * @return TR_ERROR
 */
int tr_recv_all(const tr_socket* socket, const void* buf, const size_t len, const time_t timeout);

#endif
/* @} */

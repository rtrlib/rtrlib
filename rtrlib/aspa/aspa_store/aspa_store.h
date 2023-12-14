/*
* This file is part of RTRlib.
*
* This file is subject to the terms and conditions of the MIT license.
* See the file LICENSE in the top level directory for more details.
*
* Website: http://rtrlib.realmv6.org/
*/

#ifndef RTR_ASPA_STORE_H
#define RTR_ASPA_STORE_H

#include "rtrlib/aspa/aspa_array/aspa_array.h"
#include "rtrlib/rtr/rtr.h"

/**
 * @brief A linked list storing the bond between a socket and an @c aspa_array .
 */
struct aspa_store_node {
	struct aspa_array *aspa_array;
	struct rtr_socket *rtr_socket;
	struct aspa_store_node *next;
};

/**
 * @brief Inserts a new bond between the given socket and @c aspa_array .
 * @param store The ASPA store the new node is inserted into.
 * @param rtr_socket The socket to use in the new node.
 * @param aspa_array The ASPA array to store alognside the socket.
 * @returns @c ASPA_SUCCES or @c ASPA_ERROR if the insert operations fails.
 */
int aspa_store_insert(struct aspa_store_node **store, struct rtr_socket *rtr_socket, struct aspa_array *aspa_array);

/**
 * @brief Removes the node associated with the given socket from the given ASPA store.
 * @param store The ASPA store containg the node.
 * @param rtr_socket The socket whose associated node will be removed.
 */
void aspa_store_remove(struct aspa_store_node **store, struct rtr_socket *rtr_socket);

/**
 * @brief Searches the given ASPA store for a node associated with the given socket.
 * @param store The store to search in.
 * @param rtr_socket The socket of the node.
 * @return A pointer to the @c aspa_store_node , if found, @c NULL otherwise.
 */
struct aspa_store_node **aspa_store_search_node(struct aspa_store_node **store, const struct rtr_socket *rtr_socket);

/**
 * @brief Searches the given ASPA store for an ASPA array associated with the given socket.
 * @param store The store to search in.
 * @param rtr_socket The socket of the node.
 * @return A pointer to the @c aspa_array of the node, if found, @c NULL otherwise.
 */
struct aspa_array **aspa_store_search_array(struct aspa_store_node **node, const struct rtr_socket *rtr_socket);

#endif

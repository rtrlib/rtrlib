/*
* This file is part of RTRlib.
*
* This file is subject to the terms and conditions of the MIT license.
* See the file LICENSE in the top level directory for more details.
*
* Website: http://rtrlib.realmv6.org/
*/

#include "aspa_store.h"

#include "rtrlib/aspa/aspa_private.h"
#include "rtrlib/lib/alloc_utils_private.h"

int aspa_store_insert(struct aspa_store_node **store, struct rtr_socket *rtr_socket, struct aspa_array *aspa_array)
{
	// Allocate new node
	struct aspa_store_node *new = lrtr_malloc(sizeof(struct aspa_store_node));

	if (new == NULL)
		return ASPA_ERROR;

	// Store socket and ASPA array
	new->rtr_socket = rtr_socket;
	new->aspa_array = aspa_array;

	// prepend new node
	new->next = *store; // may be NULL
	*store = new;

	return ASPA_SUCCESS;
}

void aspa_store_remove(struct aspa_store_node **head, struct rtr_socket *rtr_socket)
{
	if (head == NULL || *head == NULL)
		return;

	// If first node matches
	if (*head != NULL && (*head)->rtr_socket == rtr_socket) {
		struct aspa_store_node *tmp = *head;
		*head = (*head)->next;
		lrtr_free(tmp);
		return;
	}

	struct aspa_store_node *node = *head;
	struct aspa_store_node *prev;

	// First node is guaranteed not to match
	do {
		prev = node;
		node = node->next;
	} while (node != NULL && node->rtr_socket != rtr_socket);

	if (node == NULL)
		return;

	prev->next = node->next;
	lrtr_free(node);
}

struct aspa_array **aspa_store_search(struct aspa_store_node **node, const struct rtr_socket *rtr_socket)
{
	if (node == NULL || *node == NULL)
		return NULL;

	//	struct aspa_store_node *node = *head;

	while (*node != NULL) {
		if ((*node)->rtr_socket == rtr_socket) {
			return &(*node)->aspa_array;
		}
		node = &(*node)->next;
	}

	return NULL;
}

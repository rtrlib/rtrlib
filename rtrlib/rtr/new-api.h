//Internal types
struct rtr_transport_socket;
struct rtr_mgr_group;
struct rtr_mgr;

//external types
typedef struct rtr_transport_socket rtr_transport_socket_t;
typedef struct rtr_mgr_group rtr_mgr_group_t;
typedef struct rtr_mgr rtr_mgr_t;

/**
 * @brief Create a TCP socket that represents a connection to
 * a cache server.
 *
 * @param host The address of the target cache server
 * @param port The port of the target cache server
 * @param bind_to_address The interface where the socket should be bound to.
 * If NULL is passed, the socket is bound to all interfaces.
 * @return NULL on error
 */
rtr_transport_socket_t *rtr_transport_tcp_socket_create(
                                              char *host,
                                              uint16_t port,
                                              char *bind_to_address);

/**
 * @brief Free all resources that are associated with the given socket.
 *
 * @param socket The socket that should be freed
 */
void rtr_transport_free(rtr_transport_socket_t *socket);

/**
 * @brief Add a transport socket (cache_socket) to the group target_group.
 * If target_group is NULL, a new group is returned.
 *
 * @param target_group The group the cache_socket should be added to. If NULL is
 * passed, a new group will be returned
 * @param cache_socket The socket that should be added to target_group
 * @param preference The preference value of the cache that belongs to the
 * cache_socket
 * @param refresh_interval //TODO:
 * @param expire_interval //TODO:
 * @param retry_interval //TODO:
 * @return RTR_SUCCESS on success, else RTR_ERROR
 */
rtr_mgr_group_t *rtr_group_add_cache_socket(rtr_mgr_group_t *target_group,
                                            rtr_transport_socket_t *cache_socket,
                                            uint8_t preference,
                                            const unsigned int refresh_interval,
                                            const unsigned int expire_interval,
                                            const unsigned int retry_interval
                                            );

/**
 * @brief Free all resources associated with the given rtr_mgr_group_t.
 *
 * @param group The group that should be freed.
 */
void rtr_group_free(rtr_mgr_group_t *group);

/**
 * @brief Initialize a new rtr manager.
 *
 * @param update_fp [description]
 * @param spki_update_fp [description]
 * @param status_fp [description]
 * @param status_fp_data [description]
 * @return A new instance of rtr_mgr_t on success, else NULL
 */
rtr_mgr_t *rtr_mgr_init(const pfx_update_fp update_fp,
                        const spki_update_fp spki_update_fp,
                        const rtr_mgr_status_fp status_fp,
                        void *status_fp_data);

/**
 * @brief Add a group of cache server sockets to a rtr_manager
 *
 * @param mgr The rtr manager the group should be added to
 * @param group The group that should be added to the rtr manager
 *
 * @return RTR_SUCCESS on success, else RTR_ERROR
 */
int rtr_mgr_add_group(rtr_mgr_t *mgr, rtr_mgr_group_t *group);

/**
 * @brief Start the manager and connect to an associated cache server.
 *
 * @param mgr The manager that should be started
 * @return RTR_SUCCESS on success, else RTR_ERROR
 */
int rtr_mgr_start(rtr_mgr_t *mgr);

/**
 * @brief Stop the manager and close all associated cache server sockets.
 *
 * @param mgr The manager that should be stopped
 * @return RTR_SUCCESS on success, else RTR_ERROR
 */
int rtr_mgr_stop(rtr_mgr_t *mgr);

/**
 * @brief Free all resources that are associated with the given rtr_mgr_t.
 *
 * @param mgr The rtr_mgr_t that should be freed.
 */
void rtr_mgr_free(rtr_mgr_t *mgr);
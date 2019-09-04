/*
 * This file is part of RTRlib.
 *
 * Copyright (c) 2019, 6WIND
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */
#ifndef LRTR_UTILS_VRF_PRIVATE_H
#define LRTR_UTILS_VRF_PRIVATE_H

#include "lrtr_vrf.h"

/**
 * @brief Tells if the passed vrfname is reachable, or can be ignored
 * if name is NULL, or vrf support not needed, then return 1
 * if vrf name can be accessed, then return 1
 * @param[name] vrf name
 * @return 1 on successs
 * @return -1 on error
 */
int lrtr_vrf_api_usable(const char *name);

/**
 * @brief switchback to initial context, this call is needed after operating on vrf
 * operating on vrf here stands for either create socket ( socket()), or call
 * specific function calls ( getaddrinfo()).
 * this call lowers permission rights if needed
 * @param[name] vrf name
 * @return >=0 on success
 * @return -1 on error
 */
int lrtr_vrf_switchback(void);

/**
 * @brief switch from context, this call is needed to operate on vrf
 * operating on vrf here stands for either create socket ( socket()), or call
 * specific function calls ( getaddrinfo()).
 * if name is NULL, or vrf support not needed, then return 0
 * this call also raises permission rights if needed
 * @param[name] vrf name
 * @return >=0 on success
 * @return -1 on error
 */
int lrtr_vrf_switch_to(const char *name);

/**
 * @brief with vrf-lite, socket must be bound to vrf. bind() is called if needed.
 * @param[socket] socket created
 * @param[name] vrf name
 * @return >=0 on success
 * @return -1 on error
 */
int lrtr_vrf_bind(int socket, const char *vrfname);

#endif
